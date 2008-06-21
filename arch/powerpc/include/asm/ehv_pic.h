/*
 * EHV_PIC private definitions and structure.
 *
 * Copyright 2008-2010 Freescale Semiconductor, Inc.
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive
 * for more details.
 */
#ifndef __EHV_PIC_H__
#define __EHV_PIC_H__

#include <linux/irq.h>

#define NR_EHV_PIC_INTS 1024

#define EHV_PIC_INFO(name) EHV_PIC_##name

#define EHV_PIC_VECPRI_POLARITY_NEGATIVE 0
#define EHV_PIC_VECPRI_POLARITY_POSITIVE 1
#define EHV_PIC_VECPRI_SENSE_EDGE 0
#define EHV_PIC_VECPRI_SENSE_LEVEL 0x2
#define EHV_PIC_VECPRI_POLARITY_MASK 0x1
#define EHV_PIC_VECPRI_SENSE_MASK 0x2

struct ehv_pic {
	/* The remapper for this EHV_PIC */
	struct irq_host	*irqhost;

	/* The "linux" controller struct */
	struct irq_chip	hc_irq;

	/* core int flag */
	int coreint_flag;
};

/*
 * Exported ehv_pic functions
 */

void __init ehv_pic_init(struct device_node *node, int coreint_flag);
unsigned int ehv_pic_get_irq(void);

#endif /* __EHV_PIC_H__ */
