/* 
 * FOFOLITO - Sistema Operacional para RaspberryPi
 * Aqui estão as funções que são executadas atomicamente.
 *
 * Nos manuais da ARM é recomendado utilizar LDREX/STREX para operações
 * atômicas, no entanto, essas funções apenas funcionam em sistemas com
 * múltiplos processadores, e como nós rodamos em um sistema monoprocessador
 * precisamos encontrar uma outra solução, e a mais simples é executar essas
 * instruções com as interrupções desligadas.
 *
 * Marcos Medeiros
 */
#ifndef __ATOMIC_H__
#define __ATOMIC_H__

#include <asm/asm.h>

/* 
 * A ideia aqui (inspirada no código do FreeBSD) é: salvar o registrador
 * de status, desabilitar as interrupções, executar a operação e restaurar
 * o registrador de status, dessa maneira temos a certeza que a operação será
 * realizada de maneira atômica.
 */
#define without_interrupts(expr)	do {	\
	unsigned old_cpsr, new_cpsr;			\
	asm volatile (							\
		"mrs	%0, cpsr_all	\n"			\
		"orr	%1, %0, %2		\n"			\
		"msr	cpsr_all, %1	\n"			\
		:"=r" (old_cpsr), "=r" (new_cpsr)	\
		: "I" (CPSR_INT_DISABLE)			\
		: "cc");							\
	expr;									\
	asm volatile (							\
		"msr	cpsr_all, %0	\n"			\
		:: "r" (old_cpsr)					\
		: "cc");							\
} while (0)

static inline int atomic_exchange(int value, int *ptr) 
{
	int ret = 0;
	without_interrupts({
		ret = *ptr;
		*ptr = value;
	});
	return ret;
}

static inline void atomic_increment(int *ptr) 
{
	without_interrupts((*ptr)++);
}

static inline void atomic_decrement(int *ptr) 
{
	without_interrupts((*ptr)++);
}

#endif
