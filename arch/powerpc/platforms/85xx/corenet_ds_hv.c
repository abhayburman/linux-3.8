/*
 * Freescale Hypervisor Platform
 *
 * This is the platform file for corenet based SoC setup under the
 * Freescale Hypervisor.
 *
 * Maintained by Kumar Gala (see MAINTAINERS for contact information)
 * Copyright 2008-2011 Freescale Semiconductor Inc.
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */

#include <linux/stddef.h>
#include <linux/kernel.h>
#include <linux/pci.h>
#include <linux/kdev_t.h>
#include <linux/delay.h>
#include <linux/seq_file.h>
#include <linux/interrupt.h>

#include <asm/system.h>
#include <asm/time.h>
#include <asm/machdep.h>
#include <asm/pci-bridge.h>
#include <mm/mmu_decl.h>
#include <asm/prom.h>
#include <asm/udbg.h>
#include <asm/ehv_pic.h>
#include <asm/fsl_hcalls.h>

#include <linux/of_platform.h>
#include <sysdev/fsl_soc.h>
#include <sysdev/fsl_pci.h>
#include <sysdev/fsl_hv.h>

#include "corenet_ds.h"

/*
 * Setup the architecture
 */
#ifdef CONFIG_SMP
void __init mpc85xx_smp_init(void);
#endif

static void __init fsl_hv_setup_arch(void)
{
#ifdef CONFIG_PCI
	struct device_node *np;
#endif

	if (ppc_md.progress)
		ppc_md.progress("fsl_hv_setup_arch()", 0);

#ifdef CONFIG_SMP
	mpc85xx_smp_init();
#endif

#ifdef CONFIG_PCI
	for_each_node_by_type(np, "pci") {
		if (of_device_is_compatible(np, "fsl,p4080-pcie") ||
		    of_device_is_compatible(np, "fsl,qoriq-pcie-v2.2")) {
			struct resource rsrc;
			of_address_to_resource(np, 0, &rsrc);
			fsl_add_bridge(np, 0);
		}
	}
#endif

	printk(KERN_INFO "Corenet DS + Hypervisor platform from Freescale Semiconductor\n");
}

machine_device_initcall(p3041_hv, declare_of_platform_devices);
machine_device_initcall(p4080_hv, declare_of_platform_devices);
machine_device_initcall(p5020_hv, declare_of_platform_devices);

/*
 * Called very early, device-tree isn't unflattened
 */
static int __init p3041hv_probe(void)
{
	unsigned long root = of_get_flat_dt_root();

	if (of_flat_dt_is_compatible(root, "fsl,P3041DS-hv"))
		return 1;
	else
		return 0;
}

define_machine(p3041_hv) {
	.name			= "P3041DS HV",
	.probe			= p3041hv_probe,
	.setup_arch		= fsl_hv_setup_arch,
	.init_IRQ		= fsl_hv_pic_init,
#ifdef CONFIG_PCI
	.pcibios_fixup_bus	= fsl_pcibios_fixup_bus,
#endif
	.get_irq		= ehv_pic_get_irq,
	.restart		= fsl_hv_restart,
	.power_off		= fsl_hv_halt,
	.halt			= fsl_hv_halt,
	.calibrate_decr		= generic_calibrate_decr,
	.progress		= udbg_progress,
	.idle_loop		= cpu_idle_simple,
	.power_save		= ppc_wait,
	.init_early		= corenet_ds_init_early,
};

/*
 * Called very early, device-tree isn't unflattened
 */
static int __init p4080hv_probe(void)
{
	unsigned long root = of_get_flat_dt_root();

	if (of_flat_dt_is_compatible(root, "fsl,P4080DS-hv"))
		return 1;
	else
		return 0;
}

define_machine(p4080_hv) {
	.name			= "P4080DS HV",
	.probe			= p4080hv_probe,
	.setup_arch		= fsl_hv_setup_arch,
	.init_IRQ		= fsl_hv_pic_init,
#ifdef CONFIG_PCI
	.pcibios_fixup_bus	= fsl_pcibios_fixup_bus,
#endif
	.get_irq		= ehv_pic_get_irq,
	.restart		= fsl_hv_restart,
	.power_off		= fsl_hv_halt,
	.halt			= fsl_hv_halt,
	.calibrate_decr		= generic_calibrate_decr,
	.progress		= udbg_progress,
	.idle_loop		= cpu_idle_simple,
	.power_save		= ppc_wait,
	.init_early		= corenet_ds_init_early,
};

/*
 * Called very early, device-tree isn't unflattened
 */
static int __init p5020hv_probe(void)
{
	unsigned long root = of_get_flat_dt_root();

	if (of_flat_dt_is_compatible(root, "fsl,P5020DS-hv"))
		return 1;
	else
		return 0;
}

define_machine(p5020_hv) {
	.name			= "P5020DS HV",
	.probe			= p5020hv_probe,
	.setup_arch		= fsl_hv_setup_arch,
	.init_IRQ		= fsl_hv_pic_init,
#ifdef CONFIG_PCI
	.pcibios_fixup_bus	= fsl_pcibios_fixup_bus,
#endif
	.get_irq		= ehv_pic_get_irq,
	.restart		= fsl_hv_restart,
	.power_off		= fsl_hv_halt,
	.halt			= fsl_hv_halt,
	.calibrate_decr		= generic_calibrate_decr,
	.progress		= udbg_progress,
	.idle_loop		= cpu_idle_simple,
	.power_save		= ppc_wait,
	.init_early		= corenet_ds_init_early,
};
