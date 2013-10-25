/*
 * Freescale 85xx Core registers set
 *
 * Author: Wang Dongsheng <dongsheng.wang@freescale.com>
 *
 * Copyright 2013-2014 Freescale Semiconductor Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __ASM_FSL_SLEEP_H
#define __ASM_FSL_SLEEP_H

/*
 * Core register map definition
 * Address base on r3, we need to compatible with both 32-bit and 64-bit, so
 * the data width is 64-bit(double word).
 *
 * Acronyms:
 *	dw(data width)	0x08
 *
 * Map:
 * General-Purpose Registers
 *	GPR1(sp)		0
 *	GPR2			0x8		(dw * 1)
 *	GPR13 - GPR31		0x10 ~ 0xa0	(dw * 2 ~ dw * 20)
 * Foating-point registers
 *	FPR14 - FPR31		0xa8 ~ 0x130	(dw * 21 ~ dw * 38)
 * Registers for Branch Operations
 *	CR			0x138		(dw * 39)
 *	LR			0x140		(dw * 40)
 * Processor Control Registers
 *	MSR			0x148		(dw * 41)
 *	EPCR			0x150		(dw * 42)
 *
 *	Only e500, e500v2 need to save HID0 - HID1
 *	HID0 - HID1		0x158 ~ 0x160 (dw * 43 ~ dw * 44)
 * Timer Registers
 *	TCR			0x168		(dw * 45)
 *	TB(64bit)		0x170		(dw * 46)
 *	TBU(32bit)		0x178		(dw * 47)
 *	TBL(32bit)		0x180		(dw * 48)
 * Interrupt Registers
 *	IVPR			0x188		(dw * 49)
 *	IVOR0 - IVOR15		0x190 ~ 0x208	(dw * 50 ~ dw * 65)
 *	IVOR32 - IVOR41		0x210 ~ 0x258	(dw * 66 ~ dw * 75)
 * Software-Use Registers
 *	SPRG1			0x260		(dw * 76), 64-bit need to save.
 *	SPRG3			0x268		(dw * 77), 32-bit need to save.
 * MMU Registers
 *	PID0 - PID2		0x270 ~ 0x280	(dw * 78 ~ dw * 80)
 * Debug Registers
 *	DBCR0 - DBCR2		0x288 ~ 0x298	(dw * 81 ~ dw * 83)
 *	IAC1 - IAC4		0x2a0 ~ 0x2b8	(dw * 84 ~ dw * 87)
 *	DAC1 - DAC2		0x2c0 ~ 0x2c8	(dw * 88 ~ dw * 89)
 *
 */

#define SR_GPR1			0x000
#define SR_GPR2			0x008
#define SR_GPR13		0x010
#define SR_FPR14		0x0a8
#define SR_CR			0x138
#define SR_LR			0x140
#define SR_MSR			0x148
#define SR_EPCR			0x150
#define SR_HID0			0x158
#define SR_TCR			0x168
#define SR_TB			0x170
#define SR_TBU			0x178
#define SR_TBL			0x180
#define SR_IVPR			0x188
#define SR_IVOR0		0x190
#define SR_IVOR32		0x210
#define SR_SPRG1		0x260
#define SR_SPRG3		0x268
#define SR_PID0			0x270
#define SR_DBCR0		0x288
#define SR_IAC1			0x2a0
#define SR_DAC1			0x2c0
#define FSL_CPU_SR_SIZE		(SR_DAC1 + 0x10)

#ifndef __ASSEMBLY__

enum core_save_type {
	BASE_SAVE = 0,
	ALL_SAVE = 1,
};

extern int booke_cpu_state_save(void *buf, enum core_save_type type);
extern void *booke_cpu_state_restore(void *buf, enum core_save_type type);

#endif

#endif
