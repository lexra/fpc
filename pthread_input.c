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
 
  MODULE NAME:  PTHREAD_INPUT.C
  
  REVISION HISTORY:
  
  Date       Ver Name                  Description
  ---------- --- --------------------- -----------------------------------------
  06/30/2004 2.0 CheulBeck(whitefe)       Created   
 ...............................................................................
 
  DESCRIPTION:
  
  This Module contains functions for Input.
  
 ...............................................................................
*/  
 

/** ************************************************************************* ** 
 ** includes
 ** ************************************************************************* **/
#include <stdio.h>
#include <fcntl.h>

#include "main.h"
#include "pthread_input.h"


/** ************************************************************************* ** 
 ** defines
 ** ************************************************************************* **/
//#define m_DEBUG(format, args...)  printf(format, ## args) 
#define m_DEBUG(format, args...)  
#define m_MSG(format, args...)  printf(format, ## args) 
#define m_ERROR(format, args...)  printf(format, ## args) 


#define TW99A_I2C_WRITE_ADDR	0x88
#define TW99A_I2C_READ_ADDR	0x89



/** ************************************************************************* ** 
 ** globals
 ** ************************************************************************* **/
extern PTHREAD_STATE *gp_state_thread;
extern SETUP_PARAM *gp_setup_param;

#ifndef CONSOLE_INPUT

 #ifdef OURBOARD
void *pthread_input(void *args)
{
	PTHREAD_BUF signal;
	unsigned int dwValue;

	while (1)
	{

		// while loop should be blocked here
		if (saa7146_ir_read(&dwValue) == -1)
			break;

		switch (dwValue)
		{
		case SDVR_KEY_POWER:
		case SDVR_KEY_OSD:
		case SDVR_KEY_1:
		case SDVR_KEY_2:
		case SDVR_KEY_3:
		case SDVR_KEY_4:
		case SDVR_KEY_5:
		case SDVR_KEY_6:
		case SDVR_KEY_7:
		case SDVR_KEY_8:
		case SDVR_KEY_9:
		case SDVR_KEY_0:
		case SDVR_KEY_PLUS:
		case SDVR_KEY_MINUS:
		case SDVR_KEY_SETUP:
		case SDVR_KEY_SUB_PLUS:
		case SDVR_KEY_SUB_MINUS:
		case SDVR_KEY_LEFT:
		case SDVR_KEY_UP:
		case SDVR_KEY_DOWN:
		case SDVR_KEY_RIGHT:
		case SDVR_KEY_SEL:
		case SDVR_KEY_F1:
		case SDVR_KEY_F2:
		case SDVR_KEY_F3:
		case SDVR_KEY_F4:
		case SDVR_KEY_F5:
		case SDVR_KEY_REW:
		case SDVR_KEY_PLAY:
		case SDVR_KEY_FF:			
		case SDVR_KEY_REC:
		case SDVR_KEY_STOP:
		case SDVR_KEY_SLOW:
			m_DEBUG("!!! INPUT VALUE: 0x%02x\n", dwValue); fflush(stdout);
			signal.start_id = PTHREAD_INPUT;
			signal.m_value = dwValue;
			if (pthread_send_signal(&signal, PTHREAD_MAIN) == FALSE)
				fprintf(stderr, "pthread_input.c:error: In function 'pthread_send_signal'\n");
			break;

		default:
			break;
		}
	}

	return;
}
 #else
void *pthread_input(void *args)
{
	int i;
	UNS16 count;
	UNS8 r_data[MAX_CH_NUM];
	UNS8 w_data[MAX_CH_NUM];
	UNS8 prev_data[MAX_CH_NUM];
	S32 fd;
	time_t cur_time;
	time_t prev_time;
	IR_IOCTL_T cmd;
	PTHREAD_BUF signal;
	unsigned int dwValue;


	fd = open("/dev/au1500_gpio_ir", O_RDWR);
	if (fd < 0)
	{
		fprintf(stderr, "pthread_input.c:error: In function 'open'\n");
		return 0;
	}

	for (i = 0; i < MAX_CH_NUM; i++)
	{
		r_data[i] = 0;
		w_data[i] = 0;
		prev_data[i] = 0;
	}

	cmd = IR_VALUE;
	count = 0;
	dwValue = 0;
	cur_time = 0;
	prev_time = 0;
	while (1)
	{
		ioctl(fd, cmd, &dwValue);

		// 1st. send input data to other threads
		switch (dwValue)
		{
		case SDVR_KEY_POWER:
		case SDVR_KEY_OSD:
		case SDVR_KEY_1:
		case SDVR_KEY_2:
		case SDVR_KEY_3:
		case SDVR_KEY_4:
		case SDVR_KEY_5:
		case SDVR_KEY_6:
		case SDVR_KEY_7:
		case SDVR_KEY_8:
		case SDVR_KEY_9:
		case SDVR_KEY_0:
		case SDVR_KEY_PLUS:
		case SDVR_KEY_MINUS:
		case SDVR_KEY_SETUP:
		case SDVR_KEY_SUB_PLUS:
		case SDVR_KEY_SUB_MINUS:
		case SDVR_KEY_LEFT:
		case SDVR_KEY_UP:
		case SDVR_KEY_DOWN:
		case SDVR_KEY_RIGHT:
		case SDVR_KEY_SEL:
		case SDVR_KEY_F1:
		case SDVR_KEY_F2:
		case SDVR_KEY_F3:
		case SDVR_KEY_F4:
		case SDVR_KEY_F5:
		case SDVR_KEY_REW:
		case SDVR_KEY_PLAY:
		case SDVR_KEY_FF:			
		case SDVR_KEY_REC:
		case SDVR_KEY_STOP:
		case SDVR_KEY_SLOW:
			m_DEBUG("!!! INPUT VALUE: 0x%02x\n", dwValue); fflush(stdout);
			signal.start_id = PTHREAD_INPUT;
			signal.m_value = dwValue;
			if (pthread_send_signal(&signal, PTHREAD_MAIN) == FALSE)
				fprintf(stderr, "pthread_input.c:error: In function 'pthread_send_signal'\n");
			break;

		default:
			break;
		}

		usleep(100000);

		// 2nd. video loss check routine
		if (count == 3)
		{
			for (i = 0; i < MAX_CH_NUM; i++)
			{

  #if 1
				tw99_read_data((TW99A_I2C_READ_ADDR + i * 2) >> 1, 0x01, &r_data[i], 1);
				usleep(10000);
				//m_MSG(" VIDEO LOSS [0x%x] !!!\n", r_data[i]); fflush(stdout);

				if ((r_data[i] & 0x80) != (prev_data[i] & 0x80))
				{
					if (r_data[i] & 0x80)
					{
						w_data[i] = 0xee;
						m_MSG("\t!!! CH[%d] VIDEO LOSS !!!\n", i); fflush(stdout);
					}
					else
					{
						w_data[i] = 0xe6;
						m_MSG("\t!!! CH[%d] VIDEO ON !!!\n", i); fflush(stdout);
					}

					tw99_write_data((TW99A_I2C_WRITE_ADDR + i * 2) >> 1, 0x2f, &w_data[i], 1);
					prev_data[i] = r_data[i];

				}
  #else
				tw99_read_data((TW2804_I2C_READ_ADDR + i * 2) >> 1, 0x38, &r_data[i], 1);
				usleep(10000);

				if ((r_data[i] & 0x80) != (prev_data[i] & 0xf0))
				{
					if (r_data[i] & 0x80)
					{
						w_data[i] = 0xee;
						m_MSG("\t!!! CH[%d] VIDEO LOSS !!!\n", i); fflush(stdout);
					}
					else
					{
						w_data[i] = 0xe6;
						m_MSG("\t!!! CH[%d] VIDEO ON !!!\n", i); fflush(stdout);
					}

					//tw99_write_data((TW2804_I2C_WRITE_ADDR + i * 2) >> 1, 0x2f, &w_data[i], 1);
					prev_data[i] = r_data[i];

				}
  #endif

				usleep(10000);
			}
			count = 0;
		} 
		else
			count ++;


		// 3rd. get current time
		time(&cur_time);
		if (cur_time != prev_time)
		{
			memcpy(&gp_state_thread->state_live.cur_time, localtime(&cur_time), sizeof(struct tm));
			signal.start_id = PTHREAD_MANAGER;
			signal.m_signal = SIGNAL_2;
			if (pthread_send_signal(&signal, PTHREAD_LIVE) == FALSE)
				fprintf(stderr, "pthread_input.c:error: In function 'pthread_send_signal'\n");
			prev_time = cur_time;
		}
	}

	close(fd);
}
 #endif

#else
void *pthread_input(void *args)
{
	PTHREAD_BUF signal;
	unsigned int dwValue;

	while (1)
	{
		dwValue = 0;
		scanf("%c", &dwValue);
		fprintf(stderr, "INPUT VALUE: 0x%02x\n", dwValue);

		switch (dwValue)
		{
		case SDVR_KEY_POWER:
		case SDVR_KEY_OSD:
		case SDVR_KEY_1:
		case SDVR_KEY_2:
		case SDVR_KEY_3:
		case SDVR_KEY_4:
		case SDVR_KEY_5:
		case SDVR_KEY_6:
		case SDVR_KEY_7:
		case SDVR_KEY_8:
		case SDVR_KEY_9:
		case SDVR_KEY_0:
		case SDVR_KEY_PLUS:
		case SDVR_KEY_MINUS:
		case SDVR_KEY_SETUP:
		case SDVR_KEY_SUB_PLUS:
		case SDVR_KEY_SUB_MINUS:
		case SDVR_KEY_LEFT:
		case SDVR_KEY_UP:
		case SDVR_KEY_DOWN:
		case SDVR_KEY_RIGHT:
		case SDVR_KEY_SEL:
		case SDVR_KEY_F1:
		case SDVR_KEY_F2:
		case SDVR_KEY_F3:
		case SDVR_KEY_F4:
		case SDVR_KEY_F5:
		case SDVR_KEY_REW:
		case SDVR_KEY_PLAY:
		case SDVR_KEY_FF:			
		case SDVR_KEY_REC:
		case SDVR_KEY_STOP:
		case SDVR_KEY_SLOW:
			signal.start_id = PTHREAD_INPUT;
			signal.m_value = dwValue;
			if (pthread_send_signal(&signal, PTHREAD_MAIN) == FALSE)
				fprintf(stderr, "pthread_input.c:error: In function 'pthread_send_signal'\n");
			break;

		default:
			break;
		}

	}

	return;
}
#endif


