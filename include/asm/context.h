/* 
 * FOFOLITO - Sistema Operacional para RaspberryPi
 *
 * Contexto do processador de uma tarefa
 * Marcos Medeiros
 */
#ifndef __CONTEXT_H__
#define __CONTEXT_H__

#include <types.h>

/* Alterar essa estrutura inclusive a posição das variáveis, significa
 * alterar o código de entrada de irqs em entry.S
 * Seja cuidadoso.
 */
struct irq_context {
	uint lr;
	uint sp;
	uint cpsr;
	uint pc;
	uint r[13];
};

#endif