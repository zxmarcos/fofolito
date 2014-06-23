/* 
 * FOFOLITO - Sistema Operacional para RaspberryPi
 *
 * Marcos Medeiros
 */
#ifndef __PAGE_H__
#define __PAGE_H__

#include <types.h>

#define PAGE_SIZE		4096
#define PAGE_SHIFT		12
#define PAGE_MASK		0xFFF

struct page_table_entry {
	/* Nunca executar código nesta página */
	unsigned executeNever : 1;
	unsigned _one : 1; /* sempre 1 */
	unsigned cached : 1;
	unsigned buffered : 1;
	unsigned ap : 2;
	unsigned tex : 3;
	unsigned apx : 1;
	unsigned s : 1;
	unsigned ng : 1;
	/* O quadro físico de memória associado a esta entrada */
	unsigned frame : 20;
} __attribute__((packed));

struct page_table {
	struct page_table_entry entries[256];
} __attribute__((packed));

struct page_dir_entry {
	unsigned type : 2;
	unsigned : 1;
	unsigned ns : 1;
	unsigned : 1;
	unsigned domain : 4;
	/* essa região suporta ECC? */
	unsigned p : 1;
	unsigned table : 22;
} __attribute__((packed));

/* Descreve um diretório de páginas, um espaço de endereçamento */
struct page_dir {
	struct page_dir_entry entries[4096];
} __attribute__((packed));

#define PTE_SHIFT	12
#define PDE_SHIFT	10	

/* Abreviações úteis */
typedef struct page_dir pgd_t;
typedef struct page_table pgt_t;
typedef struct page_dir_entry pde_t;
typedef struct page_table_entry pte_t;

/* Converte um descritor de tabela para a própria tabela */
static inline pgt_t *pde_to_pgt(pde_t *pde) {
	return (pgt_t *) (pde->table >> PDE_SHIFT);
}

/* Diretórios do kernel */
extern struct page_dir *k_pgdir;
extern struct page_dir *k_init_pgdir;


#endif
