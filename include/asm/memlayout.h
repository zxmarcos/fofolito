/* 
 * FOFOLITO - Sistema Operacional para RaspberryPi
 * Função principal do kernel
 *
 * Marcos Medeiros


VMALLOC_START + VMALLOC_SIZE	+-------------------+
								| VMALLOC        	|
PAGE_OFFSET + HIGH_MEM			+-------------------+ VMALLOC_SIZE
								| KERNEL            |
PAGE_OFFSET (0xC000_0000)		+-------------------+ HIGH_MEM
								| ESPAÇO DO USUÁRIO |
								|                   |
								|                   |
								|                   |
								|                   |
								|                   |
								|                   |
								|                   |
0x0000_0000 					+-------------------+
 */

#ifndef __MEMLAYOUT_H__
#define __MEMLAYOUT_H__


/* RaspberryPi tem 256MB de memória */
#define LOW_MEM	(1024 * 1024 * 256)

#define PAGE_OFFSET		0xC0000000
#define TEXT_OFFSET		0x00020000

/* 128MB de alocação virtual */
#define VMALLOC_SIZE	(128 * 1024 * 1024)
#define VMALLOC_START	(PAGE_OFFSET + HIGH_MEM)
#define IOREMAP_START	(VMALLOC_START + VMALLOC_SIZE)
/* 128MB de remapeamento */
#define IOREMAP_SIZE	(128 * 1024 * 1024)

#endif
