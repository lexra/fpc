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
 
  MODULE NAME:  MAIN.H
  
  REVISION HISTORY:
  
  Date       Ver Name                  Description
  ---------- --- --------------------- -----------------------------------------
 06/28/2004 2.0 CheulBeck(whitefe)       Created 
 ...............................................................................
 
  DESCRIPTION:
  
  This Module contains definition for MAIN function.
  
 ...............................................................................
*/    
 
#ifndef __MAIN_H
#define __MAIN_H


/** ************************************************************************* ** 
 ** includes
 ** ************************************************************************* **/
#include <time.h> 
#include <semaphore.h>

#include "typedef.h"


/***************************************************************************** 
 ** PDEBUG
 *****************************************************************************/
#undef PDEBUG   /* undef it, just in case */
#ifdef DEBUG
 #ifdef __KERNEL__
  /* This one if debugging is on, and kernel space */
  #define PDEBUG(fmt, args...)  printk( KERN_DEBUG "debug: " fmt, ## args)
 #else
  /* This one for user space */
  #define PDEBUG(fmt, args...)  fprintf(stderr, fmt, ## args)
 #endif
#else
 #define PDEBUG(fmt, args...)                           /* not debugging: nothing */
#endif

 
/***************************************************************************** 
 ** defines
 *****************************************************************************/

//#define CIF_INTERLACE  
#define  __TRANS_THR__
#define CBR_MODE
#define FOR_DEMO
#define FAST_TEST
#define NVRAM_TEST

#define __WAIT__					0
#define __NO_WAIT__				1
#define __NOWAIT__				10


#ifndef CONSOLE_INPUT

 #ifdef OURBOARD
#define SDVR_KEY_POWER			0x80//
#define SDVR_KEY_OSD			0x01//
#define SDVR_KEY_1				0x0A
#define SDVR_KEY_2				0x0C
#define SDVR_KEY_3				0x48
#define SDVR_KEY_4				0x4C
#define SDVR_KEY_5				0x00
#define SDVR_KEY_6				0x0B
#define SDVR_KEY_7				0x09
#define SDVR_KEY_8				0x08
#define SDVR_KEY_9				0x18
#define SDVR_KEY_0				0x1F
#define SDVR_KEY_PLUS			0x05
#define SDVR_KEY_MINUS			0x07
#define SDVR_KEY_SETUP			0x54
#define SDVR_KEY_SUB_PLUS		0x15//
#define SDVR_KEY_SUB_MINUS		0x17//
#define SDVR_KEY_LEFT			0x04
#define SDVR_KEY_UP				0x02
#define SDVR_KEY_DOWN			0x06
#define SDVR_KEY_RIGHT			0x03
#define SDVR_KEY_SEL				0x41
#define SDVR_KEY_F1				0x14//
#define SDVR_KEY_F2				0x10//
#define SDVR_KEY_F3				0x1E//
#define SDVR_KEY_F4				0x40//
#define SDVR_KEY_F5				0x44//
#define SDVR_KEY_REW			0x0D
#define SDVR_KEY_PLAY			0x0E
#define SDVR_KEY_FF				0x0F
#define SDVR_KEY_REC			0x16//
#define SDVR_KEY_STOP			0x12
#define SDVR_KEY_SLOW			0x1B//
 #else
/* remote controller key mapping */
#define SDVR_KEY_POWER			0x80
#define SDVR_KEY_OSD			0x40
#define SDVR_KEY_1				0xB0
#define SDVR_KEY_2				0x70
#define SDVR_KEY_3				0xF0
#define SDVR_KEY_4				0x08
#define SDVR_KEY_5				0x88
#define SDVR_KEY_6				0x48
#define SDVR_KEY_7				0xC8
#define SDVR_KEY_8				0x28
#define SDVR_KEY_9				0xA8
#define SDVR_KEY_0				0x68
#define SDVR_KEY_PLUS			0x18
#define SDVR_KEY_MINUS			0xE8
#define SDVR_KEY_SETUP			0xC0
#define SDVR_KEY_SUB_PLUS		0x20
#define SDVR_KEY_SUB_MINUS		0xA0
#define SDVR_KEY_LEFT			0x60
#define SDVR_KEY_UP				0xE0
#define SDVR_KEY_DOWN			0x10
#define SDVR_KEY_RIGHT			0x90
#define SDVR_KEY_SEL				0x30
#define SDVR_KEY_F1				0x02
#define SDVR_KEY_F2				0x82
#define SDVR_KEY_F3				0x42
#define SDVR_KEY_F4				0xC2
#define SDVR_KEY_F5				0x22
#define SDVR_KEY_REW			0x38
#define SDVR_KEY_PLAY			0xB8
#define SDVR_KEY_FF				0x78
#define SDVR_KEY_REC			0x58
#define SDVR_KEY_STOP			0x98
#define SDVR_KEY_SLOW			0xD8
 #endif
#else
#define SDVR_KEY_POWER			0x1b// ESC
#define SDVR_KEY_OSD			0x27// '
#define SDVR_KEY_1				0x31
#define SDVR_KEY_2				0x32
#define SDVR_KEY_3				0x33
#define SDVR_KEY_4				0x34
#define SDVR_KEY_5				0x35
#define SDVR_KEY_6				0x36
#define SDVR_KEY_7				0x37
#define SDVR_KEY_8				0x38
#define SDVR_KEY_9				0x39
#define SDVR_KEY_0				0x30
#define SDVR_KEY_PLUS			0x2b// +
#define SDVR_KEY_MINUS			0x2d// -
#define SDVR_KEY_SETUP			0x3d// = 
#define SDVR_KEY_SUB_PLUS		0x5d// ]
#define SDVR_KEY_SUB_MINUS		0x5b// [
#define SDVR_KEY_LEFT			0x44
#define SDVR_KEY_UP				0x41
#define SDVR_KEY_DOWN			0x42
#define SDVR_KEY_RIGHT			0x43
#define SDVR_KEY_SEL				0x3b// ;
#define SDVR_KEY_F1				0x50
#define SDVR_KEY_F2				0x51
#define SDVR_KEY_F3				0x52
#define SDVR_KEY_F4				0x53
#define SDVR_KEY_F5				0x54
#define SDVR_KEY_REW			0x3f// ?
#define SDVR_KEY_PLAY			0x2f// /
#define SDVR_KEY_FF				0x3e// >
#define SDVR_KEY_REC			0x2e// .
#define SDVR_KEY_STOP			0x20// space
#define SDVR_KEY_SLOW			0x3c// <
#endif


/* definition for remocon key number */
#define POWER					SDVR_KEY_POWER
#define OSD						SDVR_KEY_OSD
#define NUMBER0					SDVR_KEY_0
#define NUMBER1					SDVR_KEY_1
#define NUMBER2					SDVR_KEY_2
#define NUMBER3					SDVR_KEY_3
#define NUMBER4					SDVR_KEY_4
#define NUMBER5					SDVR_KEY_5
#define NUMBER6					SDVR_KEY_6
#define NUMBER7					SDVR_KEY_7
#define NUMBER8					SDVR_KEY_8
#define NUMBER9					SDVR_KEY_9
#define NUM_PLUS					SDVR_KEY_PLUS
#define NUM_MINUS				SDVR_KEY_MINUS
#define SETUP					SDVR_KEY_SETUP
#define SUB_PLUS					SDVR_KEY_SUB_PLUS
#define SUB_MINUS				SDVR_KEY_SUB_MINUS
#define LEFT						SDVR_KEY_LEFT
#define RIGHT					SDVR_KEY_RIGHT
#define UP						SDVR_KEY_UP 
#define DOWN					SDVR_KEY_DOWN
#define SEL						SDVR_KEY_SEL
#define REW						SDVR_KEY_REW
#define PLAYBACK					SDVR_KEY_PLAY
#define FF						SDVR_KEY_FF
#define REC						SDVR_KEY_REC
#define SEARCH					SDVR_KEY_STOP
#define SLOW						SDVR_KEY_SLOW
#define RECSET					SDVR_KEY_F1
#define PTZ_ZOOMIN				SDVR_KEY_PLUS
#define PTZ_ZOOMOUT				SDVR_KEY_MINUS
#define PTZ_FOCUSIN				SDVR_KEY_F1
#define PTZ_FOCUSOUT			SDVR_KEY_F2
#define PTZ_CONTROL				SDVR_KEY_F4
#define STEP_F					SDVR_KEY_F5
#define PAUSE					SDVR_KEY_F4
#define STEP_B					SDVR_KEY_F3

/* Definition for AT4012 FONT */
#define UNIT_FONT_LINE			64

/* Definition for NVRAM Setup */
#define MPEG4_MAGICNUM			0x12345678
#define MPEG4_VERNUM			0x313630
#define TW99A_I2C_WRITE_ADDR	0x88
#define TW99A_I2C_READ_ADDR	0x89
#define MAXLEN_ETHADDR			6
#define MAXLEN_HOSTNAME			16
#define MAX_CH_NUM				4

//(00).(02).(04).(06).
#define TW2804_I2C_WRITE_ADDR	0x00
#define TW2804_I2C_READ_ADDR	0x01


/* directory for saving video stream */ 
#define	DEFAULT_REC_DIR			"/hdda/"
#define	HDDB_REC_DIR			"/hddb/"
#define	HDDC_REC_DIR			"/hddc/"
#define	HDDD_REC_DIR			"/hddd/"

/* file name for search DB */
#define DEFAULT_SEARCH_DB		"/hdda/search.db"
#define SEARCH_NDB				"/hdda/search.ndb"
#define SEARCH_TDB				"/hdda/search.tdb"

/* motion area */
#define MOTION_LINE_NUM			18
#define MAB_X_MAX				24
#define MAB_Y_MAX				18
#define MOB_X_MAX				11
#define MOB_Y_MAX				9
#define MOB_PIXEL				64
#define MOB_NUM					2
#define MAB_NUM					4
#define COLOR_MOTION			0x6c
#define COLOR_MOTION_CURSOR	0x70
#define COLOR_CURSOR			0x3c

/* osd */
#define FONTCOLOR				0x02
#define CURSORCOLOR				0x01
#define FILL						0xd8	/* Fill color */
#define BATANG					215
#define TRANS					0 		/* Transparency color */
#define NORMAL					0
#define BLINK						1
#define BLENDING					3

/* system parameter:password */
#define USER_NUM					2

/* system parameter:ptz */
#define PTZ_MAX_VENDOR			10
#define PTZ_SPEED_LEVEL			10

/* decoding trick mode */
#define trick_mode(a, b, c, d) \
		((a & 0x03) << 8) | \
		((b & 0x07) << 5) | \
		((c & 0x01) << 4) | \
		(d & 0x0f)

/* setup parameter for schedule */
#define MAX_DAY_OF_WEEK			7


/** ************************************************************************* ** 
 ** typedefs
 ** ************************************************************************* **/
typedef enum {
	MAIN_SETUP = 0,	
	MAIN_SEARCH,
	MAIN_BACKUP,
	MAIN_LIVE,
	MAIN_PTZ,
	MAIN_IDLE
} MAIN_LOC;

typedef enum {
	PTHREAD_MAIN = 0,
	PTHREAD_MANAGER,
	PTHREAD_INPUT,
	PTHREAD_SETUP,
	PTHREAD_SEARCH,
	PTHREAD_ENC,
	PTHREAD_DEC,
   	PTHREAD_LIVE,
   	PTHREAD_DISKM,
   	PTHREAD_SCHEDULE,
   	PTHREAD_TRANS,
   	PTHREAD_GPIO,
   	PTHREAD_PTZ,
	PTHREAD_MAX
} PTHREAD_ID;

typedef enum {
	PTHREAD_ITEM_0,
	PTHREAD_ITEM_1,
	PTHREAD_ITEM_2,
	PTHREAD_ITEM_3,
	PTHREAD_ITEM_4,
	PTHREAD_ITEM_5,
	PTHREAD_ITEM_6,
	PTHREAD_ITEM_7,
	PTHREAD_MAX_ITEM
} PTHREAD_ITEM;

typedef enum {
	INIT_FLAG = 0,
	WRITE_FLAG,
	READ_FLAG,
} PTHREAD_SIG_COND;

typedef enum {
	SIGNAL_0,
	SIGNAL_1,
	SIGNAL_2,
	SIGNAL_3,
	SIGNAL_4,
	SIGNAL_MAX
} PTHREAD_SIGNAL;

typedef struct __PTHREAD_BUF__ {
	PTHREAD_ID start_id;
	UNS16 m_signal;
	UNS16 m_value;
	UNS16 m_channel;
} PTHREAD_BUF;

/* ==================TYPEDEF for PTHREAD_STATE=============== */
#ifdef CBR_MODE
typedef enum {
	CBR_05M = 0,	
	CBR_10M,
	CBR_15M,
	CBR_20M,
	CBR_25M,
	CBR_30M,
	CBR_35M,
	CBR_40M,
	CBR_45M,
	CBR_50M	
} CBR_VALUE;
#endif

/* ==================TYPEDEF for PTHREAD_STATE=============== */
typedef enum {
	IDLE = 0,	
	BUSY,
	ALIVE
} MAIN_STATE;

typedef struct __STATE_MAIN__ {
	MAIN_STATE state;
	VIDEO_MODE mode;
} STATE_MAIN;

typedef enum {
	ALL = 0,
	CONTI,
	MOTION,
	SENSOR,
} MODE_REC;

typedef enum {
	MB_SIZE_1X = 1,
	MB_SIZE_2X,
	MB_SIZE_3X,
	MB_SIZE_4X
} MOTION_BLOCK_SIZE;

typedef struct __ENC_CH__ {
	UNS16 motion;
	UNS16 sensor;
	UNS16 schedule;
} ENC_CH;

typedef struct __STATE_ENC__ {
	MAIN_STATE state;	
	UNS16 manual_rec;
	UNS16 schedule_rec;
	ENC_CH enc_ch[MAX_CH_NUM];	
} STATE_ENC;

typedef enum {
	D_PLAY = 0,
	D_STOP,
	D_PAUSE,
	D_SLOW,
	D_REW,
	D_STEP_B,
	D_FF,
	D_STEP_F,
	D_IDLE
} MODE_PLAY;

typedef struct __STATE_DEC__ {
	MAIN_STATE state;
	MODE_REC mode_rec;
	MODE_PLAY mode_play;
	UNS16 play_step;
	UNS16 play_change;
	PTHREAD_BUF signal;
} STATE_DEC;

typedef enum {
	L_QUAD,	
	L_CH1,
	L_CH2,
	L_CH3,
	L_CH4,
	L_1CH_PB,
	L_4CH_PB,
	L_6CH_PB
} LIVE_MON;

typedef struct __STATE_LIVE__ {
	LIVE_MON mode_mon;
	struct tm	cur_time;	
} STATE_LIVE;

typedef struct __STATE_DB__ {
	sem_t search_sem;
} STATE_DB;

typedef struct __STATE_PTZ__ {
	MAIN_STATE state;
} STATE_PTZ;

typedef struct __PTHREAD_STATE__ {
	STATE_MAIN state_main;
	STATE_ENC state_enc;
	STATE_DEC state_dec;
	STATE_LIVE state_live;
	STATE_DB state_db;
	STATE_PTZ state_ptz;
} PTHREAD_STATE;

/* ================TYPEDEF for SETUP_PARAM================== */
typedef struct __MOTION_PARAM__ {
	UNS32 flag;
	UNS32 sensitivity;
	UNS32 area[MOTION_LINE_NUM];
	UNS32 area_disp[MOB_X_MAX][MOB_Y_MAX];
} MOTION_PARAM;

typedef struct __SENSOR_PARAM__ {
	UNS32 flag_in;
	UNS32 flag_out;
	UNS16 flag_in_noc;
	UNS16 flag_out_noc;
	UNS16 flag_out_duration;
} SENSOR_PARAM;

typedef struct __SCHEDULE_PARAM__ {
	UNS32 flag;
	struct tm from_time[MAX_DAY_OF_WEEK];
	struct tm to_time[MAX_DAY_OF_WEEK];
} SCHEDULE_PARAM;

typedef struct __GLOBAL_PARAM__ {
	UNS32 video_form;
	UNS32 resolution;
} GLOBAL_PARAM;

typedef struct __ENCODER_PARAM__ {
	UNS32 flag_conti_rec;
	UNS32 frame_rate;
	UNS32 bit_rate;
	UNS32 vbr_q;
	UNS32 cbr_q;	
	UNS32 gop_nm;
	UNS32 gop_m;
	MOTION_PARAM motion;
	SENSOR_PARAM sensor;
	SCHEDULE_PARAM	schedule;
} ENCODER_PARAM;

typedef struct __DECODER_PARAM__ {
	UNS32 video_chan;
	UNS32 de_interlace;
	UNS32 play_conti;
	UNS32 audio_chan;
} DECODER_PARAM;

typedef struct __NETWORK_PARAM__ {
	UNS32 ipaddr;
	UNS32 netmask;
	UNS32 gateway;
	UNS8 enetaddr[MAXLEN_ETHADDR];
	UNS8 hostname[MAXLEN_HOSTNAME];
} NETWORK_PARAM;

typedef struct __PASSWORD_PARAM__ {
	UNS32 id;
	UNS32 value;
} PASSWORD_PARAM;

typedef struct __LIVE_AUDIO_PARAM__ {
	UNS32 live_audio_ch;
} LIVE_AUDIO_PARAM;

typedef struct __CAMERA_PARAM__ {
	S16 brightness;
	S16 contrast;
	S16 color;
} CAMERA_PARAM;

typedef struct __PTZ_PARAM__ {
	UNS16 cam_vendor;
	UNS16 cam_speed;
	UNS16 cam_ch;
	UNS16 preset_start_pos;
	UNS16 preset_end_pos;
	UNS16 preset_interval;
	UNS16 preset_seq_flag;
} PTZ_PARAM;

typedef struct __SYSTEM_PARAM__ {
	NETWORK_PARAM network;
	CAMERA_PARAM camera[MAX_CH_NUM];
	PASSWORD_PARAM password[USER_NUM];
	LIVE_AUDIO_PARAM live_audio_param;
	PTZ_PARAM ptz;
} SYSTEM_PARAM;

typedef struct __SETUP_PARAM__ {
	UNS32 magic_num;
	UNS32 version_num;
	GLOBAL_PARAM gp;
	ENCODER_PARAM enc_ch[MAX_CH_NUM];
	DECODER_PARAM dec;
	SYSTEM_PARAM sys;
} SETUP_PARAM;

/* ================TYPEDEF for RECORD DB================== */
typedef struct __DB_RECORD__ {
	S8 fname[32];
	UNS16 conti;
	UNS16 motion;
	UNS16 sensor;
	S8 start_time[10];
	S8 end_time[8];
	UNS32 end_fp;
	UNS32 prev_db_fp;
	UNS32 next_db_fp;
} DB_RECORD;

/* type definition for picture info */
typedef struct __INFO_PICTURE__ {
	UNS16 data_type;
	UNS16 type;
	UNS16 ch;
	UNS16 loss;
	UNS16 motion_start;
	UNS16 motion_end;	
	UNS16 sensor_start;	
	UNS16 sensor_end;	
	UNS16 start_flag;
	UNS16 last_flag;
	UNS16 dummy;
	UNS32 size;
	UNS32 time;
	UNS32 start_fp;
} INFO_PICTURE;

/* ================TYPEDEF for PTHREAD_DEC================== */
typedef enum {
	TM_PLAY = 0,
	TM_FAST,
	TM_SLOW,
	TM_STEP,
	TM_PAUSE,
	TM_NON_REAL_TIME
} TRICK_MODE;

typedef enum {
	TD_FORWARD = 0,
	TD_BACKWARD
} TRICK_DIR;

typedef enum {
	TME_NORMAL = 0,
	TME_REAL
} TRICK_METHOD;
  
/** ************************************************************************* ** 
 ** function prototypes
 ** ************************************************************************* **/
#ifdef __cplusplus
extern "C" {
#endif

RETURN nvram_check_magic_num(SETUP_PARAM *psp);
RETURN nvram_load_setup_value(SETUP_PARAM *psp);
RETURN set_sys_network(SETUP_PARAM	*psp);
BOOL pthread_read_signal(PTHREAD_BUF *rbuf, PTHREAD_ID thread_num, BOOL wait);
BOOL pthread_send_signal(PTHREAD_BUF *sbuf, PTHREAD_ID thread_num);
BOOL pthread_broadcast_signal(PTHREAD_BUF *sbuf, PTHREAD_ID thread_num);
RETURN pthread_initialize_mutex(void);
RETURN pthread_destory_mutex(void);
RETURN pthread_initialize_cond(void);
RETURN pthread_destroy_cond(void);
RETURN pthread_create_manager(void);
RETURN pthread_create_input(void);
RETURN pthread_create_live(void);
RETURN pthread_create_diskm(void);
RETURN pthread_create_setup(void);
RETURN pthread_create_search(void);
RETURN pthread_create_enc(void);
RETURN pthread_create_dec(void);
RETURN pthread_create_schedule(void);
RETURN pthread_create_trans(void);
RETURN pthread_create_gpio(void);
RETURN pthread_create_ptz(void);
RETURN load_font(void);
RETURN printregion(S16 x_offset, S16 y_offset, S16 x_size, S16 y_size, S16 color, S16 blend);
RETURN printfont(S16 x_offset, S16 y_offset, S8 *str, S16 color, S16 blend);
RETURN printmotion(S16 x_offset, S16 y_offset, S16 x_size, S16 y_size, S16 font, S16 color, S16 blend);
RETURN clear_osd_all(void);

#ifdef __cplusplus
}
#endif


#endif /* __MAIN_H */

