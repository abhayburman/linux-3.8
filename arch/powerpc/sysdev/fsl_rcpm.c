/*
 * RCPM(Run Control/Power Management) support
 *
 * Copyright 2012-2014 Freescale Semiconductor Inc.
 *
 * Author: Chenhui Zhao <chenhui.zhao@freescale.com>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/of_platform.h>

#include <asm/io.h>
#include <asm/fsl_guts.h>
#include <asm/cputhreads.h>
#include <sysdev/fsl_soc.h>

const struct fsl_pm_ops *qoriq_pm_ops;

static struct ccsr_rcpm __iomem *rcpm_v1_regs;
static struct ccsr_rcpm_v2 __iomem *rcpm_v2_regs;

static void rcpm_v1_irq_mask(int cpu)
{
	int hw_cpu = get_hard_smp_processor_id(cpu);
	unsigned int mask = 1 << hw_cpu;

	setbits32(&rcpm_v1_regs->cpmimr, mask);
	setbits32(&rcpm_v1_regs->cpmcimr, mask);
	setbits32(&rcpm_v1_regs->cpmmcmr, mask);
	setbits32(&rcpm_v1_regs->cpmnmimr, mask);
}

static void rcpm_v1_irq_unmask(int cpu)
{
	int hw_cpu = get_hard_smp_processor_id(cpu);
	unsigned int mask = 1 << hw_cpu;

	clrbits32(&rcpm_v1_regs->cpmimr, mask);
	clrbits32(&rcpm_v1_regs->cpmcimr, mask);
	clrbits32(&rcpm_v1_regs->cpmmcmr, mask);
	clrbits32(&rcpm_v1_regs->cpmnmimr, mask);
}

static void rcpm_v1_set_ip_power(int enable, u32 mask)
{
	if (enable)
		setbits32(&rcpm_v1_regs->ippdexpcr, mask);
	else
		clrbits32(&rcpm_v1_regs->ippdexpcr, mask);
}

static void rcpm_v1_cpu_enter_state(int cpu, int state)
{
	unsigned int hw_cpu = get_hard_smp_processor_id(cpu);
	unsigned int mask = 1 << hw_cpu;

	switch (state) {
	case E500_PM_PH10:
		setbits32(&rcpm_v1_regs->cdozcr, mask);
		break;
	case E500_PM_PH15:
		setbits32(&rcpm_v1_regs->cnapcr, mask);
		break;
	default:
		pr_err("Unknown cpu PM state\n");
		break;
	}
}

static void rcpm_v1_cpu_exit_state(int cpu, int state)
{
	unsigned int hw_cpu = get_hard_smp_processor_id(cpu);
	unsigned int mask = 1 << hw_cpu;

	switch (state) {
	case E500_PM_PH10:
		clrbits32(&rcpm_v1_regs->cdozcr, mask);
		break;
	case E500_PM_PH15:
		clrbits32(&rcpm_v1_regs->cnapcr, mask);
		break;
	default:
		pr_err("Unknown cpu PM state\n");
		break;
	}
}

static int rcpm_v1_plat_enter_state(int state)
{
	u32 *pmcsr_reg = &rcpm_v1_regs->powmgtcsr;
	int ret = 0;
	int result;

	switch (state) {
	case PLAT_PM_SLEEP:
		setbits32(pmcsr_reg, RCPM_POWMGTCSR_SLP);

		/* At this point, the device is in sleep mode. */

		/* Upon resume, wait for RCPM_POWMGTCSR_SLP bit to be clear. */
		result = spin_event_timeout(
		  !(in_be32(pmcsr_reg) & RCPM_POWMGTCSR_SLP), 10000, 10);
		if (!result) {
			pr_err("%s: timeout waiting for SLP bit to be cleared\n",
			  __func__);
			ret = -ETIMEDOUT;
		}
		break;
	default:
		pr_err("Unsupported platform PM state\n");
		ret = -EINVAL;
	}

	return ret;
}

static void rcpm_v1_freeze_time_base(int freeze)
{
	u32 *tben_reg = &rcpm_v1_regs->ctbenr;
	static u32 mask;

	if (freeze) {
		mask = in_be32(tben_reg);
		clrbits32(tben_reg, mask);
	} else {
		setbits32(tben_reg, mask);
	}

	/* read back to push the previous write */
	in_be32(tben_reg);
}

static void rcpm_v2_freeze_time_base(int freeze)
{
	u32 *tben_reg = &rcpm_v2_regs->pctbenr;
	static u32 mask;

	if (freeze) {
		mask = in_be32(tben_reg);
		clrbits32(tben_reg, mask);
	} else {
		setbits32(tben_reg, mask);
	}

	/* read back to push the previous write */
	in_be32(tben_reg);
}

static void rcpm_v2_irq_mask(int cpu)
{
	int hw_cpu = get_hard_smp_processor_id(cpu);
	unsigned int mask = 1 << hw_cpu;

	setbits32(&rcpm_v2_regs->tpmimr0, mask);
	setbits32(&rcpm_v2_regs->tpmcimr0, mask);
	setbits32(&rcpm_v2_regs->tpmmcmr0, mask);
	setbits32(&rcpm_v2_regs->tpmnmimr0, mask);
}

static void rcpm_v2_irq_unmask(int cpu)
{
	int hw_cpu = get_hard_smp_processor_id(cpu);
	unsigned int mask = 1 << hw_cpu;

	clrbits32(&rcpm_v2_regs->tpmimr0, mask);
	clrbits32(&rcpm_v2_regs->tpmcimr0, mask);
	clrbits32(&rcpm_v2_regs->tpmmcmr0, mask);
	clrbits32(&rcpm_v2_regs->tpmnmimr0, mask);
}

static void rcpm_v2_set_ip_power(int enable, u32 mask)
{
	if (enable)
		/* set to enable power in deep sleep mode */
		setbits32(&rcpm_v2_regs->ippdexpcr[0], mask);
	else
		clrbits32(&rcpm_v2_regs->ippdexpcr[0], mask);
}

static void rcpm_v2_cpu_enter_state(int cpu, int state)
{
	unsigned int hw_cpu = get_hard_smp_processor_id(cpu);
	u32 mask = 1 << cpu_core_index_of_thread(hw_cpu);

	switch (state) {
	case E500_PM_PH10:
		setbits32(&rcpm_v2_regs->tph10setr0, 1 << hw_cpu);
		break;
	case E500_PM_PH15:
		setbits32(&rcpm_v2_regs->pcph15setr, mask);
		break;
	case E500_PM_PH20:
		setbits32(&rcpm_v2_regs->pcph20setr, mask);
		break;
	case E500_PM_PH30:
		setbits32(&rcpm_v2_regs->pcph30setr, mask);
		break;
	default:
		pr_err("Unsupported cpu PM state\n");
	}
}

static void rcpm_v2_cpu_exit_state(int cpu, int state)
{
	unsigned int hw_cpu = get_hard_smp_processor_id(cpu);
	u32 mask = 1 << cpu_core_index_of_thread(hw_cpu);

	switch (state) {
	case E500_PM_PH10:
		setbits32(&rcpm_v2_regs->tph10clrr0, 1 << hw_cpu);
		break;
	case E500_PM_PH15:
		setbits32(&rcpm_v2_regs->pcph15clrr, mask);
		break;
	case E500_PM_PH20:
		setbits32(&rcpm_v2_regs->pcph20clrr, mask);
		break;
	case E500_PM_PH30:
		setbits32(&rcpm_v2_regs->pcph30clrr, mask);
		break;
	default:
		pr_err("Unsupported cpu PM state\n");
	}
}

static int rcpm_v2_plat_enter_state(int state)
{
	u32 *pmcsr_reg = &rcpm_v2_regs->powmgtcsr;
	int ret = 0;
	int result;

	switch (state) {
	case PLAT_PM_LPM20:
		/* clear previous LPM20 status */
		setbits32(pmcsr_reg, RCPM_POWMGTCSR_P_LPM20_ST);
		/* enter LPM20 status */
		setbits32(pmcsr_reg, RCPM_POWMGTCSR_LPM20_RQ);

		/* At this point, the device is in LPM20 status. */

		/* resume ... */
		result = spin_event_timeout(
		  !(in_be32(pmcsr_reg) & RCPM_POWMGTCSR_LPM20_ST), 10000, 10);
		if (!result) {
			pr_err("%s: timeout waiting for LPM20 bit to be cleared\n",
				__func__);
			ret = -ETIMEDOUT;
		}
		break;
	default:
		pr_err("Unsupported platform PM state\n");
		ret = -EINVAL;
	}

	return ret;
}

static const struct fsl_pm_ops qoriq_rcpm_v1_ops = {
	.irq_mask = rcpm_v1_irq_mask,
	.irq_unmask = rcpm_v1_irq_unmask,
	.cpu_enter_state = rcpm_v1_cpu_enter_state,
	.cpu_exit_state = rcpm_v1_cpu_exit_state,
	.plat_enter_state = rcpm_v1_plat_enter_state,
	.set_ip_power = rcpm_v1_set_ip_power,
	.freeze_time_base = rcpm_v1_freeze_time_base,
};

static const struct fsl_pm_ops qoriq_rcpm_v2_ops = {
	.irq_mask = rcpm_v2_irq_mask,
	.irq_unmask = rcpm_v2_irq_unmask,
	.cpu_enter_state = rcpm_v2_cpu_enter_state,
	.cpu_exit_state = rcpm_v2_cpu_exit_state,
	.plat_enter_state = rcpm_v2_plat_enter_state,
	.set_ip_power = rcpm_v2_set_ip_power,
	.freeze_time_base = rcpm_v2_freeze_time_base,
};

int fsl_rcpm_init(void)
{
	struct device_node *np;

	np = of_find_compatible_node(NULL, NULL, "fsl,qoriq-rcpm-2.0");
	if (np) {
		rcpm_v2_regs = of_iomap(np, 0);
		of_node_put(np);
		if (!rcpm_v2_regs)
			return -ENOMEM;

		qoriq_pm_ops = &qoriq_rcpm_v2_ops;

	} else {
		np = of_find_compatible_node(NULL, NULL, "fsl,qoriq-rcpm-1.0");
		if (np) {
			rcpm_v1_regs = of_iomap(np, 0);
			of_node_put(np);
			if (!rcpm_v1_regs)
				return -ENOMEM;

			qoriq_pm_ops = &qoriq_rcpm_v1_ops;

		} else {
			pr_err("%s: can't find the rcpm node.\n", __func__);
			return -EINVAL;
		}
	}

	pr_info("Freescale RCPM driver\n");

	return 0;
}
