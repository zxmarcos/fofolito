/* 
 * FOFOLITO - Sistema Operacional para RaspberryPi
 *
 * Marcos Medeiros
 */
#ifndef __MM_H__
#define __MM_H__
#include <types.h>

extern const uint K_MAP_END;
uint page_alloc_init(uint total_memory, uint kstart, uint kend);
void *page_alloc();
void *page_alloc_n(uint npages);
void *page_alloc_align_n(uint npages, uint align);
void pages_info();

int kmalloc_init();
void *kmalloc(unsigned int size);
void kfree(void *ptr);

int paging_init();

#endif