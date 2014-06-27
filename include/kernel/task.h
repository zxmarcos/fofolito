/* 
 * FOFOLITO - Sistema Operacional para RaspberryPi
 * Marcos Medeiros
 */
#ifndef __TASK_H__
#define __TASK_H__

#include <asm/thread.h>
#include <kernel/list.h>

#define TASK_SUSPEND	0
#define TASK_RUNNABLE	1
#define TASK_RUNNING	2

struct task {
	unsigned pid;
	unsigned state;
	struct thread_context thread;
	struct list_head list;
	const char *name;
};


#endif/*__TASK_H__*/