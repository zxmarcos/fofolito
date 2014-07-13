/*
 * FOFOLITO - Sistema Operacional para RaspberryPi
 *
 * Esse módulo é encarregado de colocar o kernel executando em 0xC0000000
 * O kernel para ser mapeado diretamente é conveniente utilizar o tipo de
 * página chamado de section, que mapeia 1MB completo, sem necessidade de
 * um segundo descritor.
 *
 * Marcos Medeiros
 */
#include <asm/asm.h>

.section .loader

.global pre_init
pre_init:
	/* Para verificarmos se o boot está acontecendo, vamos acender o LEDOK */
	ldr		r0, =0x20200000
	mov		r1, #1
	lsl		r1, #18
	str		r1, [r0, #4]
	mov		r1, #1
	lsl		r1, #16
	str		r1, [r0, #40]

	/* Primeiro, desabilitamos todas as interrupções */
	mrs		r0, cpsr
	orr		r0, #(CPSR_IRQ_DISABLE | CPSR_FIQ_DISABLE)
	msr		cpsr, r0

	/* Configuramos uma stack temporária */
	ldr		sp, =k_tmp_stack	

	/* 
	 * Primeiro vamos limpar o diretório de páginas, temos 4096 entradas
	 * de 4 bytes cada uma.
	 */
	/* Zeramos os registradores */
	mov		r2, #0	
	mov		r3, #0	
	mov		r4, #0	
	mov		r5, #0	
	mov		r6, #0	
	mov		r7, #0	
	mov		r8, #0	
	mov		r9, #0

	ldr		r0, =PA(k_init_pgdir)
	mov		r10, r0
	/* Limpa 64 entradas por loop */
	ldr		r1, =(4096 / (8 * 8))
clear_pgdir$:
	stmia	r10!, {r2-r9}
	stmia	r10!, {r2-r9}
	stmia	r10!, {r2-r9}
	stmia	r10!, {r2-r9}

	stmia	r10!, {r2-r9}
	stmia	r10!, {r2-r9}
	stmia	r10!, {r2-r9}
	stmia	r10!, {r2-r9}

	subs	r1, r1, #1
	bne		clear_pgdir$

	/* Copia o kernel para a TEXT_OFFSET (0x20000) */
	ldr		r0, =TEXT_OFFSET
	ldr		r1, =PA(k_reloc_start)
	/* Só vamos copiar se o kernel não estiver no local correto */
	cmp		r0, r1
	ldrne	r2, =PA(k_reloc_end)
	subne	r2, r2, r1
	blne	early_memcpy

	/* Mapeia as páginas do kernel
	 * r0 = pgdir
	 * r1 = entrada na pgdir para o PAGE_OFFSET
	 * r2 = numeros de entradas para mapear,
	 *      Cada entrada mapeia 1MB de memória.
	 * r3 = enderço física da seção a mapear (1MB cada)
	 * r4 = atributos da entrada do kernel do pgdir
	 * r7 = tamanho de uma seção (1MB)
	 */
	ldr		r0, =PA(k_init_pgdir)
	ldr		r1, =(PAGE_OFFSET >> PGT_SHIFT)
	ldr		r2, =(0x40000000 >> PGT_SHIFT)
	mov		r3, #0
	ldr		r4, =K_PGT_SECTION
	ldr		r7, =(1024 * 1024)

	/* O primeiro MB é mapeado em identidade */
	str		r4, [r0]

	/* Agora mapeamos de PAGE_OFFSET :: (PAGE_OFFSET + STATIC_MEM) */
	mov		r6, #0
	/* Coloca em r0 o ponteiro para a primeira entrada de PAGE_OFFSET */
	add		r0, r0, r1, lsl #2
map_next$:
	/* Atributos + Endereço físico */
	add		r5, r4, r3
	str		r5, [r0], #4
	/* Incrementa o endereço físico */
	add		r3, r3, r7
	/* Incrementa o número de páginas mapeadas */
	add		r6, r6, #1
	cmp		r6, r2
	blo		map_next$

	/* Antes de habilitar a MMU, vamos configurar o domínio */
	mvn		r0, #0
	mcr		p15, 0, r0, c3, c0, 0
	/* Vamos setar o endereço da ttbr */
	ldr		r0, =PA(k_init_pgdir)
	mcr		p15, 0, r0, c2, c0, 0
	/* Vamos dizer a MMU, para usar apenas o TTBR0 */
	mov		r0, #0
	mcr		p15, 0, r0, c2, c0, 2

	ldr		r10, =boot_start
	/* Vamos habilitar a MMU */
	ldr		r0, =(MMU_ENABLE | MMU_XP)
	mcr		p15, 0, r0, c7, c5, 4 /* sincroniza */
	mcr		p15, 0, r0, c1, c0, 0
	mcr		p15, 0, r0, c7, c5, 4 /* sincroniza */
	mov		r0, r0
	mov		r0, r10

	/* Agora estamos prontos para ir ao HigherHalf \o\ */
	mov		pc, r0
/*
 * Função copiada e minimizada de lib/memory_s.S 
 * Só copia blocos alinhados em 4bytes
 */
early_memcpy:
	stmfd	sp!, {r4 - r12}	
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
	ldmfd	sp!, {r4 - r12}
	mov		pc, lr


/* Criamos uma stack temporária, no máximo 64 entradas */
.align 4; .rept 64; .word 0; .endr; k_tmp_stack:
.ascii "BootLoader HigherHalf"

