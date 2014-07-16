/* 
 * FOFOLITO - Sistema Operacional para RaspberryPi
 * Marcos Medeiros
 */
#ifndef __SPINLOCK_H__
#define __SPINLOCK_H__

#include <asm/atomic.h>
#include <asm/barrier.h>

typedef int spinlock_t;

static inline void spin_lock_init(spinlock_t *lock)
{
	atomic_exchange(0, lock);
}

/* 
 * Aqui apenas utilizamos o mecanismo de busywait, até conseguirmos
 * travar o lock.
 */
static inline void spin_lock(spinlock_t *lock)
{
	int value = 0;
	do {
		value = atomic_exchange(1, lock);
	} while (value == 1);
}

static inline void spin_unlock(spinlock_t *lock)
{
	atomic_exchange(0, lock);
}

#endif