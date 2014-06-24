/* 
 * FOFOLITO - Sistema Operacional para RaspberryPi
 * Função principal do kernel
 *
 * Marcos Medeiros
 */

#include <kernel/mm.h>
#include <kernel/fb.h>
#include <kernel/printk.h>
#include <driver/fb_console.h>
#include <driver/bcm2835.h>
#include <asm/irq.h>
#include <asm/page.h>
#include <asm/asm.h>
#include <asm/mmu.h>
#include <asm/offset.h>
#include <memory.h>

extern void simple_delay(int ticks);

#define DELAY	0x3f0000

/* Configura os drivers da plataforma */
void platform_setup()
{
	bcm2835_mbox_init();
	bcm2835_gpio_init();
	bcm2835_fb_init();
	bcm2835_timer_init();
}

void mm_init()
{
	page_alloc_init(STATIC_MEM, 0, __pa(k_reloc_end));
	kmalloc_init();
	paging_init();
}

static inline void led_blink()
{
	bcm2835_turn_led(1);
	simple_delay(DELAY);
	bcm2835_turn_led(0);
	simple_delay(DELAY);
}

void kernel_info()
{
	printk("FOFOLITO - Sistema Operacional para RaspberryPi\n");
	printk("Desenvolvido por Marcos Medeiros\n");
	pages_info();
}


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


void kmain()
{
	/* 
	 * A primeira coisa a se fazer é iniciar todo o gerenciador
	 * de memória.
	 */
	mm_init();
	high_vectors_setup();

#if 0

	kmalloc(128);
	/* Agora configuramos as IRQs */
	irq_init();

	platform_setup();

	/* Requisita um modo se existir um framebuffer*/
	fb_set_mode();
	/* Inicia o console sobre o framebuffer */
	fb_console_init();

	kernel_info();
	
	/* Agora habilitamos as interrupções */
	irq_enable();

	for (;;) {
		led_blink();
	}
#else
	for (;;);
#endif
}