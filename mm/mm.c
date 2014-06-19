/* 
 * FOFOLITO - Sistema Operacional para RaspberryPi
 * 
 * Gerenciador de memória física
 * A ideia é utilizar um bitmap, e uma stack com frames livres
 * Marcos Medeiros
 */

#include <kernel/mm.h>
#include <kernel/printk.h>
#include <asm/page.h>
#include <asm/asm.h>
#include <asm/offset.h>
#include <errno.h>
#include <types.h>

#define DEBUG_PAGE_ALLOC	1

unsigned int pmm_bitmap_size = 0;
/* bitmap das páginas */
unsigned int *pmm_bitmap = NULL;
/* número total de páginas de memória */
unsigned int pmm_total_pages = 0;
/* memória total no sistema */
unsigned int total_memory = 0;
unsigned int k_addr_start = 0;
unsigned int k_addr_end = 0;
unsigned int k_used_pages = 0;
unsigned int pmm_allocated_pages = 0;

void page_mark_kernel(unsigned int start, unsigned int end);
int page_find_free();

/*
 * Altera o estado de uma página de memória.
 */
static inline void page_set(unsigned int page, int free)
{
	unsigned int pos = (page / 32);
	unsigned int bit = (page % 32) ^ 31;

	printk("ps[%d]-%d\n", page, free);

	if (free) 
		pmm_bitmap[pos] &= ~(1 << bit);
	else
		pmm_bitmap[pos] |= (1 << bit);
}

static inline void k_mark_pages(unsigned start, unsigned end)
{
	while (start <= end) {
		page_set(start, 0);
		pmm_allocated_pages++;
		k_used_pages++;
		start++;
	}
}

void page_mark_kernel(unsigned int start, unsigned int end)
{
	/* marca o todo o kernel */
	unsigned int ps = start >> PAGE_SHIFT;
	unsigned int pe = (end >> PAGE_SHIFT) + 1;
	k_mark_pages(ps, pe);
	
	/* vamos marcar o diretório de páginas também */
	ps = k_pgdir >> PAGE_SHIFT;
	pe = k_pgdir_end >> PAGE_SHIFT;
	k_mark_pages(ps, pe);
	
	/* agora vamos marcar o próprio bitmap */
	ps = (unsigned int) pmm_bitmap >> PAGE_SHIFT;
	pe = (unsigned int) (pmm_bitmap + pmm_bitmap_size) >> PAGE_SHIFT;
	k_mark_pages(ps, pe);
}

unsigned int page_alloc_init(unsigned int memory,
							 unsigned int kstart,
							 unsigned int kend)
{
	k_addr_start = kstart;
	k_addr_end = kend;
	/* definie o número total de páginas unitárias na memória */
	unsigned int npages = memory >> PAGE_SHIFT;

	/* Nosso primeiro bitmap fica logo após o termino do kernel */
	pmm_bitmap = (unsigned int *) (kend);
	pmm_total_pages = npages;
	total_memory = memory;

	pmm_bitmap_size = npages / 32;
	pmm_allocated_pages = 0;

	/* Limpa o bitmap */
	unsigned int k = 0;
	while (k < (npages / 32)) {
		pmm_bitmap[k] = 0;
		k++;
	}
	/* Vamos marcar as páginas do kernel como utilizadas */
	page_mark_kernel(0, k_addr_end);
	return -EOK;
}

void pages_info()
{
	printk("Memory pages information:\n");
	printk("\tTotal: %d bytes\t\t\tFrames %d/%d\n", total_memory, pmm_allocated_pages, pmm_total_pages);
	printk("\tKernel 0x%x:0x%x\tKFrames %d\n", k_addr_start, k_addr_end, k_used_pages);
}

/*
 * Encontra uma página livre de memória
 */
int page_find_free()
{
	unsigned int k = 0;
	/* número da pagina */
	int page = -1;
	for (; k < pmm_bitmap_size; k++) {
		unsigned int bits = pmm_bitmap[k];
		/* Se este bloco já estiver preenchido pulamos para o próximo */
		if (bits == 0xFFFFFFFF)
			continue;
		
		/* Procura a posição da página livre */
		int p = 31;
		while (p > 0) {
			if (!(bits & 0x80000000)) {
				/* Cálcula o número da página */
				page = (p ^ 31) + (k * 32);
				return page;
			}
			bits <<= 1;
			p--;
		}
	}
	return page;
}

/*
 * Verifica o estado de uma página, retorna qualquer valor diferente
 * de 0 caso a página já esteja alocada.
 */
static inline int page_status(unsigned int page)
{
	return pmm_bitmap[page / 32] & (1 << (page ^ 31));
}

/*
 * Aloca uma página livre.
 */
void *page_alloc()
{
	int pos = page_find_free();
	if (pos < 0)
		return NULL;
	pmm_allocated_pages++;

#if DEBUG_PAGE_ALLOC
	printk("page_alloc pfn-> %d\n", pos);
#endif

	page_set(pos, 0);

	/* Retorna o endereço no espaço do kernel */
	return __phys_to_virt(pos << PAGE_SHIFT);
}

/*
 * Aloca n páginas consecutivas.
 * Algoritmo muito lento para muitas páginas ou quando já existem muitas páginas alocadas.
 */
void *page_alloc_n(unsigned int npages)
{
	unsigned int notfound = 1;
	unsigned int page_start = 0;
	unsigned int block = 0;
	unsigned long addr = 0;

	if (npages >= (pmm_total_pages - pmm_allocated_pages))
		return NULL;

#if DEBUG_PAGE_ALLOC
	printk("page_alloc_n: %d\n", npages);
#endif

	while (notfound) {
		unsigned int full = 0;
		/* Apenas pulamos os blocos já preenchidos */
		while (pmm_bitmap[page_start >> PAGE_SHIFT] == 0xFFFFFFFF) {
			page_start += 32;
			if (page_start >= pmm_total_pages)
				return NULL;
		}

		/* faz uma busca linear */
		for (block = 0; block < npages; block++) {
			if (page_status(page_start + block)) {
				full = 1;
				/* Pulamos para essa página agora */
				page_start += block;
				break;
			}
		}

		if (!full)
			notfound = 0;
		else page_start++;
	}
	pmm_allocated_pages += npages;

	for (block = 0; block < npages; block++)
		page_set(page_start + block, 0);

	addr = page_start << PAGE_SHIFT;
	return __phys_to_virt(addr);
}


/*
 * Aloca n páginas consecutivas, alinhado.
 * Algoritmo muito lento para muitas páginas ou quando já existem muitas páginas alocadas.
 */
void *page_alloc_align_n(unsigned int npages, unsigned int align)
{
	unsigned int block = 0;
	unsigned int page_start = 0;
	unsigned int notfound = 1;
	unsigned long addr = 0;

	if (npages >= (pmm_total_pages - pmm_allocated_pages))
		return NULL;

#if DEBUG_PAGE_ALLOC
	printk("page_alloc_align_n: %d:%d\n", npages, align);
#endif

	while (notfound) {
		unsigned int full = 0;

		/* Procura no alinhamento, uma primeira página livre */
		while (page_status(page_start)) {
			page_start += align;
			if ((page_start + npages) >= pmm_total_pages)
				return NULL;
		}

		/* faz uma busca linear */
		for (block = 0; block < npages; block++) {
			if (page_status(page_start + block)) {
				full = 1;
				break;
			}
		}
		if (!full)
			notfound = 0;
		else page_start += align;
	}
	pmm_allocated_pages += npages;
	for (block = 0; block < npages; block++)
		page_set(page_start + block, 0);

	addr = page_start << PAGE_SHIFT;
	return __phys_to_virt(addr);
}
