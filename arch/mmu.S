/* 
 * FOFOLITO - Sistema Operacional para RaspberryPi
 * 
 * Marcos Medeiros
 */
#include <asm/asm.h>

.section .text

ENTRY(mmu_get_cr)
	mrc		p15, 0, r0, c1, c0, 0
	mov		pc, lr

ENTRY(mmu_get_ttbr)
	mrc		p15, 0, r0, c2, c0, 0
	mov		pc, lr

ENTRY(mmu_get_ttbcr)
	mrc		p15, 0, r0, c2, c0, 2
	mov		pc, lr

ENTRY(mmu_get_dom)
	mrc		p15, 0, r0, c3, c0, 0
	mov		pc, lr

ENTRY(mmu_set_ttbr)
	mcr		p15, 0, r0, c2, c0, 0
	mov		r0, #0
	mcr		p15, 0, r0, c2, c0, 1
	/* Inválida o tlb imediatamente */
	mcr		p15, 0, r0, c8, c7, 0
	mov		pc, lr

ENTRY(mmu_set_ttbcr)
	mcr		p15, 0, r0, c2, c0, 2
	mov		pc, lr

ENTRY(mmu_set_dom)
	mcr		p15, 0, r0, c3, c0, 0
	mov		pc, lr

ENTRY(mmu_set_cr)
	mcr		p15, 0, r0, c1, c0, 0
	mov		pc, lr


/* Invalida o TLB inteiro */
ENTRY(mmu_flush_tlb)
	mov		r0, #0
	mcr		p15, 0, r0, c8, c7, 0
	mov		pc, lr

/* Invalida o cache de instruções inteiro */
ENTRY(mmu_flush_icache)
	mov		r0, #0
	mcr		p15, 0, r0, c8, c5, 0
	mov		pc, lr

/* Invalida o cache de dados inteiro */
ENTRY(mmu_flush_dcache)
	mov		r0, #0
	mcr		p15, 0, r0, c8, c6, 0
	mov		pc, lr

ENTRY(mmu_invpg_entry)
	ldr		r1, =0xFFF
	and		r0, r0, r1
	mcr		p15, 0, r0, c8, c7, 1
	mov		pc, lr

ENTRY(mmu_invpg_icache)
	ldr		r1, =0xFFF
	and		r0, r0, r1
	mcr		p15, 0, r0, c8, c5, 1
	mov		pc, lr

ENTRY(mmu_invpg_dcache)
	ldr		r1, =0xFFF
	and		r0, r0, r1
	mcr		p15, 0, r0, c8, c6, 1
	mov		pc, lr

ENTRY(mmu_set_vector_base)
	mcr		p15, 0, r0, c12, c0, 0
	mov		pc, lr