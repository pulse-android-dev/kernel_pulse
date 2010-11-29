/* Copyright (c) 2008-2009, Code Aurora Forum. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Code Aurora Forum nor
 *       the names of its contributors may be used to endorse or promote
 *       products derived from this software without specific prior written
 *       permission.
 *
 * Alternatively, provided that this notice is retained in full, this software
 * may be relicensed by the recipient under the terms of the GNU General Public
 * License version 2 ("GPL") and only version 2, in which case the provisions of
 * the GPL apply INSTEAD OF those given above.  If the recipient relicenses the
 * software under the GPL, then the identification text in the MODULE_LICENSE
 * macro must be changed to reflect "GPLv2" instead of "Dual BSD/GPL".  Once a
 * recipient changes the license terms to the GPL, subsequent recipients shall
 * not relicense under alternate licensing terms, including the BSD or dual
 * BSD/GPL terms.  In addition, the following license statement immediately
 * below and between the words START and END shall also then apply when this
 * software is relicensed under the GPL:
 *
 * START
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License version 2 and only version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * END
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "mt9d112.h"


struct register_address_value_pair_t const
preview_snapshot_mode_reg_settings_array[] = {
	{0x338C, 0x2703},
	{0x3390, 800},    /* Output Width (P) = 640 */
	{0x338C, 0x2705},
	{0x3390, 600},    /* Output Height (P) = 480 */
	{0x338C, 0x2707},
	{0x3390, 0x0640}, /* Output Width (S) = 1600 */
	{0x338C, 0x2709},
	{0x3390, 0x04B0}, /* Output Height (S) = 1200 */
	{0x338C, 0x270D},
	{0x3390, 0x0000}, /* Row Start (P) = 0 */
	{0x338C, 0x270F},
	{0x3390, 0x0000}, /* Column Start (P) = 0 */
	{0x338C, 0x2711},
	{0x3390, 0x04BD}, /* Row End (P) = 1213 */
	{0x338C, 0x2713},
	{0x3390, 0x064D}, /* Column End (P) = 1613 */
	{0x338C, 0x2715},
	{0x3390, 0x0000}, /* Extra Delay (P) = 0 */
	{0x338C, 0x2717},
	{0x3390, 0x2111}, /* Row Speed (P) = 8465 */
	{0x338C, 0x2719},
	{0x3390, 0x046C}, /* Read Mode (P) = 1132 */
	{0x338C, 0x271B},
	{0x3390, 0x024F}, /* Sensor_Sample_Time_pck(P) = 591 */
	{0x338C, 0x271D},
	{0x3390, 0x0102}, /* Sensor_Fine_Correction(P) = 258 */
	{0x338C, 0x271F},
	{0x3390, 0x0279}, /* Sensor_Fine_IT_min(P) = 633 */
	{0x338C, 0x2721},
	{0x3390, 0x0155}, /* Sensor_Fine_IT_max_margin(P) = 341 */
	{0x338C, 0x2723},
	{0x3390, 659},    /* Frame Lines (P) = 679 */
	{0x338C, 0x2725},
	{0x3390, 0x0824}, /* Line Length (P) = 2084 */
	{0x338C, 0x2727},
	{0x3390, 0x2020},
	{0x338C, 0x2729},
	{0x3390, 0x2020},
	{0x338C, 0x272B},
	{0x3390, 0x1020},
	{0x338C, 0x272D},
	{0x3390, 0x2007},
	{0x338C, 0x272F},
	{0x3390, 0x0004}, /* Row Start(S) = 4 */
	{0x338C, 0x2731},
	{0x3390, 0x0004}, /* Column Start(S) = 4 */
	{0x338C, 0x2733},
	{0x3390, 0x04BB}, /* Row End(S) = 1211 */
	{0x338C, 0x2735},
	{0x3390, 0x064B}, /* Column End(S) = 1611 */
	{0x338C, 0x2737},
	{0x3390, 0x04CE}, /* Extra Delay(S) = 1230 */
	{0x338C, 0x2739},
	{0x3390, 0x2111}, /* Row Speed(S) = 8465 */
	{0x338C, 0x273B},
	{0x3390, 0x0024}, /* Read Mode(S) = 36 */
	{0x338C, 0x273D},
	{0x3390, 0x0120}, /* Sensor sample time pck(S) = 288 */
	{0x338C, 0x2741},
	{0x3390, 0x0169}, /* Sensor_Fine_IT_min(P) = 361 */
	{0x338C, 0x2745},
	{0x3390, 0x04FF}, /* Frame Lines(S) = 1279 */
	{0x338C, 0x2747},
	{0x3390, 0x0824}, /* Line Length(S) = 2084 */
	{0x338C, 0x2751},
	{0x3390, 0x0000}, /* Crop_X0(P) = 0 */
	{0x338C, 0x2753},
	{0x3390, 0x0320}, /* Crop_X1(P) = 800 */
	{0x338C, 0x2755},
	{0x3390, 0x0000}, /* Crop_Y0(P) = 0 */
	{0x338C, 0x2757},
	{0x3390, 0x0258}, /* Crop_Y1(P) = 600 */
	{0x338C, 0x275F},
	{0x3390, 0x0000}, /* Crop_X0(S) = 0 */
	{0x338C, 0x2761},
	{0x3390, 0x0640}, /* Crop_X1(S) = 1600 */
	{0x338C, 0x2763},
	{0x3390, 0x0000}, /* Crop_Y0(S) = 0 */
	{0x338C, 0x2765},
	{0x3390, 0x04B0}, /* Crop_Y1(S) = 1200 */
	{0x338C, 0x222E},
	{0x3390, 0x00A0}, /* R9 Step = 160 */
	{0x338C, 0xA408},
	{0x3390, 0x001F},
	{0x338C, 0xA409},
	{0x3390, 0x0021},
	{0x338C, 0xA40A},
	{0x3390, 0x0025},
	{0x338C, 0xA40B},
	{0x3390, 0x0027},
	{0x338C, 0x2411},
	{0x3390, 0x00A0},
	{0x338C, 0x2413},
	{0x3390, 0x00C0},
	{0x338C, 0x2415},
	{0x3390, 0x00A0},
	{0x338C, 0x2417},
	{0x3390, 0x00C0},
	{0x338C, 0x2799},
	{0x3390, 0x6408}, /* MODE_SPEC_EFFECTS(P) */
	{0x338C, 0x279B},
	{0x3390, 0x6408}, /* MODE_SPEC_EFFECTS(S) */
};

static struct register_address_value_pair_t const
noise_reduction_reg_settings_array[] = {
	{0x338C, 0xA76D},
	{0x3390, 0x0003},
	{0x338C, 0xA76E},
	{0x3390, 0x0003},
	{0x338C, 0xA76F},
	{0x3390, 0},
	{0x338C, 0xA770},
	{0x3390, 21},
	{0x338C, 0xA771},
	{0x3390, 37},
	{0x338C, 0xA772},
	{0x3390, 63},
	{0x338C, 0xA773},
	{0x3390, 100},
	{0x338C, 0xA774},
	{0x3390, 128},
	{0x338C, 0xA775},
	{0x3390, 151},
	{0x338C, 0xA776},
	{0x3390, 169},
	{0x338C, 0xA777},
	{0x3390, 186},
	{0x338C, 0xA778},
	{0x3390, 199},
	{0x338C, 0xA779},
	{0x3390, 210},
	{0x338C, 0xA77A},
	{0x3390, 220},
	{0x338C, 0xA77B},
	{0x3390, 228},
	{0x338C, 0xA77C},
	{0x3390, 234},
	{0x338C, 0xA77D},
	{0x3390, 240},
	{0x338C, 0xA77E},
	{0x3390, 244},
	{0x338C, 0xA77F},
	{0x3390, 248},
	{0x338C, 0xA780},
	{0x3390, 252},
	{0x338C, 0xA781},
	{0x3390, 255},
	{0x338C, 0xA782},
	{0x3390, 0},
	{0x338C, 0xA783},
	{0x3390, 21},
	{0x338C, 0xA784},
	{0x3390, 37},
	{0x338C, 0xA785},
	{0x3390, 63},
	{0x338C, 0xA786},
	{0x3390, 100},
	{0x338C, 0xA787},
	{0x3390, 128},
	{0x338C, 0xA788},
	{0x3390, 151},
	{0x338C, 0xA789},
	{0x3390, 169},
	{0x338C, 0xA78A},
	{0x3390, 186},
	{0x338C, 0xA78B},
	{0x3390, 199},
	{0x338C, 0xA78C},
	{0x3390, 210},
	{0x338C, 0xA78D},
	{0x3390, 220},
	{0x338C, 0xA78E},
	{0x3390, 228},
	{0x338C, 0xA78F},
	{0x3390, 234},
	{0x338C, 0xA790},
	{0x3390, 240},
	{0x338C, 0xA791},
	{0x3390, 244},
	{0x338C, 0xA793},
	{0x3390, 252},
	{0x338C, 0xA794},
	{0x3390, 255},
	{0x338C, 0xA103},
	{0x3390, 6},
};


struct mt9d112_i2c_reg_conf const lens_roll_off_tbl[] = {
	{ 0x34CE, 0x81A0, WORD_LEN, 0 },
	{ 0x34D0, 0x6331, WORD_LEN, 0 },
	{ 0x34D2, 0x3394, WORD_LEN, 0 },
	{ 0x34D4, 0x9966, WORD_LEN, 0 },
	{ 0x34D6, 0x4B25, WORD_LEN, 0 },
	{ 0x34D8, 0x2670, WORD_LEN, 0 },
	{ 0x34DA, 0x724C, WORD_LEN, 0 },
	{ 0x34DC, 0xFFFD, WORD_LEN, 0 },
	{ 0x34DE, 0x00CA, WORD_LEN, 0 },
	{ 0x34E6, 0x00AC, WORD_LEN, 0 },
	{ 0x34EE, 0x0EE1, WORD_LEN, 0 },
	{ 0x34F6, 0x0D87, WORD_LEN, 0 },
	{ 0x3500, 0xE1F7, WORD_LEN, 0 },
	{ 0x3508, 0x1CF4, WORD_LEN, 0 },
	{ 0x3510, 0x1D28, WORD_LEN, 0 },
	{ 0x3518, 0x1F26, WORD_LEN, 0 },
	{ 0x3520, 0x2220, WORD_LEN, 0 },
	{ 0x3528, 0x333D, WORD_LEN, 0 },
	{ 0x3530, 0x15D9, WORD_LEN, 0 },
	{ 0x3538, 0xCFB8, WORD_LEN, 0 },
	{ 0x354C, 0x05FE, WORD_LEN, 0 },
	{ 0x3544, 0x05F8, WORD_LEN, 0 },
	{ 0x355C, 0x0596, WORD_LEN, 0 },
	{ 0x3554, 0x0611, WORD_LEN, 0 },
	{ 0x34E0, 0x00F2, WORD_LEN, 0 },
	{ 0x34E8, 0x00A8, WORD_LEN, 0 },
	{ 0x34F0, 0x0F7B, WORD_LEN, 0 },
	{ 0x34F8, 0x0CD7, WORD_LEN, 0 },
	{ 0x3502, 0xFEDB, WORD_LEN, 0 },
	{ 0x350A, 0x13E4, WORD_LEN, 0 },
	{ 0x3512, 0x1F2C, WORD_LEN, 0 },
	{ 0x351A, 0x1D20, WORD_LEN, 0 },
	{ 0x3522, 0x2422, WORD_LEN, 0 },
	{ 0x352A, 0x2925, WORD_LEN, 0 },
	{ 0x3532, 0x1D04, WORD_LEN, 0 },
	{ 0x353A, 0xFBF2, WORD_LEN, 0 },
	{ 0x354E, 0x0616, WORD_LEN, 0 },
	{ 0x3546, 0x0597, WORD_LEN, 0 },
	{ 0x355E, 0x05CD, WORD_LEN, 0 },
	{ 0x3556, 0x0529, WORD_LEN, 0 },
	{ 0x34E4, 0x00B2, WORD_LEN, 0 },
	{ 0x34EC, 0x005E, WORD_LEN, 0 },
	{ 0x34F4, 0x0F43, WORD_LEN, 0 },
	{ 0x34FC, 0x0E2F, WORD_LEN, 0 },
	{ 0x3506, 0xF9FC, WORD_LEN, 0 },
	{ 0x350E, 0x0CE4, WORD_LEN, 0 },
	{ 0x3516, 0x1E1E, WORD_LEN, 0 },
	{ 0x351E, 0x1B19, WORD_LEN, 0 },
	{ 0x3526, 0x151B, WORD_LEN, 0 },
	{ 0x352E, 0x1416, WORD_LEN, 0 },
	{ 0x3536, 0x10FC, WORD_LEN, 0 },
	{ 0x353E, 0xC018, WORD_LEN, 0 },
	{ 0x3552, 0x06B4, WORD_LEN, 0 },
	{ 0x354A, 0x0506, WORD_LEN, 0 },
	{ 0x3562, 0x06AB, WORD_LEN, 0 },
	{ 0x355A, 0x063A, WORD_LEN, 0 },
	{ 0x34E2, 0x00E5, WORD_LEN, 0 },
	{ 0x34EA, 0x008B, WORD_LEN, 0 },
	{ 0x34F2, 0x0E4C, WORD_LEN, 0 },
	{ 0x34FA, 0x0CA3, WORD_LEN, 0 },
	{ 0x3504, 0x0907, WORD_LEN, 0 },
	{ 0x350C, 0x1DFD, WORD_LEN, 0 },
	{ 0x3514, 0x1E24, WORD_LEN, 0 },
	{ 0x351C, 0x2529, WORD_LEN, 0 },
	{ 0x3524, 0x1D20, WORD_LEN, 0 },
	{ 0x352C, 0x2332, WORD_LEN, 0 },
	{ 0x3534, 0x10E9, WORD_LEN, 0 },
	{ 0x353C, 0x0BCB, WORD_LEN, 0 },
	{ 0x3550, 0x04EF, WORD_LEN, 0 },
	{ 0x3548, 0x0609, WORD_LEN, 0 },
	{ 0x3560, 0x0580, WORD_LEN, 0 },
	{ 0x3558, 0x05DD, WORD_LEN, 0 },
	{ 0x3540, 0x0000, WORD_LEN, 0 },
	{ 0x3542, 0x0000, WORD_LEN, 0 }
};


struct mt9d112_i2c_reg_conf const pll_setup_tbl[] = {
	{ 0x341E, 0x8F09, WORD_LEN, 0 },
	{ 0x341C, 0x0250, WORD_LEN, 0 },
	{ 0x341E, 0x8F09, WORD_LEN, 5 },
	{ 0x341E, 0x8F08, WORD_LEN, 0 }
};

/* Refresh Sequencer */
struct mt9d112_i2c_reg_conf const sequencer_tbl[] = {
	{ 0x338C, 0x2799, WORD_LEN, 0},
	{ 0x3390, 0x6440, WORD_LEN, 5},
	{ 0x338C, 0x279B, WORD_LEN, 0},
	{ 0x3390, 0x6440, WORD_LEN, 5},
	{ 0x338C, 0xA103, WORD_LEN, 0},
	{ 0x3390, 0x0005, WORD_LEN, 5},
	{ 0x338C, 0xA103, WORD_LEN, 0},
	{ 0x3390, 0x0006, WORD_LEN, 5}
};

struct mt9d112_reg_t mt9d112_regs = {
	.prev_snap_reg_settings = &preview_snapshot_mode_reg_settings_array[0],
	.prev_snap_reg_settings_size =
		ARRAY_SIZE(preview_snapshot_mode_reg_settings_array),
	.noise_reduction_reg_settings = &noise_reduction_reg_settings_array[0],
	.noise_reduction_reg_settings_size =
		ARRAY_SIZE(noise_reduction_reg_settings_array),
	.plltbl = &pll_setup_tbl[0],
	.plltbl_size = ARRAY_SIZE(pll_setup_tbl),
	.stbl = &sequencer_tbl[0],
	.stbl_size = ARRAY_SIZE(sequencer_tbl),
	.rftbl = &lens_roll_off_tbl[0],
	.rftbl_size = ARRAY_SIZE(lens_roll_off_tbl)
};



