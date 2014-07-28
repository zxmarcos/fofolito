/* 
 * FOFOLITO - Sistema Operacional para RaspberryPi
 * Marcos Medeiros
 */
#ifndef __ASM_IO_H__
#define __ASM_IO_H__

#include <asm/barrier.h>
#include <asm/memlayout.h>

#define io_address(x)	((x & 0x0FFFFFFF) + MMIO_START)
#define mmio_address(x)	((void *) io_address(x))

int ioremap_init();
void *ioremap(unsigned long base, unsigned size);

static inline unsigned readl(const volatile void *addr) {
	unsigned data;
	data = *((volatile unsigned *) addr);
	rmb();
	return data;
}

static inline void writel(const volatile void *addr, const unsigned data) {
	*((volatile unsigned *) addr) = data;
	wmb();
}

#endif