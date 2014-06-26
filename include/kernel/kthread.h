/* 
 * FOFOLITO - Sistema Operacional para RaspberryPi
 * 
 * Thread em modo kernel
 * Marcos Medeiros
 */
#ifndef __KTHREAD_T__
#define __KTHREAD_T__

#include <asm/thread.h>

struct kthread {
	struct hw_thread hw;
	unsigned flags;
};


#endif/*__KTHREAD_T__*/