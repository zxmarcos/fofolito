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
#include <asm/page.h>
#include <asm/asm.h>
#include <asm/irq.h>
#include <asm/offset.h>
#include <asm/platform.h>
#include <memory.h>

extern void simple_delay(int ticks);

#define DELAY	0x3f0000

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


void kmain()
{
	/* 
	 * A primeira coisa a se fazer é iniciar todo o gerenciador
	 * de memória.
	 */
	mm_init();
	
	platform_setup();



	/* Requisita um modo se existir um framebuffer*/
	fb_set_mode();
	/* Inicia o console sobre o framebuffer */
	fb_console_init();
	kernel_info();
	
	/* Agora habilitamos as interrupções */
	irq_enable();

	for (;;);
}