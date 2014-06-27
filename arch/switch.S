/* 
 * FOFOLITO - Sistema Operacional para RaspberryPi
 * Função responsável por fazer a troca de contexto
 * Marcos Medeiros
 */
#include <asm/asm.h>

.section .text

/*
 * void switch_to(struct thread_context *prev,
 *                struct thread_context *next)
 *
 */
.align 4
ENTRY(switch_to)
	stmia	r0, {r0 - r12, sp, lr}
	ldmia	r1, {r0 - r12, sp, pc}
