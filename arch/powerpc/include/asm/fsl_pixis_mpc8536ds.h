/*
 * Freescale board control FPGA for MPC8536DS.
 *
 * Copyright (C) 2009, 2011 Freescale Semiconductor, Inc. All rights reserved.
 *
 * Author: Mingkai Hu <Mingkai.hu@freescale.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */
#ifndef __PPC_FSL_SOC_H
#define __PPC_FSL_SOC_H

#include <linux/types.h>

#define PIXIS_CMD_START		0x1
#define PIXIS_CMD_STOP		0x0
#define PIXIS_CMD_START_SLEEP	0x2

#define PIXIS_MEAS_CORE		0x1
#define PIXIS_MEAS_PLAT		0x2
#define PIXIS_MEAS_ENABLE	0x1
#define PIXIS_MEAS_RECEIVE	0x2
#define PIXIS_MEAS_DISABLE	0x4

#define PIXIS_I2CDACR_START		(1 << 7)
#define PIXIS_I2CDACR_IDLE		(1 << 6)
#define PIXIS_I2CDACR_VC_MEAS		(1 << 5)
#define PIXIS_I2CDACR_VP_MEAS		(1 << 4)
#define PIXIS_I2CDACR_T_MEAS		(1 << 3)
#define PIXIS_I2CDACR_VC_BUS(x)		((x) << 2)
#define PIXIS_I2CDACR_VC_BUS_MASK	(1 << 2)
#define PIXIS_I2CDACR_VP_BUS(x)		((x) << 1)
#define PIXIS_I2CDACR_VP_BUS_MASK	(1 << 1)
#define PIXIS_I2CDACR_T_BUS(x)		((x) << 0)
#define PIXIS_I2CDACR_T_BUS_MASK	(1 << 0)

#define PIXIS_I2CACC_BYTES    4
#define PIXIS_I2CCNT_BYTES    3
#define PIXIS_I2CMAX_BYTES    2

#define PIXIS_ADC_OUTPUT_RES      10      /* 10-bit resolution */
#define PIXIS_ADC_OUTPUT_SCALE    4
#define PIXIS_ADC_OUTPUT_MAX_VAL  ((1 << PIXIS_ADC_OUTPUT_RES) - 1)

struct pixis_reg {
	u8	id;		/* 0x00 - System ID register */
	u8	ver;		/* 0x01 - System version register */
	u8	pver;		/* 0x02 - FPGA version register */
	u8	csr;		/* 0x03 - General control/status register */
	u8	rst;		/* 0x04 - Reset control register */
	u8	rst2;		/* 0x05 - Reset control register2 */
	u8	aux;		/* 0x06 - Auxiliary register */
	u8	spd;		/* 0x07 - Speed register */
	u8	aux2;		/* 0x08 - Auxiliary 2 register */
	u8	csr2;		/* 0x09 - General control/status register2 */
	u8	watch;		/* 0x0A - WATCH Register */
	u8	led;		/* 0x0B - LED Register */
	u8	pwr;		/* 0x0C - Power status register*/
	u8	res1[3];
	u8	vctl;		/* 0x10 - VELA Control Register */
	u8	vstat;		/* 0x11 - VELA Status Register */
	u8	vcfgen0;	/* 0x12 - VELA Configuration Enable Reg 0 */
	u8	vcfgen1;	/* 0x13 - VELA Configuration Enable Reg 1 */
	u8	vcore0;		/* 0x14 - VCORE0 Register */
	u8	res2;
	u8	vboot;		/* 0x16 - VBOOT Register */
	u8	vspeed0;	/* 0x17 - VSPEED0 Register */
	u8	vspeed1;	/* 0x18 - VSPEED1 Register */
	u8	vspeed2;	/* 0x19 - VSPEED2 Register */
	u8	vsysclk0;	/* 0xiA - VELA SYSCLK0 Register */
	u8	vsysclk1;	/* 0x1B - VELA SYSCLK1 Register */
	u8	vsysclk2;	/* 0x1C - VELA SYSCLK2 Register */
	u8	vddrclk0;	/* 0x1D - VELA DDRCLK0 Register */
	u8	vddrclk1;	/* 0x1E - VELA DDRCLK1 Register */
	u8	vddrclk2;	/* 0x1F - VELA DDRCLK2 Register */
	u8	i2cdacr;	/* 0x20 - I2C Data Acquistion Control Reg */
	u8	vcoreacc[4];	/* 0x21 - 0x24 : Vcore Data Accumulator Reg */
	u8	vcorecnt[3];	/* 0x25 - 0x27 : Vcore Read Counter Reg */
	u8	vcoremax[2];	/* 0x28 - 0x29 : Vcore Maximum Value Reg */
	u8	vplatacc[4];	/* 0x2A - 0x2D : Vplat Data Accumulator Reg */
	u8	vplatcnt[3];	/* 0x2E - 0x30 : Vplat Read Counter Reg */
	u8	vplatmax[2];	/* 0x31 - 0x32 : Vplat Maximum Value Reg */
	u8	tempacc[4];	/* 0x33 - 0x36 : Temp Data Accumulator Reg */
	u8	tempcnt[3];	/* 0x37 - 0x39 : Temp Read Counter Reg */
	u8	tempmax[2];	/* 0x3A - 0x3B : Temp Maximum Value Reg */
} __attribute__ ((packed));

enum board_id {
	CALAMARI = 0x15,
};

/*
 * AMMETER -- holds current measurement circuit information
 *---------------------------------------------------------------------------
 *	NOTE:  this structure assumes the following circuit:
 *
 *	Vcore (+)  shuntR
 *	o----+-/\/\-+---> Icore
 *		 |      |             --- gnd
 *		 +-(A)--|              |
 *		  \gain/               \                (+) a2DVcc
 *		   \  /                /  btmRdiv        o
 *	            \/                 \ (optional)      |
 *		    |       topRdiv    |              +--+--+
 *                  +-------/\/\-------+--------------| A/D +---/---> I2C Bus
 *			   (optional)                 +-----+
 *
 * There are two ADC chips to monitor the core and platform current
 * individually. The current is converted to voltage through a shunt
 * register, and the voltage is gained by a current shunt monitor(INA196)
 * and then connected to ADC chips' analog input through a voltage-divider
 * network.
 *
 * To calculate Icore/Iplat for a given I2C reading, we must:
 * 1) calculate the voltage corresponding to the I2C data
 * 2) divide the I2C voltage by the (optional) voltage divider scale factor
 *	voltage divider scaling factor = btmRdiv / (topRdiv + btmRdiv)
 * 3) divide the scaled voltage reading by the ammeter gain
 * 4) calculate the current across the shunt resistor give the above voltage
 *    drop:
 *	Icore = (((a2dVcc / 1024) * I2C data) * (topRdiv + btmRdiv))
 *		/ (gain * btmRdiv * shuntR)
 *            = I2C data * (a2cVcc * (topRdiv + btmRdiv)
 *		/ gain * bitmRdiv * shuntR * 1024)
 *
 * We calculate the value of the later part of the formula and scale the value
 * by 1000, then represent as the factor number in the structure.
 */
struct ammeter {
	u8		board;		/* board type */
	u32		factor;		/* factor */
	u32		adc_vcc;	/* ADC input voltage, mV */
	u8		adc_addr;	/* ADC I2C slave address */
	u32		i2c_bus;	/* DUT I2C bus number */
};

/*
 * vid_to_volt -- holds the informatation form vid to voltage
 */
struct vid_to_volt {
	u8		board;
	u8		vid;
	u32		volt;
};

struct voltage {
	u8		board;		/* board type */
	u32		min;		/* minimum allowed voltage setting */
	u32		max;		/* maximum allowed voltage setting */
	u8		en_mask;	/* FPGA voltage VID enable mask */
	u8		data_mask;	/* FPGA voltage VID data mask */
};

/*
 * TEMPERATURE -- holds information required to read temperature diode
 *
 * To read the temperature diode, we must:
 *	1) initialize temperature diode monitor (offset, sample rate)
 *	2) read temperature diode monitor
 */
struct temperature {
	u8		board;		/* board type */
	u8		offset_reg;	/* TM offset register address */
	signed char	offset;		/* TM offset (2's complement) */
	u8		rate_reg;	/* TM sample rate register address */
	u8		rate;		/* TM sample rate */
	u8		en_reg;		/* TM enable register address */
	u8		en;		/* TM enable bit */
	u8		temp_reg;	/* TM temperature register address */
	u8		i2c_fdr;	/* i2c frequency divider */
	u8		i2c_addr;	/* i2c address of TM device */
	u32		i2c_bus;	/* DUT I2C bus number */
};

struct fsl_pixis {
	struct	pixis_reg __iomem *base;

	u32	pm_cmd;
	u32	v_core;
	u32	v_plat;
	u32	i_core;
	u32	i_plat;
	u32	temp;

	struct	mutex update_lock;
};
#endif
