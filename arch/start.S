/* 
 * FOFOLITO - Sistema Operacional para RaspberryPi
 * Funções responsáveis por fazer a comunicação com a Mailbox0 do RPi
 *
 * Marcos Medeiros
 */
#include <asm/asm.h>

IMPORT(excpt_undefined_caller)
IMPORT(excpt_swi_caller)
IMPORT(excpt_prefetch_abort_caller)
IMPORT(excpt_data_abort_caller)
IMPORT(excpt_reserved_caller)
IMPORT(excpt_irq_caller)
IMPORT(excpt_fiq_caller)

/*
 * Vetor de interrupções
 */
.global interrupt_vector_table
interrupt_vector_table:
	ldr pc, _excpt_reset
	ldr	pc, _excpt_undefined
	ldr pc, _excpt_swi
	ldr	pc, _excpt_prefetch_abort
	ldr	pc, _excpt_data_abort
	ldr pc, _excpt_reserved
	ldr	pc, _excpt_irq
	ldr	pc, _excpt_fiq

/* Como essas funções estão em outros arquivos, é necessário importar */
_excpt_reset:			.word excpt_reset
_excpt_undefined:		.word excpt_undefined_caller
_excpt_swi:				.word excpt_swi_caller
_excpt_prefetch_abort:	.word excpt_prefetch_abort_caller
_excpt_data_abort:		.word excpt_data_abort_caller
_excpt_reserved:		.word excpt_reserved_caller
_excpt_irq:				.word excpt_irq_caller
_excpt_fiq:				.word excpt_fiq_caller

interrupt_vector_table_end:


/*
 * Aqui começa o nosso kernel, essa função é chamada pelo bootloader quando
 * o mesmo dispara uma interrupção reset
 */
.global boot_start
boot_start:
	/* Apenas pra ficar mais fácil de encontrar no dissambler :) */
	nop
	nop
	nop
	nop
	
	ldr		sp, =k_stack_svc
	/* Define a stack para o modo IRQ */
	mrs		r3, cpsr
	bic		r4, r3, #CPSR_MODE_MASK
	orr		r4, #CPSR_IRQ_MODE
	msr		cpsr_c, r4
	ldr		sp, =k_stack_irq

	/* Define a stack para o modo ABT */
	bic		r4, r3, #CPSR_MODE_MASK
	orr		r4, #CPSR_ABT_MODE
	msr		cpsr_c, r4
	ldr		sp, =k_stack_abt

	/* Define a stack para o modo UND */
	bic		r4, r3, #CPSR_MODE_MASK
	orr		r4, #CPSR_UND_MODE
	msr		cpsr_c, r4
	ldr		sp, =k_stack_und

	/* Volta ao modo SVC */
	msr		cpsr_c, r3

	/* Quando estamos rodando no emulador, é necessário copiar
	 * a tabela de interrupções para 0x00000000.
	 */
#ifdef __EMUVERSION__
	bl		copy_ivt
#endif
	b		kmain

.align 4
ENTRY(simple_delay)
	cmp		r0, #0
	movle	pc, lr
1:
	sub		r0, #1
	cmp		r0, #0
	bgt		1b
	mov		pc, lr

/*
 * Copia a tabela de interrupções para 0x00000000
 */
.align 4
copy_ivt:
	ldr		r0, =interrupt_vector_table
	ldr 	r1, =interrupt_vector_table_end
	mov 	r3, #0
1:
	ldr		r2, [r0]
	str		r2, [r3]
	cmp		r0, r1
	add		r0, r0, #4
	add		r3, r3, #4
	blo		1b
	mov		pc, lr


.align 4
excpt_reset:
	b	boot_start


ENTRY(K_MAP_END)
	.word k_reloc_end

