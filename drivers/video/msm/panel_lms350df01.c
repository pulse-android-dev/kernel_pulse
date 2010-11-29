/* drivers\video\msm\panel_lms350df01.c
 *
 * Copyright (C) 2009 HUAWEI Corporation.
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

#include "mddihost.h"
#include "mddihosti.h"

#include "mddi_tc358721xbg.h"
#include <mach/gpio.h>


/*ms per frame */
#define panel_mspf		17  /* defined ms per frame */

/* define sequence struct */
typedef struct  {
	uint16 reg;
	uint16 val;
} s_pairs_seq_t;

/* define poweron sequence */
static s_pairs_seq_t 
s_poweron_seq[] = {
	{0, 		1			},	//wait 1ms
	{0x07, 	0x0000		},	//reset
	{0, 		10			},	//wait 10ms
	{0x11, 	0x222f		}, 	//
	{0x12, 	0x0f00		}, 	//
	{0x13, 	0x7fe3		}, 	//
	{0x76, 	0x2213		}, 	//
	{0x74, 	0x0001		}, 	//
	{0x76, 	0x0000		}, 	//
	{0x10, 	0x560c		}, 	//
	{0, 		6*panel_mspf	},	//wait for 6frames
	{0x12, 	0x0c63		},
	{0, 		5*panel_mspf	},	//wait for 5frames
	{0x01, 	0x0b3b		}, 	// inversion setting
	//{0x02, 	0x0000		}, 	//
	{0x02,	0x0300		},	//
	{0x03, 	0xc040		}, 	// polarity setting
	//{0x08, 	0x0002		}, 	// vsync back porch
	{0x08,	0x0004		},	// vsync back porch
	//{0x09, 	0x0010		}, 	// hsync back porch
	{0x09,	0x001e		},	// hsync back porch
	{0x76, 	0x2213		}, 	//
	{0x0b, 	0x3340		}, 	//
	{0x0c, 	0x0024		}, 	//RIM 18bit
	{0x1c, 	0x7770		}, 	//
	{0x76, 	0x0000		}, 	//
	{0x0d, 	0x0000		}, 	//
	{0x0e, 	0x0500		}, 	//
	{0x14, 	0x0000		}, 	//
	{0x15, 	0x0803		}, 	//
	{0x16, 	0x0000		}, 	//
	{0x30, 	0x0005		}, 	//
	{0x31, 	0x070f		}, 	//
	{0x32, 	0x0300		}, 	//
	{0x33, 	0x0003		}, 	//
	{0x34, 	0x090c		}, 	//
	{0x35, 	0x0001		}, 	//
	{0x36, 	0x0001		}, 	//
	{0x37, 	0x0303		}, 	//
	{0x38, 	0x0f09		}, 	//
	{0x39, 	0x0105		}	//
};

/* 
 * init sequence of ILITeck's ILI9325DS for Testing 
 */
//#define PANEL_ILITECK
#ifdef PANEL_ILITECK
static s_pairs_seq_t 
s_poweron_seq_e[] = 
{
	{0x00E5, 0x8000},	   // Set the internal vcore voltage										
	{0x00E3, 0x3008}, // Set internal timing
	{0x00E7, 0x0012}, // Set internal timing
	{0x00EF, 0x1231}, // Set internal timing
	{0x0001, 0x0100}, // set SS and SM bit
	{0x0002, 0x0700}, // set 1 line inversin
	{0x0003, 0x1030}, // set GRAM write di  1.RGB Internal  
	{0x0004, 0x0000}, // Resize register


	{0x0008, 0x0505}, // set the back porch and front porch   10 back+5 front
	{0x0009, 0x0000}, // set non-display area refresh cycle ISC[3:0]
	{0x000A, 0x0008}, // FMARK function

	{0x000C, 0x0110}, // RGB interface setting //2.RGB 
	{0x000D, 0x0000}, // Frame marker Position
	{0x000F, 0x0002}, // RGB interf

	//*************Power On sequence ****************// 
	{0x0010, 0x0000}, // SAP, BT[3:0], AP, DSTB, SLP, STB
	{0x0011, 0x0007}, // DC1[2:0], DC0[2:0], VC[2:0]
	{0x0012, 0x0000}, // VREG1OUT voltage
	{0x0013, 0x0000}, // VDV[4:0] for VCOM amplitude

	{0, 200000}, // Dis-charge capacitor power voltage


	{0x0010, 0x1190}, // SAP, BT[3:0], AP, DSTB, SLP, STB

	{0x0011, 0x0227}, // DC1[2:0], DC0[2:0], VC[2:0]

	{0x00ec, 0x266F}, //new add


	{0, 50000}, // Delay 50ms

	{0x0012, 0x009a},		 // VREG1OUT voltage

	{0, 50000},					 // Delay 50ms

	{0x0013, 0x1800},		 // VDV[4:0] for VCOM amplitude
	{0x0029, 0x0021},		 // VCM[4:0] for VCOMH		Delayms(50},  
	  
	{0x0020, 0x0000},		 // GRAM horizontal Address
	{0x0021, 0x0000},		 // GRAM Vertical Address
	{0x002B, 0x000C},		 //Frane rate setting
	// ----------- Adjust the Gamma Curve ----------//
	{0x0030, 0x0401},	  
	{0x0031, 0x0307},	  
	{0x0032, 0x0302},
	{0x0035, 0x0002},	  
	{0x0036, 0x1407},	  
	{0x0037, 0x0504},	  
	{0x0038, 0x0004},	  
	{0x0039, 0x0603},	 
	{0x003C, 0x0200},	  
	{0x003D, 0x060e},
	//------------------ Set GRAM area ---------------//
	{0x0050, 0x0000}, // Horizontal GRAM Start Address
	{0x0051, 0x00EF}, // Horizontal GRAM End Address
	{0x0052, 0x0000}, // Vertical GRAM Start Address
	{0x0053, 0x013F}, // Vertical GRAM End Address   

	{0x0060, 0xa700}, // Gate Scan Line
	{0x0061, 0x0001}, // NDL,VLE, REV
	{0x006A, 0x0000}, // set scrolling line

	//-------------- Partial Display Control ---------//
	{0x0080, 0x0014},	  
	{0x0081, 0x0014},
	{0x0082, 0x007c},
	{0x0083, 0x0090},	
	{0x0084, 0x0090},
	{0x0085, 0x00f8},
	//-------------- Panel Control -------------------//


	{0x0090, 0x0010},
	{0x0092, 0x0000},
	{0x0007, 0x0133},	  // 262K color and display ON	  
	{0x0022, 0},
};
#endif

/* define poweroff sequence */
static s_pairs_seq_t 
s_poweroff_seq[] = {
	{0x10, 	0x0001		}, 	//
	{0x0b, 	0x30e1		}, 	//
	{0x07, 	0x0102		}, 	//
	{0, 		2*panel_mspf	}, 	//wait for 2frames
	{0x07, 	0x0000		}, 	//
	{0x12, 	0x0000		},	//
	{0x10, 	0x0100		}	//			
};

/* define displayon sequence */
static s_pairs_seq_t 
s_displayon_seq[] = {
	{0x07, 	0x0001		}, 	//
	{0, 		1*panel_mspf	},	//wait for 1frames
	{0x07, 	0x0101		}, 	//
	{0, 		2*panel_mspf	},	//wait for 2frames
	{0x07, 	0x0103		}	//			
};

/* define standby in sequence */
static s_pairs_seq_t 
s_standby_in_seq[] = {
	// poweroff seq;
	{0x10, 	0x0001		}	//
};

/* define standby out sequence */
static s_pairs_seq_t 
s_standby_out_seq[] = {
	{0x10, 	0x0000		}	//
	// poweron seq;
	// displayon seq;
};

/* state machine define */
typedef enum {
	E_STATE_POWERON,
	E_STATE_POWEROFF,
	E_STATE_DISPLAYON,
	E_STATE_STANDBY_IN,
	E_STATE_STANDBY_OUT
} e_panel_state_t;
static e_panel_state_t s_panel_state = E_STATE_POWEROFF;


/* GPIO and Software simulate SPI signals */

#ifdef SPI_SW_SIMULATE

/* simulating SPI interface */
#define NCS(x)	gpio_direction_output(17,x)
#define SCL(x)	gpio_direction_output(26,x)
#define SDI(x)		gpio_direction_output(88,x)
#define Delayus(x)	udelay(x)

static void WriteI(unsigned short c)
{
	unsigned int dd, da;
	unsigned char i ;
	NCS(1);
	Delayus(1);
	SCL(1);
	Delayus(1);
	dd = 0x740000 + c ;
	NCS(0);
	Delayus(1);
	for(i = 0 ; i < 24 ; i ++)
	{
		SCL(0);
		Delayus(1);
		da = (dd & 0x800000 )>0 ? 1 : 0;
		SDI(da);
		Delayus(1);
		SCL(1);
		dd = dd << 1 ;
		Delayus(1);
	}  
	NCS(1);  
}

static void WriteD(unsigned short d)
{
	unsigned int dd, da;
	unsigned char i ;
	NCS(1);
	Delayus(1);
	SCL(1);
	Delayus(1);
	dd = 0x760000 + d ;
	NCS(0);
	Delayus(1);
	for(i = 0 ; i < 24 ; i ++)
	{ 
		Delayus(1);
		SCL(0);
		Delayus(1);
		da = (dd & 0x800000 )>0 ? 1 : 0;
		SDI(da);
		Delayus(1);
		SCL(1);
		dd = dd << 1 ;
		Delayus(1);
	} 
	Delayus(1); 
	NCS(1);  
}

static void Init_Data(unsigned short c, unsigned short d)
{
	WriteI(c);
	WriteD(d);
}

#endif

static void s_reg_val_op(uint8 reg, uint16 val)
{
#ifdef SPI_SW_SIMULATE

	if(0 == reg)
		mddi_wait(val);
	else
		Init_Data(reg, val);
	
#else
{
	uint32 i;
	
	if(0 == reg){
		
#ifdef PANEL_ILITECK
		udelay(val);
#else
		mddi_wait(val);
#endif

	}
	else
	{
		uint32 spid[4], ssiints, ssictl;
		
#ifdef PANEL_ILITECK
		spid[0] = 0x00080070;
		spid[1] = 0x00010000 | reg;
		spid[2] = 0x00080072;
		spid[3] = 0x00010000 | val;
#else
		spid[0] = 0x00080074;
		spid[1] = 0x00010000 | reg;
		spid[2] = 0x00080076;
		spid[3] = 0x00010000 | val;
#endif

check_again:
		// SSIINTS.TXNUM[2:0]==000 && SSIINTS.SIFACT==0
		mddi_queue_register_read(SSIINTS, &ssiints, TRUE, 0);
		if(ssiints & 0x00701000) 
			goto check_again;

		// SSICTL.SETACT=0
		mddi_queue_register_read(SSICTL, &ssictl, TRUE, 0);
		ssictl &= 0xFFFFFFFD;
		mddi_queue_register_write(SSICTL,ssictl,TRUE,0);

		// SSITX(4)
		for(i=0; i < sizeof(spid)/sizeof(uint32); i++)
			mddi_queue_register_write(SSITX,spid[i],TRUE,0);
		
		// SSICTL.SETACT=1
		mddi_queue_register_read(SSICTL, &ssictl, TRUE, 0);
		ssictl |= 0x00000002;
		mddi_queue_register_write(SSICTL,ssictl,TRUE,0);

		// waiting FIFO empty
		//mddi_wait(1);

#if 0 // read register and compare if be equal to the val writed abovely
	{
		uint32 spiread[2], reg_read;
		spiread[0] = 0x000C0075;
		spiread[1] = 0x00010000 | reg;
check_reg:
		// SSIINTS.TXNUM[2:0]==000 && SSIINTS.SIFACT==0
		mddi_queue_register_read(SSIINTS, &ssiints, TRUE, 0);
		if(ssiints & 0x00701000) 
			goto check_reg;
		// SSIINTS.RXNEMP==0
		mddi_queue_register_read(SSIINTS, &ssiints, TRUE, 0);
		if(ssiints & 0x00000200) {
			mddi_queue_register_read(SSIRX, &reg_read, TRUE, 0);
			goto check_reg;
		}
		
		// SSICTL.SETACT=0
		mddi_queue_register_read(SSICTL, &ssictl, TRUE, 0);
		ssictl &= 0xFFFFFFFD;
		mddi_queue_register_write(SSICTL,ssictl,TRUE,0);
		
		// SSITX(2)
		for(i=0; i < sizeof(spiread)/sizeof(uint32); i++)
			mddi_queue_register_write(SSITX,spiread[i],TRUE,0);
		
		// SSICTL.SETACT=1
		mddi_queue_register_read(SSICTL, &ssictl, TRUE, 0);
		ssictl |= 0x00000002;
		mddi_queue_register_write(SSICTL,ssictl,TRUE,0);

check_read:
		// SSIINTS.RXNEMP==1 && SSIINTS.SIFACT==0
		mddi_queue_register_read(SSIINTS, &ssiints, TRUE, 0);
		if(ssiints & 0x00001000) 
			goto check_read;
		if(ssiints & 0x00000200){
			mddi_queue_register_read(SSIRX, &reg_read, TRUE, 0);
			printk("SPI write val =%x, \n     read val = %x\n", val, reg_read);
		}else
			printk("SPI write val =%x, \n     read val is not exist!\n", val);
	}
#endif

	}
}
#endif

}

/* power on op */
static void s_lms350df01_power_on(void)
{
	int i = 0;
	int count = sizeof(s_poweron_seq)/
		sizeof(s_pairs_seq_t);
	
	for(i=0; i<count; i++)
		s_reg_val_op(
		s_poweron_seq[i].reg, 
		s_poweron_seq[i].val);

	s_panel_state = E_STATE_POWERON;
}

/* power off  op*/ 
static void s_lms350df01_power_off(void)
{
	int i = 0;
	int count = sizeof(s_poweroff_seq)/
		sizeof(s_pairs_seq_t);
		
	for(i=0; i<count; i++)
		s_reg_val_op(
		s_poweroff_seq[i].reg, 
		s_poweroff_seq[i].val);

	s_panel_state = E_STATE_POWEROFF;
}

/* display on  op*/ 
static void s_lms350df01_display_on(void)
{
	int i = 0;
	int count = sizeof(s_displayon_seq)/
		sizeof(s_pairs_seq_t);

	// state check
	if(s_panel_state != E_STATE_POWERON)
		s_lms350df01_power_on();
		
	for(i=0; i<count; i++)
		s_reg_val_op(
		s_displayon_seq[i].reg, 
		s_displayon_seq[i].val);
	
	s_panel_state = E_STATE_DISPLAYON;
}

/* standby in  op*/ 
static void s_lms350df01_standby_in(void)
{
	int i = 0;
	int count = sizeof(s_standby_in_seq)/
		sizeof(s_pairs_seq_t);

	// state check
	if(s_panel_state != E_STATE_DISPLAYON)
		MDDI_MSG_ERR("Panel state should be display on\n");
	
	// poweroff
	s_lms350df01_power_off();
	
	//
	for(i=0; i<count; i++)
		s_reg_val_op(
		s_standby_in_seq[i].reg, 
		s_standby_in_seq[i].val);
	
	s_panel_state = E_STATE_STANDBY_IN;
}

/* standby out  op*/ 
static void s_lms350df01_standby_out(void)
{
	int i = 0;
	int count = sizeof(s_standby_out_seq)/
		sizeof(s_pairs_seq_t);

	// state check
	if(s_panel_state != E_STATE_STANDBY_IN)
		MDDI_MSG_ERR("Panel state should be stanby in\n");
		
	for(i=0; i<count; i++)
		s_reg_val_op(
		s_standby_out_seq[i].reg, 
		s_standby_out_seq[i].val);

	// poweron
	s_lms350df01_power_on();
	// displayon
	s_lms350df01_display_on();
	
	s_panel_state = E_STATE_STANDBY_OUT;	
}

/* export for tc358721*/
s_panel_seq_t panel_lms350df01={
	.power_on = s_lms350df01_power_on,
	.power_off = s_lms350df01_power_off,
	.display_on = s_lms350df01_display_on,
	.display_off = NULL,
	.sleep_in = NULL,
	.sleep_out = NULL,
	.standby_in = s_lms350df01_standby_in,
	.standby_out = s_lms350df01_standby_out
};

