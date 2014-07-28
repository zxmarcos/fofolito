/* 
 * FOFOLITO - Sistema Operacional para RaspberryPi
 * Funções relacionadas ao GPIO
 *
 * Marcos Medeiros
 */
#include <asm/io.h>
#include <driver/bcm2835.h>
#include <types.h>

#define GPIO_IOBASE	0x20200000
#define GPIO_SIZE	0x48
static volatile unsigned *iobase = NULL;

/* Como nosso acesso é de 32bits, divimos os endereços por 4bytes */
#define REG_FUNCSEL_0	(0x00 >> 2)
#define REG_FUNCSEL_1	(0x04 >> 2)
#define REG_FUNCSEL_2	(0x08 >> 2)
#define REG_FUNCSEL_3	(0x0C >> 2)
#define REG_FUNCSEL_4	(0x10 >> 2)
#define REG_FUNCSEL_5	(0x14 >> 2)
#define REG_OUTSET_0	(0x1C >> 2)
#define REG_OUTSET_1	(0x20 >> 2)
#define REG_OUTCLEAR_0	(0x28 >> 2)
#define REG_OUTCLEAR_1	(0x2C >> 2)
#define REG_EVENTSTAT_0	(0x40 >> 2)
#define REG_EVENTSTAT_1	(0x44 >> 2)

#define RPI_LED_OK		16

void bcm2835_gpio_init()
{
	iobase = mmio_address(GPIO_IOBASE);
	/* Configura o pino do LED como saída */
	bcm2835_gpio_setfunction(RPI_LED_OK, GPIO_FUNC_OUTPUT);
}

void bcm2835_gpio_setfunction(uint pin, uint function)
{
	if (pin >= RPI_GPIO_MAX)
		return;

	uint bank = pin / 10;
	uint pos = (pin % 10) * 3;
	uint addr = REG_FUNCSEL_0 + bank;

	uint data = readl(iobase + addr);
	data &= ~(7 << pos);
	data |= (function & 7) << pos;
	writel(iobase + addr, data);
}

void bcm2835_gpio_output_clearset(uint pin, uint clear)
{
	if (pin >= RPI_GPIO_MAX)
		return;

	uint bank = (pin / 32);
	uint pos = (pin % 32);
	uint addr = bank;

	addr += (clear) ?  REG_OUTCLEAR_0 : REG_OUTSET_0;
	writel(iobase + addr, 1 << pos);
	addr = bank + REG_EVENTSTAT_0;
	writel(iobase + addr, 1 << pos);
}

void bcm2835_turn_led(int state)
{
	bcm2835_gpio_output_clearset(RPI_LED_OK, !state);
}