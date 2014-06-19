/* 
 * FOFOLITO - Sistema Operacional para RaspberryPi
 * Implementação de lista duplamente ligada, altamente baseada na encontrada
 * no kernel do linux.
 * Marcos Medeiros
 */

#ifndef __LIST_H__
#define __LIST_H__

struct list_head {
	struct list_head *next;
	struct list_head *prev;
};

/*
 * Essa macro é utilizada para iniciar uma lista estática.
 */
#define LIST_HEAD_INIT(head) { &(head), &(head) }

static inline void INIT_LIST_HEAD(struct list_head *head)
{
    head->prev = head;
    head->next = head;
}

/*
 * Função para facilitar a inserção de um nó, quando já
 * sabemos onde estão os dois nós e queremos colocar um
 * entre eles.
 */
static inline void __list_add(struct list_head *new,
                              struct list_head *prev,
                              struct list_head *next)
{
    new->next = next;
    new->prev = prev;
    next->prev = new;
    prev->next = new;
    //printk("list_add: [%p]<->[%p]<->[%p]\n", prev, new, next);
}

static inline void list_add(struct list_head *new,
                            struct list_head *head)
{
    __list_add(new, head, head->next);
}

static inline void list_add_tail(struct list_head *new,
                                 struct list_head *head)
{
    __list_add(new, head->prev, head);
}

/*
 * Função para facilitar a remoção de um nó, quando já
 * sabemos onde estão os dois nós e queremos remover um
 * entre eles.
 */
static inline void __list_del(struct list_head *prev,
                              struct list_head *next)
{
    next->prev = prev;
    prev->next = next;
    //printf("list_del1: [%p]<->[%p]\n", next->prev, prev->next);
}

static inline void list_del(struct list_head *item)
{
    //printf("list_del0: [%p]<->[%p]<->[%p]\n", item->prev, item, item->next);
    __list_del(item->prev, item->next);
}

#define list_entry(ptr, type, member)   \
    ((type *)((char *)(ptr) - (unsigned long)(&((type *)0)->member)))

#define list_foreach(pos, head) \
    for (pos = (head)->next; pos != (head); pos = pos->next)

/*
 * Precisamos utilizar esse modo, caso haver necessidade de remover itens,
 * durante a iteração na lista. Basicamente, salvamos a próxima posição.
 */
#define list_foreach_safe(pos, n, head) \
    for (pos = (head)->next, n = pos->next; pos != (head); pos = n, n = pos->next)


/*
 * Verifica se a lista está vazia.
 */
static inline int list_is_empty(struct list_head *head)
{
    return head->next == head;
}

/*
 * Troca um nó da lista;
 */
static inline void list_replace(struct list_head *old,
                                struct list_head *new)
{
	new->next = old->next;
	new->prev = old->prev;
	new->next->prev = new;
	new->prev->next = new;
}

/*
 * Ordena uma lista ligada, funciona porém é bem lento.
 * Ordenação por seleção.
 */
static void list_sort(struct list_head *head,
                      int (*cmp)(struct list_head *, struct list_head *))
{
    struct list_head sorted;
    struct list_head *curr, *next;
    struct list_head *iter;
    INIT_LIST_HEAD(&sorted);

    if (list_is_empty(head))
        return;

    curr = head->next;
    int add = 0;
    /* Percorre a lista enquanto não chegamos ao fim */
    while (curr != head)
    {
        add = 0;
        /* salva o próximo nó */
        next = curr->next;

        /* Isolamos esse nó */
        curr->next = curr;
        curr->prev = curr;

        iter = sorted.next;
        while (iter != &sorted)
        {
            if (cmp(curr, iter) >= 0)
            {
                list_add(curr, iter->prev);
                add = 1;
                break;
            } else iter = iter->next;
        }
        /* se não foi adicionado, coloca no fim da lista */
        if (!add)
            list_add_tail(curr, &sorted);
        curr = next;
    }
    list_replace(&sorted, head);
}

#endif
