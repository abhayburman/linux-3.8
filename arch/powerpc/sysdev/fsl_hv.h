/*
 * Setup for FSL Hypervisor
 *
 * Copyright 2009-2010 Freescale Semiconductor Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#ifndef FSL_HV_H
#define FSL_HV_H

extern void __init fsl_hv_pic_init(void);
extern void fsl_hv_restart(char *cmd);
extern void fsl_hv_halt(void);

#endif
