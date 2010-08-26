/*
 * Freescale Hypervisor Platform
 *
 * This is the platform file for corenet based SoC setup under the
 * Freescale Hypervisor.
 *
 * Maintained by Kumar Gala (see MAINTAINERS for contact information)
 * Copyright 2008-2010 Freescale Semiconductor Inc.
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */

#include <linux/stddef.h>
#include <linux/kernel.h>
#include <asm/ehv_pic.h>
#include <asm/fsl_hcalls.h>
#include <linux/of_platform.h>

void __init fsl_hv_pic_init(void)
{
	struct device_node *np;
	int coreint_flag = 1;

	np = of_find_compatible_node(NULL, NULL, "epapr,hv-pic");

	if (np == NULL) {
		printk(KERN_ERR "Could not find ehv_pic node\n");
		of_node_put(np);
		return;
	}

	if (!of_find_property(np, "has-external-proxy", NULL))
		coreint_flag = 0;

	ehv_pic_init(np, coreint_flag);

	of_node_put(np);
}

void fsl_hv_restart(char *cmd)
{
	printk(KERN_INFO "hv restart\n");
	fh_partition_restart(-1);
}

void fsl_hv_halt(void)
{
	printk(KERN_INFO "hv exit\n");
	fh_partition_stop(-1);
}

