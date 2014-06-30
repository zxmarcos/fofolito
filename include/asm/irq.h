/* 
 * FOFOLITO - Sistema Operacional para RaspberryPi
 * 
 * Marcos Medeiros
 */
#ifndef __IRQ_H__
#define __IRQ_H__

#include <types.h>

#define GPU_IRQ_FLAG	0x80000000
#define ARM_IRQ(n)	(n & ~GPU_IRQ_FLAG)
#define GPU_IRQ(n)	(GPU_IRQ_FLAG | n)

typedef int (*irq_service_t)(void);

void irq_enable();
void irq_disable();
void irq_init();
void irq_enable_line(uint irq);
void irq_disable_line(uint irq);
int irq_install_service(uint irq, irq_service_t service);

#endif
