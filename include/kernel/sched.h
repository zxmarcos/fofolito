/* 
 * FOFOLITO - Sistema Operacional para RaspberryPi
 * Marcos Medeiros
 */
#ifndef __SCHED_H__
#define __SCHED_H__

#include <kernel/task.h>

int sched_init();
int sched_add_task(struct task *tsk);
void schedule();

int sched_current_pid();

#endif/*__SCHED_H__*/