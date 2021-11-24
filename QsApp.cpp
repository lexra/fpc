// QsApp.cpp: implementation of the CQsApp class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <strings.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <regex.h>
#include <sys/ioctl.h> 
#include <sys/ipc.h>
#include <signal.h>
#include <sys/msg.h>
#include <pthread.h>
#include <assert.h>
#include <termios.h>
#include <netinet/in.h>
#include <inttypes.h>
#include <stddef.h>
#include <assert.h>
#include <sys/socket.h>
#include <sys/reboot.h>
#include <math.h>
#include <sys/resource.h>


#include <asm/ioctl.h>
#include <asm/errno.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#include "list.h"
#include "Event.h"
#include "Misc.h"
#include "FpcTb.h"
#include "BaseApp.h"
#include "QsApp.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#ifdef WIN32
#define new DEBUG_NEW
#endif//WIN32
#endif // _DEBUG



//////////////////////////////////////////////////////////////////////
// define
//////////////////////////////////////////////////////////////////////

#define GPIO_FUNC_ORG           0
#define GPIO_FUNC_GPIO          1
#define GPIO_DIR_OUTPUT         0
#define GPIO_DIR_INPUT          1
#define GPIO_PULL_LOW           0
#define GPIO_PULL_HIGH          1
#define GPIO_PULL_FLOATING      2
#define GPIO_NO_PULL			3
#define GPIO_IRQ_DISABLE        0
#define GPIO_IRQ_ENABLE         1



#define MAX_STRIDE_LENGTH				(27*2)
#define MIN_STRIDE_LENGTH				(16*2)
#define DEFAULT_STRIDE_LENGTH			(21*2)
#define MAX_RESISTANCE_LEVEL    30
#define MIN_RESISTANCE_LEVEL    1



#define GUI_window_size	12	//gui windows size


//#define HOLD_STILL_INDEX	4
// jason note
#define HOLD_STILL_INDEX	5

#define Mile_KM_ratio		1.6	//not 1.609344
#define default_cool_down_time	2
#define default_work_load_level	1

#define FP_TICKS_PER_SECOND	10
#define HR_MARGIN 		10

#define MAX_RPM   120
#define MIN_RPM   30



#define CMD_GUI									33
#define FC_HELLO								0X00
#define FC_START								0X80
#define FC_STOP									0X81
#define FC_BUZZER								0X82
#define FC_PAUSE								0X83
#define FC_RESUME								0X84
#define FC_SET_AUDIO_CH						0X85
#define FC_CHECK_IPOD_DOCK						0X86
#define FC_CHECK_RUNTIME_WORKOUT_SECONDS	0X87
#define FC_SET_LOCALTIME						0X88
#define FC_COOL_DOWN							0X89
#define FC_SET_WORKLOAD						0X8A
#define FC_SET_PACE								0X8B
#define FC_SET_STRIDE							0X8C
#define FC_BUTTON								0X95
#define FC_GET_SUMMARY							0X90
#define FC_GET_SETUP							0X92
#define FC_SET_SETUP							0X93
#define FC_GET_UPDATE							0X94

//#define FC_SET_DS_BTN_TYPE						0X96
//#define FC_GET_DS_BTN_TYPE						0X97
//#define FC_SET_WORK_PAGE						0X98
//#define FC_GET_WORK_PAGE						0X99
#define FC_WORK_MODE_CHANGE					0X9A


#define FPC_FIRE_START											1
#define FPC_FIRE_STOP											2
#define FPC_FIRE_PAUSE											3
#define FPC_FIRE_RESUME										4
#define FPC_FIRE_COOL_DOWN									5
#define FPC_FIRE_SUMMARY_READY								6
#define FPC_FIRE_READ_SETUP									7


#define GP_ADC_MAGIC	'G'
#define IOCTL_GP_ADC_START		_IOW(GP_ADC_MAGIC,0x01,unsigned long)
#define IOCTL_GP_ADC_STOP		_IO(GP_ADC_MAGIC,0x02)

#define GP_PWM_MAGIC		'P'
#define PWM_IOCTL_SET_ENABLE		_IOW(GP_PWM_MAGIC, 1, unsigned int)
#define PWM_IOCTL_SET_ATTRIBUTE		_IOW(GP_PWM_MAGIC, 2, gp_pwm_config_t)
#define PWM_IOCTL_GET_ATTRIBUTE		_IOR(GP_PWM_MAGIC, 2, gp_pwm_config_t*)


#define PWM1_TIMER_ID							101
#define KEY_SCAN_TIMER_ID						102
#define FPC_WORKOUT_IN_NORMAL_ID				104
#define FPC_PROCESS_KEY_EV_ID					105
#define FPC_RETURN_WORKOUT_HOME_ID			106
#define FPC_RETURN_DEFAULT_DS_ID				107
#define FPC_POPUP_WEIGHT_ID					108
#define FPC_DISMISS_WEIGHT_ID					109
#define FPC_ADJUST_RESIST_MOTOR_ID			110
#define FPC_PROBE_CURRENT_AD_ID				111
#define BV_OK_BUZZ_ON_ID						112
#define BV_OK_BUZZ_OFF_ID						113
#define BUZZ_ON_TIMER_ID						114
#define BUZZ_OFF_TIMER_ID						115
#define IPOD_BUTTONRELEASE_TIMER_ID			116


#define FPC_SCHEDULER_1MS_ID					301
#define FPC_SCHEDULER_10MS_ID					302
#define FPC_SCHEDULER_70MS_ID					303
#define FPC_SCHEDULER_100MS_ID				304
#define FPC_SCHEDULER_200MS_ID				305
#define FPC_SCHEDULER_250MS_ID				306
#define FPC_SCHEDULER_1000MS_ID				307

#define SD55_CMD_ID											400
#define SD55_CMD_INIT_TIMEOUT_ID							401
#define SD55_CMD_RESET_CONTORLER_TIMEOUT_ID				402
#define SD55_CMD_CC_INIT_ID								403
#define SD55_CMD_DEFAULT_STRIDE_ID						404
#define SD55_CMD_RD_SPU_ID								405
#define SD55_CMD_SET_STRIDE_ID							406
#define SD55_CMD_SET_AUTO_STRIDE_ID						407




#define PCA_9555_SLAVE_0											0X24
#define PCA_9555_SLAVE_1											0X20
#define PCA_9555_SLAVE_2											0X21

#define PCA_9555_INPUT_0											0
#define PCA_9555_INPUT_1											1
#define PCA_9555_OUTPUT_0											2
#define PCA_9555_OUTPUT_1											3
#define PCA_9555_POLARITY_0										4
#define PCA_9555_POLARITY_1										5
#define PCA_9555_CONFIG_0											6
#define PCA_9555_CONFIG_1											7

#define KS_LEFT_TOP												1
#define KS_LEFT_CENTER											2
#define KS_LEFT_BOTTOM											3
#define KS_RIGHT_TOP											4
#define KS_RIGHT_CENTER										5
#define KS_RIGHT_BOTTOM										6
#define KS_START												9
#define KS_STOP													21
#define KS_WORKLOAD_UP										7
#define KS_WORKLOAD_DOWN										8
#define KS_STRIDE_UP											22
#define KS_STRIDE_DOWN											23

#define KS_NUM0													10
#define KS_NUM1													11
#define KS_NUM2													12
#define KS_NUM3													13
#define KS_NUM4													14
#define KS_NUM5													15
#define KS_NUM6													16
#define KS_NUM7													17
#define KS_NUM8													18
#define KS_NUM9													19

#define KS_DELETE												20



//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////




typedef struct work_mode_state_t {
	unsigned char type;
	unsigned int target;
	unsigned int source;
	unsigned char key;
	unsigned char shift;
} work_mode_state_t;



#define SD55_FRAME_HEADER		0xF5
#define SD55_FRAME_YES		0x5A
#define SD55_FRAME_NOT		0xA5
#define SD55_FRAME_NOTHING		0x55

typedef enum
{
	// 0x00
	SERIAL_CMD_INIT = 0,

	// 0x01 --> 設阻力馬達位置
	SERIAL_CMD_WR_RESIST_POS,
	// 0x02 --> 讀阻力馬達位置
	SERIAL_CMD_RD_RESIST_POS,
	// 0x03 --> 設跨步馬達位置
	SERIAL_CMD_WR_STRIDE_POS,
	// 0x04 --> 讀跨步馬達位置
	SERIAL_CMD_RD_STRIDE_POS,
	// 0x05 --> 設高度馬達位置
	SERIAL_CMD_WR_INCLINE_POS,
	// 0x06 --> 讀高度馬達位置
	SERIAL_CMD_RD_INCLINE_POS,
	// 0x07 --> 讀 SPU INTERVAL
	SERIAL_CMD_RD_SPU_INTERVAL,
	// 0x08 --> 讀狀態
	SERIAL_CMD_RD_STATE,
	// 0x09 --> 馬達直接控制
	SERIAL_CMD_MOTOR_CTRL,
	// 0x0A --> 歸零步數
	SERIAL_CMD_INIT_STEP,
	// 0x0B --> 讀取步數
	SERIAL_CMD_RD_STEP,
	// 0x0C --> 設定STRIDE最高段數起始值
	SERIAL_STRIDE_FIRST_POS,

	SERIAL_CMD_RESERVED1,		// 0x0D
	SERIAL_CMD_RESERVED2,		// 0x0E
	SERIAL_CMD_RESERVED3,		// 0x0F
	SERIAL_CMD_RESERVED4,		// 0x10

	// 0x11 --> 軟件复位SD55
	SERIAL_RESET_CONTORLER
}SD55_CMD;


// 阻力馬達段位
unsigned char pSD55_Resist_Level_Table[] =	
{
	6,	13,	19,	26,	33,	39,	46,	52,	59,	65, 
	72,	78,	85,	91,	98,	105,	111,	118,	124,	131,
	137,	144,	150,	157,	163,	170,	177,	183,	190,	196
};

// 跨步馬達
unsigned char pStride_Len_BitMap_Table[] = 
{
	2,	5,	9,	12,	16,	19,	23,	26,	30,	33,
	37,	40,	44,	47,	51,	54,	58,	61,	65,	68,
	72,	75,	77
};

#if 1
#define MIN_AS		45.00F
#define MAX_AS		203.00F

#define DELTA_AS	((MAX_AS - MIN_AS) / 29.00F)

/*
pAsResist_Level_Table[0]=50
pAsResist_Level_Table[1]=55
pAsResist_Level_Table[2]=60
pAsResist_Level_Table[3]=66
pAsResist_Level_Table[4]=71
pAsResist_Level_Table[5]=76
pAsResist_Level_Table[6]=81
pAsResist_Level_Table[7]=86
pAsResist_Level_Table[8]=91
pAsResist_Level_Table[9]=97
pAsResist_Level_Table[10]=102
pAsResist_Level_Table[11]=107
pAsResist_Level_Table[12]=112
pAsResist_Level_Table[13]=117
pAsResist_Level_Table[14]=122
pAsResist_Level_Table[15]=128
pAsResist_Level_Table[16]=133
pAsResist_Level_Table[17]=138
pAsResist_Level_Table[18]=143
pAsResist_Level_Table[19]=148
pAsResist_Level_Table[20]=153
pAsResist_Level_Table[21]=159
pAsResist_Level_Table[22]=164
pAsResist_Level_Table[23]=169
pAsResist_Level_Table[24]=174
pAsResist_Level_Table[25]=179
pAsResist_Level_Table[26]=184
pAsResist_Level_Table[27]=190
pAsResist_Level_Table[28]=195
pAsResist_Level_Table[29]=200
*/

// 阻力馬達段位
unsigned char pAsResist_Level_Table[] =	
{
	(unsigned char)( MIN_AS + (0.00F * DELTA_AS) ), 
	(unsigned char)( MIN_AS + (1.00F * DELTA_AS) ), 
	(unsigned char)( MIN_AS + (2.00F * DELTA_AS) ), 
	(unsigned char)( MIN_AS + (3.00F * DELTA_AS) ), 
	(unsigned char)( MIN_AS + (4.00F * DELTA_AS) ), 
	(unsigned char)( MIN_AS + (5.00F * DELTA_AS) ), 
	(unsigned char)( MIN_AS + (6.00F * DELTA_AS) ), 
	(unsigned char)( MIN_AS + (7.00F * DELTA_AS) ), 
	(unsigned char)( MIN_AS + (8.00F * DELTA_AS) ), 
	(unsigned char)( MIN_AS + (9.00F * DELTA_AS) ), 
	(unsigned char)( MIN_AS + (10.00F * DELTA_AS) ), 
	(unsigned char)( MIN_AS + (11.00F * DELTA_AS) ), 
	(unsigned char)( MIN_AS + (12.00F * DELTA_AS) ), 
	(unsigned char)( MIN_AS + (13.00F * DELTA_AS) ), 
	(unsigned char)( MIN_AS + (14.00F * DELTA_AS) ), 
	(unsigned char)( MIN_AS + (15.00F * DELTA_AS) ), 
	(unsigned char)( MIN_AS + (16.00F * DELTA_AS) ), 
	(unsigned char)( MIN_AS + (17.00F * DELTA_AS) ), 
	(unsigned char)( MIN_AS + (18.00F * DELTA_AS) ), 
	(unsigned char)( MIN_AS + (19.00F * DELTA_AS) ), 
	(unsigned char)( MIN_AS + (20.00F * DELTA_AS) ), 
	(unsigned char)( MIN_AS + (21.00F * DELTA_AS) ), 
	(unsigned char)( MIN_AS + (22.00F * DELTA_AS) ), 
	(unsigned char)( MIN_AS + (23.00F * DELTA_AS) ), 
	(unsigned char)( MIN_AS + (24.00F * DELTA_AS) ), 
	(unsigned char)( MIN_AS + (25.00F * DELTA_AS) ), 
	(unsigned char)( MIN_AS + (26.00F * DELTA_AS) ), 
	(unsigned char)( MIN_AS + (27.00F * DELTA_AS) ), 
	(unsigned char)( MIN_AS + (28.00F * DELTA_AS) ), 
	(unsigned char)( MIN_AS + (29.00F * DELTA_AS) ), 

	//39,	44,	50,	55,	76,	81,	86,	91,	96,	101, 
	//106,	111,	116,	121,	126,	131,	136,	141,	146,	151,
	//156,	161,	166,	171,	176,	179,	184,	189,	194,	200
};
#else
unsigned char pAsResist_Level_Table[] =	
{
	36,	38,	40,	43,	51,	59,	67,	75,	83,	90, 
	96,	102,	108,	114,	120,	126,	132,	138,	144,	150,
	156,	162,	168,	174,	180,	186,	192,	198,	204,	210
};
#endif

// 發電機段位, FREQ 20
unsigned char pGsResist_Level_Table[] =	
{
	90,	87,	84,	81,	79,	76,	73,	71,	68,	65, 
	62,	60,	57,	54,	51,	49,	46,	43,	40,	38,
	35,	32,	29,	27,	24,	21,	18,	16,	13,	10

	//10,	13,	16,	18,	21,	24,	27,	29,	32,	35, 
	//38,	40,	43,	46,	49,	51,	54,	57,	60,	62,
	//65,	68,	71,	73,	76,	79,	81,	84,	87,	90
};


int product_type = 1;

int IPOD_MUTE_VAR = 0;
int IPOD_VOL_VAR = 5;


int IPOD_Detect = 0;
int sd55_ok = 0;
int nCmd = 0;

unsigned char T_AD = 36;
unsigned char C_AD = 0;

int prob_ms6257 = 0;

int prob_pca955_0 = 0;
int prob_pca955_1 = 0;
int prob_pca955_2 = 0;
int SPU_PIN = 0;



//////////////////////////////////////////////////////////////////////
// routines
//////////////////////////////////////////////////////////////////////

#define GPIO_IOCTL_ID           'G'
#define GPIO_IOCTL_SET_VALUE    _IOW(GPIO_IOCTL_ID, 0, int)
#define GPIO_IOCTL_GET_VALUE    _IOR(GPIO_IOCTL_ID, 1, int)
#define GPIO_IOCTL_SET_INPUT	_IOW(GPIO_IOCTL_ID, 2, int)
#define GPIO_IOCTL_SET_PROPERTY _IOW(GPIO_IOCTL_ID, 3, int)
#define GPIO_IOCTL_SET_INT      _IOW(GPIO_IOCTL_ID, 4, int)

#define MK_GPIO_INDEX(ch, func, gid, pin) \
			(((ch) << 24) | ((func) << 16) | ((gid) << 8) | (pin))

typedef struct gpio_content_s {
	unsigned int pin_index;     /*!< @brief gpio pin index */
	unsigned int value;         /*!< @brief gpio value */
	unsigned int debounce;      /*!< @brief gpio debounce value */
} gpio_content_t;


#define GP_PWM_MAGIC					'P'
#define PWM_IOCTL_SET_ENABLE			_IOW(GP_PWM_MAGIC, 1, unsigned int)
#define PWM_IOCTL_SET_ATTRIBUTE		_IOW(GP_PWM_MAGIC, 2, gp_pwm_config_t)
#define PWM_IOCTL_GET_ATTRIBUTE		_IOR(GP_PWM_MAGIC, 2, gp_pwm_config_t*)


typedef struct gp_pwm_config_s {
	unsigned int freq;
	int duty;
	int pin_index;
} gp_pwm_config_t;


#define	MAX_SEGMENTS	101
unsigned char _workLoad_Table_cruise[MAX_SEGMENTS];	//x
unsigned char _workLoad_Table[MAX_SEGMENTS];		//x
unsigned char _workPace_Table[MAX_SEGMENTS];		//x	
unsigned char _workWatt_Table[MAX_SEGMENTS];		//x
unsigned char _segmentTime_Table[MAX_SEGMENTS];		//x


//#define BV_Board_Support	1
#define BV_CMD_POWER					0
#define BV_CMD_MODE_TV				1
#define BV_CMD_MODE_FM				2
#define BV_CMD_VOL_DOWN			3
#define BV_CMD_VOL_UP				4
#define BV_CMD_MUTE					5
#define BV_CMD_CHN_DEC				6
#define BV_CMD_CHN_INC				7
#define BV_CMD_HOLD_SEEK_UP		8
#define BV_CMD_GET_FREQ				9
#define BV_CMD_GET_VOL				10
#define BV_CMD_GET_STATUS			11
#define BV_CMD_GET_ALL_STATUS		12
#define BV_CMD_ENTER_PROG_MODE	13
#define BV_CMD_PROG_MODE_1		14
#define BV_CMD_PROG_MODE_2		15
#define BV_CMD_PROG_MODE_3		16
#define BV_CMD_PROG_MODE_4		17
#define BV_CMD_PROG_MODE_5		18
#define BV_CMD_PROG_MODE_6		19
#define BV_CMD_PROG_MODE_7		20
#define BV_CMD_PROG_MODE_8		21
#define BV_CMD_PROG_MODE_9		22
#define BV_CMD_DEC_CHANNEL		23
#define BV_CMD_INC_CHANNEL			24
#define BV_CMD_DEC_FREQ				25
#define BV_CMD_INC_FREQ				26
#define BV_CMD_SAVE_FREQ_CHAN	27
#define BV_CMD_DEL_FREQ_CHAN		28
#define BV_CMD_EXIT_PROG_MODE	29
#define BV_CMD_VOL_UP_BOARD		30
#define BV_CMD_HOLD_SEEK_DOWN	31///nick add 20100414
//************************************************************************************
//VERSION "CS800 X1.24"
#define BV_CMD_TOGGLE_900MHZ		32///nick add 20100416 Toggle 900MHz on or off
#define BV_CMD_PROG_MODE_0		33///nick add 20100416 Clear Memories and Channels
#define BV_CMD_TOGGLE_100_200KHZ 34///nick add 20100416 Toggle station increment value (100 KHz or 200 KHz)
#define BV_CMD_MUTE_BOARD			35///nick add 20100416 Toggle BV audio on and off (not used, TRUE will handle)
#define BV_CMD_EQ_ONOFF				36///nick add 20100416 Toggle Equalizer on or off (Default is ON)
//**************************************************************************************
///*********************************************************************
///nick add 20100428
///VERSION "LC900 X1.28""CS800 X1.28"
#define BV_CMD_900MHZ_STATUS		37///900MHz Mode F: "9" for 900 MHz frequency or "F" for FM frequency
#define BV_CMD_100_200KHZ_STATUS 38///Frequency Step Mode I: "1" for 100 KHz frequency incrementor "2" for 200 KHz frequency increment
//**********************************************************************
#define BV_Source_TV		1
#define BV_Source_FM		2



static unsigned char tx_buf[3];
unsigned char bv_a_family[11][2] =
{
	{0x41, 0x42}, //AB
	{0x41, 0x43}, //AC
	{0x41, 0x44}, //AD
	{0x41, 0x45}, //AE
	{0x41, 0x46}, //AF
	{0x41, 0x47}, //AG
	{0x41, 0x48}, //AH
	{0x41, 0x49}, //AI
	{0x41, 0x4A}, //AJ
	{0x41, 0x4B}, //AK
	{0x41, 0x4C}, //AL///nick add 20100414
};
unsigned char bv_b_family[21][2] =
{
	{0x42, 0x31}, //B1
	{0x42, 0x32}, //B2
	{0x42, 0x33}, //B3
	{0x42, 0x34}, //B4
	{0x42, 0x35}, //B5
	{0x42, 0x36}, //B6
	{0x42, 0x37}, //B7
	{0x42, 0x38}, //B8
	{0x42, 0x39}, //B9
	{0x42, 0x30}, //B0
	{0x42, 0x41}, //BA
	{0x42, 0x43}, //BC
	{0x42, 0x44}, //BD
	{0x42, 0x45}, //BE
	{0x42, 0x46}, //BF
	{0x42, 0x47}, //BG
	{0x42, 0x48}, //BH
	{0x42, 0x49}, //BI
	{0x42, 0x4A}, //BJ
	{0x42, 0x4B}, //BK
	{0x42, 0x4C}, //BL ///nick add 20100414
};
unsigned char bv_c_family[9][2] =
{
	{0x43, 0x41}, //CA
	{0x43, 0x42}, //CB
	{0x43, 0x44}, //CD
	{0x43, 0x45}, //CE
	{0x43, 0x46}, //CF
	{0x43, 0x53}, //CS ///nick add 20100414
	{0x43, 0x56}, //CV ///nick add 20100414
	{0x43, 0x47}, //CG ///nick add 20100428
	{0x43, 0x48}, //CH ///nick add 20100428	
};


// 	{0x41, 0x48, 0x0d}

void Bv_SendCmd(int fd, int cmd)
{
	//long i;
	int len = 3;

	switch(cmd)
	{
	case BV_CMD_POWER:
		tx_buf[0]=bv_a_family[5][0];
		tx_buf[1]=bv_a_family[5][1];
		tx_buf[2]=0xd;
		break;
	case BV_CMD_MODE_TV:
			tx_buf[0]=bv_a_family[6][0];
			tx_buf[1]=bv_a_family[6][1];
			tx_buf[2]=0xd;
		break;
	case BV_CMD_MODE_FM:
			tx_buf[0]=bv_a_family[6][0];
			tx_buf[1]=bv_a_family[6][1];
			tx_buf[2]=0xd;
		break;
	case BV_CMD_VOL_DOWN:
		//if (IPOD_VOL_VAR > 0)
		//{
		//	IPOD_VOL_VAR --;
		//}
		//IPOD_MUTE_VAR = false;
		//Set_LED(LED_MULT,false);
		break;
	case BV_CMD_VOL_UP:
		  //if (IPOD_VOL_VAR < 20)
		  //{
		  //	IPOD_VOL_VAR ++;
		  //}
		  //IPOD_MUTE_VAR = false;
		  //Set_LED(LED_MULT,false);
		break;
	case BV_CMD_MUTE:
			/*if (IPOD_MUTE_VAR == false)
			{
				IPOD_MUTE_VAR = true;
				// 無聲
				ipod_vol_tab(0);
				sprintf(Str_Temp, "Mute");
				Show_Message(CENTER, Str_Temp);
				Set_LED(LED_MULT,true);
			}
			else
			{
				IPOD_MUTE_VAR = false;
				BV_Display_Volume();
				Show_Message(CENTER, Str_Temp);
				Set_LED(LED_MULT,false);
			}*/
		break;
	case BV_CMD_CHN_DEC:
		tx_buf[0]=bv_a_family[2][0];
		tx_buf[1]=bv_a_family[2][1];
		tx_buf[2]=0xd;
		//BV_Send_Comm(tx_buf,3);	
		break;
	case BV_CMD_CHN_INC:
		tx_buf[0]=bv_a_family[3][0];
		tx_buf[1]=bv_a_family[3][1];
		tx_buf[2]=0xd;
		break;
	case BV_CMD_HOLD_SEEK_UP:
		tx_buf[0]=bv_a_family[4][0];
		tx_buf[1]=bv_a_family[4][1];
		tx_buf[2]=0xd;
		break;
		case BV_CMD_HOLD_SEEK_DOWN:
		tx_buf[0]=bv_a_family[10][0];
		tx_buf[1]=bv_a_family[10][1];
		tx_buf[2]=0xd;
		break;	
	case BV_CMD_GET_FREQ:
		tx_buf[0]=bv_c_family[1][0];
		tx_buf[1]=bv_c_family[1][1];
		tx_buf[2]=0xd;
		//BV_State_flag=3;
		//Rx2_len=0;
		break;
	case BV_CMD_GET_VOL:
		tx_buf[0]=bv_c_family[2][0];
		tx_buf[1]=bv_c_family[2][1];
		tx_buf[2]=0xd;
		//BV_State_flag=4;
		//Rx2_len=0;
		break;		
	case BV_CMD_GET_STATUS:
		tx_buf[0]=bv_c_family[3][0];
		tx_buf[1]=bv_c_family[3][1];
		tx_buf[2]=0xd;
		//BV_State_flag=2;
		//Rx2_len=0;
		break;
	case BV_CMD_GET_ALL_STATUS:
		tx_buf[0]=bv_c_family[0][0];
		tx_buf[1]=bv_c_family[0][1];
		tx_buf[2]=0xd;
		//BV_State_flag=1;
		//Rx2_len=0;
		break;
	case BV_CMD_ENTER_PROG_MODE:
		tx_buf[0]=bv_b_family[14][0];
		tx_buf[1]=bv_b_family[14][1];
		tx_buf[2]=0xd;
		break;
 	case BV_CMD_PROG_MODE_1:
		tx_buf[0]=bv_b_family[0][0];
		tx_buf[1]=bv_b_family[0][1];
		tx_buf[2]=0xd;
		break;
 	case BV_CMD_PROG_MODE_2:
		tx_buf[0]=bv_b_family[1][0];
		tx_buf[1]=bv_b_family[1][1];
		tx_buf[2]=0xd;
		break;		
 	case BV_CMD_PROG_MODE_3:
		tx_buf[0]=bv_b_family[2][0];
		tx_buf[1]=bv_b_family[2][1];
		tx_buf[2]=0xd;
		break;		
 	case BV_CMD_PROG_MODE_4:
		tx_buf[0]=bv_b_family[3][0];
		tx_buf[1]=bv_b_family[3][1];
		tx_buf[2]=0xd;
		break;		
 	case BV_CMD_PROG_MODE_5:
		tx_buf[0]=bv_b_family[4][0];
		tx_buf[1]=bv_b_family[4][1];
		tx_buf[2]=0xd;
		break;
 	case BV_CMD_PROG_MODE_6:
		tx_buf[0]=bv_b_family[5][0];
		tx_buf[1]=bv_b_family[5][1];
		tx_buf[2]=0xd;
		break;		
 	case BV_CMD_PROG_MODE_7:
		tx_buf[0]=bv_b_family[6][0];
		tx_buf[1]=bv_b_family[6][1];
		tx_buf[2]=0xd;
		break;		
 	case BV_CMD_PROG_MODE_8:
		tx_buf[0]=bv_b_family[7][0];
		tx_buf[1]=bv_b_family[7][1];
		tx_buf[2]=0xd;
		break;		
 	case BV_CMD_PROG_MODE_9:
		tx_buf[0]=bv_b_family[8][0];
		tx_buf[1]=bv_b_family[8][1];
		tx_buf[2]=0xd;
		break;		
 	case BV_CMD_DEC_CHANNEL:
		tx_buf[0]=bv_b_family[10][0];
		tx_buf[1]=bv_b_family[10][1];
		tx_buf[2]=0xd;
		break;		
 	case BV_CMD_INC_CHANNEL:
		tx_buf[0]=bv_b_family[11][0];
		tx_buf[1]=bv_b_family[11][1];
		tx_buf[2]=0xd;
		break;	
 	case BV_CMD_DEC_FREQ:
		tx_buf[0]=bv_b_family[12][0];
		tx_buf[1]=bv_b_family[12][1];
		tx_buf[2]=0xd;
		break;		
 	case BV_CMD_INC_FREQ:
		tx_buf[0]=bv_b_family[13][0];
		tx_buf[1]=bv_b_family[13][1];
		tx_buf[2]=0xd;
		break;	
 	case BV_CMD_SAVE_FREQ_CHAN:
		tx_buf[0]=bv_b_family[17][0];
		tx_buf[1]=bv_b_family[17][1];
		tx_buf[2]=0xd;
		break;	
 	case BV_CMD_DEL_FREQ_CHAN:
		tx_buf[0]=bv_b_family[18][0];
		tx_buf[1]=bv_b_family[18][1];
		tx_buf[2]=0xd;
		break;			
	case BV_CMD_EXIT_PROG_MODE:
		tx_buf[0]=bv_b_family[15][0];
		tx_buf[1]=bv_b_family[15][1];
		tx_buf[2]=0xd;
		break;
	case BV_CMD_VOL_UP_BOARD:
		tx_buf[0]=bv_a_family[1][0];
		tx_buf[1]=bv_a_family[1][1];
		tx_buf[2]=0xd;
		break;
	case BV_CMD_TOGGLE_900MHZ:
		tx_buf[0]=bv_b_family[16][0];
		tx_buf[1]=bv_b_family[16][1];
		tx_buf[2]=0xd;
		break;
 	case BV_CMD_PROG_MODE_0:
		tx_buf[0]=bv_b_family[9][0];
		tx_buf[1]=bv_b_family[9][1];
		tx_buf[2]=0xd;
		break;		
 	case BV_CMD_TOGGLE_100_200KHZ:
		tx_buf[0]=bv_b_family[20][0];
		tx_buf[1]=bv_b_family[20][1];
		tx_buf[2]=0xd;
		break;	
	case BV_CMD_MUTE_BOARD:
		tx_buf[0]=bv_a_family[8][0];
		tx_buf[1]=bv_a_family[8][1];
		tx_buf[2]=0xd;
		break;
 	case BV_CMD_EQ_ONOFF:
		tx_buf[0]=bv_b_family[19][0];
		tx_buf[1]=bv_b_family[19][1];
		tx_buf[2]=0xd;
		break;			
	case BV_CMD_900MHZ_STATUS:
		tx_buf[0]=bv_c_family[7][0];
		tx_buf[1]=bv_c_family[7][1];
		tx_buf[2]=0xd;
		//BV_State_flag=8;
		//Rx2_len=0;
		//BV_TV_FM_STATUS=0;
		break;
	case BV_CMD_100_200KHZ_STATUS:
		tx_buf[0]=bv_c_family[8][0];
		tx_buf[1]=bv_c_family[8][1];
		tx_buf[2]=0xd;
		//BV_State_flag=8;
		//Rx2_len=0;
		//BV_TV_FM_STATUS=0;
		break;
	default:
		break;	
	}
	if (-1 == write(fd, tx_buf, len))
		printf("(%s %d) WRITE() FAIL\n", __FILE__, __LINE__);

}



unsigned char Volume_IIC_Table[20][2] = 
{
////E0,C0                   ///-db       , +db     , =           ,volume 
   {0xE6, 0xC3},//-60db,+3db,-57db,1
   {0xE6, 0xC6},//-60db,+6db,-54db,2
   {0xE6, 0xC9},//-60db,+9db,-51db,3
   {0xE5, 0xC2},//-50db,+2db,-48db,4
   {0xE5, 0xC5},//-50db,+5db,-45db,5
   {0xE5, 0xC8},//-50db,+8db,-42db,6
   {0xE4, 0xC1},//-40db,+1db,-39db,7
   {0xE4, 0xC4},//-40db,+4db,-36db,8
   {0xE4, 0xC7},//-40db,+7db,-33db,9
   {0xE3, 0xC0},//-30db,+0db,-30db,10
   {0xE3, 0xC3},//-30db,+3db,-27db,11
   {0xE3, 0xC6},//-30db,+6db,-24db,12
   {0xE3, 0xC9},//-30db,+9db,-21db,13
   {0xE2, 0xC2},//-20db,+2db,-18db,14
   {0xE2, 0xC5},//-20db,+5db,-15db,15
   {0xE2, 0xC8},//-20db,+8db,-12db,16
   {0xE1, 0xC1},//-10db,+1db,-9db,17
   {0xE1, 0xC4},//-10db,+4db,-6db,18
   {0xE1, 0xC7},//-10db,+7db,-3db,19
   {0xE0, 0xC0},//-0db,+0db,+0db,20
};

void ipod_vol_tab(int fd, unsigned char index)
{
	if (index == 0)
	{
		// 無聲
		i2c_write_data(fd, 0X44, 0X79, 0, 0);
	}
	else if (index > 0 && index <= 20)
	{
		i2c_write_data(fd, 0X44, 0XD0, Volume_IIC_Table[index - 1], 2);
		i2c_write_data(fd, 0X44, 0X78, 0, 0);
	}
	else
	{
		i2c_write_data(fd, 0X44, 0X79, 0, 0);
	}
}


#define iPod_CMD_BUTTONRELEASE	0
#define iPod_CMD_PLAY_PAUSE			1
#define iPod_CMD_SKIP_INC				2
#define iPod_CMD_SKIP_DEC				3
#define iPod_CMD_VOL_DOWN				4
#define iPod_CMD_VOL_UP					5
#define iPod_CMD_MUTE						6
#define iPod_CMD_SOURCE					7

static unsigned char Tx2_Buffer[1024];

void iPod_SendCmd(int fd, int cmd)
{
	int Send2_Len = 1;



/*
Play/Pause 	0xFF 0x55 0x03 0x02 0x00 0x01 0xFA
Vol+ 	0xFF 0x55 0x03 0x02 0x00 0x02 0xF9
Vol- 	0xFF 0x55 0x03 0x02 0x00 0x04 0xF7
Skip>> 	0xFF 0x55 0x03 0x02 0x00 0x08 0xF3
<<Skip 	0xFF 0x55 0x03 0x02 0x00 0x10 0xEB
End Button 	0xFF 0x55 0x03 0x02 0x00 0x00 0xFB 


*/



//          L  M   C
// FF 55 03 00 01 02 FA // SW to M2
// (0x100 - [actual values of length + mode + command + parameter]) & 0xff
// 0x100 - 3 -2 - 0 - 0 = FB
// 100 - 3 - 0 -1 - 2

// to switch to AiR mode the following bytes are sent:
//    0xFF 0x55 0x03 0x00 0x01 0x04 0xF8 


/*
0x00 0x00
	Button Released
0x00 0x01
	Play
0x00 0x02
	Vol+
0x00 0x04
	Vol-
0x00 0x08
	Skip>
0x00 0x10
	Skip<
0x00 0x20
	Next Album
0x00 0x40
	Previous Album
0x00 0x80
	Stop
0x00 0x00 0x01
	Play
0x00 0x00 0x02
	Pause
0x00 0x00 0x04
	Mute (toggle)
0x00 0x00 0x20
	Next Playlist
0x00 0x00 0x40
	Previous Playlist
0x00 0x00 0x80
	Toggles Shuffle
0x00 0x00 0x00 0x01
	Toggles Repeat
0x00 0x00 0x00 0x04
	Ipod Off
0x00 0x00 0x00 0x08
	Ipod On
0x00 0x00 0x00 0x40
	Menu Button
0x00 0x00 0x00 0x80
	OK/Select
0x00 0x00 0x00 0x00 0x01
	Scroll Up
0x00 0x00 0x00 0x00 0x02
	Scroll Down 
*/

	
	Tx2_Buffer[0]=0xFF;
	Tx2_Buffer[1]=0x55;
	switch(cmd)
	{
		case iPod_CMD_BUTTONRELEASE:
			Tx2_Buffer[2]=0x03;	// Length
			Tx2_Buffer[3]=0x02; // Mode
			Tx2_Buffer[4]=0x00; // Command
			Tx2_Buffer[5]=0x00; // Command
			Tx2_Buffer[6]=0xFB; // CheckSum
			Send2_Len = 8;
			break;
		case iPod_CMD_PLAY_PAUSE:
			Tx2_Buffer[2]=0x03;
			Tx2_Buffer[3]=0x02;
			Tx2_Buffer[4]=0x00;
			Tx2_Buffer[5]=0x01;
			Tx2_Buffer[6]=0xFA;
			Send2_Len = 8;
			break;
		case iPod_CMD_SKIP_INC:
			Tx2_Buffer[2]=0x03;
			Tx2_Buffer[3]=0x02;
			Tx2_Buffer[4]=0x00;
			Tx2_Buffer[5]=0x08;
			Tx2_Buffer[6]=0xF3;
			Send2_Len = 8;
			break;
		case iPod_CMD_SKIP_DEC:
			Tx2_Buffer[2]=0x03;
			Tx2_Buffer[3]=0x02;
			Tx2_Buffer[4]=0x00;
			Tx2_Buffer[5]=0x10;
			Tx2_Buffer[6]=0xEB;
			Send2_Len = 8;
			break;
		case iPod_CMD_VOL_DOWN:
			Tx2_Buffer[2]=0x03;
			Tx2_Buffer[3]=0x02;
			Tx2_Buffer[4]=0x00;
			Tx2_Buffer[5]=0x04;
			Tx2_Buffer[6]=0xF7;
			Send2_Len = 8;
			break;
		case iPod_CMD_VOL_UP:
			Tx2_Buffer[2]=0x03;
			Tx2_Buffer[3]=0x02;
			Tx2_Buffer[4]=0x00;
			Tx2_Buffer[5]=0x02;
			Tx2_Buffer[6]=0xF9;
			Send2_Len = 8;
			break;
		case iPod_CMD_MUTE:
			Tx2_Buffer[2]=0x04;
			Tx2_Buffer[3]=0x02;
			Tx2_Buffer[4]=0x00;
			Tx2_Buffer[5]=0x00;
			Tx2_Buffer[6]=0x04;
			Tx2_Buffer[7]=0xF6;
			Send2_Len = 9;
			break;
		default:
			break;
	}

	if (-1 == write(fd, Tx2_Buffer, Send2_Len))
		printf("(%s %d) WRITE() FAIL\n", __FILE__, __LINE__);
}


int SendCmd(int fd, int cmd, int len, const unsigned char *value)
{
	unsigned char buff[1024];
	int cs;
	int i;
	int l;

	if (len < 0)						return -1;
	if (len > 0)
	{
		if (0 == value)				return -1;
		for(i = 0; i < len; i++)			buff[i + 3] = value[i];
	}
	l = len + 2;
	buff[0] = SD55_FRAME_HEADER;
	buff[1] = l | (0xF0 - ( l << 4 ));
	buff[2] = cmd;

	cs = buff[1]; cs += buff[2];
	for(i = 0; i < len; i++)
		cs += buff[3 + i];
	buff[len + 3] = (unsigned char)(0 - cs);
	nCmd = cmd;
	l = write(fd, buff, len + 4);

	return l;
}



#if 0
int end_of_tick = 0;

void *TickThread(void *param)
{
	static unsigned int counter = 0;

	sigset_t nset, oset;
	CQsApp *app = (CQsApp *)AfxGetApp();
	long m, n, a;
	static long o = 0;
	struct timespec tv;

	HANDLE hTickStart = app->hTickStart;
	HANDLE h[] = {hTickStart, 0, 0};
	unsigned int res;


///////////////////////
	sigemptyset(&nset);
	sigaddset(&nset, SIGHUP), sigaddset(&nset, SIGINT), sigaddset(&nset, SIGTERM), sigaddset(&nset, SIGUSR1), sigaddset(&nset, SIGALRM);
	pthread_sigmask(SIG_BLOCK, &nset, &oset);
	pthread_cleanup_push(RestoreSigmask, (void *)&oset);

START:
	end_of_tick = 0;
	counter = 0;
	res = WaitForMultipleObjects(1, h, WAIT_FOR_ANY_ONE_EVENTS, INFINITE);
	if (WAIT_FAILED == res) { dbg_printf(DBG_BASIC, "(%s %d) WaitForMultipleObjects() >  WAIT_FAILED \n", __FILE__, __LINE__); exit(1); }
	if (WAIT_OBJECT_0 + 0 != res) { goto START; }


///////////////////////
	for(;;)
	{
		clock_gettime(CLOCK_REALTIME, &tv);
		a = (tv.tv_sec * 1000000000) + tv.tv_nsec;
		m = (a % 100000000);
		n = (a - m) / 100000000;
		if (n != o)
		{
			o = n;

			counter++;
			app->PostMessage(M_TICK, (WPARAM)counter, 0);
			//SetEvent(app->hFpcTick);

		}

		if (end_of_tick)			break;
		usleep(999 * 30);
	}
///////////////////////

	goto START;


///////////////////////
	pthread_cleanup_pop(0);

dbg_printf(DBG_THREAD, "(%s %d) TickThread() EXIT\n", __FILE__, __LINE__);
	return 0;
}
#endif


extern int timer_inprocessing;


void CalculateTimer(UINT nId)
{
	CQsApp *app = (CQsApp *)AfxGetApp();

	app->DataCollection();
	app->Calculate();

	if (0 == app->SetTimer(FPC_SCHEDULER_1000MS_ID, 998, CalculateTimer))
	{
	}
}

void BuzzOffTimer(UINT nId)
{
	CQsApp *app = (CQsApp *)AfxGetApp();
	unsigned char V = 0;

	if (-1 == i2c_read_data(app->i2c0, PCA_9555_SLAVE_1, PCA_9555_INPUT_0, &V, 1))
	{
		printf("(%s %d) i2c_read_data() FAIL \n", __FILE__, __LINE__);
		return;
	}
	V &= ~BIT6;
	if (0 == product_type || 1 == product_type)
		V |= BIT5;	// LCD_BL-EN
	if (-1 == i2c_write_data(app->i2c0, PCA_9555_SLAVE_1, PCA_9555_OUTPUT_0, &V, 1))
	{
		printf("(%s %d) i2c_write_data() FAIL \n", __FILE__, __LINE__);
		return;
	}
}

void BuzzOnTimer(UINT nId)
{
	CQsApp *app = (CQsApp *)AfxGetApp();
	unsigned char V = 0;

	if (-1 == i2c_read_data(app->i2c0, PCA_9555_SLAVE_1, PCA_9555_INPUT_0, &V, 1))
	{
		printf("(%s %d) i2c_read_data() FAIL \n", __FILE__, __LINE__);
		goto again;
	}
	V |= BIT6;
	if (0 == product_type || 1 == product_type)
		V |= BIT5;
	if (-1 == i2c_write_data(app->i2c0, PCA_9555_SLAVE_1, PCA_9555_OUTPUT_0, &V, 1))
		printf("(%s %d) i2c_write_data() FAIL \n", __FILE__, __LINE__);

again:
	if (0 == app->SetTimer(BUZZ_OFF_TIMER_ID, 31, BuzzOffTimer))
	{
	}
}

int PollKeyScan(unsigned char line);


void KsTimer(UINT nId)
{
	CQsApp *app = (CQsApp *)AfxGetApp();
	static unsigned char I = 0;

	PollKeyScan(I);
	I++;
	I %= 7;
	if (0 == app->SetTimer(KEY_SCAN_TIMER_ID, 17, KsTimer))
	{
	}
}

void ProbeAdTimer(UINT nId);
void AdjustMotorTimer(UINT nId);

void AdjustMotorTimer(UINT nId)
{
	CQsApp *app = (CQsApp *)AfxGetApp();
	int D = 0;
	unsigned char V = 0;


	if (1 != product_type)
		return;

	if (-1 == i2c_read_data(app->i2c0, PCA_9555_SLAVE_1, PCA_9555_INPUT_0, &V, 1)) { return; }
	D = (int)T_AD - (int)C_AD;
	if (D <= 1 && D >= -1)
	{
		V |= BIT2; V |= BIT3;
		V |= BIT5;
		i2c_write_data(app->i2c0, PCA_9555_SLAVE_1, PCA_9555_OUTPUT_0, &V, 1);
		return;
	}
	if (D > 0)			{ V &= ~BIT2; V |= BIT3; }
	else					{ V &= ~BIT3; V |= BIT2; }
	V |= BIT5;
	i2c_write_data(app->i2c0, PCA_9555_SLAVE_1, PCA_9555_OUTPUT_0, &V, 1);

	app->SetTimer(FPC_PROBE_CURRENT_AD_ID, 3, ProbeAdTimer);
}

void ProbeAdTimer(UINT nId)
{
	unsigned char V = 0;
	gpio_content_t ctx;
	int value[] = {0, 0};
	CQsApp *app = (CQsApp *)AfxGetApp();

	if (1 != product_type)
		return;
	i2c_read_data(app->i2c0, PCA_9555_SLAVE_1, PCA_9555_INPUT_0, &V, 1);
	V |= BIT2; V |= BIT3;
	V |= BIT5;
	i2c_write_data(app->i2c0, PCA_9555_SLAVE_1, PCA_9555_OUTPUT_0, &V, 1);

	//ctx.pin_index = MK_GPIO_INDEX(0, 0, 10, 21);
	ctx.pin_index = MK_GPIO_INDEX(0, 0, 9, 20);

	ctx.value = GPIO_PULL_FLOATING; ctx.debounce = 0;
	ioctl(app->gpio, GPIO_IOCTL_SET_INPUT, &ctx);

	if (-1 == ioctl(app->adc, IOCTL_GP_ADC_START, 1))
		return;
	if (-1 == read(app->adc, &value[0], sizeof(int)))
		return;
	if (-1 == ioctl(app->adc, IOCTL_GP_ADC_STOP, 1))
		return;

	if (-1 == ioctl(app->adc, IOCTL_GP_ADC_START, 1))
		return;
	if (-1 == read(app->adc, &value[1], sizeof(int)))
		return;
	if (-1 == ioctl(app->adc, IOCTL_GP_ADC_STOP, 1))
		return;

	C_AD = ((value[0] + value[1]) / 2) >> 2;
	app->SetTimer(FPC_ADJUST_RESIST_MOTOR_ID, 3, AdjustMotorTimer);
	return;
}

void SpuTimer(UINT nId)
{
	CQsApp *app = (CQsApp *)AfxGetApp();
	unsigned char V = 0;
	gpio_content_t ctx;


/////////////////////////////////////////////////
	if (!prob_pca955_1)
	{
		goto again;
	}

/////////////////////////////////////////////////
	if (2 == product_type)
	{
		if (-1 == i2c_read_data(app->i2c0, PCA_9555_SLAVE_1, PCA_9555_INPUT_1, &V, 1))
		{
			printf("(%s %d) i2c err\n", __FILE__, __LINE__);
			goto again;
		}
		if (V & BIT6)
		{
			ctx.pin_index = MK_GPIO_INDEX(2, 0, 62, 19);
			ctx.value = IPOD_Detect = 1;
			app->rt->ipod_in_duck = 1;
			//ioctl(app->gpio, GPIO_IOCTL_SET_VALUE, &ctx);
		}
		else
		{
			ctx.pin_index = MK_GPIO_INDEX(2, 0, 62, 19);
			ctx.value = IPOD_Detect = 0;
			app->rt->ipod_in_duck = 0;
			//ioctl(app->gpio, GPIO_IOCTL_SET_VALUE, &ctx);
		}
	}
	else if (0 == product_type || 1 == product_type)
	{
		if (-1 == i2c_read_data(app->i2c0, PCA_9555_SLAVE_1, PCA_9555_INPUT_0, &V, 1))
		{
			printf("(%s %d) i2c err\n", __FILE__, __LINE__);
			goto again;
		}
		if (V & BIT4)
		{
			IPOD_Detect = 1;
			app->rt->ipod_in_duck = 1;
			//V |= BIT7;					// AUDIO_SEL0
			V |= BIT5;
			//i2c_write_data(app->i2c0, PCA_9555_SLAVE_1, PCA_9555_OUTPUT_0, &V, 1);
		}
		else
		{
			IPOD_Detect = 0;
			app->rt->ipod_in_duck = 0;
			//V &= ~BIT7;				// AUDIO_SEL
			V |= BIT5;
			//i2c_write_data(app->i2c0, PCA_9555_SLAVE_1, PCA_9555_OUTPUT_0, &V, 1);
		}
	}


/////////////////////////////////////////////////
again:
	app->Process_SPU();

	if (0 == app->SetTimer(FPC_SCHEDULER_200MS_ID, 151, SpuTimer))	 {}
}


void PollHr(void)
{
	CQsApp *app = (CQsApp *)AfxGetApp();

	static int OLD_PIN = -1;
	static unsigned int lastT = 0;
	static unsigned int lastH = 0;
	static int LAST_T = 0;
	static int LAST_H = 0;
	static unsigned int NVT = 0;
	static unsigned int NVH = 0;
	static int TEL_FIRST = 0;
	static unsigned int pulseT[4] = {0,0,0,0};
	static unsigned int pulseH[4] = {0,0,0,0};
	struct timeb tb;
	unsigned int now;
	unsigned int delta = 0;
	static unsigned int old = 0;

	int D0, D1, D2;

/////////////////////////////
	if (0 == product_type || 1 == product_type)
	{
		if (0 != app->hr->PCA9555_hardware_read())
		{
			printf("(%s %d) PCA9555_hardware_read() FAIL \n", __FILE__, __LINE__);
			return;
		}

		// chuck modify
		
		
		app->update->Heart_rate = app->hr->DisplayHeartRate;
		return;


		
	}
	if (2 == product_type)
	{
		gpio_content_t ctx;

		if (-1 != app->gpio)
		{
			ctx.pin_index = MK_GPIO_INDEX(0, 0, 11, 25);
			ctx.value = GPIO_PULL_FLOATING; ctx.debounce = 0;
			if (-1 == ioctl(app->gpio, GPIO_IOCTL_SET_INPUT, &ctx)) { printf("(%s %d) GPIO_IOCTL_SET_INPUT FAIL !! \n", __FILE__, __LINE__); return; }
			if (-1 == ioctl(app->gpio, GPIO_IOCTL_GET_VALUE, &ctx)) { printf("(%s %d) GPIO_IOCTL_GET_VALUE FAIL !! \n", __FILE__, __LINE__); return; }
			app->hr->TEL_PIN = ctx.value;

			ctx.pin_index = MK_GPIO_INDEX(5, 0, 58, 5);
			ctx.value = GPIO_PULL_FLOATING; ctx.debounce = 0;
			if (-1 == ioctl(app->gpio, GPIO_IOCTL_SET_INPUT, &ctx)) { printf("(%s %d) GPIO_IOCTL_SET_INPUT FAIL !! \n", __FILE__, __LINE__); return; }
			if (-1 == ioctl(app->gpio, GPIO_IOCTL_GET_VALUE, &ctx)) { printf("(%s %d) GPIO_IOCTL_GET_VALUE FAIL !! \n", __FILE__, __LINE__); return; }
			app->hr->HGP_PIN = ctx.value;
		}

		if (prob_pca955_1)
		{
			unsigned char V = 0;
			if (-1 == i2c_read_data(app->i2c0, PCA_9555_SLAVE_1, PCA_9555_INPUT_1, &V, 1)) { return; }
			if (V & BIT7)					SPU_PIN = 1;
			else							SPU_PIN = 0;
		}
	}


/////////////////////////////
	ftime(&tb); now = (tb.time * 1000 + tb.millitm);
	delta = now - old;
	old = now;

// normal lo
	if (0 == LAST_T && 1 == app->hr->TEL_PIN)
	{
		TEL_FIRST = 1;

		pulseT[3] = pulseT[2]; pulseT[2] = pulseT[1]; pulseT[1] = pulseT[0];
		pulseT[0] = now;

		if (pulseT[0] > pulseT[3])
		{
			D0 = (int)(pulseT[0] - pulseT[1]);
			D1 = (int)(pulseT[1] - pulseT[2]);
			D2 = (int)(pulseT[2] - pulseT[3]);
			if (D0 -D1 <= 210 && D0 -D1 >= -210 && D1 -D2 <= 210 && D1 -D2 >= -210)
				NVT = (3 * 60 * 1000)  / (pulseT[0] - pulseT[3]);
		}

		lastT = now;
	}
	if ((0 == LAST_T && 0 == app->hr->TEL_PIN) || (1 == LAST_T && 1 == app->hr->TEL_PIN))
	{
		if (now - lastT >= 3000)
		{
			TEL_FIRST = 0;
			pulseT[3] = pulseT[2] = pulseT[1] = pulseT[0] = 0;
			NVT = 0;
		}
	}
	LAST_T = app->hr->TEL_PIN;


/////////////////////////////
// normal hi
	if (0 == LAST_H && 1 == app->hr->HGP_PIN)
	{
		pulseH[3] = pulseH[2]; pulseH[2] = pulseH[1]; pulseH[1] = pulseH[0];
		pulseH[0] = now;

		if (pulseH[0] > pulseH[3])
		{
			D0 = (int)(pulseH[0] - pulseH[1]);
			D1 = (int)(pulseH[1] - pulseH[2]);
			D2 = (int)(pulseH[2] - pulseH[3]);
			if (D0 -D1 <= 210 && D0 -D1 >= -210 && D1 -D2 <= 210 && D1 -D2 >= -210)
				NVH = 3 * 60 * 1000  / (pulseH[0] - pulseH[3]);
		}

		lastH = now;
	}
	if ((1 == LAST_H && 1 == app->hr->HGP_PIN) || (0 == LAST_H && 0 == app->hr->HGP_PIN))
	{
		if (now - lastH >= 3000)
		{
			pulseH[3] = pulseH[2] = pulseH[1] = pulseH[0] = 0;
			NVH = 0;
		}
	}
	LAST_H = app->hr->HGP_PIN;


/////////////////////////////
	if (1 == TEL_FIRST)					app->hr->DisplayHeartRate = NVT;
	else									app->hr->DisplayHeartRate = NVH;
	if (app->hr->DisplayHeartRate < 40)		app->hr->DisplayHeartRate = 0;
	if (app->hr->DisplayHeartRate > 220)	app->hr->DisplayHeartRate = 220;


//printf("app->hr->TEL_PIN=%d\n", app->hr->TEL_PIN);


/////////////////////////////
	if (1)
	{
		static unsigned int counter = 0;

		if (0 == (counter % 300))
		{

#if 0
			static int S = -1;
			static int N = -1;
			static int C = 0;

			N = (int)app->hr->DisplayHeartRate;
			if (-1 == S)
			{
				app->update->Heart_rate = app->hr->DisplayHeartRate;
				C = 0;
			}
			else if (0 == app->hr->DisplayHeartRate)
			{
				app->update->Heart_rate = 0;
				C = 0;
			}
			else
			{
				if (N -S < 5 && N -S > -5)
				{
					app->update->Heart_rate = app->hr->DisplayHeartRate;
					C = 0;
				}
				else
				{
					if (C > 2)
					{
						app->update->Heart_rate = app->hr->DisplayHeartRate;
						C = 0;
					}
				}
				C++;
			}

			S = (int)app->hr->DisplayHeartRate;
#else
			app->update->Heart_rate = app->hr->DisplayHeartRate;
#endif // 0


//printf("DisplayHeartRate=%d\n", app->update->Heart_rate);


		}

		counter++;
	}


/////////////////////////////////////////////////
	if (2 == product_type || 1 == product_type)
	{
		static unsigned int OLD_SPU_TIME = 0;
		unsigned int NOW_SPU_TIME;
		static unsigned int pulseS[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

		ftime(&tb); NOW_SPU_TIME = (tb.time * 1000 + tb.millitm);
		if (1 == OLD_PIN && 0 == SPU_PIN)
		{
			if (2 == product_type)
			{
				pulseS[31] = pulseS[30];
				pulseS[30] = pulseS[29];
				pulseS[29] = pulseS[28];
				pulseS[28] = pulseS[27];
				pulseS[27] = pulseS[26];
				pulseS[26] = pulseS[25];
				pulseS[25] = pulseS[24];
				pulseS[24] = pulseS[23];
				pulseS[23] = pulseS[22];
				pulseS[22] = pulseS[21];
				pulseS[21] = pulseS[20];
				pulseS[20] = pulseS[19];
				pulseS[19] = pulseS[18];
				pulseS[18] = pulseS[17];
				pulseS[17] = pulseS[16];
				pulseS[16] = pulseS[15];
				pulseS[15] = pulseS[14];
				pulseS[14] = pulseS[13];
				pulseS[13] = pulseS[12];
				pulseS[12] = pulseS[11];
				pulseS[11] = pulseS[10];
				pulseS[10] = pulseS[9];
				pulseS[9] = pulseS[8];
				pulseS[8] = pulseS[7];
			}
			pulseS[7] = pulseS[6];
			pulseS[6] = pulseS[5];
			pulseS[5] = pulseS[4];
			pulseS[4] = pulseS[3];
			pulseS[3] = pulseS[2];
			pulseS[2] = pulseS[1];
			pulseS[1] = pulseS[0];
			pulseS[0] = NOW_SPU_TIME;
			if (pulseS[0] > pulseS[1])
			{
				unsigned int min = 300000;
				if (pulseS[0] - pulseS[1] < min)			min = pulseS[0] - pulseS[1];
				if (pulseS[1] - pulseS[2] < min)			min = pulseS[1] - pulseS[2];
				if (pulseS[2] - pulseS[3] < min)			min = pulseS[2] - pulseS[3];
				if (pulseS[3] - pulseS[4] < min)			min = pulseS[3] - pulseS[4];
				if (pulseS[4] - pulseS[5] < min)			min = pulseS[4] - pulseS[5];
				if (pulseS[5] - pulseS[6] < min)			min = pulseS[5] - pulseS[6];
				if (pulseS[6] - pulseS[7] < min)			min = pulseS[6] - pulseS[7];
				if (2 == product_type)
				{
					if (pulseS[7] - pulseS[8] < min)		min = pulseS[7] - pulseS[8];
					if (pulseS[8] - pulseS[9] < min)		min = pulseS[8] - pulseS[9];
					if (pulseS[9] - pulseS[10] < min)		min = pulseS[9] - pulseS[10];
					if (pulseS[10] - pulseS[11] < min)	min = pulseS[10] - pulseS[11];
					if (pulseS[11] - pulseS[12] < min)	min = pulseS[11] - pulseS[12];
					if (pulseS[12] - pulseS[13] < min)	min = pulseS[12] - pulseS[13];
					if (pulseS[13] - pulseS[14] < min)	min = pulseS[13] - pulseS[14];
					if (pulseS[14] - pulseS[15] < min)	min = pulseS[14] - pulseS[15];
					if (pulseS[15] - pulseS[16] < min)	min = pulseS[15] - pulseS[16];
					if (pulseS[16] - pulseS[17] < min)	min = pulseS[16] - pulseS[17];
					if (pulseS[17] - pulseS[18] < min)	min = pulseS[17] - pulseS[18];
					if (pulseS[18] - pulseS[19] < min)	min = pulseS[18] - pulseS[19];
					if (pulseS[19] - pulseS[20] < min)	min = pulseS[19] - pulseS[20];
					if (pulseS[20] - pulseS[21] < min)	min = pulseS[20] - pulseS[21];
					if (pulseS[21] - pulseS[22] < min)	min = pulseS[21] - pulseS[22];
					if (pulseS[22] - pulseS[23] < min)	min = pulseS[22] - pulseS[23];
					if (pulseS[23] - pulseS[24] < min)	min = pulseS[23] - pulseS[24];
					if (pulseS[24] - pulseS[25] < min)	min = pulseS[24] - pulseS[25];
					if (pulseS[25] - pulseS[26] < min)	min = pulseS[25] - pulseS[26];
					if (pulseS[26] - pulseS[27] < min)	min = pulseS[26] - pulseS[27];
					if (pulseS[27] - pulseS[28] < min)	min = pulseS[27] - pulseS[28];
					if (pulseS[28] - pulseS[29] < min)	min = pulseS[28] - pulseS[29];
					if (pulseS[29] - pulseS[30] < min)	min = pulseS[29] - pulseS[30];
					if (pulseS[30] - pulseS[31] < min)	min = pulseS[30] - pulseS[31];
				}
				if (min > 0 && min != 300000)
				{
					app->data->Drive_SPU_Interval = min;
					app->data->New_SPU_Pulse = 1;
				}
			}
			OLD_SPU_TIME = NOW_SPU_TIME;
		}
		if ((0 == OLD_PIN && 0 == SPU_PIN) || (1 == OLD_PIN && 1 == SPU_PIN))
		{
			if (NOW_SPU_TIME - OLD_SPU_TIME >= 7000)
				memset(pulseS, 0, sizeof(pulseS));
		}

		OLD_PIN = SPU_PIN;
	}

}


void HrTimer(UINT nId)
{
	CQsApp *app = (CQsApp *)AfxGetApp();

/////////////////////////////////////////////////
	PollHr();
	if (0 == product_type)
	{
		if (0 == app->SetTimer(FPC_SCHEDULER_1MS_ID, 100, HrTimer)) { }
		return;
	}

	if (0 == app->SetTimer(FPC_SCHEDULER_1MS_ID, 1, HrTimer)) { }
	return;
}


void *KsThread(void *param)
{
	sigset_t nset, oset;
	struct timespec request, remain;
	int s;

	sigemptyset(&nset);
	sigaddset(&nset, SIGHUP), sigaddset(&nset, SIGINT), sigaddset(&nset, SIGTERM), sigaddset(&nset, SIGUSR1), sigaddset(&nset, SIGALRM);
	pthread_sigmask(SIG_BLOCK, &nset, &oset);
	pthread_cleanup_push(RestoreSigmask, (void *)&oset);

	for(;;)
	{
		static unsigned char I = 0;

		memset(&request, 0, sizeof(request));
		if (0 != clock_gettime(CLOCK_REALTIME, &request))
		{
			printf("clock_gettime() FAIL \n");
			sched_yield();
			continue;
		}
		request.tv_nsec += (13 * 1000 * 1000);
		if (request.tv_nsec >= 1000000000)
			request.tv_sec += (request.tv_nsec / 1000000000), request.tv_nsec %= 1000000000;

		PollKeyScan(I);
		I++;
		I %= 7;

		s = clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &request, &remain);
		if (s != 0 && s != EINTR)	printf("clock_nanosleep() FAIL \n");
		if (s == EINTR)			printf("TimerThread() Interrupted...\n");
	}

	pthread_cleanup_pop(0);

	return 0;
}


void PollHr(void);

void *HrThread(void *param)
{
	sigset_t nset, oset;
	struct timespec request, remain;
	int s;


///////////////////////
	sigemptyset(&nset);
	sigaddset(&nset, SIGHUP), sigaddset(&nset, SIGINT), sigaddset(&nset, SIGTERM), sigaddset(&nset, SIGUSR1), sigaddset(&nset, SIGALRM);
	pthread_sigmask(SIG_BLOCK, &nset, &oset);
	pthread_cleanup_push(RestoreSigmask, (void *)&oset);


	for(;;)
	{
		memset(&request, 0, sizeof(request));
		if (0 != clock_gettime(CLOCK_REALTIME, &request))
		{
			printf("clock_gettime() FAIL \n");
			sched_yield();
			continue;
		}
		request.tv_nsec += (1 * 1000 * 1000);
		if (request.tv_nsec >= 1000000000)
			request.tv_sec += (request.tv_nsec / 1000000000), request.tv_nsec %= 1000000000;

		//app->hr->PCA9555_hardware_read();
		//app->hr->HeartRate_Sampling_HGP();
		//app->hr->HeartRate_Sampling_TEL();
		PollHr();

		s = clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &request, &remain);
		if (s != 0 && s != EINTR)	printf("clock_nanosleep() FAIL \n");
		if (s == EINTR)			printf("TimerThread() Interrupted...\n");
	}


///////////////////////
	pthread_cleanup_pop(0);

dbg_printf(DBG_THREAD, "(%s %d) HrThread() EXIT\n", __FILE__, __LINE__);
	return 0;
}


void *TickThread(void *param)
{
	sigset_t nset, oset;
	CQsApp *app = (CQsApp *)AfxGetApp();
	long m, n, a;
	static long o = 0;
	struct timespec tv;


///////////////////////
	sigemptyset(&nset);
	sigaddset(&nset, SIGHUP), sigaddset(&nset, SIGINT), sigaddset(&nset, SIGTERM), sigaddset(&nset, SIGUSR1), sigaddset(&nset, SIGALRM);
	pthread_sigmask(SIG_BLOCK, &nset, &oset);
	pthread_cleanup_push(RestoreSigmask, (void *)&oset);


///////////////////////
	for(;;)
	{
		memset(&tv, 0, sizeof(tv));
		if (0 != clock_gettime(CLOCK_REALTIME, &tv))
		{
	printf("(%s %d) clock_gettime() fail\n", __FILE__, __LINE__);
			sched_yield();
			continue;
		}

		a = (tv.tv_sec * 1000000000) + tv.tv_nsec;
		m = (a % 100000000);
		n = (a - m) / 100000000;
		if (n != o)
		{
			o = n;
			SetEvent(app->hFpcTick);
		}
		usleep(999 * 30);
	}
///////////////////////


///////////////////////
	pthread_cleanup_pop(0);

dbg_printf(DBG_THREAD, "(%s %d) TickThread() EXIT\n", __FILE__, __LINE__);
	return 0;
}


void *FpcThread(void *param)
{
	unsigned int res;
	CQsApp *app = (CQsApp *)param;
	HANDLE hFpcStart = app->hFpcStart;
	HANDLE h[] = {hFpcStart, 0, 0};

	for (;;)
	{
		res = WaitForMultipleObjects(1, h, WAIT_FOR_ANY_ONE_EVENTS, INFINITE);
		if (WAIT_FAILED == res) { dbg_printf(DBG_BASIC, "(%s %d) WaitForMultipleObjects() >  WAIT_FAILED \n", __FILE__, __LINE__); exit(1); }
		if (WAIT_OBJECT_0 + 0 == res)
		{
			app->ProgramStart();
		}
	}
}

static pthread_mutex_t xUatrt = PTHREAD_MUTEX_INITIALIZER;
static struct incoming_t uartQ;

void *UartThread(void *param)
{
	CQsApp *app = (CQsApp *)param;
	HANDLE hUart = app->hUart;
	int res = 0;
	struct list_head *P, *Q;
	struct incoming_t *N;
	int fd;
	int maxfd;
	fd_set rset;
	int C;
	int len;
	sigset_t nset, oset;
	unsigned char buff[1024 * 4];

	unsigned int ret;
	unsigned char *p;
	struct timeval tv;
	int last_type;

	sigemptyset(&nset);
	sigaddset(&nset, SIGHUP), sigaddset(&nset, SIGINT), sigaddset(&nset, SIGTERM), sigaddset(&nset, SIGUSR1), sigaddset(&nset, SIGALRM);
	pthread_sigmask(SIG_BLOCK, &nset, &oset);
	pthread_cleanup_push(RestoreSigmask, (void *)&oset);

WAIT:
	hUart = app->hUart;
	if(0 == hUart)														{ dbg_printf(DBG_EVENT, "(%s %d) 0 == hUart\n", __FILE__, __LINE__); goto BAITOUT; }
	ret = WaitForSingleObject(hUart, INFINITE);
	if(WAIT_FAILED == ret)											{ dbg_printf(DBG_EVENT, "(%s %d) WaitForSingleObject() WAIT_FAILED\n", __FILE__, __LINE__); goto BAITOUT; }
	if(WAIT_TIMEOUT == ret)											{ dbg_printf(DBG_EVENT, "(%s %d) WaitForSingleObject() WAIT_TIMEOUT\n", __FILE__, __LINE__); goto WAIT; }

	for(;;)
	{
		pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &last_type);
		pthread_mutex_lock(&xUatrt);
		pthread_cleanup_push(CleanupLock, (void *)&xUatrt);
		pthread_setcanceltype(last_type, NULL);
		maxfd = 0; C = 0;
		FD_ZERO(&rset);
		list_for_each_safe(P, Q, &uartQ.list)
		{
			 C++; N = list_entry(P, struct incoming_t, list); assert(0 != N); fd = N->fd; FD_SET(fd, &rset);
			if (maxfd < fd)											maxfd = fd;
		}
		pthread_cleanup_pop(1);
		if (0 == C)
		{
			dbg_printf(DBG_UART, "(%s %d) ResetEvent(hUart) goto WAIT; \n", __FILE__, __LINE__);
			ResetEvent(hUart);
			goto WAIT;
		}

		tv.tv_sec = 2; tv.tv_usec = 10000;
		if (0 > (res = select(maxfd + 1, &rset, 0, 0, &tv)))
		{
			dbg_printf(DBG_UART, "(%s %d) SELECT() FAIL\n", __FILE__, __LINE__); 
			AfxGetApp()->PostMessage(M_EXIT_INSTANCE, 0, 0);
		}
		else if (0 == res)												{ continue; }
		else
		{
			pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &last_type);
			pthread_mutex_lock(&xUatrt);
			pthread_cleanup_push(CleanupLock, (void *)&xUatrt);
			pthread_setcanceltype(last_type, NULL);

			list_for_each_safe(P, Q, &uartQ.list)
			{
				N = list_entry(P, struct incoming_t, list); assert(0 != N); fd = N->fd;
				if (FD_ISSET(fd, &rset))
				{
					memset(buff, 0, sizeof(buff));
					len = read(fd, buff, sizeof(buff));
					if (len < 0)
					{
						if (errno == EAGAIN)							{ dbg_printf(DBG_UART, "(%s %d) EAGAIN\n", __FILE__, __LINE__); continue; }
						if (errno == EWOULDBLOCK)					{ dbg_printf(DBG_UART, "(%s %d) EWOULDBLOCK\n", __FILE__, __LINE__); continue; }
						if (errno == EBADF)							{ dbg_printf(DBG_UART, "(%s %d) EBADF\n", __FILE__, __LINE__); close(fd); list_del(P); free(N); continue; }
						dbg_printf(DBG_UART, "(%s %d) len < 0, errno=%d\n", __FILE__, __LINE__, errno); AfxGetApp()->PostMessage(M_EXIT_INSTANCE, 0, 0); close(fd); list_del(P); free(N); continue;
					}
					if (len == 0)										{ dbg_printf(DBG_UART, "(%s %d) len== 0\n", __FILE__, __LINE__); close(fd); list_del(P); free(N); continue; }
					p = (unsigned char *)malloc(64 + len + 32);
					if (p)
					{
						CQsApp *app = 0;

						memset(p, 0, 64 + len);
						memcpy(p, &len, sizeof(int));
						strcpy((char *)p + sizeof(int), (char *)N->name);
						memcpy(p + 64, buff, len);

#if 0
						app->PostMessage(M_UART_DATA_RCV, (WPARAM)p, N->fd);
#else
						app = (CQsApp *)AfxGetApp();
						app->OnUartDataRcv(M_UART_DATA_RCV, (WPARAM)p, N->fd, 0);
#endif

					}
					else
					{ dbg_printf(DBG_MALLOC, "(%s %d) MALLOC() FAIL\n", __FILE__, __LINE__); }
				}
			}

			pthread_cleanup_pop(1);


		}
	}


BAITOUT:
	pthread_cleanup_pop(1);

dbg_printf(DBG_THREAD, "(%s %d) UartThread() EXIT\n", __FILE__, __LINE__); 
	return 0;
}


unsigned char key_scan[64];
static pthread_mutex_t xF = PTHREAD_MUTEX_INITIALIZER;
static struct incoming_t fpcCliQ;


static void CleanupIncoming(void *param)
{
	struct incoming_t *pQ = (struct incoming_t *)param;
	struct list_head *P, *Q;
	struct incoming_t *N;

	list_for_each_safe(P, Q, &pQ->list)
	{
		N = list_entry(P, struct incoming_t, list); assert(0 != N); list_del(P); free(N);
	}
}

void *UiThread(void *param)
{
	struct list_head *P, *Q;
	struct incoming_t *N;
	int fd;
	int maxfd = 0;
	int lsd = -1, res, v = 0;
	struct sockaddr_in servaddr;
	fd_set rset;
	sigset_t nset, oset;

	unsigned short LEN = 0;
	int len = 0;
	unsigned char *frame;

	unsigned char *p;
	int left = 1, offs = 0;
	int last_type;

	if (-1 == (lsd = socket(AF_INET, SOCK_STREAM, 0)))					{ dbg_printf(DBG_SOCKET, "(%s %d) SOCKET() FAIL\n", __FILE__, __LINE__); AfxGetApp()->PostMessage(M_EXIT_INSTANCE, 0, 0); return 0; }
	v = 1, setsockopt(lsd, SOL_SOCKET, SO_REUSEADDR, (const void *)&v, sizeof(v));
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET, servaddr.sin_addr.s_addr = htonl(INADDR_ANY), servaddr.sin_port = htons(8366);
	if (0 > bind(lsd, (struct sockaddr *)&servaddr, sizeof(struct sockaddr_in)))	{ dbg_printf(DBG_SOCKET, "(%s %d) BIND() fail\n", __FILE__, __LINE__); AfxGetApp()->PostMessage(M_EXIT_INSTANCE, 0, 0); return 0; }
	if (0 > listen(lsd, 12))												{ dbg_printf(DBG_SOCKET, "(%s %d) LISTEN() fail\n", __FILE__, __LINE__); AfxGetApp()->PostMessage(M_EXIT_INSTANCE, 0, 0); return 0; }
	v = fcntl(lsd, F_GETFL, 0); fcntl(lsd, F_SETFL, v | O_NONBLOCK);
	v = (512 * 1024); setsockopt(lsd, SOL_SOCKET, SO_SNDBUF, (const void *)&v, sizeof(v)); setsockopt(lsd, SOL_SOCKET, SO_RCVBUF, (const void *)&v, sizeof(v));

	pthread_cleanup_push(CleanupFd, (void *)&lsd);

	sigemptyset(&nset);
	sigaddset(&nset, SIGHUP), sigaddset(&nset, SIGINT), sigaddset(&nset, SIGTERM), sigaddset(&nset, SIGUSR1), sigaddset(&nset, SIGALRM);
	pthread_sigmask(SIG_BLOCK, &nset, &oset);
	pthread_cleanup_push(RestoreSigmask, (void *)&oset);

	pthread_cleanup_push(CleanupIncoming, (void *)&fpcCliQ);

	for (;;)
	{
		int clilen, connfd;
		struct sockaddr_in cliaddr;

		maxfd = 0; FD_ZERO(&rset); FD_SET(lsd, &rset);
		if (maxfd < lsd)												maxfd = lsd;

		pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &last_type);
		pthread_mutex_lock(&xF);
		pthread_cleanup_push(CleanupLock, (void *)&xF);
		pthread_setcanceltype(last_type, NULL);

		list_for_each_safe(P, Q, &fpcCliQ.list)
		{
			N = list_entry(P, struct incoming_t, list); assert(0 != N); fd = N->fd; FD_SET(fd, &rset);
			if (maxfd < fd)											maxfd = fd;
		}
		pthread_cleanup_pop(1);

		if (0 > (res = select(maxfd + 1, &rset, 0, 0, 0)))					{ dbg_printf(DBG_SOCKET, "(%s %d) SELECT() FAIL\n", __FILE__, __LINE__); AfxGetApp()->PostMessage(M_EXIT_INSTANCE, 0, 0); break; }
		if (0 == res)													{ dbg_printf(DBG_SOCKET, "(%s %d) SELECT() TIMEOUT\n", __FILE__, __LINE__); continue; }

		if (FD_ISSET(lsd, &rset))
		{
			clilen = sizeof(cliaddr);
			connfd = accept(lsd, (struct sockaddr *)&cliaddr, (socklen_t *)&clilen);
			if (-1 != connfd)
			{
				v = fcntl(connfd, F_GETFL, 0), fcntl(connfd, F_SETFL, v | O_NONBLOCK);
				v = (512 * 1024), setsockopt(connfd, SOL_SOCKET, SO_SNDBUF, (const void *)&v, sizeof(v)), setsockopt(connfd, SOL_SOCKET, SO_RCVBUF, (const void *)&v, sizeof(v));
				if (0 != (N = (struct incoming_t *)malloc(sizeof(struct incoming_t) + 1024 * 4)))
				{
					dbg_printf(DBG_SOCKET, "(%s %d) ACCEPT()\n", __FILE__, __LINE__);

					memset(N, 0, sizeof(struct incoming_t)); N->fd = connfd, memcpy(&N->cliaddr, &cliaddr, clilen); N->left = 1; N->offs = 0;

					pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &last_type);
					pthread_mutex_lock(&xF);
					pthread_cleanup_push(CleanupLock, (void *)&xF);
					pthread_setcanceltype(last_type, NULL);
					list_add_tail(&N->list, &fpcCliQ.list);
					pthread_cleanup_pop(1);
				}
				else													{ dbg_printf(DBG_MALLOC, "(%s %d) MALLOC() FAIL\n", __FILE__, __LINE__); close(connfd); }
			}
			else { dbg_printf(DBG_MALLOC, "(%s %d) ACCEPT() FAIL\n", __FILE__, __LINE__); }
			continue;
		}


		pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &last_type);
		pthread_mutex_lock(&xF);
		pthread_cleanup_push(CleanupLock, (void *)&xF);
		pthread_setcanceltype(last_type, NULL);

		list_for_each_safe(P, Q, &fpcCliQ.list)
		{
			N = list_entry(P, struct incoming_t, list); assert(0 != N);
			fd = N->fd;
			if (!FD_ISSET(fd, &rset))									{ continue; }

			left = N->left, offs = N->offs;
			p = N->msg + offs;
			if (0 == offs || 1 == offs || 2 == offs || 3 == offs)				left = 1;
			if (4 == offs || 5 == offs)									left = 1;
			if (6 == offs)
			{
				LEN = BUILD_UINT16(N->msg[5], N->msg[4]);
				if (LEN > 1000)
				{
					dbg_printf(DBG_SOCKET, "(%s %d) PROTOCOL MISMACH\n", __FILE__, __LINE__);
					N->offs = 0; N->left = 1;
					continue;
				}
				else
				{
					left = N->left = (LEN + 2);
				}
			}

			if (0 > (len = read(fd, p, left)))
			{
				if (errno == EAGAIN)									{ dbg_printf(DBG_SOCKET, "(%s %d) READ() EAGAIN\n", __FILE__, __LINE__); continue;}
				if (errno == EWOULDBLOCK)							{ dbg_printf(DBG_SOCKET, "(%s %d) READ() EWOULDBLOCK\n", __FILE__, __LINE__); continue;}
				if (errno == ECONNRESET || errno == EBADF)				{ dbg_printf(DBG_SOCKET, "(%s %d) READ() ECONNRESET\n", __FILE__, __LINE__); close(fd); list_del(P); free(N); continue; }

				dbg_printf(DBG_SOCKET, "(%s %d) READ() FAIL, errno=%d\n", __FILE__, __LINE__, errno); AfxGetApp()->PostMessage(M_EXIT_INSTANCE, 0, 0); close(fd); list_del(P); free(N); break;
			}
			if (0 == len)												{ dbg_printf(DBG_SOCKET, "(%s %d) READ() EOF\n", __FILE__, __LINE__); close(fd); list_del(P); free(N); continue; }


			if (0 == offs && 0X05 != N->msg[0])							{ dbg_printf(DBG_SOCKET, "(%s %d) PROTOCOL MISMACH\n", __FILE__, __LINE__); N->offs = 0; N->left = 1; continue; }
			if (1 == offs && 0X05 != N->msg[1])							{ dbg_printf(DBG_SOCKET, "(%s %d) PROTOCOL MISMACH\n", __FILE__, __LINE__); N->offs = 0; N->left = 1; continue; }

			offs += len, N->offs = offs;
			left -= len, N->left = left;
			if (offs >= 8 && left <= 0)
			{
				LEN = BUILD_UINT16(N->msg[5], N->msg[4]);
				if (0X50 == N->msg[LEN + 6] && 0X50 == N->msg[LEN + 7])
				{
					CQsApp *app = 0;

					v = LEN + 8;
					frame = (unsigned char *)malloc(LEN + 8 + 64 + 16);
					if (frame)
					{
						memcpy(frame, &v, sizeof(int));
						memcpy(frame + 64, N->msg, LEN + 8);

#if 0
						AfxGetApp()->PostMessage(M_UI_INCOMING, (WPARAM)frame, (LPARAM)fd);
#else
						app = (CQsApp *)AfxGetApp();
						app->OnUserMessage(M_UI_INCOMING, (WPARAM)frame, (LPARAM)fd, 0);
#endif //

					}
					else
					{
						dbg_printf(DBG_MALLOC, "(%s %d) MALLOC() FAIL\n", __FILE__, __LINE__);
					}
				}
				N->offs = 0; N->left = 1;
			}
			else
			{
			}
		}
		pthread_cleanup_pop(1);
	}
	pthread_cleanup_pop(1);
	pthread_cleanup_pop(1);
	pthread_cleanup_pop(1);

dbg_printf(DBG_THREAD, "(%s %d) UiThread() EXIT\n", __FILE__, __LINE__); 
	return 0;
}


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

int packOurProtocol(unsigned char ch, unsigned char cl, int len, const unsigned char *data, unsigned char *out)
{
	unsigned char p[1024 * 8];

	*(p + 0) = 0X05;
	*(p + 1) = 0X05;
	*(p + 2) = ch;
	*(p + 3) = cl;
	*(p + 4) = HI_UINT16(len);
	*(p + 5) = LO_UINT16(len);
	memcpy(p + 6, (void *)data, len);
	*(p + 6 + len) = 0X50;
	*(p + 7 + len) = 0X50;

	if (out)
		memcpy(out, (void *)p, len + 8);
	return (len + 8);
}

static unsigned char P1;

int PollKeyScan(unsigned char line)
{
	static int long_buzz_flag = 0;

	static unsigned char P0;
	unsigned char V = 0;

	int longPress = 0;
	int keyNum = 0;
	time_t now = 0;
	CQsApp *app = (CQsApp *)AfxGetApp();

	static unsigned char O0 = 0;
	static time_t T0 = 0;
	static unsigned char O1 = 0;
	static time_t T1 = 0;
	static unsigned char O2 = 0;
	static time_t T2 = 0;
	static unsigned char O3 = 0;
	static time_t T3 = 0;
	static unsigned char O4 = 0;
	static time_t T4 = 0;
	static unsigned char O5 = 0;
	static time_t T5 = 0;
	static unsigned char O6 = 0;
	static time_t T6 = 0;

	if (line >= 8)		return -1;
	memset(&P0, 0XFF, sizeof(P0));
	P0 &= ~(1<<line);

	if (-1 == i2c_write_data(app->i2c0, PCA_9555_SLAVE_0, PCA_9555_OUTPUT_0, (unsigned char *)&P0, 1))
	{
		printf("(%s %d) i2c_write_data() FAIL\n", __FILE__, __LINE__);
		return -1;
	}
	if (-1 == i2c_read_data(app->i2c0, PCA_9555_SLAVE_0, PCA_9555_INPUT_1, &P1, 1))
	{
		printf("(%s %d) i2c_read_data() FAIL\n", __FILE__, __LINE__);
		return -1;
	}


///////////////////////////////////////////////////////////
	if (0 == line)
	{
		if (0 == product_type || 1 == product_type)
		{
			if (-1 == i2c_read_data(app->i2c0, PCA_9555_SLAVE_1, PCA_9555_INPUT_1, &V, 1)) { return -1; }
			V &= ~BIT0; V |= BIT1; V |= BIT2; V |= BIT3;
			i2c_write_data(app->i2c0, PCA_9555_SLAVE_1, PCA_9555_OUTPUT_1, &V, 1);

			i2c_read_data(app->i2c0, PCA_9555_SLAVE_1, PCA_9555_INPUT_0, &V, 1);
			if (!(V & BIT0))
			{
				// LEFT-THUMB DOWN
				AfxGetApp()->PostMessage(M_LEFT_THUMB, 0, 0);
			}
		}
		/*if (2 == product_type)
		{
			if(-1 == i2c_read_data(app->i2c0, PCA_9555_SLAVE_1, PCA_9555_INPUT_0, &V, 1)) { return -1; }
			V &= ~BIT2; V |= BIT3; V |= BIT4;
			if (0 == product_type || 1 == product_type)
				V |= BIT5;
			i2c_write_data(app->i2c0, PCA_9555_SLAVE_1, PCA_9555_OUTPUT_0, &V, 1);

			i2c_read_data(app->i2c0, PCA_9555_SLAVE_1, PCA_9555_INPUT_0, &V, 1);
			if (!(V & BIT0))
			{
				// LEFT-THUMB DOWN
				AfxGetApp()->PostMessage(M_LEFT_THUMB, 0, 0);
			}
		}*/

		////////////
		if ((0XF9^BIT0) == P1 && 0XF9 == O0)
		{
			time(&T0);
			keyNum = 5;
			key_scan[keyNum] = 1;
			app->OnKeyScanPressed(M_KEY_SCAN_PRESSED, (WPARAM)keyNum, 0, 0);
		}
		if ((0XF9^BIT0) == P1 && (0XF9^BIT0) == O0)
		{
			time(&now);
			if (now - T0 >= 3)
			{
				if (0 == long_buzz_flag)
				{
					long_buzz_flag = 1;
					if (prob_pca955_1 && (0 == product_type || 1 == product_type || 2 == product_type))
						app->SetTimer(BUZZ_ON_TIMER_ID, 201, BuzzOnTimer);
				}
				keyNum = 5;
				key_scan[keyNum] = 2;
			}
		}
		if (0XF9 == P1 && (0XF9^BIT0) == O0)
		{
			keyNum = 5;
			time(&now);
			if (now - T0 >= 3)		longPress = 1, long_buzz_flag = 0;

#if 0
			AfxGetApp()->PostMessage(M_KEY_SCAN_RELEASED, (WPARAM)keyNum, (LPARAM)longPress);
#else
			app->OnKeyScanReleased(M_KEY_SCAN_RELEASED, (WPARAM)keyNum, (LPARAM)longPress, 0);
#endif

			key_scan[keyNum] = 0;
		}

		////////////?
		if ((0XF9^BIT3) == P1 && 0XF9 == O0)
		{
			time(&T0);
			keyNum = 7;
			key_scan[keyNum] = 1;
			app->OnKeyScanPressed(M_KEY_SCAN_PRESSED, (WPARAM)keyNum, 0, 0);
		}
		if ((0XF9^BIT3) == P1 && (0XF9^BIT3) == O0)
		{
			time(&now);
			if (now - T0 >= 3)
			{
				if (0 == long_buzz_flag)
				{
					long_buzz_flag = 1;
					if (prob_pca955_1 && (0 == product_type || 1 == product_type || 2 == product_type))
						app->SetTimer(BUZZ_ON_TIMER_ID, 201, BuzzOnTimer);
				}
				keyNum = 7;
				key_scan[keyNum] = 2;
			}
		}
		if (0XF9 == P1 && (0XF9^BIT3) == O0)
		{
			keyNum = 7;
			time(&now);
			if (now - T0 >= 3)		longPress = 1, long_buzz_flag = 0;
			app->OnKeyScanReleased(M_KEY_SCAN_RELEASED, (WPARAM)keyNum, (LPARAM)longPress, 0);
			key_scan[keyNum] = 0;
		}

		////////////
		if ((0XF9^BIT4) == P1 && 0XF9 == O0)
		{
			time(&T0);
			keyNum = 21;
			key_scan[keyNum] = 1;
			app->OnKeyScanPressed(M_KEY_SCAN_PRESSED, (WPARAM)keyNum, 0, 0);
		}
		if ((0XF9^BIT4) == P1 && (0XF9^BIT4) == O0)
		{
			time(&now);
			if (now - T0 >= 3)
			{
				if (0 == long_buzz_flag)
				{
					long_buzz_flag = 1;
					if (prob_pca955_1 && (0 == product_type || 1 == product_type || 2 == product_type))
						app->SetTimer(BUZZ_ON_TIMER_ID, 201, BuzzOnTimer);
				}
				keyNum = 21;
				key_scan[keyNum] = 2;
			}
		}
		if (0XF9 == P1 && (0XF9^BIT4) == O0)
		{
			keyNum = 21;
			time(&now);
			if (now - T0 >= 3)		longPress = 1, long_buzz_flag = 0;
			app->OnKeyScanReleased(M_KEY_SCAN_RELEASED, (WPARAM)keyNum, (LPARAM)longPress, 0);
			key_scan[keyNum] = 0;
		}


		////////////X
		if ((0XF9^BIT5) == P1 && 0XF9 == O0)
		{
			time(&T0);
			keyNum = 13;
			key_scan[keyNum] = 1;
			app->OnKeyScanPressed(M_KEY_SCAN_PRESSED, (WPARAM)keyNum, 0, 0);
		}
		if ((0XF9^BIT5) == P1 && (0XF9^BIT5) == O0)
		{
			time(&now);
			if (now - T0 >= 3)
			{
				if (0 == long_buzz_flag)
				{
					long_buzz_flag = 1;
					if (prob_pca955_1 && (0 == product_type || 1 == product_type || 2 == product_type))
						app->SetTimer(BUZZ_ON_TIMER_ID, 201, BuzzOnTimer);
				}
				keyNum = 13;
				key_scan[keyNum] = 2;
			}
		}
		if (0XF9 == P1 && (0XF9^BIT5) == O0)
		{
			keyNum = 13;
			time(&now);
			if (now - T0 >= 3)		longPress = 1, long_buzz_flag = 0;
			app->OnKeyScanReleased(M_KEY_SCAN_RELEASED, (WPARAM)keyNum, (LPARAM)longPress, 0);
			key_scan[keyNum] = 0;
		}

		O0 = P1;
	}


///////////////////////////////////////////////////////////
	if (1 == line)
	{
		if (0 == product_type || 1 == product_type)
		{
			i2c_read_data(app->i2c0, PCA_9555_SLAVE_1, PCA_9555_INPUT_1, &V, 1);
			V |= BIT0; V &= ~BIT1; V |= BIT2; V |= BIT3;
			i2c_write_data(app->i2c0, PCA_9555_SLAVE_1, PCA_9555_OUTPUT_1, &V, 1);

			i2c_read_data(app->i2c0, PCA_9555_SLAVE_1, PCA_9555_INPUT_0, &V, 1);
			if (!(V & BIT1))
			{
				// LEFT-THUMB UP
				AfxGetApp()->PostMessage(M_LEFT_THUMB, 1, 0);
			}
		}
		/*if (2 == product_type)
		{
			i2c_read_data(app->i2c0, PCA_9555_SLAVE_1, PCA_9555_INPUT_0, &V, 1);
			V |= BIT2; V &= ~BIT3; V |= BIT4;
			if (0 == product_type || 1 == product_type)
				V |= BIT5;
			i2c_write_data(app->i2c0, PCA_9555_SLAVE_1, PCA_9555_OUTPUT_0, &V, 1);

			i2c_read_data(app->i2c0, PCA_9555_SLAVE_1, PCA_9555_INPUT_0, &V, 1);
			if (!(V & BIT1))
			{
				// LEFT-THUMB UP
				AfxGetApp()->PostMessage(M_LEFT_THUMB, 1, 0);
			}
		}*/


		////////////
		if ((0XF9^BIT0) == P1 && 0XF9 == O1)
		{
			time(&T1);
			keyNum = 4;
			key_scan[keyNum] = 1;
			app->OnKeyScanPressed(M_KEY_SCAN_PRESSED, (WPARAM)keyNum, 0, 0);
		}
		if ((0XF9^BIT0) == P1 && (0XF9^BIT0) == O1)
		{
			time(&now);
			if (now - T1 >= 3)
			{
				if (0 == long_buzz_flag)
				{
					long_buzz_flag = 1;
					if (prob_pca955_1 && (0 == product_type || 1 == product_type || 2 == product_type))
						app->SetTimer(BUZZ_ON_TIMER_ID, 201, BuzzOnTimer);
				}
				keyNum = 4;
				key_scan[keyNum] = 2;
			}
		}
		if (0XF9 == P1 && (0XF9^BIT0) == O1)
		{
			keyNum = 4;
			time(&now);
			if (now - T1 >= 3)		longPress = 1, long_buzz_flag = 0;
			app->OnKeyScanReleased(M_KEY_SCAN_RELEASED, (WPARAM)keyNum, (LPARAM)longPress, 0);
			key_scan[keyNum] = 0;
		}

		////////////?
		if ((0XF9^BIT3) == P1 && 0XF9 == O1)
		{
			time(&T1);
			keyNum = 22;
			key_scan[keyNum] = 1;
			app->OnKeyScanPressed(M_KEY_SCAN_PRESSED, (WPARAM)keyNum, 0, 0);
		}
		if ((0XF9^BIT3) == P1 && (0XF9^BIT3) == O1)
		{
			time(&now);
			if (now - T1 >= 3)
			{
				if (0 == long_buzz_flag)
				{
					long_buzz_flag = 1;
					if (prob_pca955_1 && (0 == product_type || 1 == product_type || 2 == product_type))
						app->SetTimer(BUZZ_ON_TIMER_ID, 201, BuzzOnTimer);
				}
				keyNum = 22;
				key_scan[keyNum] = 2;
			}
		}
		if (0XF9 == P1 && (0XF9^BIT3) == O1)
		{
			keyNum = 22;
			time(&now);
			if (now - T1 >= 3)		longPress = 1, long_buzz_flag = 0;
			app->OnKeyScanReleased(M_KEY_SCAN_RELEASED, (WPARAM)keyNum, (LPARAM)longPress, 0);
			key_scan[keyNum] = 0;
		}

		////////////
		if ((0XF9^BIT4) == P1 && 0XF9 == O1)
		{
			time(&T1);
			keyNum = 17;
			key_scan[keyNum] = 1;
			app->OnKeyScanPressed(M_KEY_SCAN_PRESSED, (WPARAM)keyNum, 0, 0);
		}
		if ((0XF9^BIT4) == P1 && (0XF9^BIT4) == O1)
		{
			time(&now);
			if (now - T1 >= 3)
			{
				if (0 == long_buzz_flag)
				{
					long_buzz_flag = 1;
					if (prob_pca955_1 && (0 == product_type || 1 == product_type || 2 == product_type))
						app->SetTimer(BUZZ_ON_TIMER_ID, 201, BuzzOnTimer);
				}
				keyNum = 17;
				key_scan[keyNum] = 2;
			}
		}
		if (0XF9 == P1 && (0XF9^BIT4) == O1)
		{
			keyNum = 17;
			time(&now);
			if (now - T1 >= 3)		longPress = 1, long_buzz_flag = 0;
			app->OnKeyScanReleased(M_KEY_SCAN_RELEASED, (WPARAM)keyNum, (LPARAM)longPress, 0);
			key_scan[keyNum] = 0;
		}

		////////////X
		if ((0XF9^BIT5) == P1 && 0XF9 == O1)
		{
			time(&T1);
			keyNum = 14;
			key_scan[keyNum] = 1;
			app->OnKeyScanPressed(M_KEY_SCAN_PRESSED, (WPARAM)keyNum, 0, 0);
		}
		if ((0XF9^BIT5) == P1 && (0XF9^BIT5) == O1)
		{
			time(&now);
			if (now - T1 >= 3)
			{
				if (0 == long_buzz_flag)
				{
					long_buzz_flag = 1;
					if (prob_pca955_1 && (0 == product_type || 1 == product_type || 2 == product_type))
						app->SetTimer(BUZZ_ON_TIMER_ID, 201, BuzzOnTimer);
				}
				keyNum = 14;
				key_scan[keyNum] = 2;
			}
		}
		if (0XF9 == P1 && (0XF9^BIT5) == O1)
		{
			keyNum = 14;
			time(&now);
			if (now - T1 >= 3)		longPress = 1, long_buzz_flag = 0;
			app->OnKeyScanReleased(M_KEY_SCAN_RELEASED, (WPARAM)keyNum, (LPARAM)longPress, 0);
			key_scan[keyNum] = 0;
		}

		O1 = P1;
	}


///////////////////////////////////////////////////////////
	if (2 == line)
	{
		if (0 == product_type || 1 == product_type)
		{
			i2c_read_data(app->i2c0, PCA_9555_SLAVE_1, PCA_9555_INPUT_1, &V, 1);
			V |= BIT0; V |= BIT1; V &= ~BIT2; V |= BIT3;
			i2c_write_data(app->i2c0, PCA_9555_SLAVE_1, PCA_9555_OUTPUT_1, &V, 1);

			i2c_read_data(app->i2c0, PCA_9555_SLAVE_1, PCA_9555_INPUT_0, &V, 1);
			if (!(V & BIT0))
			{
				// RIGHT-THUMB DOWN
				AfxGetApp()->PostMessage(M_RIGHT_THUMB, 0, 0);
			}
		}
		/*if (2 == product_type)
		{
			i2c_read_data(app->i2c0, PCA_9555_SLAVE_1, PCA_9555_INPUT_0, &V, 1);
			V |= BIT2; V |= BIT3; V &= ~BIT4;
			if (0 == product_type || 1 == product_type)
				V |= BIT5;
			i2c_write_data(app->i2c0, PCA_9555_SLAVE_1, PCA_9555_OUTPUT_0, &V, 1);

			i2c_read_data(app->i2c0, PCA_9555_SLAVE_1, PCA_9555_INPUT_0, &V, 1);
			if (!(V & BIT0))
			{
				// RIGHT-THUMB DOWN
				AfxGetApp()->PostMessage(M_RIGHT_THUMB, 0, 0);
			}
		}*/


		////////////
		if ((0XF9^BIT0) == P1 && 0XF9 == O2)
		{
			time(&T2);
			keyNum = 1;
			key_scan[keyNum] = 1;
			app->OnKeyScanPressed(M_KEY_SCAN_PRESSED, (WPARAM)keyNum, 0, 0);
		}
		if ((0XF9^BIT0) == P1 && (0XF9^BIT0) == O2)
		{
			time(&now);
			if (now - T2 >= 3)
			{
				if (0 == long_buzz_flag)
				{
					long_buzz_flag = 1;
					if (prob_pca955_1 && (0 == product_type || 1 == product_type || 2 == product_type))
						app->SetTimer(BUZZ_ON_TIMER_ID, 201, BuzzOnTimer);
				}
				keyNum = 1;
				key_scan[keyNum] = 2;
			}
		}
		if (0XF9 == P1 && (0XF9^BIT0) == O2)
		{
			keyNum = 1;
			time(&now);
			if (now - T2 >= 3)		longPress = 1, long_buzz_flag = 0;
			app->OnKeyScanReleased(M_KEY_SCAN_RELEASED, (WPARAM)keyNum, (LPARAM)longPress, 0);
			key_scan[keyNum] = 0;
		}

		////////////
		if ((0XF9^BIT3) == P1 && 0XF9 == O2)
		{
			time(&T2);
			keyNum = 20;
			key_scan[keyNum] = 1;
			app->OnKeyScanPressed(M_KEY_SCAN_PRESSED, (WPARAM)keyNum, 0, 0);
		}
		if ((0XF9^BIT3) == P1 && (0XF9^BIT3) == O2)
		{
			time(&now);
			if (now - T2 >= 3)
			{
				if (0 == long_buzz_flag)
				{
					long_buzz_flag = 1;
					if (prob_pca955_1 && (0 == product_type || 1 == product_type || 2 == product_type))
						app->SetTimer(BUZZ_ON_TIMER_ID, 201, BuzzOnTimer);
				}
				keyNum = 20;
				key_scan[keyNum] = 2;
			}
		}
		if (0XF9 == P1 && (0XF9^BIT3) == O2)
		{
			keyNum = 20;
			time(&now);
			if (now - T2 >= 3)		longPress = 1, long_buzz_flag = 0;
			app->OnKeyScanReleased(M_KEY_SCAN_RELEASED, (WPARAM)keyNum, (LPARAM)longPress, 0);
			key_scan[keyNum] = 0;
		}

		////////////X
		if ((0XF9^BIT4) == P1 && 0XF9 == O2)
		{
			time(&T2);
			keyNum = 18;
			key_scan[keyNum] = 1;
			app->OnKeyScanPressed(M_KEY_SCAN_PRESSED, (WPARAM)keyNum, 0, 0);
		}
		if ((0XF9^BIT4) == P1 && (0XF9^BIT4) == O2)
		{
			time(&now);
			if (now - T2 >= 3)
			{
				if (0 == long_buzz_flag)
				{
					long_buzz_flag = 1;
					if (prob_pca955_1 && (0 == product_type || 1 == product_type || 2 == product_type))
						app->SetTimer(BUZZ_ON_TIMER_ID, 201, BuzzOnTimer);
				}
				keyNum = 18;
				key_scan[keyNum] = 2;
			}
		}
		if (0XF9 == P1 && (0XF9^BIT4) == O2)
		{
			keyNum = 18;
			time(&now);
			if (now - T2 >= 3)		longPress = 1, long_buzz_flag = 0;
			app->OnKeyScanReleased(M_KEY_SCAN_RELEASED, (WPARAM)keyNum, (LPARAM)longPress, 0);
			key_scan[keyNum] = 0;
		}

		////////////X
		if ((0XF9^BIT5) == P1 && 0XF9 == O2)
		{
			time(&T2);
			keyNum = 15;
			key_scan[keyNum] = 1;
			app->OnKeyScanPressed(M_KEY_SCAN_PRESSED, (WPARAM)keyNum, 0, 0);
		}
		if ((0XF9^BIT5) == P1 && (0XF9^BIT5) == O2)
		{
			time(&now);
			if (now - T2 >= 3)
			{
				if (0 == long_buzz_flag)
				{
					long_buzz_flag = 1;
					if (prob_pca955_1 && (0 == product_type || 1 == product_type || 2 == product_type))
						app->SetTimer(BUZZ_ON_TIMER_ID, 201, BuzzOnTimer);
				}
				keyNum = 15;
				key_scan[keyNum] = 2;
			}
		}
		if (0XF9 == P1 && (0XF9^BIT5) == O2)
		{
			keyNum = 15;
			time(&now);
			if (now - T2 >= 3)		longPress = 1, long_buzz_flag = 0;
			app->OnKeyScanReleased(M_KEY_SCAN_RELEASED, (WPARAM)keyNum, (LPARAM)longPress, 0);
			key_scan[keyNum] = 0;
		}

		O2 = P1;
	}


///////////////////////////////////////////////////////////
	if (3 == line)
	{
		if (0 == product_type || 1 == product_type)
		{
			i2c_read_data(app->i2c0, PCA_9555_SLAVE_1, PCA_9555_INPUT_1, &V, 1);
			V |= BIT0; V |= BIT1; V |= BIT2; V &= ~BIT3;
			i2c_write_data(app->i2c0, PCA_9555_SLAVE_1, PCA_9555_OUTPUT_1, &V, 1);

			i2c_read_data(app->i2c0, PCA_9555_SLAVE_1, PCA_9555_INPUT_0, &V, 1);
			if (!(V & BIT1))
			{
				// RIGHT-THUMB UP
				AfxGetApp()->PostMessage(M_RIGHT_THUMB, 1, 0);
			}
		}
		/*if (2 == product_type)
		{
			i2c_read_data(app->i2c0, PCA_9555_SLAVE_1, PCA_9555_INPUT_0, &V, 1);
			V |= BIT2; V |= BIT3; V |= BIT4;
			if (0 == product_type || 1 == product_type)
				V |= BIT5;
			i2c_write_data(app->i2c0, PCA_9555_SLAVE_1, PCA_9555_OUTPUT_0, &V, 1);

			i2c_read_data(app->i2c0, PCA_9555_SLAVE_1, PCA_9555_INPUT_0, &V, 1);
			if (!(V & BIT1))
			{
				// RIGHT-THUMB UP
				AfxGetApp()->PostMessage(M_RIGHT_THUMB, 1, 0);
			}
		}*/


		////////////
		if ((0XF9^BIT0) == P1 && 0XF9 == O3)
		{
			time(&T3);
			keyNum = 2;
			key_scan[keyNum] = 1;
			app->OnKeyScanPressed(M_KEY_SCAN_PRESSED, (WPARAM)keyNum, 0, 0);
		}
		if ((0XF9^BIT0) == P1 && (0XF9^BIT0) == O3)
		{
			time(&now);
			if (now - T3 >= 3)
			{
				if (0 == long_buzz_flag)
				{
					long_buzz_flag = 1;
					if (prob_pca955_1 && (0 == product_type || 1 == product_type || 2 == product_type))
						app->SetTimer(BUZZ_ON_TIMER_ID, 201, BuzzOnTimer);
				}
				keyNum = 2;
				key_scan[keyNum] = 2;
			}
		}
		if (0XF9 == P1 && (0XF9^BIT0) == O3)
		{
			keyNum = 2;
			time(&now);
			if (now - T3 >= 3)		longPress = 1, long_buzz_flag = 0;
			app->OnKeyScanReleased(M_KEY_SCAN_RELEASED, (WPARAM)keyNum, (LPARAM)longPress, 0);
			key_scan[keyNum] = 0;
		}

		////////////
		if ((0XF9^BIT3) == P1 && 0XF9 == O3)
		{
			time(&T3);
			keyNum = 10;
			key_scan[keyNum] = 1;
			app->OnKeyScanPressed(M_KEY_SCAN_PRESSED, (WPARAM)keyNum, 0, 0);
		}
		if ((0XF9^BIT3) == P1 && (0XF9^BIT3) == O3)
		{
			time(&now);
			if (now - T3 >= 3)
			{
				if (0 == long_buzz_flag)
				{
					long_buzz_flag = 1;
					if (prob_pca955_1 && (0 == product_type || 1 == product_type || 2 == product_type))
						app->SetTimer(BUZZ_ON_TIMER_ID, 201, BuzzOnTimer);
				}
				keyNum = 10;
				key_scan[keyNum] = 2;
			}
		}
		if (0XF9 == P1 && (0XF9^BIT3) == O3)
		{
			keyNum = 10;
			time(&now);
			if (now - T3 >= 3)		longPress = 1, long_buzz_flag = 0;
			app->OnKeyScanReleased(M_KEY_SCAN_RELEASED, (WPARAM)keyNum, (LPARAM)longPress, 0);
			key_scan[keyNum] = 0;
		}

		////////////X
		if ((0XF9^BIT4) == P1 && 0XF9 == O3)
		{
			time(&T3);
			keyNum = 19;
			key_scan[keyNum] = 1;
			app->OnKeyScanPressed(M_KEY_SCAN_PRESSED, (WPARAM)keyNum, 0, 0);
		}
		if ((0XF9^BIT4) == P1 && (0XF9^BIT4) == O3)
		{
			time(&now);
			if (now - T3 >= 3)
			{
				if (0 == long_buzz_flag)
				{
					long_buzz_flag = 1;
					if (prob_pca955_1 && (0 == product_type || 1 == product_type || 2 == product_type))
						app->SetTimer(BUZZ_ON_TIMER_ID, 201, BuzzOnTimer);
				}
				keyNum = 19;
				key_scan[keyNum] = 2;
			}
		}
		if (0XF9 == P1 && (0XF9^BIT4) == O3)
		{
			keyNum = 19;
			time(&now);
			if (now - T3 >= 3)		longPress = 1, long_buzz_flag = 0;
			app->OnKeyScanReleased(M_KEY_SCAN_RELEASED, (WPARAM)keyNum, (LPARAM)longPress, 0);
			key_scan[keyNum] = 0;
		}

		////////////X
		if ((0XF9^BIT5) == P1 && 0XF9 == O3)
		{
			time(&T3);
			keyNum = 16;
			key_scan[keyNum] = 1;
			app->OnKeyScanPressed(M_KEY_SCAN_PRESSED, (WPARAM)keyNum, 0, 0);
		}
		if ((0XF9^BIT5) == P1 && (0XF9^BIT5) == O3)
		{
			time(&now);
			if (now - T3 >= 3)
			{
				if (0 == long_buzz_flag)
				{
					long_buzz_flag = 1;
					if (prob_pca955_1 && (0 == product_type || 1 == product_type || 2 == product_type))
						app->SetTimer(BUZZ_ON_TIMER_ID, 201, BuzzOnTimer);
				}
				keyNum = 16;
				key_scan[keyNum] = 2;
			}
		}
		if (0XF9 == P1 && (0XF9^BIT5) == O3)
		{
			keyNum = 16;
			time(&now);
			if (now - T3 >= 3)		longPress = 1, long_buzz_flag = 0;
			app->OnKeyScanReleased(M_KEY_SCAN_RELEASED, (WPARAM)keyNum, (LPARAM)longPress, 0);
			key_scan[keyNum] = 0;
		}

		O3 = P1;
	}


///////////////////////////////////////////////////////////
	if (4 == line)
	{
		////////////
		if ((0XF9^BIT0) == P1 && 0XF9 == O4)
		{
			time(&T4);
			keyNum = 3;
			key_scan[keyNum] = 1;
			app->OnKeyScanPressed(M_KEY_SCAN_PRESSED, (WPARAM)keyNum, 0, 0);
		}
		if ((0XF9^BIT0) == P1 && (0XF9^BIT0) == O4)
		{
			time(&now);
			if (now - T4 >= 3)
			{
				if (0 == long_buzz_flag)
				{
					long_buzz_flag = 1;
					if (prob_pca955_1 && (0 == product_type || 1 == product_type || 2 == product_type))
						app->SetTimer(BUZZ_ON_TIMER_ID, 201, BuzzOnTimer);
				}
				keyNum = 3;
				key_scan[keyNum] = 2;
			}
		}
		if (0XF9 == P1 && (0XF9^BIT0) == O4)
		{
			keyNum = 3;
			time(&now);
			if (now - T4 >= 3)		longPress = 1, long_buzz_flag = 0;
			app->OnKeyScanReleased(M_KEY_SCAN_RELEASED, (WPARAM)keyNum, (LPARAM)longPress, 0);
			key_scan[keyNum] = 0;
		}

		////////////?
		if ((0XF9^BIT3) == P1 && 0XF9 == O4)
		{
			time(&T4);
			keyNum = 23;
			key_scan[keyNum] = 1;
			app->OnKeyScanPressed(M_KEY_SCAN_PRESSED, (WPARAM)keyNum, 0, 0);
		}
		if ((0XF9^BIT3) == P1 && (0XF9^BIT3) == O4)
		{
			time(&now);
			if (now - T4 >= 3)
			{
				if (0 == long_buzz_flag)
				{
					long_buzz_flag = 1;
					if (prob_pca955_1 && (0 == product_type || 1 == product_type || 2 == product_type))
						app->SetTimer(BUZZ_ON_TIMER_ID, 201, BuzzOnTimer);
				}
				keyNum = 23;
				key_scan[keyNum] = 2;
			}
		}
		if (0XF9 == P1 && (0XF9^BIT3) == O4)
		{
			keyNum = 23;
			time(&now);
			if (now - T4 >= 3)		longPress = 1, long_buzz_flag = 0;
			app->OnKeyScanReleased(M_KEY_SCAN_RELEASED, (WPARAM)keyNum, (LPARAM)longPress, 0);
			key_scan[keyNum] = 0;
		}

		////////////
		if ((0XF9^BIT4) == P1 && 0XF9 == O4)
		{
			time(&T4);
			keyNum = 9;
			key_scan[keyNum] = 1;
			app->OnKeyScanPressed(M_KEY_SCAN_PRESSED, (WPARAM)keyNum, 0, 0);
		}
		if ((0XF9^BIT4) == P1 && (0XF9^BIT4) == O4)
		{
			time(&now);
			if (now - T4 >= 3)
			{
				if (0 == long_buzz_flag)
				{
					long_buzz_flag = 1;
					if (prob_pca955_1 && (0 == product_type || 1 == product_type || 2 == product_type))
						app->SetTimer(BUZZ_ON_TIMER_ID, 201, BuzzOnTimer);
				}
				keyNum = 9;
				key_scan[keyNum] = 2;
			}
		}
		if (0XF9 == P1 && (0XF9^BIT4) == O4)
		{
			keyNum = 9;
			time(&now);
			if (now - T4 >= 3)		longPress = 1, long_buzz_flag = 0;
			app->OnKeyScanReleased(M_KEY_SCAN_RELEASED, (WPARAM)keyNum, (LPARAM)longPress, 0);
			key_scan[keyNum] = 0;
		}


		////////////X
		if ((0XF9^BIT5) == P1 && 0XF9 == O4)
		{
			time(&T4);
			keyNum = 12;
			key_scan[keyNum] = 1;
			app->OnKeyScanPressed(M_KEY_SCAN_PRESSED, (WPARAM)keyNum, 0, 0);
		}
		if ((0XF9^BIT5) == P1 && (0XF9^BIT5) == O4)
		{
			time(&now);
			if (now - T4 >= 3)
			{
				if (0 == long_buzz_flag)
				{
					long_buzz_flag = 1;
					if (prob_pca955_1 && (0 == product_type || 1 == product_type || 2 == product_type))
						app->SetTimer(BUZZ_ON_TIMER_ID, 201, BuzzOnTimer);
				}
				keyNum = 12;
				key_scan[keyNum] = 2;
			}
		}
		if (0XF9 == P1 && (0XF9^BIT5) == O4)
		{
			keyNum = 12;
			time(&now);
			if (now - T4 >= 3)		longPress = 1, long_buzz_flag = 0;
			app->OnKeyScanReleased(M_KEY_SCAN_RELEASED, (WPARAM)keyNum, (LPARAM)longPress, 0);
			key_scan[keyNum] = 0;
		}

		O4 = P1;
	}


///////////////////////////////////////////////////////////
	if (5 == line)
	{
		////////////?
		if ((0XF9^BIT0) == P1 && 0XF9 == O5)
		{
			time(&T5);
			keyNum = 8;
			key_scan[keyNum] = 1;
			app->OnKeyScanPressed(M_KEY_SCAN_PRESSED, (WPARAM)keyNum, 0, 0);
		}
		if ((0XF9^BIT0) == P1 && (0XF9^BIT0) == O5)
		{
			time(&now);
			if (now - T5 >= 3)
			{
				if (0 == long_buzz_flag)
				{
					long_buzz_flag = 1;
					if (prob_pca955_1 && (0 == product_type || 1 == product_type || 2 == product_type))
						app->SetTimer(BUZZ_ON_TIMER_ID, 201, BuzzOnTimer);
				}
				keyNum = 8;
				key_scan[keyNum] = 2;
			}
		}
		if (0XF9 == P1 && (0XF9^BIT0) == O5)
		{
			keyNum = 8;
			time(&now);
			if (now - T5 >= 3)		longPress = 1, long_buzz_flag = 0;
			app->OnKeyScanReleased(M_KEY_SCAN_RELEASED, (WPARAM)keyNum, (LPARAM)longPress, 0);
			key_scan[keyNum] = 0;
		}

		////////////X
		if ((0XF9^BIT5) == P1 && 0XF9 == O5)
		{
			time(&T5);
			keyNum = 11;
			key_scan[keyNum] = 1;
			app->OnKeyScanPressed(M_KEY_SCAN_PRESSED, (WPARAM)keyNum, 0, 0);
		}
		if ((0XF9^BIT5) == P1 && (0XF9^BIT5) == O5)
		{
			time(&now);
			if (now - T5 >= 3)
			{
				if (0 == long_buzz_flag)
				{
					long_buzz_flag = 1;
					if (prob_pca955_1 && (0 == product_type || 1 == product_type || 2 == product_type))
						app->SetTimer(BUZZ_ON_TIMER_ID, 201, BuzzOnTimer);
				}
				keyNum = 11;
				key_scan[keyNum] = 2;
			}
		}
		if (0XF9 == P1 && (0XF9^BIT5) == O5)
		{
			keyNum = 11;
			time(&now);
			if (now - T5 >= 3)		longPress = 1, long_buzz_flag = 0;
			app->OnKeyScanReleased(M_KEY_SCAN_RELEASED, (WPARAM)keyNum, (LPARAM)longPress, 0);
			key_scan[keyNum] = 0;
		}

		O5 = P1;
	}


///////////////////////////////////////////////////////////
	if (6 == line)
	{
		////////////
		if ((0XF9^BIT5) == P1 && 0XF9 == O6)
		{
			time(&T6);
			keyNum = 6;
			key_scan[keyNum] = 1;
			app->OnKeyScanPressed(M_KEY_SCAN_PRESSED, (WPARAM)keyNum, 0, 0);
		}
		if ((0XF9^BIT5) == P1 && (0XF9^BIT5) == O6)
		{
			time(&now);
			if (now - T6 >= 3)
			{
				if (0 == long_buzz_flag)
				{
					long_buzz_flag = 1;
					if (prob_pca955_1 && (0 == product_type || 1 == product_type || 2 == product_type))
						app->SetTimer(BUZZ_ON_TIMER_ID, 201, BuzzOnTimer);
				}
				keyNum = 6;
				key_scan[keyNum] = 2;
			}
		}
		if (0XF9 == P1 && (0XF9^BIT5) == O6)
		{
			keyNum = 6;
			time(&now);
			if (now - T6 >= 3)		longPress = 1, long_buzz_flag = 0;
			app->OnKeyScanReleased(M_KEY_SCAN_RELEASED, (WPARAM)keyNum, (LPARAM)longPress, 0);
			key_scan[keyNum] = 0;
		}

		O6 = P1;
	}

	return 0;
}

/*
int PollKeyScan(unsigned char line)
{
	unsigned char P0;
	unsigned char V = 0;

	int longPress = 0;
	int keyNum = 0;
	time_t now = 0;
	CQsApp *app = (CQsApp *)AfxGetApp();

	static unsigned char O0 = 0;
	static time_t T0 = 0;
	static unsigned char O1 = 0;
	static time_t T1 = 0;
	static unsigned char O2 = 0;
	static time_t T2 = 0;
	static unsigned char O3 = 0;
	static time_t T3 = 0;
	static unsigned char O4 = 0;
	static time_t T4 = 0;
	static unsigned char O5 = 0;
	static time_t T5 = 0;
	static unsigned char O6 = 0;
	static time_t T6 = 0;

	if (line >= 8)		return -1;
	memset(&P0, 0XFF, sizeof(P0));
	P0 &= ~(1<<line);

	if (-1 == i2c_write_data(app->i2c0, PCA_9555_SLAVE_0, PCA_9555_OUTPUT_0, (unsigned char *)&P0, 1))
	{
		printf("(%s %d) i2c_write_data() FAIL\n", __FILE__, __LINE__);
		return -1;
	}
	if (-1 == i2c_read_data(app->i2c0, PCA_9555_SLAVE_0, PCA_9555_INPUT_1, &P1, 1))
	{
		printf("(%s %d) i2c_read_data() FAIL\n", __FILE__, __LINE__);
		return -1;
	}


///////////////////////////////////////////////////////////
	if (0 == line)
	{
		if (0 == product_type || 1 == product_type)
		{
			i2c_read_data(app->i2c0, PCA_9555_SLAVE_1, PCA_9555_INPUT_1, &V, 1);
			V &= ~BIT0; V |= BIT1; V |= BIT2; V |= BIT3;
			i2c_write_data(app->i2c0, PCA_9555_SLAVE_1, PCA_9555_OUTPUT_1, &V, 1);

			i2c_read_data(app->i2c0, PCA_9555_SLAVE_1, PCA_9555_INPUT_0, &V, 1);


			if (!(V & BIT0))
			{
				// LEFT-THUMB DOWN
				AfxGetApp()->PostMessage(M_LEFT_THUMB, 0, 0);
			}

		}
		if (2 == product_type)
		{
			i2c_read_data(app->i2c0, PCA_9555_SLAVE_1, PCA_9555_INPUT_0, &V, 1);
			V &= ~BIT2; V |= BIT3; V |= BIT4;
			if (0 == product_type || 1 == product_type)
				V |= BIT5;
			i2c_write_data(app->i2c0, PCA_9555_SLAVE_1, PCA_9555_OUTPUT_0, &V, 1);

			i2c_read_data(app->i2c0, PCA_9555_SLAVE_1, PCA_9555_INPUT_0, &V, 1);
			if (!(V & BIT0))
			{
				// LEFT-THUMB UP
				AfxGetApp()->PostMessage(M_LEFT_THUMB, 0, 0);
			}
		}

		////////////
		if ((0XF9^BIT0) == P1 && 0XF9 == O0)
		{
			time(&T0);
			keyNum = 5;
			key_scan[keyNum] = 1;
			app->OnKeyScanPressed(M_KEY_SCAN_PRESSED, (WPARAM)keyNum, 0, 0);
		}
		if ((0XF9^BIT0) == P1 && (0XF9^BIT0) == O0)
		//if ((0XF9^BIT0) == P1 && BIT0 != (O0&BIT0))
		{
			time(&now);
			if (now - T0 >= 3)
			{
				keyNum = 5;
				key_scan[keyNum] = 2;
			}
		}
		if (0XF9 == P1 && (0XF9^BIT0) == O0)
		{
			keyNum = 5;
			time(&now);
			if (now - T0 >= 3)		longPress = 1;
			app->OnKeyScanReleased(M_KEY_SCAN_RELEASED, (WPARAM)keyNum, (LPARAM)longPress, 0);
			key_scan[keyNum] = 0;
		}

		////////////
		if ((0XF9^BIT3) == P1 && 0XF9 == O0)
		{
			time(&T0);
			keyNum = 22;
			key_scan[keyNum] = 1;
			app->OnKeyScanPressed(M_KEY_SCAN_PRESSED, (WPARAM)keyNum, 0, 0);
		}
		if ((0XF9^BIT3) == P1 && (0XF9^BIT3) == O0)
		{
			time(&now);
			if (now - T0 >= 3)
			{
				keyNum = 22;
				key_scan[keyNum] = 2;
			}
		}
		if (0XF9 == P1 && (0XF9^BIT3) == O0)
		{
			keyNum = 22;
			time(&now);
			if (now - T0 >= 3)		longPress = 1;
			app->OnKeyScanReleased(M_KEY_SCAN_RELEASED, (WPARAM)keyNum, (LPARAM)longPress, 0);
			key_scan[keyNum] = 0;
		}

		////////////
		if ((0XF9^BIT4) == P1 && 0XF9 == O0)
		{
			time(&T0);
			keyNum = 21;
			key_scan[keyNum] = 1;
			app->OnKeyScanPressed(M_KEY_SCAN_PRESSED, (WPARAM)keyNum, 0, 0);
		}
		if ((0XF9^BIT4) == P1 && (0XF9^BIT4) == O0)
		{
			time(&now);
			if (now - T0 >= 3)
			{
				keyNum = 21;
				key_scan[keyNum] = 2;
			}
		}
		if (0XF9 == P1 && (0XF9^BIT4) == O0)
		{
			keyNum = 21;
			time(&now);
			if (now - T0 >= 3)		longPress = 1;
			app->OnKeyScanReleased(M_KEY_SCAN_RELEASED, (WPARAM)keyNum, (LPARAM)longPress, 0);
			key_scan[keyNum] = 0;
		}


		////////////X
		if ((0XF9^BIT5) == P1 && 0XF9 == O0)
		{
			time(&T0);
			keyNum = 13;
			key_scan[keyNum] = 1;
			app->OnKeyScanPressed(M_KEY_SCAN_PRESSED, (WPARAM)keyNum, 0, 0);
		}
		if ((0XF9^BIT5) == P1 && (0XF9^BIT5) == O0)
		{
			time(&now);
			if (now - T0 >= 3)
			{
				keyNum = 13;
				key_scan[keyNum] = 2;
			}
		}
		if (0XF9 == P1 && (0XF9^BIT5) == O0)
		{
			keyNum = 13;
			time(&now);
			if (now - T0 >= 3)		longPress = 1;
			app->OnKeyScanReleased(M_KEY_SCAN_RELEASED, (WPARAM)keyNum, (LPARAM)longPress, 0);
			key_scan[keyNum] = 0;
		}

		O0 = P1;
	}


///////////////////////////////////////////////////////////
	if (1 == line)
	{
		if (0 == product_type || 1 == product_type)
		{
			i2c_read_data(app->i2c0, PCA_9555_SLAVE_1, PCA_9555_INPUT_1, &V, 1);
			V |= BIT0; V &= ~BIT1; V |= BIT2; V |= BIT3;
			i2c_write_data(app->i2c0, PCA_9555_SLAVE_1, PCA_9555_OUTPUT_1, &V, 1);

			i2c_read_data(app->i2c0, PCA_9555_SLAVE_1, PCA_9555_INPUT_0, &V, 1);

			if (!(V & BIT1))
			{
				// LEFT-THUMB UP
				AfxGetApp()->PostMessage(M_LEFT_THUMB, 1, 0);
			}

		}
		if (2 == product_type)
		{
			i2c_read_data(app->i2c0, PCA_9555_SLAVE_1, PCA_9555_INPUT_0, &V, 1);
			V |= BIT2; V &= ~BIT3; V |= BIT4;
			if (0 == product_type || 1 == product_type)
				V |= BIT5;
			i2c_write_data(app->i2c0, PCA_9555_SLAVE_1, PCA_9555_OUTPUT_0, &V, 1);

			i2c_read_data(app->i2c0, PCA_9555_SLAVE_1, PCA_9555_INPUT_0, &V, 1);
			if (!(V & BIT1))
			{
				// LEFT-THUMB UP
				AfxGetApp()->PostMessage(M_LEFT_THUMB, 1, 0);
			}
		}


		////////////
		if ((0XF9^BIT0) == P1 && 0XF9 == O1)
		{
			time(&T1);
			keyNum = 4;
			key_scan[keyNum] = 1;
			app->OnKeyScanPressed(M_KEY_SCAN_PRESSED, (WPARAM)keyNum, 0, 0);
		}
		if ((0XF9^BIT0) == P1 && (0XF9^BIT0) == O1)
		{
			time(&now);
			if (now - T1 >= 3)
			{
				keyNum = 4;
				key_scan[keyNum] = 2;
			}
		}
		if (0XF9 == P1 && (0XF9^BIT0) == O1)
		{
			keyNum = 4;
			time(&now);
			if (now - T1 >= 3)		longPress = 1;
			app->OnKeyScanReleased(M_KEY_SCAN_RELEASED, (WPARAM)keyNum, (LPARAM)longPress, 0);
			key_scan[keyNum] = 0;
		}

		////////////
		if ((0XF9^BIT3) == P1 && 0XF9 == O1)
		{
			time(&T1);
			keyNum = 23;
			key_scan[keyNum] = 1;
			app->OnKeyScanPressed(M_KEY_SCAN_PRESSED, (WPARAM)keyNum, 0, 0);
		}
		if ((0XF9^BIT3) == P1 && (0XF9^BIT3) == O1)
		{
			time(&now);
			if (now - T1 >= 3)
			{
				keyNum = 23;
				key_scan[keyNum] = 2;
			}
		}
		if (0XF9 == P1 && (0XF9^BIT3) == O1)
		{
			keyNum = 23;
			time(&now);
			if (now - T1 >= 3)		longPress = 1;
			app->OnKeyScanReleased(M_KEY_SCAN_RELEASED, (WPARAM)keyNum, (LPARAM)longPress, 0);
			key_scan[keyNum] = 0;
		}

		////////////X
		if ((0XF9^BIT4) == P1 && 0XF9 == O1)
		{
			time(&T1);
			keyNum = 17;
			key_scan[keyNum] = 1;
			app->OnKeyScanPressed(M_KEY_SCAN_PRESSED, (WPARAM)keyNum, 0, 0);
		}
		if ((0XF9^BIT4) == P1 && (0XF9^BIT4) == O1)
		{
			time(&now);
			if (now - T1 >= 3)
			{
				keyNum = 17;
				key_scan[keyNum] = 2;
			}
		}
		if (0XF9 == P1 && (0XF9^BIT4) == O1)
		{
			keyNum = 17;
			time(&now);
			if (now - T1 >= 3)		longPress = 1;
			app->OnKeyScanReleased(M_KEY_SCAN_RELEASED, (WPARAM)keyNum, (LPARAM)longPress, 0);
			key_scan[keyNum] = 0;
		}

		////////////X
		if ((0XF9^BIT5) == P1 && 0XF9 == O1)
		{
			time(&T1);
			keyNum = 14;
			key_scan[keyNum] = 1;
			app->OnKeyScanPressed(M_KEY_SCAN_PRESSED, (WPARAM)keyNum, 0, 0);
		}
		if ((0XF9^BIT5) == P1 && (0XF9^BIT5) == O1)
		{
			time(&now);
			if (now - T1 >= 3)
			{
				keyNum = 14;
				key_scan[keyNum] = 2;
			}
		}
		if (0XF9 == P1 && (0XF9^BIT5) == O1)
		{
			keyNum = 14;
			time(&now);
			if (now - T1 >= 3)		longPress = 1;
			app->OnKeyScanReleased(M_KEY_SCAN_RELEASED, (WPARAM)keyNum, (LPARAM)longPress, 0);
			key_scan[keyNum] = 0;
		}

		O1 = P1;
	}


///////////////////////////////////////////////////////////
	if (2 == line)
	{
		if (0 == product_type || 1 == product_type)
		{
			i2c_read_data(app->i2c0, PCA_9555_SLAVE_1, PCA_9555_INPUT_1, &V, 1);
			V |= BIT0; V |= BIT1; V &= ~BIT2; V |= BIT3;
			i2c_write_data(app->i2c0, PCA_9555_SLAVE_1, PCA_9555_OUTPUT_1, &V, 1);

			i2c_read_data(app->i2c0, PCA_9555_SLAVE_1, PCA_9555_INPUT_0, &V, 1);
			if (!(V & BIT0))
			{
				// RIGHT-THUMB DOWN
				AfxGetApp()->PostMessage(M_RIGHT_THUMB, 0, 0);
			}
		}
		if (2 == product_type)
		{
			i2c_read_data(app->i2c0, PCA_9555_SLAVE_1, PCA_9555_INPUT_0, &V, 1);
			V |= BIT2; V |= BIT3; V &= ~BIT4; V |= BIT5;
			i2c_write_data(app->i2c0, PCA_9555_SLAVE_1, PCA_9555_OUTPUT_0, &V, 1);

			i2c_read_data(app->i2c0, PCA_9555_SLAVE_1, PCA_9555_INPUT_0, &V, 1);
			if (!(V & BIT0))
			{
				// RIGHT-THUMB DOWN
				AfxGetApp()->PostMessage(M_RIGHT_THUMB, 0, 0);
			}
		}


		////////////
		if ((0XF9^BIT0) == P1 && 0XF9 == O2)
		{
			time(&T2);
			keyNum = 1;
			key_scan[keyNum] = 1;
			app->OnKeyScanPressed(M_KEY_SCAN_PRESSED, (WPARAM)keyNum, 0, 0);
		}
		if ((0XF9^BIT0) == P1 && (0XF9^BIT0) == O2)
		{
			time(&now);
			if (now - T2 >= 3)
			{
				keyNum = 1;
				key_scan[keyNum] = 2;
			}
		}
		if (0XF9 == P1 && (0XF9^BIT0) == O2)
		{
			keyNum = 1;
			time(&now);
			if (now - T2 >= 3)		longPress = 1;
			app->OnKeyScanReleased(M_KEY_SCAN_RELEASED, (WPARAM)keyNum, (LPARAM)longPress, 0);
			key_scan[keyNum] = 0;
		}

		////////////
		if ((0XF9^BIT3) == P1 && 0XF9 == O2)
		{
			time(&T2);
			keyNum = 20;
			key_scan[keyNum] = 1;
			app->OnKeyScanPressed(M_KEY_SCAN_PRESSED, (WPARAM)keyNum, 0, 0);
		}
		if ((0XF9^BIT3) == P1 && (0XF9^BIT3) == O2)
		{
			time(&now);
			if (now - T2 >= 3)
			{
				keyNum = 20;
				key_scan[keyNum] = 2;
			}
		}
		if (0XF9 == P1 && (0XF9^BIT3) == O2)
		{
			keyNum = 20;
			time(&now);
			if (now - T2 >= 3)		longPress = 1;
			app->OnKeyScanReleased(M_KEY_SCAN_RELEASED, (WPARAM)keyNum, (LPARAM)longPress, 0);
			key_scan[keyNum] = 0;
		}

		////////////X
		if ((0XF9^BIT4) == P1 && 0XF9 == O2)
		{
			time(&T2);
			keyNum = 18;
			key_scan[keyNum] = 1;
			app->OnKeyScanPressed(M_KEY_SCAN_PRESSED, (WPARAM)keyNum, 0, 0);
		}
		if ((0XF9^BIT4) == P1 && (0XF9^BIT4) == O2)
		{
			time(&now);
			if (now - T2 >= 3)
			{
				keyNum = 18;
				key_scan[keyNum] = 2;
			}
		}
		if (0XF9 == P1 && (0XF9^BIT4) == O2)
		{
			keyNum = 18;
			time(&now);
			if (now - T2 >= 3)		longPress = 1;
			app->OnKeyScanReleased(M_KEY_SCAN_RELEASED, (WPARAM)keyNum, (LPARAM)longPress, 0);
			key_scan[keyNum] = 0;
		}

		////////////X
		if ((0XF9^BIT5) == P1 && 0XF9 == O2)
		{
			time(&T2);
			keyNum = 15;
			key_scan[keyNum] = 1;
			app->OnKeyScanPressed(M_KEY_SCAN_PRESSED, (WPARAM)keyNum, 0, 0);
		}
		if ((0XF9^BIT5) == P1 && (0XF9^BIT5) == O2)
		{
			time(&now);
			if (now - T2 >= 3)
			{
				keyNum = 15;
				key_scan[keyNum] = 2;
			}
		}
		if (0XF9 == P1 && (0XF9^BIT5) == O2)
		{
			keyNum = 15;
			time(&now);
			if (now - T2 >= 3)		longPress = 1;
			app->OnKeyScanReleased(M_KEY_SCAN_RELEASED, (WPARAM)keyNum, (LPARAM)longPress, 0);
			key_scan[keyNum] = 0;
		}

		O2 = P1;
	}


///////////////////////////////////////////////////////////
	if (3 == line)
	{
		if (0 == product_type || 1 == product_type)
		{
			i2c_read_data(app->i2c0, PCA_9555_SLAVE_1, PCA_9555_INPUT_1, &V, 1);
			V |= BIT0; V |= BIT1; V |= BIT2; V &= ~BIT3;
			i2c_write_data(app->i2c0, PCA_9555_SLAVE_1, PCA_9555_OUTPUT_1, &V, 1);

			i2c_read_data(app->i2c0, PCA_9555_SLAVE_1, PCA_9555_INPUT_0, &V, 1);
			if (!(V & BIT1))
			{
				// RIGHT-THUMB UP
				AfxGetApp()->PostMessage(M_RIGHT_THUMB, 1, 0);
			}
		}
		if (2 == product_type)
		{
			i2c_read_data(app->i2c0, PCA_9555_SLAVE_1, PCA_9555_INPUT_0, &V, 1);
			V |= BIT2; V |= BIT3; V |= BIT4; V &= ~BIT5;
			i2c_write_data(app->i2c0, PCA_9555_SLAVE_1, PCA_9555_OUTPUT_0, &V, 1);

			i2c_read_data(app->i2c0, PCA_9555_SLAVE_1, PCA_9555_INPUT_0, &V, 1);
			if (!(V & BIT1))
			{
				// RIGHT-THUMB UP
				AfxGetApp()->PostMessage(M_RIGHT_THUMB, 1, 0);
			}
		}


		////////////
		if ((0XF9^BIT0) == P1 && 0XF9 == O3)
		{
			time(&T3);
			keyNum = 2;
			key_scan[keyNum] = 1;
			app->OnKeyScanPressed(M_KEY_SCAN_PRESSED, (WPARAM)keyNum, 0, 0);
		}
		if ((0XF9^BIT0) == P1 && (0XF9^BIT0) == O3)
		{
			time(&now);
			if (now - T3 >= 3)
			{
				keyNum = 2;
				key_scan[keyNum] = 2;
			}
		}
		if (0XF9 == P1 && (0XF9^BIT0) == O3)
		{
			keyNum = 2;
			time(&now);
			if (now - T3 >= 3)		longPress = 1;
			app->OnKeyScanReleased(M_KEY_SCAN_RELEASED, (WPARAM)keyNum, (LPARAM)longPress, 0);
			key_scan[keyNum] = 0;
		}

		////////////X
		if ((0XF9^BIT3) == P1 && 0XF9 == O3)
		{
			time(&T3);
			keyNum = 10;
			key_scan[keyNum] = 1;
			app->OnKeyScanPressed(M_KEY_SCAN_PRESSED, (WPARAM)keyNum, 0, 0);
		}
		if ((0XF9^BIT3) == P1 && (0XF9^BIT3) == O3)
		{
			time(&now);
			if (now - T3 >= 3)
			{
				keyNum = 10;
				key_scan[keyNum] = 2;
			}
		}
		if (0XF9 == P1 && (0XF9^BIT3) == O3)
		{
			keyNum = 10;
			time(&now);
			if (now - T3 >= 3)		longPress = 1;
			app->OnKeyScanReleased(M_KEY_SCAN_RELEASED, (WPARAM)keyNum, (LPARAM)longPress, 0);
			key_scan[keyNum] = 0;
		}

		////////////X
		if ((0XF9^BIT4) == P1 && 0XF9 == O3)
		{
			time(&T3);
			keyNum = 19;
			key_scan[keyNum] = 1;
			app->OnKeyScanPressed(M_KEY_SCAN_PRESSED, (WPARAM)keyNum, 0, 0);
		}
		if ((0XF9^BIT4) == P1 && (0XF9^BIT4) == O3)
		{
			time(&now);
			if (now - T3 >= 3)
			{
				keyNum = 19;
				key_scan[keyNum] = 2;
			}
		}
		if (0XF9 == P1 && (0XF9^BIT4) == O3)
		{
			keyNum = 19;
			time(&now);
			if (now - T3 >= 3)		longPress = 1;
			app->OnKeyScanReleased(M_KEY_SCAN_RELEASED, (WPARAM)keyNum, (LPARAM)longPress, 0);
			key_scan[keyNum] = 0;
		}

		////////////X
		if ((0XF9^BIT5) == P1 && 0XF9 == O3)
		{
			time(&T3);
			keyNum = 16;
			key_scan[keyNum] = 1;
			app->OnKeyScanPressed(M_KEY_SCAN_PRESSED, (WPARAM)keyNum, 0, 0);
		}
		if ((0XF9^BIT5) == P1 && (0XF9^BIT5) == O3)
		{
			time(&now);
			if (now - T3 >= 3)
			{
				keyNum = 16;
				key_scan[keyNum] = 2;
			}
		}
		if (0XF9 == P1 && (0XF9^BIT5) == O3)
		{
			keyNum = 16;
			time(&now);
			if (now - T3 >= 3)		longPress = 1;
			app->OnKeyScanReleased(M_KEY_SCAN_RELEASED, (WPARAM)keyNum, (LPARAM)longPress, 0);
			key_scan[keyNum] = 0;
		}

		O3 = P1;
	}


///////////////////////////////////////////////////////////
	if (4 == line)
	{
		////////////
		if ((0XF9^BIT0) == P1 && 0XF9 == O4)
		{
			time(&T4);
			keyNum = 3;
			key_scan[keyNum] = 1;
			app->OnKeyScanPressed(M_KEY_SCAN_PRESSED, (WPARAM)keyNum, 0, 0);
		}
		if ((0XF9^BIT0) == P1 && (0XF9^BIT0) == O4)
		{
			time(&now);
			if (now - T4 >= 3)
			{
				keyNum = 3;
				key_scan[keyNum] = 2;
			}
		}
		if (0XF9 == P1 && (0XF9^BIT0) == O4)
		{
			keyNum = 3;
			time(&now);
			if (now - T4 >= 3)		longPress = 1;
			app->OnKeyScanReleased(M_KEY_SCAN_RELEASED, (WPARAM)keyNum, (LPARAM)longPress, 0);
			key_scan[keyNum] = 0;
		}

		////////////
		if ((0XF9^BIT3) == P1 && 0XF9 == O4)
		{
			time(&T4);
			keyNum = 8;
			key_scan[keyNum] = 1;
			app->OnKeyScanPressed(M_KEY_SCAN_PRESSED, (WPARAM)keyNum, 0, 0);
		}
		if ((0XF9^BIT3) == P1 && (0XF9^BIT3) == O4)
		{
			time(&now);
			if (now - T4 >= 3)
			{
				keyNum = 8;
				key_scan[keyNum] = 2;
			}
		}
		if (0XF9 == P1 && (0XF9^BIT3) == O4)
		{
			keyNum = 8;
			time(&now);
			if (now - T4 >= 3)		longPress = 1;
			app->OnKeyScanReleased(M_KEY_SCAN_RELEASED, (WPARAM)keyNum, (LPARAM)longPress, 0);
			key_scan[keyNum] = 0;
		}

		////////////
		if ((0XF9^BIT4) == P1 && 0XF9 == O4)
		{
			time(&T4);
			keyNum = 9;
			key_scan[keyNum] = 1;
			app->OnKeyScanPressed(M_KEY_SCAN_PRESSED, (WPARAM)keyNum, 0, 0);
		}
		if ((0XF9^BIT4) == P1 && (0XF9^BIT4) == O4)
		{
			time(&now);
			if (now - T4 >= 3)
			{
				keyNum = 9;
				key_scan[keyNum] = 2;
			}
		}
		if (0XF9 == P1 && (0XF9^BIT4) == O4)
		{
			keyNum = 9;
			time(&now);
			if (now - T4 >= 3)		longPress = 1;
			app->OnKeyScanReleased(M_KEY_SCAN_RELEASED, (WPARAM)keyNum, (LPARAM)longPress, 0);
			key_scan[keyNum] = 0;
		}


		////////////XX
		if ((0XF9^BIT5) == P1 && 0XF9 == O4)
		{
			time(&T4);
			keyNum = 12;
			key_scan[keyNum] = 1;
			app->OnKeyScanPressed(M_KEY_SCAN_PRESSED, (WPARAM)keyNum, 0, 0);
		}
		if ((0XF9^BIT5) == P1 && (0XF9^BIT5) == O4)
		{
			time(&now);
			if (now - T4 >= 3)
			{
				keyNum = 12;
				key_scan[keyNum] = 2;
			}
		}
		if (0XF9 == P1 && (0XF9^BIT5) == O4)
		{
			keyNum = 12;
			time(&now);
			if (now - T4 >= 3)		longPress = 1;
			app->OnKeyScanReleased(M_KEY_SCAN_RELEASED, (WPARAM)keyNum, (LPARAM)longPress, 0);
			key_scan[keyNum] = 0;
		}

		O4 = P1;
	}


///////////////////////////////////////////////////////////
	if (5 == line)
	{
		////////////
		if ((0XF9^BIT0) == P1 && 0XF9 == O5)
		{
			time(&T5);
			keyNum = 7;
			key_scan[keyNum] = 1;
			app->OnKeyScanPressed(M_KEY_SCAN_PRESSED, (WPARAM)keyNum, 0, 0);
		}
		if ((0XF9^BIT0) == P1 && (0XF9^BIT0) == O5)
		{
			time(&now);
			if (now - T5 >= 3)
			{
				keyNum = 7;
				key_scan[keyNum] = 2;
			}
		}
		if (0XF9 == P1 && (0XF9^BIT0) == O5)
		{
			keyNum = 7;
			time(&now);
			if (now - T5 >= 3)		longPress = 1;
			app->OnKeyScanReleased(M_KEY_SCAN_RELEASED, (WPARAM)keyNum, (LPARAM)longPress, 0);
			key_scan[keyNum] = 0;
		}

		////////////X
		if ((0XF9^BIT5) == P1 && 0XF9 == O5)
		{
			time(&T5);
			keyNum = 11;//
			key_scan[keyNum] = 1;
			app->OnKeyScanPressed(M_KEY_SCAN_PRESSED, (WPARAM)keyNum, 0, 0);
		}
		if ((0XF9^BIT5) == P1 && (0XF9^BIT5) == O5)
		{
			time(&now);
			if (now - T5 >= 3)
			{
				keyNum = 11;//
				key_scan[keyNum] = 2;
			}
		}
		if (0XF9 == P1 && (0XF9^BIT5) == O5)
		{
			keyNum = 11;//
			time(&now);
			if (now - T5 >= 3)		longPress = 1;
			app->OnKeyScanReleased(M_KEY_SCAN_RELEASED, (WPARAM)keyNum, (LPARAM)longPress, 0);
			key_scan[keyNum] = 0;
		}

		O5 = P1;
	}


///////////////////////////////////////////////////////////
	if (6 == line)
	{
		////////////
		if ((0XF9^BIT5) == P1 && 0XF9 == O6)
		{
			time(&T6);
			keyNum = 6;
			key_scan[keyNum] = 1;
			app->OnKeyScanPressed(M_KEY_SCAN_PRESSED, (WPARAM)keyNum, 0, 0);
		}
		if ((0XF9^BIT5) == P1 && (0XF9^BIT5) == O6)
		{
			time(&now);
			if (now - T6 >= 3)
			{
				keyNum = 6;
				key_scan[keyNum] = 2;
			}
		}
		if (0XF9 == P1 && (0XF9^BIT5) == O6)
		{
			keyNum = 6;
			time(&now);
			if (now - T6 >= 3)		longPress = 1;
			app->OnKeyScanReleased(M_KEY_SCAN_RELEASED, (WPARAM)keyNum, (LPARAM)longPress, 0);
			key_scan[keyNum] = 0;
		}

		O6 = P1;
	}

	return 0;
}
*/



//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

HeartRate::HeartRate()
{
	//Init_HeartRate_Data();
}

HeartRate::~HeartRate()
{
}

// Polar_HR
unsigned char HeartRate::HeartRate_TEL_hardware_read(void)
{

#if 0
	unsigned char v = 0;
	unsigned char r = 0;

	if (-1 == i2c_fd)		{ printf("-1==i2c_fd\n"); return 0;}

	if (-1 == i2c_read_data(i2c_fd, PCA_9555_SLAVE_1, PCA_9555_INPUT_1, &v, sizeof(v)))
	{
		printf("(%s %d) i2c_read_data() FAIL\n", __FILE__, __LINE__);
	}
	if ((v & BIT4) == BIT4)		r = 1; 
	else						r = 0;

	//printf("(%s %d) BIT4=%d\n", __FILE__, __LINE__, r);
	return r;
#else

	//if (TEL_PIN)
	//printf("(%s %d) BIT4=%d\n", __FILE__, __LINE__, TEL_PIN);
	return TEL_PIN;
#endif

}

// CHR_Signal_GP
unsigned char HeartRate::HeartRate_HGP_hardware_read(void)
{

#if 0
	unsigned char v = 0;
	unsigned char r = 0;

	if (-1 == i2c_fd)		{ printf("-1==i2c_fd\n"); return 0;}
	if (-1 == i2c_read_data(i2c_fd, PCA_9555_SLAVE_1, PCA_9555_INPUT_1, &v, sizeof(v)))
	{
		printf("(%s %d) i2c_read_data() FAIL\n", __FILE__, __LINE__);
	}

	if ((v & BIT5) == BIT5)		r = 1; 
	else						r = 0;

	//printf("(%s %d) BIT5=%d\n", __FILE__, __LINE__, r);
	return r;
#else


	//printf("(%s %d) BIT5=%d\n", __FILE__, __LINE__, HGP_PIN);
	return HGP_PIN;
#endif

}

unsigned char HeartRate::PCA9555_hardware_read(void)	//by Simon.
{
	unsigned char v = 0;

	if (-1 == i2c_fd)		{ printf("-1==i2c_fd\n"); return 255; }
	if (-1 == i2c_read_data(i2c_fd, PCA_9555_SLAVE_1, PCA_9555_INPUT_1, &v, sizeof(v)))
	{
		printf("(%s %d) i2c_read_data() FAIL\n", __FILE__, __LINE__);
		return 255;
	}

	if ((v & BIT4) == BIT4){
		TEL_PIN = 1; 
	}else{
		TEL_PIN= 0;
	}
		
	if ((v & BIT5) == BIT5){
		HGP_PIN = 1; 
	}else{	
		HGP_PIN = 0;
	}

	if ((v & BIT6) == BIT6){
		SPU_PIN = 1; 
	}else{	
		SPU_PIN = 0;
	}

	return 0;
}

void HeartRate::HeartRate_Process(void)
{
	unsigned char i;
	unsigned char max_index;                      //緩衝區中最大值的序號
	unsigned char min_index;                      //緩衝區中最小值的序號
	unsigned char temp;
	unsigned short hr, deltaHr;
	hr = 0;

	//檢查采集到的無線心率，
	if(state.HR_SAMPLING_VALID_FLAG_TEL == 1){
		if(Times_Valid_TEL < MAX_VALID_TIMES)
		{
			Times_Valid_TEL++;
			if(Times_Lose_TEL)
			{
				Times_Lose_TEL--;
			}
		}
		for(i = 0; i < 3; i++)
		{
			HR_Buffer_TEL[i] = HR_Buffer_TEL[i + 1];
		}
		HR_Buffer_TEL[3] = HR_Sampling_TEL;
		state.HR_SAMPLING_VALID_FLAG_TEL = 0;
	}

	//檢查采集到的手握心率
	if(state.HR_SAMPLING_VALID_FLAG_HGP == 1){
		if(Times_Valid_HGP < MAX_VALID_TIMES)
		{
			Times_Valid_HGP++;
			if(Times_Lose_HGP)
			{
				Times_Lose_HGP--;
			};
		};
		for(i=0;i<3;i++){
			HR_Buffer_HGP[i] = HR_Buffer_HGP[i+1];
		}

		HR_Buffer_HGP[3] = HR_Sampling_HGP;
		state.HR_SAMPLING_VALID_FLAG_HGP = 0;
	}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

	//每到1.6秒計算一次心率
	Count_ticks_per_100ms --;
	if(Count_ticks_per_100ms == 0)
	{
		//Count_ticks_per_100ms = Count_ticks_reload;
		Count_ticks_per_100ms = 16;


///////////////////////////////////////////////////////////
		//無線心率

		if(Times_Valid_TEL >= MAX_VALID_TIMES)
		{
			//計算當前無線心率
			hr = 0;
			max_index = 0;
			min_index = 0;
			//為比較是否所有采樣一致准備
			temp = HR_Buffer_TEL[0];
			state.ALL_BUFFER_SAME_FLAG_TEL = 1;

			//濾波算法為四個采樣中去掉最大最小值后取平均值
			for (i = 0; i < 4; i++)
			{
				hr += HR_Buffer_TEL[i];
				if(HR_Buffer_TEL[i] > HR_Buffer_TEL[max_index])
				{
					max_index = i;
				};
				if(HR_Buffer_TEL[i] < HR_Buffer_TEL[min_index])
				{
					min_index = i;
				};
				if(temp != HR_Buffer_TEL[i])
				{
					//state  &= ~(1 << ALL_BUFFER_SAME_FLAG_TEL);   //檢查是否所有BUFFER完全相同
					state.ALL_BUFFER_SAME_FLAG_TEL = 0;
				};
			};

#if 1
/////////////////////////////////////////
			hr -= (HR_Buffer_TEL[max_index] + HR_Buffer_TEL[min_index]);
			hr /= 2;
#else
			//hr -= (HR_Buffer_TEL[max_index] + HR_Buffer_TEL[min_index]);
			hr /= 4;
#endif // 1



			if(state.ALL_BUFFER_SAME_FLAG_TEL == 1)
			{
				HeartRate_TEL = hr;
			}
			else
			{
				if(HeartRate_TEL > hr){				//by Simon
					deltaHr = HeartRate_TEL - hr;
				}else{
					deltaHr = hr - HeartRate_TEL;
				}
				if(deltaHr <= 2){
					HeartRate_TEL = hr;
				}else
					if(hr > HeartRate_TEL){
						HeartRate_TEL += (hr - HeartRate_TEL) / 3;
					}else{
						HeartRate_TEL -= (HeartRate_TEL - hr) / 3;
					};
			}
		}
		else  if(Times_Lose_TEL >= MAX_LOSE_TIMES)
		{
				for (i=0;i<4;i++)
				{
					HR_Buffer_TEL[i]=0;
				}
				Lose_Temp_TEL++;
				if(Lose_Temp_TEL > 3)
					HeartRate_TEL = 0;
		};

		if(Times_Lose_TEL < MAX_LOSE_TIMES){
			Times_Lose_TEL++;
		};




///////////////////////////////////////////////////////////
		//手握心率
		if(Times_Valid_HGP >= MAX_VALID_TIMES){	//計算當前手握心率
			hr = 0;
			max_index = 0;
			min_index = 0;
			temp = HR_Buffer_HGP[0];//為比較是否所有采樣一致准備
			state.ALL_BUFFER_SAME_FLAG_HGP = 1;
			
			//濾波算法為四個采樣中去掉最大最小值后取平均值
			for (i = 0; i < 4; i++){
				hr += HR_Buffer_HGP[i];
				if(HR_Buffer_HGP[i] > HR_Buffer_HGP[max_index]){
					max_index = i;
				};
				if(HR_Buffer_HGP[i] < HR_Buffer_HGP[min_index]){
					min_index = i;
				};
				if(temp != HR_Buffer_HGP[i]){
					state.ALL_BUFFER_SAME_FLAG_HGP = 0;   //檢查是否所有BUFFER完全相同
				};
			};
			hr -= (HR_Buffer_HGP[max_index] + HR_Buffer_HGP[min_index]);
			hr /= 2;
			if(HR_Buffer_HGP[min_index] > hr){			//by Simon
				deltaHr = HR_Buffer_HGP[min_index] - hr;
			}else{
				deltaHr = hr - HR_Buffer_HGP[min_index];
			}

			if(deltaHr > 10){						//by Simon
				if(HR_Buffer_HGP[min_index] > 80)
					hr = HR_Buffer_HGP[min_index];
				else
					hr = HR_Buffer_HGP[max_index];
			} 


			if(state.ALL_BUFFER_SAME_FLAG_HGP == 1){
				HeartRate_HGP = hr;
			}else
				if(HeartRate_HGP > hr){		//by Simon
					deltaHr = HeartRate_HGP - hr;
				}else{
					deltaHr = hr - HeartRate_HGP;
				}

				if(deltaHr <= 2){				//by Simon
					HeartRate_HGP = hr;
				}else
					if(hr > HeartRate_HGP){
						HeartRate_HGP += (hr - HeartRate_HGP) / 3;
					}else{
						HeartRate_HGP -= (HeartRate_HGP - hr) / 3;
					};
		}else
			if(Times_Lose_HGP >= MAX_LOSE_TIMES){
				for (i=0;i<4;i++){
					HR_Buffer_HGP[i]=0;
				}
				Lose_Temp_HGP++;
				if(Lose_Temp_HGP>3)HeartRate_HGP=0;
			};
		if(Times_Lose_HGP < MAX_LOSE_TIMES){
			Times_Lose_HGP++;
		};

		if(DisplayHeartRate > hr)
		{
		 	deltaHr = DisplayHeartRate - hr;	//by Simon
		}else{
			deltaHr = hr - DisplayHeartRate;	//by Simon
		}
	
		//if(abs(DisplayHeartRate-hr) > 6 ){
		if(deltaHr > 6 ){					//by Simon
			if(DisplayHeartRate < hr)
				Heart_Rate_up_Flag++;
			else
				Heart_Rate_down_Flag = 0;
			if(DisplayHeartRate < hr)
				Heart_Rate_down_Flag++;
			else
				Heart_Rate_up_Flag = 0;
		}else{
			Heart_Rate_down_Flag = 3; 
			Heart_Rate_up_Flag = 3;
		} 
   
		//無線优先
		if(HeartRate_TEL){
			hr = HeartRate_TEL;
			state.hr_ready = 1;//by Simon@2013/0401
			state.TEL_in_use = 1;//by Simon@2013/0401
		}else{
			if(HeartRate_HGP){
				hr = HeartRate_HGP;
				state.hr_ready = 1;//by Simon@2013/0401
				state.TEL_in_use = 0;	//by Simon@2013/0401
			}else{
				state.hr_ready = 0;//by Simon@2013/0401
				state.TEL_in_use = 0;	//by Simon@2013/0401		
				hr=0;
			}
		}
	
		//顯示作假
		if(DisplayHeartRate && (Heart_Rate_down_Flag == 3 || Heart_Rate_up_Flag == 3 )){
			DisplayHeartRate = hr;           //暫時這樣，再改
			Heart_Rate_down_Flag = 0;
			Heart_Rate_up_Flag = 0;
		}else
			if(DisplayHeartRate && Heart_Rate_down_Flag < 3 && Heart_Rate_down_Flag != 0){
			      if(hr < 100 && hr > 60){//(DisplayHeartRate < 80 && hr < 80)
					DisplayHeartRate=hr;
				}else{
					if(hr > 40)
					 DisplayHeartRate=DisplayHeartRate+5;
			      }
			}else
				if(DisplayHeartRate && Heart_Rate_up_Flag < 3 && Heart_Rate_up_Flag != 0){
					if(hr < 100 && hr > 60){//(DisplayHeartRate < 80 && hr < 80)
						DisplayHeartRate=hr;
					}else{
						if(hr > 40)
							DisplayHeartRate=DisplayHeartRate-5;
					}
				}else{ 
					DisplayHeartRate = hr;
					Heart_Rate_down_Flag = 0;
					Heart_Rate_up_Flag = 0;
				};
   
		if(Heart_Rate_up_Flag >= 3)Heart_Rate_up_Flag=0;

		if(Heart_Rate_down_Flag >= 3)
		Heart_Rate_down_Flag=0; 
      

		//無線优先
		if(HeartRate_TEL){
			DisplayHeartRate = HeartRate_TEL;
		}

		//檢查輸出范圍
		if(DisplayHeartRate > MAX_HEARTRATE)
		{
			DisplayHeartRate = MAX_HEARTRATE;
		}
		else if(DisplayHeartRate < MIN_HEARTRATE)
		{
			DisplayHeartRate = 0;
		};

		Times_Valid_TEL=MAX_VALID_TIMES-1;
		Times_Valid_HGP=MAX_VALID_TIMES-1;
	}
} 



void HeartRate::HeartRate_Sampling_TEL(void)	//this function will be ticked from the FPC Shceduler every 1ms.
{
	Sampling_Timer_TEL++;                                 //采樣定時器+1ms
	//采集心率的PIN腳狀態
	Pin_Buffer_TEL <<= 1;
	//if((SAMPLING_PIN & (1 << HR_TEL)) == (VALID_PULSE_TEL << HR_TEL))



	//無線心率輸入 硬體 部份
	//hardware read pin state	//by Simon

#if 1
	if(HeartRate_TEL_hardware_read()){
		Pin_Buffer_TEL++;
	}
#else
	if(TEL_PIN == 1){			//by Simon 2013/08/19
		Pin_Buffer_TEL++;
	}
#endif


	// 1KHZ的軟件濾波
	if((Pin_Buffer_TEL & BIT0) == (Pin_Buffer_TEL & BIT2))
	{
		if((Pin_Buffer_TEL & BIT0) != (Pin_Buffer_TEL & BIT1))
		{
			if(Pin_Buffer_TEL & BIT0)
			{
				Pin_Buffer_TEL |= BIT1;
			}
			else
			{
				Pin_Buffer_TEL &= ~BIT1;
			}
		}
	}

	//得到穩定的有效脈寬
	if(Pin_Buffer_TEL >= 0xFC || 0X3F == (Pin_Buffer_TEL & 0x3F))
	{

#if 0
// 發電機, CMOS_D8 / GPIO5[13] 

(QsApp.cpp 1967) Pin_Buffer_TEL=63, Pin_Buffer_TEL=1
(QsApp.cpp 1967) Pin_Buffer_TEL=127, Pin_Buffer_TEL=1
(QsApp.cpp 1967) Pin_Buffer_TEL=255, Pin_Buffer_TEL=1
(QsApp.cpp 1967) Pin_Buffer_TEL=255, Pin_Buffer_TEL=1
(QsApp.cpp 1967) Pin_Buffer_TEL=255, Pin_Buffer_TEL=1
(QsApp.cpp 1967) Pin_Buffer_TEL=255, Pin_Buffer_TEL=1
(QsApp.cpp 1967) Pin_Buffer_TEL=255, Pin_Buffer_TEL=1
(QsApp.cpp 1967) Pin_Buffer_TEL=255, Pin_Buffer_TEL=1
(QsApp.cpp 1967) Pin_Buffer_TEL=255, Pin_Buffer_TEL=1
(QsApp.cpp 1967) Pin_Buffer_TEL=255, Pin_Buffer_TEL=1

(QsApp.cpp 1967) Pin_Buffer_TEL=254, Pin_Buffer_TEL=0
(QsApp.cpp 1967) Pin_Buffer_TEL=252, Pin_Buffer_TEL=0
#endif


//printf("(%s %d) Pin_Buffer_TEL=%d, Pin_Buffer_TEL=%d\n", __FILE__, __LINE__, Pin_Buffer_TEL, TEL_PIN);
		Pulse_Width_TEL++;
	}

	//檢測有效的脈衝沿
	if(HR_Pause_Delay > 0)
	{
		HR_Pause_Delay--;
		if(0x3f == Pin_Buffer_TEL)
		{
			if(Sampling_Timer_TEL >= MIN_CYCLE){
				HR_Sampling_TEL = HR_Sampling_TEL;
				//state |=  1 << HR_SAMPLING_VALID_FLAG_TEL;
				state.HR_SAMPLING_VALID_FLAG_TEL = 1;
			}
			Sampling_Timer_TEL = 0;
			Pulse_Width_TEL = 0;
			//state &= ~(1 << CODING_FLAG_TEL);
			state.CODING_FLAG_TEL = 0;
		}
	}
	else
	{
		if(0x3f == Pin_Buffer_TEL)
		{
			if(Sampling_Timer_TEL < MIN_CYCLE)
			{
				//state |= (1 << CODING_FLAG_TEL);//在短時間內檢到多于一個脈衝沿時認為是CODING格式

	        		//在短時間內檢到多于一個脈衝沿時認為是CODING格式
				state.CODING_FLAG_TEL = 1;
			}
			else
			{
				if(Sampling_Timer_TEL <= MAX_CYCLE)
				{
					//采樣周期符合要求
					//if(state & (1 << CODING_FLAG_TEL)){
					if(state.CODING_FLAG_TEL == 1)
					{
						if(Pulse_Width_TEL >= MIN_PULSE_WIDTH_TEL_CODING && Pulse_Width_TEL <= MAX_PULSE_WIDTH_TEL_CODING)
						{
							//CODING時寬度符合要求，則采集有效
							HR_Sampling_TEL = 60000 / Sampling_Timer_TEL;
							//state |=  1 << HR_SAMPLING_VALID_FLAG_TEL;
							state.HR_SAMPLING_VALID_FLAG_TEL = 1;
						}
					}
					else
					{
						if(Pulse_Width_TEL >= MIN_PULSE_WIDTH_TEL_UNCODING && Pulse_Width_TEL <= MAX_PULSE_WIDTH_TEL_UNCODING)
						{
							//UNCODING時寬度符合要求，則采集有效
							if (0 != Sampling_Timer_TEL)
								HR_Sampling_TEL = 60000 / Sampling_Timer_TEL;
							//state |=  1 << HR_SAMPLING_VALID_FLAG_TEL;
							state.HR_SAMPLING_VALID_FLAG_TEL = 1;

//printf("(%s %d) Sampling_Timer_TEL=%d, Pulse_Width_TEL=%d, HR_Sampling_TEL=%d\n", __FILE__, __LINE__, Sampling_Timer_TEL, Pulse_Width_TEL, HR_Sampling_TEL);
						}
					}
				}//END 采樣周期最大值
			}

			//開始下次采集
			Sampling_Timer_TEL = 0;
			Pulse_Width_TEL = 0;
			//state &= ~(1 << CODING_FLAG_TEL);
			state.CODING_FLAG_TEL = 0;
		}
	}
}


void HeartRate::HeartRate_Sampling_HGP(void)	//this function will be ticked from the FPC Shceduler every 1ms.
{
	Sampling_Timer_HGP++;                                 //采樣定時器+1ms
	//采集心率的PIN腳狀態
	Pin_Buffer_HGP <<= 1;
  
	// 有線手握心率輸入 硬體 部份
#if 1
	if(HeartRate_HGP_hardware_read())
	{	//by Simon.
		Pin_Buffer_HGP++;
	}
#else
	if(HGP_PIN == 1)			//by Simon 2013/08/19
	{
		Pin_Buffer_HGP++;
	}
#endif

	// 1KHZ的軟件濾波
	if((Pin_Buffer_HGP & BIT0) == (Pin_Buffer_HGP & BIT2))
	{
		if((Pin_Buffer_HGP & BIT0) != (Pin_Buffer_HGP & BIT1))
		{
			if(Pin_Buffer_HGP & BIT0)
			{
				Pin_Buffer_HGP |= BIT1;
			}
			else
			{
				Pin_Buffer_HGP &= ~BIT1;
			}
		}
	}

	//得到穩定的有效脈寬
	if(Pin_Buffer_HGP >= 0xFC || 0X3F == (Pin_Buffer_HGP & 0x3F))
	{
		Pulse_Width_HGP++;
	}
	//檢測有效的脈衝沿
	if(0x3f == Pin_Buffer_HGP)
	{
		if(Sampling_Timer_HGP >= MIN_CYCLE && Sampling_Timer_HGP <= MAX_CYCLE)
		{
			//采樣周期符合要求
			if(Pulse_Width_HGP >= MIN_PULSE_WIDTH_HGP && Pulse_Width_HGP <= MAX_PULSE_WIDTH_HGP)
			{
				//寬度符合要求，則采集有效
				HR_Sampling_HGP = 60000 / Sampling_Timer_HGP;
				//state |=  1 << HR_SAMPLING_VALID_FLAG_HGP;
				state.HR_SAMPLING_VALID_FLAG_HGP = 1;
			}
		}
		//開始下次采集
		Sampling_Timer_HGP = 0;
		Pulse_Width_HGP = 0;
	}
}

//-----------------------------------------------------------------------------
//心率部分的初始化
void HeartRate::Init_HeartRate_Data(int fd)
{
	unsigned char i;

	i2c_fd = fd;

	for(i=0;i<4;i++){
		HR_Buffer_TEL[i] = 0;
		HR_Buffer_HGP[i] = 0;
	}
	HeartRate_TEL = 0;
	HeartRate_HGP = 0;
	Times_Valid_TEL = 0;
	Times_Valid_HGP = 0;
	Times_Lose_TEL = MAX_LOSE_TIMES;
	Times_Lose_HGP = MAX_LOSE_TIMES;
	Count_ticks_reload = MAX_CYCLE/100;// this should be 16
	Count_ticks_per_100ms = Count_ticks_reload;	// this should be 16
}



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CQsApp::CQsApp() :CApp()
{
	INIT_LIST_HEAD(&fpcCliQ.list);
	INIT_LIST_HEAD(&uartQ.list);

//////////////////////////////////////////////////////////////////////
	tables = &TABLES;
	hr = &HR;
	data = &DATA;
	memset((void *)&DATA, 0, sizeof(DATA));
	setup = &SETUP;
	rt = &RT;
	exception = &EXCEPTION;
	summary = &SUMMARY;
	update = &UPDTATE;
	memset((void *)&EXCEPTION, 0, sizeof(EXCEPTION));
	memset((void *)&SUMMARY, 0, sizeof(SUMMARY));
	memset((void *)&UPDTATE, 0, sizeof(UPDTATE));
	memset((void *)&RT, 0, sizeof(RT));
	WorkoutData_initialize();
	//setup->Workload_level = default_work_load_level;
	setup->Pace_level = 1;


//////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////
	hUart = CreateEvent(0, 1, 0, "UART");
	hFpcStart = CreateEvent(0, MANUAL_RESET, 0, "FPC_START");
	hFpcTick = CreateEvent(0, AUTO_RESET, 0, "FPC_TICK");

	tHr = 0;
	tKs = 0;
	tUi = 0;
	tTick = 0;
	tFpc = 0;
	tUart = 0;

	ttyUSB0 = -1;
	i2c0 = -1;
	pwm1 = -1;

	ipod = -1;
	bv = -1;
	gpio = -1;
	adc = -1;
}

CQsApp::~CQsApp()
{
	if (-1 != adc)														{ close(adc); adc = -1; }
	if (-1 != ipod)													{ close(ipod); ipod = -1; }
	if (-1 != bv)														{ close(bv); bv = -1; }

	if (-1 != i2c0)													{ close(i2c0), i2c0 = -1; }
	if (-1 != pwm1)													{ close(pwm1), pwm1 = -1; }
	if (-1 != gpio)													{ close(gpio), gpio = -1; }

	if (hUart)															CloseEvent(hUart);
	if(hFpcStart)														CloseEvent(hFpcStart);
	if(hFpcTick)														CloseEvent(hFpcTick);
}




//////////////////////////////////////////////////////////////////////
// message map & signal map
//////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CQsApp, CApp)
	//{{AFX_MSG_MAP(CQsApp)
	ON_MESSAGE(M_INIT_INSTANCE, &CQsApp::OnInitInstance)
	ON_MESSAGE(M_EXIT_INSTANCE, &CQsApp::OnExitInstance)
	ON_MESSAGE(M_TIMER, &CQsApp::OnTimer)
	ON_MESSAGE(M_UI_INCOMING, &CQsApp::OnUserMessage)
	ON_MESSAGE(M_KEY_SCAN_PRESSED, &CQsApp::OnKeyScanPressed)
	ON_MESSAGE(M_KEY_SCAN_RELEASED, &CQsApp::OnKeyScanReleased)
	ON_MESSAGE(M_UART_DATA_RCV, &CQsApp::OnUartDataRcv)
	ON_MESSAGE(M_LEFT_THUMB, &CQsApp::OnLeftThumb)
	ON_MESSAGE(M_RIGHT_THUMB, &CQsApp::OnRightThumb)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



//////////////////////////////////////////////////////////////////////
// message handlers
//////////////////////////////////////////////////////////////////////

unsigned int old_thumb_ts = 0;


LRESULT CQsApp::OnLeftThumb(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
	int up = 0;
	struct timeb tb;
	unsigned int now;

	if (0 == wParam)		up = 0;
	else					up = 1;

	if (IN_NORMAL == rt->workout_state)							return 0;
	if (READY_TO_SUMMARY == rt->workout_state)				return 0;
	if (READY_TO_FINISH == rt->workout_state)					return 0;
	if (READY_TO_COOL_DOWN == rt->workout_state)				return 0;

	ftime(&tb); now = tb.time * 1000 + tb.millitm;
	if (0 == old_thumb_ts || now - old_thumb_ts > 200)
	{
		if (1 == up)
		{
			data->ResistanceLevel = update->Workload_level;
			if (data->ResistanceLevel >= 30)						data->ResistanceLevel = 30 -1;
			if (data->ResistanceLevel <= 1)						data->ResistanceLevel = 1;

			rt->currnet_work_load_level = update->Workload_level = rt->Target_Workload_level = ++data->ResistanceLevel;
			exception->cmd.work_load_up = 1;
		}
		else
		{
			data->ResistanceLevel = update->Workload_level;
			if (data->ResistanceLevel >= 30)						data->ResistanceLevel = 30;
			if (data->ResistanceLevel <= 1)						data->ResistanceLevel = 2;

			rt->currnet_work_load_level = update->Workload_level = rt->Target_Workload_level = --data->ResistanceLevel;
			exception->cmd.work_load_up = 1;
		}
	}
	old_thumb_ts = now;
	return 1;
}

LRESULT CQsApp::OnRightThumb(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
	int up = 0;
	struct timeb tb;
	unsigned int now;

	if (0 == wParam)		up = 0;
	else					up = 1;

	if (IN_NORMAL == rt->workout_state)							return 0;
	if (READY_TO_SUMMARY == rt->workout_state)				return 0;
	if (READY_TO_FINISH == rt->workout_state)					return 0;
	if (READY_TO_COOL_DOWN == rt->workout_state)				return 0;

	ftime(&tb); now = tb.time * 1000 + tb.millitm;
	if (0 == old_thumb_ts || now - old_thumb_ts > 200)
	{
		if (1 == up)
		{
			if (update->Stride_length > 0)						data->StrideLength = update->Stride_length / 5;
			else												data->StrideLength = DEFAULT_STRIDE_LENGTH;

			if(data->StrideLength >= MAX_STRIDE_LENGTH)		data->StrideLength = MAX_STRIDE_LENGTH - 1;
			data->StrideLength++;
			update->Stride_length = rt->Target_Stride = data->StrideLength * 5;
			exception->cmd.stride_up = 1;
		}
		else
		{
			if (update->Stride_length > 0)						data->StrideLength = update->Stride_length / 5;
			else												data->StrideLength = DEFAULT_STRIDE_LENGTH;

			if(data->StrideLength <= MIN_STRIDE_LENGTH)		data->StrideLength = MIN_STRIDE_LENGTH + 1;
			data->StrideLength--;
			update->Stride_length = rt->Target_Stride = data->StrideLength * 5;
			exception->cmd.stride_up = 1;
		}
	}
	old_thumb_ts = now;
	return 1;
}


int CQsApp::AttachUart(const char *path, int baudrate)
{
	struct list_head *P, *Q;
	struct incoming_t *N;
	struct termios term;
	int v = 0, f = 0, fd = -1;
	int last_type;

	if (!TestUart(path))												{ dbg_printf(DBG_UART, "(%s %d) TestUart(%s) FAIL\n", __FILE__, __LINE__, path); return 0;}
	if (baudrate != 9600 && baudrate != 19200 && baudrate != 38400 && baudrate != 57600 && baudrate != 115200 && baudrate != 4800)
		baudrate = 9600;
	if (0 > (fd = open(path, O_RDWR | O_NOCTTY | O_NDELAY)))			{ dbg_printf(DBG_UART, "(%s %d) OPEN() FAIL\n", __FILE__, __LINE__); return 0; }
	if (!isatty(fd))														{ dbg_printf(DBG_UART, "(%s %d) !ISATTY() fd=%d\n", __FILE__, __LINE__, fd); close(fd); return 0; }

	memset(&term, 0, sizeof(term));
	term.c_cflag |= B9600, 	term.c_cflag |= CLOCAL, term.c_cflag |= CREAD, term.c_cflag &= ~PARENB, term.c_cflag &= ~CSTOPB;
	term.c_cflag &= ~CSIZE, term.c_cflag |= CS8, term.c_iflag = IGNPAR, term.c_cc[VMIN] = 1, term.c_cc[VTIME] = 0;
	if (115200 == baudrate)											cfsetispeed(&term, B115200), cfsetospeed(&term, B115200);
	else if (57600 == baudrate)										cfsetispeed(&term, B57600), cfsetospeed(&term, B57600);
	else if (38400 == baudrate)										cfsetispeed(&term, B38400), cfsetospeed(&term, B38400);
	else if (19200 == baudrate)										cfsetispeed(&term, B19200), cfsetospeed(&term, B19200);
	else if (9600 == baudrate)											cfsetispeed(&term, B9600), cfsetospeed(&term, B9600);
	else if (4800 == baudrate)											cfsetispeed(&term, B4800), cfsetospeed(&term, B4800);
	else																cfsetispeed(&term, B9600), cfsetospeed(&term, B9600);
	tcsetattr(fd, TCSANOW, &term); tcflush(fd, TCIFLUSH);
	v = fcntl(fd, F_GETFL, 0); fcntl(fd, F_SETFL, v | O_NONBLOCK);

	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &last_type);
	pthread_mutex_lock(&xUatrt);
	pthread_cleanup_push(CleanupLock, (void *)&xUatrt);
	pthread_setcanceltype(last_type, NULL);
	list_for_each_safe(P, Q, &uartQ.list)
	{
		N = list_entry(P, struct incoming_t, list); assert(0 != N);
		if (0 == strcmp(path, N->name))								{ f = 1; break; }
	}
	pthread_cleanup_pop(1);

	if (1 == f)														{ dbg_printf(DBG_UART, "(%s %d) 1 == f, CLOSE() fd=%d\n", __FILE__, __LINE__, fd); close(fd); return 0; }

	if (0 == (N = (struct incoming_t *)malloc(sizeof(struct incoming_t))))		{ dbg_printf(DBG_MALLOC, "(%s %d) MALLOC() FAIL\n", __FILE__, __LINE__); close(fd); return 0; }
	memset(N, 0, sizeof(struct incoming_t));
	N->fd = fd, strcpy(N->name, path);

	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &last_type);
	pthread_mutex_lock(&xUatrt);
	pthread_cleanup_push(CleanupLock, (void *)&xUatrt);
	pthread_setcanceltype(last_type, NULL);
	list_add_tail(&N->list, &uartQ.list);
	pthread_cleanup_pop(1);

	if (hUart)															SetEvent(hUart);
	return fd;
}

int CQsApp::TestUart(const char *path)
{
	struct stat lbuf;
	char pattern[128];
	int cflags = REG_EXTENDED;
	regex_t regx;
	int nmatch = 10;
	regmatch_t pmatch[10];

	if (0 == path)													return 0;
	memset(&regx, 0, sizeof(regex_t));
	strcpy(pattern, "^/dev/(tty[UMS]{1}[A-Z]{0,}[0-9]{1})$");
	if (0 != regcomp(&regx, pattern, cflags))								return 0;
	if (0 != regexec(&regx, path, nmatch, pmatch, 0))						{ dbg_printf(DBG_UART, "(%s %d) REGEXEC() FAIL\n", __FILE__, __LINE__); regfree(&regx); return 0; }
	regfree(&regx);

	if(0 > stat(path, &lbuf))											return 0;
	if(!S_ISCHR(lbuf.st_mode))											return 0;
	if(0 != access(path, R_OK))
	{
		printf("PLEASE RUN as ROOT\n");
		return 0;
	}
	return 1;
}


LRESULT CQsApp::OnInitInstance(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
	unsigned char ch = 0X0F;
	gpio_content_t ctx;

	int res = 0;
	unsigned char v;
	int err;
	struct gp_pwm_config_s attr;

	pthread_attr_t sched_attr;
	int fifo_prio;
	struct sched_param fifo_param;

	pthread_attr_init(&sched_attr);
	pthread_attr_setinheritsched(&sched_attr, PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setscope(&sched_attr, PTHREAD_SCOPE_PROCESS);
	pthread_attr_setschedpolicy(&sched_attr, SCHED_OTHER);
	fifo_prio = sched_get_priority_min(SCHED_OTHER);

	fifo_param.sched_priority = fifo_prio;
	pthread_attr_setschedparam(&sched_attr, &fifo_param);


	CApp::OnInitInstance(dwType, wParam, lParam, pResult);

	if (-1 == gpio)		gpio = open("/dev/gpio", O_RDWR);
	if (1 == product_type && -1 != gpio)
	{
		//ctx.pin_index = MK_GPIO_INDEX(0, 0, 10, 21);
		ctx.pin_index = MK_GPIO_INDEX(0, 0, 9, 20);
		ctx.value = 0;
		//if (-1 == ioctl(gpio, GPIO_IOCTL_SET_VALUE, &ctx))	printf("(%s %d) GPIO_IOCTL_SET_VALUE(0) FAIL\n", __FILE__, __LINE__);
		ctx.value = GPIO_PULL_FLOATING; ctx.debounce = 0;
		ioctl(gpio, GPIO_IOCTL_SET_INPUT, &ctx);

		if (-1 == adc)
		{
			adc = open("/dev/adc", O_RDWR);
			if (-1 == adc)
				printf("(%s %d) open('/dev/adc') Fail \n", __FILE__, __LINE__);
		}
	}
	if (2 == product_type && -1 != gpio)
	{
		//GPIO0[20] / PWM1
		//ctx.pin_index = MK_GPIO_INDEX(0, 2, 9, 20);
		//ctx.value = 1; ctx.debounce = 0;
		//ioctl(gpio, GPIO_IOCTL_SET_VALUE, &ctx);
	}

	if (2 == product_type)
	{
		if (-1 != gpio)
		{
			// BV=0, IPOD=1
			ctx.pin_index = MK_GPIO_INDEX(2, 0, 62, 19);
			ctx.value = 0;
			if (-1 == ioctl(gpio, GPIO_IOCTL_SET_VALUE, &ctx))					printf("(%s %d) FAIL, AUDIO_SEL0=1\n", __FILE__, __LINE__);
			else															printf("(%s %d) OK, AUDIO_SEL0=1\n", __FILE__, __LINE__);
		}
	}

	if (!tUart)
	{
		res = pthread_create(&tUart, NULL, UartThread, this);
		dbg_printf(DBG_THREAD, "UartThread() OK\n");
	}
	usleep(1000 * 100);
	res = AttachUart("/dev/ttyUSB0", 4800);
	if (res)																{ dbg_printf(DBG_UART, "(%s, %d) /dev/ttyUSB0 Attach OK\n", __FILE__, __LINE__); ttyUSB0 = res; }
	res = AttachUart("/dev/ttyUSB1", 9600);
	if (res)																{ dbg_printf(DBG_UART, "(%s, %d) /dev/ttyUSB1 Attach OK\n", __FILE__, __LINE__); bv = res; }
	if (-1 == ipod)
	{
		ipod = open("/dev/ttyUSB2", O_RDWR | O_NOCTTY | O_NDELAY);
		if (-1 != ipod)													SetBaudRate(ipod, 19200);
	}

	if (2 == product_type)
	{
		if (-1 == pwm1)		pwm1 = open("/dev/pwm1", O_RDWR);
		if (-1 != pwm1)
		{
			if (-1 == ioctl(pwm1, PWM_IOCTL_GET_ATTRIBUTE, &attr))			{ printf("PWM_IOCTL_GET_ATTRIBUTE, IOCTL() FAIL !!\n"); }
			attr.duty = 90; attr.freq = 20;
			if (-1 == ioctl(pwm1, PWM_IOCTL_SET_ATTRIBUTE, &attr))			{ printf("PWM_IOCTL_GET_ATTRIBUTE, IOCTL() FAIL !!\n"); }
			if (-1 == ioctl(pwm1, PWM_IOCTL_SET_ENABLE, 1))					{ printf("PWM_IOCTL_SET_ENABLE(1) FAIL\n"); }
		}
	}

	if (-1 == i2c0)		i2c0 = open("/dev/i2c-0", O_RDWR);
	if (-1 != i2c0)
	{
		if (-1 != i2c_read_data(i2c0, 0X44, 0X79, &v, 1))
		{
			prob_ms6257 = 1;
			printf("(%s %d) MS6257 PROBE OK, v=%d\n", __FILE__, __LINE__, v);
		}
		else	
		{
			printf("(%s %d) MS6257 PROBE FAIL\n", __FILE__, __LINE__);
		}
		if (prob_ms6257)
		{
			ipod_vol_tab(i2c0, IPOD_VOL_VAR);
			printf("(%s %d) IPOD VOL=%d\n", __FILE__, __LINE__, IPOD_VOL_VAR);
		}

		v = 0X00;
		res = i2c_write_data(i2c0, PCA_9555_SLAVE_0, PCA_9555_CONFIG_0, &v, 1);
		if (-1 == res)
			printf("(%s %d) PCA_9555_SLAVE_0 PROBE FAIL\n", __FILE__, __LINE__);
		if (-1 != res)
		{
			prob_pca955_0 = 1;

			res = i2c_read_data(i2c0, PCA_9555_SLAVE_1, PCA_9555_CONFIG_1, &v, 1);
			if (-1 == res)
				printf("(%s %d) PCA_9555_SLAVE_1 PROBE FAIL\n", __FILE__, __LINE__);
			if (-1 != res)
				prob_pca955_1 = 1;

			v = 0X00; res = i2c_write_data(i2c0, PCA_9555_SLAVE_2, PCA_9555_CONFIG_1, &v, 1);
			v = 0XFF; res = i2c_write_data(i2c0, PCA_9555_SLAVE_2, PCA_9555_CONFIG_0, &v, 1);
			if (-1 == res)
				printf("(%s %d) PCA_9555_SLAVE_2 PROBE FAIL\n", __FILE__, __LINE__);
			if (-1 != res)
				prob_pca955_2 = 1;
			// IN
			v |= BIT4; v |= BIT5; v |= BIT6;
			// OUT
			v &= ~BIT0; v &= ~BIT1; v &= ~BIT2; v &= ~BIT3;
			i2c_write_data(i2c0, PCA_9555_SLAVE_1, PCA_9555_CONFIG_1, &v, 1);

			i2c_read_data(i2c0, PCA_9555_SLAVE_1, PCA_9555_CONFIG_0, &v, 1);
			v |= BIT0; v |= BIT1; v |= BIT4;
			v &= ~BIT2; v &= ~BIT3; v &= ~BIT5; v &= ~BIT6; v &= ~BIT7;
			i2c_write_data(i2c0, PCA_9555_SLAVE_1, PCA_9555_CONFIG_0, &v, 1);

			i2c_read_data(i2c0, PCA_9555_SLAVE_1, PCA_9555_INPUT_0, &v, 1);
			v &= ~BIT6;	// BUZZ
			if (0 == product_type || 1 == product_type)
				v |= BIT5, v &= ~BIT7;		// 800-AS AUDIO_SEL0;
			i2c_write_data(i2c0, PCA_9555_SLAVE_1, PCA_9555_OUTPUT_0, &v, 1);

			i2c_read_data(i2c0, PCA_9555_SLAVE_1, PCA_9555_INPUT_0, &v, 1);
			v |= BIT2; v |= BIT3;
			if (0 == product_type || 1 == product_type)
				v |= BIT5;
			i2c_write_data(i2c0, PCA_9555_SLAVE_1, PCA_9555_OUTPUT_0, &v, 1);

			if (prob_pca955_0 && prob_pca955_1)
			{
				//if (0 == product_type || 1 == product_type)
				//{
					SetTimer(KEY_SCAN_TIMER_ID, 1000, KsTimer);
				//}
				/*if (!tKs && 2 == product_type)
				{
					err = pthread_create(&tKs, NULL, KsThread, this);
					dbg_printf(DBG_THREAD, "KsThread() OK\n");
				}*/
			}
			if (prob_pca955_1)
				hr->Init_HeartRate_Data(i2c0);

			if (2 == product_type && prob_pca955_1)
			{
				// 800 IPOD-Detect, SPU
				i2c_read_data(i2c0, PCA_9555_SLAVE_1, PCA_9555_CONFIG_1, &v, 1);
				// IN
				v |= BIT6; v |= BIT7;
				i2c_write_data(i2c0, PCA_9555_SLAVE_1, PCA_9555_CONFIG_1, &v, 1);

				i2c_read_data(i2c0, PCA_9555_SLAVE_1, PCA_9555_CONFIG_0, &v, 1);
				v &= ~BIT2; v &= ~BIT3; v &= ~BIT4; v &= ~BIT5;
				i2c_write_data(i2c0, PCA_9555_SLAVE_1, PCA_9555_CONFIG_0, &v, 1);
			}
		}
	}

	/*if (!tHr && 2 == product_type)
	{
		err = pthread_create(&tHr, NULL, HrThread, this);
		dbg_printf(DBG_THREAD, "HrThread() OK\n");
	}*/
	if (!tUi)
	{
		err = pthread_create(&tUi, &sched_attr, UiThread, this);
		dbg_printf(DBG_THREAD, "UiThread() OK\n");
	}
	if (!tFpc)
	{
		err = pthread_create(&tFpc, NULL, FpcThread, this);
		dbg_printf(DBG_THREAD, "FpcThread() OK\n");
	}
	if(!tTick)
	{
		err = pthread_create(&tTick, NULL, TickThread, this);
		dbg_printf(DBG_THREAD, "TickThread() OK\n");
	}


//////////////////////////////////////////////////////////////////////
	SendCmd(ttyUSB0, (int)SERIAL_CMD_INIT, 1, &ch);
	SetTimer(SD55_CMD_CC_INIT_ID, 301, 0);

	//SetTimer(8100, 2531, 0);
	return 1;
}

LRESULT CQsApp::OnExitInstance(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
	int i;
	struct list_head *P, *Q;
	struct incoming_t *N = 0;

	for (i = 0; i < 200; i++)
		KillTimer(100 + i);
	for (i = 0; i < 100; i++)
		KillTimer(SD55_CMD_ID + i);
	for (i = 0; i < 100; i++)
		KillTimer(FPC_SCHEDULER_1MS_ID + i);

	if (prob_pca955_1)
	{
		unsigned char V;

		i2c_read_data(i2c0, PCA_9555_SLAVE_1, PCA_9555_INPUT_0, &V, 1);
		V |= BIT2; V |= BIT3;
		if (0 == product_type || 1 == product_type)
			V |= BIT5;
		i2c_write_data(i2c0, PCA_9555_SLAVE_1, PCA_9555_OUTPUT_0, &V, 1);
	}


	if (tHr)
	{
		pthread_cancel(tHr);
		dbg_printf(DBG_THREAD, "(%s %d) TRYING WAIT() tHr...,   ", __FILE__, __LINE__);
		pthread_join(tHr, NULL);
		tHr = 0;
		dbg_printf(DBG_THREAD, "(%s %d) WAIT() tHr OK\n", __FILE__, __LINE__);
	}
	if (tKs)
	{
		pthread_cancel(tKs);
		dbg_printf(DBG_THREAD, "(%s %d) TRYING WAIT() tKs...,   ", __FILE__, __LINE__);
		pthread_join(tKs, NULL);
		tKs = 0;
		dbg_printf(DBG_THREAD, "(%s %d) WAIT() tKs OK\n", __FILE__, __LINE__);
	}

	if (tTick)
	{
		pthread_cancel(tTick);
		dbg_printf(DBG_THREAD, "(%s %d) TRYING WAIT() tTick...,   ", __FILE__, __LINE__);
		pthread_join(tTick, NULL);
		tTick = 0;
		dbg_printf(DBG_THREAD, "(%s %d) WAIT() tTick OK\n", __FILE__, __LINE__);
	}
	if (tUi)
	{
		pthread_cancel(tUi);
		dbg_printf(DBG_THREAD, "(%s %d) TRYING WAIT() tUi...,   ", __FILE__, __LINE__);
		pthread_join(tUi, NULL);
		tUi = 0;
		dbg_printf(DBG_THREAD, "(%s %d) WAIT() tUi OK\n", __FILE__, __LINE__);
		tUi = 0;
	}
	if (tUart)
	{
		pthread_cancel(tUart);
		dbg_printf(DBG_THREAD, "(%s %d) TRYING WAIT() tUart...,   ", __FILE__, __LINE__);
		pthread_join(tUart, NULL);
		tUart = 0;
		dbg_printf(DBG_THREAD, "(%s %d) WAIT() tUart OK\n", __FILE__, __LINE__);
	}

	CApp::OnExitInstance(dwType, wParam, lParam, pResult);

	if (-1 != i2c0)		{ close(i2c0), i2c0 = -1; }
	if (-1 != pwm1)
	{
		if (-1 == ioctl(pwm1, PWM_IOCTL_SET_ENABLE, 0))						{ printf("PWM_IOCTL_SET_ENABLE(0) FAIL\n"); }
		close(pwm1), pwm1 = -1;
	}
	list_for_each_safe(P, Q, &uartQ.list)
	{
		N = list_entry(P, struct incoming_t, list); assert(0 != N); close(N->fd); list_del(P); free(N);
	}

	return 1;
}


LRESULT CQsApp::OnTimer(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
	UINT TimerId = (UINT)wParam;


///////////////////////////////////////////////////////////
	/*if (8100 == TimerId)
	{
		SetTimer(BUZZ_ON_TIMER_ID, 31, BuzzOnTimer);
		//SetTimer(8100, 29003, 0);
		return 1;
	}*/

///////////////////////////////////////////////////////////
	if (FPC_SCHEDULER_10MS_ID == TimerId)
	{
		//SetTimer(TimerId, 11, 0);
		SetTimer(TimerId, 91, 0);
		data->Clear_Rpm_Delay++;
		return 1;
	}

///////////////////////////////////////////////////////////
	if (FPC_WORKOUT_IN_NORMAL_ID == TimerId)
	{
		rt->workout_state = IN_NORMAL;
		return 1;
	}

///////////////////////////////////////////////////////////
	if (BV_OK_BUZZ_ON_ID == TimerId)
	{
		unsigned char V = 0;

		i2c_read_data(i2c0, PCA_9555_SLAVE_1, PCA_9555_INPUT_0, &V, 1);
		V |= BIT6;
		if (0 == product_type || 1 == product_type)
			V |= BIT5;
		i2c_write_data(i2c0, PCA_9555_SLAVE_1, PCA_9555_OUTPUT_0, &V, 1);
		SetTimer(PWM1_TIMER_ID, 2002, 0);
		return 1;
	}

///////////////////////////////////////////////////////////
	if (PWM1_TIMER_ID == TimerId)
	{
		unsigned char V;

		if (prob_pca955_1)
		{
			i2c_read_data(i2c0, PCA_9555_SLAVE_1, PCA_9555_INPUT_0, &V, 1);
			V &= ~BIT6;
			if (0 == product_type || 1 == product_type)
				V |= BIT5;
			i2c_write_data(i2c0, PCA_9555_SLAVE_1, PCA_9555_OUTPUT_0, &V, 1);
		}
		return 1;
	}

///////////////////////////////////////////////////////////
	if (SD55_CMD_CC_INIT_ID == TimerId)
	{
		data->New_SPU_Pulse = 0;
		data->Drive_SPU_Interval = 0;

		if (-1 != ttyUSB0)
		{
			if (0 == product_type)
			{
				//SendCmd(ttyUSB0, (int)SERIAL_CMD_INIT, 1, &ch);
				SendCmd(ttyUSB0, (int)SERIAL_RESET_CONTORLER, 0, 0);
			}
		}

		SetTimer(SD55_CMD_RD_SPU_ID, 1000, 0);
		return 1;
	}

///////////////////////////////////////////////////////////
	if (SD55_CMD_RD_SPU_ID == TimerId)
	{
		static int once = 0;

		//////////////////////////////////////////////
		// 非 SD55
		if (2 == product_type || 1 == product_type)
		{
			if (0 == once)
			{
				once = 1;
				// MBREAK
				if (prob_pca955_1)
					SetTimer(SD55_CMD_DEFAULT_STRIDE_ID, 1, 0);

				if (prob_pca955_1)// && 2 != product_type)
				{
					// Process SPU
					SetTimer(FPC_SCHEDULER_200MS_ID, 200, SpuTimer);

					// HeartRate_Sampling_HGP
					//if (0 == product_type || 1 == product_type)
						SetTimer(FPC_SCHEDULER_1MS_ID, 1, HrTimer);
				}

				// Clear_Rpm_Delay++
				SetTimer(FPC_SCHEDULER_10MS_ID, 101, 0);
			}
			// 改由 1MS TIMER 算 SPU
			return 1;
		}

		//////////////////////////////////////////////
		// SD55
		if (0 == once)
		{
			once = 1;
			// Process SPU
			SetTimer(FPC_SCHEDULER_200MS_ID, 200, SpuTimer);

			// HeartRate_Sampling_HGP
			if (prob_pca955_1)
			{
				//if (0 == product_type || 1 == product_type)
					SetTimer(FPC_SCHEDULER_1MS_ID, 1, HrTimer);
			}

			// Clear_Rpm_Delay++
			SetTimer(FPC_SCHEDULER_10MS_ID, 101, 0);

			//SetTimer(SD55_CMD_DEFAULT_STRIDE_ID, 1000 * 30, 0);
			SetTimer(SD55_CMD_DEFAULT_STRIDE_ID, 10001, 0);
		}
		SendCmd(ttyUSB0, (int)SERIAL_CMD_RD_SPU_INTERVAL, 0, 0);
		SetTimer(SD55_CMD_RD_SPU_ID, 101, 0);
		return 1;
	}

///////////////////////////////////////////////////////////
	if (SD55_CMD_DEFAULT_STRIDE_ID == TimerId)
	{
		data->StrideLength = DEFAULT_STRIDE_LENGTH;
		update->Stride_length = rt->Target_Stride = data->StrideLength * 5;
		Set_Stride_Motor_Position(DEFAULT_STRIDE_LENGTH);
	
		rt->workLoad.current_load_level = update->Workload_level = data->ResistanceLevel = default_work_load_level;
		Set_WorkLoad_Motor_Position(default_work_load_level);
		return 1;
	}

/*
///////////////////////////////////////////////////////////
	if (FPC_SCHEDULER_1000MS_ID == TimerId)
	{
		SetTimer(TimerId, 998, 0);

		DataCollection();
		Calculate();
		return 1;
	}
*/

	return 1;
}

LRESULT CQsApp::OnUserMessage(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
	unsigned char *frame;

	if (0 == wParam)													return 1;
	pthread_cleanup_push(free, (void *)wParam);

	frame = ((unsigned char *)wParam) + 64;
	ProcessUserMessage((frame + 6), (int)lParam);

	pthread_cleanup_pop(1);

	return 1;
}

LRESULT CQsApp::OnKeyScanPressed(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
	if (prob_pca955_1)
		SetTimer(BUZZ_ON_TIMER_ID, 91, BuzzOnTimer);
	return 1;
}

LRESULT CQsApp::OnKeyScanReleased(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
	unsigned char top = 30;

	if (wParam == KS_RIGHT_TOP)				printf("KS_RIGHT_TOP\n");
	if (wParam == KS_RIGHT_CENTER)			printf("KS_RIGHT_CENTER\n");
	if (wParam == KS_LEFT_TOP)				printf("KS_LEFT_TOP\n");
	if (wParam == KS_LEFT_CENTER)			printf("KS_LEFT_CENTER\n");
	if (wParam == KS_LEFT_BOTTOM)			printf("KS_LEFT_BOTTOM\n");
	if (wParam == KS_RIGHT_BOTTOM)			printf("KS_RIGHT_BOTTOM\n");
	if (wParam == KS_START)					printf("KS_START\n");
	if (wParam == KS_STOP)					printf("KS_STOP\n");
	if (wParam == KS_DELETE)					printf("KS_DELETE\n");
	if (wParam == KS_WORKLOAD_UP)			printf("KS_WORKLOAD_UP\n");
	if (wParam == KS_WORKLOAD_DOWN)		printf("KS_WORKLOAD_DOWN\n");
	if (wParam == KS_STRIDE_UP)				printf("KS_STRIDE_UP\n");
	if (wParam == KS_STRIDE_DOWN)			printf("KS_STRIDE_DOWN\n");
	if (wParam == KS_NUM0 || wParam == KS_NUM1 || wParam == KS_NUM2 || wParam == KS_NUM3 || wParam == KS_NUM4 || wParam == KS_NUM5 || wParam == KS_NUM6 || wParam == KS_NUM7 || wParam == KS_NUM8 || wParam == KS_NUM9)
	{
		printf("KS_NUM: %d\n", wParam - KS_NUM0);
	}

	if (wParam == KS_NUM2)
	{
		unsigned char buff[] = {0X41, 0X48, 0X0D};

		if (-1 == bv)
			return 1;

		if (write(bv, (const void *)buff, sizeof(buff)) <= 0){}
		return 1;
	}

	if (wParam == KS_NUM0)
	{
		unsigned char V = 0;

		if (2 == product_type)
		{
			gpio_content_t ctx;

			ctx.pin_index = MK_GPIO_INDEX(2, 0, 62, 19);
			ctx.value = 0;
			if (-1 == ioctl(gpio, GPIO_IOCTL_SET_VALUE, &ctx))					printf("(%s %d) FAIL, AUDIO_SEL0\n", __FILE__, __LINE__);
			else															printf("(%s %d) OK, AUDIO_SEL0=0\n", __FILE__, __LINE__);
			return 1;
		}
		if(!prob_pca955_1)
			return 1;
		i2c_read_data(i2c0, PCA_9555_SLAVE_1, PCA_9555_INPUT_0, &V, 1);
		V &= ~BIT7;
		if (0 == product_type || 1 == product_type)
			V |= BIT5;
		i2c_write_data(i2c0, PCA_9555_SLAVE_1, PCA_9555_OUTPUT_0, &V, 1);

		return 1;

	}
	if (wParam == KS_NUM1)
	{
		unsigned char V = 0;

		if (2 == product_type)
		{
			gpio_content_t ctx;

			ctx.pin_index = MK_GPIO_INDEX(2, 0, 62, 19);
			ctx.value = 1;
			if (-1 == ioctl(gpio, GPIO_IOCTL_SET_VALUE, &ctx))					printf("(%s %d) FAIL, AUDIO_SEL0\n", __FILE__, __LINE__);
			else															printf("(%s %d) OK, AUDIO_SEL0=1\n", __FILE__, __LINE__);
			return 1;
		}

		if(!prob_pca955_1)
			return 1;
		i2c_read_data(i2c0, PCA_9555_SLAVE_1, PCA_9555_INPUT_0, &V, 1);
		V |= BIT7;
		if (0 == product_type || 1 == product_type)
			V |= BIT5;
		i2c_write_data(i2c0, PCA_9555_SLAVE_1, PCA_9555_OUTPUT_0, &V, 1);

		return 1;
	}


	if (wParam == KS_WORKLOAD_UP)
	{
		if (IPOD_VOL_VAR < 20)
		{
			IPOD_VOL_VAR++;
			if (prob_ms6257)				ipod_vol_tab(i2c0, IPOD_VOL_VAR);
		}
printf("(%s %d) IPOD_VOL_VAR=%d\n", __FILE__, __LINE__, IPOD_VOL_VAR);
		return 1;
	}
	if (wParam == KS_WORKLOAD_DOWN)
	{
		if (IPOD_VOL_VAR > 0)
		{
			IPOD_VOL_VAR--;
			if (prob_ms6257)				ipod_vol_tab(i2c0, IPOD_VOL_VAR);
		}
printf("(%s %d) IPOD_VOL_VAR=%d\n", __FILE__, __LINE__, IPOD_VOL_VAR);
		return 1;
	}


	if (wParam == KS_RIGHT_TOP)
	{
		if (0 != product_type)						return 0;

		if (READY_TO_SUMMARY == rt->workout_state)	return 0;
		if (READY_TO_FINISH == rt->workout_state)		return 0;
		if (READY_TO_COOL_DOWN == rt->workout_state)	return 0;

		if (update->Stride_length > 0)					data->StrideLength = update->Stride_length / 5;
		else											data->StrideLength = DEFAULT_STRIDE_LENGTH;

		if(data->StrideLength >= MAX_STRIDE_LENGTH)	data->StrideLength = MAX_STRIDE_LENGTH - 1;
		data->StrideLength++;
		update->Stride_length = rt->Target_Stride = data->StrideLength * 5;
		exception->cmd.stride_up = 1;
		Set_Stride_Motor_Position(data->StrideLength);

printf("(%s %d) STRIDE_UP=%d %d\n", __FILE__, __LINE__, data->StrideLength, rt->Target_Stride);
		return 1;

	}
	if (wParam == KS_RIGHT_CENTER)
	{
		if (0 != product_type)						return 0;

		if (READY_TO_SUMMARY == rt->workout_state)	return 0;
		if (READY_TO_FINISH == rt->workout_state)		return 0;
		if (READY_TO_COOL_DOWN == rt->workout_state)	return 0;

		if (update->Stride_length > 0)					data->StrideLength = update->Stride_length / 5;
		else											data->StrideLength = DEFAULT_STRIDE_LENGTH;

		if(data->StrideLength <= MIN_STRIDE_LENGTH)	data->StrideLength = MIN_STRIDE_LENGTH + 1;
		data->StrideLength--;
		update->Stride_length = rt->Target_Stride = data->StrideLength * 5;
		exception->cmd.stride_up = 1;
		Set_Stride_Motor_Position(data->StrideLength);

printf("(%s %d) STRIDE_DOWN=%d %d\n", __FILE__, __LINE__, data->StrideLength, rt->Target_Stride);
		return 1;
	}
	if (wParam == KS_RIGHT_BOTTOM)
	{
		if (IN_NORMAL == rt->workout_state || READY_TO_FINISH == rt->workout_state)
		{
			exception->cmd.start = 1;
			exception->cmd.stop = 0;
			exception->cmd.pause = 0;
			TellFpcStart();
			return 0;
		}

		exception->cmd.pause = 0;
		return 0;
	}
	if (wParam == KS_LEFT_BOTTOM)
	{
		if (IN_NORMAL == rt->workout_state || READY_TO_FINISH == rt->workout_state)
		{
			return 0;
		}

		if (0 ==  exception->cmd.pause)
		{
			exception->cmd.stop = 0;
			exception->cmd.pause = 1;
		}
		else
		{
			exception->cmd.stop = 1;
			exception->cmd.pause = 0;

		}
		return 0;
	}
	if (wParam == KS_LEFT_TOP)
	{
		if (READY_TO_SUMMARY == rt->workout_state)	return 0;
		if (READY_TO_FINISH == rt->workout_state)		return 0;
		if (READY_TO_COOL_DOWN == rt->workout_state)	return 0;
		data->ResistanceLevel = update->Workload_level;
		if (data->ResistanceLevel >= top)				data->ResistanceLevel = top - 1;
		if (data->ResistanceLevel <= 1)					data->ResistanceLevel = 1;

		rt->currnet_work_load_level = update->Workload_level = rt->Target_Workload_level = ++data->ResistanceLevel;
		exception->cmd.work_load_up = 1;
		Set_WorkLoad_Motor_Position(data->ResistanceLevel);
printf("(%s %d) WORKLOAD_UP=%d %d\n", __FILE__, __LINE__, data->ResistanceLevel, update->Workload_level);
		return 0;
	}
	if (wParam == KS_LEFT_CENTER)
	{
		if (READY_TO_SUMMARY == rt->workout_state)	return 0;
		if (READY_TO_FINISH == rt->workout_state)		return 0;
		if (READY_TO_COOL_DOWN == rt->workout_state)	return 0;
		data->ResistanceLevel = update->Workload_level;
		if (data->ResistanceLevel >= top)				data->ResistanceLevel = top;
		if (data->ResistanceLevel <= 1)					data->ResistanceLevel = 2;

		rt->currnet_work_load_level = update->Workload_level = rt->Target_Workload_level = --data->ResistanceLevel;
		exception->cmd.work_load_up = 1;
printf("(%s %d) WORKLOAD_DOWN=%d %d\n", __FILE__, __LINE__, data->ResistanceLevel, update->Workload_level);
		Set_WorkLoad_Motor_Position(data->ResistanceLevel);
		return 0;
	}

	return 1;
}

void OnTtyUsbChar(int infd, unsigned char ch)
{
	CQsApp *app = (CQsApp *)AfxGetApp();

	static unsigned char buff[1024 * 2];
	static unsigned char *p = buff;
	static unsigned char LEN = 0;
	int cs = 0;
	int i;
	static unsigned char CMD;


	//F5 + D2 + 5A + SUM
	if (0 == (p - buff))
	{
		if (ch != SD55_FRAME_HEADER)
		{
			LEN = 0; p = buff;
			return;
		}
		CMD = nCmd;
		*p++ = ch;
		return;
	}
	if (1 == (p - buff))
	{
		*p++ = ch;
		LEN = LO_UINT8(ch);
		return;
	}

	if ((p - buff) < (LEN + 1))
	{
		*p++ = ch;
		return;
	}

///////////////////////////////////
	*p++ = ch;
	cs = 0;
	for (i = 0; i < LEN + 1; i++)
		cs += buff[i + 1];
	cs &= 0XFF;
	if (0 == cs)
	{
		sd55_ok = 1;
//printf("CMD=%d\n", CMD);

		switch (CMD)
		{
		default:
			break;
		case SERIAL_CMD_INIT:
			if(buff[2] != SD55_FRAME_YES){	}
			break;
		case SERIAL_RESET_CONTORLER:
			if(buff[2] != SD55_FRAME_YES){}
			break;
		case SERIAL_CMD_WR_RESIST_POS:		// 0x01
		case SERIAL_CMD_WR_STRIDE_POS:		// 0x03
		case SERIAL_CMD_WR_INCLINE_POS:	// 0x05
		case SERIAL_CMD_MOTOR_CTRL:		// 0x09
		case SERIAL_CMD_INIT_STEP:			// 0x0A
			if(buff[2] != SD55_FRAME_YES){}
			break;
		// 0x08 --> 讀狀態
		case SERIAL_CMD_RD_STATE:
			app->data->State = buff[2];
			break;
		// 0x06 --> 讀高度馬達位置
		case SERIAL_CMD_RD_INCLINE_POS:
			break;
		// 0x02 --> 讀阻力馬達位置
		case SERIAL_CMD_RD_RESIST_POS:
			app->data->ResistancePosition = buff[2];
			break;
		// 0x04 --> 讀跨步馬達位置
		case SERIAL_CMD_RD_STRIDE_POS:
			app->data->StridePosition = buff[2];
			break;

		// 0x07 --> 讀 SPU INTERVAL
		case SERIAL_CMD_RD_SPU_INTERVAL:
			i = BUILD_UINT32(buff[5], buff[4], buff[3], buff[2]);
			if (i < 32768)				app->data->Drive_SPU_Interval = i;
			else						app->data->Drive_SPU_Interval = 0;
			if(app->data->Drive_SPU_Interval)
				app->data->New_SPU_Pulse = 1;
			break;

		// 0x0B --> 讀取步數
		case SERIAL_CMD_RD_STEP:
			i = BUILD_UINT32(buff[5], buff[4], buff[3], buff[2]);
			if (i < 32768)				app->data->Step = i;
			else						app->data->Step = 0;
			break;
		}
	}
	else
	{
//printf("CS NG\n");
	}

	LEN = 0; p = buff;
	return;
}

LRESULT CQsApp::OnUartDataRcv(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
	int len;
	char path[128];
	unsigned char *buff = (unsigned char *)wParam + 64;

	memcpy(&len, (void *)wParam, sizeof(int));
	memset(path, 0, sizeof(path)), strcpy(path, ((char *)wParam) + sizeof(int));

	if (0 == wParam)													return 1;
	pthread_cleanup_push(free, (void *)wParam);

	if (0 == strcmp(path, "/dev/ttyUSB0"))
	{
		for (int i = 0; i < len; i++)											OnTtyUsbChar((int)lParam, buff[i]);
		goto baiout;
	}

	if (0 == strcmp(path, "/dev/ttyUSB1"))
	{
		printf("(%s %d) BV_OK\n", __FILE__, __LINE__);


		SetTimer(BV_OK_BUZZ_ON_ID, 300, 0);
		//SetTimer(BV_OK_BUZZ_OFF_ID, 200 + 2000, 0);
		goto baiout;
	}

baiout:
	pthread_cleanup_pop(1);
	return 1;
}


/*
LRESULT CQsApp::OnTick(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
	return 1;
}
*/

void CQsApp::InitWorkout(unsigned short default_min)
{
	if(setup->Workout_time_1000 == 255)
		setup->Workout_time_1000 = 0;
	if(setup->Workout_time == 255)
		setup->Workout_time = 0;

	if(setup->Workout_time_1000 == 0)
	{
		if(setup->Workout_time == 0)
		{
			if(default_min > 0)
			{
				rt->total_workout_time = 60 * default_min;
				update->Time_remaining_1000 = rt->total_workout_time / 1000;
				update->Time_remaining = rt->total_workout_time % 1000;
			}
			else
			{
				update->Time_remaining_1000 = 0;
				update->Time_remaining = 0;
			}
		}
		else
		{
			rt->total_workout_time = setup->Workout_time_1000*1000 + setup->Workout_time;
			update->Time_remaining 	 = setup->Workout_time;
			update->Time_remaining_1000 = setup->Workout_time_1000;
		}
	}
	else
	{
			rt->total_workout_time = setup->Workout_time_1000*1000 + setup->Workout_time;
			update->Time_remaining  = setup->Workout_time;
			update->Time_remaining_1000 = setup->Workout_time_1000;
	}	

	update->Time_elapsed_1000 = 0;
	update->Time_elapsed = 0;	

	exception->cmd.pause  = 0;
	exception->cmd.stop  = 0;
	exception->cmd.resume  = 0;
	exception->cmd.start  = 1;
	rt->total_workout_time_tick = rt->total_workout_time * FP_TICKS_PER_SECOND;
	rt->exception_result = EXCEPTION_CONTINUE;
}

void CQsApp::Update_GUI_bar(void)
{
	unsigned char i,j;
	unsigned char total_bar_count;
	total_bar_count = rt->total_segment+1;
	
	if(exception->cmd.auto_populate_pace == 1)
	{
		for(i=rt->segment_index; i<rt->workLoad_TableUsed ; i++)
		{
			rt->workPace_Table[i] = update->Pace_RPM;
		}
		rt->workPace_Table[i] = update->Pace_RPM;
	}
	else
	{
		rt->workPace_Table[rt->segment_index] = update->Pace_RPM;
	}

	if(total_bar_count >= GUI_window_size)
	{
		rt->workPace_Table[rt->segment_index] = update->Pace_RPM;
		if(rt->segment_index > HOLD_STILL_INDEX){
			rt->GUI_Bar_window_Right = rt->segment_index + (GUI_window_size-HOLD_STILL_INDEX-1);
			if(rt->GUI_Bar_window_Right >= total_bar_count){
				rt->GUI_Bar_window_Right = total_bar_count;
				rt->GUI_Bar_window_Left = rt->GUI_Bar_window_Right - GUI_window_size;
				rt->GUI_Bar_window_index = rt->segment_index - rt->GUI_Bar_window_Left;
			}else{
				rt->GUI_Bar_window_Left = (rt->GUI_Bar_window_Right +1) - GUI_window_size;
				rt->GUI_Bar_window_index = HOLD_STILL_INDEX;
			}
		}else{
			rt->GUI_Bar_window_index = rt->segment_index;
		}
		
		update->workload_index = rt->GUI_Bar_window_index;
		for(i=rt->GUI_Bar_window_Left,j=0; j<GUI_window_size;i++, j++){
			if(exception->cmd.hr_cruise == 0){
				update->Workload_bar[j] = 
					rt->workLoad_Table[i];//_barwork_load[i];
			}else{
				update->Workload_bar[j] = 
					rt->workLoad_Table_cruise[i];
			}
			update->Pace_bar[j] =  
				rt->workPace_Table[i];
		}
		if(j<(GUI_window_size-1)){
			update->Workload_bar[j] = 1;
			update->Pace_bar[j] =  0;
		}		
	}else{
		rt->workPace_Table[rt->segment_index] = update->Pace_RPM;
		rt->GUI_Bar_window_index = rt->segment_index;
		update->workload_index = rt->GUI_Bar_window_index;
		for(i=0;i<=rt->workLoad_TableUsed;i++){
			if(exception->cmd.hr_cruise == 0){
				update->Workload_bar[i] = rt->workLoad_Table[i];
			}else{
				update->Workload_bar[i] = 
					rt->workLoad_Table_cruise[i];
			}
			update->Pace_bar[i] =  
				rt->workPace_Table[i];//_barwork_load[i];
		}
	}
	if(exception->cmd.hr_cruise == 1){
		rt->workLoad_Table[rt->segment_index-1] = 
				rt->workLoad_Table_cruise[rt->segment_index-1];
	}else{
		rt->workLoad_Table_cruise[rt->segment_index] =
				rt->workLoad_Table[rt->segment_index];
	}
}

void CQsApp::InitSegment(unsigned char segment_time_min)
{
	unsigned char i;

	rt->segment_index = 0;
	exception->cmd.pause = 0; 
	rt->no_heart_rate_detect = 1000;
	rt->tick_1sec_per100ms_reload = 10;
	rt->tick_1sec_per100ms = rt->tick_1sec_per100ms_reload;
	rt->tick_30sec_reload = FP_TICKS_PER_SECOND * 30;

	rt->tick_30sec = rt->tick_30sec_reload;

	if (0 == segment_time_min)
		segment_time_min = 20;
	rt->total_segment = rt->total_workout_time / (segment_time_min*60);
	rt->workLoad_TableUsed = rt->total_segment;

	if(rt->workLoad_TableUsed > MAX_SEGMENTS)
		rt->workLoad_TableUsed  = MAX_SEGMENTS;

	for(i = 0; i < rt->workLoad_TableUsed; i++)
	{
		rt->segmentTime_Table[i] = segment_time_min*60;
	}

	if(setup->Workload_level == 0)
		setup->Workload_level = 1;
	if(setup->Workload_level == 255)
		setup->Workload_level = 1;
	update->Workload_level = rt->Target_Workload_level = rt->workLoad.current_load_level = setup->Workload_level;


	rt->GUI_Bar_window_index = 0;	
	rt->GUI_Bar_window_Left = 0;
	rt->GUI_Bar_window_Right = rt->GUI_Bar_window_Left + GUI_window_size-1;

	for(i = 0; i < rt->workLoad_TableUsed ; i++)
	{
		rt->workLoad_Table[i] = setup->Workload_level;
		rt->workPace_Table[i] = 0;
		rt->segmentTime_Table[i] = segment_time_min*60;
	}

	rt->segmentTime_Table[i] = 60;
	rt->workLoad_Table[i] = 1;
	rt->workPace_Table[i] = 0;
	Update_GUI_bar();
}

void CQsApp::AdjustMachineWorkLoad(unsigned char adj_type, unsigned char specify_level)
{
	switch(adj_type)
	{
	case SEGMENT_LOAD_ADJ:
		Set_WorkLoad_Motor_Position(rt->workLoad.current_load_level);
		break;
 	case SEGMENT_INDEX_LOAD_ADJ:
 		Set_WorkLoad_Motor_Position(rt->workLoad_Table[rt->segment_index]);
 		break;
 	case INDEXED_TARGET_LOAD_ADJ_WLClass2:	//add for Weight loss Rolling Hills by Simon@20120325
 		Set_WorkLoad_Motor_Position(rt->workLoad_Table[rt->segment_index]);
 		break;
 	case INDEXED_TARGET_LOAD_ADJ_WLClass3:	//add for Weight loss Rolling Hills by Simon@20120325
 		Set_WorkLoad_Motor_Position(rt->workLoad_Table[rt->segment_index]);
 		break;
 	case INDEXED_TARGET_LOAD_ADJ_WLClass20:	//add for Weight loss Rolling Hills by Simon@20120325
 		Set_WorkLoad_Motor_Position(rt->workLoad_Table[rt->segment_index]);
 		break;
	default:
		break;
	}
}

unsigned char CQsApp::GetLower10WattWorkLoad_cruise(void)
{
	unsigned char targetLevel,currentLevel;
	signed short deltaWatt;	

	currentLevel = rt->workLoad_Table_cruise[rt->segment_index];
	targetLevel = currentLevel;
	deltaWatt = 0;
	if(targetLevel >1){
		do{
			if(targetLevel == 1)break;
			targetLevel --;
			deltaWatt = rt->base_cruise_watt - tables->Get_60rpm_Watt_ByLevel(targetLevel);
			if(targetLevel == 1)break;
		}while(deltaWatt < 10);
		if(deltaWatt == 10){
			rt->base_cruise_watt -= 10;
		}
		if(deltaWatt > 10){
			if((currentLevel-1) > targetLevel)targetLevel++;
			rt->base_cruise_watt -= 10;
		}
		if(deltaWatt < 10){
			if(currentLevel > targetLevel)rt->base_cruise_watt -= 10;
		}		
	}
	return targetLevel;
}

unsigned char CQsApp::GetHiger10WattWorkLoad_cruise(void)
{
	unsigned char targetLevel,currentLevel;
	signed short deltaWatt;

	currentLevel = rt->workLoad_Table_cruise[rt->segment_index];
	targetLevel = currentLevel;
	deltaWatt = 0;
	if(targetLevel <30){
		do{
			if(targetLevel == 30)break;
			targetLevel ++;
			deltaWatt = tables->Get_60rpm_Watt_ByLevel(targetLevel) - rt->base_cruise_watt;
			if(targetLevel == 30)break;
		}while(deltaWatt < 10);
		if(deltaWatt == 10){
			rt->base_cruise_watt += 10;
		}
		if(deltaWatt > 10){
			if((targetLevel-1) > currentLevel)targetLevel--;
			rt->base_cruise_watt += 10;
		}
		if(deltaWatt < 10){
			if(targetLevel > currentLevel)rt->base_cruise_watt += 10;
		}		
	}
	return targetLevel;
}

void CQsApp::HR_Cruise(void)
{
	unsigned char i,calc_level;
 	unsigned char cruise_hr;
	unsigned char adjust_required = 0;

	rt->current_heart_rate = hr->DisplayHeartRate;
	update->Target_heart_rate = setup->Target_heart_rate;
	
	cruise_hr = setup->Target_heart_rate;

	if(rt->current_heart_rate > cruise_hr)
	{
		rt->deltaHR = rt->current_heart_rate - cruise_hr;
		if(rt->deltaHR > 12)
		{
			rt->exception_result = EXCEPTION_OVERHR_BREAK;
			return;
		}
	}

	adjust_required = 0;
	if(rt->current_heart_rate > (cruise_hr+2)){
		adjust_required = 1;
	}							
	if(rt->current_heart_rate < (cruise_hr -2)){
		adjust_required = 1;
	}
	if(adjust_required == 1)
	{
		if(rt->tick_30sec >0)rt->tick_30sec --;
	}
	else
	{
		rt->tick_30sec = rt->tick_30sec_reload;
	}
	if(rt->tick_30sec==0)
	{
		calc_level = 0;
		rt->tick_30sec = rt->tick_30sec_reload;
		adjust_required = 0;
		if(rt->current_heart_rate > (cruise_hr+2)){
			calc_level = GetLower10WattWorkLoad_cruise();
			adjust_required = 1;
		}
		if(rt->current_heart_rate < (cruise_hr-2)){							
			calc_level = GetHiger10WattWorkLoad_cruise();
			adjust_required = 1;
		}
		if(adjust_required == 1)
		{
			for(i=rt->segment_index;i<rt->workLoad_TableUsed;i++)
			{
				rt->workLoad_Table_cruise[i] = calc_level;
			}
			rt->workLoad.current_load_level = calc_level;
			update->Workload_level = rt->workLoad.current_load_level; //added by simon@20130509
			AdjustMachineWorkLoad(SEGMENT_LOAD_ADJ, 0);
			Update_GUI_bar();
		}
	}
	///////////////////////////////////////////////////////////////////////////
}




void CQsApp::Set_Stride_Motor_Position(unsigned char position)
{
	unsigned char ch[2];

	if(position < MIN_STRIDE_LENGTH || position > MAX_STRIDE_LENGTH)
		return;
	data->StrideLength = position;

	ch[0] = 0X0F;
	ch[1] = pStride_Len_BitMap_Table[data->StrideLength -MIN_STRIDE_LENGTH];

	if (-1 != ttyUSB0)
		SendCmd(ttyUSB0, (int)SERIAL_CMD_WR_STRIDE_POS, 2, ch);
}

extern int gp_pwm_set_config(int handle, struct gp_pwm_config_s *config);

void CQsApp::Set_WorkLoad_Motor_Position(unsigned char position)
{
	unsigned char ch;

	if(position < MIN_RESISTANCE_LEVEL || position > MAX_RESISTANCE_LEVEL)
		return;

	if (2 == product_type)
	{
		struct gp_pwm_config_s attr;

		if (-1 == pwm1)
			return;
		ch = pGsResist_Level_Table[position - 1];
		//if (-1 == ioctl(pwm1, PWM_IOCTL_GET_ATTRIBUTE, &attr))				{ printf("PWM_IOCTL_GET_ATTRIBUTE, IOCTL() FAIL !!\n"); }
		attr.duty = ch;
		attr.freq = 20;
		attr.pin_index = 0;
		if (-1 == ioctl(pwm1, PWM_IOCTL_SET_ATTRIBUTE, &attr))				{ printf("PWM_IOCTL_SET_ATTRIBUTE, IOCTL() FAIL !!\n"); }
		if (-1 == ioctl(pwm1, PWM_IOCTL_SET_ENABLE, 1))						{ printf("PWM_IOCTL_SET_ENABLE(1) FAIL\n"); }
		return;
	}

	//if (sd55_ok)
	if (0 == product_type)
	{
		ch = pSD55_Resist_Level_Table[position - 1];
		SendCmd(ttyUSB0, (int)SERIAL_CMD_WR_RESIST_POS, 1, &ch);
		return;
	}

	if (-1 == adc)
		return;
	ch = pAsResist_Level_Table[position - 1];
	T_AD = ch;
	SetTimer(FPC_PROBE_CURRENT_AD_ID, 7, ProbeAdTimer);
}

void CQsApp::Init_SPU(void)
{
	unsigned char i;

	for(i = 0; i < 3; i++)		data->Rpm_Buffer[i] = 0;
	data->Buffer_Index = 0;
	data->Rpm = 0;
	data->Clear_Rpm_Delay = 0;
}

void CQsApp::Process_SPU(void)
{
	unsigned short rpm = 0;
	unsigned short rpm_sum = 0;
	unsigned char i;

	if(data->New_SPU_Pulse == 1)
	{
		data->New_SPU_Pulse = 0;

		if (data->Drive_SPU_Interval == 0)
			data->Drive_SPU_Interval = 1;
		rpm = (60000 / data->Drive_SPU_Interval);
		if(2 == product_type)
				rpm /= 52;

///////////////////////////////
		if(rpm > 0)
			data->Clear_Rpm_Delay = 0;
		//if(rpm > 30)
		if(rpm > 5)
		{
			if(0 == data->Rpm_Buffer[0] && 0 == data->Rpm_Buffer[1] && 0 == data->Rpm_Buffer[2])
			{
				for(i = 0; i < 3; i++)
				{
					data->Rpm_Buffer[i] = rpm;
					data->Buffer_Index = 3;
				}
			}
			else
			{
				data->Rpm_Buffer[data->Buffer_Index] = rpm;
				data->Buffer_Index++;                                         
			}
			if(data->Buffer_Index >= 3)
			{
				rpm_sum = 0;
				for(i=0; i<3; i++)rpm_sum += data->Rpm_Buffer[i];
				data->Rpm = rpm_sum/3;                                

				if(data->Rpm > 199)
					data->Rpm = 199;
				data->Buffer_Index = 0;
			}
		}
		else
		{
			data->Rpm = (unsigned short)(rpm);
			for(i = 0; i < 3; i++)
				data->Rpm_Buffer[i] = 0;
			data->Buffer_Index = 0;
		}
	}

	if(data->Clear_Rpm_Delay > 70)
		Init_SPU();


	update->Pace_RPM = data->Rpm;


}

void CQsApp::TellFpcWait(void)
{
	ResetEvent(hFpcStart);
	return;
}	

void CQsApp::TellFpcStart(void)
{
	WakeupFpc();
	return;
}

void CQsApp::FpcWaitForStart(void)
{
	WaitForSingleObject(hFpcStart, INFINITE);
	return;
}

void CQsApp::WakeupFpc(void)
{
	if(exception->cmd.start != 1)
		return;
	SetEvent(hFpcStart);
	return;
}

void CQsApp::WaitForProgramTick(void)
{
	WaitForSingleObject(hFpcTick, INFINITE);
	return;
}

void CQsApp::Update_Workout_Time_Elapsed(void)
{
	rt->elapsed_time++;
	if(rt->elapsed_time >= 1000)
	{
		rt->elapsed_time = 0;
		rt->elapsed_time_1000++;
	}

	update->Time_elapsed ++;
	if(update->Time_elapsed >= 1000)
	{
		update->Time_elapsed = 0;
		update->Time_elapsed_1000++;
	}

	if(update->Time_elapsed_1000 == 5)
	{
		//90:00 = 90*60 = 5400;
		if(update->Time_elapsed == 400)
		{
			update->Time_elapsed = 0;
			update->Time_elapsed_1000 = 0;
		}
	}
}

void CQsApp::Update_Workout_Time_Remaining(void)
{
	if(update->Segment_time > 1)
		update->Segment_time--;
	
	if(update->Time_remaining >0)
		update->Time_remaining --;
	
	if(update->Time_remaining == 0)
	{
		if(update->Time_remaining_1000 >0)
		{
			update->Time_remaining_1000--;
			update->Time_remaining = 1000;
		}
	}
}

unsigned char CQsApp::ProgramStart(void)
{
	WorkoutData_initialize();
	memset((void *)&rt->total, 0, sizeof(struct WorkOutSummaryData));
	memset((void *)&rt->cooldown, 0, sizeof(struct WorkOutSummaryData));
	memset((void *)&rt->workout, 0, sizeof(struct WorkOutSummaryData));
	memset((void *)&rt->warmup, 0, sizeof(struct WorkOutSummaryData));
//rt->Target_Stride

	SetTimer(FPC_SCHEDULER_1000MS_ID, 1, CalculateTimer);


	setup->Age = 35;
	setup->Weight = 150;
	setup->Workout_time_1000 = (99 * 60) / 1000;
	setup->Workout_time = (99 * 60) % 1000;
	setup->Calorie_goal = 90;
	setup->Segments = 20;
	setup->Workload_level = default_work_load_level;
	setup->Pace_level = 1;

	rt->workout_state = IN_RUNNING;
	rt->Average_Rpm_ACC = 0;
	rt->Average_Heart_Rate_ACC = 0;
	rt->waste_rpm_elapsed_time = 0;
	rt->waste_hr_elapsed_time = 0;
	rt->workLoad.current_load_level = rt->Target_Workload_level = default_work_load_level;
	data->StrideLength = DEFAULT_STRIDE_LENGTH;


	rt->watt_calc_mod	= CALC_BY_CURRENT_LOAD_LEVEL;
	rt->load_adj_mode = SEGMENT_LOAD_ADJ;
	exception->cmd.auto_populate_pace = 1;

	InitWorkout(90);
	InitSegment(1);

	AdjustMachineWorkLoad(SEGMENT_LOAD_ADJ, 0);
	rt->exception_result = EXCEPTION_CONTINUE;

	while(rt->total_workout_time_tick > 0 && rt->exception_result == EXCEPTION_CONTINUE)
	{
		update->Segment_time = 60;
 		rt->segment_time_tick = FP_TICKS_PER_SECOND * update->Segment_time;

/*
///////////////////
// jason note
		if(exception->cmd.hr_cruise == 1)
		{
			rt->workLoad.current_load_level = rt->workLoad_Table[rt->segment_index];
			update->Workload_level = rt->workLoad.current_load_level;
			AdjustMachineWorkLoad(SEGMENT_LOAD_ADJ, 0);
		}
*/


		while(rt->segment_time_tick > 0 && rt->exception_result == EXCEPTION_CONTINUE)
		{
			rt->current_heart_rate = hr->DisplayHeartRate;

			rt->exception_result = ExceptionHandler(MANUAL_QUICK_START);
			if(rt->exception_result == EXCEPTION_COOLDOWN)				{ break; }
			if(rt->exception_result == EXCEPTION_BREAK)						{ break; }

			if(exception->cmd.pause == 0)
			{
				if(rt->segment_time_tick > 0)
				{
					rt->segment_time_tick--;
				}
				if(rt->segment_time_tick == 0)
				{
					rt->segment_index++;
				}
				if(rt->total_workout_time_tick > 0)
				{
					rt->total_workout_time_tick --;
				}

				if(rt->tick_1sec_per100ms > 0)								rt->tick_1sec_per100ms--;	

				if(rt->tick_1sec_per100ms == 0)
				{
					rt->tick_1sec_per100ms = rt->tick_1sec_per100ms_reload;

					Update_Workout_Time_Elapsed();
					Update_Workout_Time_Remaining();
					
					Update_GUI_bar();
				}

				if(exception->cmd.hr_cruise == 1)
					HR_Cruise();
			}

			WaitForProgramTick();
		}
	}


	EndofProgram(MANUAL_QUICK_START);
	rt->summary_state.has_cooldown = 0;
	//CalculateSummaryData();

	KillTimer(FPC_SCHEDULER_1000MS_ID);


	SetTimer(SD55_CMD_DEFAULT_STRIDE_ID, 1, 0);
	SetTimer(FPC_WORKOUT_IN_NORMAL_ID, 100, 0);
	TellFpcWait();
	return 0;
}

void CQsApp::Default_CoolDown(void)
{
	rt->workout_state = READY_TO_COOL_DOWN;

	rt->Average_Heart_Rate = 0;
	rt->Max_HeartRate = 0;
	rt->Average_Rpm = 0;
	rt->Max_Rpm	 = 0;
	
 	update->Segment_time	 = 120;
 	update->Segment_time_1000 = 0;
 	update->Time_remaining = 120;
 	update->Time_remaining_1000 = 0;

	rt->total_workout_time_tick = FP_TICKS_PER_SECOND * default_cool_down_time * 60;
	rt->workLoad.current_load_level = default_work_load_level;
	update->Workload_level = rt->workLoad.current_load_level;
	//AdjustMachineWorkLoad(SEGMENT_LOAD_ADJ, 0);

	rt->tick_1sec_per100ms = rt->tick_1sec_per100ms_reload;
	rt->exception_result = EXCEPTION_CONTINUE;
	rt->segment_index = rt->workLoad_TableUsed;

	while(rt->total_workout_time_tick > 0 && rt->exception_result == EXCEPTION_CONTINUE)
	{
		rt->exception_result = ExceptionHandler(DEFAULT_COOL_DOWN);
		if(exception->cmd.pause == 0)
		{
			///////////////////////////////////////////////////////////////////////////
			//Workout Time Update
			if(rt->total_workout_time_tick > 0)
				rt->total_workout_time_tick --;

			//////////////////////////////////////////////////////
			//DataScreen Time / Bar Update
			//timer elapsed 1 sec after 100 tick count;

			if(rt->tick_1sec_per100ms >0)
				rt->tick_1sec_per100ms--;	
			if(rt->tick_1sec_per100ms == 0)
			{
				rt->tick_1sec_per100ms = rt->tick_1sec_per100ms_reload;

				Update_Workout_Time_Elapsed();
				Update_Workout_Time_Remaining();	

// jason
CollectCooldownSummaryData();

				Update_GUI_bar();
			}
			//////////////////////////////////////////////////////
		}

		//if (EXCEPTION_BREAK != rt->exception_result)
		WaitForProgramTick();
	}	

	//Summary data collection
	rt->workout_state = READY_TO_FINISH;
	CollectCooldownSummaryData();


	SetTimer(SD55_CMD_DEFAULT_STRIDE_ID, 1, 0);
}

struct BiPace ppiPaceTable[]= {
	{50, 60},//1
	{50, 65},//2
	{50, 68},//3
	{50, 70},//4
	{50, 73},//5
	{55, 75},//6
	{55, 78},//7
	{55, 80},//8
	{55, 83},//9
	{55, 85},//10
	{60, 88},//11
	{60, 90},//12
	{60, 93},//13
	{60, 95},//14
	{60, 98},//15
	{65,100},//16
	{65,103},//17
	{65,105},//18
	{65,108},//19
	{65,110} //20
};

unsigned char CQsApp::GetWorkPace_of_ppi(unsigned char level)
{
	return ppiPaceTable[level-1].pace1;
}
unsigned char CQsApp::GetWorkPaceLevel_of_ppi(unsigned char rpm)
{
	unsigned char i;
	for(i=0;i<20;i++){
		if(ppiPaceTable[i].pace1 >= rpm){
			if(ppiPaceTable[i].pace1 == rpm)return (i+1);
			if(i<=1){
				return 1;
			}else{
				return(i-1);
			}
		}
	}
	return (20);
}

void CQsApp::Default_CoolDown_pace_process(unsigned char for_program)
{
	unsigned char i;
	float fl;
	switch(for_program){
	case CUSTOM_PACE:	
	case CUSTOM_ULTRA:		
	case CUSTOM_HILLS:		
	case PERFORMANCE_PACE_RAMP:
	case PERFORMANCE_PACE_INTERVAL:
	case WEIGHT_LOSS_WALK_AND_RUN:
		for(i=rt->segment_index; i < rt->workLoad_TableUsed ; i++){
			rt->workPace_Table[i] = rt->workPace_Table[rt->segment_index];
		}
		fl = rt->Average_Rpm;
		fl = fl*0.4;
		setup->Pace_level = GetWorkPaceLevel_of_ppi((unsigned char) fl);	
		update->Pace_level = setup->Pace_level;
		break;
	}
	Default_CoolDown();
}

void CQsApp::CollectCooldownSummaryData(void)	// Every secods, Calculate the Calories, Mets, Distance, HeartRate called from FPC_Scheduler_Task() in Fast_task.c 
{
	rt->cooldown.Time_elapsed	 = rt->elapsed_time;
	rt->cooldown.Time_elapsed_1000 = rt->elapsed_time_1000;
	
	rt->cooldown.Calories_burned  = rt->Calories;
	rt->cooldown.Distance_metric = rt->Distance_metric;
	rt->cooldown.Distance_imperial  = rt->Distance_imperial;

	rt->cooldown.Max_HeartRate = rt->Max_HeartRate;
	rt->cooldown.Average_Heart_Rate = rt->Average_Heart_Rate;
	rt->cooldown.Average_Heart_Rate_base = rt->Average_Heart_Rate_base;
	
	rt->cooldown.Max_Rpm	 = rt->Max_Rpm;
	rt->cooldown.Average_Rpm	 = rt->Average_Rpm;	
	rt->cooldown.Average_Rpm_base = rt->Average_Rpm_base;	

	rt->summary_state.has_cooldown = 1;
}

void CQsApp::GetIndexedWorkload_WLClass2_populate(void)
{
	unsigned char i;

	for(i=rt->segment_index;i < rt->workLoad_TableUsed;i++){
		switch(i%2){
		case 0://rest
			rt->workLoad_Table[i] = rt->WLClass2_LoadTable[rt->workLoad.current_load_level-1].work_load[0];
			break;
		case 1://work
			rt->workLoad_Table[i] = rt->WLClass2_LoadTable[rt->workLoad.current_load_level-1].work_load[1];
			break;
		}	
	}
}

void CQsApp::GetIndexedWorkload_WLClass20_Pace(void)
{
	unsigned char i;
	for(i=rt->segment_index;i< rt->workLoad_TableUsed;i++){
		rt->workLoad_Table[i]=
			rt->WLClass20_LoadTable[rt->workLoad.current_load_level-1].work_load[i];
		rt->workPace_Table[i]=
			rt->WLClass20_PaceTable[rt->workLoad.current_load_level-1].work_pace[i];
	}
}

void CQsApp::GetIndexedWorkload_WLClass20_populate(void)
{
	unsigned char i;
	for(i=rt->segment_index;i<rt->workLoad_TableUsed;i++){
		rt->workLoad_Table[i]= rt->WLClass20_LoadTable[rt->workLoad.current_load_level-1].work_load[i];
	}	
}

unsigned char CQsApp::GetRestSegmentLoadLevel_by_level(unsigned char work_level)
{
	float work_wattF;
	unsigned char j;
	unsigned short rest_watt;
	if(work_level >1){
		work_wattF = tables->Get_60rpm_Watt_ByLevel(work_level);
	}else{
		work_wattF = tables->Get_60rpm_Watt_ByLevel(1);
	}
	rest_watt = (unsigned short)(work_wattF*0.65);
	for(j=work_level;j>0;j--){
		if(tables->Get_60rpm_Watt_ByLevel(j) <= rest_watt)break;
	}
	if(j==0)j=1;
	return j;
}

void CQsApp::PopulateWorkLoad_Intervals_hrci3(void)
{
	unsigned char i,work_level,rest_level;	
	work_level = rt->adjusted_Target_Workload;
	rest_level = GetRestSegmentLoadLevel_by_level(work_level);	
	for(i=rt->segment_index; i<rt->total_segment ;i++){
		if(i==0){
			rt->workLoad_Table[i] = work_level;
		}else{
			if(i >= rt->segment_index){
				switch(i%3){
				case 1://WORK_SEGMENT:
					rt->workLoad_Table[i] = work_level;
					break;
				
				case 2://WORK_SEGMENT:
					rt->workLoad_Table[i] = work_level;
					break;

				case 0://REST_SEGMENT:
					rt->workLoad_Table[i] = rest_level;
					break;	
				}
			}
		}
	}
	rt->workLoad.current_load_level = rt->workLoad_Table[rt->segment_index];
	update->Workload_level = rt->workLoad.current_load_level;
}

struct WLClass3 Cardio360_LoadTable[]=
{
	//低阻力 	中阻力	高阻力
	//class 
	//0	1,	2
	{{1,	4,	7}},
	{{1,	7,	8}},
	{{1,	8,	9}},
	{{1,	10,	11}},
	{{1,	11,	12}},
	{{4,	11,	12}},
	{{4,	11,	12}},
	{{4,	12,	12}},
	{{4,	12,	13}},
	{{4,	12,	14}},
	{{7,	12,	14}},
	{{7,	13,	14}},
	{{7,	13,	15}},
	{{7,	13,	15}},
	{{8,	13,	15}},
	{{8,	13,	16}},
	{{8,	14,	16}},
	{{8,	14,	17}},
	{{10,	14,	18}},
	{{10,	14,	18}},
	{{10,	14,	18}},
	{{10,	16,	18}},
	{{11,	16,	18}},
	{{11,	16,	19}},
	{{11,	16,	19}},
	{{11,	16,	19}},
	{{12,	16,	19}},
	{{12,	17,	20}},
	{{12,	18,	20}},
	{{14,	19,	21}}
};

void CQsApp::GetIndexedWorkload_C360(void)
{
	unsigned char i;
	for(i=rt->segment_index;i< rt->workLoad_TableUsed;i++){
 		rt->workLoad_Table[i]=
 			Cardio360_LoadTable[rt->workLoad.current_load_level-1].work_load[rt->C360_Table[rt->random_index[i]].work_load_class];
	}

	rt->indexed_Target_Workload = rt->workLoad_Table[rt->segment_index];
}

unsigned char CQsApp::ExceptionHandler(unsigned char for_program)
{
	unsigned char i;

//if a stop cmd is issued from GUI
	if(exception->cmd.stop == 1)
	{
		exception->cmd.stop = 0;
		return EXCEPTION_BREAK;
	}

//if a cool down cmd is issued from GUI
	if(exception->cmd.cool_down == 1)
	{
		exception->cmd.cool_down = 0;
		return EXCEPTION_COOLDOWN;
	}

//Workload change cmd is issued from GUI
	if(exception->cmd.work_load_up == 1)
	{
		exception->cmd.work_load_up = 0;

		if(rt->Target_Workload_level > 0)
		{
			rt->workLoad.current_load_level = rt->Target_Workload_level;
			update->Workload_level = rt->workLoad.current_load_level;
			rt->Target_Workload_level = 0;
			switch(rt->load_adj_mode)
			{
			case INDEXED_TARGET_LOAD_ADJ_WLClass2:
				GetIndexedWorkload_WLClass2_populate();
				AdjustMachineWorkLoad(INDEXED_TARGET_LOAD_ADJ_WLClass2,0);
				break;

			case INDEXED_TARGET_LOAD_ADJ_WLClass20:
				switch(for_program)
				{
				case PERFORMANCE_CARDIO_CHALLENGE:
					GetIndexedWorkload_WLClass20_Pace();
					break;
				default:
					GetIndexedWorkload_WLClass20_populate();
					break;
				}
				AdjustMachineWorkLoad(INDEXED_TARGET_LOAD_ADJ_WLClass20, rt->segment_index);
				break;
			case INDEXED_TARGET_LOAD_ADJ_WLClass3:
				GetIndexedWorkload_C360();
				if(exception->cmd.hr_cruise == 0)
				{
					AdjustMachineWorkLoad(INDEXED_TARGET_LOAD_ADJ_WLClass3, rt->segment_index);
				}
				break;
				
			default:
				switch(for_program)
				{
				case CUSTOM_HILLS:
					rt->workLoad_Table[rt->segment_index] = rt->workLoad.current_load_level;
					AdjustMachineWorkLoad(SEGMENT_LOAD_ADJ, rt->segment_index);
					break;
				case CUSTOM_ULTRA:
					rt->workLoad_Table[rt->segment_index] = rt->workLoad.current_load_level;
					AdjustMachineWorkLoad(SEGMENT_LOAD_ADJ, rt->segment_index);
					break;
				case HRC_INTERVALS:
					rt->adjusted_Target_Workload = rt->workLoad.current_load_level;
					PopulateWorkLoad_Intervals_hrci3();
					AdjustMachineWorkLoad(SEGMENT_LOAD_ADJ,0);
					break;
				case MANUAL_MANUAL:
					for(i=rt->segment_index;i<rt->total_segment;i++)
					{
						rt->workLoad_Table[i] = rt->workLoad.current_load_level;	
					}
					AdjustMachineWorkLoad(SEGMENT_LOAD_ADJ,0);
					break;

				default:
					for(i=rt->segment_index;i<rt->total_segment;i++)
					{
						rt->workLoad_Table[i] = rt->workLoad.current_load_level;
					}
					AdjustMachineWorkLoad(SEGMENT_LOAD_ADJ,0);
					break;
				}
				break;	
			}			
		}//else{
	}


#if 1
	if(exception->cmd.stride_up == 1)
	{
 		exception->cmd.stride_up = 0;
		if(rt->Target_Stride > 0)
		{
			data->StrideLength = rt->Target_Stride / 5;
			rt->Target_Stride = 0;
		}
		else
		{
			if(data->StrideLength < MAX_STRIDE_LENGTH)	
				data->StrideLength += 1;
		}
		if(data->StrideLength > 0)
		{
			Set_Stride_Motor_Position(data->StrideLength);
		}
	}
#endif // 



///////
//Target_Pace_Level
	if(exception->cmd.pace_up == 1)
	{
		exception->cmd.pace_up = 0;
		if(rt->Target_Pace_Level > 0)
		{
			setup->Pace_level = rt->Target_Pace_Level;
			switch(rt->pace_adj_mode)
			{
			case INDEXED_TARGET_PACE_ADJ_WLClass2:
				setup->Pace_level = rt->Target_Pace_Level;
				UpdateIndexedPaceTable();
				break;

			case INDEXED_TARGET_PACE_ADJ_WLClass20:
				setup->Pace_level = rt->Target_Pace_Level;
				UpdateIndexedPaceTable_WLCLASS20();
				break;

			default:
				switch(for_program)
				{
				case CUSTOM_HILLS:
					for(i=rt->segment_index;i<rt->workLoad_TableUsed;i++){
						rt->workPace_Table[i] = 
							GetWorkPace_of_ppi(rt->Target_Pace_Level);
					}
					break;
					
				case CUSTOM_PACE:
					rt->workPace_Table[rt->segment_index] = GetWorkPace_of_ppi(rt->Target_Pace_Level);
					break;
				
				case CUSTOM_ULTRA:
					rt->workPace_Table[rt->segment_index] = GetWorkPace_of_ppi(rt->Target_Pace_Level);
					break;
				}
			}
			rt->Target_Pace_Level = 0;
		}else{
		}
	}
	return EXCEPTION_CONTINUE;
}	

void CQsApp::UpdateIndexedPaceTable(void)
{
	unsigned char i,j;
	j=0;
	for(i=0; i < rt->workLoad_TableUsed ; i++){
		switch(j){
		case 0:
			j=1;
			if(i >= rt->segment_index)
				rt->workPace_Table[i] = rt->BiPaceTable[rt->Target_Pace_Level-1].pace0;
			break;
		case 1:
			j=0;
			if(i >= rt->segment_index)
				rt->workPace_Table[i] = rt->BiPaceTable[rt->Target_Pace_Level-1].pace1;
			break;
		}
	}
}

void CQsApp::UpdateIndexedPaceTable_WLCLASS20(void)
{
	unsigned char i;
	for(i=rt->segment_index; i < rt->workLoad_TableUsed ; i++){
		rt->workPace_Table[i] = rt->WLClass20_PaceTable[rt->Target_Pace_Level-1].work_pace[i];
	}
}

void CQsApp::EndofProgram(unsigned char for_program)
{
//user over hr
	if(rt->exception_result == EXCEPTION_OVERHR_BREAK)
	{
		rt->workout_state = READY_TO_COOL_DOWN;
	}

//user stop
	if(rt->exception_result == EXCEPTION_BREAK)
	{
	}

//user cooldown
	if(rt->exception_result == EXCEPTION_COOLDOWN)
		rt->workout_state = READY_TO_COOL_DOWN;

//normal timeout
	if(rt->total_workout_time_tick == 0)
	{
		switch(for_program)
		{
		case CARDIO360_DEMO:
			rt->workout_state = READY_TO_FINISH;
			rt->exception_result = EXCEPTION_BREAK;
			break;
		default:
			rt->workout_state = READY_TO_COOL_DOWN;
			break;
		}
	}

//normal time up conditiob
	if(rt->exception_result == EXCEPTION_CONTINUE)
	{
		switch(for_program)
		{
		case CARDIO360_DEMO:
			rt->workout_state = READY_TO_FINISH;
			rt->exception_result = EXCEPTION_BREAK;
			break;
		default:
			rt->workout_state = READY_TO_COOL_DOWN;
			break;
		}
	}

	exception->cmd.auto_stride = 0;
 	exception->cmd.pause = 0;

	switch(rt->exception_result)
	{
	case EXCEPTION_BREAK:
		if(rt->summary_state.did_warmup == 1)
		{
			if(rt->summary_state.has_warmup == 0)
			{
				CollectWarmUpSummaryData();
			}
			else
			{
				CollectWorkOutSummaryData();
			}
		}else
		{
			CollectWorkOutSummaryData();
		}

///////////////////////////////////////////////////////////
		rt->workout_state = READY_TO_SUMMARY;
		SetTimer(SD55_CMD_DEFAULT_STRIDE_ID, 1, 0);
		break;

	case EXCEPTION_OVERHR_BREAK:
		if(rt->summary_state.did_warmup == 1)
		{
			if(rt->summary_state.has_warmup==0)
			{
				CollectWarmUpSummaryData();
			}
			else
			{
				CollectWorkOutSummaryData();
			}
		}
		else
		{
			CollectWorkOutSummaryData();
		}		
		Default_CoolDown_pace_process(for_program);
		break;

	case EXCEPTION_COOLDOWN:
		if(rt->summary_state.did_warmup == 1)
		{
			if(rt->summary_state.has_warmup == 0)
			{
				CollectWarmUpSummaryData();
			}
			else
			{
				CollectWorkOutSummaryData();
			}
		}
		else
		{
			CollectWorkOutSummaryData();
		}		
		Default_CoolDown_pace_process(for_program);
		break;

	default:
		rt->tick_1sec_per100ms = 10 * rt->tick_1sec_per100ms_reload;
		while(rt->exception_result != EXCEPTION_COOLDOWN)
		{
			rt->exception_result = ExceptionHandler(for_program);
			if(rt->tick_1sec_per100ms >0)
				rt->tick_1sec_per100ms--;	
			if(rt->tick_1sec_per100ms == 0)
			{
				break;
			}
			WaitForProgramTick();
		}
		if(rt->exception_result == EXCEPTION_COOLDOWN)
		{	
			CollectWorkOutSummaryData();
			Default_CoolDown_pace_process(for_program);
		}
		else
		{
			CollectWorkOutSummaryData();
			SetTimer(SD55_CMD_DEFAULT_STRIDE_ID, 1, 0);
		}
		break;
	}

	exception->cmd.auto_populate_pace = 0;
	exception->cmd.auto_populate_load = 0;
	exception->cmd.distance_started = 0;
	exception->cmd.start_distance_set = 0;
	exception->cmd.start = 0;
	exception->cmd.stop = 0;
	exception->cmd.resume = 0;
	exception->cmd.pause = 0;
	key_scan[24] = FPC_FIRE_SUMMARY_READY;
	rt->workout_state = READY_TO_FINISH;
}

void CQsApp::CollectWarmUpSummaryData(void)
{
	rt->warmup.Time_elapsed	 = rt->elapsed_time;
	rt->warmup.Time_elapsed_1000 = rt->elapsed_time_1000;
	
	rt->warmup.Calories_burned  = rt->Calories;
	rt->warmup.Distance_metric  = rt->Distance_metric;
	rt->warmup.Distance_imperial  = rt->Distance_imperial;

	rt->warmup.Max_HeartRate	 = rt->Max_HeartRate;
	rt->warmup.Average_Heart_Rate = rt->Average_Heart_Rate;
	rt->warmup.Average_Heart_Rate_base
						 = rt->Average_Heart_Rate_base;
	
	rt->warmup.Max_Rpm		 = rt->Max_Rpm;
	rt->warmup.Average_Rpm	 = rt->Average_Rpm;
	rt->warmup.Average_Rpm_base = rt->Average_Rpm_base;
	
	rt->summary_state.has_warmup = 1;
}

void CQsApp::CollectWorkOutSummaryData(void)
{
	rt->workout.Time_elapsed	 = rt->elapsed_time;
	rt->workout.Time_elapsed_1000 = rt->elapsed_time_1000;
	
	rt->workout.Calories_burned  = rt->Calories;
	rt->workout.Distance_metric  = rt->Distance_metric;
	rt->workout.Distance_imperial  = rt->Distance_imperial;
	
	rt->workout.Max_HeartRate	 = rt->Max_HeartRate;
	rt->workout.Average_Heart_Rate = rt->Average_Heart_Rate;
	rt->workout.Average_Heart_Rate_base
						 = rt->Average_Heart_Rate_base;
	
	rt->workout.Max_Rpm	 = rt->Max_Rpm;
	rt->workout.Average_Rpm	 = rt->Average_Rpm;
	rt->workout.Average_Rpm_base = rt->Average_Rpm_base;
	
	rt->summary_state.has_workout = 1;//GOT_WORKOUT_SUMMARY;
}

void CQsApp::DataCollection(void)
{
 	if(exception->cmd.hr_cruise == 1){
 		update->Target_heart_rate = rt->target_cruise_heart_rate;
 	}else{
 		update->Target_heart_rate = setup->Target_heart_rate;
 	}
	//update->Heart_rate = hr->DisplayHeartRate;
	update->Stride_length = data->StrideLength * 5;

	if(setup->Pace_level == 0)
		setup->Pace_level = 1;
	update->Pace_level = setup->Pace_level;	
}

#define MAG 1000.00F
void	CQsApp::Calculate(void)
{

	unsigned char i;
	unsigned short temp_short;
	unsigned long integer,total_base;
	float delta_distance;

	rt->Watts = Watts_Calc();
	update->Watts = rt->Watts;	

	rt->Calories_HR  = (float)((9.0 * rt->Watts) + 120); //Ellipical
	rt->Calories += rt->Calories_HR/3600;	// 卡路里
	update->Calories_per_hour_1000cal = ((unsigned short)rt->Calories_HR)/1000;
	update->Calories_per_hour = ((unsigned short)rt->Calories_HR) - update->Calories_per_hour_1000cal *1000;

	if(rt->Calories > 1000)
	{
		update->Calories_burned_1000cal = rt->Calories/1000;
		update->Calories_burned = rt->Calories - update->Calories_burned_1000cal*1000;
	}
	else
	{
		update->Calories_burned_1000cal = 0;
		update->Calories_burned = rt->Calories;
	}

	rt->Mets_metric = rt->Calories_HR/(float)(setup->Weight);
	rt->Mets_imperial = rt->Mets_metric/0.45;
	
	if(1)
	{
		update->Mets = (unsigned short)rt->Mets_metric;
	}
	else
	{
		update->Mets = (unsigned short)rt->Mets_imperial;
	}

	delta_distance = rt->Calories_HR*Mile_KM_ratio/(107.0*3600);

	rt->Distance_metric 	+= delta_distance;//rt->Calories_HR*1.6093/(107.0*3600);
	rt->Distance_imperial  = rt->Distance_metric/Mile_KM_ratio;
	//KM
	integer = (unsigned short)(rt->Distance_metric*MAG);	
	update->Distance_km_i  = integer/MAG;
	update->Distance_km_f = integer-update->Distance_km_i*MAG;
	//Mile
	integer = (unsigned short)(rt->Distance_imperial*MAG);
	update->Distance_mi_i  = integer/MAG;
	update->Distance_mi_f  = integer-update->Distance_mi_i*MAG;

	//remain distance calculation
	if(exception->cmd.distance_started == 1)
	{
		if(exception->cmd.start_distance_set == 0)
		{
			exception->cmd.start_distance_set = 1;
			rt->start_Distance_metric = rt->Distance_metric;
		}
		if(exception->cmd.pause == 1)
		{
			rt->start_Distance_metric += delta_distance;
		}
		rt->RemainDistance_metric = rt->target_workout_distance - (rt->Distance_metric - rt->start_Distance_metric);
	}
	else
	{
		rt->RemainDistance_metric = rt->target_workout_distance;
	}
	
	//unit conversion
	rt->RemainDistance_imperial = rt->RemainDistance_metric/Mile_KM_ratio;
	//unit convertion
	integer = (unsigned short)(rt->RemainDistance_metric*MAG);	
	update->Distance_remaining_km_i  = integer/MAG;
	update->Distance_remaining_km_f = integer - update->Distance_remaining_km_i*MAG;
	//unit convertion
	integer = (unsigned short)(rt->RemainDistance_imperial*MAG);
	update->Distance_remaining_mi_i = integer/MAG;
	update->Distance_remaining_mi_f = integer - update->Distance_remaining_mi_i*MAG;

	temp_short = hr->DisplayHeartRate;

	if(temp_short > rt->Max_HeartRate)
		rt->Max_HeartRate = temp_short;

	if(exception->cmd.pause == 0)
	{
		if(temp_short == 0)
		{
			rt->waste_hr_elapsed_time ++;
		}
		else
		{	
			rt->Average_Heart_Rate_ACC += temp_short;
			total_base = rt->elapsed_time + rt->elapsed_time_1000*1000 - 
					rt->waste_hr_elapsed_time + 1;
			rt->Average_Heart_Rate_base = total_base;

			rt->Average_Heart_Rate = (unsigned short)(rt->Average_Heart_Rate_ACC/total_base);
		}
	}

	temp_short = Get_RPM();
	if(temp_short > rt->Max_Rpm)
		rt->Max_Rpm = temp_short;	

	if(exception->cmd.pause == 0){
		if(temp_short == 0){
			rt->waste_rpm_elapsed_time ++;
		}else{
			rt->Average_Rpm_ACC += temp_short;
			total_base = rt->elapsed_time + rt->elapsed_time_1000*1000 - 
					rt->waste_rpm_elapsed_time + 1;
			rt->Average_Rpm_base = total_base;
			rt->Average_Rpm = (unsigned short)(rt->Average_Rpm_ACC/total_base);
		}
	}

	if(exception->cmd.auto_stride == 1){
		for(i=0;i<AS_TABLE_COUNT;i++){
			if(temp_short <= tables->Get_AS_rpm(i)){//asTable[i].rpm){
				rt->Auto_Stride = tables->Get_AS_stride(i);//asTable[i].stride;
				break;
			}		
		}
		if(i==12){
			rt->Auto_Stride = tables->Get_AS_stride(11);//asTable[11].stride;
		}

// jason
		Set_Stride_Motor_Position(rt->Auto_Stride);
	}


}

unsigned char CQsApp::Get_RunRPM(void)
{
	unsigned short temp_rpm;

	temp_rpm = data->Rpm;
	if(temp_rpm == 0) return 0;
	if(temp_rpm > MAX_RPM)			// 120
		temp_rpm = MAX_RPM;
	if(temp_rpm < MIN_RPM)			// 30
		temp_rpm = MIN_RPM;
	return (unsigned char)temp_rpm;
}

unsigned short CQsApp::Get_RPM(void)
{
	return (unsigned short)data->Rpm;	//*1.7);	//(word)((float)inBios.Rpm/10.5);
}

float CQsApp::Watts_Calc(void)
{
	float watt;
	unsigned char rpm = Get_RunRPM();
	watt = 0;
	if(rpm >= 30){
		rpm = (rpm - 30)/10;
		switch(rt->watt_calc_mod){
		case CALC_BY_INDEXED_LOAD_LEVEL:
			break;
		case CALC_BY_CURRENT_LOAD_LEVEL:
			watt = tables->Get_Watt(rt->workLoad.current_load_level, rpm);
			break;
		}
	}
	return watt;
}

unsigned short CQsApp::Watts_Calc_cruise(void)
{
	float watt;
	unsigned char rpm = Get_RunRPM();
	watt = 0;
	if(rpm >= 30){
		switch(rt->watt_calc_mod){
		case CALC_BY_INDEXED_LOAD_LEVEL:
			watt = tables->Get_Watt(rt->workLoad_Table[rt->segment_index], RPM60_COL);
			break;
		
		case CALC_BY_CURRENT_LOAD_LEVEL :
			watt = tables->Get_Watt(rt->workLoad_Table[rt->workLoad.current_load_level], RPM60_COL);
			break;
		}
	}
	return watt;
}

unsigned char CQsApp::ProcessUserMessage(unsigned char *buff, int fd)
{
	unsigned char reply[1024];
	unsigned char out[1024 * 4];
	unsigned char H, F, L;
	unsigned char c;
	unsigned short s;

	H = buff[0];
	if(H & BIT7)
		return 0;
	F = buff[1];
	L = buff[2];
	H |= BIT7; buff[0] = H;


//if (F != 149)
	//printf("(%s %d) FUNCTION:%d\n", __FILE__, __LINE__, F);

	/*if(FC_WORK_MODE_CHANGE == F)
	{
		struct work_mode_state_t state;

		state.type = buff[3];
		state.target = BUILD_UINT32(buff[7], buff[6], buff[5], buff[4]);
		state.source = BUILD_UINT32(buff[11], buff[10], buff[9], buff[8]);
		state.key = buff[12];
		state.shift = buff[13];

		if (0 == buff[3])
		{
			OnWorkModeChange(&state);
		}
		else if (1 == buff[3])
		{
			OnDataScreenChange(&state);
		}

		reply[0] = H; reply[1] = F; L = 0; reply[2] = L;
		packOurProtocol(CMD_GUI | (1 << 6), 0, L + 3, (unsigned char *)reply, out);
		if (write(fd, (const void *)out, L + 3 + 8) <= 0){}
		return 0;
	}*/

	if(0X79 == F)
	{
		sync(); sync(); sync();
		sync(); sync(); sync();
		reply[0] = H; reply[1] = F; L = 0, reply[2] = L;
		packOurProtocol(33 | (1 << 6), 0, L + 3, (unsigned char *)reply, out);
		if (write(fd, (const void *)out, L + 3 + 8) <= 0){}
		reboot(RB_AUTOBOOT);
		return 0;
	}
	if(FC_HELLO == F && L == 5)
	{
		memcpy(reply, buff, L + 3);
		packOurProtocol(CMD_GUI | (1 << 6), 0, L + 3, (unsigned char *)reply, out);
		if (write(fd, (const void *)out, L + 3 + 8) <= 0){}
		return 0;
	}
	if(FC_SET_SETUP == F)
	{
		s = BUILD_UINT16(buff[4], buff[3]); setup->Age= s;
		s = BUILD_UINT16(buff[6], buff[5]); setup->Weight= s;
		s = BUILD_UINT16(buff[8], buff[7]); setup->Workout_time_1000= s / 1000; setup->Workout_time_1000= s % 1000;
		s = BUILD_UINT16(buff[10], buff[9]); setup->Workout_distance = s;
		s = BUILD_UINT16(buff[12], buff[11]); setup->Workload_level = s;
		s = BUILD_UINT16(buff[14], buff[13]); setup->Pace_level = s;
		s = BUILD_UINT16(buff[16], buff[15]); setup->Calorie_goal = s;
		s = BUILD_UINT16(buff[18], buff[17]); setup->Target_heart_rate = s;
		s = BUILD_UINT16(buff[20], buff[19]); setup->Work_heart_rate = s;
		s = BUILD_UINT16(buff[22], buff[21]); setup->Gender = s;
		s = BUILD_UINT16(buff[24], buff[23]); setup->Segments = s;
		for (c = 0; c < 10; c++)
		{
			s = BUILD_UINT16(buff[25 + 1 + c * 2], buff[25 + c * 2]); setup->Segments_time[c] = s;
		}
		for (c = 0; c < 30; c++)
		{
			s = BUILD_UINT16(buff[45 + 1 + c * 2], buff[45 + c * 2]); setup->Workload[c] = s;
		}
		for (c = 0; c < 30; c++)
		{
			s = BUILD_UINT16(buff[105 + 1 + c * 2], buff[105 + c * 2]); setup->Pace[c] = s;
		}

		reply[0] = H; reply[1] = F; L = 1, reply[2] = L; reply[3] = 0;
		packOurProtocol(CMD_GUI | (1 << 6), 0, L + 3, (unsigned char *)reply, out);
		if (write(fd, (const void *)out, L + 3 + 8) <= 0){}
		return 0;
	}

	if(FC_GET_SETUP == F)
	{
		s = setup->Age; reply[3] = HI_UINT16(s); reply[4] = LO_UINT16(s);
		s = setup->Weight; reply[5] = HI_UINT16(s); reply[6] = LO_UINT16(s);
		s = setup->Workout_time_1000 * 1000 + setup->Workout_time; reply[7] = HI_UINT16(s); reply[8] = LO_UINT16(s);
		s = setup->Workout_distance; reply[9] = HI_UINT16(s); reply[10] = LO_UINT16(s);
		s = setup->Workload_level; reply[11] = HI_UINT16(s); reply[12] = LO_UINT16(s);
		s = setup->Pace_level; reply[13] = HI_UINT16(s); reply[14] = LO_UINT16(s);
		s = setup->Calorie_goal; reply[15] = HI_UINT16(s); reply[16] = LO_UINT16(s);
		s = setup->Target_heart_rate; reply[17] = HI_UINT16(s); reply[18] = LO_UINT16(s);
		s = setup->Work_heart_rate; reply[19] = HI_UINT16(s); reply[20] = LO_UINT16(s);
		s = setup->Gender; reply[21] = HI_UINT16(s); reply[22] = LO_UINT16(s);
		s = setup->Segments; reply[23] = HI_UINT16(s); reply[24] = LO_UINT16(s);
		for (c = 0; c < 10; c++)
		{
			s = setup->Segments_time[c]; reply[25 + c * 2] = HI_UINT16(s); reply[25 + 1 + c * 2] = LO_UINT16(s);
		}
		for (c = 0; c < 30; c++)
		{
			s = setup->Workload[c]; reply[45 + c * 2] = HI_UINT16(s); reply[45 + 1 + c * 2] = LO_UINT16(s);
		}
		for (c = 0; c < 30; c++)
		{
			s = setup->Pace[c]; reply[105 + + c * 2] = HI_UINT16(s); reply[105 + 1 + c * 2] = LO_UINT16(s);
		}

		reply[0] = H; reply[1] = F; L = 162, reply[2] = L;;
		packOurProtocol(CMD_GUI | (1 << 6), 0, L + 3, (unsigned char *)reply, out);
		if (write(fd, (const void *)out, L + 3 + 8) <= 0){}
		return 0;
	}
	if(FC_GET_UPDATE == F)
	{
		//sched_yield();

		s = rt->audio.audio_source; reply[3] = HI_UINT16(s); reply[4] = LO_UINT16(s);
		s = update->Time_elapsed_1000 * 1000 + update->Time_elapsed; reply[5] = HI_UINT16(s); reply[6] = LO_UINT16(s);
		s = update->Time_remaining_1000 * 1000 + update->Time_remaining; reply[7] = HI_UINT16(s); reply[8] = LO_UINT16(s);
		s = update->Distance_km_i * 1000 + update->Distance_km_f; reply[9] = HI_UINT16(s); reply[10] = LO_UINT16(s);
		s = update->Distance_remaining_km_i * 1000 + update->Distance_remaining_km_f; reply[11] = HI_UINT16(s); reply[12] = LO_UINT16(s);
		s = update->Heart_rate; reply[13] = HI_UINT16(s); reply[14] = LO_UINT16(s);
		s = update->Target_heart_rate; reply[15] = HI_UINT16(s); reply[16] = LO_UINT16(s);
		s = update->Calories_burned_1000cal * 1000 + update->Calories_burned; reply[17] = HI_UINT16(s); reply[18] = LO_UINT16(s);
		s = update->Calories_per_hour_1000cal * 1000 + update->Calories_per_hour; reply[19] = HI_UINT16(s); reply[20] = LO_UINT16(s);
		s = update->Watts; reply[21] = HI_UINT16(s); reply[22] = LO_UINT16(s);
		s = update->Mets; reply[23] = HI_UINT16(s); reply[24] = LO_UINT16(s);
		s = update->Workload_level; reply[25] = HI_UINT16(s); reply[26] = LO_UINT16(s);
		s = update->Pace_RPM; reply[27] = HI_UINT16(s); reply[28] = LO_UINT16(s);
		s = update->Pace_level; reply[29] = HI_UINT16(s); reply[30] = LO_UINT16(s);

		s = update->Stride_length; reply[31] = HI_UINT16(s); reply[32] = LO_UINT16(s);
		s = update->Segment_time_1000 * 1000 + update->Segment_time; reply[33] = HI_UINT16(s); reply[34] = LO_UINT16(s);
		//s = rt->segment_index; reply[35] = HI_UINT16(s); reply[36] = LO_UINT16(s);
		s = update->workload_index; reply[35] = HI_UINT16(s); reply[36] = LO_UINT16(s);
		for (c = 0; c < GUI_window_size; c++)
		{
			s = (unsigned short)update->Workload_bar[c]; reply[37 + 2 * c] = HI_UINT16(s); reply[38 + 2 * c] = LO_UINT16(s);
		}
		for (c = 0; c < GUI_window_size; c++)
		{
			s = (unsigned short)update->Pace_bar[c]; reply[61 + 2 * c] = HI_UINT16(s); reply[62 + 2 * c] = LO_UINT16(s);
		}

		reply[0] = H; reply[1] = F; L = 82, reply[2] = L;
		packOurProtocol(CMD_GUI | (1 << 6), 0, L + 3, (unsigned char *)reply, out);
		if (write(fd, (const void *)out, L + 3 + 8) <= 0) {}
		return 0;
	}
	if(FC_GET_SUMMARY == F)
	{
		s = summary->Warmup_time_elapsed_1000 * 1000 + summary->Warmup_time_elapsed; reply[3] = HI_UINT16(s); reply[4] = LO_UINT16(s);
		s = summary->Warmup_avg_heart_rate; reply[5] = HI_UINT16(s); reply[6] = LO_UINT16(s);
		s = summary->Warmup_max_heart_rate; reply[7] = HI_UINT16(s); reply[8] = LO_UINT16(s);
		s = summary->Warmup_average_pace; reply[9] = HI_UINT16(s); reply[10] = LO_UINT16(s);
		s = summary->Warmup_max_pace; reply[11] = HI_UINT16(s); reply[12] = LO_UINT16(s);
		s = summary->Warmup_distance_km_i * 1000 + summary->Warmup_distance_km_f; reply[13] = HI_UINT16(s); reply[14] = LO_UINT16(s);
		s = summary->Warmup_calories_burned_1000 * 1000 + summary->Warmup_calories_burned; reply[15] = HI_UINT16(s); reply[16] = LO_UINT16(s);
		s = summary->Workout_time_elapsed_1000 * 1000 + summary->Workout_time_elapsed; reply[17] = HI_UINT16(s); reply[18] = LO_UINT16(s);
		s = summary->Workout_avg_heart_rate; reply[19] = HI_UINT16(s); reply[20] = LO_UINT16(s);
		s = summary->Workout_max_heart_rate; reply[21] = HI_UINT16(s); reply[22] = LO_UINT16(s);
		s = summary->Workout_average_pace; reply[23] = HI_UINT16(s); reply[24] = LO_UINT16(s);
		s = summary->Workout_max_pace; reply[25] = HI_UINT16(s); reply[26] = LO_UINT16(s);
		s = summary->Workout_distance_km_i * 1000 + summary->Workout_distance_km_f; reply[27] = HI_UINT16(s); reply[28] = LO_UINT16(s);
		s = summary->Workout_calories_burned_1000 * 1000 + summary->Workout_calories_burned; reply[29] = HI_UINT16(s); reply[30] = LO_UINT16(s);
		s = summary->Cooldown_time_elapsed_1000 * 1000 + summary->Cooldown_time_elapsed; reply[31] = HI_UINT16(s); reply[32] = LO_UINT16(s);
		s = summary->Cooldown_avg_heart_rate; reply[33] = HI_UINT16(s); reply[34] = LO_UINT16(s);
		s = summary->Cooldown_max_heart_rate; reply[35] = HI_UINT16(s); reply[36] = LO_UINT16(s);
		s = summary->Cooldown_average_pace; reply[37] = HI_UINT16(s); reply[38] = LO_UINT16(s);
		s = summary->Cooldown_max_pace; reply[39] = HI_UINT16(s); reply[40] = LO_UINT16(s);
		s = summary->Cooldown_distance_km_i * 1000 + summary->Cooldown_distance_km_f; reply[41] = HI_UINT16(s); reply[42] = LO_UINT16(s);
		s = summary->Cooldown_calories_burned_1000 * 1000 + summary->Cooldown_calories_burned; reply[43] = HI_UINT16(s); reply[44] = LO_UINT16(s);
		s = summary->Total_time_elapsed_1000 * 1000 + summary->Total_time_elapsed; reply[45] = HI_UINT16(s); reply[46] = LO_UINT16(s);
		s = summary->Total_avg_heart_rate; reply[47] = HI_UINT16(s); reply[48] = LO_UINT16(s);
		s = summary->Total_max_heart_rate; reply[49] = HI_UINT16(s); reply[50] = LO_UINT16(s);
		s = summary->Total_average_pace; reply[51] = HI_UINT16(s); reply[52] = LO_UINT16(s);
		s = summary->Total_max_pace; reply[53] = HI_UINT16(s); reply[54] = LO_UINT16(s);
		s = summary->Total_distance_km_i * 1000 + summary->Total_distance_km_f; reply[55] = HI_UINT16(s); reply[56] = LO_UINT16(s);
		s = summary->Total_calories_burned_1000 * 1000 + summary->Total_calories_burned; reply[57] = HI_UINT16(s); reply[58] = LO_UINT16(s);
		s = summary->Vo2; reply[59] = HI_UINT16(s); reply[60] = LO_UINT16(s);
		reply[0] = H; reply[1] = F; L = 58, reply[2] = L;
		packOurProtocol(CMD_GUI | (1 << 6), 0, L + 3, (unsigned char *)reply, out);
		if (write(fd, (const void *)out, L + 3 + 8) <= 0){}
		return 0;
	}
	/*if(FC_SET_WORKLOAD == F)
	{
		rt->Target_Workload_level = buff[3];
		update->Workload_level = rt->Target_Workload_level;
		exception->cmd.work_load_up = 1;
		reply[0] = H; reply[1] = F; L = 1, reply[2] = L; reply[3] = 0;
		packOurProtocol(CMD_GUI | (1 << 6), 0, L + 3, (unsigned char *)reply, out);
		if (write(fd, (const void *)out, L + 3 + 8) <= 0){}
		return 0;
	}*/
	if(FC_SET_PACE== F)
	{
		rt->Target_Pace_Level = buff[3];
		exception->cmd.pace_up = 1;
		reply[0] = H; reply[1] = F; L = 1, reply[2] = L; reply[3] = 0;
		packOurProtocol(CMD_GUI | (1 << 6), 0, L + 3, (unsigned char *)reply, out);
		if (write(fd, (const void *)out, L + 3 + 8) <= 0){}
		return 0;
	}
	if(FC_SET_STRIDE == F)
	{
		update->Stride_length = rt->Target_Stride = buff[3];
		data->StrideLength =  rt->Target_Stride / 5;
		exception->cmd.stride_up = 1;
		reply[0] = H; reply[1] = F; L = 1, reply[2] = L; reply[3] = 0;
		packOurProtocol(CMD_GUI | (1 << 6), 0, L + 3, (unsigned char *)reply, out);
		if (write(fd, (const void *)out, L + 3 + 8) <= 0){}
		return 0;
	}
	if(FC_BUZZER == F)
	{
		c = 0;
		i2c_read_data(i2c0, PCA_9555_SLAVE_1, PCA_9555_INPUT_0, &c, 1);
		c |= BIT6;
		if (0 == product_type || 1 == product_type)
			c |= BIT5;
		i2c_write_data(i2c0, PCA_9555_SLAVE_1, PCA_9555_OUTPUT_0, &c, 1);
		SetTimer(PWM1_TIMER_ID, 100, 0);
		reply[0] = H; reply[1] = F; L = 0, reply[2] = L;
		packOurProtocol(CMD_GUI | (1 << 6), 0, L + 3, (unsigned char *)reply, out);
		if (write(fd, (const void *)out, L + 3 + 8) <= 0){}
		return 0;
	}
	if(FC_BUTTON == F)
	{
		//sched_yield();

		reply[0] = H; reply[1] = F; L = 24, reply[2] = L;
		memcpy(&reply[3 + 0], &key_scan[1], 6);		// S1~6
		reply[3 + 6] = key_scan[9];				// S9
		reply[3 + 7] = key_scan[21];
		reply[3 + 8] = key_scan[7];
		reply[3 + 9] = key_scan[8];
		reply[3 + 10] = key_scan[22];
		reply[3 + 11] = key_scan[23];
		memcpy(&reply[3 + 12], &key_scan[10], 11);
		reply[3 + 23] = key_scan[24];
		if (0 != key_scan[24])
			key_scan[24] = 0;
		packOurProtocol(CMD_GUI | (1 << 6), 0, L + 3, (unsigned char *)reply, out);
		if (write(fd, (const void *)out, L + 3 + 8) <= 0) {}
		return 0;
	}
	if (FC_CHECK_RUNTIME_WORKOUT_SECONDS == F)
	{
		s = update->Time_elapsed_1000 * 1000 + update->Time_elapsed;
		reply[3] = HI_UINT16(s); reply[4] = LO_UINT16(s);
		s = update->Time_remaining_1000 * 1000 + update->Time_remaining;
		reply[5] = HI_UINT16(s); reply[6] = LO_UINT16(s);
		s = update->Segment_time_1000 * 1000 + update->Segment_time;
		reply[7] = HI_UINT16(s); reply[8] = LO_UINT16(s);
		if (rt->workout_state == READY_TO_COOL_DOWN)		s = 1;
		if (rt->workout_state == READY_TO_FINISH)			s = 2;
		if (rt->workout_state == READY_TO_SUMMARY)		s = 3;
		reply[9] = HI_UINT16(s); reply[10] = LO_UINT16(s);
		reply[0] = H; reply[1] = F; L = 8, reply[2] = L;
		packOurProtocol(CMD_GUI | (1 << 6), 0, L + 3, (unsigned char *)reply, out);
		if (write(fd, (const void *)out, L + 3 + 8) <= 0){}
		return 0;
	}

	return 0;
}

void CQsApp::WorkoutData_initialize(void)
{
	memset((void *)rt, 0, sizeof(struct RunTimeVar));

	rt->workLoad_Table = _workLoad_Table;
	rt->workWatt_Table = _workWatt_Table;
	rt->workPace_Table = _workPace_Table;
	rt->workLoad_Table_cruise = _workLoad_Table_cruise;
	rt->segmentTime_Table = _segmentTime_Table;
	memset((void *)exception, 0, sizeof(struct Exception));
	memset((void *)summary, 0, sizeof(struct WorkoutSummary));
	memset((void *)update, 0, sizeof(UPDTATE));
}


