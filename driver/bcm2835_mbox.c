/* 
 * FOFOLITO - Sistema Operacional para RaspberryPi
 * Funções responsáveis por fazer a comunicação com a Mailbox0 do RPi
 *
 * Marcos Medeiros
 */
#include <types.h>
#include <asm/io.h>
#include <driver/bcm2835.h>
#include <errno.h>

#define MBOX_IOBASE	0x2000B880
#define MBOX_SIZE	0x24

/* A mailbox é mapeada nesse endereço de memória */
volatile unsigned *mbox_reg = NULL;

/* Registradores da MAILBOX */
#define REG_READ	(0x00 >> 2)
#define REG_STATUS	(0x18 >> 2)
#define REG_WRITE	(0x20 >> 2)

/* Alguns bits do registrado de STATUS */
#define MBOX_FULL	0x80000000	
#define MBOX_EMPTY	0x40000000

int bcm2835_mbox_init()
{
	mbox_reg = ioremap(MBOX_IOBASE, MBOX_SIZE);
	return -EOK;
}

/*
 * Lê uma mensagem da mailbox de um canal.
 */
uint bcm2835_mbox_read(uint channel)
{
	uint val = 0;
	/* Espera até a mailbox ter algo para nos dar */
	while (mbox_reg[REG_STATUS] & MBOX_EMPTY);

	/* Espera até a mensagem do nosso canal */
	do {
		val = mbox_reg[REG_READ];
	} while ((val & 0xF) != channel);

	/* Retorna somente os 28bits de dados, sem o canal */
	return val & ~0xF;
}

/*
 * Envia uma mensagem para um canal.
 */
void bcm2835_mbox_write(uint channel, uint data)
{
	/* Espera até a mailbox ter espaço vazio */
	while (mbox_reg[REG_STATUS] & MBOX_FULL);

	mbox_reg[REG_WRITE] = (data & ~0xF) | (channel & 0xF);
}