/*
 * Enter and leave deep sleep/sleep state on MPC85xx
 *
 * Author: Scott Wood <scottwood@freescale.com>
 *
 * Copyright (C) 2006-2014 Freescale Semiconductor, Inc. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 */

#include <asm/page.h>
#include <asm/ppc_asm.h>
#include <asm/reg.h>
#include <asm/asm-offsets.h>
#include <asm/fsl_sleep.h>

	.section .data
	.align	5
	.global mpc85xx_sleep_save_area
mpc85xx_sleep_save_area:
/* assume that FSL_CPU_SR_SIZE is equal to or large than STATE_SAVE_SIZE */
	.space	FSL_CPU_SR_SIZE

#ifdef CONFIG_PPC32
#define SS_TB		0x00
#define SS_HID		0x08 /* 2 HIDs */
#define SS_IAC		0x10 /* 2 IACs */
#define SS_DAC		0x18 /* 2 DACs */
#define SS_DBCR		0x20 /* 3 DBCRs */
#define SS_PID		0x2c /* 3 PIDs */
#define SS_SPRG		0x38 /* 8 SPRGs */
#define SS_IVOR		0x58 /* 20 interrupt vectors */
#define SS_TCR		0xa8
#define SS_BUCSR	0xac
#define SS_L1CSR	0xb0 /* 2 L1CSRs */
#define SS_MSR		0xb8
#define SS_USPRG	0xbc
#define SS_GPREG	0xc0 /* r12-r31 */
#define SS_LR		0x110
#define SS_CR		0x114
#define SS_SP		0x118
#define SS_CURRENT	0x11c
#define SS_IVPR		0x120
#define SS_BPTR		0x124


#define STATE_SAVE_SIZE 0x128

	.section .data
	.align	5
ccsrbase_low:
	.long	0
ccsrbase_high:
	.long	0
powmgtreq:
	.long	0

	.section .text
	.align	12

	/*
	 * r3 = high word of physical address of CCSR
	 * r4 = low word of physical address of CCSR
	 * r5 = JOG or deep sleep request
	 *      JOG-0x00200000, deep sleep-0x00100000
	 */
_GLOBAL(mpc85xx_enter_deep_sleep)
	lis	r6, ccsrbase_low@ha
	stw	r4, ccsrbase_low@l(r6)
	lis	r6, ccsrbase_high@ha
	stw	r3, ccsrbase_high@l(r6)

	lis	r6, powmgtreq@ha
	stw	r5, powmgtreq@l(r6)

	lis	r10, mpc85xx_sleep_save_area@h
	ori	r10, r10, mpc85xx_sleep_save_area@l

	mfspr	r5, SPRN_HID0
	mfspr	r6, SPRN_HID1

	stw	r5, SS_HID+0(r10)
	stw	r6, SS_HID+4(r10)

	mfspr	r4, SPRN_IAC1
	mfspr	r5, SPRN_IAC2
	mfspr	r6, SPRN_DAC1
	mfspr	r7, SPRN_DAC2

	stw	r4, SS_IAC+0(r10)
	stw	r5, SS_IAC+4(r10)
	stw	r6, SS_DAC+0(r10)
	stw	r7, SS_DAC+4(r10)

	mfspr	r4, SPRN_DBCR0
	mfspr	r5, SPRN_DBCR1
	mfspr	r6, SPRN_DBCR2

	stw	r4, SS_DBCR+0(r10)
	stw	r5, SS_DBCR+4(r10)
	stw	r6, SS_DBCR+8(r10)

	mfspr	r4, SPRN_PID0
	mfspr	r5, SPRN_PID1
	mfspr	r6, SPRN_PID2

	stw	r4, SS_PID+0(r10)
	stw	r5, SS_PID+4(r10)
	stw	r6, SS_PID+8(r10)

	mfspr	r4, SPRN_SPRG0
	mfspr	r5, SPRN_SPRG1
	mfspr	r6, SPRN_SPRG2
	mfspr	r7, SPRN_SPRG3

	stw	r4, SS_SPRG+0x00(r10)
	stw	r5, SS_SPRG+0x04(r10)
	stw	r6, SS_SPRG+0x08(r10)
	stw	r7, SS_SPRG+0x0c(r10)

	mfspr	r4, SPRN_SPRG4
	mfspr	r5, SPRN_SPRG5
	mfspr	r6, SPRN_SPRG6
	mfspr	r7, SPRN_SPRG7

	stw	r4, SS_SPRG+0x10(r10)
	stw	r5, SS_SPRG+0x14(r10)
	stw	r6, SS_SPRG+0x18(r10)
	stw	r7, SS_SPRG+0x1c(r10)

	mfspr	r4, SPRN_IVPR
	stw	r4, SS_IVPR(r10)

	mfspr	r4, SPRN_IVOR0
	mfspr	r5, SPRN_IVOR1
	mfspr	r6, SPRN_IVOR2
	mfspr	r7, SPRN_IVOR3

	stw	r4, SS_IVOR+0x00(r10)
	stw	r5, SS_IVOR+0x04(r10)
	stw	r6, SS_IVOR+0x08(r10)
	stw	r7, SS_IVOR+0x0c(r10)

	mfspr	r4, SPRN_IVOR4
	mfspr	r5, SPRN_IVOR5
	mfspr	r6, SPRN_IVOR6
	mfspr	r7, SPRN_IVOR7

	stw	r4, SS_IVOR+0x10(r10)
	stw	r5, SS_IVOR+0x14(r10)
	stw	r6, SS_IVOR+0x18(r10)
	stw	r7, SS_IVOR+0x1c(r10)

	mfspr	r4, SPRN_IVOR8
	mfspr	r5, SPRN_IVOR9
	mfspr	r6, SPRN_IVOR10
	mfspr	r7, SPRN_IVOR11

	stw	r4, SS_IVOR+0x20(r10)
	stw	r5, SS_IVOR+0x24(r10)
	stw	r6, SS_IVOR+0x28(r10)
	stw	r7, SS_IVOR+0x2c(r10)

	mfspr	r4, SPRN_IVOR12
	mfspr	r5, SPRN_IVOR13
	mfspr	r6, SPRN_IVOR14
	mfspr	r7, SPRN_IVOR15

	stw	r4, SS_IVOR+0x30(r10)
	stw	r5, SS_IVOR+0x34(r10)
	stw	r6, SS_IVOR+0x38(r10)
	stw	r7, SS_IVOR+0x3c(r10)

	mfspr	r4, SPRN_IVOR32
	mfspr	r5, SPRN_IVOR33
	mfspr	r6, SPRN_IVOR34
	mfspr	r7, SPRN_IVOR35

	stw	r4, SS_IVOR+0x40(r10)
	stw	r5, SS_IVOR+0x44(r10)
	stw	r6, SS_IVOR+0x48(r10)
	stw	r7, SS_IVOR+0x4c(r10)

	mfspr	r4, SPRN_TCR
	mfspr	r5, SPRN_BUCSR
	mfspr	r6, SPRN_L1CSR0
	mfspr	r7, SPRN_L1CSR1
	mfspr	r8, SPRN_USPRG0

	stw	r4, SS_TCR(r10)
	stw	r5, SS_BUCSR(r10)
	stw	r6, SS_L1CSR+0(r10)
	stw	r7, SS_L1CSR+4(r10)
	stw	r8, SS_USPRG+0(r10)

	stmw	r12, SS_GPREG(r10)

	mfmsr	r4
	mflr	r5
	mfcr	r6

	stw	r4, SS_MSR(r10)
	stw	r5, SS_LR(r10)
	stw	r6, SS_CR(r10)
	stw	r1, SS_SP(r10)
	stw	r2, SS_CURRENT(r10)

1:	mftbu	r4
	mftb	r5
	mftbu	r6
	cmpw	r4, r6
	bne	1b

	stw	r4, SS_TB+0(r10)
	stw	r5, SS_TB+4(r10)

	lis	r5, ccsrbase_low@ha
	lwz	r4, ccsrbase_low@l(r5)
	lis	r5, ccsrbase_high@ha
	lwz	r3, ccsrbase_high@l(r5)

	/* Disable machine checks and critical exceptions */
	mfmsr	r5
	rlwinm	r5, r5, 0, ~MSR_CE
	rlwinm	r5, r5, 0, ~MSR_ME
	mtmsr	r5
	isync

	/* Use TLB1[15] to map the CCSR at 0xf0000000 */
	lis	r5, 0x100f
	mtspr	SPRN_MAS0, r5
	lis	r5, 0xc000
	ori	r5, r5, 0x0500
	mtspr	SPRN_MAS1, r5
	lis	r5, 0xf000
	ori	r5, r5, 0x000a
	mtspr	SPRN_MAS2, r5
	rlwinm	r5, r4, 0, 0xfffff000
	ori	r5, r5, 0x0005
	mtspr	SPRN_MAS3, r5
	mtspr	SPRN_MAS7, r3
	isync
	tlbwe
	isync

	lis	r3, 0xf000
	lwz	r4, 0x20(r3)
	stw	r4, SS_BPTR(r10)

	lis	r3, 0xf002	/* L2 cache controller at CCSR+0x20000 */
	bl	flush_disable_L2
	bl	__flush_disable_L1

	/* Enable I-cache, so as not to upset the bus
	 * with our loop.
	 */

	mfspr	r4, SPRN_L1CSR1
	ori	r4, r4, 1
	mtspr	SPRN_L1CSR1, r4
	isync

	/* Set boot page translation */
	lis	r3, 0xf000
	lis	r4, (mpc85xx_deep_resume - PAGE_OFFSET)@h
	ori	r4, r4, (mpc85xx_deep_resume - PAGE_OFFSET)@l
	rlwinm	r4, r4, 20, 0x000fffff
	oris	r4, r4, 0x8000
	stw	r4, 0x20(r3)
	lwz	r4, 0x20(r3)		/* read-back to flush write */
	twi	0, r4, 0
	isync

	/* Disable the decrementer */
	mfspr	r4, SPRN_TCR
	rlwinm	r4, r4, 0, ~TCR_DIE
	mtspr	SPRN_TCR, r4

	mfspr	r4, SPRN_TSR
	oris	r4, r4, TSR_DIS@h
	mtspr	SPRN_TSR, r4

	/* set PMRCCR[VRCNT] to wait power stable for 40ms */
	lis	r3, 0xf00e
	lwz	r4, 0x84(r3)
	clrlwi	r4, r4, 16
	oris	r4, r4, 0x12a3
	stw	r4, 0x84(r3)
	lwz	r4, 0x84(r3)

	/* set deep sleep bit in POWMGTSCR */
	lis	r3, powmgtreq@ha
	lwz	r8, powmgtreq@l(r3)

	lis	r3, 0xf00e
	lwz	r4, 0x80(r3)
	or	r4, r4, r8
	stw	r4, 0x80(r3)
	lwz	r4, 0x80(r3)		/* read-back to flush write */
	twi	0, r4, 0
	isync

	mftb	r5
1:	/* spin until either we enter deep sleep, or the sleep process is
	 * aborted due to a pending wakeup event.  Wait some time between
	 * accesses, so we don't flood the bus and prevent the pmc from
	 * detecting an idle system.
	 */

	mftb	r4
	subf	r7, r5, r4
	cmpwi	r7, 1000
	blt	1b
	mr	r5, r4

	lwz	r6, 0x80(r3)
	andis.	r6, r6, 0x0010
	bne	1b
	b	2f

2:	mfspr	r4, SPRN_PIR
	andi.	r4, r4, 1
99:	bne	99b

	/* Establish a temporary 64MB 0->0 mapping in TLB1[1]. */
	lis	r4, 0x1001
	mtspr	SPRN_MAS0, r4
	lis	r4, 0xc000
	ori	r4, r4, 0x0800
	mtspr	SPRN_MAS1, r4
	li	r4, 0
	mtspr	SPRN_MAS2, r4
	li	r4, 0x0015
	mtspr	SPRN_MAS3, r4
	li	r4, 0
	mtspr	SPRN_MAS7, r4
	isync
	tlbwe
	isync

	lis	r3, (3f - PAGE_OFFSET)@h
	ori	r3, r3, (3f - PAGE_OFFSET)@l
	mtctr	r3
	bctr

	/* Locate the resume vector in the last word of the current page. */
	. = mpc85xx_enter_deep_sleep + 0xffc
mpc85xx_deep_resume:
	b	2b

3:
	/* Restore the contents of TLB1[0].  It is assumed that it covers
	 * the currently executing code and the sleep save area, and that
	 * it does not alias our temporary mapping (which is at virtual zero).
	 */
	lis	r3, (TLBCAM - PAGE_OFFSET)@h
	ori	r3, r3, (TLBCAM - PAGE_OFFSET)@l

	lwz	r4, 0(r3)
	lwz	r5, 4(r3)
	lwz	r6, 8(r3)
	lwz	r7, 12(r3)
	lwz	r8, 16(r3)

	mtspr	SPRN_MAS0, r4
	mtspr	SPRN_MAS1, r5
	mtspr	SPRN_MAS2, r6
	mtspr	SPRN_MAS3, r7
	mtspr	SPRN_MAS7, r8

	isync
	tlbwe
	isync

	/* Access the ccsrbase address with TLB1[0] */
	lis	r5, ccsrbase_low@ha
	lwz	r4, ccsrbase_low@l(r5)
	lis	r5, ccsrbase_high@ha
	lwz	r3, ccsrbase_high@l(r5)

	/* Use TLB1[15] to map the CCSR at 0xf0000000 */
	lis	r5, 0x100f
	mtspr	SPRN_MAS0, r5
	lis	r5, 0xc000
	ori	r5, r5, 0x0500
	mtspr	SPRN_MAS1, r5
	lis	r5, 0xf000
	ori	r5, r5, 0x000a
	mtspr	SPRN_MAS2, r5
	rlwinm	r5, r4, 0, 0xfffff000
	ori	r5, r5, 0x0005
	mtspr	SPRN_MAS3, r5
	mtspr	SPRN_MAS7, r3
	isync
	tlbwe
	isync

	lis	r3, 0xf002	/* L2 cache controller at CCSR+0x20000 */
	bl	invalidate_enable_L2

	/* Access the MEM(r10) with TLB1[0] */
	lis	r10, mpc85xx_sleep_save_area@h
	ori	r10, r10, mpc85xx_sleep_save_area@l

	lis	r3, 0xf000
	lwz	r4, SS_BPTR(r10)
	stw	r4, 0x20(r3)		/* restore BPTR */

	/* Program shift running space to PAGE_OFFSET */
	mfmsr	r3
	lis	r4, 1f@h
	ori	r4, r4, 1f@l

	mtsrr1	r3
	mtsrr0	r4
	rfi

1:	/* Restore the rest of TLB1, in ascending order so that
	 * the TLB1[1] gets invalidated first.
	 *
	 * XXX: It's better to invalidate the temporary mapping
	 * TLB1[15] for CCSR before restore any TLB1 entry include 0.
	 */
	lis	r4, 0x100f
	mtspr	SPRN_MAS0, r4
	lis	r4, 0
	mtspr	SPRN_MAS1, r4
	isync
	tlbwe
	isync

	lis	r3, (TLBCAM + 5*4 - 4)@h
	ori	r3, r3, (TLBCAM + 5*4 - 4)@l
	li	r4, 15
	mtctr	r4

2:
	lwz	r5, 4(r3)
	lwz	r6, 8(r3)
	lwz	r7, 12(r3)
	lwz	r8, 16(r3)
	lwzu	r9, 20(r3)

	mtspr	SPRN_MAS0, r5
	mtspr	SPRN_MAS1, r6
	mtspr	SPRN_MAS2, r7
	mtspr	SPRN_MAS3, r8
	mtspr	SPRN_MAS7, r9

	isync
	tlbwe
	isync
	bdnz	2b

	lis	r10, mpc85xx_sleep_save_area@h
	ori	r10, r10, mpc85xx_sleep_save_area@l

	lwz	r5, SS_HID+0(r10)
	lwz	r6, SS_HID+4(r10)

	isync
	mtspr	SPRN_HID0, r5
	isync

	msync
	mtspr	SPRN_HID1, r6
	isync

	lwz	r4, SS_IAC+0(r10)
	lwz	r5, SS_IAC+4(r10)
	lwz	r6, SS_DAC+0(r10)
	lwz	r7, SS_DAC+4(r10)

	mtspr	SPRN_IAC1, r4
	mtspr	SPRN_IAC2, r5
	mtspr	SPRN_DAC1, r6
	mtspr	SPRN_DAC2, r7

	lwz	r4, SS_DBCR+0(r10)
	lwz	r5, SS_DBCR+4(r10)
	lwz	r6, SS_DBCR+8(r10)

	mtspr	SPRN_DBCR0, r4
	mtspr	SPRN_DBCR1, r5
	mtspr	SPRN_DBCR2, r6

	lwz	r4, SS_PID+0(r10)
	lwz	r5, SS_PID+4(r10)
	lwz	r6, SS_PID+8(r10)

	mtspr	SPRN_PID0, r4
	mtspr	SPRN_PID1, r5
	mtspr	SPRN_PID2, r6

	lwz	r4, SS_SPRG+0x00(r10)
	lwz	r5, SS_SPRG+0x04(r10)
	lwz	r6, SS_SPRG+0x08(r10)
	lwz	r7, SS_SPRG+0x0c(r10)

	mtspr	SPRN_SPRG0, r4
	mtspr	SPRN_SPRG1, r5
	mtspr	SPRN_SPRG2, r6
	mtspr	SPRN_SPRG3, r7

	lwz	r4, SS_SPRG+0x10(r10)
	lwz	r5, SS_SPRG+0x14(r10)
	lwz	r6, SS_SPRG+0x18(r10)
	lwz	r7, SS_SPRG+0x1c(r10)

	mtspr	SPRN_SPRG4, r4
	mtspr	SPRN_SPRG5, r5
	mtspr	SPRN_SPRG6, r6
	mtspr	SPRN_SPRG7, r7

	lwz	r4, SS_IVPR(r10)
	mtspr	SPRN_IVPR, r4

	lwz	r4, SS_IVOR+0x00(r10)
	lwz	r5, SS_IVOR+0x04(r10)
	lwz	r6, SS_IVOR+0x08(r10)
	lwz	r7, SS_IVOR+0x0c(r10)

	mtspr	SPRN_IVOR0, r4
	mtspr	SPRN_IVOR1, r5
	mtspr	SPRN_IVOR2, r6
	mtspr	SPRN_IVOR3, r7

	lwz	r4, SS_IVOR+0x10(r10)
	lwz	r5, SS_IVOR+0x14(r10)
	lwz	r6, SS_IVOR+0x18(r10)
	lwz	r7, SS_IVOR+0x1c(r10)

	mtspr	SPRN_IVOR4, r4
	mtspr	SPRN_IVOR5, r5
	mtspr	SPRN_IVOR6, r6
	mtspr	SPRN_IVOR7, r7

	lwz	r4, SS_IVOR+0x20(r10)
	lwz	r5, SS_IVOR+0x24(r10)
	lwz	r6, SS_IVOR+0x28(r10)
	lwz	r7, SS_IVOR+0x2c(r10)

	mtspr	SPRN_IVOR8, r4
	mtspr	SPRN_IVOR9, r5
	mtspr	SPRN_IVOR10, r6
	mtspr	SPRN_IVOR11, r7

	lwz	r4, SS_IVOR+0x30(r10)
	lwz	r5, SS_IVOR+0x34(r10)
	lwz	r6, SS_IVOR+0x38(r10)
	lwz	r7, SS_IVOR+0x3c(r10)

	mtspr	SPRN_IVOR12, r4
	mtspr	SPRN_IVOR13, r5
	mtspr	SPRN_IVOR14, r6
	mtspr	SPRN_IVOR15, r7

	lwz	r4, SS_IVOR+0x40(r10)
	lwz	r5, SS_IVOR+0x44(r10)
	lwz	r6, SS_IVOR+0x48(r10)
	lwz	r7, SS_IVOR+0x4c(r10)

	mtspr	SPRN_IVOR32, r4
	mtspr	SPRN_IVOR33, r5
	mtspr	SPRN_IVOR34, r6
	mtspr	SPRN_IVOR35, r7

	lwz	r4, SS_TCR(r10)
	lwz	r5, SS_BUCSR(r10)
	lwz	r6, SS_L1CSR+0(r10)
	lwz	r7, SS_L1CSR+4(r10)
	lwz	r8, SS_USPRG+0(r10)

	mtspr	SPRN_TCR, r4
	mtspr	SPRN_BUCSR, r5

	msync
	isync
	mtspr	SPRN_L1CSR0, r6
	isync

	mtspr	SPRN_L1CSR1, r7
	isync

	mtspr	SPRN_USPRG0, r8

	lmw	r12, SS_GPREG(r10)

	lwz	r1, SS_SP(r10)
	lwz	r2, SS_CURRENT(r10)
	lwz	r4, SS_MSR(r10)
	lwz	r5, SS_LR(r10)
	lwz	r6, SS_CR(r10)

	msync
	mtmsr	r4
	isync

	mtlr	r5
	mtcr	r6

	li	r4, 0
	mtspr	SPRN_TBWL, r4

	lwz	r4, SS_TB+0(r10)
	lwz	r5, SS_TB+4(r10)

	mtspr	SPRN_TBWU, r4
	mtspr	SPRN_TBWL, r5

	lis	r3, 1
	mtdec	r3

	blr
#endif


#define do_save_gpr_reg(reg, addr) \
	PPC_STL reg, addr(r10)

#define do_restore_gpr_reg(reg, addr) \
	PPC_LL reg, addr(r10)

#define do_save_fpr_reg(reg, addr) \
	stfd	reg, addr(r10)

#define do_restore_fpr_reg(reg, addr) \
	lfd	reg, addr(r10)

#define do_save_spr_reg(reg, addr) \
	mfspr	r0, SPRN_##reg ;\
	PPC_STL r0, addr(r10)

#define do_restore_spr_reg(reg, addr) \
	PPC_LL r0, addr(r10) ;\
	mtspr	SPRN_##reg, r0

#define do_save_special_reg(special, addr) \
	mf##special	r0 ;\
	PPC_STL r0, addr(r10)

#define do_restore_special_reg(special, addr) \
	PPC_LL r0, addr(r10) ;\
	mt##special	r0

#define do_sr_general_gpr_regs(func) \
	do_##func##_gpr_reg(r1, SR_GPR1) ;\
	do_##func##_gpr_reg(r2, SR_GPR2) ;\
	do_##func##_gpr_reg(r13, SR_GPR13 + 0x00) ;\
	do_##func##_gpr_reg(r14, SR_GPR13 + 0x08) ;\
	do_##func##_gpr_reg(r15, SR_GPR13 + 0x10) ;\
	do_##func##_gpr_reg(r16, SR_GPR13 + 0x18) ;\
	do_##func##_gpr_reg(r17, SR_GPR13 + 0x20) ;\
	do_##func##_gpr_reg(r18, SR_GPR13 + 0x28) ;\
	do_##func##_gpr_reg(r19, SR_GPR13 + 0x30) ;\
	do_##func##_gpr_reg(r20, SR_GPR13 + 0x38) ;\
	do_##func##_gpr_reg(r21, SR_GPR13 + 0x40) ;\
	do_##func##_gpr_reg(r22, SR_GPR13 + 0x48) ;\
	do_##func##_gpr_reg(r23, SR_GPR13 + 0x50) ;\
	do_##func##_gpr_reg(r24, SR_GPR13 + 0x58) ;\
	do_##func##_gpr_reg(r25, SR_GPR13 + 0x60) ;\
	do_##func##_gpr_reg(r26, SR_GPR13 + 0x68) ;\
	do_##func##_gpr_reg(r27, SR_GPR13 + 0x70) ;\
	do_##func##_gpr_reg(r28, SR_GPR13 + 0x78) ;\
	do_##func##_gpr_reg(r29, SR_GPR13 + 0x80) ;\
	do_##func##_gpr_reg(r30, SR_GPR13 + 0x88) ;\
	do_##func##_gpr_reg(r31, SR_GPR13 + 0x90)

#define do_sr_fpr_regs(func) \
	do_##func##_fpr_reg(fr14, SR_FPR14 + 0x00) ;\
	do_##func##_fpr_reg(fr15, SR_FPR14 + 0x08) ;\
	do_##func##_fpr_reg(fr16, SR_FPR14 + 0x10) ;\
	do_##func##_fpr_reg(fr17, SR_FPR14 + 0x18) ;\
	do_##func##_fpr_reg(fr18, SR_FPR14 + 0x20) ;\
	do_##func##_fpr_reg(fr19, SR_FPR14 + 0x28) ;\
	do_##func##_fpr_reg(fr20, SR_FPR14 + 0x30) ;\
	do_##func##_fpr_reg(fr21, SR_FPR14 + 0x38) ;\
	do_##func##_fpr_reg(fr22, SR_FPR14 + 0x40) ;\
	do_##func##_fpr_reg(fr23, SR_FPR14 + 0x48) ;\
	do_##func##_fpr_reg(fr24, SR_FPR14 + 0x50) ;\
	do_##func##_fpr_reg(fr25, SR_FPR14 + 0x58) ;\
	do_##func##_fpr_reg(fr26, SR_FPR14 + 0x60) ;\
	do_##func##_fpr_reg(fr27, SR_FPR14 + 0x68) ;\
	do_##func##_fpr_reg(fr28, SR_FPR14 + 0x70) ;\
	do_##func##_fpr_reg(fr29, SR_FPR14 + 0x78) ;\
	do_##func##_fpr_reg(fr30, SR_FPR14 + 0x80) ;\
	do_##func##_fpr_reg(fr31, SR_FPR14 + 0x88)

#define do_sr_general_branch_regs(func) \
	do_##func##_special_reg(CR, SR_CR)

#define do_sr_general_pcr_regs(func) \
	do_##func##_spr_reg(EPCR, SR_EPCR) ;\
	do_##func##_spr_reg(HID0, SR_HID0 + 0x00)

#define do_sr_e500_pcr_regs(func) \
	do_##func##_spr_reg(HID1, SR_HID0 + 0x08)

#define do_sr_save_tb_regs \
	do_save_spr_reg(TBRU, SR_TBU) ;\
	do_save_spr_reg(TBRL, SR_TBL)

#define do_sr_restore_tb_regs \
	do_restore_spr_reg(TBWU, SR_TBU) ;\
	do_restore_spr_reg(TBWL, SR_TBL)

#define do_sr_general_time_regs(func) \
	do_sr_##func##_tb_regs ;\
	do_##func##_spr_reg(TCR, SR_TCR)

#define do_sr_interrupt_regs(func) \
	do_##func##_spr_reg(IVPR, SR_IVPR) ;\
	do_##func##_spr_reg(IVOR0, SR_IVOR0 + 0x00) ;\
	do_##func##_spr_reg(IVOR1, SR_IVOR0 + 0x08) ;\
	do_##func##_spr_reg(IVOR2, SR_IVOR0 + 0x10) ;\
	do_##func##_spr_reg(IVOR3, SR_IVOR0 + 0x18) ;\
	do_##func##_spr_reg(IVOR4, SR_IVOR0 + 0x20) ;\
	do_##func##_spr_reg(IVOR5, SR_IVOR0 + 0x28) ;\
	do_##func##_spr_reg(IVOR6, SR_IVOR0 + 0x30) ;\
	do_##func##_spr_reg(IVOR7, SR_IVOR0 + 0x38) ;\
	do_##func##_spr_reg(IVOR8, SR_IVOR0 + 0x40) ;\
	do_##func##_spr_reg(IVOR10, SR_IVOR0 + 0x50) ;\
	do_##func##_spr_reg(IVOR11, SR_IVOR0 + 0x58) ;\
	do_##func##_spr_reg(IVOR12, SR_IVOR0 + 0x60) ;\
	do_##func##_spr_reg(IVOR13, SR_IVOR0 + 0x68) ;\
	do_##func##_spr_reg(IVOR14, SR_IVOR0 + 0x70) ;\
	do_##func##_spr_reg(IVOR15, SR_IVOR0 + 0x78)

#define do_e6500_sr_interrupt_regs(func) \
	do_##func##_spr_reg(IVOR9, SR_IVOR0 + 0x48) ;\
	do_##func##_spr_reg(IVOR32, SR_IVOR32 + 0x00) ;\
	do_##func##_spr_reg(IVOR33, SR_IVOR32 + 0x08) ;\
	do_##func##_spr_reg(IVOR35, SR_IVOR32 + 0x18) ;\
	do_##func##_spr_reg(IVOR36, SR_IVOR32 + 0x20) ;\
	do_##func##_spr_reg(IVOR37, SR_IVOR32 + 0x28) ;\
	do_##func##_spr_reg(IVOR38, SR_IVOR32 + 0x30) ;\
	do_##func##_spr_reg(IVOR39, SR_IVOR32 + 0x38) ;\
	do_##func##_spr_reg(IVOR40, SR_IVOR32 + 0x40) ;\
	do_##func##_spr_reg(IVOR41, SR_IVOR32 + 0x48)

#define do_e5500_sr_interrupt_regs(func) \
	do_##func##_spr_reg(IVOR9, SR_IVOR0 + 0x48) ;\
	do_##func##_spr_reg(IVOR35, SR_IVOR32 + 0x18) ;\
	do_##func##_spr_reg(IVOR36, SR_IVOR32 + 0x20) ;\
	do_##func##_spr_reg(IVOR37, SR_IVOR32 + 0x28) ;\
	do_##func##_spr_reg(IVOR38, SR_IVOR32 + 0x30) ;\
	do_##func##_spr_reg(IVOR39, SR_IVOR32 + 0x38) ;\
	do_##func##_spr_reg(IVOR40, SR_IVOR32 + 0x40) ;\
	do_##func##_spr_reg(IVOR41, SR_IVOR32 + 0x48)

#define do_e500_sr_interrupt_regs(func) \
	do_##func##_spr_reg(IVOR32, SR_IVOR32 + 0x00) ;\
	do_##func##_spr_reg(IVOR33, SR_IVOR32 + 0x08) ;\
	do_##func##_spr_reg(IVOR34, SR_IVOR32 + 0x10)

#define do_e500mc_sr_interrupt_regs(func) \
	do_##func##_spr_reg(IVOR9, SR_IVOR0 + 0x48) ;\
	do_##func##_spr_reg(IVOR35, SR_IVOR32 + 0x18) ;\
	do_##func##_spr_reg(IVOR36, SR_IVOR32 + 0x20) ;\
	do_##func##_spr_reg(IVOR37, SR_IVOR32 + 0x28) ;\
	do_##func##_spr_reg(IVOR38, SR_IVOR32 + 0x30) ;\
	do_##func##_spr_reg(IVOR39, SR_IVOR32 + 0x38) ;\
	do_##func##_spr_reg(IVOR40, SR_IVOR32 + 0x40) ;\
	do_##func##_spr_reg(IVOR41, SR_IVOR32 + 0x48)

#define do_sr_general_software_regs(func) \
	do_##func##_spr_reg(SPRG1, SR_SPRG1) ;\
	do_##func##_spr_reg(SPRG3, SR_SPRG3)

#define do_sr_general_mmu_regs(func) \
	do_##func##_spr_reg(PID0, SR_PID0 + 0x00)

#define do_sr_e500_mmu_regs(func) \
	do_##func##_spr_reg(PID1, SR_PID0 + 0x08) ;\
	do_##func##_spr_reg(PID2, SR_PID0 + 0x10)

#define do_sr_debug_regs(func) \
	do_##func##_spr_reg(DBCR0, SR_DBCR0 + 0x00) ;\
	do_##func##_spr_reg(DBCR1, SR_DBCR0 + 0x08) ;\
	do_##func##_spr_reg(DBCR2, SR_DBCR0 + 0x10) ;\
	do_##func##_spr_reg(IAC1, SR_IAC1 + 0x00) ;\
	do_##func##_spr_reg(IAC2, SR_IAC1 + 0x08) ;\
	do_##func##_spr_reg(DAC1, SR_DAC1 + 0x00) ;\
	do_##func##_spr_reg(DAC2, SR_DAC1 + 0x08)

#define do_e6500_sr_debug_regs(func) \
	do_##func##_spr_reg(IAC3, SR_IAC1 + 0x10) ;\
	do_##func##_spr_reg(IAC4, SR_IAC1 + 0x18)

_GLOBAL(fsl_cpu_base_save)
	do_sr_general_gpr_regs(save)
	do_sr_general_branch_regs(save)
	do_sr_general_pcr_regs(save)
	do_sr_general_software_regs(save)
	do_sr_general_mmu_regs(save)

1:
	mfspr	r5, SPRN_TBRU
	do_sr_general_time_regs(save)
	mfspr	r6, SPRN_TBRU
	cmpw	r5, r6
	bne	1b

	blr

_GLOBAL(fsl_cpu_base_restore)
	do_sr_general_gpr_regs(restore)
	do_sr_general_branch_regs(restore)
	do_sr_general_pcr_regs(restore)
	do_sr_general_software_regs(restore)
	do_sr_general_mmu_regs(restore)

	isync

1:
	/* Restore Time registers */
	/* clear tb lower to avoid wrap */
	li	r0, 0
	mtspr	SPRN_TBWL, r0
	do_sr_general_time_regs(restore)

	/* clear out status */
	mfspr   r0,SPRN_TSR
	mtspr   SPRN_TSR, r0

	blr

/* Base registers, e500v1, e500v2 need to do some special save/restore */
_GLOBAL(e500_base_special_save)
	lis	r12, 0
	ori	r12, r12, PVR_VER_E500V1@l
	cmpw	r11, r12
	beq	500f

	lis	r12, 0
	ori	r12, r12, PVR_VER_E500V2@l
	cmpw	r11, r12
	bne	1f

500:
	do_sr_e500_pcr_regs(save)
	do_sr_e500_mmu_regs(save)
1:
	blr

_GLOBAL(e500_base_special_restore)
	lis	r12, 0
	ori	r12, r12, PVR_VER_E500V1@l
	cmpw	r11, r12
	beq	500f

	lis	r12, 0
	ori	r12, r12, PVR_VER_E500V2@l
	cmpw	r11, r12
	bne	1f

500:
	do_sr_e500_pcr_regs(save)
	do_sr_e500_mmu_regs(save)
1:
	blr

_GLOBAL(fsl_cpu_append_save)
	mfspr	r0, SPRN_PVR
	rlwinm	r11, r0, 16, 16, 31

	lis	r12, 0
	ori	r12, r12, PVR_VER_E6500@l
	cmpw	r11, r12
	beq	e6500_append_save

	lis	r12, 0
	ori	r12, r12, PVR_VER_E5500@l
	cmpw	r11, r12
	beq	e5500_append_save

	lis	r12, 0
	ori	r12, r12, PVR_VER_E500MC@l
	cmpw	r11, r12
	beq	e500mc_append_save

	lis	r12, 0
	ori	r12, r12, PVR_VER_E500V2@l
	cmpw	r11, r12
	beq	e500v2_append_save

	lis	r12, 0
	ori	r12, r12, PVR_VER_E500V1@l
	cmpw	r11, r12
	beq	e500v1_append_save

	b	1f

e6500_append_save:
	do_e6500_sr_interrupt_regs(save)
	do_e6500_sr_debug_regs(save)
	b	1f

e5500_append_save:
	do_e5500_sr_interrupt_regs(save)
	b	1f

e500mc_append_save:
	do_e500mc_sr_interrupt_regs(save)
	b	1f

e500v2_append_save:
e500v1_append_save:
	do_e500_sr_interrupt_regs(save)

1:
	do_sr_interrupt_regs(save)
	do_sr_debug_regs(save)

	blr

_GLOBAL(fsl_cpu_append_restore)
	mfspr	r0, SPRN_PVR
	rlwinm	r11, r0, 16, 16, 31

	lis	r12, 0
	ori	r12, r12, PVR_VER_E6500@l
	cmpw	r11, r12
	beq	e6500_append_restore

	lis	r12, 0
	ori	r12, r12, PVR_VER_E5500@l
	cmpw	r11, r12
	beq	e5500_append_restore

	lis	r12, 0
	ori	r12, r12, PVR_VER_E500MC@l
	cmpw	r11, r12
	beq	e500mc_append_restore

	lis	r12, 0
	ori	r12, r12, PVR_VER_E500V2@l
	cmpw	r11, r12
	beq	e500v2_append_restore

	lis	r12, 0
	ori	r12, r12, PVR_VER_E500V1@l
	cmpw	r11, r12
	beq	e500v1_append_restore

	b	1f

e6500_append_restore:
	do_e6500_sr_interrupt_regs(restore)
	do_e6500_sr_debug_regs(restore)
	b	1f

e5500_append_restore:
	do_e5500_sr_interrupt_regs(restore)
	b	1f

e500mc_append_restore:
	do_e500mc_sr_interrupt_regs(restore)
	b	1f

e500v2_append_restore:
e500v1_append_restore:
	do_e500_sr_interrupt_regs(restore)

1:
	do_sr_interrupt_regs(restore)
	do_sr_debug_regs(restore)

	sync

	blr

/*
 * r3 = the virtual address of buffer
 * r4 = core save type, 0-BASE_SAVE, 1-ALL_SAVE
 * r5 = the return address for resume
 */
_GLOBAL(booke_cpu_state_save)
	mr	r10, r3
	mflr	r9

	do_save_gpr_reg(r5, SR_LR)
	do_save_special_reg(msr, SR_MSR)

	/* if core_save_type is BASE_SAVE, goto 1f */
	cmpwi	r4, 0
	beq	1f

	bl	fsl_cpu_append_save

1:
	bl	e500_base_special_save

	bl	fsl_cpu_base_save

	mtlr	r9
	blr

/*
 * r3 = the virtual address of buffer
 * r4 = core save type, 0-BASE_SAVE, 1-ALL_SAVE
 * return:
 *   r3 = saved return address
 *   r4 = save MSR value
 */
_GLOBAL(booke_cpu_state_restore)
	mr	r10, r3
	mflr	r9

	/* if core_save_type is BASE_SAVE, goto 1f */
	cmpwi	r4, 0
	beq	1f

	bl	fsl_cpu_append_restore

1:
	bl	e500_base_special_restore

	bl	fsl_cpu_base_restore

	/* return MSR */
	PPC_LL r4, SR_MSR(r10)

	/* return the saved lr register value */
	do_restore_gpr_reg(r3, SR_LR)

	mtlr	r9
	blr
