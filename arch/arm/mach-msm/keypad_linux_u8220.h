/* arch/arm/mach-msm/keypad_linux_u8220.h
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
 
#ifndef _KEYPAD_LINUX_U8220_H
#define _KEYPAD_LINUX_U8220_H
#include <linux/input.h>
int init_u8220_keypad(int keypad);
struct input_dev *msm_keypad_get_input_dev(void);
#endif
