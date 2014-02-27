/*
 * Corenet based SoC DS Setup
 *
 * Maintained by Kumar Gala (see MAINTAINERS for contact information)
 *
 * Copyright 2009-2011 Freescale Semiconductor Inc.
 *
 * This program is free software; you can redistribute	it and/or modify it
 * under  the terms of	the GNU General	 Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */

#include <linux/kernel.h>
#include <linux/pci.h>
#include <linux/kdev_t.h>
#include <linux/delay.h>
#include <linux/interrupt.h>

#include <asm/time.h>
#include <asm/machdep.h>
#include <asm/pci-bridge.h>
#include <asm/ppc-pci.h>
#include <mm/mmu_decl.h>
#include <asm/prom.h>
#include <asm/udbg.h>
#include <asm/mpic.h>
#include <asm/qe.h>
#include <asm/qe_ic.h>

#include <linux/of_platform.h>
#include <sysdev/fsl_soc.h>
#include <sysdev/fsl_pci.h>
#include "smp.h"

#include <linux/fsl_usdpaa.h>

#if defined(CONFIG_PPC64) && defined(CONFIG_SUSPEND)
static void fsl_suspend_disable_irqs(void)
{
	__hard_irq_disable();
}
#endif

void __init corenet_ds_pic_init(void)
{
	struct mpic *mpic;
	unsigned int flags = MPIC_BIG_ENDIAN | MPIC_SINGLE_DEST_CPU |
		MPIC_NO_RESET;

#ifdef CONFIG_QUICC_ENGINE
	struct device_node *np;
#endif

	if (ppc_md.get_irq == mpic_get_coreint_irq)
		flags |= MPIC_ENABLE_COREINT;

	mpic = mpic_alloc(NULL, 0, flags, 0, 512, " OpenPIC  ");
	BUG_ON(mpic == NULL);

	mpic_init(mpic);

#ifdef CONFIG_QUICC_ENGINE
	np = of_find_compatible_node(NULL, NULL, "fsl,qe-ic");
	if (np) {
		qe_ic_init(np, 0, qe_ic_cascade_low_mpic,
				qe_ic_cascade_high_mpic);
		of_node_put(np);
	}
#endif
}

/*
 * Setup the architecture
 */
void __init corenet_ds_setup_arch(void)
{
#ifdef CONFIG_QUICC_ENGINE
	struct device_node *np;
#endif
	mpc85xx_smp_init();

#if defined(CONFIG_PCI) && defined(CONFIG_PPC64)
	pci_devs_phb_init();
#endif

	fsl_pci_assign_primary();

	swiotlb_detect_4g();

#if defined(CONFIG_PPC64) && defined(CONFIG_SUSPEND)
	/*
	 * really disable irq by writing MSR_EE for 64-bit mode when suspend
	 */
	ppc_md.suspend_disable_irqs = fsl_suspend_disable_irqs;
#endif

	fsl_rcpm_init();

	pr_info("%s board from Freescale Semiconductor\n", ppc_md.name);

#ifdef CONFIG_QUICC_ENGINE
	np = of_find_compatible_node(NULL, NULL, "fsl,qe");
	if (!np) {
		pr_err("%s: Could not find Quicc Engine node\n", __func__);
		return;
	}
	qe_reset();
	of_node_put(np);
#endif
}

static const struct of_device_id of_device_ids[] = {
	{
		.compatible	= "simple-bus",
	},
	{
		.compatible	= "fsl,dpaa",
	},
	{
		.compatible	= "mdio-mux-gpio",
	},
	{
		.compatible	= "fsl,fpga-ngpixis",
	},
	{
		.compatible	= "fsl,fpga-qixis",
	},
	{
		.compatible	= "fsl,srio",
	},
	{
		.compatible	= "fsl,p4080-pcie",
	},
	{
		.compatible	= "fsl,qoriq-pcie-v2.2",
	},
	{
		.compatible	= "fsl,qoriq-pcie-v2.3",
	},
	{
		.compatible	= "fsl,qoriq-pcie-v2.4",
	},
	{
		.compatible	= "fsl,qoriq-pcie-v3.0",
	},
	{	.compatible	= "fsl,qe",
	},
	/* The following two are for the Freescale hypervisor */
	{
		.name		= "hypervisor",
	},
	{
		.name		= "handles",
	},
	{}
};

int __init corenet_ds_publish_devices(void)
{
	return of_platform_bus_probe(NULL, of_device_ids, NULL);
}

/* Early setup is required for large chunks of contiguous (and coarsely-aligned)
 * memory. The following shoe-horns Q/Bman "init_early" calls into the
 * platform setup to let them parse their CCSR nodes early on. */
#ifdef CONFIG_FSL_QMAN_CONFIG
void __init qman_init_early(void);
#endif
#ifdef CONFIG_FSL_BMAN_CONFIG
void __init bman_init_early(void);
#endif
#ifdef CONFIG_FSL_PME2_CTRL
void __init pme2_init_early(void);
#endif

__init void corenet_ds_init_early(void)
{
#ifdef CONFIG_FSL_QMAN_CONFIG
	qman_init_early();
#endif
#ifdef CONFIG_FSL_BMAN_CONFIG
	bman_init_early();
#endif
#ifdef CONFIG_FSL_PME2_CTRL
	pme2_init_early();
#endif
#ifdef CONFIG_FSL_USDPAA
	fsl_usdpaa_init_early();
#endif
}
