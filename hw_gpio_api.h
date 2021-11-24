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
 
  MODULE NAME:  HW_GPIO_API.H
  
  REVISION HISTORY:
  
  Date       Ver Name                  Description
  ---------- --- --------------------- -----------------------------------------
 12/13/2004  1.0 jigwan Kang(xchannel)  Created 
 ...............................................................................
 
  DESCRIPTION:
  
  This Module contains definition for AU1500 GPIO function.
  
 ...............................................................................
*/    
 
#ifndef _HW_GPIO_API_H
#define _HW_GPIO_API_H


/** ************************************************************************* ** 
 ** includes
 ** ************************************************************************* **/
#include "typedef.h"
#include <asm/au1000_gpio.h>

 
/** ************************************************************************* ** 
 ** defines
 ** ************************************************************************* **/
/* GPIO */
#define SYS_PINFUNC					0xB190002C
#define SYS_PF_USB					(1<<15)	/* 2nd USB device/host */
#define SYS_PF_U3					(1<<14)	/* GPIO23/U3TXD */
#define SYS_PF_U2					(1<<13)	/* GPIO22/U2TXD */
#define SYS_PF_U1					(1<<12)	/* GPIO21/U1TXD */
#define SYS_PF_SRC					(1<<11)	/* GPIO6/SROMCKE */
#define SYS_PF_CK5					(1<<10)	/* GPIO3/CLK5 */
#define SYS_PF_CK4					(1<<9)	/* GPIO2/CLK4 */
#define SYS_PF_IRF					(1<<8)	/* GPIO15/IRFIRSEL */
#define SYS_PF_UR3					(1<<7)	/* GPIO[14:9]/UART3 */
#define SYS_PF_I2D					(1<<6)	/* GPIO8/I2SDI */
#define SYS_PF_I2S					(1<<5)	/* I2S/GPIO[29:31] */
#define SYS_PF_NI2					(1<<4)	/* NI2/GPIO[24:28] */
#define SYS_PF_U0					(1<<3)	/* U0TXD/GPIO20 */
#define SYS_PF_RD					(1<<2)	/* IRTXD/GPIO19 */
#define SYS_PF_A97					(1<<1)	/* AC97/SSL1 */
#define SYS_PF_S0					(1<<0)	/* SSI_0/GPIO[16:18] */
#define SYS_TRIOUTRD					0xB1900100
#define SYS_TRIOUTCLR				0xB1900100
#define SYS_OUTPUTRD				0xB1900108
#define SYS_OUTPUTSET				0xB1900108
#define SYS_OUTPUTCLR				0xB190010C
#define SYS_PINSTATERD				0xB1900110
#define SYS_PININPUTEN				0xB1900110

/* GPIO2, Au1500 only */
#define GPIO2_BASE					0xB1700000
#define GPIO2_DIR					(GPIO2_BASE + 0)
#define GPIO2_DATA_EN				(GPIO2_BASE + 8)
#define GPIO2_PIN_STATE				(GPIO2_BASE + 0xC)
#define GPIO2_INT_ENABLE				(GPIO2_BASE + 0x10)
#define GPIO2_ENABLE					(GPIO2_BASE + 0x14)


#define GPIO1_PIN_DIRECTION_SET		0x0FC0

/* gpio1,  set format : high word is bitmask, low word is value */
#define GPIO1_SENSOR_OUT_PORT		0x800      /* <-- 1bit access */

/* gpio2,  set format : high word is bitmask, low word is value */
#define GPIO2_PIN_DIRECTION_SET		0x7000

#ifdef OURBOARD
// AUDIO_SEL0~3, GIPO212~215
#define GPIO2_SPEECH_CH1			0x0000
#define GPIO2_SPEECH_CH2			0x4000
#define GPIO2_SPEECH_CH3			0x2000
#define GPIO2_SPEECH_CH4			0x6000
#define GPIO2_SPEECH_EN				0x1000

#define GPIO2_CH1_PLAYBACK			0x0400
#else
#define GPIO2_SPEECH_CH1			0x0000	// <-- 3bit access
#define GPIO2_SPEECH_CH2			0x4000	// gpio214, SPEECH_LOOP_SEL0
#define GPIO2_SPEECH_CH3			0x2000	// gpio213, SPEECH_LOOP_SEL1
#define GPIO2_SPEECH_CH4			0x6000	// gpio213,214
#define GPIO2_SPEECH_EN				0x1000	// gpio212, SPEECH_LOOP_ENB_n
#endif



/** ************************************************************************* ** 
 ** function prototypes
 ** ************************************************************************* **/
#ifdef __cplusplus
extern "C" {
#endif

RETURN au1500_gpio_open();
void au1500_gpio_close();
VIDEO_MODE get_video_mode();
RETURN set_live_audio_ch(UNS32 arg);
RETURN set_spot_out_ch(UNS32 arg);
RETURN set_sensor_out_ch(UNS32 arg, BOOL normal_open);
RETURN au1500_gpio_in(UNS32 *arg);
S32 au1500_gpio_ioctl(UNS32 cmd, UNS32 *arg);

#ifdef __cplusplus
}
#endif

#endif /* _HW_GPIO_API_H */

