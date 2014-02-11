/*
 * Support Power Management feature
 *
 * Copyright 2014 Freescale Semiconductor Inc.
 *
 * Author: Chenhui Zhao <chenhui.zhao@freescale.com>
 *
 * This program is free software; you can redistribute	it and/or modify it
 * under  the terms of	the GNU General	 Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */

#include <linux/kernel.h>
#include <linux/suspend.h>
#include <linux/of_platform.h>
#include <linux/export.h>
#include <linux/fsl_devices.h>

#include <sysdev/fsl_soc.h>

#define FSL_SLEEP		0x1
#define FSL_DEEP_SLEEP		0x2

/* specify the sleep state of the present platform */
int sleep_pm_state;
/* supported sleep modes by the present platform */
static unsigned int sleep_modes;

static int qoriq_suspend_enter(suspend_state_t state)
{
	int ret = 0;

	switch (state) {
	case PM_SUSPEND_STANDBY:

		if (cur_cpu_spec->cpu_flush_caches)
			cur_cpu_spec->cpu_flush_caches();

		ret = qoriq_pm_ops->plat_enter_state(sleep_pm_state);

		break;

	default:
		ret = -EINVAL;

	}

	return ret;
}

static int qoriq_suspend_valid(suspend_state_t state)
{
	if (state == PM_SUSPEND_STANDBY && (sleep_modes & FSL_SLEEP))
		return 1;

	return 0;
}

static int qoriq_suspend_begin(suspend_state_t state)
{
	set_pm_suspend_state(state);

	return 0;
}

static void qoriq_suspend_end(void)
{
	set_pm_suspend_state(PM_SUSPEND_ON);
}

static const struct platform_suspend_ops qoriq_suspend_ops = {
	.valid = qoriq_suspend_valid,
	.enter = qoriq_suspend_enter,
	.begin = qoriq_suspend_begin,
	.end = qoriq_suspend_end,
};

static int __init qoriq_suspend_init(void)
{
	struct device_node *np;

	sleep_modes = FSL_SLEEP;
	sleep_pm_state = PLAT_PM_SLEEP;

	np = of_find_compatible_node(NULL, NULL, "fsl,qoriq-rcpm-2.0");
	if (np)
		sleep_pm_state = PLAT_PM_LPM20;

	suspend_set_ops(&qoriq_suspend_ops);
	set_pm_suspend_state(PM_SUSPEND_ON);

	return 0;
}
arch_initcall(qoriq_suspend_init);
