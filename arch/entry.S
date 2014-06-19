/* 
 * FOFOLITO - Sistema Operacional para RaspberryPi
 * Aqui estão as funções que são pontos de entrada para o kernel
 * Marcos Medeiros
 */
#include <asm/asm.h>

.section .text

IMPORT(irq_handler)


.align 4
ENTRY(excpt_irq_caller)
	sub		lr, lr, #4
	stmfd	sp!, {lr}
	/* Vamos salvar todos os registradores na pilha */
	stmfd	sp!, {r0 - r12}
	bl		irq_handler
	ldmfd	sp!, {r0 - r12}
	/* o processador guarda o endereço de retorno + 4 em lr */
	ldmfd	sp!, {pc}^


/* Chamado quando uma instrução não reconhecida é encontrada */
.align 4
ENTRY(excpt_undefined_caller)
	mrc		p15, 0, r0, c6, c0, 0
	mrc		p15, 0, r1, c6, c0, 1
	mrc		p15, 0, r2, c6, c0, 2
	ldr		r3, =0xF0DE0000
	b		undefined_handler

.align 4
ENTRY(excpt_prefetch_abort_caller)
	mrc		p15, 0, r0, c6, c0, 0
	mrc		p15, 0, r1, c6, c0, 1
	mrc		p15, 0, r2, c6, c0, 2
	ldr		r3, =0xF0DE0001
	b		prefetch_abort_handler

.align 4
ENTRY(excpt_data_abort_caller)
	mrc		p15, 0, r0, c6, c0, 0
	mrc		p15, 0, r1, c6, c0, 1
	mrc		p15, 0, r2, c6, c0, 2
	ldr		r3, =0xF0DE0002
	b		abort_handler

.align 4
ENTRY(excpt_reserved_caller)
	b		reserved_handler

.align 4
ENTRY(excpt_fiq_caller)
	b		fiq_handler

ENTRY(excpt_swi_caller)
	b		swi_handler

