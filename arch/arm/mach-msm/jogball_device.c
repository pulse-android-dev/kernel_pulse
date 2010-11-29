/* arch/arm/mach-msm/jogball_device.c
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

#include <asm/mach-types.h>
#include <linux/platform_device.h>
#include <linux/jogball_driver.h>
#include <linux/input.h>

static struct jogball_button jogball_buttons[] = {

      {
		.gpio		= GPIO_UP,  /*108   */
		.desc		= "jogball_up",
	},

    {
		.gpio		= GPIO_DOWN,  /*82    */
		.desc		= "jogball_down",
	},

	{
		.gpio		= GPIO_LEFT,   /* 84     */
		.desc		= "jogball_left",
	},
	
	{
		.gpio		= GPIO_RIGHT,  /* 85     */
		.desc		= "jogball_right",
	},
	
};

static struct jogball_platform_data jogball_data = {
	.buttons	= jogball_buttons,
	.nbuttons	= ARRAY_SIZE(jogball_buttons),
};

static struct platform_device jogball_device = {
	.name		= "jogball",
	.id		= -1,
	.dev		= {
		.platform_data	= &jogball_data,
	},
};

int init_jogball(void)
{
   return platform_device_register(&jogball_device);
}
