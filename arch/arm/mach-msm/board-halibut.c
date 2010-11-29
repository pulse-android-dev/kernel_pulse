/* linux/arch/arm/mach-msm/board-halibut.c
 *
 * Copyright (C) 2007 Google, Inc.
 * Copyright (c) 2008-2009, Code Aurora Forum. All rights reserved.
 * Author: Brian Swetland <swetland@google.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/bootmem.h>
#include <linux/i2c.h>
#include <linux/android_pmem.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/partitions.h>

#include <linux/synaptics_i2c_rmi.h>
#include <mach/hardware.h>
#include <mach/irqs.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/flash.h>
#include <asm/setup.h>

#include <asm/mach/mmc.h>
#include <mach/vreg.h>
#include <mach/mpp.h>
#include <mach/gpio.h>
#include <mach/board.h>
#include <mach/msm_iomap.h>
#include <mach/msm_serial_hs.h>
#include <mach/msm_hsusb.h>
#include <mach/vreg.h>
#include <mach/msm_rpcrouter.h>
#include <mach/memory.h>
#include <mach/camera.h>

#ifdef CONFIG_USB_FUNCTION
#include <linux/usb/mass_storage_function.h>
#endif
#ifdef CONFIG_USB_ANDROID
#include <linux/usb/android.h>
#include <mach/rpc_hsusb.h>
#endif

#include "devices.h"
#include "keypad_linux_u8220.h"
#include "msm_vibrator.h"
#include "jogball_device.h"
#include "socinfo.h"
#include "msm-keypad-devices.h"
#include "pm.h"
#include "smd_private.h"
#define USB_SERIAL_LEN 20
static unsigned char usb_serial_num[USB_SERIAL_LEN]={'H', 'W'};

#ifdef CONFIG_MSM_STACKED_MEMORY
#define MSM_SMI_BASE		0x100000
#define MSM_SMI_SIZE		0x800000

#define MSM_PMEM_GPU0_BASE	MSM_SMI_BASE
#define MSM_PMEM_GPU0_SIZE	0x800000
#endif

/*GPU1 can't migrate to SMI  */

#define MSM_PMEM_GPU1_SIZE	0x800000

#define  MSM_FB_BASE      0x3E00000
#define  MSM_FB_SIZE	0x200000	// 2 M
#define MSM_PMEM_CAMERA_SIZE	0xa00000 //10M

#define MSM_PMEM_ADSP_BASE    0x3200000
#define MSM_PMEM_ADSP_SIZE    0xc00000 //12M
#define MSM_PMEM_MDP_SIZE	0x800000

static struct resource smc91x_resources[] = {
	[0] = {
		.start	= 0x9C004300,
		.end	= 0x9C004400,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= MSM_GPIO_TO_INT(49),
		.end	= MSM_GPIO_TO_INT(49),
		.flags	= IORESOURCE_IRQ,
	},
};

#ifdef CONFIG_USB_FUNCTION
static struct usb_mass_storage_platform_data usb_mass_storage_pdata = {
#if defined(CONFIG_HUAWEI_USB_PID_GENERAL_RELEASE)
    .nluns          = 0x01,
    .buf_size       = 16384,
    .vendor         = "Android",
    .product        = "Adapter",
#else
	.nluns          = 0x01,
	.buf_size       = 16384,
	.vendor 		= "T-Mobile",
	.product		= "3G Phone",
#endif	
	.release        = 0xffff,
};

static struct platform_device mass_storage_device = {
	.name           = "usb_mass_storage",
	.id             = -1,
	.dev            = {
		.platform_data          = &usb_mass_storage_pdata,
	},
};
#endif

#ifdef CONFIG_USB_ANDROID
static struct android_usb_platform_data android_usb_pdata = {
	.vendor_id	= 0x05C6,
	.product_id	= 0xF000,
	.adb_product_id	= 0x9015,
	.version	= 0x0100,
	.product_name	= "Qualcomm HSUSB Device",
	.manufacturer_name = "Qualcomm Incorporated",
	.nluns = 1,
};

static struct platform_device android_usb_device = {
	.name	= "android_usb",
	.id		= -1,
	.dev		= {
		.platform_data = &android_usb_pdata,
	},
};
#endif

#ifdef CONFIG_USB_FUNCTION
static void hsusb_gpio_init(void)
{
	if (gpio_request(111, "ulpi_data_0"))
		pr_err("failed to request gpio ulpi_data_0\n");
	if (gpio_request(112, "ulpi_data_1"))
		pr_err("failed to request gpio ulpi_data_1\n");
	if (gpio_request(113, "ulpi_data_2"))
		pr_err("failed to request gpio ulpi_data_2\n");
	if (gpio_request(114, "ulpi_data_3"))
		pr_err("failed to request gpio ulpi_data_3\n");
	if (gpio_request(115, "ulpi_data_4"))
		pr_err("failed to request gpio ulpi_data_4\n");
	if (gpio_request(116, "ulpi_data_5"))
		pr_err("failed to request gpio ulpi_data_5\n");
	if (gpio_request(117, "ulpi_data_6"))
		pr_err("failed to request gpio ulpi_data_6\n");
	if (gpio_request(118, "ulpi_data_7"))
		pr_err("failed to request gpio ulpi_data_7\n");
	if (gpio_request(119, "ulpi_dir"))
		pr_err("failed to request gpio ulpi_dir\n");
	if (gpio_request(120, "ulpi_next"))
		pr_err("failed to request gpio ulpi_next\n");
	if (gpio_request(121, "ulpi_stop"))
		pr_err("failed to request gpio ulpi_stop\n");
}

static unsigned usb_gpio_lpm_config[] = {
	GPIO_CFG(111, 1, GPIO_INPUT, GPIO_NO_PULL, GPIO_2MA),	/* DATA 0 */
	GPIO_CFG(112, 1, GPIO_INPUT, GPIO_NO_PULL, GPIO_2MA),	/* DATA 1 */
	GPIO_CFG(113, 1, GPIO_INPUT, GPIO_NO_PULL, GPIO_2MA),	/* DATA 2 */
	GPIO_CFG(114, 1, GPIO_INPUT, GPIO_NO_PULL, GPIO_2MA),	/* DATA 3 */
	GPIO_CFG(115, 1, GPIO_INPUT, GPIO_NO_PULL, GPIO_2MA),	/* DATA 4 */
	GPIO_CFG(116, 1, GPIO_INPUT, GPIO_NO_PULL, GPIO_2MA),	/* DATA 5 */
	GPIO_CFG(117, 1, GPIO_INPUT, GPIO_NO_PULL, GPIO_2MA),	/* DATA 6 */
	GPIO_CFG(118, 1, GPIO_INPUT, GPIO_NO_PULL, GPIO_2MA),	/* DATA 7 */
	GPIO_CFG(119, 1, GPIO_INPUT, GPIO_NO_PULL, GPIO_2MA),	/* DIR */
	GPIO_CFG(120, 1, GPIO_INPUT, GPIO_NO_PULL, GPIO_2MA),	/* NEXT */
	GPIO_CFG(121, 1, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA),	/* STOP */
};

static unsigned usb_gpio_lpm_unconfig[] = {
	GPIO_CFG(111, 1, GPIO_OUTPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DATA 0 */
	GPIO_CFG(112, 1, GPIO_OUTPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DATA 1 */
	GPIO_CFG(113, 1, GPIO_OUTPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DATA 2 */
	GPIO_CFG(114, 1, GPIO_OUTPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DATA 3 */
	GPIO_CFG(115, 1, GPIO_OUTPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DATA 4 */
	GPIO_CFG(116, 1, GPIO_OUTPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DATA 5 */
	GPIO_CFG(117, 1, GPIO_OUTPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DATA 6 */
	GPIO_CFG(118, 1, GPIO_OUTPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DATA 7 */
	GPIO_CFG(119, 1, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DIR */
	GPIO_CFG(120, 1, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* NEXT */
	GPIO_CFG(121, 1, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_2MA), /* STOP */
};

static int usb_config_gpio(int config)
{
	int pin, rc;

	if (config) {
		for (pin = 0; pin < ARRAY_SIZE(usb_gpio_lpm_config); pin++) {
			rc = gpio_tlmm_config(usb_gpio_lpm_config[pin],
					      GPIO_ENABLE);
			if (rc) {
				printk(KERN_ERR
				       "%s: gpio_tlmm_config(%#x)=%d\n",
				       __func__, usb_gpio_lpm_config[pin], rc);
				return -EIO;
			}
		}
	} else {
		for (pin = 0; pin < ARRAY_SIZE(usb_gpio_lpm_unconfig); pin++) {
			rc = gpio_tlmm_config(usb_gpio_lpm_unconfig[pin],
					      GPIO_ENABLE);
			if (rc) {
				printk(KERN_ERR
				       "%s: gpio_tlmm_config(%#x)=%d\n",
				       __func__, usb_gpio_lpm_config[pin], rc);
				return -EIO;
			}
		}
	}

	return 0;
}
#endif


static struct platform_device smc91x_device = {
	.name		= "smc91x",
	.id		= 0,
	.num_resources	= ARRAY_SIZE(smc91x_resources),
	.resource	= smc91x_resources,
};

#ifdef CONFIG_USB_FUNCTION
static struct usb_function_map usb_functions_map[] = {
	{"modem", 0},
	{"pcui", 1},
	{"mass_storage", 2},
	{"adb", 3},
	{"diag", 4},
};

/* dynamic composition */
static struct usb_composition usb_func_composition[] = {
	{
		.product_id         = 0x9012,
		.functions	    = 0x5, /* 0101 */
	},

	{
		.product_id         = 0x9013,
		.functions	    = 0x15, /* 10101 */
	},

	{
		.product_id         = 0x9014,
		.functions	    = 0x30, /* 110000 */
	},

	{
		.product_id         = 0x9015,
		.functions          = 0x12, /* 10010 */
	},

	{
		.product_id         = 0x9016,
		.functions	    = 0xD, /* 01101 */
	},

	{
		.product_id         = 0x9017,
		.functions	    = 0x1D, /* 11101 */
	},

	{
		.product_id         = 0xF000,
		.functions	    = 0x10, /* 10000 */
	},

	{
		.product_id         = 0xF009,
		.functions	    = 0x20, /* 100000 */
	},

	{
#if defined(CONFIG_HUAWEI_USB_PID_GENERAL_RELEASE)
        .product_id         = 0x1502,
#else
		.product_id         = 0x1501,
#endif
       .functions	    = 0x1F, /* 00011111 */
	},

	{
		.product_id         = 0x901A,
		.functions          = 0x0F, /* 01111 */
	},

};
#endif

#ifdef CONFIG_USB_ANDROID
static void hsusb_phy_reset(void)
{
	msm_hsusb_phy_reset();
}
#endif

static struct msm_hsusb_platform_data msm_hsusb_pdata = {
#ifdef CONFIG_USB_ANDROID
	.phy_reset	= hsusb_phy_reset,
#endif
#ifdef CONFIG_USB_FUNCTION
	.version	= 0x0100,
	.phy_info	= USB_PHY_EXTERNAL,
#if defined(CONFIG_HUAWEI_USB_PID_GENERAL_RELEASE)
    .vendor_id          = 0x12D1,
    .product_name       = "Android Mobile Adapter",
    .serial_number      = NULL,
    .manufacturer_name  = "Huawei Incorporated",
#else
	.vendor_id			= 0x12D1,
	.product_name		= "T-Mobile 3G Phone",
	.serial_number		= NULL,
	.manufacturer_name	= "Huawei Incorporated",
#endif
	.compositions	= usb_func_composition,
	.num_compositions = ARRAY_SIZE(usb_func_composition),
	.function_map   = usb_functions_map,
	.num_functions	= ARRAY_SIZE(usb_functions_map),
	.ulpi_data_1_pin = 112,
	.ulpi_data_3_pin = 114,
	.config_gpio 	= usb_config_gpio,
#endif
};

#define SND(desc, num) { .name = #desc, .id = num }
static struct snd_endpoint snd_endpoints_list[] = {
	SND(HANDSET, 0),
	SND(MONO_HEADSET, 2),
	SND(HEADSET, 3),
	SND(SPEAKER, 6),
	SND(BT, 12),
	SND(IN_S_SADC_OUT_HANDSET, 16),
	SND(IN_S_SADC_OUT_SPEAKER_PHONE, 25),
	SND(HEADSET_AND_SPEAKER,26),
	SND(BT_EC_OFF,27),
	SND(CURRENT, 29),
};
#undef SND

static struct msm_snd_endpoints halibut_snd_endpoints = {
	.endpoints = snd_endpoints_list,
	.num = sizeof(snd_endpoints_list) / sizeof(struct snd_endpoint)
};

static struct platform_device halibut_snd = {
	.name = "msm_snd",
	.id = -1,
	.dev    = {
		.platform_data = &halibut_snd_endpoints
	},
};

static struct android_pmem_platform_data android_pmem_pdata = {
	.name = "pmem",
	.no_allocator = 0,
	.cached = 1,
};

static struct android_pmem_platform_data android_pmem_camera_pdata = {
	.name = "pmem_camera",
	.no_allocator = 1,
	.cached = 1,
};

static struct android_pmem_platform_data android_pmem_adsp_pdata = {
	.name = "pmem_adsp",
	.no_allocator = 0,
	.cached = 0,
};

#ifdef CONFIG_MSM_STACKED_MEMORY
static struct android_pmem_platform_data android_pmem_gpu0_pdata = {
	.name = "pmem_gpu0",
	.start = MSM_PMEM_GPU0_BASE,
	.size = MSM_PMEM_GPU0_SIZE,
	.no_allocator = 1,
	.cached = 0,
};
#endif

static struct android_pmem_platform_data android_pmem_gpu1_pdata = {
	.name = "pmem_gpu1",
	.no_allocator = 1,
	.cached = 0,
};

static struct platform_device android_pmem_device = {
	.name = "android_pmem",
	.id = 0,
	.dev = { .platform_data = &android_pmem_pdata },
};

static struct platform_device android_pmem_camera_device = {
	.name = "android_pmem",
	.id = 4,
	.dev = { .platform_data = &android_pmem_camera_pdata },
};

static struct platform_device android_pmem_adsp_device = {
	.name = "android_pmem",
	.id = 1,
	.dev = { .platform_data = &android_pmem_adsp_pdata },
};

#ifdef CONFIG_MSM_STACKED_MEMORY
static struct platform_device android_pmem_gpu0_device = {
	.name = "android_pmem",
	.id = 2,
	.dev = { .platform_data = &android_pmem_gpu0_pdata },
};
#endif

static struct platform_device android_pmem_gpu1_device = {
	.name = "android_pmem",
	.id = 3,
	.dev = { .platform_data = &android_pmem_gpu1_pdata },
};

static void msm_fb_mddi_power_save(int on)
{			

} 
        
#define PM_VID_EN_CONFIG_PROC          24
#define PM_VID_EN_API_PROG             0x30000061
#define PM_VID_EN_API_VERS             0x00010001 
        
static struct msm_rpc_endpoint *pm_vid_en_ep;
        
static int msm_fb_pm_vid_en(int on)
{
        int rc = 0;
        struct msm_fb_pm_vid_en_req {
                struct rpc_request_hdr hdr;
                uint32_t on;
        } req;
        
        pm_vid_en_ep = msm_rpc_connect(PM_VID_EN_API_PROG,
                                        PM_VID_EN_API_VERS, 0);
        if (IS_ERR(pm_vid_en_ep)) {
                printk(KERN_ERR "%s: msm_rpc_connect failed! rc = %ld\n",
                        __func__, PTR_ERR(pm_vid_en_ep));
                return -EINVAL;
        }

        req.on = cpu_to_be32(on);
        rc = msm_rpc_call(pm_vid_en_ep,
                        PM_VID_EN_CONFIG_PROC,
                        &req, sizeof(req),
                        5 * HZ);
        if (rc)
                printk(KERN_ERR
                        "%s: msm_rpc_call failed! rc = %d\n", __func__, rc);
                
        msm_rpc_close(pm_vid_en_ep);
        return rc;
} 

static int mddi_get_panel_num(void)
{
	if (machine_is_msm7201a_surf())
		return 2;
	else
		return 1;
}

static int mddi_toshiba_backlight_level(int level)
{
	int out_val;

	if (machine_is_msm7201a_ffa()) {
		switch (level) {
		case 0:
			out_val = 0x00001387;
			break;
		case 1:
			out_val = 3500;
			break;
		case 2:
			out_val = 3200;
			break;
		case 3:
			out_val = 2700;
			break;
		case 4:
			out_val = 2200;
			break;
		default:
			out_val = 0x0000;
			break;
		}
	} else {
		switch (level) {
		case 0:
			out_val = 0x0000;
			break;
		case 1:
			out_val = 1250;
			break;
		case 2:
			out_val = 2500;
			break;
		case 3:
			out_val = 3750;
			break;
		case 4:
			out_val = 4999;
			break;
		default:
			out_val = 0x00001387;
			break;
		}
	}

	return out_val;
}

static int mddi_tc358723xbg_backlight_level(int level)
{
	//0~15
	return level;

}
static int mddi_sharp_backlight_level(int level)
{
	if (machine_is_msm7201a_ffa())
		return level;
	else
		return -1;
}

static struct tvenc_platform_data tvenc_pdata = {
        .pm_vid_en = msm_fb_pm_vid_en,
};

static struct mddi_platform_data mddi_pdata = {
        .mddi_power_save = msm_fb_mddi_power_save,
};

static struct msm_panel_common_pdata mddi_toshiba_pdata = {
	.backlight_level = mddi_toshiba_backlight_level,
	.panel_num = mddi_get_panel_num,
};
static struct msm_panel_common_pdata mddi_tc358723xbg_pdata = {
	.backlight_level = mddi_tc358723xbg_backlight_level,
};

static struct msm_panel_common_pdata mddi_sharp_pdata = {
	.backlight_level = mddi_sharp_backlight_level,
};

static struct resource msm_fb_resources[] = {
	{
		.flags  = IORESOURCE_DMA,
	}
};

static struct platform_device msm_fb_device = {
	.name   = "msm_fb",
	.id     = 0,
	.num_resources  = ARRAY_SIZE(msm_fb_resources),
	.resource       = msm_fb_resources,
};

static struct platform_device mddi_toshiba_device = {
	.name   = "mddi_toshiba",
	.id     = 0,
	.dev    = {
		.platform_data = &mddi_toshiba_pdata,
	}
};
static struct platform_device mddi_tc358723xbg_device = {
	.name   = "mddi_tc23xbg_vga",
	.id     = 0,
	.dev    = {
		.platform_data = &mddi_tc358723xbg_pdata,
	}
};

static struct platform_device mddi_sharp_device = {
	.name   = "mddi_sharp_qvga",
	.id     = 0,
	.dev    = {
		.platform_data = &mddi_sharp_pdata,
	}
};

#ifdef CONFIG_BT
static struct platform_device msm_bt_power_device = {
	.name = "bt_power",
};

enum {
	BT_WAKE,
	BT_RFR,
	BT_CTS,
	BT_RX,
	BT_TX,
	BT_PCM_DOUT,
	BT_PCM_DIN,
	BT_PCM_SYNC,
	BT_PCM_CLK,
	BT_HOST_WAKE,
};

static unsigned bt_config_power_on[] = {
       GPIO_CFG(42, 0, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA),	/* WAKE */
	GPIO_CFG(43, 2, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA),	/* RFR */
	GPIO_CFG(44, 2, GPIO_INPUT,  GPIO_NO_PULL, GPIO_2MA),	        /* CTS */
	GPIO_CFG(45, 2, GPIO_INPUT,  GPIO_NO_PULL, GPIO_2MA),	        /* Rx */
	GPIO_CFG(46, 3, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA),	/* Tx */
	GPIO_CFG(68, 1, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA),	/* PCM_DOUT */
	GPIO_CFG(69, 1, GPIO_INPUT,  GPIO_NO_PULL, GPIO_2MA),	       /* PCM_DIN */
	GPIO_CFG(70, 2, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA),	/* PCM_SYNC */
	GPIO_CFG(71, 2, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA),	/* PCM_CLK */
      GPIO_CFG(92, 0, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA),
	GPIO_CFG(83, 0, GPIO_INPUT,  GPIO_NO_PULL, GPIO_2MA),	/* HOST_WAKE*/	
};
static unsigned bt_config_power_off[] = {
	GPIO_CFG(42, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA),	/* wake*/
       GPIO_CFG(43, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA),	/* RFR */
	GPIO_CFG(44, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA),	/* CTS */
	GPIO_CFG(45, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA),	/* Rx */
	GPIO_CFG(46, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA),	/* Tx */
	GPIO_CFG(68, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA),	/* PCM_DOUT */
	GPIO_CFG(69, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA),	/* PCM_DIN */
	GPIO_CFG(70, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA),	/* PCM_SYNC */
	GPIO_CFG(71, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA),	/* PCM_CLK */
       GPIO_CFG(92, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA),	/* reset */
	GPIO_CFG(83, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA),	/* HOST_WAKE */
};

static int bluetooth_power(int on)
{
	struct vreg *vreg_bt;
	int pin, rc;

	printk(KERN_DEBUG "%s\n", __func__);

	/* do not have vreg bt defined, gp6 is the same */
	/* vreg_get parameter 1 (struct device *) is ignored */
	if (on) {
		for (pin = 0; pin < ARRAY_SIZE(bt_config_power_on); pin++) {
			rc = gpio_tlmm_config(bt_config_power_on[pin],
					      GPIO_ENABLE);
			if (rc) {
				printk(KERN_ERR
				       "%s: gpio_tlmm_config(%#x)=%d\n",
				       __func__, bt_config_power_on[pin], rc);
				return -EIO;
			}
		}

            rc = gpio_direction_output(92, 1);  /*92 -->1*/
            if (rc) {
			printk(KERN_ERR "%s: generation BTS4020 main clock is failed (%d)\n",
			       __func__, rc);
			return -EIO;
		}
	} else {
           rc = gpio_direction_output(92, 0);  
		for (pin = 0; pin < ARRAY_SIZE(bt_config_power_off); pin++) {
			rc = gpio_tlmm_config(bt_config_power_off[pin],
					      GPIO_ENABLE);
			if (rc) {
				printk(KERN_ERR
				       "%s: gpio_tlmm_config(%#x)=%d\n",
				       __func__, bt_config_power_off[pin], rc);
				return -EIO;
			}
		}
	}
	return 0;
}

static void __init bt_power_init(void)
{
	msm_bt_power_device.dev.platform_data = &bluetooth_power;
}
#else
#define bt_power_init(x) do {} while (0)
#endif

static struct resource bluesleep_resources[] = {
	{
		.name	= "gpio_host_wake",
		.start	= 83,
		.end	= 83,
		.flags	= IORESOURCE_IO,
	},
	{
		.name	= "gpio_ext_wake",
		.start	= 42,
		.end	= 42,
		.flags	= IORESOURCE_IO,
	},
	{
		.name	= "host_wake",
		.start	= MSM_GPIO_TO_INT(83),
		.end	= MSM_GPIO_TO_INT(83),
		.flags	= IORESOURCE_IRQ,
	},
};

static struct platform_device msm_bluesleep_device = {
	.name = "bluesleep",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(bluesleep_resources),
	.resource	= bluesleep_resources,
};

static struct platform_device u8220_leds_key_device = {
	.name = "pmic-leds",
	.id		= -1,
};
static struct platform_device huawei_battery_device = {
	.name = "u8220_battery",
	.id		= -1,
};
static struct platform_device msm_wlan_ar6000 = {
	.name		= "wlan_ar6000",
	.id		= 1,
	.num_resources	= 0,
	.resource	= NULL,
};
static struct platform_device *devices[] __initdata = {
#if !defined(CONFIG_MSM_SERIAL_DEBUGGER)
	&msm_device_uart3,
#endif
	&msm_device_uart_dm1,
	&msm_device_smd,
	&msm_device_nand,
	&msm_device_i2c,
	&smc91x_device,
	&msm_device_tssc,
	&android_pmem_camera_device,
	&android_pmem_device,
	&android_pmem_adsp_device,
#ifdef CONFIG_MSM_STACKED_MEMORY
	&android_pmem_gpu0_device,
#endif
	&android_pmem_gpu1_device,
	&msm_device_hsusb_otg,
	&msm_device_hsusb_host,
#if defined(CONFIG_USB_FUNCTION) || defined(CONFIG_USB_ANDROID)
	&msm_device_hsusb_peripheral,
#endif
#ifdef CONFIG_USB_FUNCTION
	&mass_storage_device,
#endif
#ifdef CONFIG_USB_ANDROID
	&android_usb_device,
#endif

#ifdef CONFIG_BT
	&msm_bt_power_device,
#endif
	&halibut_snd,
	&msm_bluesleep_device,
	&msm_fb_device,
	&huawei_battery_device,
	&mddi_tc358723xbg_device,
	&mddi_toshiba_device,
	&mddi_sharp_device,
	&u8220_leds_key_device,
	&msm_wlan_ar6000,
};

extern struct sys_timer msm_timer;

static struct i2c_board_info i2c_devices[] = {
	{
		I2C_BOARD_INFO("mt9t013_liteon", 0x6C >> 1),
	},
	{
		I2C_BOARD_INFO("mt9t013_byd", 0x6C),
	},
	{
		I2C_BOARD_INFO("ov3647", 0x90 >> 1),
	},
	{
		I2C_BOARD_INFO("ov7690", 0x42 >> 1),
	},
	{
		I2C_BOARD_INFO("cpt_ts", 0x0a),	
		.irq = MSM_GPIO_TO_INT(57)  /*gpio 57 is interupt for touchscreen.*/

	},
        {
                I2C_BOARD_INFO(SYNAPTICS_I2C_RMI_NAME, 0x20),                
                .irq = MSM_GPIO_TO_INT(57)  /*gpio 57 is interupt for touchscreen.*/
        },
	{
	   I2C_BOARD_INFO("GS", 0xA6 >> 1),	  
	    .irq = MSM_GPIO_TO_INT(31)
	},	

	{
	    I2C_BOARD_INFO("gs_st", 0x3a >> 1),	  
	   .irq = MSM_GPIO_TO_INT(31)
	},	  


	{
	    I2C_BOARD_INFO("freescale", 0x1c),	  
	    .irq = MSM_GPIO_TO_INT(31)
	},
	{
		I2C_BOARD_INFO("akm8973", 0x3c>>1),//7 bit addr, no write bit
	 	.irq = MSM_GPIO_TO_INT(88)
		
	}	
};

static uint32_t camera_off_gpio_table[] = {
	/* parallel CAMERA interfaces */
	GPIO_CFG(0,  0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DAT0 */
	GPIO_CFG(1,  0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DAT1 */
	GPIO_CFG(2,  0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DAT2 */
	GPIO_CFG(3,  0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DAT3 */
	GPIO_CFG(4,  0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DAT4 */
	GPIO_CFG(5,  0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DAT5 */
	GPIO_CFG(6,  0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DAT6 */
	GPIO_CFG(7,  0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DAT7 */
	GPIO_CFG(8,  0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DAT8 */
	GPIO_CFG(9,  0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DAT9 */
	GPIO_CFG(10, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DAT10 */
	GPIO_CFG(11, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DAT11 */
	GPIO_CFG(12, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* PCLK */
	GPIO_CFG(13, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* HSYNC_IN */
	GPIO_CFG(14, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* VSYNC_IN */
	GPIO_CFG(15, 0, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_2MA), /* MCLK */
};

static uint32_t camera_on_gpio_table[] = {
   /* parallel CAMERA interfaces */
   GPIO_CFG(0,  1, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DAT0 */
   GPIO_CFG(1,  1, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DAT1 */
   GPIO_CFG(2,  1, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DAT2 */
   GPIO_CFG(3,  1, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DAT3 */
   GPIO_CFG(4,  1, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DAT4 */
   GPIO_CFG(5,  1, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DAT5 */
   GPIO_CFG(6,  1, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DAT6 */
   GPIO_CFG(7,  1, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DAT7 */
   GPIO_CFG(8,  1, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DAT8 */
   GPIO_CFG(9,  1, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DAT9 */
   GPIO_CFG(10, 1, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DAT10 */
   GPIO_CFG(11, 1, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* DAT11 */
   GPIO_CFG(12, 1, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_16MA), /* PCLK */
   GPIO_CFG(13, 1, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* HSYNC_IN */
   GPIO_CFG(14, 1, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* VSYNC_IN */
   GPIO_CFG(15, 1, GPIO_OUTPUT, GPIO_PULL_DOWN, GPIO_16MA), /* MCLK */
   GPIO_CFG(21, 0, GPIO_OUTPUT, GPIO_PULL_DOWN, GPIO_2MA), /* CAMIF_SHDN_INS */
   GPIO_CFG(29, 0, GPIO_OUTPUT, GPIO_PULL_DOWN, GPIO_2MA), /* CAMIF_SHDN_OUTS */
   GPIO_CFG(89, 0, GPIO_OUTPUT, GPIO_PULL_DOWN, GPIO_2MA), /* reset */
   GPIO_CFG(109, 0, GPIO_OUTPUT, GPIO_PULL_DOWN, GPIO_2MA), /* vcm */
   GPIO_CFG(30, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), /* module */
};

static void config_gpio_table(uint32_t *table, int len)
{
	int n, rc;
	for (n = 0; n < len; n++) {
		rc = gpio_tlmm_config(table[n], GPIO_ENABLE);
		if (rc) {
			printk(KERN_ERR "%s: gpio_tlmm_config(%#x)=%d\n",
				__func__, table[n], rc);
			break;
		}
	}
}

static void config_camera_on_gpios(void)
{
	config_gpio_table(camera_on_gpio_table,
		ARRAY_SIZE(camera_on_gpio_table));
}

static void config_camera_off_gpios(void)
{
	config_gpio_table(camera_off_gpio_table,
		ARRAY_SIZE(camera_off_gpio_table));
}

static int32_t sensor_vreg_enable
(
    struct msm_camera_sensor_vreg *sensor_vreg,
    uint8_t vreg_num
)

{
    struct vreg *vreg_handle;
    uint8_t temp_vreg_sum;
    int32_t rc;
    
    if(sensor_vreg == NULL)
    {
        return 0;
    }
    
    for(temp_vreg_sum = 0; temp_vreg_sum < vreg_num;temp_vreg_sum++)
    {
        vreg_handle = vreg_get(0, sensor_vreg[temp_vreg_sum].vreg_name);
    	if (!vreg_handle) {
    		printk(KERN_ERR "vreg_handle get failed\n");
    		return -EIO;
    	}
    	rc = vreg_set_level(vreg_handle, sensor_vreg[temp_vreg_sum].mv);
    	if (rc) {
    		printk(KERN_ERR "vreg_handle set level failed\n");
    		return -EIO;
    	}
    	rc = vreg_enable(vreg_handle);
    	if (rc) {
    		printk(KERN_ERR "vreg_handle enable failed\n");
    		return -EIO;
    	}
    }
    return 0;
}

static int32_t sensor_vreg_disable
(
    struct msm_camera_sensor_vreg *sensor_vreg,
    uint8_t vreg_num
)
{
    struct vreg *vreg_handle;
    uint8_t temp_vreg_sum;
    int32_t rc;
    
    if(sensor_vreg == NULL)
    {
        return 0;
    }

    for(temp_vreg_sum = 0; temp_vreg_sum < vreg_num;temp_vreg_sum++)
    {
        vreg_handle = vreg_get(0, sensor_vreg[temp_vreg_sum].vreg_name);
    	if (!vreg_handle) {
    		printk(KERN_ERR "vreg_handle get failed\n");
    		return -EIO;
    	}
    	rc = vreg_disable(vreg_handle);
    	if (rc) {
    		printk(KERN_ERR "vreg_handle disable failed\n");
    		return -EIO;
    	}
    }
    return 0;
}
static int32_t sensor_master_init_control_slave
(
    struct msm_camera_sensor_info *master_sensor_info
);


 struct msm_camera_sensor_vreg sensor_vreg_array[] = {
    {
		.vreg_name   = "gp3",
		.mv	  = 2600,
	}, 
    {
		.vreg_name   = "gp1",
		.mv	  = 2800,
	},    
    {
		.vreg_name   = "gp2",
		.mv	  = 1800,
	},
   
};
#define MSM_PROBE_INIT(name) name##_probe_init
static struct msm_camera_sensor_info msm_camera_sensor[] = {
    {
		.sensor_reset   = 89,
		.sensor_pwd	  = 29,
		.vcm_pwd      = 109,
		.sensor_name  = "ov3647",
		.flash_type		= MSM_CAMERA_FLASH_NONE,
#ifdef CONFIG_MSM_CAMERA
		.sensor_probe = MSM_PROBE_INIT(ov3647),
#endif
        .sensor_vreg  = sensor_vreg_array,
        .vreg_num     = ARRAY_SIZE(sensor_vreg_array),
        .vreg_enable_func = sensor_vreg_enable,
        .vreg_disable_func = sensor_vreg_disable,
        .slave_sensor = 0,
        .master_init_control_slave = sensor_master_init_control_slave,
	},
    {
		.sensor_reset   = 89,
		.sensor_pwd	  = 29,
		.vcm_pwd      = 109,
		.sensor_name  = "mt9t013_liteon",
		.flash_type		= MSM_CAMERA_FLASH_NONE,
#ifdef CONFIG_MSM_CAMERA
		.sensor_probe = MSM_PROBE_INIT(mt9t013_liteon),
#endif
        .sensor_module_id  = 30,
        .sensor_module_value  = 0,
        .sensor_vreg  = sensor_vreg_array,
        .vreg_num     = ARRAY_SIZE(sensor_vreg_array),
        .vreg_enable_func = sensor_vreg_enable,
        .vreg_disable_func = sensor_vreg_disable,
        .slave_sensor = 0,
        .master_init_control_slave = sensor_master_init_control_slave,
	},
    {
		.sensor_reset   = 89,
		.sensor_pwd	  = 29,
		.vcm_pwd      = 109,
		.sensor_name  = "mt9t013_byd",
		.flash_type		= MSM_CAMERA_FLASH_NONE,
#ifdef CONFIG_MSM_CAMERA
		.sensor_probe = MSM_PROBE_INIT(mt9t013_byd),
#endif
        .sensor_module_id  = 30,
        .sensor_module_value  = 1,
        .sensor_vreg  = sensor_vreg_array,
        .vreg_num     = ARRAY_SIZE(sensor_vreg_array),
        .vreg_enable_func = sensor_vreg_enable,
        .vreg_disable_func = sensor_vreg_disable,
        .slave_sensor = 0,
        .master_init_control_slave = sensor_master_init_control_slave,
	},
    {
		.sensor_reset   = 89,
		.sensor_pwd	  = 21,
		.vcm_pwd      = 0,
		.sensor_name  = "ov7690",
		.flash_type		= MSM_CAMERA_FLASH_NONE,
#ifdef CONFIG_MSM_CAMERA
		.sensor_probe = MSM_PROBE_INIT(ov7690),
#endif
		.sensor_vreg  = sensor_vreg_array,
        .vreg_num     = ARRAY_SIZE(sensor_vreg_array),
        .vreg_enable_func = sensor_vreg_enable,
        .vreg_disable_func = sensor_vreg_disable,
        .slave_sensor = 1,
        .slave_elude_func = NULL,
	},
};
#undef MSM_PROBE_INIT
static int32_t sensor_master_init_control_slave
(
    struct msm_camera_sensor_info *master_sensor_info
)
{
    int32_t rc = -1;
    uint8_t temp_sensor_sum;
    struct msm_camera_sensor_info *master_sensor = NULL;
    struct msm_camera_sensor_info *slave_sensor = NULL;
    if(master_sensor_info == NULL)
    {
        return -1;
    }

    for(temp_sensor_sum = 0; temp_sensor_sum < ARRAY_SIZE(msm_camera_sensor);temp_sensor_sum++)
    {
        if(msm_camera_sensor[temp_sensor_sum].slave_sensor)
        {
            if(msm_camera_sensor[temp_sensor_sum].slave_elude_func)
            {
                if(msm_camera_sensor[temp_sensor_sum].slave_elude_func(master_sensor_info,&msm_camera_sensor[temp_sensor_sum]) == 0)
                {
                    rc = 0;
                }
            }
        }
    }
    
    return rc;
}

static struct msm_camera_device_platform_data msm_camera_device_data = {
	.camera_gpio_on  = config_camera_on_gpios,
	.camera_gpio_off = config_camera_off_gpios,
	.snum = ARRAY_SIZE(msm_camera_sensor),
	.sinfo = &msm_camera_sensor[0],
	.ioext.mdcphy = MSM_MDC_PHYS,
	.ioext.mdcsz  = MSM_MDC_SIZE,
	.ioext.appphy = MSM_CLK_CTL_PHYS,
	.ioext.appsz  = MSM_CLK_CTL_SIZE,
};

static void __init msm_camera_add_device(void)
{
	msm_camera_register_device(NULL, 0, &msm_camera_device_data);
	config_camera_off_gpios();
}

static void __init halibut_init_irq(void)
{
	msm_init_irq();
}

static struct msm_acpu_clock_platform_data halibut_clock_data = {
	.acpu_switch_time_us = 50,
	.max_speed_delta_khz = 256000,
	.vdd_switch_time_us = 62,
	.power_collapse_khz = 19200000,
	.wait_for_irq_khz = 128000000,
	.max_axi_khz = 128000,
};

void msm_serial_debug_init(unsigned int base, int irq,
				struct device *clk_device, int signal_irq);
static void sdcc_gpio_init(void)
{
#ifdef CONFIG_MMC_MSM_CARD_HW_DETECTION
	int rc = 0;
	if (gpio_request(49, "sdc1_status_irq"))
		pr_err("failed to request gpio sdc1_status_irq\n");
	rc = gpio_tlmm_config(GPIO_CFG(49, 0, GPIO_INPUT, GPIO_PULL_UP,
				GPIO_2MA), GPIO_ENABLE);
	if (rc)
		printk(KERN_ERR "%s: Failed to configure GPIO %d\n",
				__func__, rc);
#endif
	int rc = 0;	
	if (gpio_request(28, "wifi_wow_irq"))		
	    printk("failed to request gpio wifi_wow_irq\n");	

	rc = gpio_tlmm_config(GPIO_CFG(28, 0, GPIO_INPUT, GPIO_PULL_DOWN, GPIO_2MA), GPIO_ENABLE);	
	if (rc)		
	    printk(KERN_ERR "%s: Failed to configure GPIO %d\n",__func__, rc);
	/* SDC1 GPIOs */
#ifdef CONFIG_MMC_MSM_SDC1_SUPPORT
	if (gpio_request(51, "sdc1_data_3"))
		pr_err("failed to request gpio sdc1_data_3\n");
	if (gpio_request(52, "sdc1_data_2"))
		pr_err("failed to request gpio sdc1_data_2\n");
	if (gpio_request(53, "sdc1_data_1"))
		pr_err("failed to request gpio sdc1_data_1\n");
	if (gpio_request(54, "sdc1_data_0"))
		pr_err("failed to request gpio sdc1_data_0\n");
	if (gpio_request(55, "sdc1_cmd"))
		pr_err("failed to request gpio sdc1_cmd\n");
	if (gpio_request(56, "sdc1_clk"))
		pr_err("failed to request gpio sdc1_clk\n");
#endif

	if (machine_is_msm7201a_ffa())
		return;

	/* SDC2 GPIOs */
#ifdef CONFIG_MMC_MSM_SDC2_SUPPORT
	if (gpio_request(62, "sdc2_clk"))
		pr_err("failed to request gpio sdc2_clk\n");
	if (gpio_request(63, "sdc2_cmd"))
		pr_err("failed to request gpio sdc2_cmd\n");
	if (gpio_request(64, "sdc2_data_3"))
		pr_err("failed to request gpio sdc2_data_3\n");
	if (gpio_request(65, "sdc2_data_2"))
		pr_err("failed to request gpio sdc2_data_2\n");
	if (gpio_request(66, "sdc2_data_1"))
		pr_err("failed to request gpio sdc2_data_1\n");
	if (gpio_request(67, "sdc2_data_0"))
		pr_err("failed to request gpio sdc2_data_0\n");
#endif

	/* SDC4 GPIOs */
#ifdef CONFIG_MMC_MSM_SDC4_SUPPORT
	if (gpio_request(19, "sdc4_data_3"))
		pr_err("failed to request gpio sdc4_data_3\n");
	if (gpio_request(20, "sdc4_data_2"))
		pr_err("failed to request gpio sdc4_data_2\n");
	if (gpio_request(21, "sdc4_data_1"))
		pr_err("failed to request gpio sdc4_data_1\n");
	if (gpio_request(107, "sdc4_cmd"))
		pr_err("failed to request gpio sdc4_cmd\n");
	if (gpio_request(108, "sdc4_data_0"))
		pr_err("failed to request gpio sdc4_data_0\n");
	if (gpio_request(109, "sdc4_clk"))
		pr_err("failed to request gpio sdc4_clk\n");
#endif
}

static unsigned sdcc_cfg_data[][6] = {
	/* SDC1 configs */
	{
	GPIO_CFG(51, 1, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_8MA),
	GPIO_CFG(52, 1, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_8MA),
	GPIO_CFG(53, 1, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_8MA),
	GPIO_CFG(54, 1, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_8MA),
	GPIO_CFG(55, 1, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_8MA),
	GPIO_CFG(56, 1, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_8MA),
	},
	/* SDC2 configs */
	{
	GPIO_CFG(62, 2, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_8MA),
	GPIO_CFG(63, 2, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_8MA),
	GPIO_CFG(64, 2, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_8MA),
	GPIO_CFG(65, 2, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_8MA),
	GPIO_CFG(66, 2, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_8MA),
	GPIO_CFG(67, 2, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_8MA),
	},
	{
	/* SDC3 configs */
	},
	/* SDC4 configs */
	{
#ifdef CONFIG_MMC_MSM_SDC4_SUPPORT	
	GPIO_CFG(19, 3, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_8MA),
	GPIO_CFG(20, 3, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_8MA),
	GPIO_CFG(21, 3, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_8MA),
	GPIO_CFG(107, 1, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_8MA),
	GPIO_CFG(108, 1, GPIO_OUTPUT, GPIO_PULL_UP, GPIO_8MA),
	GPIO_CFG(109, 1, GPIO_OUTPUT, GPIO_NO_PULL, GPIO_8MA),
#endif	
	}
};

static unsigned long vreg_sts, gpio_sts;
static struct mpp *mpp_mmc;
static struct vreg *vreg_mmc;

static void msm_sdcc_setup_gpio(int dev_id, unsigned int enable)
{
	int i, rc;

	if (!(test_bit(dev_id, &gpio_sts)^enable))
		return;

	if (enable)
		set_bit(dev_id, &gpio_sts);
	else
		clear_bit(dev_id, &gpio_sts);

	for (i = 0; i < ARRAY_SIZE(sdcc_cfg_data[dev_id - 1]); i++) {
		rc = gpio_tlmm_config(sdcc_cfg_data[dev_id - 1][i],
			enable ? GPIO_ENABLE : GPIO_DISABLE);
		if (rc) {
			printk(KERN_ERR "%s: gpio_tlmm_config(%#x)=%d\n",
				__func__, sdcc_cfg_data[dev_id - 1][i], rc);
		}
	}
}

static uint32_t msm_sdcc_setup_power(struct device *dv, unsigned int vdd)
{
	int rc = 0;
	struct platform_device *pdev;

	pdev = container_of(dv, struct platform_device, dev);
	msm_sdcc_setup_gpio(pdev->id, !!vdd);

	if (vdd == 0) {
		if (!vreg_sts)
			return 0;

		clear_bit(pdev->id, &vreg_sts);

		if (!vreg_sts) {
			if (machine_is_msm7201a_ffa())
				rc = mpp_config_digital_out(mpp_mmc,
				     MPP_CFG(MPP_DLOGIC_LVL_MSMP,
				     MPP_DLOGIC_OUT_CTRL_LOW));
			else
				rc = 0;
			if (rc)
				printk(KERN_ERR "%s: return val: %d \n",
					__func__, rc);
		}
		return 0;
	}

	if (!vreg_sts) {
		if (machine_is_msm7201a_ffa())
			rc = mpp_config_digital_out(mpp_mmc,
			     MPP_CFG(MPP_DLOGIC_LVL_MSMP,
			     MPP_DLOGIC_OUT_CTRL_HIGH));
		else {
			rc = vreg_set_level(vreg_mmc, 2850);
			if (!rc)
				rc = vreg_enable(vreg_mmc);
		}
		if (rc)
			printk(KERN_ERR "%s: return val: %d \n",
					__func__, rc);
	}
	set_bit(pdev->id, &vreg_sts);
	return 0;
}

#ifdef CONFIG_MMC_MSM_CARD_HW_DETECTION
static unsigned int halibut_sdcc_slot_status(struct device *dev)
{
	return (unsinged int) gpio_get_value(49);
}
#endif

static struct mmc_platform_data halibut_sdcc_data = {
	.ocr_mask	= MMC_VDD_28_29,
	.translate_vdd	= msm_sdcc_setup_power,
#ifdef CONFIG_MMC_MSM_CARD_HW_DETECTION
	.status         = halibut_sdcc_slot_status,
	.status_irq	= MSM_GPIO_TO_INT(49),
	.irq_flags      = IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
#endif
};

static uint32_t msm_sdcc_setup_power_wifi(struct device *dv, unsigned int vdd)
{
	struct platform_device *pdev;

	pdev = container_of(dv, struct platform_device, dev);
	msm_sdcc_setup_gpio(pdev->id, !!vdd);

	return 0;
}

static struct mmc_platform_data halibut_sdcc_data_wifi = {
	.ocr_mask	= MMC_VDD_28_29,
	.translate_vdd	= msm_sdcc_setup_power_wifi
};
static void __init halibut_init_mmc(void)
{
	if (machine_is_msm7201a_ffa()) {
		mpp_mmc = mpp_get(NULL, "mpp3");
		if (!mpp_mmc) {
			printk(KERN_ERR "%s: mpp get failed (%ld)\n",
			       __func__, PTR_ERR(vreg_mmc));
			return;
		}
	} else {
		vreg_mmc = vreg_get(NULL, "wlan");
		if (IS_ERR(vreg_mmc)) {
			printk(KERN_ERR "%s: vreg get failed (%ld)\n",
			       __func__, PTR_ERR(vreg_mmc));
			return;
		}
	}

	sdcc_gpio_init();
#ifdef CONFIG_MMC_MSM_SDC1_SUPPORT
	msm_add_sdcc(1, &halibut_sdcc_data);
#endif

	msm_add_sdcc(2, &halibut_sdcc_data_wifi);
	if (machine_is_msm7201a_surf()) {
#ifdef CONFIG_MMC_MSM_SDC2_SUPPORT
//		msm_add_sdcc(2, &halibut_sdcc_data);
#endif
#ifdef CONFIG_MMC_MSM_SDC4_SUPPORT
		msm_add_sdcc(4, &halibut_sdcc_data);
#endif
	}
}

static struct msm_panel_common_pdata mdp_pdata = {
	.gpio = 97,
};

static void __init msm_fb_add_devices(void)
{
	msm_fb_register_device("mdp", &mdp_pdata);
        msm_fb_register_device("ebi2", 0);
        msm_fb_register_device("pmdh", &mddi_pdata);
        msm_fb_register_device("emdh", 0);
        msm_fb_register_device("tvenc", &tvenc_pdata);
}
static unsigned jogball_gpio_table[] = {
        GPIO_CFG(36, 0, GPIO_OUTPUT,  GPIO_PULL_UP, GPIO_2MA),
        GPIO_CFG(82, 0, GPIO_INPUT, GPIO_NO_PULL, GPIO_2MA),	
	GPIO_CFG(84, 0, GPIO_INPUT, GPIO_NO_PULL, GPIO_2MA),	
	GPIO_CFG(85, 0, GPIO_INPUT,  GPIO_NO_PULL, GPIO_2MA),	       
	GPIO_CFG(108, 0, GPIO_INPUT,  GPIO_NO_PULL, GPIO_2MA),	  
	
};

static void config_jogball_gpios(void)
{
	config_gpio_table(jogball_gpio_table,
		ARRAY_SIZE(jogball_gpio_table));
    
}

static void
msm_i2c_gpio_config(int iface, int config_type)
{
	int gpio_scl;
	int gpio_sda;
	if (iface) {
		gpio_scl = 95;
		gpio_sda = 96;
	} else {
		gpio_scl = 60;
		gpio_sda = 61;
	}
	if (config_type) {
		gpio_tlmm_config(GPIO_CFG(gpio_scl, 1, GPIO_INPUT,
					GPIO_NO_PULL, GPIO_16MA), GPIO_ENABLE);
		gpio_tlmm_config(GPIO_CFG(gpio_sda, 1, GPIO_INPUT,
					GPIO_NO_PULL, GPIO_16MA), GPIO_ENABLE);
	} else {
		gpio_tlmm_config(GPIO_CFG(gpio_scl, 0, GPIO_OUTPUT,
					GPIO_NO_PULL, GPIO_16MA), GPIO_ENABLE);
		gpio_tlmm_config(GPIO_CFG(gpio_sda, 0, GPIO_OUTPUT,
					GPIO_NO_PULL, GPIO_16MA), GPIO_ENABLE);
	}
}

static struct msm_i2c_platform_data msm_i2c_pdata = {
	.clk_freq = 100000,
	.msm_i2c_config_gpio = msm_i2c_gpio_config,
};

static void __init msm_device_i2c_init(void)
{
	if (gpio_request(60, "i2c_pri_clk"))
		pr_err("failed to request gpio i2c_pri_clk\n");
	if (gpio_request(61, "i2c_pri_dat"))
		pr_err("failed to request gpio i2c_pri_dat\n");
	if (gpio_request(95, "i2c_sec_clk"))
		pr_err("failed to request gpio i2c_sec_clk\n");
	if (gpio_request(96, "i2c_sec_dat"))
		pr_err("failed to request gpio i2c_sec_dat\n");

	msm_device_i2c.dev.platform_data = &msm_i2c_pdata;
}

static struct msm_pm_platform_data msm_pm_data[MSM_PM_SLEEP_MODE_NR] = {
	[MSM_PM_SLEEP_MODE_POWER_COLLAPSE].latency = 16000,
	[MSM_PM_SLEEP_MODE_POWER_COLLAPSE_NO_XO_SHUTDOWN].latency = 12000,
	[MSM_PM_SLEEP_MODE_RAMP_DOWN_AND_WAIT_FOR_INTERRUPT].latency = 2000,
};

#define USB_SERIAL_KEY_SIZE 16
typedef struct _app_usb_serial
{
  /* Stores imei reason for apps */
  unsigned char usb_serial[USB_SERIAL_KEY_SIZE];
} app_usb_serial;
static app_usb_serial *usb_serial_info;
static void __init halibut_init(void)
{
    sensor_vreg_disable(sensor_vreg_array,ARRAY_SIZE(sensor_vreg_array));
	if (socinfo_init() < 0)
		BUG();

	if (machine_is_msm7201a_ffa()) {
		smc91x_resources[0].start = 0x98000300;
		smc91x_resources[0].end = 0x98000400;
		smc91x_resources[1].start = MSM_GPIO_TO_INT(85);
		smc91x_resources[1].end = MSM_GPIO_TO_INT(85);
	}

	/* All 7x01 2.0 based boards are expected to have RAM chips capable
	 * of 160 MHz. */
	if (cpu_is_msm7x01()
	    && SOCINFO_VERSION_MAJOR(socinfo_get_version()) == 2)
		halibut_clock_data.max_axi_khz = 160000;

#if defined(CONFIG_MSM_SERIAL_DEBUGGER)
	msm_serial_debug_init(MSM_UART3_PHYS, INT_UART3,
			      &msm_device_uart3.dev, 1);
#endif
	usb_serial_info = smem_alloc(SMEM_ID_VENDOR0, sizeof(app_usb_serial));
	if (!usb_serial_info)
	{
		printk(KERN_INFO "%s: Can't find usb serial number\n", __func__);
	}
	else
	{
	    printk(KERN_INFO"usb_serial:%s\n", usb_serial_info->usb_serial);
		strncat(usb_serial_num, usb_serial_info->usb_serial, USB_SERIAL_KEY_SIZE);
		msm_hsusb_pdata.serial_number = (char *)usb_serial_num;
	}
	msm_hsusb_pdata.soc_version = socinfo_get_version();
	msm_acpu_clock_init(&halibut_clock_data);
	msm_device_hsusb_peripheral.dev.platform_data = &msm_hsusb_pdata,
	msm_device_hsusb_host.dev.platform_data = &msm_hsusb_pdata,
	platform_add_devices(devices, ARRAY_SIZE(devices));
	msm_camera_add_device();
	msm_device_i2c_init();
	i2c_register_board_info(0, i2c_devices, ARRAY_SIZE(i2c_devices));

	init_u8220_keypad(1);

	config_jogball_gpios();
	init_jogball();
	halibut_init_mmc();
#ifdef CONFIG_USB_FUNCTION
	hsusb_gpio_init();
#endif
	msm_fb_add_devices();
	bt_power_init();
#ifdef CONFIG_USB_ANDROID
	msm_hsusb_rpc_connect();
	msm_hsusb_set_vbus_state(1) ;
#endif
	init_vibrator_device();
	msm_pm_set_platform_data(msm_pm_data);
}

static void __init msm_halibut_allocate_memory_regions(void)
{
	void *addr;
	unsigned long size;

	size = MSM_PMEM_MDP_SIZE;
	addr = alloc_bootmem(size);
	android_pmem_pdata.start = __pa(addr);
	android_pmem_pdata.size = size;
	printk(KERN_INFO "allocating %lu bytes at %p (%lx physical)"
	       "for pmem\n", size, addr, __pa(addr));

  	size = MSM_PMEM_CAMERA_SIZE;
	addr = alloc_bootmem(size);
	android_pmem_camera_pdata.start = __pa(addr);
	android_pmem_camera_pdata.size = size;
	printk(KERN_INFO "allocating %lu bytes at %p (%lx physical)"
	       "for camera pmem\n", size, addr, __pa(addr));

	size = MSM_PMEM_ADSP_SIZE;
	//addr = alloc_bootmem(size);
	android_pmem_adsp_pdata.start =  MSM_PMEM_ADSP_BASE;
	android_pmem_adsp_pdata.size = size;
	printk(KERN_INFO "allocating %lu bytes at %p (%lx physical)"
	       "for adsp pmem\n", size, addr, __pa(addr));

	size = MSM_PMEM_GPU1_SIZE;
	addr = alloc_bootmem_aligned(size, 0x100000);
	android_pmem_gpu1_pdata.start = __pa(addr);
	android_pmem_gpu1_pdata.size = size;
	printk(KERN_INFO "allocating %lu bytes at %p (%lx physical)"
	       "for gpu1 pmem\n", size, addr, __pa(addr));	
	
	size = MSM_FB_SIZE;
	//addr =alloc_bootmem(size);
	msm_fb_resources[0].start = MSM_FB_BASE;
	msm_fb_resources[0].end =  msm_fb_resources[0].start +size -1;
	printk(KERN_INFO "allocating %lu bytes at %p (%lx physical) for fb\n",
			size, addr, __pa(addr));	
}

static void __init halibut_map_io(void)
{
	msm_shared_ram_phys = 0x01F00000;

	msm_map_common_io();
	msm_clock_init(msm_clocks_7x01a, msm_num_clocks_7x01a);
	msm_halibut_allocate_memory_regions();
}

MACHINE_START(HALIBUT, "Halibut Board (QCT SURF7200A)")
#ifdef CONFIG_MSM_DEBUG_UART
	.phys_io        = MSM_DEBUG_UART_PHYS,
	.io_pg_offst    = ((MSM_DEBUG_UART_BASE) >> 18) & 0xfffc,
#endif
	.boot_params	= 0x10000100,
	.map_io		= halibut_map_io,
	.init_irq	= halibut_init_irq,
	.init_machine	= halibut_init,
	.timer		= &msm_timer,
MACHINE_END

MACHINE_START(MSM7201A_FFA, "QCT FFA7201A Board")
#ifdef CONFIG_MSM_DEBUG_UART
	.phys_io        = MSM_DEBUG_UART_PHYS,
	.io_pg_offst    = ((MSM_DEBUG_UART_BASE) >> 18) & 0xfffc,
#endif
	.boot_params	= 0x10000100,
	.map_io		= halibut_map_io,
	.init_irq	= halibut_init_irq,
	.init_machine	= halibut_init,
	.timer		= &msm_timer,
MACHINE_END

MACHINE_START(MSM7201A_SURF, "QCT SURF7201A Board")
#ifdef CONFIG_MSM_DEBUG_UART
	.phys_io        = MSM_DEBUG_UART_PHYS,
	.io_pg_offst    = ((MSM_DEBUG_UART_BASE) >> 18) & 0xfffc,
#endif
	.boot_params	= 0x10000100,
	.map_io		= halibut_map_io,
	.init_irq	= halibut_init_irq,
	.init_machine	= halibut_init,
	.timer		= &msm_timer,
MACHINE_END
