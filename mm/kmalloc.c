/*
 * FOFOLITO - Sistema Operacional para RaspberryPi
 * Alocador dinâmico de memória.
 * Marcos Medeiros
 */

/*
 * LIMITAÇÕES:
 *   Até o momento só podemos alocar no máximo:
 *        PAGE_SIZE - sizeof(pgdescr_t) - sizeof(blkdescr_t)
 * TODO:
 *   Implementar suporte a SuperPáginas.
 *   Implementar a reorganização da lista de blocos livres na página. - OK (02/07/2013)
 *   Implementar caches, para diminuir a fragmentação.
 */
#include <asm/page.h>
#include <kernel/mm.h>
#include <kernel/printk.h>
#include <kernel/list.h>
#include <kernel/spinlock.h>
#include <errno.h>

#define DEBUG_KMALLOC   0
#if DEBUG_KMALLOC
#define debug_msg       printk
#else
#define debug_msg(...)
#endif

/*
 * Essa definição é baseada na quantidade de bits do processador,
 * para que as alocações sejam alinhadas
 */
#define MIN_SIZE        4
#define MIN_MASK        0x00000003

#define MAGIC_MASK      0xFFFF0000
#define PD_FLAG_MAGIC   0xCAFE0000
#define BD_FLAG_MAGIC   0xDEADBEEF

#define USED_FLAG       1
#define FREE_FLAG       0

/* Converte um descritor de bloco para seu endereço */
#define block_to_ptr(block) \
    ((void *)(((char *) block) + sizeof(blkdescr_t)))
/* Converte um ponteiro para um descritor de bloco */
#define ptr_to_block(ptr)   \
    ((blkdescr_t *) (((char *) ptr) - sizeof(blkdescr_t)))
/* Consegue o primeiro descritor de bloco de um descritor de página */
#define get_first_block(pd) \
    ((blkdescr_t *) (pd + 1))
/*
 * Como os blocos são consecutivos na memória, a partir de um
 * bloco é possível conseguir o próximo bloco.
 */
#define get_next_block(block) \
    ((blkdescr_t *)(((char *) block) + (block)->size + sizeof(blkdescr_t)))

/*
 * Nossa 'cabeça' do heap, é nessa lista que ficam todas as páginas
 * que requisitamos ao alocador de páginas.
 */
struct list_head heap_head = LIST_HEAD_INIT(heap_head);

/* Nosso lock */
static spinlock_t kmalloc_lock;

/*
 * Cada página contém 3 listas, a primeira é a lista de páginas requisitadas
 * ao sistema, as listas free e used são listas utilizadas para gerenciar
 * os blocos que estão dentro dessa página.
 */
typedef struct page_descr {
    unsigned long flags;
    unsigned int available;
    struct list_head list;
    struct list_head free;
    struct list_head used;
} pgdescr_t;

typedef struct block_descr {
    unsigned int flags;
    struct page_descr *root;
    struct list_head list;
    unsigned int size;
} blkdescr_t;

static inline int is_valid_page(pgdescr_t *pd)
{
    return ((pd->flags & MAGIC_MASK) == PD_FLAG_MAGIC);
}

static inline int is_valid_block(blkdescr_t *bd)
{
    return (bd->flags == BD_FLAG_MAGIC);
}

/*
 * Função responsável por requisitar ao SO, páginas de memória
 * contínuas.
 */
static inline void *__get_free_pages(unsigned npages)
{
    return page_alloc();
}

static inline void setup_block_descr(blkdescr_t *blk,
                                     pgdescr_t *root,
                                     unsigned int size)
{
    debug_msg("[%p] setup: root [%p], size: %i, head [%p]\n", blk, root, size, &blk->list);
    blk->size = size;
    //blk->flag = FREE_FLAG;
    blk->root = root;
    blk->flags = BD_FLAG_MAGIC;
    INIT_LIST_HEAD(&blk->list);
}

static inline void setup_page_descr(pgdescr_t *pd)
{
    pd->flags = PD_FLAG_MAGIC;
    pd->available = PAGE_SIZE - sizeof(pgdescr_t);
    /* Iniciamos a lista de blocos dessa página */
    INIT_LIST_HEAD(&pd->free);
    INIT_LIST_HEAD(&pd->used);

    /*
     * Inicialmente temos apenas um bloco livre contendo toda
     * memória disponível nesse bloco.
     */
    pd->available -= sizeof(blkdescr_t);
    blkdescr_t *first = get_first_block(pd);
    setup_block_descr(first, pd, pd->available);
    list_add(&first->list, &pd->free);

    debug_msg("pgdescr_t configurado: %ld bytes livres!\n", pd->available);
}

/* Troca dois ponteiros */
static inline void __swap_pointers(void **a, void **b)
{
    void *aux = *a;
    *a = *b;
    *b = aux;
}

/*
 * Aqui juntamos dois blocos livres consecutivos.
 */
static int join_blocks(blkdescr_t *a, blkdescr_t *b)
{
    pgdescr_t *pd = NULL;
    unsigned join_size = 0;

    if (!is_valid_block(a) || !is_valid_block(b))
        return -EINVPARAM;

    /* Verifica se os dois pertencem ao mesmo descritor */
    if (a->root != b->root)
        return -EINVPARAM;

    pd = a->root;

    /* Se o bloco b estiver antes do bloco a, trocamos os ponteiros */
    if ((unsigned long) a > (unsigned long) b) {
        __swap_pointers((void *) &a, (void *) &b);
    } else if (a == b) {
        /* Não podemos juntar o mesmo bloco :P */
        return -EINVPARAM;
    }
    /* Remove o b da lista */
    list_del(&b->list);

    join_size = b->size + sizeof(blkdescr_t);
    a->size += join_size;
    pd->available += sizeof(blkdescr_t);
    return -EOK;
}

static int __addr_cmp(struct list_head *a,
                      struct list_head *b)
{
    unsigned long a_addr = (unsigned long) list_entry(a, blkdescr_t, list);
    unsigned long b_addr = (unsigned long) list_entry(b, blkdescr_t, list);

    return b_addr - a_addr;
}

static inline int __is_consecutive(blkdescr_t *a, blkdescr_t *b)
{
    blkdescr_t *x = get_next_block(a);
    if (x == b)
        return 1;
    return 0;
}

static void sort_and_join(pgdescr_t *root)
{
    blkdescr_t *a, *b;
    /* reorganiza os blocos livres */
    list_sort(&root->free, __addr_cmp);

    struct list_head *pos, *pos_safe;
    list_foreach_safe(pos, pos_safe, &root->free) {
        /* não podemos fazer join com a cabeça da lista :P */
        if (pos->prev != &root->free) {
            a = list_entry(pos->prev, blkdescr_t, list);
            b = list_entry(pos, blkdescr_t, list);
            /* se as entradas forem consecutivas, faz o join */
            if (__is_consecutive(a, b)) {
                //     debug_msg("Join blocks %p - %p\n", a, b);
                join_blocks(a, b);
            }
        }
    }
}

/*
 * Aqui quebramos um bloco livre.
 * Ao quebrar um bloco ele automaticamente é tomado como alocado,
 * e com o que sobra é criado um novo bloco livre.
 */
static int break_block(blkdescr_t *blk, unsigned int size)
{
    pgdescr_t *pd = blk->root;
    
    /* Apenas dividimos blocos :P */
    if (size > blk->size) {
        return -EINVPARAM;
    } else if (size < blk->size) {
        /* Cálcula o tamanho do novo bloco */
        unsigned nb_size = blk->size - size;

        /*
         * Se o próximo bloco não tiver pelo menos espaço para
         * o cabeçalho e o tamanho mínimo, não dividimos esse bloco.
         */
        if (nb_size < (sizeof(blkdescr_t) + MIN_SIZE))
            return -ENOMEM;

        blk->size = size;
        /* Removemos nosso bloco da lista de blocos livres */
        list_del(&blk->list);
        /*
         * Retiramos o tamanho desse bloco + tamanho do descritor
         * para um novo bloco da nossa página.
         */
        pd->available -= sizeof(blkdescr_t);
        if (pd->available) {
            blkdescr_t *new = get_next_block(blk);
            setup_block_descr(new, pd, nb_size - sizeof(blkdescr_t));
            list_add(&new->list, &pd->free);
        }

    } else
        /*
         * Se o bloco tiver exatamente o mesmo tamanho, então apenas o
         * retiramos da lista de blocos livres
         */
        list_del(&blk->list);

    pd->available -= size;


    /* Adicionamos a lista de blocos utilizados */
    list_add(&blk->list, &pd->used);
    return -EOK;
}

#if DEBUG_KMALLOC
void kmalloc_debug()
{
    struct list_head *pos;
    list_foreach(pos, &heap_head) {
        pgdescr_t *pd = list_entry(pos, pgdescr_t, list);
        debug_msg("Disponível na página %p = %ld\n", pd, pd->available);
        debug_msg("Listando blocos livres:\n");
        struct list_head *bpos;
        list_foreach(bpos, &pd->free) {
            blkdescr_t *blk = list_entry(bpos, blkdescr_t, list);
            debug_msg("Bloco %p = %ld\n", blk, blk->size);
        }
        debug_msg("Listando blocos utilizados:\n");
        list_foreach(bpos, &pd->used) {
            blkdescr_t *blk = list_entry(bpos, blkdescr_t, list);
            debug_msg("Bloco %p = %ld\n", blk, blk->size);
        }
    }
}
#endif

int kmalloc_init()
{
    pgdescr_t *pd = (pgdescr_t *) __get_free_pages(1);
    if (!pd) {
        return -1;
    }
    INIT_LIST_HEAD(&heap_head);
    /* Adiciona a página a nossa lista */
    list_add(&pd->list, &heap_head);
    setup_page_descr(pd);
    spin_lock_init(&kmalloc_lock);
    return 0;
}

/* Função utilitária para cálcular o tamanho mínimo. */
static inline unsigned __roundsize(unsigned size)
{
    if (size & MIN_MASK) {
        size &= ~MIN_MASK;
        size += MIN_SIZE;
    }
    return size;
}

/*
 * Aloca um bloco de memória.
 */
void *kmalloc(unsigned int size)
{
    size = __roundsize(size);
    unsigned counter = 0;
    struct list_head *pg_pos;
    pgdescr_t *page;

    /* Temos uma limitação de tamanho máximo alocado */
    if (size >= (sizeof(blkdescr_t) + PAGE_SIZE - sizeof(pgdescr_t)))
        return NULL;

    /* 
     * Este código não é reentrante e é crítico.
     * Apenas uma thread pode executa-lo por vez.
     */
    spin_lock(&kmalloc_lock);

try_again:
    list_foreach(pg_pos, &heap_head) {
        pgdescr_t *pd = list_entry(pg_pos, pgdescr_t, list);
        /* Heap corrompido? */
        if (!is_valid_page(pd))
            goto failed;

        /* Verifica se temos espaço nessa página para alocar */
        if (pd->available >= size) {
            struct list_head *blk_pos, *blk_pos_safe;
            /* Percorremos a lista de blocos livres */
            list_foreach_safe(blk_pos, blk_pos_safe, &pd->free) {
                blkdescr_t *blk = list_entry(blk_pos, blkdescr_t, list);
                if (!is_valid_block(blk))
                    goto failed;
                
                if (blk->size >= size) {
                    if (!break_block(blk, size)) {
                        /*
                         * NOTA: A partir desse momento, não podemos mais
                         * continuar iterando a lista, pois essa entrada não
                         * pertence mais a lista de livres.
                         * Antes de retornar, temos que liberar o lock.
                         */
                        spin_unlock(&kmalloc_lock);
                        return block_to_ptr(blk);
                    }
                }
            }
        }
        counter++;
    }
    /*
     * Se chegamos aqui é porque não temos memória, então vamos tentar
     * alocar mais páginas de memória.
     */
    page = (pgdescr_t *) __get_free_pages(1);
    if (!page)
        goto failed;
    
    /* Adiciona a página a nossa lista */
    list_add(&page->list, &heap_head);
    setup_page_descr(page);
    /* Tentamos alocar novamente :) */
    goto try_again;

failed:
    spin_unlock(&kmalloc_lock);
    return NULL;
}


/*
 * Desaloca um bloco de memória.
 */
void kfree(void *b_ptr)
{
    if (!b_ptr)
        return;
    blkdescr_t *blk = ptr_to_block(b_ptr);
    if (is_valid_block(blk)) {
        /* Remove da lista atual */
        list_del(&blk->list);
        list_add(&blk->list, &blk->root->free);
        sort_and_join(blk->root);
    }
}
