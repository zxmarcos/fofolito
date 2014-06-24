/*
 * FOFOLITO - Sistema Operacional para RaspberryPi
 * Remapeador de MMIO
 * Marcos Medeiros
 */

#include <asm/page.h>
#include <asm/memlayout.h>
#include <kernel/mm.h>
#include <kernel/printk.h>
#include <kernel/list.h>
#include <errno.h>

static char *ioremap_base;
static unsigned ioremap_page = 0;

struct ioremap_area {
	struct list_head list;
	void *vaddr;
	unsigned start;
	unsigned size;
	unsigned pages;
};

static struct list_head io_areas;

int ioremap_init()
{
	INIT_LIST_HEAD(&io_areas);
	ioremap_base = (char *) IOREMAP_START;
	ioremap_page = 0;
	return -EOK;
}


static inline
unsigned pages_between(unsigned addr, unsigned size)
{
	unsigned start = addr >> PAGE_SHIFT;
	unsigned end = (addr + size);

	if (end & PAGE_MASK) {
		end &= ~PAGE_MASK;
		end += PAGE_SIZE;
	}

	return (end >> PAGE_SHIFT) - start;
}

void *ioremap(unsigned long base, unsigned size)
{
	struct ioremap_area *area = kmalloc(sizeof(*area));
	void *addr = NULL;

	if (!area) {
		return NULL;
	}

	/* O tamanho que o caller pediu */
	area->size = size;
	/* O endereço inicial da página */
	area->vaddr = ioremap_base + (ioremap_page * PAGE_SIZE);
	area->start = base;
	area->pages = pages_between(base, size);

	list_add(&area->list, &io_areas);

	unsigned pfn = base >> PAGE_SHIFT;

	for (unsigned i = 0; i < area->pages; i++) {
		page_map(&k_pgdir, ioremap_base + (ioremap_page * PAGE_SIZE), pfn);
		pfn++;
		ioremap_page++;
	}

	addr = ((char *) area->vaddr + (base & PAGE_MASK));
	return addr;
}