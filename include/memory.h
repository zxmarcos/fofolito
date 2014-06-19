#ifndef __MEMORY_H__
#define __MEMORY_H__

#include <types.h>

void memset(void *addr, uchar val, uint size);
void memcpy(void *dst, const void *src, uint size);
/* Versões otimizadas escritas em assembly */
void memset_s(void *addr, uchar val, uint size);
void memcpy_s(void *dst, const void *src, uint size);

#endif/*__MEMORY_H__*/