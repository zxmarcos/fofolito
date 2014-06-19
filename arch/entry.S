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


/*
 * Como é através de uma irq ou uma syscall que é possível trocar processos,
 * então é necessário salvar todos os dados originais onde ocorreu a IRQ.
 * Nosso irqframe tem essa estrutura.
 * struct irq_context {
 *	  uint lr;
 *	  uint sp;
 *    uint cpsr;
 *	  uint pc;
 *	  uint r[12];
 * };
 *
 */
ENTRY(irq_entry)
	sub		lr, lr, #4

    /**************************************************************************/
    /* push irq_context START                                                 */
    /**************************************************************************/
	/* vamos subir na pilha 12 uints para podermos utilizar o modo
	 * de transferência multipla stm.
	 */
	sub		sp, sp, #(12 * 4)
	/* salva os registradores de r0 a r12 no frame */
	stmia	sp, {r0 - r12}
	/* salva o pc no frame */
	stmfd	sp!, {lr}		
	/* salva o cpsr no frame */
	mrs		r0, spsr
	stmfd	sp!, {r0}
	/* Agora precisamos voltar ao modo anterior e pegar os
	 * registradores r13 e r14 que são exclusivos de cada modo
	 */
	mrs		r1, cpsr
	mvn		r2, #CPSR_MODE_MASK
	/* Limpamos os bits de modo do nosso cpsr atual */
	and		r2, r1, r2
	/* Deixamos só os bits de modo no spsr do modo antigo */
	and		r3, r0, #CPSR_MODE_MASK
	/**************************************************************************/
	/* Forma nosso novo cpsr                                                  */ 
	/* É importante notar que se o último modo é USR, então                   */
	/* não podemos voltar para ele pois ele não tem mais privilégios          */
	/* o que fazemos é mudar para o modo SYS.                                 */
	/**************************************************************************/
	cmp		r3, #CPSR_USR_MODE
	orreq	r2, r2, #CPSR_SYS_MODE
	orrne	r2, r2, r3
	msr		cpsr, r2
	/* Salva nossos registradores guardados nesse modo */
	mov		r4, sp
	mov		r5, lr

	/* Volta ao modo irq */
	msr		cpsr, r1
	stmfd	sp!, {r4, r5}

/****************************************************************************/
/* push irq_context END                                                     */
/****************************************************************************/
	mov		r0, sp
	bl		irq_handler

/****************************************************************************/
/* pop irq_context START                                                    */
/****************************************************************************/

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

