/* 
 * FOFOLITO - Sistema Operacional para RaspberryPi
 * Funções relacionadas ao mapeamento de paginação.
 * Marcos Medeiros
 */
#include <asm/asm.h>
#include <asm/page.h>
#include <asm/offset.h>
#include <asm/mmu.h>
#include <kernel/mm.h>
#include <memory.h>
#include <errno.h>
#include <types.h>

#define DEBUG_PAGEMAP	1

static inline void pde_init(pde_t *pde, struct page_table *table)
{
	pde->type = PDE_TYPE_COARSE;
	pde->domain = DOMAIN_KERNEL;
	pde->p = 0;
	pde->ns = 0;
	pde->table = __pa(table) >> PGT_SHIFT;
}

static inline void pte_init(pte_t *pte, unsigned frame)
{
	pte->frame = frame;
	pte->_one = 1;
	pte->ap = PROT_KERNEL;
}

int page_map(struct page_dir *dir, void *va, unsigned frame)
{
	pde_t *pde;
	pte_t *pte;

	unsigned long vaddr = (unsigned long) va;

	pde = pde_offset(dir, vaddr);

	/* Se a tabela pra essa entra ainda não existir */
	if (pde_none(pde)) {
		/*
		 * Aloca uma nova page_table
		 * XXX: Implementar um alocador de page_tables, pois aqui estamos
		 *      desperdiçando muito espaço, uma vez que uma page_table tem 1KB
		 *      e uma página tem 4KB
		 */
		struct page_table *table = page_alloc();
		if (!table) {
			return -ENOMEM;
		}
		memset(table, 0, sizeof(struct page_table));

		pde_init(pde, table);
	}

	pte = pte_offset_fast(pde, vaddr);
	pte_init(pte, frame);

	return -EOK;
}


int page_load_directory(struct page_dir *dir)
{
	/* Como os endereços são virtuais precisamos converte-los */
	mmu_set_ttbr((struct page_dir *)__pa(dir));
	return -EOK;
}