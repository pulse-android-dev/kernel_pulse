/* include/asm/mach-msm/htc_pwrsink.h
 *
 * Copyright (C) 2008 HTC Corporation.
 * Copyright (C) 2007 Google, Inc.
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
#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/hrtimer.h>
#include <linux/timed_output.h>

#include <mach/msm_rpcrouter.h>

#define PM_LIBPROG		0x30000061
#define PM_LIBVERS		0x00010001 /* 0x00010001 */

#define HTC_PROCEDURE_SET_VIB_ON_OFF	22
#define PMIC_VIBRATOR_LEVEL	(3100)

struct pmic_vibrator_data {
	struct timed_output_dev dev;
	struct hrtimer timer;
	spinlock_t lock;
	struct work_struct work_vibrator_on;
	struct work_struct work_vibrator_off;
	
};

static void set_pmic_vibrator(int on)
{
	
	static struct msm_rpc_endpoint *vib_endpoint;
	struct set_vib_on_off_req {
		struct rpc_request_hdr hdr;
		uint32_t data;
	} req;
	
	if (!vib_endpoint) {
		vib_endpoint = msm_rpc_connect(PM_LIBPROG, PM_LIBVERS, 0);
		if (IS_ERR(vib_endpoint)) {
			vib_endpoint = 0;
			return;
		}
	}

	if (on)
		req.data = cpu_to_be32(PMIC_VIBRATOR_LEVEL);
	else
		req.data = cpu_to_be32(0);

	msm_rpc_call(vib_endpoint, HTC_PROCEDURE_SET_VIB_ON_OFF, &req,
		sizeof(req), 5 * HZ);
}

static void pmic_vibrator_on(struct work_struct *work)
{
	set_pmic_vibrator(1);
}

static void pmic_vibrator_off(struct work_struct *work)
{    
	set_pmic_vibrator(0);
}

static void timed_vibrator_on(struct timed_output_dev *dev)
{ 
	struct pmic_vibrator_data *data = 
		container_of(dev, struct pmic_vibrator_data, dev);
	schedule_work(&data->work_vibrator_on);
}

static void timed_vibrator_off(struct timed_output_dev *dev)
{
	struct pmic_vibrator_data *data =
		container_of(dev, struct pmic_vibrator_data, dev);
	schedule_work(&data->work_vibrator_off);
}

static void vibrator_enable(struct timed_output_dev *dev, int value)
{

	struct pmic_vibrator_data	*data =
		container_of(dev, struct pmic_vibrator_data, dev);
	unsigned long	flags;
	
	spin_lock_irqsave(&data->lock, flags);
	
	hrtimer_cancel(&data->timer);

	if (value == 0)
		
		timed_vibrator_off(dev);
	else {
		value = (value > 15000 ? 15000 : value);

		timed_vibrator_on(dev);

		hrtimer_start(&data->timer,
			      ktime_set(value / 1000, (value % 1000) * 1000000),
			      HRTIMER_MODE_REL);
	}
	spin_unlock_irqrestore(&data->lock, flags);
}

static int vibrator_get_time(struct timed_output_dev *dev)
{
	struct pmic_vibrator_data *data =
		container_of(dev, struct pmic_vibrator_data, dev);
	if (hrtimer_active(&data->timer)) {
		ktime_t r = hrtimer_get_remaining(&data->timer);
		return r.tv.sec * 1000 + r.tv.nsec / 1000000;
	} 
	else
		return 0;
}

static enum hrtimer_restart vibrator_timer_func(struct hrtimer *timer)
{
	struct pmic_vibrator_data	*data =
		container_of(timer, struct pmic_vibrator_data, timer);
	timed_vibrator_off(&data->dev);
	return HRTIMER_NORESTART;
}

static int msm_pmic_vibrator_probe(struct platform_device *pdev)
{
      int ret =0;
	struct pmic_vibrator_data *data;
	data = kzalloc(sizeof(struct pmic_vibrator_data) ,GFP_KERNEL);
	if (!data)
	return -ENOMEM;
	INIT_WORK(&data->work_vibrator_on, pmic_vibrator_on);
	INIT_WORK(&data->work_vibrator_off, pmic_vibrator_off);
	hrtimer_init(&data->timer, CLOCK_MONOTONIC,
		HRTIMER_MODE_REL);
	data->timer.function= vibrator_timer_func;
	spin_lock_init(&data->lock);

	data->dev.name = "vibrator";
	data->dev.get_time = vibrator_get_time;
	data->dev.enable = vibrator_enable;
	
	ret = timed_output_dev_register(&data->dev);
	if (ret < 0){
		timed_output_dev_unregister(&data->dev);
		kfree(data);
		return ret;
	}
	platform_set_drvdata(pdev, data);
	return 0;
}

static int msm_pmic_vibrator_remove(struct platform_device *pdev)
{
	struct pmic_vibrator_data *data = platform_get_drvdata(pdev);
	timed_output_dev_unregister(&data->dev);
	kfree(data);
	return 0;
}

static struct platform_device vibrator_device = {
	.name		= "time_vibrator",
	.id		= -1,
};

static struct platform_driver msm_pmic_vibrator = {
	.probe		= msm_pmic_vibrator_probe,
	.remove		= msm_pmic_vibrator_remove,
	.driver		= {
		.name		= "time_vibrator",
		.owner		= THIS_MODULE,
	},
};

int init_vibrator_device(void)
{
   return platform_device_register(&vibrator_device);
}

static int __init msm_init_pmic_vibrator(void)
{
	
	return platform_driver_register(&msm_pmic_vibrator);
}

static void __exit msm_exit_pmic_vibrator(void)
{
	platform_driver_unregister(&msm_pmic_vibrator);
}


module_init(msm_init_pmic_vibrator);
module_exit(msm_exit_pmic_vibrator);

MODULE_AUTHOR("dKF14049@huawei.com>");
MODULE_DESCRIPTION("timed output pmic vibrator device");
MODULE_LICENSE("GPL");

