
#ifndef __ASMIO_H__
#define __ASMIO_H__

#include <asm/barrier.h>

int ioremap_init();
void *ioremap(unsigned long base, unsigned size);


#endif