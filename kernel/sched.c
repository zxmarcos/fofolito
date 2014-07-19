/* 
 * FOFOLITO - Sistema Operacional para RaspberryPi
 * Escalonador de processos.
 *
 * Os processos são escalonados utilizando o algoritmo Round Robin
 * Marcos Medeiros
 */
#include <asm/thread.h>
#include <asm/irq.h>
#include <kernel/task.h>
#include <kernel/sched.h>
#include <kernel/list.h>
#include <types.h>
#include <errno.h>

static struct list_head run_queue;
static int sched_initalized = 0;
static struct task *current_task = NULL;

/* Tarefa 'idle', é nesse contexto que o kernel é iniciado */
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

struct task *sched_current_task()
{
	return current_task;
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

/*
 * Inicializa o escalonador.
 * Iniciando a lista de processos em execução e definindo
 * o contexto para a tarefa inicial ('idle'), que é onde o boot
 * é executado.
 */
int sched_init()
{
	INIT_LIST_HEAD(&run_queue);

	/* 
	 * Define a tarefa atual como idle e coloca ela no fim da lista
	 * de execução do escalonador.
	 */
	current_task = &idle_task;
	sched_add_task(current_task);

	sched_initalized = 1;
	return -EOK;
}

/*
 * Coloca uma tarefa no fim da lista de execução.
 */
int sched_add_task(struct task *tsk)
{
	if (!tsk || !sched_initalized) {
		return -EINVPARAM;
	}
	list_add_tail(&tsk->list, &run_queue);
	return -EOK;
}

void sched_yield()
{
	unsigned flags;

	irq_save_state(flags);
	schedule();
	irq_restore_state(flags);
}

/*
 * Aqui é onde as tarefas são selecionadas e trocadas, é importante
 * que o contexto seja atômico, pois iremos manipular estruturas globais.
 * As interrupções estão desabilitadas aqui.
 */
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
			goto do_switch;
		}
	} else {
		/* Coloca a tarefa atual no fim da fila */
		sched_push_task(current_task);

		/* Se for a mesma tarefa não é preciso fazer nada */
		if (last == next) {
			return;
		}
		current_task = next;
		goto do_switch;
	}
	return;

	/*
	 * Depois dessa rotina estaremos executando no contexto de outra
	 * tarefa. A primeira vez que uma tarefa é selecionada para ser
	 * executada depois de criada, ela não retorna a esse ponto, mas
	 * sim em 'ret_from_fork' para começar a execução de imediato.
	 */
do_switch:
	switch_to(&last->thread, &current_task->thread);
}
