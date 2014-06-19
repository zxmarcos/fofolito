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
#include <asm/offset.h>

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
	page_alloc_init(HIGH_MEM, 0, (unsigned) &k_reloc_end);
	kmalloc_init();
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
	

	kmalloc(128);
#if 0
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