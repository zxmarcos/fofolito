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
void sched_tick();
void sched_yield();
int sched_current_pid();
struct task *sched_current_task();

#endif/*__SCHED_H__*/