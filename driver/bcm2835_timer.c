/* 
 * FOFOLITO - Sistema Operacional para RaspberryPi
 * Driver de temporalizador
 * No manuais é dito que é um ap804 modificado.
 * Marcos Medeiros
 */
#include <asm/barrier.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/platform.h>
#include <kernel/printk.h>
#include <kernel/sched.h>
#include <driver/bcm2835.h>
#include <errno.h>
#include <types.h>

#define TIMER_IOBASE	0x2000B000
#define TIMER_SIZE		0x424
#define TIMER_IRQ		ARM_IRQ(0)
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

#define REG_CTRL_32BIT		(1 << 1)
#define REG_CTRL_PRESCL_16	(1 << 2)
#define REG_CTRL_PRESCL_256	(2 << 2)
#define REG_CTRL_INTEN		(1 << 5)
#define REG_CTRL_ENABLE		(1 << 7)
#define REG_CTRL_FRC_ENABLE	(1 << 9)

#define clear_irq()	do { timer[REG_IRQCLEAR] = 1; } while (0)

static int bcm2835_timer_handler()
{
	clear_irq();
	sched_tick();
	return -EOK;
}

int bcm2835_timer_init()
{
	timer = mmio_address(TIMER_IOBASE);

	irq_install_service(TIMER_IRQ, &bcm2835_timer_handler);

	/* Vamos desligar o timer antes de configurar */
	timer[REG_CTRL] = 0x00;
	clear_irq();
	
	/*
	 * Vamos calcular a frequência do timer, segundo a documentação
	 * temos a equação: timer_clock = apb_clock / (pre_div + 1),
	 * onde o apb_clock é o clock do sistema.
	 * O valor padrão desse registrador é 126.
	 *
	 * timer_clock = 250000000 / (126 + 7)
	 * timer_clock ~= 1,97MHz
	 *
	 * Mas vamos deixar nosso clock em 1MHz para facilitar os cálculos
	 * 1.000.000 = 250.000.000 / (pre_div + 1)
	 * 1.000.000 x (pre_div + 1) = 250.000.000
	 * pre_div + 1 = 250.000.000 / 1.000.000
	 * pre_div + 1= 250
	 * pre_div = 250 - 1
	 * pre_div = 249
	 */

	timer[REG_PREDIV] = 249;
	const unsigned long timer_clock = 1000000; /* 1MHz */
	const unsigned long MHz = 1000000;

	/*
     * Segundo a documentação do timer (sp804) da ARM, para
     * calcular o valor de intervalo temos:
     *
     * interval = ms x timer_clock (em MHz)
     *
     * Queremos 1 tick a cada 50ms, então...
	 */
	//timer[REG_RELOAD] = 50 * (timer_clock / MHz);
	timer[REG_LOAD] = 50 * (timer_clock / MHz);

	/* 
	 * Habilita o temporalizador com um contador de 23bits, interrupções habilitadas 
     * e o contador livre também é habilitado.
	 */
	timer[REG_CTRL] = REG_CTRL_32BIT | REG_CTRL_INTEN | REG_CTRL_ENABLE | REG_CTRL_FRC_ENABLE;

	do_dsb();
	
	irq_enable_line(ARM_IRQ(0));
	return -EOK;
}

uint bcm2835_timer_read()
{
	return timer[REG_FREECNT];
}
