/* 
 * FOFOLITO - Sistema Operacional para RaspberryPi
 * 
 * Marcos Medeiros
 */
#ifndef __IRQ_H__
#define __IRQ_H__

#include <types.h>
#include <asm/asm.h>

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

/* Salva o estado atual das IRQs e as desabilita */
static inline unsigned __irq_save_state()
{
	unsigned flags;
	asm volatile ("mrs %0, cpsr		\n"
				  "cpsid i			\n"
				  : "=r" (flags)
				  :: "memory", "cc");
	return flags & CPSR_IRQ_DISABLE;
}

#define irq_save_state(t) do { t = __irq_save_state(); } while (0)

static inline void irq_restore_state(unsigned flags)
{
	unsigned current_flags = 0;
	flags &= CPSR_IRQ_DISABLE;
	asm volatile ("mrs %0, cpsr		\n"
				  "bic %0, %2		\n"
				  "orr %0, %0, %1	\n"
				  "msr cpsr_c, %0	\n"
				  : "+r" (current_flags)
				  : "r" (flags), "i" (CPSR_IRQ_DISABLE)
				  : "memory", "cc");
}

#endif
