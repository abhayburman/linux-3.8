#ifndef __PPC_FSL_SOC_H
#define __PPC_FSL_SOC_H
#ifdef __KERNEL__

#include <asm/mmu.h>

struct spi_device;

extern phys_addr_t get_immrbase(void);
#if defined(CONFIG_CPM2) || defined(CONFIG_QUICC_ENGINE) || defined(CONFIG_8xx)
extern u32 get_brgfreq(void);
extern u32 get_baudrate(void);
#else
static inline u32 get_brgfreq(void) { return -1; }
static inline u32 get_baudrate(void) { return -1; }
#endif
extern u32 fsl_get_sys_freq(void);

struct spi_board_info;
struct device_node;

extern void fsl_rstcr_restart(char *cmd);

#ifdef CONFIG_FSL_PMC
int mpc85xx_pmc_set_wake(struct device *dev, bool enable);
void mpc85xx_pmc_set_lossless_ethernet(int enable);
#else
static inline int mpc85xx_pmc_set_wake(struct device *dev, bool enable)
{
	return -ENODEV;
}
#define mpc85xx_pmc_set_lossless_ethernet(enable)	do { } while (0)
#endif

#if defined(CONFIG_FB_FSL_DIU) || defined(CONFIG_FB_FSL_DIU_MODULE)

/* The different ports that the DIU can be connected to */
enum fsl_diu_monitor_port {
	FSL_DIU_PORT_DVI,	/* DVI */
	FSL_DIU_PORT_LVDS,	/* Single-link LVDS */
	FSL_DIU_PORT_DLVDS	/* Dual-link LVDS */
};

struct platform_diu_data_ops {
	u32 (*get_pixel_format)(enum fsl_diu_monitor_port port,
		unsigned int bpp);
	void (*set_gamma_table)(enum fsl_diu_monitor_port port,
		char *gamma_table_base);
	void (*set_monitor_port)(enum fsl_diu_monitor_port port);
	void (*set_pixel_clock)(unsigned int pixclock);
	enum fsl_diu_monitor_port (*valid_monitor_port)
		(enum fsl_diu_monitor_port port);
	void (*release_bootmem)(void);
};

extern struct platform_diu_data_ops diu_ops;
#endif

void fsl_hv_restart(char *cmd);
void fsl_hv_halt(void);

/*
 * Cast the ccsrbar to 64-bit parameter so that the assembly
 * code can be compatible with both 32-bit & 36-bit.
 */
extern void mpc85xx_enter_deep_sleep(u64 ccsrbar, u32 powmgtreq);

static inline void mpc85xx_enter_jog(u64 ccsrbar, u32 powmgtreq)
{
	mpc85xx_enter_deep_sleep(ccsrbar, powmgtreq);
}

struct fsl_pm_ops {
	void (*irq_mask)(int cpu);
	void (*irq_unmask)(int cpu);
	void (*cpu_enter_state)(int cpu, int state);
	void (*cpu_exit_state)(int cpu, int state);
	int (*plat_enter_state)(int state);
	void (*freeze_time_base)(int freeze);
	void (*set_ip_power)(int enable, u32 mask);
};

extern const struct fsl_pm_ops *qoriq_pm_ops;

#define E500_PM_PH10	1
#define E500_PM_PH15	2
#define E500_PM_PH20	3
#define E500_PM_PH30	4
#define E500_PM_DOZE	E500_PM_PH10
#define E500_PM_NAP	E500_PM_PH15

extern int sleep_pm_state;

#define PLAT_PM_SLEEP	20
#define PLAT_PM_LPM20	30

#ifdef CONFIG_T104x_DEEPSLEEP
extern int fsl_dp_iomap(void);
extern void fsl_dp_iounmap(void);

extern int fsl_enter_epu_deepsleep(void);
extern void __entry_deep_sleep(void);

extern void fsl_dp_fsm_setup(void *dcsr_base);
extern void fsl_dp_fsm_clean(void *dcsr_base);

extern void fsl_dp_enter_low(void *ccsr_base, void *dcsr_base,
				void *pld_base, int pld_flag);
#endif

extern int fsl_rcpm_init(void);
#endif
#endif
