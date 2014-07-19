/* 
 * FOFOLITO - Sistema Operacional para RaspberryPi
 * Marcos Medeiros
 */
#include <asm/irq.h>
#include <kernel/task.h>
#include <kernel/list.h>
#include <kernel/semaphore.h>
#include <kernel/sched.h>
#include <errno.h>

struct waiter_info {
	struct task *task;
	struct list_head list;
};

int semaphore_init(struct semaphore *sem, int value)
{
	if (!sem)
		return -EINVPARAM;
	sem->counter = value;
	INIT_LIST_HEAD(&sem->waiters);
	return -EOK;
}

static inline int wait_here(struct semaphore *sem)
{
	struct waiter_info info;
	struct task *task = sched_current_task();

	info.task = task;
	INIT_LIST_HEAD(&info.list);

	list_add_tail(&info.list, &sem->waiters);
	task->state = TASK_SUSPEND;

	int pid = sched_current_pid();
	while (task->state != TASK_RUNNABLE) {
		sched_yield();
	}
	return -EOK;
}

int down(struct semaphore *sem)
{
	int ret = -EOK;
	unsigned flags;

	irq_save_state(flags);
	if (sem->counter > 0) {
		sem->counter--;
	} else {
		sem->counter = 0;
		ret = wait_here(sem);
	}
	irq_restore_state(flags);
	return ret;
}

static inline int wake_up_first(struct semaphore *sem)
{
	struct waiter_info *info = list_entry(sem->waiters.next, struct waiter_info, list);
	list_del(&info->list);
	info->task->state = TASK_RUNNABLE;
	return -EOK;
}

int up(struct semaphore *sem)
{
	int ret = -EOK;
	unsigned flags;

	irq_save_state(flags);
	if (list_is_empty(&sem->waiters)) {
		sem->counter++;
	} else {
		ret = wake_up_first(sem);
	}
	irq_restore_state(flags);
	return ret;
}
