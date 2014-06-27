/* 
 * FOFOLITO - Sistema Operacional para RaspberryPi
 * Driver de temporalizador
 * No manuais é dito que é um ap804 modificado.
 * Marcos Medeiros
 */
#include <types.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/barrier.h>
#include <driver/bcm2835.h>
#include <kernel/printk.h>
#include <kernel/sched.h>
#include <errno.h>

#define TIMER_IOBASE	0x2000B000
#define TIMER_SIZE		0x424
static volatile unsigned *timer = NULL;

#define REG_LOAD		(0x400 >> 2)
#define REG_VALUE		(0x404 >> 2)
#define REG_CTRL		(0x408 >> 2)
#define REG_IRQCLEAR	(0x40C >> 2)
#define REG_RAWIRQ		(0x410 >> 2)
#define REG_MASKIRQ		(0x414 >> 2)
#define REG_RELOAD		(0x418 >> 2)
#define REG_PREDIV		(0x41C >> 2)
#define REG_FREECNT		(0x420 >> 2)

#define REG_CTRL_23BIT		(1 << 1)
#define REG_CTRL_PRESCL_16	(1 << 2)
#define REG_CTRL_PRESCL_256	(2 << 2)
#define REG_CTRL_INTEN		(1 << 5)
#define REG_CTRL_ENABLE		(1 << 7)
#define REG_CTRL_FRC_ENABLE	(1 << 9)

#define clear_irq()	do { timer[REG_IRQCLEAR] = 1; } while (0)

static int bcm2835_timer_handler()
{
	clear_irq();
	schedule();
	return -EOK;
}

int bcm2835_timer_init()
{
	timer = ioremap(TIMER_IOBASE, TIMER_SIZE);

	irq_install_service(ARM_IRQ(0), &bcm2835_timer_handler);

	/* 
	 * Habilita o temporalizador com um contador de 23bits, interrupções habilitadas 
     * e o contador livre também é habilitado.
	 */
	timer[REG_CTRL] = REG_CTRL_23BIT | REG_CTRL_INTEN | REG_CTRL_ENABLE | REG_CTRL_FRC_ENABLE;
	timer[REG_RELOAD] = 0xF0000;
	irq_enable_line(ARM_IRQ(0));
	return -EOK;
}

uint bcm2835_timer_read()
{
	return timer[REG_FREECNT];
}