#ifndef __DRIVERS_TOUCHSCREEN_PIXCIR_TS_H
#define __DRIVERS_TOUCHSCREEN_PIXCIR_TS_H

#include <mach/gpio.h>

static int attb_read_val(void);
static void pixcir_init(void);
static void pixcir_reset(void);

#define X_MAX 1024//800//13800//1024
#define Y_MAX 600//480//8000//768

#define MAX_SUPPORT_POINT 5

#define IOMUX_NAME_SIZE 48

//#define BUTTON   //if have button on TP

/*********************Platform gpio define************************/
//#define	S5PC1XX
//#define 	MINI6410
//#define 	MINI210
//#define		AMLOGIC


	#define ATTB			RK30_PIN4_PC2//RK29_PIN4_PD5
	#define get_attb_value	gpio_get_value
	#define	RESETPIN_CFG	//s3c_gpio_cfgpin(RK29_PIN4_PD5,S3C_GPIO_OUTPUT)
	//rk29_mux_api_set(PWM_MUX_NAME, PWM_MUX_MODE_GPIO);
	#define	RESETPIN_SET0 	gpio_direction_output(RK30_PIN4_PD0,0)//(RK29_PIN4_PD5,0)
	#define	RESETPIN_SET1	gpio_direction_output(RK30_PIN4_PD0,1)//(RK29_PIN4_PD5,1)


static int attb_read_val(void)
{
	return get_attb_value(ATTB);
}

static void pixcir_reset(void)
{
	RESETPIN_CFG;
	RESETPIN_SET0;
	mdelay(10);
	RESETPIN_SET1;
}

static void pixcir_init(void)
{
	pixcir_reset();
	mdelay(60);
#ifdef AMLOGIC
	/* set input mode */
    	gpio_direction_input(GPIO_PIXCIR_PENIRQ);
    	/* set gpio interrupt #0 source=GPIOD_24, and triggered by falling edge(=1) */
    	gpio_enable_edge_int(50+24, 1, 0);
#endif
	RESETPIN_SET1;//RESETPIN_SET0;
}

#endif

