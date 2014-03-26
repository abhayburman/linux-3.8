/*
 * T104x RDB Setup
 * Should apply for RDB platform of T1040 and it's personalities.
 * viz T1040/T1042
 *
 * Copyright 2014 Freescale Semiconductor Inc.
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
#include <linux/phy.h>

#include <asm/time.h>
#include <asm/machdep.h>
#include <asm/pci-bridge.h>
#include <mm/mmu_decl.h>
#include <asm/prom.h>
#include <asm/udbg.h>
#include <asm/mpic.h>

#include <linux/of_platform.h>
#include <sysdev/fsl_soc.h>
#include <sysdev/fsl_pci.h>
#include <asm/ehv_pic.h>

#include "corenet_ds.h"

#if defined(CONFIG_FB_FSL_DIU) || defined(CONFIG_FB_FSL_DIU_MODULE)
/*DIU Pixel ClockCR offset in scfg*/
#define CCSR_SCFG_PIXCLKCR      0x28

/* DIU Pixel Clock bits of the PIXCLKCR */
#define PIXCLKCR_PXCKEN		0x80000000
#define PIXCLKCR_PXCKINV	0x40000000
#define PIXCLKCR_PXCKDLY	0x0000FF00
#define PIXCLKCR_PXCLK_MASK	0x00FF0000

/* Some CPLD register definitions */
#define CPLD_DIUCSR		0x16

#define CPLD_DIUCSR_DVIEN	0x80
#define CPLD_DIUCSR_BACKLIGHT	0x0f

/**
 * t104xrdb_set_monitor_port: switch the output to a different monitor port
 */
static void t104xrdb_set_monitor_port(enum fsl_diu_monitor_port port)
{
	struct device_node *cpld_node;
	static void __iomem *cpld_base;

	cpld_node = of_find_compatible_node(NULL, NULL, "fsl,t104xrdb-cpld");
	if (!cpld_node) {
		pr_err("T104xRDB: missing CPLD node\n");
		return;
	}

	cpld_base = of_iomap(cpld_node, 0);
	if (!cpld_base) {
		pr_err("T104xRDB: could not map cpld registers\n");
		goto exit;
	}

	switch (port) {
	case FSL_DIU_PORT_DVI:
		/* Enable the DVI(HDMI) port, disable the DFP and
		 * the backlight
		 */
		clrbits8(cpld_base + CPLD_DIUCSR, CPLD_DIUCSR_DVIEN);
		break;
	case FSL_DIU_PORT_LVDS:
		/*
		 * LVDS also needs backlight enabled, otherwise the display
		 * will be blank.
		 */
		/* Enable the DFP port, disable the DVI*/
		setbits8(cpld_base + CPLD_DIUCSR, 0x01 << 8);
		setbits8(cpld_base + CPLD_DIUCSR, 0x01 << 4);
		setbits8(cpld_base + CPLD_DIUCSR, CPLD_DIUCSR_BACKLIGHT);
		break;
	default:
		pr_err("T104xRDB: unsupported monitor port %i\n", port);
	}

exit:
	of_node_put(cpld_node);
}

/**
 * t104xrdb_set_pixel_clock: program the DIU's clock
 *
 * @pixclock: the wavelength, in picoseconds, of the clock
 */
void t104xrdb_set_pixel_clock(unsigned int pixclock)
{
	struct device_node *scfg_np = NULL;
	void __iomem *scfg;
	unsigned long freq;
	u64 temp;
	u32 pxclk;

	/* Map the global utilities registers. */
	scfg_np = of_find_compatible_node(NULL, NULL, "fsl,t1040-scfg");
	if (!scfg_np) {
		freq = temp;
		pr_err("T104xRDB: missing supplemental configuration unit device node\n");
		return;
	}

	scfg = of_iomap(scfg_np, 0);
	of_node_put(scfg_np);
	if (!scfg) {
		pr_err("T104xRDB: could not map device\n");
		return;
	}

	/* Convert pixclock from a wavelength to a frequency */
	temp = 1000000000000ULL;
	do_div(temp, pixclock);
	freq = temp;

	/*
	 * 'pxclk' is the ratio of the platform clock to the pixel clock.
	 * This number is programmed into the PIXCLKCR register, and the valid
	 * range of values is 2-255.
	 */
	pxclk = DIV_ROUND_CLOSEST(fsl_get_sys_freq(), freq);
	pxclk = clamp_t(u32, pxclk, 2, 255);

	/* Disable the pixel clock, and set it to non-inverted and no delay */
	clrbits32(scfg + CCSR_SCFG_PIXCLKCR,
		  PIXCLKCR_PXCKEN | PIXCLKCR_PXCKDLY | PIXCLKCR_PXCLK_MASK);

	/* Enable the clock and set the pxclk */
	setbits32(scfg + CCSR_SCFG_PIXCLKCR, PIXCLKCR_PXCKEN | (pxclk << 16));

	iounmap(scfg);
}

/**
 * t104xrdb_valid_monitor_port: set the monitor port for sysfs
 */
enum fsl_diu_monitor_port
t104xrdb_valid_monitor_port(enum fsl_diu_monitor_port port)
{
	switch (port) {
	case FSL_DIU_PORT_DVI:
	case FSL_DIU_PORT_LVDS:
		return port;
	default:
		return FSL_DIU_PORT_DVI; /* Dual-link LVDS is not supported */
	}
}

#endif

/*
 * Called very early, device-tree isn't unflattened
 */
static int __init t104x_rdb_probe(void)
{
	unsigned long root = of_get_flat_dt_root();

#if defined(CONFIG_FB_FSL_DIU) || defined(CONFIG_FB_FSL_DIU_MODULE)
	diu_ops.set_monitor_port	= t104xrdb_set_monitor_port;
	diu_ops.set_pixel_clock		= t104xrdb_set_pixel_clock;
	diu_ops.valid_monitor_port	= t104xrdb_valid_monitor_port;
#endif

	if (of_flat_dt_is_compatible(root, "fsl,T1040RDB") ||
		of_flat_dt_is_compatible(root, "fsl,T1042RDB") ||
		of_flat_dt_is_compatible(root, "fsl,T1042RDB_PI"))

		return 1;

	/* Check if we're running under the Freescale hypervisor */
	if (of_flat_dt_is_compatible(root, "fsl,T1040RDB-hv") ||
		of_flat_dt_is_compatible(root, "fsl,T1042RDB-hv") ||
		of_flat_dt_is_compatible(root, "fsl,T1042RDB_PI-hv")) {
		ppc_md.init_IRQ = ehv_pic_init;
		ppc_md.get_irq = ehv_pic_get_irq;
		ppc_md.restart = fsl_hv_restart;
		ppc_md.power_off = fsl_hv_halt;
		ppc_md.halt = fsl_hv_halt;
		return 1;
	}

	return 0;
}

define_machine(t104x_rdb) {
	.name			= "T104x RDB",
	.probe			= t104x_rdb_probe,
	.setup_arch		= corenet_ds_setup_arch,
	.init_IRQ		= corenet_ds_pic_init,
#ifdef CONFIG_PCI
	.pcibios_fixup_bus	= fsl_pcibios_fixup_bus,
#endif
/* coreint doesn't play nice with lazy EE, use legacy mpic for now */
#ifdef CONFIG_PPC64
	.get_irq		= mpic_get_irq,
#else
	.get_irq		= mpic_get_coreint_irq,
#endif
	.restart		= fsl_rstcr_restart,
	.calibrate_decr		= generic_calibrate_decr,
	.progress		= udbg_progress,
#ifdef CONFIG_PPC64
	.power_save		= book3e_idle,
#else
	.power_save		= e500_idle,
#endif
	.init_early		= corenet_ds_init_early,
};

machine_arch_initcall(t104x_rdb, corenet_ds_publish_devices);

#ifdef CONFIG_SWIOTLB
machine_arch_initcall(t104x_rdb, swiotlb_setup_bus_notifier);
#endif
