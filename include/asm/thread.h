/* 
 * FOFOLITO - Sistema Operacional para RaspberryPi
 * 
 * Contexto de hardware de uma thread
 * Marcos Medeiros
 */
#ifndef __THREAD_H__
#define __THREAD_H__

struct cpu_context {
	unsigned long r[13];
	unsigned long sp;
	unsigned long pc;
};

struct thread_context {
	struct cpu_context cpu;
};

void switch_to(struct thread_context *prev, struct thread_context *next);

#endif/*__THREAD_H__*/