/* 
 * FOFOLITO - Sistema Operacional para RaspberryPi
 * Marcos Medeiros
 */
#include <asm/thread.h>
#include <kernel/task.h>
#include <kernel/list.h>
#include <types.h>
#include <errno.h>

static struct list_head run_queue;
static int sched_initalized = 0;
static struct task *current_task = NULL;

static struct task idle_task = {
	.pid = 0,
	.state = TASK_RUNNABLE,
	.name = "idle",
};

int sched_current_pid()
{
	if (current_task)
		return current_task->pid;
	return 0;
}

#define RQ_NEXT_TASK()	list_entry(run_queue.next, struct task, list)

/* Pega a próxima tarefa a executar */
static inline
struct task *sched_pop_task()
{
	/* Nenhuma tarefa na lista */
	if (run_queue.next == &run_queue)
		return NULL;

	struct task *tsk = RQ_NEXT_TASK();
	list_del(&tsk->list);

	return tsk;
}

static inline
void sched_push_task(struct task *tsk)
{
	list_add_tail(&tsk->list, &run_queue);
}

int sched_init()
{
	INIT_LIST_HEAD(&run_queue);

	current_task = &idle_task;

	sched_add_task(current_task);

	sched_initalized = 1;
	return -EOK;
}

int sched_add_task(struct task *tsk)
{
	if (!tsk || !sched_initalized) {
		return -EINVPARAM;
	}
	list_add_tail(&tsk->list, &run_queue);
	return -EOK;
}


void schedule()
{
	struct task *last;
	struct task *next;

	/* O escalonador ainda não foi iniciado */
	if (!sched_initalized)
		return;

	if (!current_task)
		return;

	last = current_task;
	next = sched_pop_task();

	/* Não existe próxima tarefa */
	if (!next) {
		/* Se já não estiver em idle */
		if (last != &idle_task) {
			current_task = &idle_task;
			switch_to(&last->thread, &current_task->thread);
		}
	} else {
		/* Coloca a tarefa atual no fim da fila */
		sched_push_task(current_task);
		if (last == next) {
			return;
		}
		current_task = next;
		switch_to(&last->thread, &current_task->thread);
	}
	printk(":S:");
}
