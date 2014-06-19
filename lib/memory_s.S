/* 
 * FOFOLITO - Sistema Operacional para RaspberryPi
 * Funções otimizadas para manipular memória.
 * As funções aqui tiram proveito da capacidade de carregar
 * e gravar dados simultaneamente usando ldm e stm.
 * Dividindo os blocos em 256,128,64,32,16,8,4,2 e 1 bytes.
 * E sempre evitando os branchs onde possível, que afetam
 * também a performance.
 *
 * Marcos Medeiros
 */
/*
 * void memcpy(char *dst, const char *src, size_t size)
 *
 * LAYOUT dos registradores
 *
 * r0 - dst
 * r1 - src
 * r2 - size
 * r3:r12 - scratch
 *
 */
#include <asm/asm.h>

ENTRY(memcpy_s)
	stmfd	sp!, {r4 - r12}
	/**************************************************/
	/* Copia blocos maiores                           */
	/**************************************************/
	cmp		r2, #256
x256_copy$:
	ldmcsia	r1!, {r3-r10}
	stmcsia	r0!, {r3-r10}
	ldmcsia	r1!, {r3-r10}
	stmcsia	r0!, {r3-r10}
	ldmcsia	r1!, {r3-r10}
	stmcsia	r0!, {r3-r10}
	ldmcsia	r1!, {r3-r10}
	stmcsia	r0!, {r3-r10}
	ldmcsia	r1!, {r3-r10}
	stmcsia	r0!, {r3-r10}
	ldmcsia	r1!, {r3-r10}
	stmcsia	r0!, {r3-r10}
	ldmcsia	r1!, {r3-r10}
	stmcsia	r0!, {r3-r10}
	ldmcsia	r1!, {r3-r10}
	stmcsia	r0!, {r3-r10}
	
	subcs	r2, r2, #256
	cmp		r2, #256
	bcs		x256_copy$
	

	cmp		r2, #128
x128_copy$:
	ldmcsia	r1!, {r3-r10}
	stmcsia	r0!, {r3-r10}
	ldmcsia	r1!, {r3-r10}
	stmcsia	r0!, {r3-r10}
	ldmcsia	r1!, {r3-r10}
	stmcsia	r0!, {r3-r10}
	ldmcsia	r1!, {r3-r10}
	stmcsia	r0!, {r3-r10}
	subcs	r2, r2, #128
	cmp		r2, #128
	bcs		x128_copy$

	cmp		r2, #64
x64_copy$:
	ldmcsia	r1!, {r3-r10}
	stmcsia	r0!, {r3-r10}
	ldmcsia	r1!, {r3-r10}
	stmcsia	r0!, {r3-r10}
	subcs	r2, r2, #64
	cmp		r2, #64
	bcs		x64_copy$

	cmp		r2, #32
x32_copy$:
	ldmcsia	r1!, {r3-r10}
	stmcsia	r0!, {r3-r10}
	subcs	r2, r2, #32
	cmp		r2, #32
	bcs		x32_copy$

	/**************************************************/
	/* Copia blocos menores                           */
	/**************************************************/
	cmp		r2, #16
x16_copy$:
	ldmcsia	r1!, {r3-r6}
	stmcsia	r0!, {r3-r6}
	subcs	r2, r2, #16
	cmp		r2, #16
	bcs		x16_copy$

	cmp		r2, #8
x8_copy$:
	ldmcsia	r1!, {r3-r4}
	stmcsia	r0!, {r3-r4}
	subcs	r2, r2, #8
	cmp		r2, #8
	bcs		x8_copy$

	cmp		r2, #4
x4_copy$:
	ldrcs	r3, [r1], #4
	strcs	r3, [r0], #4
	subcs	r2, r2, #4
	cmp		r2, #4
	bcs		x4_copy$

	cmp		r2, #2
x2_copy$:
	ldrcsh	r3, [r1], #2
	strcsh	r3, [r0], #2
	subcs	r2, r2, #2
	cmp		r2, #2
	bcs		x2_copy$

	cmp		r2, #0
x1_copy$:
	ldrneb	r3, [r1], #1
	strneb	r3, [r0], #1
	subnes	r2, r2, #1
	bne		x1_copy$

	ldmfd	sp!, {r4 - r12}
	mov		pc, lr

/*
 * void memset(char *dst, uint value, size_t size)
 *
 * LAYOUT dos registradores
 *
 * r0 - dst
 * r1 - value
 * r2 - size
 * r3:r12 - scratch
 */
ENTRY(memset_s)
	stmfd	sp!, {r4 - r12}

	/* Aqui vamos deixar o valor em dword:
	 * valor original = 0x000000AB
	 * valor modificado = 0xABABABAB
	 */
	and		r1, r1, #255
	orr		r1, r1, r1, lsl #8
	orr		r1, r1, r1, lsl #16
	orr		r1, r1, r1, lsl #24

	mov		r3, r1
	mov		r4, r1
	mov		r5, r1
	mov		r6, r1
	mov		r7, r1
	mov		r8, r1
	mov		r9, r1
	mov		r10, r1

	cmp		r2, #256
x256_set$:
	stmcsia	r0!, {r3-r10}
	stmcsia	r0!, {r3-r10}
	stmcsia	r0!, {r3-r10}
	stmcsia	r0!, {r3-r10}
	stmcsia	r0!, {r3-r10}
	stmcsia	r0!, {r3-r10}
	stmcsia	r0!, {r3-r10}
	stmcsia	r0!, {r3-r10}
	
	subcs	r2, r2, #256
	cmp		r2, #256
	bcs		x256_set$
	

	cmp		r2, #128
x128_set$:
	stmcsia	r0!, {r3-r10}
	stmcsia	r0!, {r3-r10}
	stmcsia	r0!, {r3-r10}
	stmcsia	r0!, {r3-r10}
	subcs	r2, r2, #128
	cmp		r2, #128
	bcs		x128_set$

	cmp		r2, #64
x64_set$:
	stmcsia	r0!, {r3-r10}
	stmcsia	r0!, {r3-r10}
	subcs	r2, r2, #64
	cmp		r2, #64
	bcs		x64_set$

	cmp		r2, #32
x32_set$:
	stmcsia	r0!, {r3-r10}
	subcs	r2, r2, #32
	cmp		r2, #32
	bcs		x32_set$

	cmp		r2, #16
x16_set$:
	stmcsia	r0!, {r3-r6}
	subcs	r2, r2, #16
	cmp		r2, #16
	bcs		x16_set$

	cmp		r2, #8
x8_set$:
	stmcsia	r0!, {r3-r4}
	subcs	r2, r2, #8
	cmp		r2, #8
	bcs		x8_set$

	cmp		r2, #4
x4_set$:
	strcs	r3, [r0], #4
	subcs	r2, r2, #4
	cmp		r2, #4
	bcs		x4_set$

	cmp		r2, #2
x2_set$:
	strcsh	r3, [r0], #2
	subcs	r2, r2, #2
	cmp		r2, #2
	bcs		x2_set$

	cmp		r2, #0
x1_set$:
	strneb	r3, [r0], #1
	subnes	r2, r2, #1
	bne		x1_set$

	ldmfd	sp!, {r4 - r12}
	mov		pc, lr
