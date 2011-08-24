/*
 * Freescale board control FPGA for MPC8536DS.
 *
 * Copyright (C) 2009, 2011 Freescale Semiconductor, Inc. All rights reserved.
 *
 * Author: Mingkai hu <Mingkai.hu@freescale.com>
 *
 * Based on the bare code wrote by Chris Pettinato (ra5171@freescale.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/i2c.h>
#include <asm/machdep.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/proc_fs.h>
#include <linux/vmalloc.h>
#include <asm/uaccess.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/seq_file.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <asm/fsl_pixis_mpc8536ds.h>

static struct proc_dir_entry *pixis_proc_root;
static struct proc_dir_entry *pixis_proc_pm_ctrl;
static struct fsl_pixis *pixis;

/*
 * acdb -- database of Icore ammeter circuit information
 */
static struct ammeter acdb[] = {
	/* board	factor	adc_vcc	adc_addr	i2c_bus */
	{CALAMARI,	6040,	3360,	0x4d,	1}
};

/*
 * apdb -- database of Iplat ammeter circuit information
 */
static struct ammeter apdb[] = {
	/* board	factor	adc_vcc	adc_addr	i2c_bus */
	{CALAMARI,	5500,	3360,	0x4d,	2}
};

/*
 * viddb -- database of vid to voltage transmition
 */
static struct vid_to_volt viddb[] = {
	{CALAMARI,	0x20,	1100},
	{CALAMARI,	0x28,	1000},
};

/*
 * vcdb -- database of Vcore logic information
 */
static struct voltage vcdb[] = {
	/* board	min	max	en_mask	data_mask */
	{CALAMARI,	1000,	1100,	0x80,	0x7f},
};

/*
 * vpdb -- database of Vplat logic information
 */
static struct voltage vpdb[] = {
	/* board	min	max	en_mask	data_mask */
	{CALAMARI,	1000,	1000,	0,	0},
};

/*
 * tdb -- database of temperature diode information
 */
static struct temperature tdb[] = {
	{CALAMARI,	0x11,	0x00,	0x0a,	0x0a,	0x09,	0x00,
			0x01,	0x0e,	0x4c,	2},
};

/*
 * ammcore_match -- match board to the AMMETER database.
 */
static struct ammeter *ammcore_match(struct fsl_pixis *pixis)
{
	int i;
	u8 board_id = in_8(&pixis->base->id);
	struct ammeter	*match = NULL;

	for (i = 0; i < sizeof(acdb) / sizeof(*match); i++) {
		if (acdb[i].board == board_id) {
			match = &acdb[i];
			break;
		}
	}

	return match;
}

/*
 * ammplat_match -- match board to the ammplat database.
 */
static struct ammeter *ammplat_match(struct fsl_pixis *pixis)
{
	int i;
	u8 board_id = in_8(&pixis->base->id);
	struct ammeter *match = NULL;

	for (i = 0; i < sizeof(apdb) / sizeof(*match); i++) {
		if (apdb[i].board == board_id) {
			match = &apdb[i];
			break;
		}
	}

	return match;
}

/*
 * vcore_match -- match board to the Vcore database.
 */
static u32 vid_match(struct fsl_pixis *pixis, u8 vid)
{
	int i;
	u32 volt = 0;
	u8 board_id = in_8(&pixis->base->id);

	for (i = 0; i < sizeof(viddb) / sizeof(struct vid_to_volt); i++) {
		if ((viddb[i].board == board_id) && (viddb[i].vid == vid)) {
			volt = viddb[i].volt;
			break;
		}
	}

	return volt;
}

/*
 * vcore_match -- match board to the Vcore database.
 */
static struct voltage *vcore_match(struct fsl_pixis *pixis)
{
	int i;
	u8 board_id = in_8(&pixis->base->id);
	struct voltage *match = NULL;

	for (i = 0; i < sizeof(vcdb) / sizeof(*match); i++) {
		if (vcdb[i].board == board_id) {
			match = &vcdb[i];
			break;
		}
	}

	return match;
}

/*
 * vplat_match -- match board to the Vplat database.
 */
static struct voltage *vplat_match(struct fsl_pixis *pixis)
{
	int i;
	u8 board_id = in_8(&pixis->base->id);
	struct voltage *match = NULL;

	for (i = 0; i < sizeof(vpdb) / sizeof(*match); i++) {
		if (vpdb[i].board == board_id) {
			match = &vpdb[i];
			break;
		}
	}

	return match;
}

/*
 * temperature_match -- match board to the struct temperature database.
 */
static struct temperature *temperature_match(struct fsl_pixis *pixis)
{
	int i;
	u8 board_id = in_8(&pixis->base->id);
	struct temperature *match = NULL;

	for (i = 0; i < sizeof(tdb) / sizeof(*match); i++) {
		if (tdb[i].board == board_id) {
			match = &tdb[i];
			break;
		}
	}

	return match;
}

static u32 pixis_pm_halt(void)
{
	u8 dacr;

	/* retrieve current status */
	dacr = in_8(&pixis->base->i2cdacr);
	if (!(dacr & PIXIS_I2CDACR_IDLE)) {
		/* halt any active measurements */
		dacr &= ~PIXIS_I2CDACR_START;
		out_8(&pixis->base->i2cdacr, dacr);
	}

	return 0;
}

/*
 * pixis_get_voltage -- read Vcore or Vplat voltage setting
 */
static u32 pixis_get_voltage(struct fsl_pixis *pixis, int type)
{
	u32 volt, *volt_addr = NULL;
	u8 vid_mask, vid_data;
	struct voltage *v_info = NULL;

	/* load voltage information for board */
	if (type == PIXIS_MEAS_CORE) {
		volt_addr = &pixis->v_core;
		v_info = vcore_match(pixis);
	} else if (type == PIXIS_MEAS_PLAT) {
		volt_addr = &pixis->v_plat;
		v_info = vplat_match(pixis);
	}
	if (v_info == NULL)
		return -ENXIO;

	mutex_lock(&pixis->update_lock);
	if (v_info->min == v_info->max) {
		/* voltage is not programmable */
		*volt_addr = v_info->min;
	} else {
		/* read vid setting */
		vid_data = in_8(&pixis->base->vcore0);
		vid_mask = v_info->data_mask;
		vid_data &= vid_mask;

		/* translate vid setting into voltage */
		volt = vid_match(pixis, vid_data);
		*volt_addr = volt;
	}
	mutex_unlock(&pixis->update_lock);

	return 0;
}

/*
 * The basic transfer function is volts = vdd * val / 1024, except
 * that at val's limits (0 and 1023), val should have 0.25 added to it.
 * We scale val by a factor of four so we can add one instead of 0.25.
 */
static inline u32 pixis_volts_from_adc(u32 vdd, u32 val)
{
	val = val * PIXIS_ADC_OUTPUT_SCALE;

	if ((val == 0) ||
		(val == PIXIS_ADC_OUTPUT_MAX_VAL * PIXIS_ADC_OUTPUT_SCALE))
		val++;

	return  val * vdd / ((1 << PIXIS_ADC_OUTPUT_RES)
					* PIXIS_ADC_OUTPUT_SCALE);
}

/*
 * pixis_get_current  --  measure instantaneous DUT Vcore or Vplat
 * current demand
 */
static u32 pixis_get_current(struct fsl_pixis *pixis, int type, int mode)
{
	int i;
	u32 status;
	struct ammeter *amm_info = NULL;
	u8 dacr, meas, bus, bus_mask, tmp;
	u8 *acc_addr, *cnt_addr;
	u32 adc_meas = 0, adc_cnt = 0;
	u32 adc_volt, *curr_addr;

	pr_debug("PIXIS: measure %s current(%d)\n",
		(type == PIXIS_MEAS_CORE) ? "core" : "platform", mode);
	/* load current measurement circuit information for board */
	if (type == PIXIS_MEAS_CORE) {
		amm_info = ammcore_match(pixis);
		meas = PIXIS_I2CDACR_VC_MEAS;
		bus = PIXIS_I2CDACR_VC_BUS((amm_info->i2c_bus - 1));
		acc_addr = pixis->base->vcoreacc;
		cnt_addr = pixis->base->vcorecnt;
		curr_addr = &pixis->i_core;
		bus_mask = PIXIS_I2CDACR_VC_BUS_MASK;
	} else if (type == PIXIS_MEAS_PLAT) {
		amm_info = ammplat_match(pixis);
		meas = PIXIS_I2CDACR_VP_MEAS;
		bus = PIXIS_I2CDACR_VP_BUS((amm_info->i2c_bus - 1));
		acc_addr = pixis->base->vplatacc;
		cnt_addr = pixis->base->vplatcnt;
		curr_addr = &pixis->i_plat;
		bus_mask = PIXIS_I2CDACR_VP_BUS_MASK;
	}
	if (amm_info == NULL)
		return -ENXIO;

	mutex_lock(&pixis->update_lock);

	switch (mode) {
	case PIXIS_MEAS_ENABLE:

		/* halt any active measurements */
		status = pixis_pm_halt();
		if (status) {
			mutex_lock(&pixis->update_lock);
			return status;
		}

		/* enable current measurement */
		dacr = in_8(&pixis->base->i2cdacr);
		dacr |= PIXIS_I2CDACR_START | meas | bus;

		/* begin i2c data acquisition */
		out_8(&pixis->base->i2cdacr, dacr);

		status = 0;
		break;
	case PIXIS_MEAS_RECEIVE:
		/* halt any active measurements */
		status = pixis_pm_halt();
		if (status) {
			mutex_lock(&pixis->update_lock);
			return status;
		}

		/* retrieve accumulator data from FPGA */
		for (i = 0; i < PIXIS_I2CACC_BYTES; i++) {
			tmp = in_8(&acc_addr[i]);
			adc_meas += (u32)tmp <<
					(((PIXIS_I2CACC_BYTES - 1) - i) * 8);
		}
		pr_debug("\tacc reg: 0x%x\n", adc_meas);

		/* retrieve count data from FPGA */
		for (i = 0; i < PIXIS_I2CCNT_BYTES; i++) {
			tmp = in_8(&cnt_addr[i]);
			adc_cnt += (u32)tmp <<
					(((PIXIS_I2CCNT_BYTES - 1) - i) * 8);
		}
		pr_debug("\tcnt reg: 0x%x\n", adc_cnt);
		if (adc_cnt < 1) {
			pr_err("PIXIS: FPGA did not collect data.\n");
			mutex_lock(&pixis->update_lock);
			return -EIO;
		}

		/* calculate the voltage from the I2C data, unit: mV */
		adc_volt = pixis_volts_from_adc(amm_info->adc_vcc,
						adc_meas / adc_cnt);

		/* calculate the current across voltage-divider network */
		*curr_addr = adc_volt * amm_info->factor / 1000;

		status = 0;
		break;
	case PIXIS_MEAS_DISABLE:
		/* halt any active measurements */
		status = pixis_pm_halt();
		if (status) {
			mutex_lock(&pixis->update_lock);
			return status;
		}

		/* enable current measurement */
		dacr = in_8(&pixis->base->i2cdacr);
		dacr &= ~meas & ~bus_mask;

		/* begin i2c data acquisition */
		out_8(&pixis->base->i2cdacr, dacr);

		status = 0;
		break;
	default:
		status = -EINVAL;
		break;
	}

	mutex_unlock(&pixis->update_lock);
	return status;
}

/*
 * pixis_get_temperature -- measure instantaneous DUT temperature
 */
static u32 pixis_get_temperature(struct fsl_pixis *pixis, int mode)
{
	int i = 0;
	u8 dacr, tmp;
	u32 status;
	u32 temp_meas = 0, temp_cnt = 0;
	struct temperature *temp_info = NULL;

	pr_debug("PIXIS: measure temperature(%d)\n", mode);
	/* load temperature information for board */
	temp_info = temperature_match(pixis);
	if (temp_info == NULL)
		return -ENXIO;

	mutex_lock(&pixis->update_lock);

	switch (mode) {
	case PIXIS_MEAS_ENABLE:
		/* halt any active measurements */
		status = pixis_pm_halt();
		if (status) {
			mutex_unlock(&pixis->update_lock);
			return status;
		}

		/* enable current measurement */
		dacr = in_8(&pixis->base->i2cdacr);
		dacr |= PIXIS_I2CDACR_START | PIXIS_I2CDACR_T_MEAS |
			PIXIS_I2CDACR_T_BUS((temp_info->i2c_bus - 1));

		/* begin i2c data acquisition */
		out_8(&pixis->base->i2cdacr, dacr);
		status = 0;
		break;
	case PIXIS_MEAS_RECEIVE:
		/* halt any active measurements */
		status = pixis_pm_halt();
		if (status) {
			mutex_unlock(&pixis->update_lock);
			return status;
		}

		/* retrieve accumulator data from FPGA */
		for (i = 0; i < PIXIS_I2CACC_BYTES; i++) {
			tmp = in_8(&pixis->base->tempacc[i]);
			temp_meas += (u32)tmp <<
					(((PIXIS_I2CACC_BYTES - 1) - i) * 8);
		}
		pr_debug("\tacc reg: 0x%x\n", temp_meas);

		/* retrieve count data from FPGA */
		for (i = 0; i < PIXIS_I2CCNT_BYTES; i++) {
			tmp = in_8(&pixis->base->tempcnt[i]);
			temp_cnt += (u32)tmp <<
					(((PIXIS_I2CCNT_BYTES - 1) - i) * 8);
		}
		pr_debug("\tcnt reg: 0x%x\n", temp_cnt);
		if (temp_cnt < 1) {
			pr_err("PIXIS: FPGA did not collect data.\n");
			mutex_unlock(&pixis->update_lock);
			return -EIO;
		}

		pixis->temp = temp_meas / temp_cnt;
		status = 0;
		break;
	case PIXIS_MEAS_DISABLE:
		/* halt any active measurements */
		status = pixis_pm_halt();
		if (status) {
			mutex_unlock(&pixis->update_lock);
			return status;
		}

		/* enable current measurement */
		dacr = in_8(&pixis->base->i2cdacr);
		dacr &= ~PIXIS_I2CDACR_T_MEAS & ~PIXIS_I2CDACR_T_BUS_MASK;

		/* begin i2c data acquisition */
		out_8(&pixis->base->i2cdacr, dacr);

		status = 0;
		break;
	default:
		status = -EINVAL;
		break;
	}

	mutex_unlock(&pixis->update_lock);
	return status;
}

static u32 pixis_start_pm(void)
{
	u32 status;

	/* enable temperature and current measurements */
	status = pixis_get_temperature(pixis, PIXIS_MEAS_ENABLE);
	status |= pixis_get_current(pixis, PIXIS_MEAS_CORE, PIXIS_MEAS_ENABLE);
	status |= pixis_get_current(pixis, PIXIS_MEAS_PLAT, PIXIS_MEAS_ENABLE);
	if (status) {
		pr_err("PIXIS: pm enable error.\n");
		return -EIO;
	}

	/* read DUT Vcore/Vplat voltage */
	status = pixis_get_voltage(pixis, PIXIS_MEAS_CORE);
	status |= pixis_get_voltage(pixis, PIXIS_MEAS_PLAT);
	if (status) {
		pr_err("PIXIS: pm read voltage error.\n");
		return -EIO;
	}

	return status;
}

static u32 pixis_stop_pm(void)
{
	u32 status;

	/* get temperature and current measurements results */
	status = pixis_get_temperature(pixis, PIXIS_MEAS_RECEIVE);
	status |= pixis_get_current(pixis, PIXIS_MEAS_CORE, PIXIS_MEAS_RECEIVE);
	status |= pixis_get_current(pixis, PIXIS_MEAS_PLAT, PIXIS_MEAS_RECEIVE);
	if (status) {
		pr_err("PIXIS: pm receive error.\n");
		return -EIO;
	}

	/* disable temperature and current measurements */
	status = pixis_get_temperature(pixis, PIXIS_MEAS_DISABLE);
	status |= pixis_get_current(pixis, PIXIS_MEAS_CORE, PIXIS_MEAS_DISABLE);
	status |= pixis_get_current(pixis, PIXIS_MEAS_PLAT, PIXIS_MEAS_DISABLE);
	if (status) {
		pr_err("PIXIS: pm disable error.\n");
		return -EIO;
	}

	return status;
}

/*
 * Thest two functions are provided for Power Manamgement
 * code to call it.
 */
u32 pixis_start_pm_sleep(void)
{
	u32 status = 0;

	if (pixis->pm_cmd == PIXIS_CMD_START_SLEEP) {
		pr_info("PIXIS: start power monitor...\n");
		status = pixis_start_pm();
	}

	if (status)
		pr_err("PIXIS: fail to start pm on sleep mode.\n");

	return status;
}
EXPORT_SYMBOL(pixis_start_pm_sleep);

u32 pixis_stop_pm_sleep(void)
{
	u32 status = 0;

	if (pixis->pm_cmd == PIXIS_CMD_START_SLEEP) {
		pr_info("PIXIS: stop power monitor...\n");
		status = pixis_stop_pm();
	}

	pixis->pm_cmd = PIXIS_CMD_STOP;
	if (status)
		pr_err("PIXIS: fail to stop pm on sleep mode.\n");

	return status;
}
EXPORT_SYMBOL(pixis_stop_pm_sleep);

static int pixis_proc_write_ctrl(struct file *file, const char __user *buffer,
				unsigned long count, void *data)
{
	u8 local_buf[count + 1];
	int err;
	unsigned long pm_cmd;

	if (copy_from_user(local_buf, buffer, count))
		return -EFAULT;

	mutex_lock(&pixis->update_lock);
	local_buf[count] = '\0';
	err = strict_strtoul(local_buf, 10, &pm_cmd);
	if (err) {
		mutex_unlock(&pixis->update_lock);
		return err;
	}
	pixis->pm_cmd = (u32)pm_cmd;
	mutex_unlock(&pixis->update_lock);

	switch (pixis->pm_cmd) {
	case PIXIS_CMD_START:
		pr_info("Start Power Monitoring...\n");
		pixis_start_pm();
		break;
	case PIXIS_CMD_STOP:
		pr_info("Stop Power Monitoring...\n");
		pixis_stop_pm();
		break;
	case PIXIS_CMD_START_SLEEP:
		pr_info("Prepare to start Power Monitoring in sleep mode...\n");
		break;
	default:
		return -EIO;
	}

	return count;
}

/*
 * pm_status
 */
static int pixis_proc_show_status(struct seq_file *m, void *v)
{
	u8 id, ver, pver, dacr;

	id = in_8(&pixis->base->id);
	ver = in_8(&pixis->base->ver);
	pver = in_8(&pixis->base->pver);
	dacr = in_8(&pixis->base->i2cdacr);

	seq_printf(m, "System ID	:	0x%x\n", id);
	seq_printf(m, "System version	:	0x%x\n", ver);
	seq_printf(m, "FPGA version	:	0x%x\n", pver);
	seq_putc(m, '\n');
	seq_printf(m, "I2C control reg	:	0x%x\n", dacr);
	seq_printf(m, "I2C PM command	:	0x%x\n", pixis->pm_cmd);
	seq_putc(m, '\n');

	return 0;
}

static int pixis_proc_status_open(struct inode *inode, struct file *file)
{
	return single_open(file, pixis_proc_show_status, NULL);
}

static const struct file_operations pixis_proc_status_fops = {
	.open		= pixis_proc_status_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

/*
 * pm_results
 */
static int pixis_proc_show_result(struct seq_file *m, void *v)
{
	u32 vcore, icore, vplat, iplat, temp;

	vcore = pixis->v_core;
	icore = pixis->i_core;
	vplat = pixis->v_plat;
	iplat = pixis->i_plat;
	temp = pixis->temp;

	seq_printf(m, "Vcore     :    %4d  mV\n", vcore);
	seq_printf(m, "Icore     :    %4d  mA\n", icore);
	seq_printf(m, "Core Pwr  :    %4d  mW\n", vcore * icore / 1000);
	seq_putc(m, '\n');
	seq_printf(m, "Vplat     :    %4d  mV\n", vplat);
	seq_printf(m, "Iplat     :    %4d  mA\n", iplat);
	seq_printf(m, "Plat Pwr  :    %4d  mW\n", vplat * iplat / 1000);
	seq_putc(m, '\n');
	seq_printf(m, "Temp      :    %4d  C\n", temp);
	seq_putc(m, '\n');

	return 0;
}

static int pixis_proc_result_open(struct inode *inode, struct file *file)
{
	return single_open(file, pixis_proc_show_result, NULL);
}

static const struct file_operations pixis_proc_result_fops = {
	.open		= pixis_proc_result_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

/*
 * Using the I2C controller to configure the address pointer register
 * of ADT7461 to the external temperature register's address.
 */
enum chips { adt7461 };

static int adt7461_probe(struct i2c_client *client,
				const struct i2c_device_id *id)
{
	struct temperature *temp_info = NULL;

	/* load temperature information for board */
	temp_info = temperature_match(pixis);
	if (temp_info == NULL)
		return -ENXIO;

	/* Set Conversion Rate Register */
	i2c_smbus_write_byte_data(client, temp_info->rate_reg,
						temp_info->rate);
	/* Set Address Pointer Register */
	i2c_smbus_write_byte(client, temp_info->temp_reg);

	return 0;
}

static int adt7461_remove(struct i2c_client *client)
{
	return 0;
}

static const struct i2c_device_id adt7461_id[] = {
	{ "adt7461", adt7461 },
	{ }
};

static struct i2c_driver adt7461_driver = {
	.driver = {
		.name = "adt7461",
	},
	.probe = adt7461_probe,
	.remove = adt7461_remove,
	.id_table = adt7461_id,
};

static int __init fsl_pixis_init(void)
{
	struct resource r;
	struct device_node *np;

	pixis = kmalloc(sizeof(*pixis), GFP_KERNEL);
	if (!pixis)
		return -ENOMEM;

	memset(pixis, 0, sizeof(*pixis));

	np = of_find_compatible_node(NULL, NULL, "fsl,mpc8536ds-fpga-pixis");
	if (!np) {
		pr_err("PIXIS: can't find device node"
				" 'fsl,mpc8536ds-fpga-pixis'\n");
		kfree(pixis);
		return -ENXIO;
	}

	of_address_to_resource(np, 0, &r);
	of_node_put(np);

	pixis->base = ioremap(r.start, r.end - r.start + 1);
	if (!pixis->base) {
		pr_err("PIXIS: can't map FPGA cfg register!\n");
		kfree(pixis);
		return -EIO;
	}

	mutex_init(&pixis->update_lock);

	/* Create the /proc entry */
	pixis_proc_root = proc_mkdir("pixis_ctrl", NULL);
	if (!pixis_proc_root) {
		pr_err("PIXIS: failed to create pixis_ctrl entry.\n");
		kfree(pixis);
		iounmap(pixis->base);
		return -ENOMEM;
	}

	pixis_proc_pm_ctrl = create_proc_entry("pm_control", S_IRUGO | S_IWUSR,
							pixis_proc_root);
	pixis_proc_pm_ctrl->write_proc = pixis_proc_write_ctrl;

	proc_create("pm_result", 0, pixis_proc_root, &pixis_proc_result_fops);
	proc_create("pm_status", 0, pixis_proc_root, &pixis_proc_status_fops);

	/* register ADT7461 in order to configure it */
	i2c_add_driver(&adt7461_driver);

	return 0;
}

static void __exit fsl_pixis_exit(void)
{
	iounmap(pixis->base);
	kfree(pixis);
	remove_proc_entry("pixis_control", pixis_proc_root);
	remove_proc_entry("pixis_ctrl", NULL);
	i2c_del_driver(&adt7461_driver);
}

MODULE_AUTHOR("Mingkai hu <Mingkai.hu@freescale.com>");
MODULE_DESCRIPTION("Freescale board control FPGA driver");
MODULE_LICENSE("GPL");

module_init(fsl_pixis_init);
module_exit(fsl_pixis_exit);
