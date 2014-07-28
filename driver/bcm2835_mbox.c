/* 
 * FOFOLITO - Sistema Operacional para RaspberryPi
 * Funções responsáveis por fazer a comunicação com a Mailbox0 do RPi
 *
 * Marcos Medeiros
 */
#include <asm/io.h>
#include <asm/irq.h>
#include <kernel/printk.h>
#include <driver/bcm2835.h>
#include <errno.h>
#include <types.h>

#define MBOX_IOBASE	0x2000B880
#define MBOX_SIZE	0x24
#define MBOX_IRQ	ARM_IRQ(1)


/* Registradores da MAILBOX */
#define REG_READ	(0x00 >> 2)
#define REG_STATUS	(0x18 >> 2)
#define REG_CONFIG	(0x1C >> 2)
#define REG_WRITE	(0x20 >> 2)

/* Alguns bits do registrado de STATUS */
#define MBOX_FULL	0x80000000	
#define MBOX_EMPTY	0x40000000

static volatile unsigned *iobase = NULL;

static int bcm2835_mbox_handler()
{
	return -EOK;
}

int bcm2835_mbox_init()
{
	iobase = mmio_address(MBOX_IOBASE);
	irq_install_service(MBOX_IRQ, &bcm2835_mbox_handler);
	/* TODO: habilitar as irqs no registrador CONFIG */
	irq_enable_line(MBOX_IRQ);
	return -EOK;
}

/*
 * Lê uma mensagem da mailbox de um canal.
 */
uint bcm2835_mbox_read(uint channel)
{
	uint value = 0;
	/* Espera até a mailbox ter algo para nos dar */
	while (readl(iobase + REG_STATUS) & MBOX_EMPTY);

	/* Espera até a mensagem do nosso canal */
	do {
		value = readl(iobase + REG_READ);
	} while ((value & 0xF) != channel);

	/* Retorna somente os 28bits de dados, sem o canal */
	return value & ~0xF;
}

/*
 * Envia uma mensagem para um canal.
 */
void bcm2835_mbox_write(uint channel, unsigned data)
{
	/* Espera até a mailbox ter espaço vazio */
	while (readl(iobase + REG_STATUS) & MBOX_FULL);
	writel(iobase + REG_WRITE, (data & ~0xF) | (channel & 0xF));
}