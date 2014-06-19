/* 
 * FOFOLITO - Sistema Operacional para RaspberryPi
 * 
 * Funções de divisão e módulos.
 * Marcos Medeiros
 */
#include <div.h>
#include <types.h>

uint __aeabi_uidiv(uint num, uint den)
{
	uint r = 0;
	uint q = 0;
	for (int i = 31; i >= 0; i--) {
		r <<= 1;
		r |= ((num >> i) & 1);
		if (r >= den) {
			r -= den;
			q |= (1 << i);
		}
	}
	return q;
}

uint __aeabi_uidivmod(uint num, uint den)
{
	uint r = 0;
	for (int i = 31; i >= 0; i--) {
		r <<= 1;
		r |= ((num >> i) & 1);
		if (r >= den) {
			r -= den;
		}
	}
	return r;
}

int __aeabi_idiv(int num, int den)
{
	int r = 0;
	int q = 0;
	/* cálcula o sinal do resultado */
	int sign = (num & 0x80000000) ^ (den & 0x80000000);
	
	/* Precisamos deixar como positivos os operandos */
	if (num < 0)
		num *= -1;
	if (den < 0)
		den *= -1;

	for (int i = 31; i >= 0; i--) {
		r <<= 1;
		r |= ((num >> i) & 1);
		if (r >= den) {
			r -= den;
			q |= (1 << i);
		}
	}

	if (sign)
		q *= -1;
	return q;
}

int __aeabi_idivmod(int num, int den)
{
	int r = 0;
	/* Precisamos deixar como positivos os operandos */
	if (num < 0)
		num *= -1;
	if (den < 0)
		den *= -1;

	for (int i = 31; i >= 0; i--) {
		r <<= 1;
		r |= ((num >> i) & 1);
		if (r >= den) {
			r -= den;
		}
	}

	return r;
}