/*
 * Freescale deep sleep FSM (finite-state machine) configuration
 *
 * Copyright 2013 Freescale Semiconductor Inc.
 *
 * Author: Hongbo Zhang <hongbo.zhang@freescale.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */

#include <linux/io.h>
#include <linux/types.h>

#define FSL_STRIDE_4B	4
#define FSL_STRIDE_8B	8

/* Event Processor Global Control Register */
#define	EPGCR		0x000

/* Event Processor EVT Pin Control Registers */
#define	EPEVTCR0	0x050
#define	EPEVTCR1	0x054
#define	EPEVTCR2	0x058
#define	EPEVTCR3	0x05C
#define	EPEVTCR4	0x060
#define	EPEVTCR5	0x064
#define	EPEVTCR6	0x068
#define	EPEVTCR7	0x06C
#define	EPEVTCR8	0x070
#define	EPEVTCR9	0x074

/* Event Processor Crosstrigger Control Register */
#define	EPXTRIGCR	0x090

/* Event Processor Input Mux Control Registers */
#define	EPIMCR0		0x100
#define	EPIMCR1		0x104
#define	EPIMCR2		0x108
#define	EPIMCR3		0x10C
#define	EPIMCR4		0x110
#define	EPIMCR5		0x114
#define	EPIMCR6		0x118
#define	EPIMCR7		0x11C
#define	EPIMCR8		0x120
#define	EPIMCR9		0x124
#define	EPIMCR10	0x128
#define	EPIMCR11	0x12C
#define	EPIMCR12	0x130
#define	EPIMCR13	0x134
#define	EPIMCR14	0x138
#define	EPIMCR15	0x13C
#define	EPIMCR16	0x140
#define	EPIMCR17	0x144
#define	EPIMCR18	0x148
#define	EPIMCR19	0x14C
#define	EPIMCR20	0x150
#define	EPIMCR21	0x154
#define	EPIMCR22	0x158
#define	EPIMCR23	0x15C
#define	EPIMCR24	0x160
#define	EPIMCR25	0x164
#define	EPIMCR26	0x168
#define	EPIMCR27	0x16C
#define	EPIMCR28	0x170
#define	EPIMCR29	0x174
#define	EPIMCR30	0x178
#define	EPIMCR31	0x17C

/* Event Processor SCU Mux Control Registers */
#define	EPSMCR0		0x200
#define	EPSMCR1		0x208
#define	EPSMCR2		0x210
#define	EPSMCR3		0x218
#define	EPSMCR4		0x220
#define	EPSMCR5		0x228
#define	EPSMCR6		0x230
#define	EPSMCR7		0x238
#define	EPSMCR8		0x240
#define	EPSMCR9		0x248
#define	EPSMCR10	0x250
#define	EPSMCR11	0x258
#define	EPSMCR12	0x260
#define	EPSMCR13	0x268
#define	EPSMCR14	0x270
#define	EPSMCR15	0x278

/* Event Processor Event Control Registers */
#define	EPECR0		0x300
#define	EPECR1		0x304
#define	EPECR2		0x308
#define	EPECR3		0x30C
#define	EPECR4		0x310
#define	EPECR5		0x314
#define	EPECR6		0x318
#define	EPECR7		0x31C
#define	EPECR8		0x320
#define	EPECR9		0x324
#define	EPECR10		0x328
#define	EPECR11		0x32C
#define	EPECR12		0x330
#define	EPECR13		0x334
#define	EPECR14		0x338
#define	EPECR15		0x33C

/* Event Processor Action Control Registers */
#define	EPACR0		0x400
#define	EPACR1		0x404
#define	EPACR2		0x408
#define	EPACR3		0x40C
#define	EPACR4		0x410
#define	EPACR5		0x414
#define	EPACR6		0x418
#define	EPACR7		0x41C
#define	EPACR8		0x420
#define	EPACR9		0x424
#define	EPACR10		0x428
#define	EPACR11		0x42C
#define	EPACR12		0x430
#define	EPACR13		0x434
#define	EPACR14		0x438
#define	EPACR15		0x43C

/* Event Processor Counter Control Registers */
#define	EPCCR0		0x800
#define	EPCCR1		0x804
#define	EPCCR2		0x808
#define	EPCCR3		0x80C
#define	EPCCR4		0x810
#define	EPCCR5		0x814
#define	EPCCR6		0x818
#define	EPCCR7		0x81C
#define	EPCCR8		0x820
#define	EPCCR9		0x824
#define	EPCCR10		0x828
#define	EPCCR11		0x82C
#define	EPCCR12		0x830
#define	EPCCR13		0x834
#define	EPCCR14		0x838
#define	EPCCR15		0x83C

/* Event Processor Counter Compare Registers */
#define	EPCMPR0		0x900
#define	EPCMPR1		0x904
#define	EPCMPR2		0x908
#define	EPCMPR3		0x90C
#define	EPCMPR4		0x910
#define	EPCMPR5		0x914
#define	EPCMPR6		0x918
#define	EPCMPR7		0x91C
#define	EPCMPR8		0x920
#define	EPCMPR9		0x924
#define	EPCMPR10	0x928
#define	EPCMPR11	0x92C
#define	EPCMPR12	0x930
#define	EPCMPR13	0x934
#define	EPCMPR14	0x938
#define	EPCMPR15	0x93C

/* Event Processor Counter Register */
#define EPCTR0		0xA00
#define EPCTR1		0xA04
#define EPCTR2		0xA08
#define EPCTR3		0xA0C
#define EPCTR4		0xA10
#define EPCTR5		0xA14
#define EPCTR6		0xA18
#define EPCTR7		0xA1C
#define EPCTR8		0xA20
#define EPCTR9		0xA24
#define EPCTR10		0xA28
#define EPCTR11		0xA2C
#define EPCTR12		0xA30
#define EPCTR13		0xA34
#define EPCTR14		0xA38
#define EPCTR15		0xA3C
#define EPCTR16		0xA40
#define EPCTR17		0xA44
#define EPCTR18		0xA48
#define EPCTR19		0xA4C
#define EPCTR20		0xA50
#define EPCTR21		0xA54
#define EPCTR22		0xA58
#define EPCTR23		0xA5C
#define EPCTR24		0xA60
#define EPCTR25		0xA64
#define EPCTR26		0xA68
#define EPCTR27		0xA6C
#define EPCTR28		0xA70
#define EPCTR29		0xA74
#define EPCTR30		0xA78
#define EPCTR31		0xA7C

/* NPC triggered Memory-Mapped Access Registers */
#define	NCR		0x000
#define MCCR1		0x0CC
#define	MCSR1		0x0D0
#define	MMAR1LO		0x0D4
#define	MMAR1HI		0x0D8
#define	MMDR1		0x0DC
#define	MCSR2		0x0E0
#define	MMAR2LO		0x0E4
#define	MMAR2HI		0x0E8
#define	MMDR2		0x0EC
#define	MCSR3		0x0F0
#define	MMAR3LO		0x0F4
#define	MMAR3HI		0x0F8
#define	MMDR3		0x0FC

/* RCPM Core State Action Control Register 0 */
#define	CSTTACR0	0xB00

/* RCPM Core Group 1 Configuration Register 0 */
#define	CG1CR0		0x31C

/* Block offsets */
#define	RCPM_BLOCK_OFFSET	0x00022000
#define	EPU_BLOCK_OFFSET	0x00000000
#define	NPC_BLOCK_OFFSET	0x00001000

static void *g_dcsr_base;

static inline void rcpm_write(u32 offset, u32 val)
{
	out_be32(g_dcsr_base + RCPM_BLOCK_OFFSET + offset, val);
}

static inline void epu_write(u32 offset, u32 val)
{
	out_be32(g_dcsr_base + EPU_BLOCK_OFFSET + offset, val);
}

static inline void npc_write(u32 offset, u32 val)
{
	out_be32(g_dcsr_base + NPC_BLOCK_OFFSET + offset, val);
}

/**
 * fsl_dp_fsm_setup - Configure EPU's FSM
 * @dcsr_base: the base address of DCSR registers
 */
void fsl_dp_fsm_setup(void *dcsr_base)
{
	u32 offset;

	/*
	 * Globle static variable is safe here, becsuse this function is only
	 * called once at the last stage of suspend, when there is only one CPU
	 * running and task switching is also disabled.
	 */
	g_dcsr_base = dcsr_base;

	/* Disable All SCU Actions */
	for (offset = EPACR0; offset <= EPACR15; offset += FSL_STRIDE_4B)
		epu_write(offset, 0);

	/* Clear EPEVTCRn */
	for (offset = EPEVTCR0; offset <= EPEVTCR9; offset += FSL_STRIDE_4B)
		epu_write(offset, 0);

	/* Clear Event Processor Global Control Register */
	epu_write(EPGCR, 0);

	/* Clear EPSMCRn */
	for (offset = EPSMCR0; offset <= EPSMCR15; offset += FSL_STRIDE_8B)
		epu_write(offset, 0);

	/* Clear EPCCRn */
	for (offset = EPCCR0; offset <= EPCCR15; offset += FSL_STRIDE_4B)
		epu_write(offset, 0);

	/* Clear EPCMPRn */
	for (offset = EPCMPR0; offset <= EPCMPR15; offset += FSL_STRIDE_4B)
		epu_write(offset, 0);

	/*
	 * Clear EPCTRn
	 * Warm Device Reset does NOT reset these counter, so clear them
	 * explicitly. Or, the second time entering deep sleep will fail.
	 */
	for (offset = EPCTR0; offset <= EPCTR31; offset += FSL_STRIDE_4B)
		epu_write(offset, 0);

	/* Clear EPIMCRn */
	for (offset = EPIMCR0; offset <= EPIMCR31; offset += FSL_STRIDE_4B)
		epu_write(offset, 0);

	/* Clear EPXTRIGCRn */
	epu_write(EPXTRIGCR, 0);

	/* Disable all SCUs EPECRn */
	for (offset = EPECR0; offset <= EPECR15; offset += FSL_STRIDE_4B)
		epu_write(offset, 0);

	/* Set up the SCU chaining */
	epu_write(EPECR15, 0x00000004);
	epu_write(EPECR14, 0x02000084);
	epu_write(EPECR13, 0x08000084);
	epu_write(EPECR12, 0x80000084);
	epu_write(EPECR11, 0x90000084);
	epu_write(EPECR10, 0x42000084);
	epu_write(EPECR9, 0x08000084);
	epu_write(EPECR8, 0x60000084);
	epu_write(EPECR7, 0x80000084);
	epu_write(EPECR6, 0x80000084);
	epu_write(EPECR5, 0x08000004);
	epu_write(EPECR4, 0x20000084);
	epu_write(EPECR3, 0x80000084);
	epu_write(EPECR2, 0xF0004004);

	/* EVT Pin Configuration. SCU8 triger EVT2, and SCU11 triger EVT9 */
	epu_write(EPEVTCR2, 0x80000001);
	epu_write(EPEVTCR9, 0xB0000001);

	/* Configure the EPU Counter Values */
	epu_write(EPCMPR15, 0x000000FF);
	epu_write(EPCMPR14, 0x000000FF);
	epu_write(EPCMPR12, 0x000000FF);
	epu_write(EPCMPR11, 0x000000FF);
	epu_write(EPCMPR10, 0x000000FF);
	epu_write(EPCMPR9, 0x000000FF);
	epu_write(EPCMPR8, 0x000000FF);
	epu_write(EPCMPR5, 0x00000020);
	epu_write(EPCMPR4, 0x000000FF);
	epu_write(EPCMPR2, 0x000000FF);

	/* Configure the EPU Counters */
	epu_write(EPCCR15, 0x92840000);
	epu_write(EPCCR14, 0x92840000);
	epu_write(EPCCR12, 0x92840000);
	epu_write(EPCCR11, 0x92840000);
	epu_write(EPCCR10, 0x92840000);
	epu_write(EPCCR9, 0x92840000);
	epu_write(EPCCR8, 0x92840000);
	epu_write(EPCCR5, 0x92840000);
	epu_write(EPCCR4, 0x92840000);
	epu_write(EPCCR2, 0x92840000);

	/* Configure the SCUs Inputs */
	epu_write(EPSMCR15, 0x76000000);
	epu_write(EPSMCR14, 0x00000031);
	epu_write(EPSMCR13, 0x00003100);
	epu_write(EPSMCR12, 0x7F000000);
	epu_write(EPSMCR11, 0x31740000);
	epu_write(EPSMCR10, 0x65000030);
	epu_write(EPSMCR9, 0x00003000);
	epu_write(EPSMCR8, 0x64300000);
	epu_write(EPSMCR7, 0x30000000);
	epu_write(EPSMCR6, 0x7C000000);
	epu_write(EPSMCR5, 0x00002E00);
	epu_write(EPSMCR4, 0x002F0000);
	epu_write(EPSMCR3, 0x2F000000);
	epu_write(EPSMCR2, 0x6C700000);

	/* Configure the SCUs Actions */
	epu_write(EPACR15, 0x02000000);
	epu_write(EPACR14, 0x04000000);
	epu_write(EPACR13, 0x06000000);
	epu_write(EPACR12, 0x00000003);
	epu_write(EPACR10, 0x00000020);
	epu_write(EPACR9, 0x0000001C);
	epu_write(EPACR5, 0x00000040);
	epu_write(EPACR3, 0x00000080);

	/* Configure the SCUs and Timers Mux */
	epu_write(EPIMCR31, 0x76000000);
	epu_write(EPIMCR28, 0x76000000);
	epu_write(EPIMCR22, 0x6C000000);
	epu_write(EPIMCR20, 0x48000000);
	epu_write(EPIMCR16, 0x6A000000);
	epu_write(EPIMCR12, 0x44000000);
	epu_write(EPIMCR5, 0x40000000);
	epu_write(EPIMCR4, 0x44000000);

	/* Configure pulse or level triggers */
	epu_write(EPXTRIGCR, 0x0000FFDF);

	/* Configure the NPC tMMA registers*/
	npc_write(NCR, 0x80000000);
	npc_write(MCCR1, 0);
	npc_write(MCSR1, 0);
	npc_write(MMAR1LO, 0);
	npc_write(MMAR1HI, 0);
	npc_write(MMDR1, 0);
	npc_write(MCSR2, 0);
	npc_write(MMAR2LO, 0);
	npc_write(MMAR2HI, 0);
	npc_write(MMDR2, 0);
	npc_write(MCSR3, 0x80000000);
	npc_write(MMAR3LO, 0x000E2130);
	npc_write(MMAR3HI, 0x00030000);
	npc_write(MMDR3, 0x00020000);

	/* Configure RCPM for detecting Core0’s PH15 state */
	rcpm_write(CSTTACR0, 0x00001001);
	rcpm_write(CG1CR0, 0x00000001);

}

void fsl_dp_fsm_clean(void *dcsr_base)
{

	epu_write(EPEVTCR2, 0);
	epu_write(EPEVTCR9, 0);

	epu_write(EPGCR, 0);
	epu_write(EPECR15, 0);

	rcpm_write(CSTTACR0, 0);
	rcpm_write(CG1CR0, 0);

}
