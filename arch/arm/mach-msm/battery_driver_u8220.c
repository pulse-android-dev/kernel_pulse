/* arch/arm/mach-msm/u8220_battery.c
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
#include <proc_comm.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/power_supply.h>
#include <linux/platform_device.h>
#include <linux/debugfs.h>
#include <mach/board.h>
#include <linux/timer.h>
#include <battery_device_u8220.h>

#define BUF_SUM 6

#define HEALTH_TEMP_MAX 45       /* define temperature max,and decide whether it's overheat  */
#define HEALTH_VOLT_MAX 4250     /* define voltage max,and decide whether it's overvoltage  */

#define USB_VIRTUAL_VOL 25    /* virtual voltage in usb charging(mV)  */
#define AC_VIRTUAL_VOL 80     /* virtual voltage in ac charging(mV)  */
#define BATTERY_POLLING_JIFFIES (3 * HZ)  /* define timer's time,3*Hz=3000ms  */
struct battery_info_reply
{
    u32 batt_id;                     /* Battery ID from ADC */
    u32 batt_vol;                 /* Battery voltage from ADC */
    u32 batt_temp;             /* Battery Temperature (C) from formula and ADC */
    u32 batt_current;          /* Battery current from ADC */
    u32 level;                    /* formula */
    u32 charging_source;           /* 0: no cable, 1:usb, 2:AC */
    u32 charging_enabled;    /* 0: Disable, 1: Enable */
    u32 full_bat;                     /* Full capacity of battery (mAh) */
    u32 present_in;
    u32 charging_event;

    u32 power_off_charging;

};

struct huawei_battery_info
{
    int present;

    /* lock to protect the battery info */
    struct mutex              lock;
    struct battery_info_reply rep;
};

static struct huawei_battery_info huawei_batt_info;

static enum power_supply_property huawei_battery_properties[] =
{
    POWER_SUPPLY_PROP_STATUS,
    POWER_SUPPLY_PROP_HEALTH,
    POWER_SUPPLY_PROP_TECHNOLOGY,
    POWER_SUPPLY_PROP_PRESENT,
    POWER_SUPPLY_PROP_CAPACITY,
};

static enum power_supply_property huawei_power_properties[] =
{
    POWER_SUPPLY_PROP_ONLINE,
};

typedef enum
{
    CHARGER_BATTERY = 0,
    CHARGER_USB,
    CHARGER_AC,
    CHARGER_INVALID
} charger_type_t;

struct volt_capacity
{
    int volt;
    int capacity_level;
};

struct volt_capacity volt_capacity_value[] =
{
    {.volt = 3521, .capacity_level =   0,},
    {.volt = 3544, .capacity_level =   1,},
    {.volt = 3573, .capacity_level =   2,},
    {.volt = 3576, .capacity_level =   3,},
    {.volt = 3582, .capacity_level =   4,},
    {.volt = 3586, .capacity_level =   5,},
    {.volt = 3592, .capacity_level =   6,},
    {.volt = 3598, .capacity_level =   7,},
    {.volt = 3604, .capacity_level =   8,},
    {.volt = 3610, .capacity_level =   9,},
    {.volt = 3618, .capacity_level =  10,},
    {.volt = 3625, .capacity_level =  11,},
    {.volt = 3630, .capacity_level =  12,},
    {.volt = 3634, .capacity_level =  13,},
    {.volt = 3640, .capacity_level =  14,},
    {.volt = 3646, .capacity_level =  15,},
    {.volt = 3652, .capacity_level =  16,},
    {.volt = 3654, .capacity_level =  17,},
    {.volt = 3658, .capacity_level =  18,},
    {.volt = 3662, .capacity_level =  19,},
    {.volt = 3665, .capacity_level =  20,},
    {.volt = 3669, .capacity_level =  21,},
    {.volt = 3672, .capacity_level =  22,},
    {.volt = 3676, .capacity_level =  23,},
    {.volt = 3678, .capacity_level =  24,},
    {.volt = 3680, .capacity_level =  25,},
    {.volt = 3681, .capacity_level =  26,},
    {.volt = 3684, .capacity_level =  27,},
    {.volt = 3688, .capacity_level =  28,},
    {.volt = 3689, .capacity_level =  29,},
    {.volt = 3692, .capacity_level =  30,},
    {.volt = 3692, .capacity_level =  31,},
    {.volt = 3693, .capacity_level =  32,},
    {.volt = 3697, .capacity_level =  33,},
    {.volt = 3699, .capacity_level =  34,},
    {.volt = 3700, .capacity_level =  35,},
    {.volt = 3703, .capacity_level =  36,},
    {.volt = 3707, .capacity_level =  37,},
    {.volt = 3709, .capacity_level =  38,},
    {.volt = 3710, .capacity_level =  39,},
    {.volt = 3715, .capacity_level =  40,},
    {.volt = 3717, .capacity_level =  41,},
    {.volt = 3720, .capacity_level =  42,},
    {.volt = 3724, .capacity_level =  43,},
    {.volt = 3727, .capacity_level =  44,},
    {.volt = 3730, .capacity_level =  45,},
    {.volt = 3732, .capacity_level =  46,},
    {.volt = 3737, .capacity_level =  47,},
    {.volt = 3738, .capacity_level =  48,},
    {.volt = 3733, .capacity_level =  49,},
    {.volt = 3748, .capacity_level =  50,},
    {.volt = 3750, .capacity_level =  51,},
    {.volt = 3752, .capacity_level =  52,},
    {.volt = 3758, .capacity_level =  53,},
    {.volt = 3762, .capacity_level =  54,},
    {.volt = 3767, .capacity_level =  55,},
    {.volt = 3772, .capacity_level =  56,},
    {.volt = 3775, .capacity_level =  57,},
    {.volt = 3781, .capacity_level =  58,},
    {.volt = 3786, .capacity_level =  59,},
    {.volt = 3789, .capacity_level =  60,},
    {.volt = 3795, .capacity_level =  61,},
    {.volt = 3800, .capacity_level =  62,},
    {.volt = 3804, .capacity_level =  63,},
    {.volt = 3810, .capacity_level =  64,},
    {.volt = 3815, .capacity_level =  65,},
    {.volt = 3822, .capacity_level =  66,},
    {.volt = 3826, .capacity_level =  67,},
    {.volt = 3836, .capacity_level =  68,},
    {.volt = 3841, .capacity_level =  69,},
    {.volt = 3845, .capacity_level =  70,},
    {.volt = 3855, .capacity_level =  71,},
    {.volt = 3861, .capacity_level =  72,},
    {.volt = 3865, .capacity_level =  73,},
    {.volt = 3873, .capacity_level =  74,},
    {.volt = 3880, .capacity_level =  75,},
    {.volt = 3887, .capacity_level =  76,},
    {.volt = 3893, .capacity_level =  77,},
    {.volt = 3905, .capacity_level =  78,},
    {.volt = 3909, .capacity_level =  79,},
    {.volt = 3915, .capacity_level =  80,},
    {.volt = 3926, .capacity_level =  81,},
    {.volt = 3935, .capacity_level =  82,},
    {.volt = 3942, .capacity_level =  83,},
    {.volt = 3949, .capacity_level =  84,},
    {.volt = 3960, .capacity_level =  85,},
    {.volt = 3964, .capacity_level =  86,},
    {.volt = 3976, .capacity_level =  87,},
    {.volt = 3985, .capacity_level =  88,},
    {.volt = 3992, .capacity_level =  89,},
    {.volt = 4002, .capacity_level =  90,},
    {.volt = 4010, .capacity_level =  91,},
    {.volt = 4020, .capacity_level =  92,},
    {.volt = 4027, .capacity_level =  93,},
    {.volt = 4040, .capacity_level =  94,},
    {.volt = 4048, .capacity_level =  95,},
    {.volt = 4060, .capacity_level =  96,},
    {.volt = 4071, .capacity_level =  97,},
    {.volt = 4081, .capacity_level =  98,},
    {.volt = 4100, .capacity_level =  99,},
    {.volt = 4134, .capacity_level = 100,},
};


typedef enum
{
    CHG_IDLE_ST,                                          /* Charger state machine entry point.       */
    CHG_WALL_IDLE_ST,                               /* Wall charger state machine entry point.  */
    CHG_WALL_TRICKLE_ST,                          /* Wall charger low batt charging state.    */
    CHG_WALL_NO_TX_FAST_ST,                 /* Wall charger high I charging state.      */
    CHG_WALL_FAST_ST,                              /* Wall charger high I charging state.      */
    CHG_WALL_TOPOFF_ST,                         /* Wall charger top off charging state.     */
    CHG_WALL_MAINT_ST,                           /* Wall charger maintance charging state.   */
    CHG_WALL_TX_WAIT_ST,                       /* Wall charger TX WAIT charging state.     */
    CHG_WALL_ERR_WK_BAT_WK_CHG_ST, /* Wall CHG ERR: weak batt and weak charger.*/
    CHG_WALL_ERR_WK_BAT_BD_CHG_ST,  /* Wall CHG ERR: weak batt and bad charger. */
    CHG_WALL_ERR_GD_BAT_BD_CHG_ST,  /* Wall CHG ERR: good batt and bad charger. */
    CHG_WALL_ERR_GD_BAT_WK_CHG_ST, /* Wall CHG ERR: good batt and weak charger.*/
    CHG_WALL_ERR_BD_BAT_GD_CHG_ST, /* Wall CHG ERR: Bad batt and good charger. */
    CHG_WALL_ERR_BD_BAT_WK_CHG_ST,/* Wall CHG ERR: Bad batt and weak charger. */
    CHG_WALL_ERR_BD_BAT_BD_CHG_ST,/* Wall CHG ERR: Bad batt and bad charger.  */
    CHG_WALL_ERR_GD_BAT_BD_BTEMP_CHG_ST,/* Wall CHG ERR: GD batt and BD batt temp */
    CHG_WALL_ERR_WK_BAT_BD_BTEMP_CHG_ST,/* Wall CHG ERR: WK batt and BD batt temp */
    CHG_USB_IDLE_ST,                                            /* USB charger state machine entry point.   */
    CHG_USB_TRICKLE_ST,                                      /* USB charger low batt charging state.     */
    CHG_USB_NO_TX_FAST_ST,                              /* USB charger high I charging state.       */
    CHG_USB_FAST_ST,                                          /* USB charger high I charging state.       */
    CHG_USB_TOPOFF_ST,                                     /* USB charger top off state charging state.*/
    CHG_USB_DONE_ST,                                         /* USB charger Done charging state.         */
    CHG_USB_ERR_WK_BAT_WK_CHG_ST,     /* USB CHG ERR: weak batt and weak charger. */
    CHG_USB_ERR_WK_BAT_BD_CHG_ST,     /* USB CHG ERR: weak batt and bad charger.  */
    CHG_USB_ERR_GD_BAT_BD_CHG_ST,     /* USB CHG ERR: good batt and bad charger.  */
    CHG_USB_ERR_GD_BAT_WK_CHG_ST,    /* USB CHG ERR: good batt and weak charger. */
    CHG_USB_ERR_BD_BAT_GD_CHG_ST,    /* USB CHG ERR: Bad batt and good charger.  */
    CHG_USB_ERR_BD_BAT_WK_CHG_ST,   /* USB CHG ERR: Bad batt and weak charger.  */
    CHG_USB_ERR_BD_BAT_BD_CHG_ST,   /* USB CHG ERR: Bad batt and bad charger.   */
    CHG_USB_ERR_GD_BAT_BD_BTEMP_CHG_ST,/* USB CHG ERR: GD batt and BD batt temp */
    CHG_USB_ERR_WK_BAT_BD_BTEMP_CHG_ST,/* USB CHG ERR: WK batt and BD batt temp */
    CHG_VBATDET_CAL_ST,                                  /* VBATDET calibration state*/
    CHG_INVALID_ST
} chg_state_type;

static int get_usb_ac_charging_status(chg_state_type charging_status_para)
{
    int result_val;

    switch (charging_status_para)
    {
        case CHG_WALL_TRICKLE_ST:             /* Wall charger low batt charging state.    */
        case CHG_WALL_NO_TX_FAST_ST:       /* Wall charger high I charging state.      */
        case CHG_WALL_FAST_ST:                  /* Wall charger high I charging state.      */
        case CHG_WALL_TOPOFF_ST:               /* Wall charger top off charging state.     */
        case CHG_WALL_TX_WAIT_ST:            /* Wall charger TX WAIT charging state.     */
        case CHG_USB_TRICKLE_ST:              /* USB charger low batt charging state.     */
        case CHG_USB_NO_TX_FAST_ST:        /* USB charger high I charging state.       */
        case CHG_USB_FAST_ST:                   /* USB charger high I charging state.       */
        case CHG_USB_TOPOFF_ST:               /* USB charger top off state charging state.*/
            result_val = POWER_SUPPLY_STATUS_CHARGING;
            break;
        case CHG_IDLE_ST:                     /* Charger state machine entry point.  */
        case CHG_WALL_IDLE_ST:            /* Wall charger state machine entry point.  */
        case CHG_USB_IDLE_ST:              /* USB charger state machine entry point.   */
        case CHG_VBATDET_CAL_ST:      /* VBATDET calibration state*/
            result_val = POWER_SUPPLY_STATUS_NOT_CHARGING;
            break;
        case CHG_WALL_MAINT_ST:                /* Wall charger maintance charging state.  */
        case CHG_USB_DONE_ST:                   /* USB charger Done charging state.   */
            result_val = POWER_SUPPLY_STATUS_FULL;
            break;
        case CHG_WALL_ERR_WK_BAT_WK_CHG_ST:             /* Wall CHG ERR: weak batt and weak charger.*/
        case CHG_WALL_ERR_WK_BAT_BD_CHG_ST:             /* Wall CHG ERR: weak batt and bad charger. */
        case CHG_WALL_ERR_GD_BAT_BD_CHG_ST:              /* Wall CHG ERR: good batt and bad charger. */
        case CHG_WALL_ERR_GD_BAT_WK_CHG_ST:              /* Wall CHG ERR: good batt and weak charger.*/
        case CHG_WALL_ERR_BD_BAT_GD_CHG_ST:              /*   Wall CHG ERR: Bad batt and good charger. */
        case CHG_WALL_ERR_BD_BAT_WK_CHG_ST:              /*   Wall CHG ERR: Bad batt and weak charger. */
        case CHG_WALL_ERR_BD_BAT_BD_CHG_ST:              /*   Wall CHG ERR: Bad batt and bad charger.  */
        case CHG_WALL_ERR_GD_BAT_BD_BTEMP_CHG_ST:   /*   Wall CHG ERR: GD batt and BD batt temp */
        case CHG_WALL_ERR_WK_BAT_BD_BTEMP_CHG_ST:   /* USB CHG ERR: WK batt and BD batt temp */
        case CHG_USB_ERR_WK_BAT_WK_CHG_ST:               /* USB CHG ERR: weak batt and weak charger. */
        case CHG_USB_ERR_WK_BAT_BD_CHG_ST:               /* USB CHG ERR: weak batt and bad charger.  */
        case CHG_USB_ERR_GD_BAT_BD_CHG_ST:               /* USB CHG ERR: good batt and bad charger.  */
        case CHG_USB_ERR_GD_BAT_WK_CHG_ST:              /* USB CHG ERR: good batt and weak charger. */
        case CHG_USB_ERR_BD_BAT_GD_CHG_ST:              /*  USB CHG ERR: Bad batt and good charger.  */
        case CHG_USB_ERR_BD_BAT_WK_CHG_ST:             /* USB CHG ERR: Bad batt and weak charger.  */
        case CHG_USB_ERR_BD_BAT_BD_CHG_ST:              /* USB CHG ERR: Bad batt and bad charger.   */
        case CHG_USB_ERR_GD_BAT_BD_BTEMP_CHG_ST:  /* USB CHG ERR: GD batt and BD batt temp */
        case CHG_USB_ERR_WK_BAT_BD_BTEMP_CHG_ST:  /*   USB CHG ERR: WK batt and BD batt temp */
            result_val = POWER_SUPPLY_STATUS_DISCHARGING;
            break;
        case CHG_INVALID_ST:
            result_val = POWER_SUPPLY_STATUS_UNKNOWN;
            break;
        default:
            result_val = POWER_SUPPLY_STATUS_UNKNOWN;
    }

    return result_val;
}

static int get_power_source(chg_state_type charging_status_parameter, u32 present_para)
{
    int result_value;

    switch (charging_status_parameter)
    {
        case CHG_WALL_TRICKLE_ST:             /* Wall charger low batt charging state.    */
        case CHG_WALL_NO_TX_FAST_ST:       /* Wall charger high I charging state.      */
        case CHG_WALL_FAST_ST:                  /* Wall charger high I charging state.      */
        case CHG_WALL_TOPOFF_ST:               /* Wall charger top off charging state.     */
        case CHG_WALL_TX_WAIT_ST:            /* Wall charger TX WAIT charging state.     */
        case CHG_WALL_MAINT_ST:               /* Wall charger maintance charging state.  */
            result_value = CHARGER_AC;
            break;
        case CHG_USB_TRICKLE_ST:              /* USB charger low batt charging state.     */
        case CHG_USB_NO_TX_FAST_ST:        /* USB charger high I charging state.       */
        case CHG_USB_FAST_ST:                   /* USB charger high I charging state.       */
        case CHG_USB_TOPOFF_ST:               /* USB charger top off state charging state.*/
        case CHG_USB_DONE_ST:                  /* USB charger Done charging state.   */
            result_value = CHARGER_USB;
            break;
        default:
            if (present_para)
            {
                result_value = CHARGER_BATTERY;
            }
            else
            {
                result_value = CHARGER_INVALID;
            }
    }

    return result_value;
}

static int huawei_battery_get_charging_status(void)
{
    u32 level;
    charger_type_t charger;
    int ret;
    int usb_ac_ret;

    charger = get_power_source(huawei_batt_info.rep.charging_event, huawei_batt_info.rep.present_in);
    usb_ac_ret = get_usb_ac_charging_status(huawei_batt_info.rep.charging_event);
    switch (charger)
    {
        case CHARGER_BATTERY:
            ret = POWER_SUPPLY_STATUS_NOT_CHARGING;
            break;
        case CHARGER_USB:
        case CHARGER_AC:
            level = huawei_batt_info.rep.level;
            if (level == 100)
            {
                ret = POWER_SUPPLY_STATUS_FULL;
            }
            else
            {
                ret = POWER_SUPPLY_STATUS_CHARGING;
            }

            break;
        default:
            ret = POWER_SUPPLY_STATUS_UNKNOWN;
    }

    if ((POWER_SUPPLY_STATUS_FULL == ret) || (POWER_SUPPLY_STATUS_FULL == usb_ac_ret))
    {
        usb_ac_ret = POWER_SUPPLY_STATUS_FULL;
    }

    return usb_ac_ret;
}


static int huawei_battery_get_battery_health(void)
{
    int health_present;
    int health_temp;
    int health_volt;
    int relt;

    health_present = huawei_batt_info.rep.present_in;
    health_temp = huawei_batt_info.rep.batt_temp;
    health_volt = huawei_batt_info.rep.batt_vol;
    if (!health_present)
    {
        relt = POWER_SUPPLY_HEALTH_UNKNOWN;
    }
    else if (health_temp / 10 > HEALTH_TEMP_MAX)
    {
        relt = POWER_SUPPLY_HEALTH_OVERHEAT;
    }
    else if (health_volt > HEALTH_VOLT_MAX)
    {
        relt = POWER_SUPPLY_HEALTH_OVERVOLTAGE;
    }
    else
    {
        relt = POWER_SUPPLY_HEALTH_GOOD;
    }

    return relt;
}

static int huawei_battery_get_property(struct power_supply *       psy,
                                       enum power_supply_property  psp,
                                       union power_supply_propval *val)
{
    switch (psp)
    {
        case POWER_SUPPLY_PROP_STATUS:
            val->intval = huawei_battery_get_charging_status();
            break;
        case POWER_SUPPLY_PROP_HEALTH:
            val->intval = huawei_battery_get_battery_health();
            break;
        case POWER_SUPPLY_PROP_TECHNOLOGY:
            val->intval = POWER_SUPPLY_TECHNOLOGY_LION;
            break;
        case POWER_SUPPLY_PROP_PRESENT:
            val->intval = huawei_batt_info.present;
            break;
        case POWER_SUPPLY_PROP_CAPACITY:
            mutex_lock(&huawei_batt_info.lock);
            val->intval = huawei_batt_info.rep.level;
            mutex_unlock(&huawei_batt_info.lock);
            break;
        default:
            return -EINVAL;
    }

    return 0;
}

static int huawei_power_get_property(struct power_supply *       psy,
                                     enum power_supply_property  psp,
                                     union power_supply_propval *val)
{
    charger_type_t charger;

    charger = get_power_source(huawei_batt_info.rep.charging_event, huawei_batt_info.rep.present_in);
    switch (psp)
    {
        case POWER_SUPPLY_PROP_ONLINE:
            if (psy->type == POWER_SUPPLY_TYPE_MAINS)
            {
                val->intval = (charger == CHARGER_AC ? 1 : 0);
            }
            else if (psy->type == POWER_SUPPLY_TYPE_USB)
            {
                val->intval = (charger == CHARGER_USB ? 1 : 0);
            }
            else
            {
                val->intval = 0;
            }

            break;
        default:
            return -EINVAL;
    }

    return 0;
}

static char *supply_list[] = {
    "battery",
};

/* huawei dedicated attributes */

static struct power_supply huawei_power_supplies[] =
{
    {
        .name = "battery",
        .type = POWER_SUPPLY_TYPE_BATTERY,
        .properties = huawei_battery_properties,
        .num_properties = ARRAY_SIZE(huawei_battery_properties),
        .get_property = huawei_battery_get_property,
    },
    {
        .name = "usb",
        .type = POWER_SUPPLY_TYPE_USB,
        .supplied_to = supply_list,
        .num_supplicants = ARRAY_SIZE(supply_list),
        .properties = huawei_power_properties,
        .num_properties = ARRAY_SIZE(huawei_power_properties),
        .get_property = huawei_power_get_property,
    },
    {
        .name = "ac",
        .type = POWER_SUPPLY_TYPE_MAINS,
        .supplied_to = supply_list,
        .num_supplicants = ARRAY_SIZE(supply_list),
        .properties = huawei_power_properties,
        .num_properties = ARRAY_SIZE(huawei_power_properties),
        .get_property = huawei_power_get_property,
    },
};

enum
{
    BATT_ID = 0,
    BATT_LEVEL,
    BATT_VOL,
    BATT_TEMP,
    BATT_CURRENT,
    CHARGING_SOURCE,
    CHARGING_ENABLED,
    FULL_BAT,
    BATT_PRESENT_IN,
    CHARGING_EVENT,

    POWER_OFF_CHARGING,

};

static ssize_t huawei_battery_show_property(struct device *          dev,
                                            struct device_attribute *attr,
                                            char *                   buf);

#define HUAWEI_BATTERY_ATTR(_name) \
    {                                       \
        .attr = { .name = # _name, .mode = S_IRUGO, .owner = THIS_MODULE }, \
                                                             .show  = huawei_battery_show_property, \
                                                             .store = NULL, \
                                                             }

static struct device_attribute huawei_battery_attrs[] =
{
    HUAWEI_BATTERY_ATTR(batt_id),
    HUAWEI_BATTERY_ATTR(level),
    HUAWEI_BATTERY_ATTR(batt_vol),
    HUAWEI_BATTERY_ATTR(batt_temp),
    HUAWEI_BATTERY_ATTR(batt_current),
    HUAWEI_BATTERY_ATTR(charging_source),
    HUAWEI_BATTERY_ATTR(charging_enabled),
    HUAWEI_BATTERY_ATTR(full_bat),
    HUAWEI_BATTERY_ATTR(present_in),
    HUAWEI_BATTERY_ATTR(charging_event),

    HUAWEI_BATTERY_ATTR(power_off_charging),

};

static int huawei_battery_create_attrs(struct device * dev)
{
    int i, rc;

    for (i = 0; i < ARRAY_SIZE(huawei_battery_attrs); i++)
    {
        rc = device_create_file(dev, &huawei_battery_attrs[i]);
        if (rc)
        {
            goto huawei_attrs_failed;
        }
    }

    goto succeed;

huawei_attrs_failed:
    while (i--)
    {
        device_remove_file(dev, &huawei_battery_attrs[i]);
    }

succeed:
    return rc;
}

int get_batt_mV(unsigned id)
{
    if (msm_proc_comm(PCOM_GET_BATT_MV_LEVEL, &id, NULL))
    {
        return -1;
    }
    else
    {
        return (int) id;
    }
}


int get_batt_temp_and_source(unsigned  *id1, unsigned  *id2)
{
    if (msm_proc_comm(POCM_RESERVED_101, id1, id2))
    {
        return -1;
    }
    else
    {
        return 0;
    }
}

static unsigned int vbatt_filter(unsigned int input_value)
{
    static unsigned int buf[BUF_SUM];
    static unsigned int index  = 0;
    static unsigned int fulled = 0;
    unsigned int total = 0;
    unsigned int i = 0;

    if (!fulled)
    {
        buf[index] = input_value;
        index++;
        if (index >= BUF_SUM)
        {
            fulled = 1;
        }

        return input_value;
    }
    else
    {
        index = index % BUF_SUM;
        buf[index] = input_value;
        index++;
    }

    for (i = 0; i < BUF_SUM; i++)
    {
        total += buf[i];
    }

    return total / BUF_SUM;
}


/*===========================================================================

FUNCTION  volt_find_capacity_level_value

DESCRIPTION
  Read current_volt and then find the corresponding capacity level value

DEPENDENCIES
  Read battery volt from arm9.

INPUT VALUE
  The current volt
RETURN VALUE
  The corresponding capacity level value

SIDE EFFECTS
  None

===========================================================================*/
static int volt_find_capacity_level_value(int current_volt)
{
    int i;

    if (current_volt <= volt_capacity_value[0].volt)
    {
        return volt_capacity_value[0].capacity_level;
    }

    if (current_volt >= volt_capacity_value[ARRAY_SIZE(volt_capacity_value) - 1].volt)
    {
        return volt_capacity_value[ARRAY_SIZE(volt_capacity_value) - 1].capacity_level;
    }

    for (i = 0; i < ARRAY_SIZE(volt_capacity_value); i++)
    {
        if (i >= ARRAY_SIZE(volt_capacity_value) - 1)
        {
            return volt_capacity_value[i].capacity_level;
        }

        if ((current_volt >= volt_capacity_value[i].volt) && (current_volt <= volt_capacity_value[i + 1].volt))
        {
            if (current_volt <= ((volt_capacity_value[i].volt + volt_capacity_value[i + 1].volt) / 2))
            {
                return volt_capacity_value[i].capacity_level;
            }
            else
            {
                return volt_capacity_value[i + 1].capacity_level;
            }
        }
    }

    return 0;
}


int huawei_battery_status_update(struct battery_info_reply curr_info)
{
    mutex_lock(&huawei_batt_info.lock);

    huawei_batt_info.rep.level = curr_info.level;
    huawei_batt_info.rep.present_in = curr_info.present_in;
    huawei_batt_info.present = curr_info.present_in;
    huawei_batt_info.rep.batt_vol = curr_info.batt_vol;
    huawei_batt_info.rep.charging_enabled = curr_info.charging_enabled;
    huawei_batt_info.rep.batt_temp = curr_info.batt_temp;
    huawei_batt_info.rep.charging_event = curr_info.charging_event;

    huawei_batt_info.rep.power_off_charging = curr_info.power_off_charging;

    mutex_unlock(&huawei_batt_info.lock);
    {
        power_supply_changed(&huawei_power_supplies[CHARGER_BATTERY]);
        power_supply_changed(&huawei_power_supplies[CHARGER_USB]);
	    power_supply_changed(&huawei_power_supplies[CHARGER_AC]);
    }
    return 0;
}

static void huawei_get_batt_info(unsigned long arg_pdev)
{
    struct platform_device *pdev = (struct platform_device *)arg_pdev;
    struct battery_info_reply info;
    struct battery_platform_data *bpdata = pdev->dev.platform_data;

    unsigned mV_id = 0;

    unsigned id1 = 0;
    unsigned id2 = 0;

    mod_timer(&bpdata->timer, jiffies + BATTERY_POLLING_JIFFIES);

    info.batt_vol = get_batt_mV(mV_id);
    if (info.batt_vol < 0)
    {
        printk(KERN_ERR "get batt mv level failed, info.batt_vol = %d\n", info.batt_vol);
        return;
    }


    get_batt_temp_and_source(&id1, &id2);
    info.charging_enabled = id1 & 0xff;
    info.batt_temp = id2 * 10;

    info.power_off_charging = (id1 & 0xff00) >> 8;

    info.present_in = (id1 & 0xff0000) >> 16;
    info.charging_event = (id1 & 0xff000000) >> 24;
    if (info.charging_enabled < 0)
    {
        printk(KERN_ERR "get whether batt is charging failed, info.charging_enabled = %d\n", info.charging_enabled);
        return;
    }

    info.batt_vol = vbatt_filter(info.batt_vol);

    if (CHARGER_USB == get_power_source(huawei_batt_info.rep.charging_event, huawei_batt_info.rep.present_in))
    {
        info.level = volt_find_capacity_level_value(info.batt_vol - USB_VIRTUAL_VOL);
    }
    else if (CHARGER_AC == get_power_source(huawei_batt_info.rep.charging_event, huawei_batt_info.rep.present_in))
    {
        info.level = volt_find_capacity_level_value(info.batt_vol - AC_VIRTUAL_VOL);
    }
    else
    {
        info.level = volt_find_capacity_level_value(info.batt_vol);
    }

    if ((info.power_off_charging == 1) && (info.charging_enabled == 1))
    {
        info.power_off_charging = 1;
    }
    else
    {
        info.power_off_charging = 0;
    }

    huawei_battery_status_update(info);
}

static ssize_t huawei_battery_show_property(struct device *          dev,
                                            struct device_attribute *attr,
                                            char *                   buf)
{
    int i = 0;
    const long off = attr - huawei_battery_attrs;

    mutex_lock(&huawei_batt_info.lock);
    switch (off)
    {
        case BATT_ID:
            i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n", huawei_batt_info.rep.batt_id);
            break;

        case POWER_OFF_CHARGING:
            i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n", huawei_batt_info.rep.power_off_charging);
            break;

        case BATT_LEVEL:
            i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n", huawei_batt_info.rep.level);
            break;
        case BATT_VOL:
            i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n", huawei_batt_info.rep.batt_vol);
            break;
        case BATT_TEMP:
            i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n", huawei_batt_info.rep.batt_temp);
            break;
        case BATT_CURRENT:
            i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n", huawei_batt_info.rep.batt_current);
            break;
        case CHARGING_SOURCE:
            i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n", huawei_batt_info.rep.charging_source);
            break;
        case CHARGING_ENABLED:
            i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n", huawei_batt_info.rep.charging_enabled);
            break;
        case FULL_BAT:
            i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n", huawei_batt_info.rep.full_bat);
            break;
        case BATT_PRESENT_IN:
            i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n", huawei_batt_info.rep.present_in);
            break;
        case CHARGING_EVENT:
            i += scnprintf(buf + i, PAGE_SIZE - i, "%d\n", huawei_batt_info.rep.charging_event);
            break;
        default:
            i = -EINVAL;
    }

    mutex_unlock(&huawei_batt_info.lock);
    return i;
}

static int huawei_battery_probe(struct platform_device *pdev)
{
    int i, rc = 0;
    struct battery_platform_data *bpdata = pdev->dev.platform_data;

    /* init power supplier framework */
    for (i = 0; i < ARRAY_SIZE(huawei_power_supplies); i++)
    {
        rc = power_supply_register(&pdev->dev, &huawei_power_supplies[i]);
        if (rc)
        {
            printk(KERN_ERR "Failed to register power supply (%d)\n", rc);
        }
    }

    /* create huawei detail attributes */
    huawei_battery_create_attrs(huawei_power_supplies[CHARGER_BATTERY].dev);

    init_timer(&bpdata->timer);

    bpdata->timer.expires = jiffies + BATTERY_POLLING_JIFFIES;

    bpdata->timer.data = (unsigned long)pdev;
    bpdata->timer.function = huawei_get_batt_info;
    add_timer(&bpdata->timer);
    return 0;
}

static struct platform_driver huawei_battery_driver =
{
    .probe     = huawei_battery_probe,
    .driver    = {
        .name  = "battery",
        .owner = THIS_MODULE,
    },
};

static int __devinit huawei_battery_init(void)
{
    mutex_init(&huawei_batt_info.lock);
    return platform_driver_register(&huawei_battery_driver);
}

module_init(huawei_battery_init);
MODULE_DESCRIPTION("Huawei Battery Driver");
MODULE_LICENSE("GPL");

