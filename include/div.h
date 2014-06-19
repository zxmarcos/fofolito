/* 
 * FOFOLITO - Sistema Operacional para RaspberryPi
 * 
 * Funções de divisão e módulos.
 * Marcos Medeiros
 */
#ifndef ___DIV_H___
#define ___DIV_H___

#include <types.h>

uint __aeabi_uidiv(uint num, uint den);
uint __aeabi_uidivmod(uint num, uint den);
int __aeabi_idiv(int num, int den);
int __aeabi_idivmod(int num, int den);

#endif/*__IDIV_H__*/
