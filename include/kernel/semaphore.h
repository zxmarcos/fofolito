/* 
 * FOFOLITO - Sistema Operacional para RaspberryPi
 * Marcos Medeiros
 */
#ifndef __SEMAPHORE_H__
#define __SEMAPHORE_H__

#include <kernel/list.h>

struct semaphore {
	int counter;
	struct list_head waiters;
};

int semaphore_init(struct semaphore *sem, int value);
int down(struct semaphore *sem);
int up(struct semaphore *sem);

#endif