/* 
 * FOFOLITO - Sistema Operacional para RaspberryPi
 *
 * Marcos Medeiros
 */
#include <types.h>
#include <errno.h>
#include <kernel/printk.h>

void prefetch_abort_handler()
{
	printk("pagefault: PABT_Handler");
	for (;;);
}

void abort_handler()
{
	printk("pagefault: ABT_Handler");
	for (;;);
}

void undefined_handler()
{
	printk("UND_Handler");
	for (;;);
}

void reserved_handler()
{
	printk("RES_Handler");
	for (;;);
}

void fiq_handler()
{
	printk("FIQ_Handler");
	for (;;);
}

void swi_handler()
{
	printk("SWI_Handler");
	for (;;);
}