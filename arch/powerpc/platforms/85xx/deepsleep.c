/*
 * Support deep sleep feature
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
#include <linux/of_platform.h>
#include <asm/machdep.h>
#include <sysdev/fsl_soc.h>

#define SIZE_1MB	0x100000
#define SIZE_2MB	0x200000

#define CCSR_SCFG_DPSLPCR	0xfc000
#define CCSR_SCFG_DPSLPCR_WDRR_EN	0x1
#define CCSR_SCFG_SPARECR2	0xfc504
#define CCSR_SCFG_SPARECR3	0xfc508

#define CCSR_GPIO1_GPDIR	0x130000
#define CCSR_GPIO1_GPODR	0x130004
#define CCSR_GPIO1_GPDAT	0x130008
#define CCSR_GPIO1_GPDIR_29		0x4

/* 128 bytes buffer for restoring data broke by DDR training initialization */
#define DDR_BUF_SIZE	128
static u8 ddr_buff[DDR_BUF_SIZE] __aligned(64);

static void *dcsr_base, *ccsr_base;

int fsl_dp_iomap(void)
{
	struct device_node *np;
	const u32 *prop;
	int ret = 0;
	u64 ccsr_phy_addr, dcsr_phy_addr;

	np = of_find_node_by_type(NULL, "soc");
	if (!np) {
		pr_err("%s: Can't find the node of \"soc\"\n", __func__);
		ret = -EINVAL;
		goto ccsr_err;
	}
	prop = of_get_property(np, "ranges", NULL);
	if (!prop) {
		pr_err("%s: Can't find the property of \"ranges\"\n", __func__);
		of_node_put(np);
		ret = -EINVAL;
		goto ccsr_err;
	}
	ccsr_phy_addr = of_translate_address(np, prop + 1);
	ccsr_base = ioremap((phys_addr_t)ccsr_phy_addr, SIZE_2MB);
	of_node_put(np);
	if (!ccsr_base) {
		ret = -ENOMEM;
		goto ccsr_err;
	}

	np = of_find_compatible_node(NULL, NULL, "fsl,dcsr");
	if (!np) {
		pr_err("%s: Can't find the node of \"fsl,dcsr\"\n", __func__);
		ret = -EINVAL;
		goto dcsr_err;
	}
	prop = of_get_property(np, "ranges", NULL);
	if (!prop) {
		pr_err("%s: Can't find the property of \"ranges\"\n", __func__);
		of_node_put(np);
		ret = -EINVAL;
		goto dcsr_err;
	}
	dcsr_phy_addr = of_translate_address(np, prop + 1);
	dcsr_base = ioremap((phys_addr_t)dcsr_phy_addr, SIZE_1MB);
	of_node_put(np);
	if (!dcsr_base) {
		ret = -ENOMEM;
		goto dcsr_err;
	}

	return 0;

dcsr_err:
	iounmap(ccsr_base);
ccsr_err:
	ccsr_base = NULL;
	dcsr_base = NULL;
	return ret;
}

void fsl_dp_iounmap(void)
{
	if (dcsr_base) {
		iounmap(dcsr_base);
		dcsr_base = NULL;
	}

	if (ccsr_base) {
		iounmap(ccsr_base);
		ccsr_base = NULL;
	}
}

static void fsl_dp_ddr_save(void *ccsr_base)
{
	u32 ddr_buff_addr;

	/*
	 * DDR training initialization will break 128 bytes at the beginning
	 * of DDR, therefore, save them so that the bootloader will restore
	 * them. Assume that DDR is mapped to the address space started with
	 * CONFIG_PAGE_OFFSET.
	 */
	memcpy(ddr_buff, (void *)CONFIG_PAGE_OFFSET, DDR_BUF_SIZE);

	/* assume ddr_buff is in the physical address space of 4GB */
	ddr_buff_addr = (u32)(__pa(ddr_buff) & 0xffffffff);

	/*
	 * the bootloader will restore the first 128 bytes of DDR from
	 * the location indicated by the register SPARECR3
	 */
	out_be32(ccsr_base + CCSR_SCFG_SPARECR3, ddr_buff_addr);
}

static void fsl_dp_set_resume_pointer(void *ccsr_base)
{
	u32 resume_addr;

	/* the bootloader will finally jump to this address to return kernel */
#ifdef CONFIG_PPC32
	resume_addr = (u32)(__pa(__entry_deep_sleep));
#else
	resume_addr = (u32)(__pa(*(u64 *)__entry_deep_sleep) & 0xffffffff);
#endif

	/* use the register SPARECR2 to save the resume address */
	out_be32(ccsr_base + CCSR_SCFG_SPARECR2, resume_addr);

}

int fsl_enter_epu_deepsleep(void)
{

	fsl_dp_ddr_save(ccsr_base);

	fsl_dp_set_resume_pointer(ccsr_base);

	/*  enable Warm Device Reset request. */
	setbits32(ccsr_base + CCSR_SCFG_DPSLPCR, CCSR_SCFG_DPSLPCR_WDRR_EN);

	/* set GPIO1_29 as an output pin (not open-drain), and output 0 */
	clrbits32(ccsr_base + CCSR_GPIO1_GPDAT, CCSR_GPIO1_GPDIR_29);
	clrbits32(ccsr_base + CCSR_GPIO1_GPODR, CCSR_GPIO1_GPDIR_29);
	setbits32(ccsr_base + CCSR_GPIO1_GPDIR, CCSR_GPIO1_GPDIR_29);

	fsl_dp_fsm_setup(dcsr_base);

	fsl_dp_enter_low(ccsr_base, dcsr_base, qixis_base);

	/* disable Warm Device Reset request */
	clrbits32(ccsr_base + CCSR_SCFG_DPSLPCR, CCSR_SCFG_DPSLPCR_WDRR_EN);

	fsl_dp_fsm_clean(dcsr_base);

	return 0;
}
