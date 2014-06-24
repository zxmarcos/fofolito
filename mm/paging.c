/* 
 * FOFOLITO - Sistema Operacional para RaspberryPi
 *
 * Marcos Medeiros
 */

#include <kernel/mm.h>
#include <kernel/printk.h>
#include <asm/page.h>
#include <asm/asm.h>
#include <asm/offset.h>
#include <errno.h>
#include <memory.h>
#include <types.h>

/*
 * Configura todas as páginas do kernel
 */
static void setup_kernel_dir()
{
	/* Limpa o diretório */
	memset(&k_pgdir, 0, sizeof(struct page_dir));

	char *vaddr = (char *) PAGE_OFFSET;
	char *endaddr = (char *) (PAGE_OFFSET + STATIC_MEM);

	unsigned pfn = 0;

	while (vaddr <= endaddr) {
		page_map(&k_pgdir, vaddr, pfn);
		vaddr += PAGE_SIZE;
		pfn++;
	}

}

int paging_init()
{
	setup_kernel_dir();
	page_load_directory(&k_pgdir);
	return -EOK;
}