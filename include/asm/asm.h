/* 
 * FOFOLITO - Sistema Operacional para RaspberryPi
 * Essas definições são incluidas por arquivos escritos em assembly
 *
 * Marcos Medeiros
 */
#ifndef __ASM_H
#define __ASM_H

#define PAGE_OFFSET		0xC0000000
#define TEXT_OFFSET		0x00020000

/* Domínios */
#define DOMAIN_KERNEL	0
#define DOMAIN_USER		1
#define DOMAIN_MMIO		2
#define DOMAIN_AC_NO		0
#define DOMAIN_AC_CLIENT	1
#define DOMAIN_AC_RESERVED	2
#define DOMAIN_AC_MANAGER	3

/* Proteção */
#define PROT_KERNEL		3
#define PROT_USER_RO	2
#define PROT_USER_RW	3


/* RaspberryPi tem 256MB de memória */
#define STATIC_MEM	(1024 * 1024 * 256)

#ifdef __ASSEMBLER__

#define MMU_ENABLE			(1 << 0)
#define MMU_ALIGN			(1 << 1)
#define MMU_WRBUF			(1 << 3)
#define MMU_XP				(1 << 23)
#define MMU_L1CACHE

#define MK_DOMAIN(n, t)	((t & 3) << (n << 2))

#define PGT_SHIFT		20
#define MK_SEC(b,c,xn,domain,ap,tex,apx,s,ng)	\
 	(((b & 1) << 2)			|	\
 	((c & 1) << 3)			|	\
 	((xn & 1) << 4)			|	\
 	((domain & 0xF) << 5)	|	\
 	((ap & 3) << 10)		|	\
 	((tex & 7) << 12)		|	\
 	((apx & 1) << 15)		|	\
 	((s & 1) << 16)			|	\
 	((ng & 1) << 17) | 0x02)

#define K_PGT_SECTION	MK_SEC(1,1,0,DOMAIN_KERNEL,PROT_KERNEL,0,0,0,0)

#define ENTRY(name)	.text; .global name; name:
#define IMPORT(name) .extern name

/* Converte um endereço virtual para físico */
#define PA(addr)	(addr - PAGE_OFFSET)

#endif/*__ASSEMBLER__*/

#define	CPSR_MODE_MASK	0x1f
#define CPSR_USR_MODE	0x10
#define CPSR_FIQ_MODE	0x11
#define CPSR_IRQ_MODE	0x12
#define CPSR_SVC_MODE	0x13
#define CPSR_ABT_MODE	0x17
#define CPSR_UND_MODE	0x1b
#define CPSR_SYS_MODE	0x1f

#define CPSR_IRQ_DISABLE	0x80
#define CPSR_FIQ_DISABLE	0x40
#define CPSR_INT_DISABLE	(CPSR_FIQ_DISABLE | CPSR_IRQ_DISABLE)

#ifndef __ASSEMBLER__
/* Macros úteis para converter endereços físicos e virtuais */
#define __virt_to_phys(x)	((unsigned long)(x) - PAGE_OFFSET)
#define __phys_to_virt(x)	((void *) ((unsigned long)(x) + PAGE_OFFSET))
#define __va(x)	(__phys_to_virt(x))
#define __pa(x)	(__virt_to_phys(x))
#endif


#endif