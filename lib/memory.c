/* 
 * FOFOLITO - Sistema Operacional para RaspberryPi
 *
 * Funções altamente genéricas (e lentas) para manipular memória.
 * Marcos Medeiros
 */
#include <memory.h>
#include <types.h>

void memset(void *addr, uchar val, uint size)
{
	uchar *ptr = (uchar *) addr;
	uint k = 0;
	while (k < size) {
		*ptr = val;
		ptr++;
		k++;
	}
}

void memcpy(void *dst, const void *src, uint size)
{
	const uchar *psrc = (uchar *) src;
	uchar *pdst = (uchar *) dst;
	uint k = 0;
	while (k < size) {
		*pdst++ = *psrc++;
		k++;
	}
}