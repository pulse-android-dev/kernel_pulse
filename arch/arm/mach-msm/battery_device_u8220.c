/* arch\arm\mach-msm\battery_device_u8220.c
 *
 * Copyright (C) 2008-2009 HUAWEI Corporation
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

#include <linux/platform_device.h>
#include <battery_device_u8220.h>

static struct battery_platform_data huawei_battery_pdata; 

static struct platform_device huawei_battery_device = {
	.name           = "battery",
	.id                 = -1,
	.dev              = {
		.platform_data          = &huawei_battery_pdata,
	},
};

int init_huawei_battery(void)
{
   return platform_device_register(&huawei_battery_device);
}
