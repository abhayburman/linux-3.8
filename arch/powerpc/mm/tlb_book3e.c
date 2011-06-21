/*
 * ISA 2.06+ book3e MMU support
 *
 * Copyright 2011 Freescale Semiconductor, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; version 2 of the
 * License.
 */

#include <linux/mm.h>
#include <linux/irq.h>
#include <asm/mmu.h>
#include <asm/pgtable.h>

void book3e_tlb_preload(struct mm_struct *mm, unsigned long ea, pte_t pte)
{
	unsigned long flags;
	unsigned long mas2;
	u64 mas7_3;
	u32 shifted_pid;

	/* Don't handle non-standard page sizes */
	if ((pte_val(pte) & _PAGE_PSIZE_MSK) != _PAGE_PSIZE)
		return;

	local_irq_save(flags);

	shifted_pid = mfspr(SPRN_PID) << MAS6_SPID_SHIFT;
	mtspr(SPRN_MAS6, shifted_pid);

	asm volatile(ASM_MMU_FTR_IFSET(PPC_TLBSRX_DOT(0, %0),
				       "tlbsx 0, %0", %1)
		     : : "r" (ea), "i" (MMU_FTR_USE_TLBRSRV) : "cc");

	mtspr(SPRN_MAS1, MAS1_VALID | shifted_pid | (_PAGE_PSIZE >> 1));

	mas2 = ea & PAGE_MASK;
	mas2 |= (pte_val(pte) >> PTE_WIMGE_SHIFT) & MAS2_WIMGE_MASK;

	mas7_3 = (u64)pte_pfn(pte) << PAGE_SHIFT;
	mas7_3 |= (pte_val(pte) >> PTE_BAP_SHIFT) & MAS3_BAP_MASK;
	mas7_3 |= (pte_val(pte) >> 8) & (MAS3_U0 | MAS3_U1 | MAS3_U2 | MAS3_U3);

	if (!((pte_val(pte) & _PAGE_DIRTY)))
		mas7_3 &= ~(MAS3_SW | MAS3_UW);

	mtspr(SPRN_MAS2, mas2);
#ifdef CONFIG_PPC64
	mtspr(SPRN_MAS7_MAS3, mas7_3);
#else
	mtspr(SPRN_MAS7, upper_32_bits(mas7_3));
	mtspr(SPRN_MAS3, lower_32_bits(mas7_3));
#endif

	asm volatile("tlbwe");
	local_irq_restore(flags);
}
