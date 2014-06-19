/* 
 * FOFOLITO - Sistema Operacional para RaspberryPi
 * Memory Barriers implementados em hardware
 *
 * Marcos Medeiros
 */
#ifndef __ASM_BARRIER_H
#define __ASM_BARRIER_H

#define do_dsb()	asm volatile("mcr p15, 0, %0, c7, c10, 4" ::"r" (0): "memory")
#define do_dmb()	asm volatile("mcr p15, 0, %0, c7, c10, 5" ::"r" (0): "memory")

#endif
