/* 
 * FOFOLITO - Sistema Operacional para RaspberryPi
 * 
 * Marcos Medeiros
 */
#ifndef __MMU_H__
#define __MMU_H__

#include <types.h>

#define MMU_CR_ENABLE		0x001
#define MMU_CR_ALIGN_FAULT	0x002
#define MMU_CR_WRBUF_ENABLE	0x008
#define MMU_CR_S_BIT		0x100
#define MMU_CR_R_BIT		0x200
#define MMU_CR_XP_BIT		(1 << 23)

uint mmu_get_ttbr();
uint mmu_get_ttbcr();
uint mmu_get_dom();
uint mmu_get_cr();
void mmu_set_ttbr(void *addr);
void mmu_set_ttbcr(uint value);
void mmu_set_dom(uint value);
void mmu_set_cr(uint value);

void mmu_set_vector_base(void *vaddr);

void mmu_flush_tlb();
void mmu_flush_icash();
void mmu_flush_dcash();
void mmu_invpg_entry(void *page);
void mmu_invpg_icache(void *page);
void mmu_invpg_dcache(void *page);


#endif