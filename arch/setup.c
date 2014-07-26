/*
 * FOFOLITO - Sistema Operacional para RaspberryPi
 * Remapeador de MMIO
 * Marcos Medeiros
 */

#include <asm/page.h>
#include <asm/memlayout.h>
#include <asm/platform.h>
#include <asm/mmu.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <kernel/mm.h>
#include <kernel/printk.h>
#include <driver/bcm2835.h>
#include <errno.h>
#include <memory.h>

/*
 * Mapeia a tabela de exceções na última página
 */
void high_vectors_setup()
{
	char *vectors = page_alloc();
	extern char interrupt_vector_table;
	extern char interrupt_vector_table_end;
	unsigned size = (unsigned)(&interrupt_vector_table_end - &interrupt_vector_table);

	memset(vectors, 0, PAGE_SIZE);
	memcpy_s(vectors, &interrupt_vector_table, size);

	char *hivec = (char *) 0xFFFFF000;
	page_map(&k_pgdir, hivec, PFN_FROM_VA(vectors));
	mmu_set_vector_base(hivec);
}

void arch_early_init()
{
	unsigned pfn = 0;
	char *vaddr, *endaddr;
	high_vectors_setup();
	
	/* Mapeia os locais de MMIO */
	vaddr = (char *) MMIO_START;
	endaddr = (char *) (MMIO_START + MMIO_SIZE);

	pfn = (0x20000000 >> PAGE_SHIFT);
	while (vaddr <= endaddr) {
		page_map(&k_pgdir, vaddr, pfn);
		vaddr += PAGE_SIZE;
		pfn++;
	}
}

/* Configura os drivers da plataforma */
void arch_setup()
{
	bcm2835_mbox_init();
	bcm2835_fb_init();
	bcm2835_gpio_init();
}

/* Configura o temporalizador */
int timer_init()
{
	return bcm2835_timer_init();
}