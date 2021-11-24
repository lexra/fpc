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
 
  MODULE NAME:  MAIN.C
  
  REVISION HISTORY:
  
  Date       Ver Name                  Description
  ---------- --- --------------------- -----------------------------------------
  06/28/2004 2.0 CheulBeck(whitefe)       Created   
 ...............................................................................
 
  DESCRIPTION:
  
  This Module contains MAIN function.
  
 ...............................................................................
*/  
 

/** ************************************************************************* ** 
 ** includes
 ** ************************************************************************* **/
#include <ctype.h>
#include <dirent.h>
#include <fcntl.h>
#include <pthread.h>
#include <stddef.h>      // offsetof()
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <unistd.h>

#include "main.h"

#include "hw_at2041_api.h"
#include "hw_at4012_api.h"
#include "hw_nvram_api.h"
#include "hw_tw9903_api.h"
#include "hw_gpio_api.h"


/** ************************************************************************* ** 
 ** defines
 ** ************************************************************************* **/
//#define m_DEBUG(format, args...)	printf(format, ## args) 
#define m_DEBUG(format, args...)  

//#define m_MSG(format, args...)		printf(format, ## args) 
#define m_MSG(format, args...)		

#define m_ERROR(format, args...)	printf(format, ## args) 


/** ************************************************************************* ** 
 ** globals
 ** ************************************************************************* **/
PTHREAD_STATE *gp_state_thread;
SETUP_PARAM *gp_setup_param;

  
S32 main(S32 argc, S8 **argv)
{
	BOOL quit_main = FALSE;
	UNS16 input_value = 0;
	PTHREAD_BUF signal;
	MAIN_LOC present_location = MAIN_LIVE;
	PTHREAD_ID prev_id;
	S32 ret;

	// 
	// 1st. open device driver
	//

	// at2041 device open
	at2041_open();

#ifndef AT2041_UPLOAD_FIRMWARE_TEST
	// at4012 device open
	at4012_open();

	// nvram device open
	if (nvram_open() == FAILURE)
		return 255;

	// tw9903 device open
	if (tw99_open() == -1)
		return 255;

	// au1500 gpio open
	if (au1500_gpio_open() == FAILURE)
		return 255;
#endif

	// at2041 boot image loading
	sleep(1);
	at2041_load_image();

#ifndef AT2041_UPLOAD_FIRMWARE_TEST
	//
	//2nd. allocate thread state structure object
	//
	gp_state_thread = malloc(sizeof(PTHREAD_STATE));
	if (gp_state_thread == NULL)
	{
		fprintf(stderr, "main.c:error In function 'malloc'\n");
		return FAILURE;
	}
	memset(gp_state_thread, 0, sizeof(PTHREAD_STATE));
	gp_state_thread->state_main.state = IDLE;
	gp_state_thread->state_enc.state = IDLE;
	gp_state_thread->state_dec.state = IDLE;

	// check NTSC or PAL
	gp_state_thread->state_main.mode = get_video_mode();

	// search db semaphore init : value 1
	ret = sem_init(&gp_state_thread->state_db.search_sem, 0, 1);

	//
	// 3rd. allocate setup parameter structure object
	//
	gp_setup_param = (SETUP_PARAM *)malloc(sizeof(SETUP_PARAM));
	if (gp_setup_param == NULL)
	{
		fprintf(stderr, "main.c:error In function 'malloc'\n");
		return FAILURE;
	}
	memset(gp_setup_param, 0, sizeof(SETUP_PARAM));

	//
	// 4th. check magic number and set default parameter value to NVRAM
	//
	if (nvram_check_magic_num(gp_setup_param) == FAILURE)
	{
		fprintf(stderr, "main.c:error In function 'nvram_check_magic_num'\n");
		return FAILURE;
	}

#ifdef NVRAM_TEST
	//
	// 5th. load setup parameter value from NVRAM
	//
	if (nvram_load_setup_value(gp_setup_param) == FAILURE)
	{
		fprintf(stderr, "main.c:error In function 'nvram_load_setup_value'\n");
		return FAILURE;
	}
#endif

	//
	// 6th. create manager thread : able to check thread state
	//
	if (pthread_create_manager() == FALSE)
	{
		fprintf(stderr, "main.c:error: In function 'pthread_create_input'\n");
	}

	//
	// 7th. create input thread : able to receive input
	//
	if (pthread_create_input() == FALSE)
	{
		fprintf(stderr, "main.c:error: In function 'pthread_create_input'\n");
	}

	//
	// 8th. create live thread : able to monitor live
	//
	if (pthread_create_live() == FALSE)
	{
		fprintf(stderr, "main.c:error: In function 'pthread_create_live'\n");
	}

	//
	// 9th. create diskm thread : able to manage disk
	//
	if (pthread_create_diskm() == FALSE)
	{
		fprintf(stderr, "main.c:error: In function 'pthread_create_diskm'\n");
	}

	//
	// 10th. create schedule thread : able to record timely
	//
	if (pthread_create_schedule() == FALSE)
	{
		fprintf(stderr, "main.c:error: In function 'pthread_create_schedule'\n");
	}

#ifdef __TRANS_THR__
	//
	// 11th. create trans thread : able to manage disk
	//
	if (pthread_create_trans() == FALSE)
	{
		fprintf(stderr, "main.c:error: In function 'pthread_create_trans'\n");
	}
#endif

	//
	// 12th. create gpio thread : able to gpio read/write
	//
	if (pthread_create_gpio() == FALSE)
	{
		fprintf(stderr, "main.c:error: In function 'pthread_create_gpio'\n");
	}

	// Main thread while loop
	while (!quit_main)
	{
		// waiting signal from input thread
		pthread_read_signal(&signal, PTHREAD_MAIN, TRUE);
		m_MSG("\tsignal>[main][id:%02d][value:0x%04x]\n", signal.start_id, signal.m_value);

		// check start id : pass only input thread
		if (signal.start_id != PTHREAD_INPUT)
		{
			fprintf(stderr, "main.c:error: Received Invalid Signal ID\n");
		}

		// quit main program
		if (signal.m_value == POWER)
		{
			quit_main = TRUE;
			m_DEBUG(" !!! PRESS POWER OFF BUTTON !!! \n"); fflush(stdout);
			break;
		}

		// receive input value
		// check state : decision for input path
		input_value = signal.m_value;
		if (gp_state_thread->state_main.state == IDLE)
		{
			switch (input_value)
			{
			case SETUP:
				gp_state_thread->state_main.state = BUSY;
				if (pthread_create_setup() == FALSE)
				{
					fprintf(stderr, "main.c:error: In function 'pthread_create_setup'\n");
				}
				present_location = MAIN_SETUP;
				break;

			case REC:
				// display REC monitoring
				prev_id = signal.start_id;
				signal.start_id = PTHREAD_MAIN;
				if (pthread_send_signal(&signal, PTHREAD_MANAGER) == FALSE)	// rec manipulated by manager thread
				{
					fprintf(stderr, "main.c:error: In function 'pthread_send_signal'\n");
				}
				signal.start_id = prev_id;
				present_location = MAIN_IDLE;
				break;

			case SEARCH:
				gp_state_thread->state_main.state = BUSY;
				if (pthread_create_search() == FALSE)
				{
					fprintf(stderr, "main.c:error: In function 'pthread_create_search'\n");
				}
				present_location = MAIN_SEARCH;
				break;

		    case PTZ_CONTROL:	// Pan/Tilt/Zoom Control
				gp_state_thread->state_main.state = BUSY;
				if (pthread_create_ptz() == FALSE)
				{
					fprintf(stderr, "main.c:error: In function pthread_create_ptz'\n");
				}
				present_location = MAIN_PTZ;
				break;

			default :
				present_location = MAIN_LIVE;
				break;
			}
		}
		else if (gp_state_thread->state_main.state == BUSY)
		{
			switch(present_location)
			{
			case MAIN_SETUP:
				if (pthread_send_signal(&signal, PTHREAD_SETUP) == FALSE)	// resend signal
					fprintf(stderr, "main.c:error: In function 'pthread_send_signal'\n");
				break;

			case MAIN_SEARCH:
				if (pthread_send_signal(&signal, PTHREAD_SEARCH) == FALSE)
					fprintf(stderr, "main.c:error: In function 'pthread_send_signal'\n");
				break;

			case MAIN_PTZ:	// Pan/Tilt/Zoom Control
				if (pthread_send_signal(&signal, PTHREAD_PTZ) == FALSE)
					fprintf(stderr, "main.c:error: In function 'pthread_send_signal'\n");
				break;

			default:
				break;
			}
		}

		// proceed last input
		if (present_location == MAIN_LIVE || gp_state_thread->state_dec.state != IDLE)
		{
			signal.start_id = PTHREAD_INPUT;// why not PTHREAD_MAIN ??
			if (pthread_send_signal(&signal, PTHREAD_LIVE) == FALSE)
			{
				fprintf(stderr, "main.c:error: In function 'pthread_send_signal'\n");
			}
		}
 	}
	// end of while (!QUIT_SDVR_MAIN)

	if (gp_state_thread->state_enc.state == BUSY)
	{
		gp_state_thread->state_enc.state = IDLE;
		usleep(500000);
	}
	m_DEBUG("\n");
	m_DEBUG("Closing device...");

	// hw reset at2041
	at2041_reset();
	sleep(1);

	// destroy semaphore
	sem_destroy(&gp_state_thread->state_db.search_sem);

	// free malloc buffer
	free(gp_state_thread);
	free(gp_setup_param);
	m_DEBUG("Ok           \n\n");

	// close devices
	au1500_gpio_close();
	tw99_close();
	nvram_close();
	at4012_close();
#endif

	at2041_close();
	return 0;
}

#ifdef NVRAM_TEST
RETURN nvram_check_magic_num(SETUP_PARAM *psp)
{
	int i, j, k;
	UNS32 value;

	value = get_nvram_para(
		offsetof(SETUP_PARAM, magic_num), sizeof(psp->magic_num));
	if (value == MPEG4_MAGICNUM)
		return SUCCESS;

	// write magic number to NVRAM
	value = MPEG4_MAGICNUM;
	set_nvram_para(
		offsetof(SETUP_PARAM, magic_num), sizeof(psp->magic_num), &value);

	value = MPEG4_VERNUM;
	set_nvram_para(
		offsetof(SETUP_PARAM, version_num), sizeof(psp->version_num), &value);

	//
	// GLOBAL PARAMETER
	//

	// format : '0' NTSC, '1' PAL, default is '0'
	if (gp_state_thread->state_main.mode == PAL)
	{
		value = PAL;
	}
	else
	{
		value = NTSC;
	}
	set_nvram_para(
		offsetof(SETUP_PARAM, gp.video_form), sizeof(psp->gp.video_form), &value);

	// resolution : '0' 720 x 480(576), '1' 720 x 240(288), '2' 360 x 240(288)
	value = 2;
	set_nvram_para(
		offsetof(SETUP_PARAM, gp.resolution), sizeof(psp->gp.resolution), &value);

	//
	// ENCODER PARAMETER 
	//
	for (i = 0; i < MAX_CH_NUM; i++)
	{
		// CONTINUE RECORD FLAG : '0' OFF, '1' ON, default is '1'
		value = 1;
		set_nvram_para(offsetof(SETUP_PARAM, enc_ch[i].flag_conti_rec), 
			sizeof(psp->enc_ch[i].flag_conti_rec), &value);

		// frame rate : 0 ~ 30
		if (gp_state_thread->state_main.mode == PAL)
		{
			value = 25;
		}
		else
		{
			value = 30;
		}
		set_nvram_para(offsetof(SETUP_PARAM, enc_ch[i].frame_rate), 
			sizeof(psp->enc_ch[i].frame_rate), &value);

		// bit rate : '0' VBR, '1' CBR(Picture level), '2' CBR(MB level)
		value = 0;
		set_nvram_para(offsetof(SETUP_PARAM, enc_ch[i].bit_rate), 
			sizeof(psp->enc_ch[i].bit_rate), &value);

		// VBR Q value : 1 ~ 31
		value = 5;
		set_nvram_para(offsetof(SETUP_PARAM, enc_ch[i].vbr_q), 
			sizeof(psp->enc_ch[i].vbr_q), &value);

		// CBR Q value : 0 ~ 1000
#ifdef CBR_MODE
		value = 5;
#else
		value = 100;
#endif
		set_nvram_para(offsetof(SETUP_PARAM, enc_ch[i].cbr_q), 
			sizeof(psp->enc_ch[i].cbr_q), &value);

		// GOP_NM : N/M > 0, default is 5
		value = 8;
		set_nvram_para(offsetof(SETUP_PARAM, enc_ch[i].gop_nm), 
			sizeof(psp->enc_ch[i].gop_nm), &value);

		// GOP_M : M > 0, default is 3
		value = 1;
		set_nvram_para(offsetof(SETUP_PARAM, enc_ch[i].gop_m), 
			sizeof(psp->enc_ch[i].gop_m), &value);

		// MOTION FLAG : '0' OFF, '1' ON, default is '0'
		value = 0;
		set_nvram_para(offsetof(SETUP_PARAM, enc_ch[i].motion.flag), 
			sizeof(psp->enc_ch[i].motion.flag), &value);

		// MOTION SENSITIVITY : 0 ~ 4
		value = 2;
		set_nvram_para(offsetof(SETUP_PARAM, enc_ch[i].motion.sensitivity), 
			sizeof(psp->enc_ch[i].motion.sensitivity), &value);

		// MOTION AREA
		value = 0;
		for (j = 0; j < MOTION_LINE_NUM; j++)
		{
			set_nvram_para(offsetof(SETUP_PARAM, enc_ch[i].motion.area[j]),
				sizeof(psp->enc_ch[i].motion.area[j]), &value);
		}

		// MOTION AREA DISPLAY
		value = 0;
		for (j = 0; j < MOB_X_MAX; j++)
		{
			for (k = 0; k < MOB_Y_MAX; k++)
			{
				set_nvram_para(
					offsetof(SETUP_PARAM, enc_ch[i].motion.area_disp[j][k]), 
						sizeof(psp->enc_ch[i].motion.area_disp[j][k]), &value);
			}
		}

		// SENSOR IN FLAG : '0' OFF, '1' ON, default is '0'
		value = 0;
		set_nvram_para(offsetof(SETUP_PARAM, enc_ch[i].sensor.flag_in), 
			sizeof(psp->enc_ch[i].sensor.flag_in), &value);

		// SENSOR IN NORMAL OPEN/CLOSE : '0' OPEN, '1' CLOSE, default is '1'
		value = 1;
		set_nvram_para(offsetof(SETUP_PARAM, enc_ch[i].sensor.flag_in_noc), 
			sizeof(psp->enc_ch[i].sensor.flag_in_noc), &value);

		// SENSOR OUT FLAG : '0' OFF, '1' ON, default is '0'
		value = 0;
		set_nvram_para(offsetof(SETUP_PARAM, enc_ch[i].sensor.flag_out), 
			sizeof(psp->enc_ch[i].sensor.flag_out), &value);

		// SENSOR OUT NORMAL OPEN/CLOSE : '0' OPEN, '1' CLOSE, default is '1'
		value = 1;
		set_nvram_para(offsetof(SETUP_PARAM, enc_ch[i].sensor.flag_out_noc), 
			sizeof(psp->enc_ch[i].sensor.flag_out_noc), &value);

		// SENSOR OUT DURATION : default is 5 sec
		value = 5;
		set_nvram_para(offsetof(SETUP_PARAM, enc_ch[i].sensor.flag_out_duration), 
			sizeof(psp->enc_ch[i].sensor.flag_out_duration), &value);

		// SCHEDULE FLAG : '0' OFF, '1' ON, default is '0'
		value = 0;
		set_nvram_para(offsetof(SETUP_PARAM, enc_ch[i].schedule.flag), 
			sizeof(psp->enc_ch[i].schedule.flag), &value);

		// SCHEDULE TIME : from->2004-01-01 00:00, to->2004-01-01 00:00
		for (j = 0; j < MAX_DAY_OF_WEEK; j++)
		{
			value = 2004 -1900;// year
			set_nvram_para(
				offsetof(SETUP_PARAM, enc_ch[i].schedule.from_time[j].tm_year), 
					sizeof(psp->enc_ch[i].schedule.from_time[j].tm_year), &value);

			value = 1 - 1;	// month
			set_nvram_para(
				offsetof(SETUP_PARAM, enc_ch[i].schedule.from_time[j].tm_mon), 
					sizeof(psp->enc_ch[i].schedule.from_time[j].tm_mon), &value);

			value = 1;// day
			set_nvram_para(
				offsetof(SETUP_PARAM, enc_ch[i].schedule.from_time[j].tm_mday),
					sizeof(psp->enc_ch[i].schedule.from_time[j].tm_mday), &value);

			value = 0;// hour
			set_nvram_para(
				offsetof(SETUP_PARAM, enc_ch[i].schedule.from_time[j].tm_hour),
					sizeof(psp->enc_ch[i].schedule.from_time[j].tm_hour), &value);

			value = 0;// minute
			set_nvram_para(
				offsetof(SETUP_PARAM, enc_ch[i].schedule.from_time[j].tm_min),
					sizeof(psp->enc_ch[i].schedule.from_time[j].tm_min), &value);

			value = 0;// second
			set_nvram_para(
				offsetof(SETUP_PARAM, enc_ch[i].schedule.from_time[j].tm_sec),
					sizeof(psp->enc_ch[i].schedule.from_time[j].tm_sec), &value);

			value = 2004 -1900;// year
			set_nvram_para(
				offsetof(SETUP_PARAM, enc_ch[i].schedule.to_time[j].tm_year),
					sizeof(psp->enc_ch[i].schedule.to_time[j].tm_year), &value);

			value = 1 - 1;	// month
			set_nvram_para(
				offsetof(SETUP_PARAM, enc_ch[i].schedule.to_time[j].tm_mon),
					sizeof(psp->enc_ch[i].schedule.to_time[j].tm_mon), &value);

			value = 1;// day
			set_nvram_para(
				offsetof(SETUP_PARAM, enc_ch[i].schedule.to_time[j].tm_mday), 
					sizeof(psp->enc_ch[i].schedule.to_time[j].tm_mday), &value);

			value = 0;// hour
			set_nvram_para(
				offsetof(SETUP_PARAM, enc_ch[i].schedule.to_time[j].tm_hour),
					sizeof(psp->enc_ch[i].schedule.to_time[j].tm_hour), &value);

			value = 0;// minute
			set_nvram_para(
				offsetof(SETUP_PARAM, enc_ch[i].schedule.to_time[j].tm_min),
					sizeof(psp->enc_ch[i].schedule.to_time[j].tm_min), &value);

			value = 0;// second
			set_nvram_para(
				offsetof(SETUP_PARAM, enc_ch[i].schedule.to_time[j].tm_sec), 
					sizeof(psp->enc_ch[i].schedule.to_time[j].tm_sec), &value);
		}
	}

	//
	// DECODER PARAMETER
	//
	// video channel select : '0' CH1, '1' CH2, '2' CH3, '3' CH4, '4' Multi CH
	value = 4;
	set_nvram_para(offsetof(SETUP_PARAM, dec.video_chan), 
		sizeof(psp->dec.video_chan), &value);

	// interlace : '0' interlace, '1' non-interlace
	value = 0;
	set_nvram_para(offsetof(SETUP_PARAM, dec.de_interlace), 
		sizeof(psp->dec.de_interlace), &value);

	// play_conti : '0' single file play, '1' continuous file play
	value = 1;
	set_nvram_para(offsetof(SETUP_PARAM, dec.play_conti), 
		sizeof(psp->dec.play_conti), &value);

	// audio channel select : '0' CH1, '1' CH2, '2' CH3, '3' CH4
	value = 0;
	set_nvram_para(offsetof(SETUP_PARAM, dec.audio_chan), 
		sizeof(psp->dec.audio_chan), &value);

	//
	// SYSTEM PARAMETER
	//
	// network ip configuration : default 192.168.1.100
	value = (100 & 0xff) << 24 | (1 & 0xff) << 16 | (168 & 0xff) << 8 | (192 & 0xff);
	set_nvram_para(offsetof(SETUP_PARAM, sys.network.ipaddr), 
		sizeof(psp->sys.network.ipaddr), &value);

	// network netmask configuration : default 255.255.255.0
	value = (0 & 0xff) << 24 | (255 & 0xff) << 16 | (255 & 0xff) << 8 | (255 & 0xff);
	set_nvram_para(offsetof(SETUP_PARAM, sys.network.netmask),
		sizeof(psp->sys.network.netmask), &value);

	// network gateway configuration : default 192.168.1.1
	value = (1 & 0xff) << 24 | (1 & 0xff) << 16 | (168 & 0xff) << 8 | (192 & 0xff);
	set_nvram_para(offsetof(SETUP_PARAM, sys.network.gateway),
		sizeof(psp->sys.network.gateway), &value);

	// password configuration : default 000000
	for (i = 0; i < USER_NUM; i++)
	{
		value = i;
		set_nvram_para(offsetof(SETUP_PARAM, sys.password[i].id),
			sizeof(psp->sys.password[i].id), &value);

		value = 0;
		set_nvram_para(offsetof(SETUP_PARAM, sys.password[i].value),
			sizeof(psp->sys.password[i].value), &value);
	}

	// live audio configuration : default live audio=ch1
	value = 0;
	set_nvram_para(offsetof(SETUP_PARAM, sys.live_audio_param.live_audio_ch), 
		sizeof(psp->sys.live_audio_param.live_audio_ch), &value);

	// camera configuration : default brightness=0, contrast=96, color=0
	for(i = 0; i < MAX_CH_NUM; i++)
	{
		value = 0;
		set_nvram_para(offsetof(SETUP_PARAM, sys.camera[i].brightness), 
			sizeof(psp->sys.camera[i].brightness), &value);

		value = 96;
		set_nvram_para(offsetof(SETUP_PARAM, sys.camera[i].contrast),
			sizeof(psp->sys.camera[i].contrast), &value);

		value = 0;
		set_nvram_para(offsetof(SETUP_PARAM, sys.camera[i].color),
			sizeof(psp->sys.camera[i].color), &value);
	}

	// ptz configuration : default cam_vendor=scc-641, cam_speed=3, cam_ch=1
	value = 4;
	set_nvram_para(offsetof(SETUP_PARAM, sys.ptz.cam_vendor), 
		sizeof(psp->sys.ptz.cam_vendor), &value);

	value = 2;
	set_nvram_para(offsetof(SETUP_PARAM, sys.ptz.cam_speed),
		sizeof(psp->sys.ptz.cam_speed), &value);

	value = 0;
	set_nvram_para(offsetof(SETUP_PARAM, sys.ptz.cam_ch),
		sizeof(psp->sys.ptz.cam_ch), &value);

	return SUCCESS;
}
#else
RETURN nvram_check_magic_num(SETUP_PARAM *psp)
{
	unsigned int i, j, k;
	UNS32 value;

	//
	// GLOBAL PARAMETER
	//

	// format : '0' NTSC, '1' PAL, default is '0'
	if (gp_state_thread->state_main.mode == PAL)
	{		
		psp->gp.video_form = PAL;
	}
	else
	{
		psp->gp.video_form = NTSC;
	}

	// resolution : '0' 720 x 480(576), '1' 720 x 240(288), '2' 360 x 240(288)
	psp->gp.resolution = 2;

	// 
	// ENCODER PARAMETER 
	//
	for (i = 0; i < MAX_CH_NUM; i++)
	{
		// CONTINUE RECORD FLAG : '0' OFF, '1' ON, default is '1'
		psp->enc_ch[i].flag_conti_rec = 1;

		// frame rate : 0 ~ 30
		if (gp_state_thread->state_main.mode == PAL)
		{		
			psp->enc_ch[i].frame_rate = 25;
		}
		else
		{	
			psp->enc_ch[i].frame_rate = 30;
		}

		// bit rate : '0' VBR, '1' CBR(Picture level), '2' CBR(MB level)
		psp->enc_ch[i].bit_rate = 0;

		// VBR Q value : 1 ~ 31
		psp->enc_ch[i].vbr_q = 5;

		// CBR Q value : 0 ~ 1000
#ifdef CBR_MODE
		psp->enc_ch[i].cbr_q = 5;
#else
		psp->enc_ch[i].cbr_q = 100;
#endif

		// GOP_NM : N/M > 0, default is 5
		psp->enc_ch[i].gop_nm = 8;

		// GOP_M : M > 0, default is 3
		psp->enc_ch[i].gop_m = 1;

		// MOTION FLAG : '0' OFF, '1' ON, default is '0'
		psp->enc_ch[i].motion.flag = 0;

		// MOTION SENSITIVITY : 0 ~ 4
		psp->enc_ch[i].motion.sensitivity = 2;

		// MOTION AREA
		for (j = 0; j < MOTION_LINE_NUM; j++)
		{
			psp->enc_ch[i].motion.area[j] = 0;
		}	

		// MOTION AREA DISPLAY
		for (j = 0; j < MOB_X_MAX; j++)
		{
			for (k = 0; k < MOB_Y_MAX; k++)
				psp->enc_ch[i].motion.area_disp[j][k] = 0;
		}

		// SENSOR IN FLAG : '0' OFF, '1' ON, default is '0'
		psp->enc_ch[i].sensor.flag_in = 0;
		psp->enc_ch[i].sensor.flag_in_noc = 1; // default is normal close

		// SENSOR OUT FLAG : '0' OFF, '1' ON, default is '0'
		psp->enc_ch[i].sensor.flag_out = 0;
		psp->enc_ch[i].sensor.flag_out_noc = 1;// default is normal close
		psp->enc_ch[i].sensor.flag_out_duration = 5;

		// SCHEDULE FLAG : '0' OFF, '1' ON, default is '0'
		psp->enc_ch[i].schedule.flag = 0;

		// SCHEDULE TIME : from->2004-01-01 00:00, to->2004-01-01 00:00
		for (j = 0; j < MAX_DAY_OF_WEEK; j++)
		{
			psp->enc_ch[i].schedule.from_time[j].tm_year = 2004 -1900;	// year
			psp->enc_ch[i].schedule.from_time[j].tm_mon = 1 - 1;// month
			psp->enc_ch[i].schedule.from_time[j].tm_mday = 1;// day
			psp->enc_ch[i].schedule.from_time[j].tm_hour = 0;// hour
			psp->enc_ch[i].schedule.from_time[j].tm_min = 0;// minute
			psp->enc_ch[i].schedule.from_time[j].tm_sec = 0;// second
			psp->enc_ch[i].schedule.to_time[j].tm_year = 2004 -1900;	// year
			psp->enc_ch[i].schedule.to_time[j].tm_mon = 1 - 1;// month
			psp->enc_ch[i].schedule.to_time[j].tm_mday = 1;// day
			psp->enc_ch[i].schedule.to_time[j].tm_hour = 0;// hour
			psp->enc_ch[i].schedule.to_time[j].tm_min = 0;// minute
			psp->enc_ch[i].schedule.to_time[j].tm_sec = 0;	// second
		}
   	}

	// 
	// DECODER PARAMETER
	//

	// video channel select : '0' CH1, '1' CH2, '2' CH3, '3' CH4, '4' Multi CH
	psp->dec.video_chan = 4;

	// de_interlace : '0' interlace, '1' non-interlace
	psp->dec.de_interlace = 0;

	// play_conti : '0' single file play, '1' continuous file play
	psp->dec.play_conti = 1;

	// audio channel select : '0' CH1, '1' CH2, '2' CH3, '3' CH4
	psp->dec.audio_chan = 0;

	//
	// SYSTEM PARAMETER 
	//	

	// network ip configuration : default 192.168.1.100
	psp->sys.network.ipaddr = (100 & 0xff) << 24 | 
		(1 & 0xff) << 16 | (168 & 0xff) << 8 | (192 & 0xff);

	// network netmask configuration : default 255.255.255.0
	psp->sys.network.netmask = (0 & 0xff) << 24 | 
		(255 & 0xff) << 16 | (255 & 0xff) << 8 | (255 & 0xff); 

	// network gateway configuration : default 192.168.1.1
	psp->sys.network.gateway = (1 & 0xff) << 24 | 
		(1 & 0xff) << 16 | (168 & 0xff) << 8 | (192 & 0xff);

	// password configuration : default 000000
	for (i = 0; i < USER_NUM; i++)
	{
		psp->sys.password[i].id = i;
		psp->sys.password[i].value = 0x000000;
	}

	// live audio configuration : default live audio=ch1
	psp->sys.live_audio_param.live_audio_ch = 0;

	// camera configuration : default brightness=0, contrast=96, color=0
	for(i = 0; i < MAX_CH_NUM; i++)
	{
		psp->sys.camera[i].brightness = 0;
		psp->sys.camera[i].contrast = 96;
		psp->sys.camera[i].color = 0;
	}

	// ptz configuration : default cam_vendor=scc-641, cam_speed=3, cam_ch=1
	psp->sys.ptz.cam_vendor = 4;
	psp->sys.ptz.cam_speed = 2;
	psp->sys.ptz.cam_ch = 0;

	// write magic number to NVRAM
	return SUCCESS;
}
#endif

RETURN nvram_load_setup_value(SETUP_PARAM *psp)
{
	int i, j, k;
	UNS32 value;

	psp->version_num = get_nvram_para(
		offsetof(SETUP_PARAM, version_num), sizeof(psp->version_num));

	//
	// GLOBAL PARAMETER
	//

	// format : '0' NTSC, '1' PAL, default is '0'
	psp->gp.video_form = get_nvram_para(
		offsetof(SETUP_PARAM, gp.video_form), sizeof(psp->gp.video_form));

	psp->gp.video_form = gp_state_thread->state_main.mode;

	// resolution : '0' 720 x 480(576), '1' 720 x 240(288), '2' 360 x 240(288)
	psp->gp.resolution = get_nvram_para(
		offsetof(SETUP_PARAM, gp.resolution), sizeof(psp->gp.resolution));


	//
	// ENCODER PARAMETER
	//
	for (i = 0; i < MAX_CH_NUM; i++)
	{
		// CONTINUE RECORD FLAG : '0' OFF, '1' ON, default is '1'
		psp->enc_ch[i].flag_conti_rec = get_nvram_para(
			offsetof(SETUP_PARAM, enc_ch[i].flag_conti_rec),
				sizeof(psp->enc_ch[i].flag_conti_rec));  	

		// frame rate : 0 ~ 30
		psp->enc_ch[i].frame_rate = get_nvram_para(
			offsetof(SETUP_PARAM, enc_ch[i].frame_rate),
				sizeof(psp->enc_ch[i].frame_rate));

		// bit rate : '0' VBR, '1' CBR(Picture level), '2' CBR(MB level)
		psp->enc_ch[i].bit_rate = get_nvram_para(
			offsetof(SETUP_PARAM, enc_ch[i].bit_rate),
				sizeof(psp->enc_ch[i].bit_rate));

		// VBR Q value : 1 ~ 31
		psp->enc_ch[i].vbr_q = get_nvram_para(
			offsetof(SETUP_PARAM, enc_ch[i].vbr_q), sizeof(psp->enc_ch[i].vbr_q));

		// CBR Q value : 0 ~ 1000
		psp->enc_ch[i].cbr_q = get_nvram_para(
			offsetof(SETUP_PARAM, enc_ch[i].cbr_q), sizeof(psp->enc_ch[i].cbr_q));

		// GOP_NM : N/M > 0, default is 5
		psp->enc_ch[i].gop_nm = get_nvram_para(
			offsetof(SETUP_PARAM, enc_ch[i].gop_nm), sizeof(psp->enc_ch[i].gop_nm));

		// GOP_M : M > 0, default is 3
		psp->enc_ch[i].gop_m = get_nvram_para(
			offsetof(SETUP_PARAM, enc_ch[i].gop_m), sizeof(psp->enc_ch[i].gop_m));

		// MOTION FLAG : '0' OFF, '1' ON, default is '0'
		psp->enc_ch[i].motion.flag = get_nvram_para(
			offsetof(SETUP_PARAM, enc_ch[i].motion.flag),
				sizeof(psp->enc_ch[i].motion.flag));

		// MOTION SENSITIVITY : 0 ~ 4
		psp->enc_ch[i].motion.sensitivity = get_nvram_para(
			offsetof(SETUP_PARAM, enc_ch[i].motion.sensitivity),
				sizeof(psp->enc_ch[i].motion.sensitivity));

		// MOTION AREA
		for (j = 0; j < MOTION_LINE_NUM; j++)
		{
			psp->enc_ch[i].motion.area[j] = get_nvram_para(
				offsetof(SETUP_PARAM, enc_ch[i].motion.area[j]), 
					sizeof(psp->enc_ch[i].motion.area[j]));
		}

		// MOTION AREA DISPLAY
		value = 0;
		for (j = 0; j < MOB_X_MAX; j++)
		{
			for (k = 0; k < MOB_Y_MAX; k++)
			{
				psp->enc_ch[i].motion.area_disp[j][k] = get_nvram_para(
					offsetof(SETUP_PARAM, enc_ch[i].motion.area_disp[j][k]), 
						sizeof(psp->enc_ch[i].motion.area_disp[j][k]));
			}
		}

		// SENSOR IN FLAG : '0' OFF, '1' ON, default is '0'
		psp->enc_ch[i].sensor.flag_in = get_nvram_para(
			offsetof(SETUP_PARAM, enc_ch[i].sensor.flag_in),
				sizeof(psp->enc_ch[i].sensor.flag_in));

		psp->enc_ch[i].sensor.flag_in_noc = get_nvram_para(
			offsetof(SETUP_PARAM, enc_ch[i].sensor.flag_in_noc),
				sizeof(psp->enc_ch[i].sensor.flag_in_noc));

		// SENSOR OUT FLAG : '0' OFF, '1' ON, default is '0'
		psp->enc_ch[i].sensor.flag_out = get_nvram_para(
			offsetof(SETUP_PARAM, enc_ch[i].sensor.flag_out),
				sizeof(psp->enc_ch[i].sensor.flag_out));

		psp->enc_ch[i].sensor.flag_out_noc = get_nvram_para(
			offsetof(SETUP_PARAM, enc_ch[i].sensor.flag_out_noc),
				sizeof(psp->enc_ch[i].sensor.flag_out_noc));

		psp->enc_ch[i].sensor.flag_out_duration= get_nvram_para(
			offsetof(SETUP_PARAM, enc_ch[i].sensor.flag_out_duration),
				sizeof(psp->enc_ch[i].sensor.flag_out_duration));

		// SCHEDULE FLAG : '0' OFF, '1' ON, default is '0'
		psp->enc_ch[i].schedule.flag = get_nvram_para(
			offsetof(SETUP_PARAM, enc_ch[i].schedule.flag),
				sizeof(psp->enc_ch[i].schedule.flag));

		// SCHEDULE TIME : from->2004-01-01 00:00, to->2004-01-01 00:00
		for (j = 0; j < MAX_DAY_OF_WEEK; j++)
		{
			psp->enc_ch[i].schedule.from_time[j].tm_year = get_nvram_para(
				offsetof(SETUP_PARAM, enc_ch[i].schedule.from_time[j].tm_year),
					sizeof(psp->enc_ch[i].schedule.from_time[j].tm_year));

			psp->enc_ch[i].schedule.from_time[j].tm_mon = get_nvram_para(
				offsetof(SETUP_PARAM, enc_ch[i].schedule.from_time[j].tm_mon),
					sizeof(psp->enc_ch[i].schedule.from_time[j].tm_mon));

			psp->enc_ch[i].schedule.from_time[j].tm_mday = get_nvram_para(
				offsetof(SETUP_PARAM, enc_ch[i].schedule.from_time[j].tm_mday),
					sizeof(psp->enc_ch[i].schedule.from_time[j].tm_mday));

			psp->enc_ch[i].schedule.from_time[j].tm_hour = get_nvram_para(
				offsetof(SETUP_PARAM, enc_ch[i].schedule.from_time[j].tm_hour),
					sizeof(psp->enc_ch[i].schedule.from_time[j].tm_hour));

			psp->enc_ch[i].schedule.from_time[j].tm_min = get_nvram_para(
				offsetof(SETUP_PARAM, enc_ch[i].schedule.from_time[j].tm_min),
					sizeof(psp->enc_ch[i].schedule.from_time[j].tm_min));

			psp->enc_ch[i].schedule.from_time[j].tm_sec = get_nvram_para(
				offsetof(SETUP_PARAM, enc_ch[i].schedule.from_time[j].tm_sec),
					sizeof(psp->enc_ch[i].schedule.from_time[j].tm_sec));

			psp->enc_ch[i].schedule.to_time[j].tm_year = get_nvram_para(
				offsetof(SETUP_PARAM, enc_ch[i].schedule.to_time[j].tm_year),
					sizeof(psp->enc_ch[i].schedule.to_time[j].tm_year));

			psp->enc_ch[i].schedule.to_time[j].tm_mon = get_nvram_para(
				offsetof(SETUP_PARAM, enc_ch[i].schedule.to_time[j].tm_mon),
					sizeof(psp->enc_ch[i].schedule.to_time[j].tm_mon));

			psp->enc_ch[i].schedule.to_time[j].tm_mday = get_nvram_para(
				offsetof(SETUP_PARAM, enc_ch[i].schedule.to_time[j].tm_mday),
					sizeof(psp->enc_ch[i].schedule.to_time[j].tm_mday));

			psp->enc_ch[i].schedule.to_time[j].tm_hour = get_nvram_para(
				offsetof(SETUP_PARAM, enc_ch[i].schedule.to_time[j].tm_hour),
					sizeof(psp->enc_ch[i].schedule.to_time[j].tm_hour));

			psp->enc_ch[i].schedule.to_time[j].tm_min = get_nvram_para(
				offsetof(SETUP_PARAM, enc_ch[i].schedule.to_time[j].tm_min),
					sizeof(psp->enc_ch[i].schedule.to_time[j].tm_min));

			psp->enc_ch[i].schedule.to_time[j].tm_sec = get_nvram_para(
				offsetof(SETUP_PARAM, enc_ch[i].schedule.to_time[j].tm_sec),
					sizeof(psp->enc_ch[i].schedule.to_time[j].tm_sec));
		}
   	}

	// 
	// DECODER PARAMETER
	//

	// video channel select : '0' CH1, '1' CH2, '2' CH3, '3' CH4, '4' Multi CH
	psp->dec.video_chan = get_nvram_para(
		offsetof(SETUP_PARAM, dec.video_chan), sizeof(psp->dec.video_chan));

	// de_interlace : '0' interlace, '1' non-interlace
	psp->dec.de_interlace = get_nvram_para(
		offsetof(SETUP_PARAM, dec.de_interlace), sizeof(psp->dec.de_interlace));

	// play_conti : '0' single file play, '1' continuous file play
	psp->dec.play_conti = get_nvram_para(
		offsetof(SETUP_PARAM, dec.play_conti), sizeof(psp->dec.play_conti));

	// audio channel select : '0' CH1, '1' CH2, '2' CH3, '3' CH4
	psp->dec.audio_chan = get_nvram_para(
		offsetof(SETUP_PARAM, dec.audio_chan), sizeof(psp->dec.audio_chan));

	// 
	// SYSTEM PARAMETER 
	//	

	// network ip configuration : default 192.168.1.100
	psp->sys.network.ipaddr = get_nvram_para(
		offsetof(SETUP_PARAM, sys.network.ipaddr), sizeof(psp->sys.network.ipaddr));

	// network netmask configuration : default 255.255.255.0
	psp->sys.network.netmask = get_nvram_para(
		offsetof(SETUP_PARAM, sys.network.netmask), sizeof(psp->sys.network.netmask));

	// network gateway configuration : default 192.168.1.1
	psp->sys.network.gateway = get_nvram_para(
		offsetof(SETUP_PARAM, sys.network.gateway), sizeof(psp->sys.network.gateway));

	// password configuration : default 000000
	for (i = 0; i < USER_NUM; i++)
	{
		psp->sys.password[i].id = get_nvram_para(
			offsetof(SETUP_PARAM, sys.password[i].id), sizeof(psp->sys.password[i].id));

		psp->sys.password[i].value = get_nvram_para(
			offsetof(SETUP_PARAM, sys.password[i].value),
				sizeof(psp->sys.password[i].value));
	}

	// live audio configuration : default live audio=ch1
	psp->sys.live_audio_param.live_audio_ch = get_nvram_para(
		offsetof(SETUP_PARAM, sys.live_audio_param.live_audio_ch),
			sizeof(psp->sys.live_audio_param.live_audio_ch));

	set_live_audio_ch(psp->sys.live_audio_param.live_audio_ch);

	// camera configuration : default brightness=5, contrast=5, hue=5, sharpness=5
	for (i = 0; i < MAX_CH_NUM; i++)
	{
		psp->sys.camera[i].brightness = get_nvram_para(
			offsetof(SETUP_PARAM, sys.camera[i].brightness),
				sizeof(psp->sys.camera[i].brightness));
#ifndef OURBOARD
		//#define TW99A_I2C_WRITE_ADDR	0x88	#define TW99A_I2C_READ_ADDR	0x89

		tw99_write_data((TW99A_I2C_WRITE_ADDR + (i * 2)) >> 1, 
			0x10, (char *)&psp->sys.camera[i].brightness, 1);
#else
		tw99_write_data((TW2804_I2C_WRITE_ADDR + (i * 2)) >> 1, 
			0x12 + i * 0x40, (char *)&psp->sys.camera[i].brightness, 1);
#endif

		psp->sys.camera[i].contrast = get_nvram_para(
			offsetof(SETUP_PARAM, sys.camera[i].contrast),
				sizeof(psp->sys.camera[i].contrast));
#ifndef OURBOARD
		tw99_write_data((TW99A_I2C_WRITE_ADDR + (i * 2)) >> 1, 
			0x11, (char *)&psp->sys.camera[i].contrast, 1);
#else
		tw99_write_data((TW99A_I2C_WRITE_ADDR + (i * 2)) >> 1, 
			0x11, (char *)&psp->sys.camera[i].contrast, 1);
#endif

		psp->sys.camera[i].color = get_nvram_para(
			offsetof(SETUP_PARAM, sys.camera[i].color),
				sizeof(psp->sys.camera[i].color));

#ifndef OURBOARD
		tw99_write_data((TW99A_I2C_WRITE_ADDR + (i * 2)) >> 1, 
			0x15, (char *)&psp->sys.camera[i].color, 1);
#else
		tw99_write_data((TW99A_I2C_WRITE_ADDR + (i * 2)) >> 1, 
			0x0f, (char *)&psp->sys.camera[i].color, 1);
#endif

	}

	// ptz configuration : default cam_vendor=scc--641, cam_speed=2, cam_ch=1
	psp->sys.ptz.cam_vendor = get_nvram_para(
		offsetof(SETUP_PARAM, sys.ptz.cam_vendor), sizeof(psp->sys.ptz.cam_vendor));

	psp->sys.ptz.cam_speed = get_nvram_para(
		offsetof(SETUP_PARAM, sys.ptz.cam_speed), sizeof(psp->sys.ptz.cam_speed));

	psp->sys.ptz.cam_ch = get_nvram_para(
		offsetof(SETUP_PARAM, sys.ptz.cam_ch), sizeof(psp->sys.ptz.cam_ch));

	return SUCCESS;
}

RETURN set_sys_network(SETUP_PARAM *psp)
{
	UNS8 ip[12];
	UNS8 netmask[12];
	UNS8 gateway[12];
	UNS16 value_u16;
	S8 str_net[80];
	S8 str_ip[20];
	S8 str_netmask[20];
	S8 str_gw[20];

	strcpy(str_net, "ifconfig eth0 ");

	// network ip configuration : default 192.168.1.100
	sprintf(str_ip, "%ld.%ld.%ld.%ld ", 
		(psp->sys.network.ipaddr & 0xff), (psp->sys.network.ipaddr >> 8) & 0xff, 
		(psp->sys.network.ipaddr >> 16) & 0xff, (psp->sys.network.ipaddr >> 24) & 0xff);
	strcat(str_net, str_ip);

	// network netmask configuration : default 255.255.255.0
	sprintf(str_ip, "netmask %ld.%ld.%ld.%ld", 
		(psp->sys.network.netmask & 0xff), (psp->sys.network.netmask >> 8) & 0xff,
		(psp->sys.network.netmask >> 16) & 0xff, (psp->sys.network.netmask >> 24) & 0xff);
	strcat(str_net, str_ip);

	if (system(str_net) != 0)
	{
		fprintf(stderr, "pthread_setup.c:error: In function 'system'\n");
	}
	strcpy(str_net, "route ");
	strcat(str_net, "add default gw ");

	// network gateway configuration : default 192.168.1.1
	sprintf(str_ip, "%ld.%ld.%ld.%ld",
		(psp->sys.network.gateway & 0xff), (psp->sys.network.gateway >> 8) & 0xff, 
		(psp->sys.network.gateway >> 16) & 0xff, (psp->sys.network.gateway >> 24) & 0xff);
	strcat(str_net, str_ip);

	if (system(str_net) != 0)
	{
		fprintf(stderr, "pthread_setup.c:error: In function 'system'\n");
	}
	printf("\n");

	return SUCCESS;
}


