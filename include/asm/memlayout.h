/* 
 * FOFOLITO - Sistema Operacional para RaspberryPi
 * Função principal do kernel
 *
 * Marcos Medeiros
 */
/*

TEXT_OFFSET:
	Endereço físico onde o kernel foi linkado. Isso é necessário
	para mapearmos o kernel no endereço virtual nas páginas corretas.

PAGE_OFFSET:
	Endereço virtual onde começa o kernel, geralmente em 0xC0000000.

STATIC_MEM:
	Quantidade de memória física que é mapeada diretamente
	dentro do espaço do kernel permanentemente.

VMALLOC_START:
	Endereço onde começara as alocações virtuais de memória,
	isso é, as páginas não precisam ser contínuas na memória
	possibilitando alocações maiores.

VMALLOC_SIZE:
	Tamanho da área de alocação virtual, 128MB.

IOREMAP_START:
	Endereço onde começara as remapeações de memória para
	dispositivos fazerem E/S. (MMIO)

IOREMAP_SIZE:
	Tamanho da área de remapeamento de E/S. 128MB

HIGH_VECTOR:
	Precisamos mapear os vetores na parte de cima do endereçamento,
	já que a primeira página é utilizada pra pegar referências nulas.


0xFFFF_FFFF     +-------------------+
                | ARM VECTOR TABLE  |
0xFFFF_F000     +-------------------+
                | MMIO              |
0xF000_0000     +-------------------+
                | IOREMAP           |
IOREMAP_START   +-------------------+
                | VMALLOC        	|
VMALLOC_START   +-------------------+
                | KERNEL            |
PAGE_OFFSET     +-------------------+
				| ESPAÇO DO USUÁRIO |
				|                   |
				|                   |
				|                   |
				|                   |
				|                   |
				|                   |
				|                   |
0x0000_0000 	+-------------------+
 */

#ifndef __MEMLAYOUT_H__
#define __MEMLAYOUT_H__


/* RaspberryPi tem 256MB de memória */
#define LOW_MEM	(1024 * 1024 * 256)

#define PAGE_OFFSET		0xC0000000
#define TEXT_OFFSET		0x00020000

/* 128MB de alocação virtual */
#define VMALLOC_SIZE	(128 * 1024 * 1024)
#define VMALLOC_START	(PAGE_OFFSET + STATIC_MEM)
#define IOREMAP_START	(VMALLOC_START + VMALLOC_SIZE)
/* 128MB de remapeamento */
#define IOREMAP_SIZE	(128 * 1024 * 1024)

#define MMIO_START	0xF0000000
/* Nós só podemos mapear até 0xFFFFF000 */
#define MMIO_SIZE	0x0FFFEFFF

#endif
