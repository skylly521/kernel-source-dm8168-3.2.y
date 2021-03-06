/*
 * linux/arch/arm/mach-omap2/devices.c
 *
 * OMAP2 platform device setup/initialization
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
#include <linux/gpio.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/davinci_emac.h>
#include <linux/ahci_platform.h>

#include <mach/hardware.h>
#include <mach/irqs.h>
#include <asm/mach-types.h>
#include <asm/mach/map.h>
#include <asm/pmu.h>
#include <asm/hardware/edma.h>

#include <plat/tc.h>
#include <plat/board.h>
#include <plat/mcbsp.h>
#include <plat/mmc.h>
#include <plat/dma.h>
#include <plat/omap_hwmod.h>
#include <plat/omap_device.h>
#include <plat/omap4-keypad.h>
#include <plat/asp.h>

#include "smartreflex.h"
#include "pcie-ti81xx.h"

#include "mux.h"
#include "control.h"
#include "devices.h"

#define L3_MODULES_MAX_LEN 12
#define L3_MODULES 3

static int __init omap3_l3_init(void)
{
	int l;
	struct omap_hwmod *oh;
	struct platform_device *pdev;
	char oh_name[L3_MODULES_MAX_LEN];

	/*
	 * To avoid code running on other OMAPs in
	 * multi-omap builds
	 */
	if (!(cpu_is_omap34xx()))
		return -ENODEV;

	l = snprintf(oh_name, L3_MODULES_MAX_LEN, "l3_main");

	oh = omap_hwmod_lookup(oh_name);

	if (!oh)
		pr_err("could not look up %s\n", oh_name);

	pdev = omap_device_build("omap_l3_smx", 0, oh, NULL, 0,
							   NULL, 0, 0);

	WARN(IS_ERR(pdev), "could not build omap_device for %s\n", oh_name);

	return IS_ERR(pdev) ? PTR_ERR(pdev) : 0;
}
postcore_initcall(omap3_l3_init);

static int __init omap4_l3_init(void)
{
	int l, i;
	struct omap_hwmod *oh[3];
	struct platform_device *pdev;
	char oh_name[L3_MODULES_MAX_LEN];

	/* If dtb is there, the devices will be created dynamically */
	if (of_have_populated_dt())
		return -ENODEV;

	/*
	 * To avoid code running on other OMAPs in
	 * multi-omap builds
	 */
	if (!(cpu_is_omap44xx()))
		return -ENODEV;

	for (i = 0; i < L3_MODULES; i++) {
		l = snprintf(oh_name, L3_MODULES_MAX_LEN, "l3_main_%d", i+1);

		oh[i] = omap_hwmod_lookup(oh_name);
		if (!(oh[i]))
			pr_err("could not look up %s\n", oh_name);
	}

	pdev = omap_device_build_ss("omap_l3_noc", 0, oh, 3, NULL,
						     0, NULL, 0, 0);

	WARN(IS_ERR(pdev), "could not build omap_device for %s\n", oh_name);

	return IS_ERR(pdev) ? PTR_ERR(pdev) : 0;
}
postcore_initcall(omap4_l3_init);

#if defined(CONFIG_VIDEO_OMAP2) || defined(CONFIG_VIDEO_OMAP2_MODULE)

static struct resource omap2cam_resources[] = {
	{
		.start		= OMAP24XX_CAMERA_BASE,
		.end		= OMAP24XX_CAMERA_BASE + 0xfff,
		.flags		= IORESOURCE_MEM,
	},
	{
		.start		= INT_24XX_CAM_IRQ,
		.flags		= IORESOURCE_IRQ,
	}
};

static struct platform_device omap2cam_device = {
	.name		= "omap24xxcam",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(omap2cam_resources),
	.resource	= omap2cam_resources,
};
#endif

static struct resource omap3isp_resources[] = {
	{
		.start		= OMAP3430_ISP_BASE,
		.end		= OMAP3430_ISP_END,
		.flags		= IORESOURCE_MEM,
	},
	{
		.start		= OMAP3430_ISP_CCP2_BASE,
		.end		= OMAP3430_ISP_CCP2_END,
		.flags		= IORESOURCE_MEM,
	},
	{
		.start		= OMAP3430_ISP_CCDC_BASE,
		.end		= OMAP3430_ISP_CCDC_END,
		.flags		= IORESOURCE_MEM,
	},
	{
		.start		= OMAP3430_ISP_HIST_BASE,
		.end		= OMAP3430_ISP_HIST_END,
		.flags		= IORESOURCE_MEM,
	},
	{
		.start		= OMAP3430_ISP_H3A_BASE,
		.end		= OMAP3430_ISP_H3A_END,
		.flags		= IORESOURCE_MEM,
	},
	{
		.start		= OMAP3430_ISP_PREV_BASE,
		.end		= OMAP3430_ISP_PREV_END,
		.flags		= IORESOURCE_MEM,
	},
	{
		.start		= OMAP3430_ISP_RESZ_BASE,
		.end		= OMAP3430_ISP_RESZ_END,
		.flags		= IORESOURCE_MEM,
	},
	{
		.start		= OMAP3430_ISP_SBL_BASE,
		.end		= OMAP3430_ISP_SBL_END,
		.flags		= IORESOURCE_MEM,
	},
	{
		.start		= OMAP3430_ISP_CSI2A_REGS1_BASE,
		.end		= OMAP3430_ISP_CSI2A_REGS1_END,
		.flags		= IORESOURCE_MEM,
	},
	{
		.start		= OMAP3430_ISP_CSIPHY2_BASE,
		.end		= OMAP3430_ISP_CSIPHY2_END,
		.flags		= IORESOURCE_MEM,
	},
	{
		.start		= OMAP3630_ISP_CSI2A_REGS2_BASE,
		.end		= OMAP3630_ISP_CSI2A_REGS2_END,
		.flags		= IORESOURCE_MEM,
	},
	{
		.start		= OMAP3630_ISP_CSI2C_REGS1_BASE,
		.end		= OMAP3630_ISP_CSI2C_REGS1_END,
		.flags		= IORESOURCE_MEM,
	},
	{
		.start		= OMAP3630_ISP_CSIPHY1_BASE,
		.end		= OMAP3630_ISP_CSIPHY1_END,
		.flags		= IORESOURCE_MEM,
	},
	{
		.start		= OMAP3630_ISP_CSI2C_REGS2_BASE,
		.end		= OMAP3630_ISP_CSI2C_REGS2_END,
		.flags		= IORESOURCE_MEM,
	},
	{
		.start		= INT_34XX_CAM_IRQ,
		.flags		= IORESOURCE_IRQ,
	}
};

static struct platform_device omap3isp_device = {
	.name		= "omap3isp",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(omap3isp_resources),
	.resource	= omap3isp_resources,
};

int omap3_init_camera(struct isp_platform_data *pdata)
{
	omap3isp_device.dev.platform_data = pdata;
	return platform_device_register(&omap3isp_device);
}

static inline void omap_init_camera(void)
{
#if defined(CONFIG_VIDEO_OMAP2) || defined(CONFIG_VIDEO_OMAP2_MODULE)
	if (cpu_is_omap24xx())
		platform_device_register(&omap2cam_device);
#endif
}

int __init omap4_keyboard_init(struct omap4_keypad_platform_data
			*sdp4430_keypad_data, struct omap_board_data *bdata)
{
	struct platform_device *pdev;
	struct omap_hwmod *oh;
	struct omap4_keypad_platform_data *keypad_data;
	unsigned int id = -1;
	char *oh_name = "kbd";
	char *name = "omap4-keypad";

	oh = omap_hwmod_lookup(oh_name);
	if (!oh) {
		pr_err("Could not look up %s\n", oh_name);
		return -ENODEV;
	}

	keypad_data = sdp4430_keypad_data;

	pdev = omap_device_build(name, id, oh, keypad_data,
			sizeof(struct omap4_keypad_platform_data), NULL, 0, 0);

	if (IS_ERR(pdev)) {
		WARN(1, "Can't build omap_device for %s:%s.\n",
						name, oh->name);
		return PTR_ERR(pdev);
	}
	oh->mux = omap_hwmod_mux_init(bdata->pads, bdata->pads_cnt);

	return 0;
}

#if defined(CONFIG_OMAP_MBOX_FWK) || defined(CONFIG_OMAP_MBOX_FWK_MODULE)
static inline void omap_init_mbox(void)
{
	struct omap_hwmod *oh;
	struct platform_device *pdev;

	oh = omap_hwmod_lookup("mailbox");
	if (!oh) {
		pr_err("%s: unable to find hwmod\n", __func__);
		return;
	}

	pdev = omap_device_build("omap-mailbox", -1, oh, NULL, 0, NULL, 0, 0);
	WARN(IS_ERR(pdev), "%s: could not build device, err %ld\n",
						__func__, PTR_ERR(pdev));
}
#else
static inline void omap_init_mbox(void) { }
#endif /* CONFIG_OMAP_MBOX_FWK */

static inline void omap_init_sti(void) {}

#if defined(CONFIG_SND_SOC) || defined(CONFIG_SND_SOC_MODULE)

#if defined(CONFIG_SOC_OMAPTI81XX)
struct platform_device ti81xx_pcm_device = {
	.name		= "davinci-pcm-audio",
	.id		= -1,
};

static void ti81xx_init_pcm(void)
{
	platform_device_register(&ti81xx_pcm_device);
}
#else
static struct platform_device omap_pcm = {
	.name	= "omap-pcm-audio",
	.id	= -1,
};

/*
 * OMAP2420 has 2 McBSP ports
 * OMAP2430 has 5 McBSP ports
 * OMAP3 has 5 McBSP ports
 * OMAP4 has 4 McBSP ports
 */
OMAP_MCBSP_PLATFORM_DEVICE(1);
OMAP_MCBSP_PLATFORM_DEVICE(2);
OMAP_MCBSP_PLATFORM_DEVICE(3);
OMAP_MCBSP_PLATFORM_DEVICE(4);
OMAP_MCBSP_PLATFORM_DEVICE(5);

static void omap_init_audio(void)
{
	platform_device_register(&omap_mcbsp1);
	platform_device_register(&omap_mcbsp2);
	if (cpu_is_omap243x() || cpu_is_omap34xx() || cpu_is_omap44xx()) {
		platform_device_register(&omap_mcbsp3);
		platform_device_register(&omap_mcbsp4);
	}
	if (cpu_is_omap243x() || cpu_is_omap34xx())
		platform_device_register(&omap_mcbsp5);

	platform_device_register(&omap_pcm);
}

#endif /* defined(CONFIG_SOC_OMAPTI81XX) */

#else
static inline void omap_init_audio(void) {}
#endif

#if defined(CONFIG_SND_OMAP_SOC_MCPDM) || \
		defined(CONFIG_SND_OMAP_SOC_MCPDM_MODULE)

static void omap_init_mcpdm(void)
{
	struct omap_hwmod *oh;
	struct platform_device *pdev;

	oh = omap_hwmod_lookup("mcpdm");
	if (!oh) {
		printk(KERN_ERR "Could not look up mcpdm hw_mod\n");
		return;
	}

	pdev = omap_device_build("omap-mcpdm", -1, oh, NULL, 0, NULL, 0, 0);
	WARN(IS_ERR(pdev), "Can't build omap_device for omap-mcpdm.\n");
}
#else
static inline void omap_init_mcpdm(void) {}
#endif

#if defined(CONFIG_SPI_OMAP24XX) || defined(CONFIG_SPI_OMAP24XX_MODULE)

#include <plat/mcspi.h>

static int omap_mcspi_init(struct omap_hwmod *oh, void *unused)
{
	struct platform_device *pdev;
	char *name = "omap2_mcspi";
	struct omap2_mcspi_platform_config *pdata;
	static int spi_num;
	struct omap2_mcspi_dev_attr *mcspi_attrib = oh->dev_attr;

	pdata = kzalloc(sizeof(*pdata), GFP_KERNEL);
	if (!pdata) {
		pr_err("Memory allocation for McSPI device failed\n");
		return -ENOMEM;
	}

	pdata->num_cs = mcspi_attrib->num_chipselect;
	switch (oh->class->rev) {
	case OMAP2_MCSPI_REV:
	case OMAP3_MCSPI_REV:
			pdata->regs_offset = 0;
			break;
	case OMAP4_MCSPI_REV:
			pdata->regs_offset = OMAP4_MCSPI_REG_OFFSET;
			break;
	default:
			pr_err("Invalid McSPI Revision value\n");
			return -EINVAL;
	}

	spi_num++;
	pdev = omap_device_build(name, spi_num, oh, pdata,
				sizeof(*pdata),	NULL, 0, 0);
	WARN(IS_ERR(pdev), "Can't build omap_device for %s:%s\n",
				name, oh->name);
	kfree(pdata);
	return 0;
}

static void omap_init_mcspi(void)
{
	omap_hwmod_for_each_by_class("mcspi", omap_mcspi_init, NULL);
}

#else
static inline void omap_init_mcspi(void) {}
#endif

static struct resource omap2_pmu_resource = {
	.start	= 3,
	.end	= 3,
	.flags	= IORESOURCE_IRQ,
};

static struct resource omap3_pmu_resource = {
	.start	= INT_34XX_BENCH_MPU_EMUL,
	.end	= INT_34XX_BENCH_MPU_EMUL,
	.flags	= IORESOURCE_IRQ,
};

static struct platform_device omap_pmu_device = {
	.name		= "arm-pmu",
	.id		= ARM_PMU_DEVICE_CPU,
	.num_resources	= 1,
};

static void omap_init_pmu(void)
{
	if (cpu_is_omap24xx())
		omap_pmu_device.resource = &omap2_pmu_resource;
	else if (cpu_is_omap34xx())
		omap_pmu_device.resource = &omap3_pmu_resource;
	else
		return;

	platform_device_register(&omap_pmu_device);
}


#if defined(CONFIG_CRYPTO_DEV_OMAP_SHAM) || defined(CONFIG_CRYPTO_DEV_OMAP_SHAM_MODULE)

#ifdef CONFIG_ARCH_OMAP2
static struct resource omap2_sham_resources[] = {
	{
		.start	= OMAP24XX_SEC_SHA1MD5_BASE,
		.end	= OMAP24XX_SEC_SHA1MD5_BASE + 0x64,
		.flags	= IORESOURCE_MEM,
	},
	{
		.start	= INT_24XX_SHA1MD5,
		.flags	= IORESOURCE_IRQ,
	}
};
static int omap2_sham_resources_sz = ARRAY_SIZE(omap2_sham_resources);
#else
#define omap2_sham_resources		NULL
#define omap2_sham_resources_sz		0
#endif

#ifdef CONFIG_ARCH_OMAP3
static struct resource omap3_sham_resources[] = {
	{
		.start	= OMAP34XX_SEC_SHA1MD5_BASE,
		.end	= OMAP34XX_SEC_SHA1MD5_BASE + 0x64,
		.flags	= IORESOURCE_MEM,
	},
	{
		.start	= INT_34XX_SHA1MD52_IRQ,
		.flags	= IORESOURCE_IRQ,
	},
	{
		.start	= OMAP34XX_DMA_SHA1MD5_RX,
		.flags	= IORESOURCE_DMA,
	}
};
static int omap3_sham_resources_sz = ARRAY_SIZE(omap3_sham_resources);
#else
#define omap3_sham_resources		NULL
#define omap3_sham_resources_sz		0
#endif

static struct platform_device sham_device = {
	.name		= "omap-sham",
	.id		= -1,
};

static void omap_init_sham(void)
{
	if (cpu_is_omap24xx()) {
		sham_device.resource = omap2_sham_resources;
		sham_device.num_resources = omap2_sham_resources_sz;
	} else if (cpu_is_omap34xx()) {
		sham_device.resource = omap3_sham_resources;
		sham_device.num_resources = omap3_sham_resources_sz;
	} else {
		pr_err("%s: platform not supported\n", __func__);
		return;
	}
	platform_device_register(&sham_device);
}
#else
static inline void omap_init_sham(void) { }
#endif

#if defined(CONFIG_CRYPTO_DEV_OMAP_AES) || defined(CONFIG_CRYPTO_DEV_OMAP_AES_MODULE)

#ifdef CONFIG_ARCH_OMAP2
static struct resource omap2_aes_resources[] = {
	{
		.start	= OMAP24XX_SEC_AES_BASE,
		.end	= OMAP24XX_SEC_AES_BASE + 0x4C,
		.flags	= IORESOURCE_MEM,
	},
	{
		.start	= OMAP24XX_DMA_AES_TX,
		.flags	= IORESOURCE_DMA,
	},
	{
		.start	= OMAP24XX_DMA_AES_RX,
		.flags	= IORESOURCE_DMA,
	}
};
static int omap2_aes_resources_sz = ARRAY_SIZE(omap2_aes_resources);
#else
#define omap2_aes_resources		NULL
#define omap2_aes_resources_sz		0
#endif

#ifdef CONFIG_ARCH_OMAP3
static struct resource omap3_aes_resources[] = {
	{
		.start	= OMAP34XX_SEC_AES_BASE,
		.end	= OMAP34XX_SEC_AES_BASE + 0x4C,
		.flags	= IORESOURCE_MEM,
	},
	{
		.start	= OMAP34XX_DMA_AES2_TX,
		.flags	= IORESOURCE_DMA,
	},
	{
		.start	= OMAP34XX_DMA_AES2_RX,
		.flags	= IORESOURCE_DMA,
	}
};
static int omap3_aes_resources_sz = ARRAY_SIZE(omap3_aes_resources);
#else
#define omap3_aes_resources		NULL
#define omap3_aes_resources_sz		0
#endif

static struct platform_device aes_device = {
	.name		= "omap-aes",
	.id		= -1,
};

static void omap_init_aes(void)
{
	if (cpu_is_omap24xx()) {
		aes_device.resource = omap2_aes_resources;
		aes_device.num_resources = omap2_aes_resources_sz;
	} else if (cpu_is_omap34xx()) {
		aes_device.resource = omap3_aes_resources;
		aes_device.num_resources = omap3_aes_resources_sz;
	} else {
		pr_err("%s: platform not supported\n", __func__);
		return;
	}
	platform_device_register(&aes_device);
}

#else
static inline void omap_init_aes(void) { }
#endif

/*-------------------------------------------------------------------------*/

#if defined(CONFIG_MMC_OMAP) || defined(CONFIG_MMC_OMAP_MODULE)

static inline void omap242x_mmc_mux(struct omap_mmc_platform_data
							*mmc_controller)
{
	if ((mmc_controller->slots[0].switch_pin > 0) && \
		(mmc_controller->slots[0].switch_pin < OMAP_MAX_GPIO_LINES))
		omap_mux_init_gpio(mmc_controller->slots[0].switch_pin,
					OMAP_PIN_INPUT_PULLUP);
	if ((mmc_controller->slots[0].gpio_wp > 0) && \
		(mmc_controller->slots[0].gpio_wp < OMAP_MAX_GPIO_LINES))
		omap_mux_init_gpio(mmc_controller->slots[0].gpio_wp,
					OMAP_PIN_INPUT_PULLUP);

	omap_mux_init_signal("sdmmc_cmd", 0);
	omap_mux_init_signal("sdmmc_clki", 0);
	omap_mux_init_signal("sdmmc_clko", 0);
	omap_mux_init_signal("sdmmc_dat0", 0);
	omap_mux_init_signal("sdmmc_dat_dir0", 0);
	omap_mux_init_signal("sdmmc_cmd_dir", 0);
	if (mmc_controller->slots[0].caps & MMC_CAP_4_BIT_DATA) {
		omap_mux_init_signal("sdmmc_dat1", 0);
		omap_mux_init_signal("sdmmc_dat2", 0);
		omap_mux_init_signal("sdmmc_dat3", 0);
		omap_mux_init_signal("sdmmc_dat_dir1", 0);
		omap_mux_init_signal("sdmmc_dat_dir2", 0);
		omap_mux_init_signal("sdmmc_dat_dir3", 0);
	}

	/*
	 * Use internal loop-back in MMC/SDIO Module Input Clock
	 * selection
	 */
	if (mmc_controller->slots[0].internal_clock) {
		u32 v = omap_ctrl_readl(OMAP2_CONTROL_DEVCONF0);
		v |= (1 << 24);
		omap_ctrl_writel(v, OMAP2_CONTROL_DEVCONF0);
	}
}

void __init omap242x_init_mmc(struct omap_mmc_platform_data **mmc_data)
{
	char *name = "mmci-omap";

	if (!mmc_data[0]) {
		pr_err("%s fails: Incomplete platform data\n", __func__);
		return;
	}

	omap242x_mmc_mux(mmc_data[0]);
	omap_mmc_add(name, 0, OMAP2_MMC1_BASE, OMAP2420_MMC_SIZE,
					INT_24XX_MMC_IRQ, mmc_data[0]);
}

#endif

/*-------------------------------------------------------------------------*/

#if defined(CONFIG_HDQ_MASTER_OMAP) || defined(CONFIG_HDQ_MASTER_OMAP_MODULE)
#if defined(CONFIG_SOC_OMAP2430) || defined(CONFIG_SOC_OMAP3430)
#define OMAP_HDQ_BASE	0x480B2000
#endif
static struct resource omap_hdq_resources[] = {
	{
		.start		= OMAP_HDQ_BASE,
		.end		= OMAP_HDQ_BASE + 0x1C,
		.flags		= IORESOURCE_MEM,
	},
	{
		.start		= INT_24XX_HDQ_IRQ,
		.flags		= IORESOURCE_IRQ,
	},
};
static struct platform_device omap_hdq_dev = {
	.name = "omap_hdq",
	.id = 0,
	.dev = {
		.platform_data = NULL,
	},
	.num_resources	= ARRAY_SIZE(omap_hdq_resources),
	.resource	= omap_hdq_resources,
};
static inline void omap_hdq_init(void)
{
	(void) platform_device_register(&omap_hdq_dev);
}
#else
static inline void omap_hdq_init(void) {}
#endif

/*---------------------------------------------------------------------------*/

#if defined(CONFIG_VIDEO_OMAP2_VOUT) || \
	defined(CONFIG_VIDEO_OMAP2_VOUT_MODULE)
#if defined(CONFIG_FB_OMAP2) || defined(CONFIG_FB_OMAP2_MODULE)
static struct resource omap_vout_resource[3 - CONFIG_FB_OMAP2_NUM_FBS] = {
};
#else
static struct resource omap_vout_resource[2] = {
};
#endif

static struct platform_device omap_vout_device = {
	.name		= "omap_vout",
	.num_resources	= ARRAY_SIZE(omap_vout_resource),
	.resource 	= &omap_vout_resource[0],
	.id		= -1,
};
static void omap_init_vout(void)
{
	if (platform_device_register(&omap_vout_device) < 0)
		printk(KERN_ERR "Unable to register OMAP-VOUT device\n");
}
#else
static inline void omap_init_vout(void) {}
#endif

/*-------------------------------------------------------------------------*/

#ifdef CONFIG_SOC_OMAPTI81XX

#define TI81XX_TPCC_BASE		0x49000000
#define TI81XX_TPTC0_BASE		0x49800000
#define TI81XX_TPTC1_BASE		0x49900000
#define TI81XX_TPTC2_BASE		0x49a00000
#define TI81XX_TPTC3_BASE		0x49b00000

#define TI81XX_SCM_BASE_EDMA		0x00000f90

static struct resource ti81xx_edma_resources[] = {
	{
		.name	= "edma_cc0",
		.start	= TI81XX_TPCC_BASE,
		.end	= TI81XX_TPCC_BASE + SZ_32K - 1,
		.flags	= IORESOURCE_MEM,
	},
	{
		.name	= "edma_tc0",
		.start	= TI81XX_TPTC0_BASE,
		.end	= TI81XX_TPTC0_BASE + SZ_1K - 1,
		.flags	= IORESOURCE_MEM,
	},
	{
		.name	= "edma_tc1",
		.start	= TI81XX_TPTC1_BASE,
		.end	= TI81XX_TPTC1_BASE + SZ_1K - 1,
		.flags	= IORESOURCE_MEM,
	},
	{
		.name	= "edma_tc2",
		.start	= TI81XX_TPTC2_BASE,
		.end	= TI81XX_TPTC2_BASE + SZ_1K - 1,
		.flags	= IORESOURCE_MEM,
	},
	{
		.name	= "edma_tc3",
		.start	= TI81XX_TPTC3_BASE,
		.end	= TI81XX_TPTC3_BASE + SZ_1K - 1,
		.flags	= IORESOURCE_MEM,
	},
	{
		.name	= "edma0",
		.start	= TI81XX_IRQ_EDMA_COMP,
		.flags	= IORESOURCE_IRQ,
	},
	{
		.name	= "edma0_err",
		.start	= TI81XX_IRQ_EDMA_ERR,
		.flags	= IORESOURCE_IRQ,
	},
};

static const s16 ti816x_dma_rsv_chans[][2] = {
	/* (offset, number) */
	{0, 4},
	{26, 6},
	{48, 4},
	{56, 8},
	{-1, -1}
};

static const s16 ti816x_dma_rsv_slots[][2] = {
	/* (offset, number) */
	{0, 4},
	{26, 6},
	{48, 4},
	{56, 8},
	{64, 127},
	{-1, -1}
};

/* Four Transfer Controllers on TI816X */
static const s8 ti816x_queue_tc_mapping[][2] = {
	/* {event queue no, TC no} */
	{0, 0},
	{1, 1},
	{2, 2},
	{3, 3},
	{-1, -1}
};

static const s8 ti816x_queue_priority_mapping[][2] = {
	/* {event queue no, Priority} */
	{0, 0},
	{1, 1},
	{2, 2},
	{3, 3},
	{-1, -1}
};

static struct edma_soc_info ti816x_edma_info[] = {
	{
		.n_channel		= 64,
		.n_region		= 5,	/* 0-2, 4-5 */
		.n_slot			= 512,
		.n_tc			= 4,
		.n_cc			= 1,
		.rsv_chans		= ti816x_dma_rsv_chans,
		.rsv_slots		= ti816x_dma_rsv_slots,
		.queue_tc_mapping	= ti816x_queue_tc_mapping,
		.queue_priority_mapping	= ti816x_queue_priority_mapping,
	},
};

static struct platform_device ti816x_edma_device = {
	.name		= "edma",
	.id		= -1,
	.dev = {
		.platform_data = ti816x_edma_info,
	},
	.num_resources	= ARRAY_SIZE(ti81xx_edma_resources),
	.resource	= ti81xx_edma_resources,
};

int __init ti81xx_register_edma(void)
{
	struct platform_device *pdev;
	static struct clk *edma_clk;

	if (cpu_is_ti816x())
		pdev = &ti816x_edma_device;
	else {
		pr_err("%s: platform not supported\n", __func__);
		return -ENODEV;
	}

	edma_clk = clk_get(NULL, "tpcc_ick");
	if (IS_ERR(edma_clk)) {
		printk(KERN_ERR "EDMA: Failed to get clock\n");
		return -EBUSY;
	}
	clk_enable(edma_clk);
	edma_clk = clk_get(NULL, "tptc0_ick");
	if (IS_ERR(edma_clk)) {
		printk(KERN_ERR "EDMA: Failed to get clock\n");
		return -EBUSY;
	}
	clk_enable(edma_clk);
	edma_clk = clk_get(NULL, "tptc1_ick");
	if (IS_ERR(edma_clk)) {
		printk(KERN_ERR "EDMA: Failed to get clock\n");
		return -EBUSY;
	}
	clk_enable(edma_clk);
	edma_clk = clk_get(NULL, "tptc2_ick");
	if (IS_ERR(edma_clk)) {
		printk(KERN_ERR "EDMA: Failed to get clock\n");
		return -EBUSY;
	}
	clk_enable(edma_clk);
	edma_clk = clk_get(NULL, "tptc3_ick");
	if (IS_ERR(edma_clk)) {
		printk(KERN_ERR "EDMA: Failed to get clock\n");
		return -EBUSY;
	}
	clk_enable(edma_clk);


	return platform_device_register(pdev);
}

#endif

#ifdef CONFIG_TI816X_SMARTREFLEX

/* smartreflex platform data */
#define TI816X_SR_HVT_CNTRL_OFFSET	0x06AC
#define TI816X_SR_SVT_CNTRL_OFFSET	0x06A8
#define TI816X_SR_HVT_ERR2VOLT_GAIN	0xD
#define TI816X_SR_SVT_ERR2VOLT_GAIN	0x12
#define TI816X_SR_HVT_ERR_MIN_LIMIT	0xF6
#define TI816X_SR_SVT_ERR_MIN_LIMIT	0xF8

/* Refer TRM to know the Err2VoltGain factor and MinError Limits
 * for different step sizes. Update this table for both the sensors
 * (HVT and SVT) according to your step size, default step size is
 * 15mV. Factors changing with step-size are e2v_gain, err_minlimit.
 * Don't forgot to change the step size in platform data structure,
 * ti816x_sr_pdata.
 */
static struct ti816x_sr_sdata sr_sensor_data[] = {
	{
		.efuse_offs	= TI816X_SR_HVT_CNTRL_OFFSET,
		.e2v_gain	= TI816X_SR_HVT_ERR2VOLT_GAIN,
		.err_minlimit	= TI816X_SR_HVT_ERR_MIN_LIMIT,
		.err_maxlimit	= 0x2,
		.err_weight	= 0x4,
		.senn_mod	= 0x1,
		.senp_mod	= 0x1,
	},
	{
		.efuse_offs	= TI816X_SR_SVT_CNTRL_OFFSET,
		.e2v_gain	= TI816X_SR_SVT_ERR2VOLT_GAIN,
		.err_minlimit	= TI816X_SR_SVT_ERR_MIN_LIMIT,
		.err_maxlimit	= 0x2,
		.err_weight	= 0x4,
		.senn_mod	= 0x1,
		.senp_mod	= 0x1,
	},
};

static struct ti816x_sr_platform_data ti816x_sr_pdata = {
	.vd_name		= "vdd_avs",
	.ip_type		= 2,
	.irq_delay		= 2000,
	.no_of_vds		= 1,
	.no_of_sens		= ARRAY_SIZE(sr_sensor_data),
	.vstep_size_uv		= 15000,
	.enable_on_init		= true,
	.sr_sdata		= sr_sensor_data,
};

static struct resource ti816x_sr_resources[] = {
	{
		.name	=	"sr_hvt",
		.start	=	TI816X_SR0_BASE,
		.end	=	TI816X_SR0_BASE + SZ_4K - 1,
		.flags	=	IORESOURCE_MEM,
	},
	{
		.name	=	"sr_hvt",
		.start	=	TI81XX_IRQ_SMRFLX0,
		.end	=	TI81XX_IRQ_SMRFLX0,
		.flags	=	IORESOURCE_IRQ,
	},
	{
		.name	=	"sr_svt",
		.start	=	TI816X_SR1_BASE,
		.end	=	TI816X_SR1_BASE + SZ_4K - 1,
		.flags	=	IORESOURCE_MEM,
	},
	{
		.name	=	"sr_svt",
		.start	=	TI81XX_IRQ_SMRFLX1,
		.end	=	TI81XX_IRQ_SMRFLX1,
		.flags	=	IORESOURCE_IRQ,
	},
};

/* VCORE for SR regulator init */
static struct platform_device ti816x_sr_device = {
	.name		= "smartreflex",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(ti816x_sr_resources),
	.resource	= ti816x_sr_resources,
	.dev = {
		.platform_data = &ti816x_sr_pdata,
	},
};

static void __init ti816x_sr_init(void)
{
	if (platform_device_register(&ti816x_sr_device))
		printk(KERN_ERR "failed to register ti816x_sr device\n");
	else
		printk(KERN_INFO "registered ti816x_sr device\n");
}
#endif

/*-------------------------------------------------------------------------*/

#ifdef CONFIG_MACH_TI8168EVM

#define TI816X_EMAC1_BASE		(0x4A100000)
#define TI816X_EMAC2_BASE		(0x4A120000)
#define TI816X_EMAC_CNTRL_OFFSET	(0x0)
#define TI816X_EMAC_CNTRL_MOD_OFFSET	(0x900)
#define TI816X_EMAC_CNTRL_RAM_OFFSET	(0x2000)
#define TI816X_EMAC_MDIO_OFFSET		(0x800)
#define TI816X_EMAC_CNTRL_RAM_SIZE	(0x2000)
#define TI816X_EMAC1_HW_RAM_ADDR	(0x4A102000)
#define TI816X_EMAC2_HW_RAM_ADDR	(0x4A122000)

#define TI816X_EMAC_PHY_MASK		(0xF)
#define TI816X_EMAC_MDIO_FREQ		(1000000)

static struct mdio_platform_data ti816x_mdio_pdata = {
	.bus_freq       = TI816X_EMAC_MDIO_FREQ,
};

static struct resource ti816x_mdio_resources[] = {
	{
		.start  = TI816X_EMAC1_BASE + TI816X_EMAC_MDIO_OFFSET,
		.end	= TI816X_EMAC1_BASE + TI816X_EMAC_MDIO_OFFSET +
		SZ_256 - 1,
		.flags  = IORESOURCE_MEM,
	}
};

static struct platform_device ti816x_mdio_device = {
	.name           = "davinci_mdio",
	.id             = 0,
	.num_resources  = ARRAY_SIZE(ti816x_mdio_resources),
	.resource	= ti816x_mdio_resources,
	.dev.platform_data = &ti816x_mdio_pdata,
};

static struct emac_platform_data ti816x_emac1_pdata = {
	.rmii_en	=	0,
	.phy_id		=	"0:01",
	.gigabit_en	=	1,
};

static struct emac_platform_data ti816x_emac2_pdata = {
	.rmii_en	=	0,
	.phy_id		=	"0:02",
	.gigabit_en	=	1,
};

static struct resource ti816x_emac1_resources[] = {
	{
		.start	=	TI816X_EMAC1_BASE,
		.end	=	TI816X_EMAC1_BASE + TI816X_EMAC_MDIO_OFFSET - 1,
		.flags	=	IORESOURCE_MEM,
	},
	{
		.start	=	TI816X_IRQ_MACRXTHR0,
		.end	=	TI816X_IRQ_MACRXTHR0,
		.flags	=	IORESOURCE_IRQ,
	},
	{
		.start	=	TI816X_IRQ_MACRXINT0,
		.end	=	TI816X_IRQ_MACRXINT0,
		.flags	=	IORESOURCE_IRQ,
	},
	{
		.start	=	TI816X_IRQ_MACTXINT0,
		.end	=	TI816X_IRQ_MACTXINT0,
		.flags	=	IORESOURCE_IRQ,
	},
	{
		.start	=	TI816X_IRQ_MACMISC0,
		.end	=	TI816X_IRQ_MACMISC0,
		.flags	=	IORESOURCE_IRQ,
	},
};

static struct resource ti816x_emac2_resources[] = {
	{
		.start	=	TI816X_EMAC2_BASE,
		.end	=	TI816X_EMAC2_BASE + 0x3FFF,
		.flags	=	IORESOURCE_MEM,
	},
	{
		.start	=	TI816X_IRQ_MACRXTHR1,
		.end	=	TI816X_IRQ_MACRXTHR1,
		.flags	=	IORESOURCE_IRQ,
	},
	{
		.start	=	TI816X_IRQ_MACRXINT1,
		.end	=	TI816X_IRQ_MACRXINT1,
		.flags	=	IORESOURCE_IRQ,
	},
	{
		.start	=	TI816X_IRQ_MACTXINT1,
		.end	=	TI816X_IRQ_MACTXINT1,
		.flags	=	IORESOURCE_IRQ,
	},
	{
		.start	=	TI816X_IRQ_MACMISC1,
		.end	=	TI816X_IRQ_MACMISC1,
		.flags	=	IORESOURCE_IRQ,
	},
};
static struct platform_device ti816x_emac1_device = {
	.name	=	"davinci_emac",
	.id	=	0,
	.num_resources	=	ARRAY_SIZE(ti816x_emac1_resources),
	.resource	=	ti816x_emac1_resources,
};

static struct platform_device ti816x_emac2_device = {
	.name	=	"davinci_emac",
	.id	=	1,
	.num_resources	=	ARRAY_SIZE(ti816x_emac2_resources),
	.resource	=	ti816x_emac2_resources,
};

void ti816x_emac_mux(void)
{
	omap_mux_init_signal("gmii1_rxclk", OMAP_MUX_MODE1);
	omap_mux_init_signal("gmii1_rxd0", OMAP_MUX_MODE1);
	omap_mux_init_signal("gmii1_rxd1", OMAP_MUX_MODE1);
	omap_mux_init_signal("gmii1_rxd2", OMAP_MUX_MODE1);
	omap_mux_init_signal("gmii1_rxd3", OMAP_MUX_MODE1);
	omap_mux_init_signal("gmii1_rxd4", OMAP_MUX_MODE1);
	omap_mux_init_signal("gmii1_rxd5", OMAP_MUX_MODE1);
	omap_mux_init_signal("gmii1_rxd6", OMAP_MUX_MODE1);
	omap_mux_init_signal("gmii1_rxd7", OMAP_MUX_MODE1);
	omap_mux_init_signal("gmii1_rxdv", OMAP_MUX_MODE1);
	omap_mux_init_signal("gmii1_gtxclk", OMAP_MUX_MODE1);
	omap_mux_init_signal("gmii1_txd0", OMAP_MUX_MODE1);
	omap_mux_init_signal("gmii1_txd1", OMAP_MUX_MODE1);
	omap_mux_init_signal("gmii1_txd2", OMAP_MUX_MODE1);
	omap_mux_init_signal("gmii1_txd3", OMAP_MUX_MODE1);
	omap_mux_init_signal("gmii1_txd4", OMAP_MUX_MODE1);
	omap_mux_init_signal("gmii1_txd5", OMAP_MUX_MODE1);
	omap_mux_init_signal("gmii1_txd6", OMAP_MUX_MODE1);
	omap_mux_init_signal("gmii1_txd7", OMAP_MUX_MODE1);
	omap_mux_init_signal("gmii1_txen", OMAP_MUX_MODE1);
	omap_mux_init_signal("gmii1_txclk", OMAP_MUX_MODE1);
	omap_mux_init_signal("gmii1_col", OMAP_MUX_MODE1);
	omap_mux_init_signal("gmii1_crs", OMAP_MUX_MODE1);
	omap_mux_init_signal("gmii1_rxer", OMAP_MUX_MODE1);
}

void ti816x_ethernet_init(void)
{
	u32 mac_lo, mac_hi;

	mac_lo = omap_ctrl_readl(TI81XX_CONTROL_MAC_ID0_LO);
	mac_hi = omap_ctrl_readl(TI81XX_CONTROL_MAC_ID0_HI);
	ti816x_emac1_pdata.mac_addr[0] = mac_hi & 0xFF;
	ti816x_emac1_pdata.mac_addr[1] = (mac_hi & 0xFF00) >> 8;
	ti816x_emac1_pdata.mac_addr[2] = (mac_hi & 0xFF0000) >> 16;
	ti816x_emac1_pdata.mac_addr[3] = (mac_hi & 0xFF000000) >> 24;
	ti816x_emac1_pdata.mac_addr[4] = mac_lo & 0xFF;
	ti816x_emac1_pdata.mac_addr[5] = (mac_lo & 0xFF00) >> 8;

	ti816x_emac1_pdata.ctrl_reg_offset = TI816X_EMAC_CNTRL_OFFSET;
	ti816x_emac1_pdata.ctrl_mod_reg_offset = TI816X_EMAC_CNTRL_MOD_OFFSET;
	ti816x_emac1_pdata.ctrl_ram_offset = TI816X_EMAC_CNTRL_RAM_OFFSET;
	ti816x_emac1_pdata.ctrl_ram_size = TI816X_EMAC_CNTRL_RAM_SIZE;
	ti816x_emac1_pdata.version = EMAC_VERSION_2;
	ti816x_emac1_pdata.hw_ram_addr = TI816X_EMAC1_HW_RAM_ADDR;
	ti816x_emac1_pdata.interrupt_enable = NULL;
	ti816x_emac1_pdata.interrupt_disable = NULL;
	ti816x_emac1_device.dev.platform_data = &ti816x_emac1_pdata;
	platform_device_register(&ti816x_emac1_device);

	platform_device_register(&ti816x_mdio_device);
	clk_add_alias(NULL, dev_name(&ti816x_mdio_device.dev),
				  NULL, &ti816x_emac1_device.dev);

	mac_lo = omap_ctrl_readl(TI81XX_CONTROL_MAC_ID1_LO);
	mac_hi = omap_ctrl_readl(TI81XX_CONTROL_MAC_ID1_HI);
	ti816x_emac2_pdata.mac_addr[0] = mac_hi & 0xFF;
	ti816x_emac2_pdata.mac_addr[1] = (mac_hi & 0xFF00) >> 8;
	ti816x_emac2_pdata.mac_addr[2] = (mac_hi & 0xFF0000) >> 16;
	ti816x_emac2_pdata.mac_addr[3] = (mac_hi & 0xFF000000) >> 24;
	ti816x_emac2_pdata.mac_addr[4] = mac_lo & 0xFF;
	ti816x_emac2_pdata.mac_addr[5] = (mac_lo & 0xFF00) >> 8;

	ti816x_emac2_pdata.ctrl_reg_offset = TI816X_EMAC_CNTRL_OFFSET;
	ti816x_emac2_pdata.ctrl_mod_reg_offset = TI816X_EMAC_CNTRL_MOD_OFFSET;
	ti816x_emac2_pdata.ctrl_ram_offset = TI816X_EMAC_CNTRL_RAM_OFFSET;
	ti816x_emac2_pdata.ctrl_ram_size = TI816X_EMAC_CNTRL_RAM_SIZE;
	ti816x_emac2_pdata.version = EMAC_VERSION_2;
	ti816x_emac2_pdata.hw_ram_addr = TI816X_EMAC2_HW_RAM_ADDR;
	ti816x_emac2_pdata.interrupt_enable = NULL;
	ti816x_emac2_pdata.interrupt_disable = NULL;
	ti816x_emac2_device.dev.platform_data = &ti816x_emac2_pdata;
	platform_device_register(&ti816x_emac2_device);

	ti816x_emac_mux();
}
#else
static inline void ti816x_ethernet_init(void) {}
#endif

/*-------------------------------------------------------------------------*/

#if defined(CONFIG_SOC_OMAPTI81XX) && defined(CONFIG_PCI)
static struct ti81xx_pcie_data ti81xx_pcie_data = {
	.msi_irq_base	= MSI_IRQ_BASE,
	.msi_irq_num	= MSI_NR_IRQS,
};

static struct resource ti81xx_pcie_resources[] = {
	{
		/* Register space */
		.name		= "pcie-regs",
		.start		= TI816X_PCIE_REG_BASE,
		.end		= TI816X_PCIE_REG_BASE + SZ_16K - 1,
		.flags		= IORESOURCE_MEM,
	},
	{
		/* Non-prefetch memory */
		.name		= "pcie-nonprefetch",
		.start		= TI816X_PCIE_MEM_BASE,
		.end		= TI816X_PCIE_MEM_BASE + SZ_256M - 1,
		.flags		= IORESOURCE_MEM,
	},
	{
		/* IO window */
		.name		= "pcie-io",
		.start		= TI816X_PCIE_IO_BASE,
		.end		= TI816X_PCIE_IO_BASE + SZ_2M + SZ_1M - 1,
		.flags		= IORESOURCE_IO,
	},
	{
		/* Inbound memory window */
		.name		= "pcie-inbound0",
		.start		= (0x80000000),
		.end		= (0x80000000) + SZ_2G - 1,
		.flags		= IORESOURCE_MEM,
	},
	{
		/* Legacy Interrupt */
		.name		= "legacy_int",
		.start		= TI81XX_IRQ_PCIINT0,
		.end		= TI81XX_IRQ_PCIINT0,
		.flags		= IORESOURCE_IRQ,
	},
#ifdef CONFIG_PCI_MSI
	{
		/* MSI Interrupt Line */
		.name		= "msi_int",
		.start		= TI81XX_IRQ_PCIINT1,
		.end		= TI81XX_IRQ_PCIINT1,
		.flags		= IORESOURCE_IRQ,
	},
#endif
};

static struct platform_device ti81xx_pcie_device = {
	.name		= "ti81xx_pcie",
	.id		= 0,
	.dev		= {
		.platform_data = &ti81xx_pcie_data,
	},
	.num_resources	= ARRAY_SIZE(ti81xx_pcie_resources),
	.resource	= ti81xx_pcie_resources,
};

static inline void ti81xx_init_pcie(void)
{
	if (!cpu_is_ti81xx())
		return;

	if (cpu_is_ti816x()) {
		omap_ctrl_writel(TI816X_PCIE_PLLMUX_25X |
						 TI81XX_PCIE_DEVTYPE_RC,
						 TI816X_CONTROL_PCIE_CFG);

		/* MSI clearing is "write 0 to clear" */
		ti81xx_pcie_data.msi_inv = 1;

		ti81xx_pcie_data.device_id = 0xb800;
	} else if (cpu_is_ti814x()) {

		omap_ctrl_writel(TI81XX_PCIE_DEVTYPE_RC,
						 TI814X_CONTROL_PCIE_CFG);
		/*
		 * Force x1 lane as TI814X only supports x1 while the PCIe
		 * registers read x2 leading to wrong capability printed form
		 * PCIe configuration.
		 */
		ti81xx_pcie_data.force_x1 = 1;

		if (0 /* cpu_is_dm385() */)
			ti81xx_pcie_data.device_id = 0xb802;
		else
			ti81xx_pcie_data.device_id = 0xb801;
	}

	platform_device_register(&ti81xx_pcie_device);
}
#else
static inline void ti81xx_init_pcie(void) {}
#endif

/*-------------------------------------------------------------------------*/

#if defined(CONFIG_SATA_AHCI_PLATFORM) ||		\
	defined(CONFIG_SATA_AHCI_PLATFORM_MODULE)

static int ti81xx_ahci_plat_init(struct device *dev, void __iomem *base);
static void ti81xx_ahci_plat_exit(struct device *dev);

static struct ahci_platform_data omap_sata0_pdata = {
	.init	= ti81xx_ahci_plat_init,
	.exit	= ti81xx_ahci_plat_exit,
};

static u64 omap_sata_dmamask = DMA_BIT_MASK(32);

/* SATA PHY control register offsets */
#define SATA_P0PHYCR_REG	0x178
#define SATA_P1PHYCR_REG	0x1F8

#define SATA_PHY_ENPLL(x)	((x) << 0)
#define SATA_PHY_MPY(x)		((x) << 1)
#define SATA_PHY_LB(x)		((x) << 5)
#define SATA_PHY_CLKBYP(x)	((x) << 7)
#define SATA_PHY_RXINVPAIR(x)	((x) << 9)
#define SATA_PHY_LBK(x)		((x) << 10)
#define SATA_PHY_RXLOS(x)	((x) << 12)
#define SATA_PHY_RXCDR(x)	((x) << 13)
#define SATA_PHY_RXEQ(x)	((x) << 16)
#define SATA_PHY_RXENOC(x)	((x) << 20)
#define SATA_PHY_TXINVPAIR(x)	((x) << 21)
#define SATA_PHY_TXCM(x)	((x) << 22)
#define SATA_PHY_TXSWING(x)	((x) << 23)
#define SATA_PHY_TXDE(x)	((x) << 27)

#define TI81XX_SATA_BASE	0x4A140000

/* These values are tried and tested and not expected to change.
 * Hence not using a macro to generate them.
 */
#define TI814X_SATA_PHY_CFGRX0_VAL	0x008FCC22
#define TI814X_SATA_PHY_CFGRX1_VAL	0x008E0500
#define TI814X_SATA_PHY_CFGRX2_VAL	0x7BDEF000
#define TI814X_SATA_PHY_CFGRX3_VAL	0x1F180B0F
#define TI814X_SATA_PHY_CFGTX0_VAL	0x01003622
#define TI814X_SATA_PHY_CFGTX1_VAL	0x40000002
#define TI814X_SATA_PHY_CFGTX2_VAL	0x00C201F8
#define TI814X_SATA_PHY_CFGTX3_VAL	0x073CE39E

#define TI813X_SATA_PHY_CFGRX0_VAL	0x00C7CC22
#define TI813X_SATA_PHY_CFGRX1_VAL	0x008E0500
#define TI813X_SATA_PHY_CFGRX2_VAL	0x7BDEF000
#define TI813X_SATA_PHY_CFGRX3_VAL	0x1F180B0F
#define TI813X_SATA_PHY_CFGTX0_VAL	0x01001622
#define TI813X_SATA_PHY_CFGTX1_VAL	0x40000002
#define TI813X_SATA_PHY_CFGTX2_VAL	0x00000000
#define TI813X_SATA_PHY_CFGTX3_VAL	0x073CE39E

static int ti81xx_ahci_plat_init(struct device *dev, void __iomem *base)
{
	unsigned int phy_val;
	int ret;
	struct clk *sata_clk;

	sata_clk = clk_get(dev, NULL);
	if (IS_ERR(sata_clk)) {
		pr_err("ahci : Failed to get SATA clock\n");
		return PTR_ERR(sata_clk);
	}

	if (!base) {
		pr_err("ahci : SATA reg space not mapped, PHY enable failed\n");
		ret = -ENOMEM;
		goto err;
	}

	ret = clk_enable(sata_clk);
	if (ret) {
		pr_err("ahci : Clock enable failed\n");
		goto err;
	}

	if (cpu_is_ti816x()) {
		phy_val = SATA_PHY_ENPLL(1) |
			SATA_PHY_MPY(8) |
			SATA_PHY_LB(0) |
			SATA_PHY_CLKBYP(0) |
			SATA_PHY_RXINVPAIR(0) |
			SATA_PHY_LBK(0) |
			SATA_PHY_RXLOS(1) |
			SATA_PHY_RXCDR(4) |
			SATA_PHY_RXEQ(1) |
			SATA_PHY_RXENOC(1) |
			SATA_PHY_TXINVPAIR(0) |
			SATA_PHY_TXCM(0) |
			SATA_PHY_TXSWING(7) |
			SATA_PHY_TXDE(0);

		writel(phy_val, base + SATA_P0PHYCR_REG);
		writel(phy_val, base + SATA_P1PHYCR_REG);
	} else if (cpu_is_ti814x()) {

		if (0 /* cpu_is_dm385() */) {
			/* Configuring for 20Mhz clock source on TI813x */
			writel(TI813X_SATA_PHY_CFGRX0_VAL,
				   base + TI814X_SATA_PHY_CFGRX0_OFFSET);
			writel(TI813X_SATA_PHY_CFGRX1_VAL,
				   base + TI814X_SATA_PHY_CFGRX1_OFFSET);
			writel(TI813X_SATA_PHY_CFGRX2_VAL,
				   base + TI814X_SATA_PHY_CFGRX2_OFFSET);
			writel(TI813X_SATA_PHY_CFGRX3_VAL,
				   base + TI814X_SATA_PHY_CFGRX3_OFFSET);
			writel(TI813X_SATA_PHY_CFGTX0_VAL,
				   base + TI814X_SATA_PHY_CFGTX0_OFFSET);
			writel(TI813X_SATA_PHY_CFGTX1_VAL,
				   base + TI814X_SATA_PHY_CFGTX1_OFFSET);
			writel(TI813X_SATA_PHY_CFGTX2_VAL,
				   base + TI814X_SATA_PHY_CFGTX2_OFFSET);
			writel(TI813X_SATA_PHY_CFGTX3_VAL,
				   base + TI814X_SATA_PHY_CFGTX3_OFFSET);

		} else {
			/* Configuring for 100Mhz clock source on TI814x */
			writel(TI814X_SATA_PHY_CFGRX0_VAL,
				   base + TI814X_SATA_PHY_CFGRX0_OFFSET);
			writel(TI814X_SATA_PHY_CFGRX1_VAL,
				   base + TI814X_SATA_PHY_CFGRX1_OFFSET);
			writel(TI814X_SATA_PHY_CFGRX2_VAL,
				   base + TI814X_SATA_PHY_CFGRX2_OFFSET);
			writel(TI814X_SATA_PHY_CFGRX3_VAL,
				   base + TI814X_SATA_PHY_CFGRX3_OFFSET);
			writel(TI814X_SATA_PHY_CFGTX0_VAL,
				   base + TI814X_SATA_PHY_CFGTX0_OFFSET);
			writel(TI814X_SATA_PHY_CFGTX1_VAL,
				   base + TI814X_SATA_PHY_CFGTX1_OFFSET);
			writel(TI814X_SATA_PHY_CFGTX2_VAL,
				   base + TI814X_SATA_PHY_CFGTX2_OFFSET);
			writel(TI814X_SATA_PHY_CFGTX3_VAL,
				   base + TI814X_SATA_PHY_CFGTX3_OFFSET);
		}
	}

	return 0;
err:
	clk_put(sata_clk);
	return ret;
}

static void ti81xx_ahci_plat_exit(struct device *dev)
{
	struct clk *sata_clk;

	sata_clk = clk_get(dev, NULL);
	if (IS_ERR(sata_clk)) {
		pr_err("ahci : Failed to get SATA clock\n");
		return;
	}

	clk_disable(sata_clk);
	clk_put(sata_clk);
}

/* resources will be filled by soc specific init routine */
static struct resource omap_ahci0_resources[] = {
	{
		.start	= TI81XX_SATA_BASE,
		.end	= TI81XX_SATA_BASE + 0x10fff,
		.flags	= IORESOURCE_MEM,
	},
	{
		.start	= TI81XX_IRQ_SATA,
		.flags	= IORESOURCE_IRQ,
	}
};

static struct platform_device omap_ahci0_device = {
	.name	= "ahci",
	.id		= 0,
	.dev	= {
		.platform_data = &omap_sata0_pdata,
		.coherent_dma_mask	= DMA_BIT_MASK(32),
		.dma_mask		= &omap_sata_dmamask,
	},
	.num_resources	= ARRAY_SIZE(omap_ahci0_resources),
	.resource	= omap_ahci0_resources,
};

static inline void omap_init_ahci(void)
{
	if ((cpu_is_ti81xx()) /* && (!cpu_is_ti811x()) */) {
		platform_device_register(&omap_ahci0_device);
	}
}
#else
static inline void omap_init_ahci(void) {}
#endif

/*-------------------------------------------------------------------------*/

#if defined(CONFIG_SOC_OMAPTI81XX)
static struct resource dm385_mcasp_resource[] = {
	{
		.name = "mcasp",
		.start = TI81XX_ASP1_BASE,
		.end = TI81XX_ASP1_BASE + (SZ_1K * 12) - 1,
		.flags = IORESOURCE_MEM,
	},
	/* TX event */
	{
		.start = TI81XX_DMA_MCASP1_AXEVT,
		.end = TI81XX_DMA_MCASP1_AXEVT,
		.flags = IORESOURCE_DMA,
	},
	/* RX event */
	{
		.start = TI81XX_DMA_MCASP1_AREVT,
		.end = TI81XX_DMA_MCASP1_AREVT,
		.flags = IORESOURCE_DMA,
	},
};

static struct resource ti81xx_mcasp_resource[] = {
	{
		.name = "mcasp",
		.start = TI81XX_ASP2_BASE,
		.end = TI81XX_ASP2_BASE + (SZ_1K * 12) - 1,
		.flags = IORESOURCE_MEM,
	},
	/* TX event */
	{
		.start = TI81XX_DMA_MCASP2_AXEVT,
		.end = TI81XX_DMA_MCASP2_AXEVT,
		.flags = IORESOURCE_DMA,
	},
	/* RX event */
	{
		.start = TI81XX_DMA_MCASP2_AREVT,
		.end = TI81XX_DMA_MCASP2_AREVT,
		.flags = IORESOURCE_DMA,
	},
};

static struct platform_device ti81xx_mcasp_device = {
	.name = "davinci-mcasp",
};

void __init ti81xx_register_mcasp(int id, struct snd_platform_data *pdata)
{
	if (machine_is_ti8168evm() || machine_is_ti8148evm()) {
		ti81xx_mcasp_device.id = 2;
		ti81xx_mcasp_device.resource = ti81xx_mcasp_resource;
		ti81xx_mcasp_device.num_resources = ARRAY_SIZE(ti81xx_mcasp_resource);
	} else {
		pr_err("%s: platform not supported\n", __func__);
		return;
	}

	ti81xx_mcasp_device.dev.platform_data = pdata;
	platform_device_register(&ti81xx_mcasp_device);
}
#endif

/*-------------------------------------------------------------------------*/

static int __init omap2_init_devices(void)
{
	/*
	 * please keep these calls, and their implementations above,
	 * in alphabetical order so they're easier to sort through.
	 */
#if !defined(CONFIG_SOC_OMAPTI81XX)
	omap_init_audio();
#endif
	omap_init_mcpdm();
	omap_init_camera();
	omap_init_mbox();
	omap_init_mcspi();
	omap_init_pmu();
	omap_hdq_init();
	omap_init_sti();
	omap_init_sham();
	omap_init_aes();
	omap_init_vout();
#ifdef CONFIG_SOC_OMAPTI81XX
	ti81xx_register_edma();
	ti81xx_init_pcm();
#if defined(CONFIG_TI816X_SMARTREFLEX) && defined(CONFIG_MACH_TI8168EVM)
	ti816x_sr_init();
	ti816x_ethernet_init();
#endif
	ti81xx_init_pcie();
#endif
	omap_init_ahci();

	return 0;
}
arch_initcall(omap2_init_devices);

#if defined(CONFIG_OMAP_WATCHDOG) || defined(CONFIG_OMAP_WATCHDOG_MODULE)
static int __init omap_init_wdt(void)
{
	int id = -1;
	struct platform_device *pdev;
	struct omap_hwmod *oh;
	char *oh_name = "wd_timer2";
	char *dev_name = "omap_wdt";

	if (!cpu_class_is_omap2())
		return 0;

	oh = omap_hwmod_lookup(oh_name);
	if (!oh) {
		pr_err("Could not look up wd_timer%d hwmod\n", id);
		return -EINVAL;
	}

	pdev = omap_device_build(dev_name, id, oh, NULL, 0, NULL, 0, 0);
	WARN(IS_ERR(pdev), "Can't build omap_device for %s:%s.\n",
				dev_name, oh->name);
	return 0;
}
subsys_initcall(omap_init_wdt);
#endif
