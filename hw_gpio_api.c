/* set tabstop=4 */
/********************************************************************************
 *                                                                              *
 * Copyright(C) 2004  Penta-Micro                                               *
 *                                                                              *
 * ALL RIGHT RESERVED                                                           *
 *                                                                              *
 * This software is the property of Penta-Micro and is furnished under          *
 * license by Penta-Micro. This software may be used only in accordance         *	
 * with the terms of said license. This copyright notice may not be             *
 * removed, modified or obliterated without the prior written permission        *
 * of Penta-Micro.                                                              *
 *                                                                              *
 * This software may not be copyed, transmitted, provided to or otherwise       *
 * made available to any other person, company, corporation	or other entity     *
 * except as specified in the terms of said license.                            *
 *                                                                              *
 * No right, title, ownership or other interest in the software is hereby       *
 * granted or transferred.                                                      *
 *                                                                              *
 * The information contained herein is subject to change without notice and     *
 * should not be construed as a commitment by Penta-Micro.                      *
 *                                                                              *
 ********************************************************************************
 
  MODULE NAME:  HW_GPIO_API.C
  
  REVISION HISTORY:
  
  Date       Ver Name                  Description
  ---------- --- --------------------- -----------------------------------------
  12/13/2004 1.0 jigwan Kang(xchannel)   Created   
 ...............................................................................
 
  DESCRIPTION:
  
  This Module contains functions for AU1500 GPIO.
  
 ...............................................................................
*/  
 

/** ************************************************************************* ** 
 ** includes
 ** ************************************************************************* **/
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <asm/ioctl.h>
#include <asm/errno.h>
//#include <asm/au1000_gpio.h>
#include <sys/ioctl.h>

#include "hw_gpio_api.h"


/** ************************************************************************* ** 
 ** defines
 ** ************************************************************************* **/
//#define m_DEBUG(format, args...)  printf(format, ## args) 
#define m_DEBUG(format, args...)  
#define m_MSG(format, args...)	printf(format, ## args) 
#define m_ERROR(format, args...)  printf(format, ## args)


/** ************************************************************************* ** 
 ** locals
 ** ************************************************************************* **/
static S32 fd_gpio = -1;


RETURN au1500_gpio_open() 
{
	if (fd_gpio >= 0)
		return PASS;

	fd_gpio = open( "/dev/au1000_gpio", O_RDWR);
	if(fd_gpio < 0)
	{
		fprintf(stderr, "/dev/au1000_gpio open error\n");
		return FAILURE;
	}

	return SUCCESS;
}

void au1500_gpio_close()
{
   	close(fd_gpio);
	fd_gpio = -1;
}

VIDEO_MODE get_video_mode()
{
	UNS32 temp_gpio_val;
	VIDEO_MODE gv_mode_val = NTSC;

	ioctl(fd_gpio, AU1000GPIO_IN, &temp_gpio_val);//read port
	gv_mode_val = (temp_gpio_val & 0x01) ? NTSC: PAL;	// GPIO0 = 1 for NTSC
	printf("\nCurrent Mode = %s\n", gv_mode_val ? "PAL": "NTSC");

	return gv_mode_val;	
}

RETURN set_live_audio_ch(UNS32 arg)
{
	UNS32 temp_gpio_val;

	switch(arg)
	{
	case 0:
		temp_gpio_val = (GPIO2_PIN_DIRECTION_SET << 16) |GPIO2_SPEECH_CH1;
		break;

	case 1:
		temp_gpio_val = (GPIO2_PIN_DIRECTION_SET << 16) |GPIO2_SPEECH_CH2;
		break;

	case 2:
		temp_gpio_val = (GPIO2_PIN_DIRECTION_SET << 16) | GPIO2_SPEECH_CH3;
		break;

	case 3:
		temp_gpio_val = (GPIO2_PIN_DIRECTION_SET << 16) | GPIO2_SPEECH_CH4;
		break;

	case 4:
		temp_gpio_val = (GPIO2_PIN_DIRECTION_SET << 16) | GPIO2_SPEECH_EN;
		break;

	default:
		temp_gpio_val = (GPIO2_PIN_DIRECTION_SET << 16) | GPIO2_SPEECH_CH1;
		break;
	}

#if 1
	ioctl(fd_gpio, AU1500GPIO2_SET, &temp_gpio_val);
#else
	au1500gpio2_output(temp_gpio_val);
#endif

	return SUCCESS;	
}

RETURN set_spot_out_ch(UNS32 arg)
{
	UNS32 temp_gpio_val = 0xC0;

	ioctl(fd_gpio, AU1000GPIO_CLEAR, &temp_gpio_val);
	temp_gpio_val = (UNS32)(arg << 6);
	ioctl(fd_gpio, AU1000GPIO_SET, &temp_gpio_val);
 
	return SUCCESS;	
}

RETURN set_sensor_out_ch(UNS32 arg, BOOL normal_open)
{
	UNS32 temp_gpio_val;

	temp_gpio_val = (UNS32)(GPIO1_SENSOR_OUT_PORT >> arg);
	if(normal_open)
		ioctl(fd_gpio, AU1000GPIO_SET,   &temp_gpio_val);
	else 
		ioctl(fd_gpio, AU1000GPIO_CLEAR, &temp_gpio_val);
    
	return SUCCESS;
}

RETURN au1500_gpio_in(UNS32 *arg)
{

#if 1
	ioctl(fd_gpio, AU1000GPIO_IN, arg);
#else
	au1000gpio_in(arg);
#endif

	return SUCCESS;
}

int au1500_gpio_ioctl(UNS32 cmd, UNS32 *arg)
{
	ioctl(fd_gpio, cmd, arg);
}


/* end of hw_gpio_api.c */

