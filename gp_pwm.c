/**************************************************************************
 *                                                                        *
 *         Copyright (c) 2010 by Generalplus Technology Co., Ltd.         *
 *                                                                        *
 *  This software is copyrighted by and is the property of Generalplus    *
 *  Technology Co., Ltd. All rights are reserved by Generalplus Technology*
 *  Co., Ltd. This software may only be used in accordance with the       *
 *  corresponding license agreement. Any unauthorized use, duplication,   *
 *  distribution, or disclosure of this software is expressly forbidden.  *
 *                                                                        *
 *  This Copyright notice MUST not be removed or modified without prior   *
 *  written consent of Generalplus Technology Co., Ltd.                   *
 *                                                                        *
 *  Generalplus Technology Co., Ltd. reserves the right to modify this    *
 *  software without notice.                                              *
 *                                                                        *
 *  Generalplus Technology Co., Ltd.                                      *
 *  19, Innovation First Road, Science-Based Industrial Park,             *
 *  Hsin-Chu, Taiwan, R.O.C.                                              *
 *                                                                        *
 **************************************************************************/

 /**
 * @file gp_pwm.c
 * @brief pwm driver interface
 * @author zaimingmeng
 */

#include <mach/kernel.h>
#include <mach/module.h>
#include <mach/cdev.h>
#include <mach/diag.h>

#include <mach/gp_timer.h>
#include <mach/gp_pwm.h>
#include <mach/gp_board.h>
#include <mach/hal/hal_timer.h>
#include <mach/hal/hal_gpio.h>
#include <mach/gp_gpio.h>
#include <mach/hal/hal_clock.h>
#include <mach/gp_pin_grp.h>
#include <mach/general.h>
/**************************************************************************
 *                          D A T A    T Y P E S                          *
 **************************************************************************/

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/
static LIST_HEAD(gp_pwm_list);
static uint32_t pwm_walkaround_set = 0;
static int g_pwm_suspendFlagSave[5] = {0};
static gp_pwm_config_t g_pwm_suspendConfigSave[5];

static int gp_pwm_walkaround ( gp_pwm_t *pwm ){
	struct gp_pwm_config_s pwm_config_ori;
	struct gp_pwm_config_s pwm_config_walkaround;

	//printk("gp_pwm_walkaround duty[%x]freq[%x]\n",
		//		pwm->config.duty, pwm->config.freq);
	//Save original setting
	pwm_config_ori.duty = pwm->config.duty;
	pwm_config_ori.freq = pwm->config.freq;

	/* FIXME : the duty of the first will be wroing. */
	pwm_config_walkaround.duty = (UINT32) 100 - pwm->config.duty;
	pwm_config_walkaround.freq = (UINT32) pwm->config.freq;

	gp_pwm_set_config((int)pwm, &pwm_config_walkaround);
	gp_pwm_disable((int)pwm);
	gp_pwm_enable((int)pwm);

	/* Work around : the duty of the first will be wroing. */
	mdelay(5);
	gp_pwm_set_config((int)pwm, &pwm_config_ori);
	gp_pwm_disable((int)pwm);
	gp_pwm_enable((int)pwm);

	return 0;
}

static int gp_pwm_walkaround_all ( int handle ){
	int i = 0;
	int pwm_handle = 0;
	struct gp_pwm_s *pwm = (struct gp_pwm_s *)handle;

	gp_pwm_walkaround(pwm);
	for(i=1; i<5; i++){
		if( pwm->id != i ){
			printk("*PWM init[%d]duty[%x]freq[%x]\n", i,
				g_pwm_suspendConfigSave[i].duty, g_pwm_suspendConfigSave[i].freq);
			//Todo: Get GID value. And change GID from function to GPIO.
			pwm_handle = (int)gp_pwm_request(i);
			if( g_pwm_suspendConfigSave[i].duty == 0){
				g_pwm_suspendConfigSave[i].duty = 50;
				g_pwm_suspendConfigSave[i].freq = 2000;
				printk("*PWM Default duty[%x]freq[%x]\n",
					g_pwm_suspendConfigSave[i].duty, g_pwm_suspendConfigSave[i].freq);
			}
			gp_pwm_set_config(pwm_handle, &g_pwm_suspendConfigSave[i]);
			gp_pwm_walkaround((gp_pwm_t *)pwm_handle);
			gp_pwm_disable((int)pwm_handle);
			gp_pwm_release(pwm_handle);
		}
	}
	return 0;
}


/**
 * @brief pwm pin index save function
 * @param pin_index [in] GPIO pin index
 * @return none
 */
void gp_pwm_config_save(int pwm_id, gp_pwm_config_t* config)
{
	printk("[%s][%d], [%x][%x][%x]\n", __FUNCTION__, __LINE__, config->duty, config->freq, config->pin_index);
	memcpy(&g_pwm_suspendConfigSave[pwm_id], config, sizeof(gp_pwm_config_t));
}
EXPORT_SYMBOL(gp_pwm_config_save);

/**
 * @brief pwm enable function
 * @param handle [in] pwm handle
 * @return success: 0,  erro: ERROR_ID
 */

int __tcmfunc gp_pwm_enable_suspend(int handle)
{
	struct gp_pwm_s *pwm = NULL;
	if(0 == handle){
		return -EINVAL;
	}

	pwm = (struct gp_pwm_s *)handle;
	gpHalTimerSetCtrl(pwm->id,0x40d); /* down counting */
	return 0;
}

int gp_pwm_enable(int handle)
{
	struct gp_pwm_s *pwm = NULL;
	if(0 == handle){
		return -EINVAL;
	}

	pwm = (struct gp_pwm_s *)handle;

	if( pwm_walkaround_set == 0 ){
		//printk("PWM init[%d]\n", pwm->id);
		pwm_walkaround_set = 1;
		gp_pwm_walkaround_all( handle );
	}
	else{
		gpHalTimerSetCtrl(pwm->id,0x40d); /* down counting */
	}

	return 0;
}
EXPORT_SYMBOL(gp_pwm_enable);

/**
 * @brief pwm disable function
 * @param handle [in] pwm handle
 * @return success: 0,  erro: ERROR_ID
 */
int gp_pwm_disable(int handle)
{
	struct gp_pwm_s *pwm = NULL;

	if(0 == handle){
		return -EINVAL;
	}

	pwm = (struct gp_pwm_s *)handle;

	gpHalTimerSetCtrl(pwm->id,0);

	return 0;
}
EXPORT_SYMBOL(gp_pwm_disable);

/**
 * @brief pwm config set function
 * @param handle [in] pwm handle
 * @param config [in] config struct value(freq and duty)
 * @return success: 0,  erro: ERROR_ID
 */
int __tcmfunc gp_pwm_set_config(int handle, struct gp_pwm_config_s *config)
{
	struct gp_pwm_s *pwm = NULL;

	if(0 == handle){
		return -EINVAL;
	}

	pwm = (struct gp_pwm_s *)handle;

	pwm->config.freq = config->freq;
	pwm->config.duty = config->duty;
	if(config->freq) {
		UINT32 apbHz;
		UINT32 temp;

		apbHz = gpHalTimerGetBaseClk();

		if(config->freq <= 500) {
			gpHalTimerPrescaleSet(pwm->id, 1000-1);
			temp = apbHz/(config->freq*1000)-1;
		}
		else {
			gpHalTimerPrescaleSet(pwm->id, 0);
			temp = apbHz/(config->freq)-1;
		}
		gpHalTimerLoadSet(pwm->id,temp);

		if((0 < config->duty)&&(config->duty < 100)) {
			temp = config->duty * temp / 100 - 1;
		}
		else if(0 == config->duty) {
			temp = 1;
		}
		else {
			temp--;
		}

		gpHalTimerCmpSet(pwm->id, temp);
	}
	return 0;
}
EXPORT_SYMBOL(gp_pwm_set_config);

/**
 * @brief pwm init function
 * @param handle [in] pwm handle
 * @return success: 0,  erro: ERROR_ID
 */
//Todo: Removed me.
static int gp_pwm_init(int handle)
{
	struct gp_pwm_s *pwm = NULL;

	printk("[%s][%d]\n", __FUNCTION__, __LINE__);
	if(0 == handle){
		return -EINVAL;
	}
	pwm = (struct gp_pwm_s *)handle;
	gpHalTimerInit(pwm->id);

	return 0;
}

/**
 * @brief pwm request function
 * @param pwm_id [in] pwm channel index
 * @return success: 0,  erro: ERROR_ID
 */
int __tcmfunc gp_pwm_request(int pwm_id)
{
	int found = 0;
	struct gp_pwm_s *pwm = NULL;

	list_for_each_entry(pwm, &gp_pwm_list, list){
		if(pwm->id == pwm_id){
			found = 1;
			break;
		}
	}

	if(found){
		return (int)pwm;
	}
	else{
		return 0;
	}
}
EXPORT_SYMBOL(gp_pwm_request);

/**
 *@brief pwm free function
 *@param handle [in] pwm handle
 * @return success: 0,  erro: ERROR_ID
 */
int __tcmfunc gp_pwm_release(int handle)
{
	return 0;
}
EXPORT_SYMBOL(gp_pwm_release);

/**
 * @brief   pwm driver open
 */
static int gp_pwm_fops_open(struct inode *inode, struct file *file)
{
	int ret = 0;
	int found = 0;
	int minor = iminor(inode);
	struct gp_pwm_s *pwm = NULL;

	list_for_each_entry(pwm, &gp_pwm_list, list){
		if(minor == pwm->dev.minor){
			found = 1;
			break;
		}
	}

	if(!found){
		file->private_data = NULL;
		ret = -EBUSY;
	}
	else{
		file->private_data = pwm;
	}

	return ret;
}

/**
 * @brief   pwm driver ioctl
 */
static long gp_pwm_fops_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	int pwm_handle = 0;
	
	struct gp_pwm_s *pwm = (struct gp_pwm_s *)file->private_data;

	if(NULL == pwm){
		return -EFAULT;
	}

	switch(cmd){
		case PWM_IOCTL_SET_ENABLE:
			if(1 == arg){
				gp_pwm_init((int)pwm);
				gp_pwm_set_config((int)pwm, &pwm->config);
				gp_pwm_enable((int)pwm);
			}
			else{
				gp_pwm_disable((int)pwm);
			}
			break;

		case PWM_IOCTL_SET_ATTRIBUTE:
			{
				gp_pwm_config_t config;

				if (copy_from_user((void*) &config, (const void __user *) arg, sizeof(gp_pwm_config_t))) {
					ret = -EIO;
					break;
				}
				pwm->config.freq = config.freq;
				pwm->config.duty = config.duty;
				pwm_handle = (int)gp_pwm_request(1);
				gp_pwm_set_config(pwm_handle, &config);
				gp_pwm_disable(pwm_handle);
				gp_pwm_enable(pwm_handle);
			}
			break;

		case PWM_IOCTL_GET_ATTRIBUTE:
			if (copy_to_user ((void __user *) arg, (const void *) &pwm->config, sizeof(gp_pwm_config_t))) {
				ret = -EIO;
			}
			break;

		default:
			ret = -ENOTTY;
	}

	return ret;
}

static int
gpiocfgOut(
	int pin_index,
	int level
)
{
	int handle;
	gpio_content_t ctx;

	ctx.pin_index = pin_index;
	handle = gp_gpio_request( ctx.pin_index, NULL );

	gp_gpio_set_function( handle, GPIO_FUNC_GPIO );
	gp_gpio_set_direction( handle, GPIO_DIR_OUTPUT );
	gp_gpio_set_output( handle, level, 0 );

	gp_gpio_release( handle );
	return	0;
}

static int
gpiocfgInput(
	int pin_index
)
{
	int handle;
	gpio_content_t ctx;

	ctx.pin_index = pin_index;
	handle = gp_gpio_request( ctx.pin_index, NULL );

	gp_gpio_set_function( handle, GPIO_FUNC_GPIO );
	gp_gpio_set_direction( handle, GPIO_DIR_INPUT );

	gp_gpio_release( handle );
	return	0;
}

struct file_operations gp_pwm_fops = {
	.owner          = THIS_MODULE,
	.open		 = gp_pwm_fops_open,
	.unlocked_ioctl = gp_pwm_fops_ioctl,
};

/**
 * @brief   pwm driver probe
 */
static int gp_pwm_probe(struct platform_device *pdev)
{
	int ret;
	char *name = NULL;
	struct gp_pwm_s *pwm = NULL;

	pwm = kzalloc(sizeof(gp_pwm_t), GFP_KERNEL);
	if(!pwm)
		return -ENOMEM;

	INIT_LIST_HEAD(&pwm->list);

	name = kzalloc(5, GFP_KERNEL);
	if(!name){
		ret = -ENOMEM;
		goto _err_alloc_;
	}

	memset(name, 0, 5);
	snprintf(name, 5, "pwm%d", pdev->id);

	pwm->dev.name  = name;
	pwm->dev.fops  = &gp_pwm_fops;
	pwm->dev.minor = MISC_DYNAMIC_MINOR;
	pwm->id = pdev->id;

	ret = misc_register(&pwm->dev);
	if(ret != 0){
		DIAG_ERROR(KERN_ALERT "pwm probe register fail\n");
		ret = -EINVAL;
		goto _err_timer_;
	}

	list_add(&pwm->list, &gp_pwm_list);

	platform_set_drvdata(pdev, pwm);

	//printk("PWM probe ok, %d\n", pwm->id);
	return 0;

_err_timer_:
	kfree(name);

_err_alloc_:
	kfree(pwm);

	return ret;
}

#ifdef CONFIG_PM

static int pwm_dram_suspend  = 0;
static int gp_pwm_suspend(struct platform_device *pdev, pm_message_t state){
	
	if(PM_EVENT_SUSPEND == state.event){
		pwm_dram_suspend = 1;
		return 0;
	}

	//If PWM mode
	if (gpHalTimerOutputModeGet(pdev->id) == 1){
		pwm_walkaround_set = 0;
		g_pwm_suspendFlagSave[pdev->id] = 1;
		//printk("[%s][%d]id[%d]\n", __FUNCTION__, __LINE__, pdev->id);
	}
	return 0;
}

static int gp_pwm_resume(struct platform_device *pdev){

	int i = 0;
	int pwm_current_handle = 0;

	if(1 == pwm_dram_suspend){
		pwm_dram_suspend = 0;
		return 0;
	}


	if ( g_pwm_suspendFlagSave[pdev->id] == 1 ){
		//printk("[%s][%d]id[%d]\n", __FUNCTION__, __LINE__, pdev->id);
		//Apply walk around for all PWM
		if( pwm_walkaround_set == 0 ){
			//printk("PWM init[%d]\n", pdev->id);
			pwm_current_handle = (int)gp_pwm_request(pdev->id);
			gp_pwm_set_config(pwm_current_handle, &g_pwm_suspendConfigSave[pdev->id]);
			//Turn off all GID
			for(i=1; i<5; i++){
				if( g_pwm_suspendFlagSave[pdev->id] == 1 ){
					//Set for Input
					gpiocfgInput(g_pwm_suspendConfigSave[i].pin_index);
					//Turn off GID
					gpHalGpioSetPadGrp(g_pwm_suspendConfigSave[i].pin_index & 0xff00ffff);
				}
			}
			pwm_walkaround_set = 1;
			gp_pwm_walkaround_all( pwm_current_handle );
			gp_pwm_release(pwm_current_handle);
			//Turn on all GID
			for(i=1; i<5; i++){
				if( g_pwm_suspendFlagSave[i] == 1 ){
					//Set output
					gpiocfgOut(g_pwm_suspendConfigSave[i].pin_index, 0);
					//Turn on GID
					gpHalGpioSetPadGrp(g_pwm_suspendConfigSave[i].pin_index);
				}
			}
			g_pwm_suspendFlagSave[pdev->id] = 0;
		}
		else{
			pwm_current_handle = (int)gp_pwm_request(pdev->id);
			gp_pwm_set_config(pwm_current_handle, &g_pwm_suspendConfigSave[pdev->id]);
			gp_pwm_enable(pwm_current_handle);
			gp_pwm_release(pwm_current_handle);
		}
	}
	return 0;
}
#else
#define gp_pwm_suspend NULL
#define gp_pwm_resume NULL
#endif

/**
 * @brief   pwm driver remove
 */
static int gp_pwm_remove(struct platform_device *pdev)
{
	int found = 0;
	struct gp_pwm_s *pwm = NULL;

	list_for_each_entry(pwm, &gp_pwm_list, list){
		if(pwm->id == pdev->id){
			found = 1;
			break;
		}
	}

	if(1 == found){
		misc_deregister(&pwm->dev);

		if(pwm->dev.name){
			kfree(pwm->dev.name);
		}
		list_del(&pwm->list);
		kfree(pwm);
	}
	return 0;
}

/**
 * @brief   pwm driver define
 */
static struct platform_driver gp_pwm_driver = {
	.probe  = gp_pwm_probe,
	.remove = gp_pwm_remove,
	.suspend = gp_pwm_suspend,
	.resume = gp_pwm_resume,
	.driver = {
		.name  = "gp-pwm",
		.owner = THIS_MODULE,
	}
};

/**
 * @brief   wdt device release
 */
static void gp_pwm_device_release(struct device *dev)
{
	DIAG_INFO("remove pwm device ok\n");
}
/*Timer 0 is system timer.*/
#if 0
static struct platform_device gp_pwm_device0 = {
	.name = "gp-pwm",
	.id = 0,
	.dev = {
		.release = gp_pwm_device_release,
	},
};
#endif

static struct platform_device gp_pwm_device1 = {
	.name = "gp-pwm",
	.id = 1,
	.dev = {
		.release = gp_pwm_device_release,
	},
};

static struct platform_device gp_pwm_device2 = {
	.name = "gp-pwm",
	.id = 2,
	.dev = {
		.release = gp_pwm_device_release,
	},
};

static struct platform_device gp_pwm_device3 = {
	.name = "gp-pwm",
	.id = 3,
	.dev = {
		.release = gp_pwm_device_release,
	},
};

static struct platform_device gp_pwm_device4 = {
	.name = "gp-pwm",
	.id = 4,
	.dev = {
		.release = gp_pwm_device_release,
	},
};

static struct platform_device *gp_pwm_devices[] = {
	//&gp_pwm_device0,
	&gp_pwm_device1,
	&gp_pwm_device2,
	&gp_pwm_device3,
	&gp_pwm_device4,
};

/**
 * @brief   pwm driver init
 */
static int __init gp_pwm_drv_init(void)
{
	platform_add_devices(gp_pwm_devices, ARRAY_SIZE(gp_pwm_devices));
	return platform_driver_register(&gp_pwm_driver);
}

/**
 * @brief   pwm driver exit
 */
static void __exit gp_pwm_drv_exit(void)
{
	int i;

	for(i = 0; i < ARRAY_SIZE(gp_pwm_devices); i++){
		platform_device_unregister(gp_pwm_devices[i]);
	}

	platform_driver_unregister(&gp_pwm_driver);
}

module_init(gp_pwm_drv_init);
module_exit(gp_pwm_drv_exit);

/**************************************************************************
 *                  M O D U L E    D E C L A R A T I O N                  *
 **************************************************************************/
MODULE_AUTHOR("Generalplus");
MODULE_DESCRIPTION("GP PWM Driver");
MODULE_LICENSE_GP;
