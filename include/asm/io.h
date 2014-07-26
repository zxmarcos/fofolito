
#ifndef __ASMIO_H__
#define __ASMIO_H__

#include <asm/barrier.h>
#include <asm/memlayout.h>

#define io_address(x)	((x & 0x0FFFFFFF) + MMIO_START)
#define mmio_address(x)	((void *) io_address(x))

int ioremap_init();
void *ioremap(unsigned long base, unsigned size);


#endif