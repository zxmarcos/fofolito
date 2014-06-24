/* 
 * FOFOLITO - Sistema Operacional para RaspberryPi
 *
 * Marcos Medeiros
 */
#ifndef __PAGE_H__
#define __PAGE_H__

#include <types.h>
#include <asm/asm.h>

#define PAGE_SIZE		4096
#define PAGE_SHIFT		12
#define PAGE_MASK		0xFFF

#define PFN_FROM_PA(paddr)	((paddr) >> PAGE_SHIFT)
/* Retorna o número da página associado a um endereço virtual */
#define PFN_FROM_VA(vaddr)	PFN_FROM_PA(__virt_to_phys(vaddr))

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


#define PDE_TYPE_COARSE	0x01


/* Descreve um diretório de páginas, um espaço de endereçamento */
struct page_dir {
	struct page_dir_entry entries[4096];
} __attribute__((packed));

#define PTE_SHIFT	12
#define PTE_MASK	0xFF
#define PGT_SHIFT	10
#define PDE_SHIFT	20	
#define PDE_MASK	0xFFF

/* Abreviações úteis */
typedef struct page_dir_entry pde_t;
typedef struct page_table_entry pte_t;

/* Diretórios do kernel */
extern struct page_dir k_pgdir;
extern struct page_dir k_init_pgdir;


/* 
 * Recebe um diretório e um endereço e retorna a descrição
 * da tabela que mapeia a página do endereço
 */
static inline pde_t *
pde_offset(struct page_dir *dir, unsigned long address) {
	if (dir == NULL)
		return NULL;
	return &dir->entries[(address >> PDE_SHIFT) & PDE_MASK];
}

/*
 * Verifica se uma pde existe.
 */
static inline int pde_none(pde_t *pde) {
	if (!pde)
		return 1;
	if ((pde->type != PDE_TYPE_COARSE) || !pde->table) {
		return 1;
	}
	return 0;
}

/*
 * Converte um pde para um page_table
 */
static inline struct page_table *
pde_to_page_table(pde_t *entry) {
	if (entry == NULL)
		return NULL;
	return (struct page_table *)__phys_to_virt(entry->table << PGT_SHIFT);
}

/*
 * Recebe uma tabela de páginas e um endereço e retorna
 * a descrição da entrada que mapeia a página do endereço
 */
static inline pte_t *
pte_offset(struct page_table *table, unsigned long address) {
	if (table == NULL)
		return NULL;
	return &table->entries[(address >> PTE_SHIFT) & PTE_MASK];
}

/*
 * Converte um pde diretamente pra um pte baseado no endereço.
 */
static inline pte_t *
pte_offset_fast(pde_t *entry, unsigned long address) {
	struct page_table *table = pde_to_page_table(entry);
	return pte_offset(table, address);
}

/* Verifica se um pte não existe */
static inline int pte_none(pte_t *pte) {
	if (!pte)
		return 1;
	if (!pte->frame || !pte->_one)
		return 1;
	return 0;
}

/*
 * Mapeia uma página no espaço de endereçamento, no endereço virtual
 * indicado por va
 */
int page_map(struct page_dir *dir, void *va, unsigned frame);

int page_load_directory(struct page_dir *dir);

#endif
