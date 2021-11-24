// FpcApp.cpp: implementation of the CFpcApp class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

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
//#include <sys/inotify.h>
#include <termios.h>
#include <netinet/in.h>
#include <inttypes.h>
#include <stddef.h>
#include <assert.h>
#include <sys/socket.h>
#include <sys/reboot.h>
#include <linux/rtc.h>

#include <asm/ioctl.h>
#include <asm/errno.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
//#include <linux/miscdevice.h>
#include <tinyalsa/asoundlib.h>
//#include <sys/soundcard.h>


#include "list.h"
#include "Event.h"
#include "Misc.h"
#include "BaseApp.h"
#include "FpcTb.h"
#include "Sd55Data.h"
#include "FpcData.h"
#include "Sd55App.h"
#include "FpcApp.h"


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

#define RK29_IOCTL_VOL_SRC_GET		0
#define RK29_IOCTL_VOL_SRC_SET		1
#define RK29_IOCTL_VOL_MUTE		2
#define RK29_IOCTL_VOL_SET			3

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



// 0: sd55
// 1: bike
// 2: 發電機

int product_type = 0;
char McuVersion[] ={48,48,48,48};
int buzz_type = 0;
int bl_sleep_type = 0;
int hr_pirority_type = 0;

float mph = 0.00F;


int bl_on = 1;


// /dev/block/vold/8:1
// /proc/tty/driver/usbserial
/*
0: module:pl2303 name:"pl2303" vendor:067b product:2303 num_ports:1 port:1 path:usb-spmp-1.2
1: module:pl2303 name:"pl2303" vendor:067b product:2303 num_ports:1 port:1 path:usb-spmp-1.3
2: module:pl2303 name:"pl2303" vendor:067b product:2303 num_ports:1 port:1 path:usb-spmp-1.4
*/


#define CONFIG_READ_SAVES										1


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


#define CMD_ACK_BIT													14
#define CMD_UART													1
#define CMD_I2C														2
#define CMD_SPI														3
#define CMD_USB_UART												4
#define CMD_GPIO													5
#define CMD_FPC														32
#define CMD_GUI														33
#define CMD_IP														63

#define SOCKET_BUFFER_SIZE											(512 * 1024)
//#define SOCKET_BUFFER_SIZE											(8 * 1024)

#define FPC_SERVICE_LISTEN_PORT									8366




//////////////////////////////////////////////////////////////////////
// define
//////////////////////////////////////////////////////////////////////

// ADC1 / GPIO0[20] / PWM1

// GPIO5[13]
#define GPIO5_13_CHN	5
#define GPIO5_13_PIN	13
#define GPIO5_13_GID	54
#define GPIO5_13_FUN	7
#define GPIO5_13_IDX	((GPIO5_13_CHN<<24)|(GPIO5_13_FUN<<16)|(GPIO5_13_GID<<8)|GPIO5_13_PIN)

#define GP_ADC_MAGIC	'G'
#define IOCTL_GP_ADC_START		_IOW(GP_ADC_MAGIC,0x01,unsigned long)	/*!< start ad conversion*/
#define IOCTL_GP_ADC_STOP		_IO(GP_ADC_MAGIC,0x02)	/*!<stop ad conversion*/

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


#define GP_PWM_MAGIC		'P'
#define PWM_IOCTL_SET_ENABLE		_IOW(GP_PWM_MAGIC, 1, unsigned int)
#define PWM_IOCTL_SET_ATTRIBUTE		_IOW(GP_PWM_MAGIC, 2, gp_pwm_config_t)
#define PWM_IOCTL_GET_ATTRIBUTE		_IOR(GP_PWM_MAGIC, 2, gp_pwm_config_t*)

typedef struct gp_pwm_config_s {
	unsigned int freq;
	int duty;
	int pin_index;
} gp_pwm_config_t;


#define default_cool_down_time	2
#define default_work_load_level	1



//////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////

struct WLClass20_Load randomWattTable[] =
{
	{{ 2,  	2,	10,	 6,	 6,	 10,	 2,	 2,	4,	10,	10,	4,	 2,	 2,	 10,	 6,	 6,	 10,	 2,	 2 }},
	{{ 3,  	3,	10,	 6,	 6,	 10,	3,	3,	5,	10,	10,	5,	3,	3,	 10,	 6,	 6,	 10,	 3,	 3 }},
	{{ 3, 	3,	10,	 7,	7,	10,	3,	3,	6,	10,	10,	6,	3,	3,	10,	7,	 7,	 10,	 3,	 3 }},
	{{ 4, 	4,	10,	 7,	7,	10,	4,	4,	6,	10,	10,	6,	4,	4,	10,	7,	 7,	 10,	 4,	 4 }},
	{{ 4, 	4,	11,	 8,	8,	11,	4,	4,	7,	11,	11,	7,	4,	4,	11,	8,	 8,	 11,	 4,	 4 }},
	{{ 5, 	5,	11,	 8,	8,	11,	5,	5,	7,	11,	11,	7,	5,	5,	11,	8,	 8,	 11,	 5,	 5 }},
	{{ 6, 	6,	11,	8,	8,	11,	6,	6,	8,	11,	11,	8,	6,	6,	11,	8,	8,	11,	 6,	 6 }},
	{{ 6, 	6,	11,	9,	9,	11,	6,	6,	8,	11,	11,	8,	6,	6,	11,	9,	9,	11,	 6,	 6 }},
	{{ 7, 	7,	11,	9,	9,	11,	7,	7,	8,	11,	11,	8,	7,	7,	11,	9,	9,	11,	 7,	 7 }},
	{{ 7, 	7,	12,	9,	9,	12,	7,	7,	9,	12,	12,	9,	7,	7,	12,	9,	9,	12,	 7,	 7 }},
	{{ 8,	8,	12,	10,	10,	12,	8,	8,	9,	12,	12,	9,	8,	8,	12,	10,	10,	12,	 8,	 8 }},
	{{ 8, 	8,	12,	10,	10,	12,	8,	8,	9,	12,	12,	9,	8,	8,	12,	10,	10,	12,	 8,	 8 }},
	{{ 8, 	8,	12,	10,	10,	12,	8,	8,	10,	12,	12,	10,	8,	8,	12,	10,	10,	12,	 8,	 8 }},
	{{ 9, 	9,	13,	10,	10,	13,	9,	9,	10,	13,	13,	10,	9,	9,	13,	10,	10,	13,	 9,	 9 }},
	{{ 9, 	9,	13,	11,	11,	13,	9,	9,	10,	13,	13,	10,	9,	9,	13,	11,	11,	13,	9,	9 }},
	{{ 9, 	9,	13,	11,	11,	13,	9,	9,	10,	13,	13,	10,	9,	9,	13,	11,	11,	13,	9,	9 }},
	{{ 10, 	10,	13,	11,	11,	13,	10,	10,	11,	13,	13,	11,	10,	10,	13,	11,	11,	13,	10,	10 }},
	{{ 10, 	10,	13,	11,	11,	13,	10,	10,	11,	13,	13,	11,	10,	10,	13,	11,	11,	13,	10,	10 }},
	{{ 10,	10,	14,	11,	11,	14,	10,	10,	11,	14,	14,	11,	10,	10,	14,	11,	11,	14,	10,	10 }},
	{{ 10,	10,	14,	12,	12,	14,	10,	10,	11,	14,	14,	11,	10,	10,	14,	12,	12,	14,	10,	10 }}
};


unsigned short saved_target_hr = 180;
int manual_distance_flag = 0;
int update_cooldown_pace_flag = 0;
int resume_mod_flag = 0;
int udisk_detect = 0;


int IPOD_PREV_FLAG = 0;
int IPOD_MUTE_VAR = 0;
int IPOD_VOL_VAR = 5;
//int IPOD_VOL_VAR = 8;

int IPOD_Detect = 0;
extern int sd55_ok;

int prob_ms6257 = 0;
unsigned char T_AD = (unsigned char)round( MIN_AS + (0.00F * DELTA_AS) );
unsigned char C_AD = 0;
//unsigned char L_AD = 0;

int prob_pca955_0 = 0;
int prob_pca955_1 = 0;
int prob_pca955_2 = 0;
extern int SPU_PIN;


unsigned char _workLoad_Table_cruise[MAX_SEGMENTS];	//x
unsigned char _workLoad_Table[MAX_SEGMENTS];		//x
unsigned char _workPace_Table[MAX_SEGMENTS];		//x	
unsigned char _workWatt_Table[MAX_SEGMENTS];		//x

unsigned char _segmentTime_Table[MAX_SEGMENTS];		//x


//int source_ipod_focused = 0;

int focus_ok = 1;;
int focus_key[9];
int age_focused = 0;
int weight_focused = 0;
int workload_focused = 0;
int distance_focused = 0;
int time_focused = 0;
int calorie_focused = 0;
int gender_focused = 0;
int pace_focused = 0;
int segments_focused = 0;
int hr_focused = 0;
int scroll_idx = 0;
struct SetupData saved_setup[4];

int read_setup_ok = 0;


unsigned char thumb_cruise = 0;
unsigned char last_thumb_cruise = 0;
unsigned char thumb_start = 0;
unsigned char last_thumb_start = 0;
unsigned char thumb_disp = 0;
unsigned char last_thumb_disp = 0;
unsigned char thumb_minus = 0;
unsigned char last_thumb_minus = 0;
unsigned char thumb_plus = 0;
unsigned char last_thumb_plus = 0;
unsigned char thumb_enter = 0;
unsigned char last_thumb_enter = 0;

unsigned char key_scan[64];
unsigned char MCU_Key_Scan[64];
unsigned char Update_Key_Scan[64];

unsigned int old_thumb_ts = 0;




//////////////////////////////////////////////////////////////////////
// routines
//////////////////////////////////////////////////////////////////////

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


#define weight_max 350


#define Workload_level_max 30 //freeman
#define segment_index_max 11 //freeman


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
	int vol_fd = -1;
	char str[32];

	vol_fd = open("/dev/vol", O_RDWR);
	if (-1 == vol_fd)
	{
		printf("(%s %d) OPEN FAIL\n", __FILE__, __LINE__);
		return;
	}

	if (index > 15)		index = 15;
	memset(str, 0, 32);
	sprintf(str, "%d", index);
	write(vol_fd, str, 1 + strlen(str));
	close(vol_fd);

	printf("(%s %d) ipod_vol_tab(%d)\n", __FILE__, __LINE__, index);
}

void Bv_SendCmd(int fd, int cmd)
{
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
		if (IPOD_VOL_VAR > 0)
		{
			IPOD_VOL_VAR--;
		}
		IPOD_MUTE_VAR = 0;
		break;
	case BV_CMD_VOL_UP:
		if (IPOD_VOL_VAR < 15)
		{
			IPOD_VOL_VAR ++;
		}
		IPOD_MUTE_VAR = 0;
		break;
	case BV_CMD_MUTE:
		if (IPOD_MUTE_VAR == 0)
		{
			IPOD_MUTE_VAR = 1;
		}
		else
		{
			IPOD_MUTE_VAR = 0;
		}
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


#define iPod_CMD_BUTTONRELEASE			0
#define iPod_CMD_PLAY_PAUSE				1
#define iPod_CMD_SKIP_INC					2
#define iPod_CMD_SKIP_DEC					3
#define iPod_CMD_VOL_DOWN					4
#define iPod_CMD_VOL_UP					5
#define iPod_CMD_MUTE						6
#define iPod_CMD_SOURCE					7

#define iPod_CMD_NEXT						8
#define iPod_CMD_PREV						9
#define iPod_CMD_MODE_2					10
#define iPod_CMD_PLAY						11
#define iPod_CMD_PAUSE						12
#define iPod_CMD_STOP						13
#define iPod_CMD_SELECT						14
#define iPod_CMD_ON							15
#define iPod_CMD_MENU						16



static unsigned char Tx2_Buffer[1024];

void iPod_SendCmd(int fd, int cmd)
{
	int Send2_Len = 1;

	Tx2_Buffer[0]=0xFF;
	Tx2_Buffer[1]=0x55;

//          L  M   C
// FF-55-03-00-01-02-FA // SW to M2
// (0x100 - [actual values of length + mode + command + parameter]) & 0xff
// 0x100 - 3 -2 - 0 - 0 = FB
// 100 - 3 - 0 -1 - 2

// to switch to AiR mode the following bytes are sent:
//    FF-55-03-00-01-04-F8 
// 0xFF 0x55 
// 0x03 			// LEN
// 0x00 
// 0x01 0x02
// 0xFA


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


0x00 0x00 0x20
	Next Playlist
0x00 0x00 0x40
	Previous Playlist 


*/
	switch(cmd)
	{
// FF-55-03-00-01-02-FA
		case iPod_CMD_MODE_2:
			Tx2_Buffer[2]=0x03;		// LEN
			Tx2_Buffer[3]=0x00;		// mode
			Tx2_Buffer[4]=0x01;		// cmd
			Tx2_Buffer[5]=0x02;		// cmd
			Tx2_Buffer[6]=0xFA;
			Send2_Len = 7;
			break;

		case iPod_CMD_PREV:
// FF-55-04-02-00-00-40-BA
			Tx2_Buffer[2]=0x04;		// LEN
			Tx2_Buffer[3]=0x02;		// mode
			Tx2_Buffer[4]=0x00;		// cmd
			Tx2_Buffer[5]=0x00;		// cmd
			Tx2_Buffer[6]=0x40;		// cmd
// 0x100 - ((Sum of all data in A, B, C & D) & 0xFF) 
			Tx2_Buffer[7]=0xBA;
			Send2_Len = 8;
			break;

		case iPod_CMD_NEXT:
// FF-55-04-02-00-00-20-DA
// FF-55-03-02-00-20-DB
			Tx2_Buffer[2] = 0x04;		// len
			Tx2_Buffer[3] = 0x02;		// mode
			Tx2_Buffer[4] = 0x00;		// cmd
			Tx2_Buffer[5] = 0x00;		// cmd
			Tx2_Buffer[6] = 0x20;		// cmd
			Tx2_Buffer[7] = 0xDA;
			Send2_Len = 8;
			break;

/////////////////
// FF-55-03-02-00-00-FB
		case iPod_CMD_BUTTONRELEASE:
			Tx2_Buffer[2]=0x03;	// Length
			Tx2_Buffer[3]=0x02; // Mode
			Tx2_Buffer[4]=0x00; // Command
			Tx2_Buffer[5]=0x00; // Command
			Tx2_Buffer[6]=0xFB; // CheckSum
			Send2_Len = 7;
			break;

		case iPod_CMD_PLAY:
// FF-55-04-02-00-00-01-F9
			Tx2_Buffer[2]=0x04;
			Tx2_Buffer[3]=0x02;
			Tx2_Buffer[4]=0x00;
			Tx2_Buffer[5]=0x00;
			Tx2_Buffer[6]=0x01;
			Tx2_Buffer[7]=0xF9;
			Send2_Len = 8;
			break;
		case iPod_CMD_PAUSE:	// not toggle
// FF-55-04-02-00-00-02-F8
			Tx2_Buffer[2]=0x04;
			Tx2_Buffer[3]=0x02;
			Tx2_Buffer[4]=0x00;
			Tx2_Buffer[5]=0x00;
			Tx2_Buffer[6]=0x02;
			Tx2_Buffer[7]=0xF8;
			Send2_Len = 8;
			break;

		case iPod_CMD_MENU:
// FF-55-05-02-00-00-00-40-B9
			Tx2_Buffer[2]=0x05;
			Tx2_Buffer[3]=0x02;
			Tx2_Buffer[4]=0x00;
			Tx2_Buffer[5]=0x00;
			Tx2_Buffer[6]=0x00;
			Tx2_Buffer[7]=0x80;
			Tx2_Buffer[8]=0x79;
			Send2_Len = 9;
			break;

		case iPod_CMD_ON:
// FF-55-05-02-00-00-00-08-F1
			Tx2_Buffer[2]=0x05;
			Tx2_Buffer[3]=0x02;
			Tx2_Buffer[4]=0x00;
			Tx2_Buffer[5]=0x00;
			Tx2_Buffer[6]=0x00;
			Tx2_Buffer[7]=0x80;
			Tx2_Buffer[8]=0x79;
			Send2_Len = 9;
			break;

		case iPod_CMD_SELECT:
// FF-55-05-02-00-00-00-80-79
			Tx2_Buffer[2]=0x05;
			Tx2_Buffer[3]=0x02;
			Tx2_Buffer[4]=0x00;
			Tx2_Buffer[5]=0x00;
			Tx2_Buffer[6]=0x00;
			Tx2_Buffer[7]=0x80;
			Tx2_Buffer[8]=0x79;
			Send2_Len = 9;
			break;

		case iPod_CMD_STOP:
// FF-55-03-02-00-80-7B
			Tx2_Buffer[2]=0x04;
			Tx2_Buffer[3]=0x02;
			Tx2_Buffer[4]=0x00;
			Tx2_Buffer[5]=0x80;
			Tx2_Buffer[6]=0x80;
			Send2_Len = 7;
			break;

		case iPod_CMD_PLAY_PAUSE:
// FF-55-03-02-00-01-FA
			Tx2_Buffer[2]=0x03;
			Tx2_Buffer[3]=0x02;
			Tx2_Buffer[4]=0x00;
			Tx2_Buffer[5]=0x01;
			Tx2_Buffer[6]=0xFA;
			Send2_Len = 7;
			break;
		case iPod_CMD_SKIP_INC:
// FF-55-03-02-00-08-F3
			Tx2_Buffer[2]=0x03;
			Tx2_Buffer[3]=0x02;
			Tx2_Buffer[4]=0x00;
			Tx2_Buffer[5]=0x08;
			Tx2_Buffer[6]=0xF3;
			Send2_Len = 7;
			break;
		case iPod_CMD_SKIP_DEC:
// FF-55-03-02-00-10-EB
			Tx2_Buffer[2]=0x03;
			Tx2_Buffer[3]=0x02;
			Tx2_Buffer[4]=0x00;
			Tx2_Buffer[5]=0x10;
			Tx2_Buffer[6]=0xEB;
			Send2_Len = 7;
			break;
		case iPod_CMD_VOL_DOWN:
// FF-55-03-02-00-04-F7
			Tx2_Buffer[2]=0x03;
			Tx2_Buffer[3]=0x02;
			Tx2_Buffer[4]=0x00;
			Tx2_Buffer[5]=0x04;
			Tx2_Buffer[6]=0xF7;
			Send2_Len = 7;
			break;
		case iPod_CMD_VOL_UP:
// FF-55-03-02-00-02-F9
			Tx2_Buffer[2]=0x03;
			Tx2_Buffer[3]=0x02;
			Tx2_Buffer[4]=0x00;
			Tx2_Buffer[5]=0x02;
			Tx2_Buffer[6]=0xF9;
			Send2_Len = 7;
			break;
		case iPod_CMD_MUTE:
// FF-55-04-02-00-00-04-F6
			Tx2_Buffer[2]=0x04;
			Tx2_Buffer[3]=0x02;
			Tx2_Buffer[4]=0x00;
			Tx2_Buffer[5]=0x00;
			Tx2_Buffer[6]=0x04;
			Tx2_Buffer[7]=0xF6;
			Send2_Len = 8;
			break;
		default:
			break;
	}

	if (-1 == write(fd, Tx2_Buffer, Send2_Len))
		printf("(%s %d) WRITE() FAIL\n", __FILE__, __LINE__);
}

extern int timer_inprocessing;


extern int sd55_err_state_ack;


void IpodDetectTimer(UINT nId)
{
	CFpcApp *app = (CFpcApp *)AfxGetApp();
	int fd;
	char value = 'Z';

	//rt->audio.audio_source = 1;
	fd = open("/sys/class/gpio/gpio244/value", O_RDONLY);
	if (-1 != fd)
	{
		static int OLD_IPOD_Detect = 0;

		if (read(fd, &value, 1) > 0)
		{
			if(value == '0')
			{
				IPOD_Detect = app->rt->ipod_in_duck = 1;
			}
			else	
			{
				IPOD_Detect = app->rt->ipod_in_duck = 0;
			}
		}
		close(fd);


		if (OLD_IPOD_Detect != IPOD_Detect)
		{
			if (1 == IPOD_Detect)
			{
				printf("(%s %d) IPOD ATTACHED\n", __FILE__, __LINE__);

				if (0 == bl_on)
				{
					ipod_vol_tab(-1, 0);
				}
				else
				{
					if (!IPOD_MUTE_VAR)			ipod_vol_tab(-1, IPOD_VOL_VAR);
					else							ipod_vol_tab(-1, 0);
				}
			}
			else
			{
				printf("(%s %d) IPOD DETACHED\n", __FILE__, __LINE__);
				ipod_vol_tab(-1, 0);
			}
		}

		OLD_IPOD_Detect = IPOD_Detect;
	}

	app->SetTimer(nId, 201, IpodDetectTimer);
}

void BlOffTimer(UINT nId) //back light off
{
	// GPIO3_C5
	int fd;
	char off = '0';
	//CFpcApp *app = (CFpcApp *)AfxGetApp();

	fd = open("/sys/class/gpio/gpio245/value", O_WRONLY);
	if (-1 != fd)
	{
		bl_on = 0;

		write(fd, (void *)&off, 1);
		close(fd);
	}

	ipod_vol_tab(-1, 0);
}

void MoveMCU_ErrorCode_To_Fpc(unsigned char state)
{
  CFpcApp *app = (CFpcApp *)AfxGetApp();
  app->data->State = state;
}

void MoveMCU_Servo_Motor_Bike_Status_To_FPC(unsigned char state)
{
	key_scan[24] =  state ;
}

void MoveMCU_Get_Mcu_Version_To_FPC(char Version[])
{
	McuVersion[0]= Version[0];
	McuVersion[1]= Version[1];	
	McuVersion[2]= Version[2];
	McuVersion[3]= Version[3];
}




void Sd55ChkRevTimer(UINT nId)
{
	unsigned char state = 0;
	CFpcApp *app = (CFpcApp *)AfxGetApp();

	//printf(" app->data->State   =0X%02X \n", app->data->State);

	if (1 == sd55_err_state_ack)
	{
		//Bit 0: 阻力故障
		//Bit 1: 跨步1 故障
		//Bit 2: 跨步2 故障
		//Bit 3: 高度 故障
		if (app->data->State & BIT0)
		{
			state = 1;
			goto BAITOUT;
		}
		if (app->data->State & BIT1)
		{
			state = 2;
			goto BAITOUT;
		}
		if (app->data->State & BIT2)
		{
			state = 3;
			goto BAITOUT;
		}

		goto BAITOUT;
	}

	//state = 4;

BAITOUT:
	key_scan[24] = state;
	//printf("Sd55ChkRevTimer key_scan[24]  %d= \n", key_scan[24]);
}

void Sd55ChkSndTimer(UINT nId)
{
	CFpcApp *app = (CFpcApp *)AfxGetApp();

	if (-1 == app->ttyUSB0)
		return;
	sd55_err_state_ack = 0;
	SendCmdB(app->ttyUSB0,0,0x0022,0);
	app->SetTimer(SD55_RCV_CHK_TIMER_ID, 503, Sd55ChkRevTimer);
	app->SetTimer(SD55_SND_CHK_TIMER_ID, 10001, Sd55ChkSndTimer);
}

void IpodButtonRelease(UINT nId)
{
	CFpcApp *app = (CFpcApp *)AfxGetApp();

	if (-1 == app->ipod)						return;
	iPod_SendCmd(app->ipod, iPod_CMD_BUTTONRELEASE);
}

void IpodNextTrack(UINT nId)
{
	CFpcApp *app = (CFpcApp *)AfxGetApp();

	if (-1 == app->ipod)						return;
	iPod_SendCmd(app->ipod, iPod_CMD_SKIP_INC);
	app->SetTimer(IPOD_BUTTONRELEASE_TIMER1_ID, 121, IpodButtonRelease);
	app->SetTimer(NEXT_TRACK_TIMER_ID, 17, IpodNextTrack);
}

void IpodPrevTrack(UINT nId)
{
	CFpcApp *app = (CFpcApp *)AfxGetApp();

	if (-1 == app->ipod)						return;
	iPod_SendCmd(app->ipod, iPod_CMD_SKIP_DEC);
	app->SetTimer(IPOD_BUTTONRELEASE_TIMER1_ID, 121, IpodButtonRelease);
	app->SetTimer(PREV_TRACK_TIMER_ID, 17, IpodPrevTrack);
}

void UdiskAttachedTimer(UINT nId)
{
	printf("(%s %d) UDISK ATTACHED\n", __FILE__, __LINE__);
	udisk_detect = 1;
}


// /proc/mounts
// /dev/block/vold/8:17 /mnt/udisk

void UdiskDetectTimer(UINT nId)
{
	CFpcApp *app = (CFpcApp *)AfxGetApp();

	//unsigned char c;
	//struct stat lbuf;
	//int l;
	//int fd = -1;
	static int newdetect = 0;
	static int oldetect = 0;

#if 1
	regex_t preg;
	int cflags = REG_EXTENDED;
	int err;
	size_t nmatch = 12;
	regmatch_t pmatch[12];
	char pattern[256];

	int f = 0;
	char line[1024];
	FILE *fp = 0;

	fp = fopen("/proc/mounts", "rb");
	if (fp)
	{
		setbuf(fp, 0);
		memset(&preg, 0, sizeof(regex_t));
		strcpy(pattern, "^/dev/block/vold/(.+) /mnt/usb_storage vfat rw,.+$");
		regcomp(&preg, pattern, cflags);
		// err = regcomp(&preg, "^.+scsi([0-9]+)[ \\t]+.+Lun:[ \\t]+([0-9]+)$", cflags);

		while((fgets(line, sizeof(line), fp)) != 0)
		{
			StripCrLf(line);
			err = regexec(&preg, line, nmatch, pmatch, 0);
			if (0 == err)
			{
				f = 1;
				break;
			}
		}

		regfree(&preg);
		fclose(fp);
		sync();
	}

	if (0 == f)			newdetect = 0;
	else					newdetect = 1;

	if (0 == oldetect && 1 == newdetect)
	{
		app->SetTimer(UDISK_DETECT_TIMER_ID + 1, 5003, UdiskAttachedTimer);
	}
	if (1 == oldetect && 0 == newdetect)
	{
		printf("(%s %d) UDISK DETACHED\n", __FILE__, __LINE__);
		for (int i = 0; i < 100; i++)
			app->KillTimer(UDISK_DETECT_TIMER_ID + 1);
		udisk_detect = 0;
	}

	oldetect = newdetect;
	app->SetTimer(UDISK_DETECT_TIMER_ID, 2003, UdiskDetectTimer);
#else
	sprintf(tmp, "/dev/block/vold/8:1");
	if(0 == stat(tmp, &lbuf))
	{
		fd = open(tmp, O_RDWR);
		if (-1 == fd)
			newdetect = 0;
		else
		{
			l = read(fd, &c, 1);
			if (l <= 0)			newdetect = 0;
			else					newdetect = 1;
			close(fd);
		}
	}
	else
	{
		newdetect = 0;
	}
	if (0 == oldetect && 1 == newdetect)
	{
		app->SetTimer(UDISK_DETECT_TIMER_ID + 1, 5003, UdiskAttachedTimer);
	}
	if (1 == oldetect && 0 == newdetect)
	{
		printf("(%s %d) UDISK DETACHED\n", __FILE__, __LINE__);
		for (int i = 0; i < 100; i++)
			app->KillTimer(UDISK_DETECT_TIMER_ID + 1);
		udisk_detect = 0;
	}
	oldetect = newdetect;

	app->SetTimer(UDISK_DETECT_TIMER_ID, 2003, UdiskDetectTimer);
#endif // 1

}

void ClearRpmTimer(UINT nId)
{
	CFpcApp *app = (CFpcApp *)AfxGetApp();

	app->data->Clear_Rpm_Delay++;
	if (0 == product_type)
		app->SetTimer(FPC_SCHEDULER_10MS_ID, 31, ClearRpmTimer);
	else
		app->SetTimer(FPC_SCHEDULER_10MS_ID, 47, ClearRpmTimer);
}

void CalculateTimer(UINT nId)
{
	CFpcApp *app = (CFpcApp *)AfxGetApp();

	app->DataCollection();
	app->Calculate();
	app->SetTimer(FPC_SCHEDULER_1000MS_ID, 998, CalculateTimer);

	//if (-1 != app->ttyUSB0)
	//	SendCmd(app->ttyUSB0, (int)SERIAL_CMD_RD_STATE, 0, 0);
}



int BuzzStatus = 0;
int McuBuzzStatus =0;

void MoveMCU_Buzzer_Status(unsigned char Status)
{

   McuBuzzStatus = Status ;
  /* if(BuzzStatus !=McuBuzzStatus)
   {
     SendCmdB(app->ttyUSB0,1,0x0018,0);
   }
   */

}


void BuzzOffTimer(UINT nId)
{
	int fd;
	char off = '1';
	CFpcApp *app = (CFpcApp *)AfxGetApp();

	SendCmdB(app->ttyUSB0,1,0x0018,0);
	BuzzStatus = 0;

///////////////////////////////////////////////////////////
	fd = open("/sys/class/gpio/gpio245/value", O_WRONLY);
	if (-1 != fd)
	{
		write(fd, (void *)&off, 1);
		close(fd);
	}
}

void BuzzOnTimer(UINT nId)
{
	CFpcApp *app = (CFpcApp *)AfxGetApp();

	printf("BuzzOnTimer IN \n");
	if(0 == buzz_type)
	{
		if(BuzzStatus == 0)
		{
			SendCmdB(app->ttyUSB0,1,0x0018,1);
			BuzzStatus = 1;
		}
	 	else
	 	{
		SendCmdB(app->ttyUSB0,1,0x0018,0);
		SendCmdB(app->ttyUSB0,1,0x0018,1);
	 	}

		app->SetTimer(BUZZ_OFF_TIMER_ID, 31, BuzzOffTimer);
	}
}


int getProduct_type()
{
	return product_type;
}



void MoveMCU_Key_ScanToUpdate_Key_Scan(unsigned char key ,unsigned char status)
{
	int index = -1;
	int fd;
	char off = '1';
	CFpcApp *app = (CFpcApp *)AfxGetApp();


///////////////////////////////////////////////////////////
	if (0 == bl_on)
	{
		if (!IPOD_MUTE_VAR)			ipod_vol_tab(-1, IPOD_VOL_VAR);
		else							ipod_vol_tab(-1, 0);
	}
	bl_on = 1;

	fd = open("/sys/class/gpio/gpio245/value", O_WRONLY);
	if (-1 != fd)
	{
		write(fd, (void *)&off, 1);
		close(fd);
	}
	if (app->rt->workout_state == IN_RUNNING)
	{
		app->KillTimer(BL_OFF_TIMER_ID);
	}
	else
	{
		if (1 == bl_sleep_type)
			app->SetTimer(BL_OFF_TIMER_ID, 1000 * 60 * 15 + 1, BlOffTimer);
		else if (2 == bl_sleep_type)
			app->SetTimer(BL_OFF_TIMER_ID, 1000 * 60 * 30 + 1, BlOffTimer);
		else if (3 == bl_sleep_type)
			app->SetTimer(BL_OFF_TIMER_ID, 1000 * 60 * 40 + 1, BlOffTimer);
		else if (4 == bl_sleep_type)
			app->SetTimer(BL_OFF_TIMER_ID, 1000 * 60 * 60 + 1, BlOffTimer);
	}


///////////////////////////////////////////////////////////
	printf("MoveMCU_Key_ScanToUpdate_Key_Scan In \n");
	switch(key)
	{
		case 0:
		index = 4;
		break;

		case 3:
		index = 8;
		break;
		
		case 4:
		index = 7;
		break;

		case 5:
		index = 15;
		break;
		
		case 8:
		index = 3;
		break;
		
		case 12:
		index = 19;
		break;
		
		case 13:
		index = 16;
		break;
		
		case 14:
		index = 25;
		break;
		
		case 15:
		index = 24;
		break;
		
		case 16:
		index = 0;
		break;
		
		case 19:
		index = 22;
		break;	
		
		case 20:
		index =20;
		break;
		
		case 21:
		index = 17;
		break;

		case 22:
		index = 27;
		break;
		
		case 24:
		index = 1;
		break;	
		
		case 27:
		index = 12;
		break;
		
		case 28:
		index = 21;
		break;
		
		case 29:
		index = 18;
		break;

		case 30:
		index = 31;
		break;
		
		case 31:
		index = 30;
		break;
		
		case 32:
		index = 2;
		break;
		
		case 36:
		index = 6;
		break;
		
		case 37:
		index = 14;
		break;

		case 38:
		index = 29;
		break;
		
		case 40:
		index = 9;
		break;
		
		case 45:
		index = 13;
		break;
		
		case 53:
		index = 5;
		break;	
		
		
		
	}
	if(index != -1) MCU_Key_Scan[index]= status;
	/*for(int i=0;i<23;i++)
	{
		printf("MCU_Key_Scan[%d] = %d \n",i,MCU_Key_Scan[i]);
	}*/
}


void KsTimer(UINT nId)
{
	CFpcApp *app = (CFpcApp *)AfxGetApp();
	static int KsTimer_keyNum = 0;
	int KeyNumNow =0;
	int KeyCount =0;
	time_t now = 0;
	static time_t KsTimer_T0 = 0;

	for(int i=0; i<32; i++)
	{
		if(MCU_Key_Scan[i]==1)
		{
			KeyNumNow =i;
			KeyCount++;
		}
	}

	if(KeyCount ==1)
	{
		if(KeyNumNow == KsTimer_keyNum)
		{
			time(&now);
			if (now - KsTimer_T0 >= 3 && Update_Key_Scan[KsTimer_keyNum] !=2)
			{
				Update_Key_Scan[KsTimer_keyNum] = 2;
				app->SetTimer(BUZZ_ON_TIMER_ID, 59, BuzzOnTimer);
			}
		}
		else
		{	
			Update_Key_Scan[KsTimer_keyNum] = 0;
			KsTimer_keyNum = KeyNumNow ;
		    time(&KsTimer_T0);
			Update_Key_Scan[KsTimer_keyNum] = 1;
			app->SetTimer(BUZZ_ON_TIMER_ID, 59, BuzzOnTimer);
		}
	}
	else if (KsTimer_keyNum != -1)
	{
	
		KsTimer_keyNum = -1;
		for(int i =0;i<32;i++)
		{
			Update_Key_Scan[i]=0;
		}
	}
	for(int i=0; i<32; i++)
	{
		if(Update_Key_Scan[i]!=0)
		{
			printf("MCU_Key_Scan[%d] = %d \n",i,Update_Key_Scan[i]);
			KeyNumNow =i;
			KeyCount++;
		}
	}
//	 SendCmdB(app->ttyUSB0,0,0x10,0);
	if (0 == app->SetTimer(KEY_SCAN_TIMER_ID, 53, KsTimer)){}


}






void SpuTimer(UINT nId)
{
	CFpcApp *app = (CFpcApp *)AfxGetApp();
	//unsigned char V = 0;
	//int i2c0 = app->i2c0;
	
/*
/////////////////////////////////////////////////
	if (!prob_pca955_1)
	{
		goto again;
	}

///////////////////////////////
	if (2 == product_type)
	{
		if (-1 == i2c_read_data(i2c0, PCA_9555_SLAVE_1, PCA_9555_INPUT_1, &V, 1))
		{
			printf("(%s %d) i2c err\n", __FILE__, __LINE__);
			goto again;
		}
		if (V & BIT6)
		{
			IPOD_Detect = 0;
			app->rt->ipod_in_duck = 0;
		}
		else
		{
			IPOD_Detect = 1;
			app->rt->ipod_in_duck = 1;
		}
	}
	else if (0 == product_type || 1 == product_type)
	{
		if (-1 == i2c_read_data(i2c0, PCA_9555_SLAVE_1, PCA_9555_INPUT_0, &V, 1))
		{
			printf("(%s %d) i2c err\n", __FILE__, __LINE__);
			goto again;
		}

		if (V & BIT4)
		{
			IPOD_Detect = 0;
			//V |= BIT7;
			V |= BIT5;
			//i2c_write_data(i2c0, PCA_9555_SLAVE_1, PCA_9555_OUTPUT_0, &V, 1);
			app->rt->ipod_in_duck = 0;
		}
		else
		{
			IPOD_Detect = 1;
			//V &= ~BIT7;
			V |= BIT5;
			//i2c_write_data(i2c0, PCA_9555_SLAVE_1, PCA_9555_OUTPUT_0, &V, 1);
			app->rt->ipod_in_duck = 1;
		}
	}


/////////////////////////////////////////////////
again:
*/
	app->Process_SPU();


/////////////////////////////////////////////////
	if (0 == product_type)
		app->SetTimer(FPC_SCHEDULER_200MS_ID, 503, SpuTimer);
	else
	{
		//SendCmdB(app->ttyUSB0,0,0x0016,0);
		app->SetTimer(FPC_SCHEDULER_200MS_ID, 503, SpuTimer);
	}
}


unsigned char HR_Buff[8] ={0};

void PollHrB(void)
{
	CFpcApp *app = (CFpcApp *)AfxGetApp();
	char hr = 0;
    

	//printf("HeartRate_TEL %d \n",app->hr->HeartRate_TEL);
	//printf("HeartRate_HGP %d \n",app->hr->HeartRate_HGP);

	

	if(0==hr_pirority_type)
	{
		if(app->hr->HeartRate_TEL !=0)
		{
			hr = app->hr->HeartRate_TEL;
		}
		else
		{
			hr = app->hr->HeartRate_HGP;
		}
		
	}
	else
	{
		if(app->hr->HeartRate_HGP !=0)
		{
		   hr = app->hr->HeartRate_HGP;
		}
		else
		{
		   hr = app->hr->HeartRate_TEL;
		}
	}
    //===================
	HR_Buff[0] = hr;
	hr = app->hr->DisplayHeartRate - HR_Buff[0];
	if(hr <=3 && hr>=-3)
	{
	app->hr->DisplayHeartRate = HR_Buff[0];
	}
	else if (hr <=5 && hr>=-5)
	{
	app->hr->DisplayHeartRate = (HR_Buff[0]+app->hr->DisplayHeartRate)/2;
	}
	else if(HR_Buff[0] == HR_Buff[1] && HR_Buff[0] == HR_Buff[2])
	{
		app->hr->DisplayHeartRate = HR_Buff[0];
	}
	for(int i=0;i<7;i++)
	{
	 HR_Buff[i+1] =HR_Buff[i];
	}
	//============
	//app->hr->DisplayHeartRate = hr;
	app->update->Heart_rate = app->hr->DisplayHeartRate;
	//printf("update->Heart_rate =%d \n",app->update->Heart_rate);
	//printf("%d \n",app->update->Heart_rate);
	return;
}



void HrTimer(UINT nId)
{
	CFpcApp *app = (CFpcApp *)AfxGetApp();

	PollHrB();
	
	//if (0 == product_type)
	//if (0 == product_type || 1 == product_type)
	//{	
	//	SendCmdB(app->ttyUSB0,0,0x0011,0);
	//	SendCmdB(app->ttyUSB0,0,0x0012,0);
		app->SetTimer(FPC_SCHEDULER_1MS_ID, 997, HrTimer);
	//	return;
	//}

	//if (0 == app->SetTimer(FPC_SCHEDULER_1MS_ID, 1, HrTimer)) { }
	//return;
}



static pthread_mutex_t xF = PTHREAD_MUTEX_INITIALIZER;
static struct incoming_t fpcCliQ;


void *TickThread(void *param)
{
	sigset_t nset, oset;
	CFpcApp *app = (CFpcApp *)AfxGetApp();
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
			printf("clock_gettime() FAIL \n");
			sched_yield();
			continue;
		}
		a = (tv.tv_sec * 1000000000) + tv.tv_nsec;

#if 0
		m = (a % (100000000 / 3));
		n = (a - m) / (100000000 / 3);
#else
		m = (a % 100000000);
		n = (a - m) / 100000000;
#endif // 1

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

void *FpcThread(void *param)
{
	unsigned int res;
	CFpcApp *app = (CFpcApp *)param;
	HANDLE hFpcStart = app->hFpcStart;
	HANDLE h[] = {hFpcStart, 0, 0};

	for (;;)
	{
		res = WaitForMultipleObjects(1, h, WAIT_FOR_ANY_ONE_EVENTS, INFINITE);
		if (WAIT_FAILED == res) { dbg_printf(DBG_BASIC, "(%s %d) WaitForMultipleObjects() >  WAIT_FAILED \n", __FILE__, __LINE__); exit(1); }
		if (WAIT_OBJECT_0 + 0 == res)
		{
			app->ProgramEngineStart();
		}
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
	servaddr.sin_family = AF_INET, servaddr.sin_addr.s_addr = htonl(INADDR_ANY), servaddr.sin_port = htons(FPC_SERVICE_LISTEN_PORT);
	if (0 > bind(lsd, (struct sockaddr *)&servaddr, sizeof(struct sockaddr_in)))	{ dbg_printf(DBG_SOCKET, "(%s %d) BIND() fail\n", __FILE__, __LINE__); AfxGetApp()->PostMessage(M_EXIT_INSTANCE, 0, 0); return 0; }
	if (0 > listen(lsd, 12))												{ dbg_printf(DBG_SOCKET, "(%s %d) LISTEN() fail\n", __FILE__, __LINE__); AfxGetApp()->PostMessage(M_EXIT_INSTANCE, 0, 0); return 0; }
	v = fcntl(lsd, F_GETFL, 0); fcntl(lsd, F_SETFL, v | O_NONBLOCK);
	v = SOCKET_BUFFER_SIZE; setsockopt(lsd, SOL_SOCKET, SO_SNDBUF, (const void *)&v, sizeof(v)); setsockopt(lsd, SOL_SOCKET, SO_RCVBUF, (const void *)&v, sizeof(v));

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
				v = SOCKET_BUFFER_SIZE, setsockopt(connfd, SOL_SOCKET, SO_SNDBUF, (const void *)&v, sizeof(v)), setsockopt(connfd, SOL_SOCKET, SO_RCVBUF, (const void *)&v, sizeof(v));
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
					//left = N->left, offs = N->offs;
					//p = N->msg + offs;
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
					v = LEN + 8;
					frame = (unsigned char *)malloc(LEN + 8 + 64 + 120);
					if (frame)
					{
						CFpcApp *app = 0;

						memcpy(frame, &v, sizeof(int));
						memcpy(frame + 64, N->msg, LEN + 8);

#if 0
						AfxGetApp()->PostMessage(M_UI_INCOMING, (WPARAM)frame, (LPARAM)fd);
#else
						app = (CFpcApp *)AfxGetApp();
						app->OnUserMessage(M_UI_INCOMING, (WPARAM)frame, (LPARAM)fd, 0);
#endif


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
				//printf("(%s %d) offs=%d left=%d\n", __FILE__, __LINE__, offs, left);
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






//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFpcApp::CFpcApp() : CSd55App()
{
	INIT_LIST_HEAD(&fpcCliQ.list);

	hFpcStart = CreateEvent(0, MANUAL_RESET, 0, "FPC_START"), assert(hFpcStart != 0);
	hFpcTick = CreateEvent(0, AUTO_RESET, 0, "FPC_TICK"), assert(hFpcTick != 0);

	tHr = 0;
	tKs = 0;
	tUi = 0;
	tFpc = 0;
	tTick = 0;

	i2c0 = -1;
	pwm1 = -1;
	gpio = -1;

	ipod = -1;
	bv = -1;
	adc = -1;
}

CFpcApp::~CFpcApp()
{
	if (-1 != adc)														{ close(adc); adc = -1; }
	if (-1 != ipod)													{ close(ipod); ipod = -1; }
	if (-1 != bv)														{ close(bv); bv = -1; }

	if (-1 != i2c0)													{ close(i2c0), i2c0 = -1; }
	if (-1 != pwm1)
	{
		if (-1 == ioctl(pwm1, PWM_IOCTL_SET_ENABLE, 0))					{ printf("PWM_IOCTL_SET_ENABLE(0) FAIL\n"); }
		close(pwm1), pwm1 = -1;
	}
	if (-1 != gpio)													{ close(gpio), gpio = -1; }

	if(hFpcStart)														CloseEvent(hFpcStart);
	if(hFpcTick)														CloseEvent(hFpcTick);
}


//////////////////////////////////////////////////////////////////////
// message map & signal map
//////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CFpcApp, CSd55App)
	//{{AFX_MSG_MAP(CFpcApp)
	ON_MESSAGE(M_INIT_INSTANCE, &CFpcApp::OnInitInstance)
	ON_MESSAGE(M_EXIT_INSTANCE, &CFpcApp::OnExitInstance)
	ON_MESSAGE(M_TIMER, &CFpcApp::OnTimer)
	ON_MESSAGE(M_UI_INCOMING, &CFpcApp::OnUserMessage)
	ON_MESSAGE(M_WAIT_FPC_END_OF_PROGRAM, &CFpcApp::OnWaitFpcEndOfProgram)
	ON_MESSAGE(M_KEY_SCAN_PRESSED, &CFpcApp::OnKeyScanPressed)
	ON_MESSAGE(M_KEY_SCAN_RELEASED, &CFpcApp::OnKeyScanReleased)
	ON_MESSAGE(M_LEFT_THUMB, &CFpcApp::OnLeftThumb)
	ON_MESSAGE(M_RIGHT_THUMB, &CFpcApp::OnRightThumb)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



//////////////////////////////////////////////////////////////////////
// message handlers
//////////////////////////////////////////////////////////////////////


// 阻力馬達段位
extern unsigned char pSD55_Resist_Level_Table[];
// 跨步馬達
extern unsigned char pStride_Len_BitMap_Table[];
// 阻力馬達段位
extern unsigned char pAsResist_Level_Table[];
// 發電機段位, FREQ 20
extern unsigned char pGsResist_Level_Table[];



/*
進入Pause mode: 
1. RPM=0 持續5秒,自動進入Pause Mode 
2. 手動按STOP鍵,進入Pause Mode(不論是否有RPM,強制進入Pause)
從Pause Mode回到data screen
1.	如果是NO RPM進入Pause Mode, 則(1)有RPM訊號或(2)手動Resume鍵都可回到data screen
2.	如果是手動按STOP鍵進入Pause Mode, 則只能手動Resume鍵回到data screen
*/

unsigned char CFpcApp::OnDataScreenChange(struct work_mode_state_t *state)
{
	unsigned int target;
	unsigned int source;
	unsigned char key;
	unsigned char shift;

	char vkeyName[32];
	unsigned char karr;
	unsigned char top = 30;

	if (WP_GLUTE_BUSTER_RUN == setup->Work_mode)			top = 20;
	if (WP_ROLLING_HILLS_RUN == setup->Work_mode)			top = 20;
	if (WP_ROLLING_HILLS_RUN == setup->Work_mode)			top = 20;
	if (WP_HILL_INTVALS_RUN == setup->Work_mode)				top = 20;
	if (WP_SINGLE_HILL_RUN == setup->Work_mode)				top = 20;
	if (WP_RANDOM_HILLS_RUN == setup->Work_mode)			top = 20;
	if (WP_CARDIO_CHALLENGE_RUN == setup->Work_mode)		top = 20;

	source = state->source;
	target = state->target;
	key = state->key;
	shift = state->shift;


	//if (1 != source && 2 != source  && 3 != source && 4 != source && 5 != source && 6 != source)
	//	hrc = 0;

	if (IN_NORMAL == rt->workout_state)	
	{
		printf("(%s %d) rt->workout_state=IN_NORMAL\n", __FILE__, __LINE__);
		return 0;
	}
	if (READY_TO_FINISH == rt->workout_state)
	{
		printf("(%s %d) rt->workout_state=READY_TO_FINISH\n", __FILE__, __LINE__);
		return 0;
	}
	//if (0 != shift)												return 0;
	if (6 != source)
	{
		if (KS_NUM0 == key || KS_NUM1 == key || KS_NUM2 == key || KS_NUM3 == key || KS_NUM4 == key || KS_NUM5 == key || KS_NUM5 == key || KS_NUM6 == key || KS_NUM7 == key || KS_NUM8 == key || KS_NUM9 == key)
		{
			printf("(%s %d) NUM KEY\n", __FILE__, __LINE__);
			return 0;
		}
	}

///////////////////////////////////////////////////////////
	if (KS_STOP == key)
	{
		if (0 == exception->cmd.stop && 1 == exception->cmd.start)
		{
			if (0 == exception->cmd.pause)
			{
				exception->cmd.pause = 1;
				return 0;
			}
			exception->cmd.stop = 1;
			exception->cmd.pause = 0;
		}
		return 0;
	}

///////////////////////////////////////////////////////////
	if (source != target)
	{
		printf("(%s %d) source != target\n", __FILE__, __LINE__);
		return 0;
	}
	//if (4 != source)
	//	source_ipod_focused = 0;


///////////////////////////////////////////////////////////
	if (KS_LEFT_TOP == key)									sprintf(vkeyName, "KS_LEFT_TOP"), karr = 0;
	else if (KS_LEFT_CENTER == key)							sprintf(vkeyName, "KS_LEFT_CENTER"), karr = 1;
	else if (KS_LEFT_BOTTOM == key)							sprintf(vkeyName, "KS_LEFT_BOTTOM"), karr = 2;
	else if (KS_RIGHT_TOP == key)								sprintf(vkeyName, "KS_RIGHT_TOP"), karr = 3;
	else if (KS_RIGHT_CENTER == key)							sprintf(vkeyName, "KS_RIGHT_CENTER"), karr = 4;
	else if (KS_RIGHT_BOTTOM == key)							sprintf(vkeyName, "KS_RIGHT_BOTTOM"), karr = 5;
	else if (KS_START == key)									sprintf(vkeyName, "KS_START"), karr = 6;
	else if (KS_STOP == key)									sprintf(vkeyName, "KS_STOP"), karr = 7;
	else if (KS_WORKLOAD_UP == key)							sprintf(vkeyName, "KS_WORKLOAD_UP"), karr = 8;
	else if (KS_WORKLOAD_DOWN == key)						sprintf(vkeyName, "KS_WORKLOAD_DOWN"), karr = 9;
	else if (KS_STRIDE_UP == key)								sprintf(vkeyName, "KS_STRIDE_UP"), karr = 10;
	else if (KS_STRIDE_DOWN == key)							sprintf(vkeyName, "KS_STRIDE_DOWN"), karr = 11;
	else if (KS_NUM0 == key)									sprintf(vkeyName, "KS_NUM0"), karr = 12;
	else if (KS_NUM1 == key)									sprintf(vkeyName, "KS_NUM1"), karr = 13;
	else if (KS_NUM2 == key)									sprintf(vkeyName, "KS_NUM2"), karr = 14;
	else if (KS_NUM3 == key)									sprintf(vkeyName, "KS_NUM3"), karr = 15;
	else if (KS_NUM4 == key)									sprintf(vkeyName, "KS_NUM4"), karr = 16;
	else if (KS_NUM5 == key)									sprintf(vkeyName, "KS_NUM5"), karr = 17;
	else if (KS_NUM6 == key)									sprintf(vkeyName, "KS_NUM6"), karr = 18;
	else if (KS_NUM7 == key)									sprintf(vkeyName, "KS_NUM7"), karr = 19;
	else if (KS_NUM8 == key)									sprintf(vkeyName, "KS_NUM8"), karr = 20;
	else if (KS_NUM9 == key)									sprintf(vkeyName, "KS_NUM9"), karr = 21;
	else if (KS_DELETE == key)									sprintf(vkeyName, "KS_DELETE"), karr = 22;
	else														sprintf(vkeyName, "KS_UNDEF"), karr = 255;
	//printf("(%s %d) OnDataScreen(), %d->%d  %s(%d)\n", __FILE__, __LINE__, source, target, vkeyName, karr);
	printf("(%s %d) OnDataScreen(), %d->%d  %s\n", __FILE__, __LINE__, source, target, vkeyName);


	//if (exception->cmd.cool_down == 1)		return 0;


///////////////////////////////////////////////////////////
	if (KS_WORKLOAD_UP == key)
	{
		if (1 == exception->cmd.pause)
			return 0;
		if (1 == exception->cmd.hr_cruise)
			return 0;

		// for HR CONTROL return to manual mode
		if (
			(rt->workout_mode == PROGRESS_MODE || rt->workout_mode == RESUME_MODE)
			&& (
				setup->Work_mode == WP_INTERVAL_HRC_RUN
				|| setup->Work_mode == WP_TARGET_HRC_RUN
				|| setup->Work_mode == WP_AEROBIC_HRC_RUN
				|| setup->Work_mode == WP_WL_HRC_RUN
				|| setup->Work_mode == WP_WL_RUN
				)
		)
		{
			if(rt->segment_index == 0)						return 0;
			if (READY_TO_SUMMARY == rt->workout_state)	return 0;
			if (READY_TO_FINISH == rt->workout_state)		return 0;
			//if (READY_TO_COOL_DOWN == rt->workout_state)	return 0;

			if (0 == update->Workload_level)
				update->Workload_level = 1;
			if (update->Workload_level < 30)
				update->Workload_level = update->Workload_level++;

			for (int i = rt->segment_index; i < rt->workLoad_TableUsed; i++)
				rt->workLoad_Table[rt->segment_index] = update->Workload_level;

			rt->Target_Workload_level = update->Workload_level;
			exception->cmd.work_load_up = 1;

			resume_mod_flag = 1;
			return 0;
		}

		if (1 == product_type)
		{
			if (READY_TO_COOL_DOWN != rt->workout_state && setup->Work_mode == WP_FITNESS_RUN)
				return 0;
		}
		else
		{
			if (READY_TO_COOL_DOWN != rt->workout_state && (setup->Work_mode == WP_CALORIE_GOAL_RUN || setup->Work_mode == WP_FITNESS_RUN))
				return 0;
		}

		if (READY_TO_SUMMARY == rt->workout_state)		return 0;
		if (READY_TO_FINISH == rt->workout_state)			return 0;
		//if (READY_TO_COOL_DOWN == rt->workout_state)		return 0;

		data->ResistanceLevel = update->Workload_level;
		if (data->ResistanceLevel >= top)				data->ResistanceLevel = top -1;
		if (data->ResistanceLevel <= 1)					data->ResistanceLevel = 1;

		//rt->currnet_work_load_level = update->Workload_level = rt->Target_Workload_level = ++data->ResistanceLevel;
		//update->Workload_level = rt->Target_Workload_level = rt->Target_Pace_Level = ++data->ResistanceLevel;
		update->Workload_level = rt->Target_Workload_level = ++data->ResistanceLevel;
		exception->cmd.work_load_up = 1;
		//if (rt->Target_Pace_Level > 20)
		//	rt->Target_Pace_Level = 20;
		//exception->cmd.pace_up = 1;
		printf("(%s %d) WORKLOAD_UP=%d %d\n", __FILE__, __LINE__, data->ResistanceLevel, update->Workload_level);
		return 0;
	}
	if (KS_WORKLOAD_DOWN == key)
	{
		if (1 == exception->cmd.pause)
			return 0;
		if (1 == exception->cmd.hr_cruise)
			return 0;

		// for HR CONTROL return to manual mode
		if (
			(rt->workout_mode == PROGRESS_MODE || rt->workout_mode == RESUME_MODE)
			&& (
				setup->Work_mode == WP_INTERVAL_HRC_RUN
				|| setup->Work_mode == WP_TARGET_HRC_RUN
				|| setup->Work_mode == WP_AEROBIC_HRC_RUN
				|| setup->Work_mode == WP_WL_HRC_RUN
				|| setup->Work_mode == WP_WL_RUN
				)
		)
		{
			if(rt->segment_index == 0)						return 0;
			if (READY_TO_SUMMARY == rt->workout_state)	return 0;
			if (READY_TO_FINISH == rt->workout_state)		return 0;
			//if (READY_TO_COOL_DOWN == rt->workout_state)	return 0;

			if (0 == update->Workload_level)
				update->Workload_level = 1;
			if (update->Workload_level > 1)
				update->Workload_level = update->Workload_level--;

			for (int i = rt->segment_index; i < rt->workLoad_TableUsed; i++)
				rt->workLoad_Table[rt->segment_index] = update->Workload_level;

			rt->Target_Workload_level = update->Workload_level;
			exception->cmd.work_load_up = 1;

			resume_mod_flag = 1;
			return 0;
		}


		if (1 == product_type)
		{
			if (READY_TO_COOL_DOWN != rt->workout_state && setup->Work_mode == WP_FITNESS_RUN)
				return 0;
		}
		else
		{
			if (READY_TO_COOL_DOWN != rt->workout_state && (setup->Work_mode == WP_CALORIE_GOAL_RUN || setup->Work_mode == WP_FITNESS_RUN))
				return 0;
		}

		if (READY_TO_SUMMARY == rt->workout_state)	return 0;
		if (READY_TO_FINISH == rt->workout_state)		return 0;
		//if (READY_TO_COOL_DOWN == rt->workout_state)	return 0;

		data->ResistanceLevel = update->Workload_level;
		if (data->ResistanceLevel >= top)				data->ResistanceLevel = top;
		if (data->ResistanceLevel <= 1)					data->ResistanceLevel = 2;

		//rt->currnet_work_load_level = update->Workload_level = rt->Target_Workload_level = --data->ResistanceLevel;
		update->Workload_level = rt->Target_Workload_level = --data->ResistanceLevel;
		exception->cmd.work_load_up = 1;
		//if (rt->Target_Pace_Level > 20)
		//	rt->Target_Pace_Level = 20;
		//exception->cmd.pace_up = 1;
		printf("(%s %d) WORKLOAD_DOWN=%d %d\n", __FILE__, __LINE__, data->ResistanceLevel, update->Workload_level);
		return 0;
	}

	if (KS_START == key)
	{
		if (0 == exception->cmd.start)
		{
			printf("KS_START, BUT (0 == exception->cmd.start)\n");
			return 0;
		}
		if (1 == exception->cmd.pause)
		{
			exception->cmd.pause = 0;
			return 0;
		}
		return 0;
	}

#if 0
	if (KS_STOP == key)
	{
		if (0 == exception->cmd.stop && 1 == exception->cmd.start)
		{
			if (0 == exception->cmd.pause)
			{
				exception->cmd.pause = 1;
				return 0;
			}
			exception->cmd.stop = 1;
			exception->cmd.pause = 0;
		}
		return 0;
	}
#endif

	if (1 == source)
	{
		// SOURCE IPOD
		if (KS_LEFT_TOP == key)
		{
			return 0;
		}

		// COOL DOWN
		if (KS_LEFT_BOTTOM == key)
		{
			if (exception->cmd.cool_down == 1)
				return 0;
			if (READY_TO_COOL_DOWN != rt->workout_state)			exception->cmd.cool_down = 1;
			return 0;
		}

		// HRC CRUISE 
		if (KS_RIGHT_TOP == key)
		{
			if (0 == shift)
			{
				update->Target_heart_rate = 0;
				exception->cmd.hr_cruise = 0;
				setup->Target_heart_rate = saved_target_hr;
				printf("(%s %d) EXIT HRC CRUISE\n", __FILE__, __LINE__);
			}
			else
			{
				saved_target_hr = setup->Target_heart_rate;
				update->Target_heart_rate = setup->Target_heart_rate = rt->target_cruise_heart_rate = update->Heart_rate;

				for (int i = rt->segment_index; i < rt->workLoad_TableUsed; i++)
				{
					if (setup->Work_mode == WP_RANDOM_HILLS_RUN || setup->Work_mode == WP_SINGLE_HILL_RUN 
						|| setup->Work_mode == WP_ROLLING_HILLS_RUN  || setup->Work_mode == WP_GLUTE_BUSTER_RUN || setup->Work_mode == WP_CARDIO_CHALLENGE_RUN)
					{
						rt->watt_calc_mod = CALC_BY_INDEXED_LOAD_LEVEL;
						rt->workLoad_Table_cruise[i] = rt->workLoad_Table[rt->segment_index];
					}
					else
					{
						rt->workLoad_Table_cruise[i] = rt->workLoad.current_load_level;
					}
				}

				if (rt->watt_calc_mod == CALC_BY_CURRENT_LOAD_LEVEL)
					update->Workload_level = rt->workLoad.current_load_level;
				else
					update->Workload_level = rt->workLoad_Table_cruise[rt->segment_index];

				if(rt->base_cruise_watt == 0)
					rt->base_cruise_watt = tables->Get_60rpm_Watt_ByLevel(rt->workLoad_Table_cruise[rt->segment_index]);

				exception->cmd.hr_cruise = 1;
				printf("(%s %d) ENTER HRC CRUISE (%d)\n", __FILE__, __LINE__, update->Target_heart_rate);
			}
			return 0;
		}
		return 0;
	}


	if (2 == source)
	{
		// SOURCE IPOD
		if (KS_LEFT_TOP == key)
		{
			return 0;
		}

		// COOL DOWN
		if (KS_LEFT_BOTTOM == key)
		{
			//printf("(%s %d)IN exception->cmd.cool_down = %d  \n", __FILE__, __LINE__,exception->cmd.cool_down);
			if (exception->cmd.cool_down == 1)
				return 0;
			if (READY_TO_COOL_DOWN != rt->workout_state)			exception->cmd.cool_down = 1;

			//printf("(%s %d)OUT exception->cmd.cool_down = %d  \n", __FILE__, __LINE__,exception->cmd.cool_down);
			return 0;
		}

		// HRC CRUISE
		if (KS_RIGHT_TOP == key)
		{
			if (0 == shift)
			{
				update->Target_heart_rate = 0;
				exception->cmd.hr_cruise = 0;
				setup->Target_heart_rate = saved_target_hr;
				printf("(%s %d) EXIT HRC CRUISE\n", __FILE__, __LINE__);
			}
			else
			{
				saved_target_hr = setup->Target_heart_rate;
				update->Target_heart_rate = setup->Target_heart_rate = rt->target_cruise_heart_rate = update->Heart_rate;

				for (int i = rt->segment_index; i < rt->workLoad_TableUsed; i++)
				{
					if (setup->Work_mode == WP_RANDOM_HILLS_RUN || setup->Work_mode == WP_SINGLE_HILL_RUN 
						|| setup->Work_mode == WP_ROLLING_HILLS_RUN  || setup->Work_mode == WP_GLUTE_BUSTER_RUN || setup->Work_mode == WP_CARDIO_CHALLENGE_RUN)
					{
						rt->watt_calc_mod = CALC_BY_INDEXED_LOAD_LEVEL;
						rt->workLoad_Table_cruise[i] = rt->workLoad_Table[rt->segment_index];
					}
					else
					{
						rt->workLoad_Table_cruise[i] = rt->workLoad.current_load_level;
					}
				}

				if (rt->watt_calc_mod == CALC_BY_CURRENT_LOAD_LEVEL)
					update->Workload_level = rt->workLoad.current_load_level;
				else
					update->Workload_level = rt->workLoad_Table[rt->segment_index];

				if(rt->base_cruise_watt == 0)
					rt->base_cruise_watt = tables->Get_60rpm_Watt_ByLevel(rt->workLoad_Table_cruise[rt->segment_index]);

				exception->cmd.hr_cruise = 1;
				printf("(%s %d) ENTER HRC CRUISE (%d)\n", __FILE__, __LINE__, update->Target_heart_rate);
			}
			return 0;
		}
		return 0;
	}


	if (3 == source)
	{
		// SCROLL DATA
		if (KS_LEFT_TOP == key)
		{
			return 0;
		}

		// CHANGE DATA
		if (KS_LEFT_CENTER == key)
		{
			return 0;
		}

		// FAN
		if (KS_LEFT_BOTTOM == key)
		{
			return 0;
		}

		// KPH
		if (KS_RIGHT_TOP == key)
		{
			return 0;
		}

		return 0;
	}


	if (4 == source)
	{
		// SOURCE IPOD
		if (KS_LEFT_TOP == key)
		{

#if 0
			// 發電機
			if (2 == product_type)
			{
				gpio_content_t ctx;

				if (0 == rt->audio.audio_source)
				{
					ctx.pin_index = MK_GPIO_INDEX(2, 0, 62, 19);
					ctx.value = 1;
					ioctl(gpio, GPIO_IOCTL_SET_VALUE, &ctx);
					rt->audio.audio_source = 1;
				}
				else
				{
					ctx.pin_index = MK_GPIO_INDEX(2, 0, 62, 19);
					ctx.value = 0;
					ioctl(gpio, GPIO_IOCTL_SET_VALUE, &ctx);
					rt->audio.audio_source = 0;
				}
			}
			// BIKE, SD55
			else
			{
				unsigned char V = 0;

				i2c_read_data(i2c0, PCA_9555_SLAVE_1, PCA_9555_INPUT_0, &V, 1);
				if (0 == rt->audio.audio_source)
				{
					V |= BIT7;
					if (0 == product_type || 1 == product_type)
						V |= BIT5;
					i2c_write_data(i2c0, PCA_9555_SLAVE_1, PCA_9555_OUTPUT_0, &V, 1);
					rt->audio.audio_source = 1;
				}
				else
				{
					V &= ~BIT7;
					if (0 == product_type || 1 == product_type)
						V |= BIT5;	// LCD_BL-EN
					i2c_write_data(i2c0, PCA_9555_SLAVE_1, PCA_9555_OUTPUT_0, &V, 1);
					rt->audio.audio_source = 0;
				}
			}
#endif // 0

			return 0;
		}


		// CH NEXT, 
		// NEXT/PREV
		if (KS_LEFT_CENTER == key)
		{
			if (-1 == ipod)
				return 0;
			if (0 == shift)
			{
				for (int i = 0; i < 3; i++)
					KillTimer(NEXT_TRACK_TIMER_ID);
				return 0;
			}
			SetTimer(NEXT_TRACK_TIMER_ID, 17, IpodNextTrack);
			//SetTimer(DELAY_NEXT_TRACK_TIMER_ID, 17, 0);
			return 0;
		}
		// CH PREV
		if (KS_LEFT_BOTTOM == key)
		{
			if (-1 == ipod)
				return 0;
			if (0 == shift)
			{
				for (int i = 0; i < 3; i++)	
					KillTimer(PREV_TRACK_TIMER_ID);
				return 0;
			}
			//SetTimer(PREV_TRACK_TIMER_ID, 17, IpodPrevTrack);
			SetTimer(DELAY_PREV_TRACK_TIMER_ID, 43, 0);
			return 0;
		}

		// MUTE
		if (KS_RIGHT_TOP == key)
		{
			IPOD_MUTE_VAR = !IPOD_MUTE_VAR;

			if (IPOD_MUTE_VAR)
				ipod_vol_tab(i2c0, 0);
			else
				ipod_vol_tab(i2c0, IPOD_VOL_VAR);
			return 0;
		}

		// V+
		if (KS_RIGHT_CENTER == key)
		{
			IPOD_MUTE_VAR = 0;
			if (IPOD_VOL_VAR < 15)
			{
				IPOD_VOL_VAR++;
				ipod_vol_tab(i2c0, IPOD_VOL_VAR);
			}
			return 0;
		}

		// V-
		if (KS_RIGHT_BOTTOM == key)
		{
			IPOD_MUTE_VAR = 0;
			if (IPOD_VOL_VAR > 0)
			{
				IPOD_VOL_VAR--;
				ipod_vol_tab(i2c0, IPOD_VOL_VAR);
			}
			return 0;
		}

		return 0;
	}


	if (5 == source)
	{
		// AUTO STRIDE
		if (KS_LEFT_CENTER == key)
		{
			if (0 != product_type)						return 0;

			if (READY_TO_SUMMARY == rt->workout_state)	return 0;
			if (READY_TO_FINISH == rt->workout_state)		return 0;
			//if (READY_TO_COOL_DOWN == rt->workout_state)	return 0;
			exception->cmd.auto_stride = !exception->cmd.auto_stride;
			return 0;
		}

		// S+
		if (KS_RIGHT_TOP == key)
		{
			if (0 != product_type)						return 0;
			if (1 == exception->cmd.pause)					return 0;

			if (READY_TO_SUMMARY == rt->workout_state)	return 0;
			if (READY_TO_FINISH == rt->workout_state)		return 0;
			//if (READY_TO_COOL_DOWN == rt->workout_state)	return 0;

			if (update->Stride_length > 0)					data->StrideLength = update->Stride_length / 5;
			else											data->StrideLength = DEFAULT_STRIDE_LENGTH;

			if(data->StrideLength >= MAX_STRIDE_LENGTH)	data->StrideLength = MAX_STRIDE_LENGTH - 1;
			data->StrideLength++;
			update->Stride_length = rt->Target_Stride = data->StrideLength * 5;
			exception->cmd.stride_up = 1;
			exception->cmd.auto_stride = 0;
printf("(%s %d) STRIDE_UP=%d %d\n", __FILE__, __LINE__, data->StrideLength, rt->Target_Stride);
			return 0;
		}

		// S-
		if (KS_RIGHT_CENTER == key)
		{
			if (0 != product_type)						return 0;
			if (1 == exception->cmd.pause)					return 0;

			if (READY_TO_SUMMARY == rt->workout_state)	return 0;
			if (READY_TO_FINISH == rt->workout_state)		return 0;
			//if (READY_TO_COOL_DOWN == rt->workout_state)	return 0;

			if (update->Stride_length > 0)					data->StrideLength = update->Stride_length / 5;
			else											data->StrideLength = DEFAULT_STRIDE_LENGTH;

			if(data->StrideLength <= MIN_STRIDE_LENGTH)	data->StrideLength = MIN_STRIDE_LENGTH + 1;
			data->StrideLength--;
			update->Stride_length = rt->Target_Stride = data->StrideLength * 5;
			exception->cmd.stride_up = 1;
			exception->cmd.auto_stride = 0;
printf("(%s %d) STRIDE_DOWN=%d %d\n", __FILE__, __LINE__, data->StrideLength, rt->Target_Stride);
			return 0;
		}

		return 0;
	}


	if (6 == source)
	{
		double p = 0;

		// Weight+
		if (KS_LEFT_TOP == key || KS_WORKLOAD_UP == key)
		{
			if (READY_TO_SUMMARY == rt->workout_state)	return 0;
			if (READY_TO_FINISH == rt->workout_state)		return 0;
			if (READY_TO_COOL_DOWN == rt->workout_state)	return 0;

			p = setup->weight * (double)2.2F;
			if (p >= 350)									p = 350;
			else if (p < 350 && p >= 40)
			{
				p += 1;
			}
			else											p = 40;
			setup->weight = (double)p / (double)2.2F;
			setup->Weight = (unsigned short)round(setup->weight);
printf("(%s %d) setup->Weight=%d %f\n", __FILE__, __LINE__, setup->Weight, setup->weight);
			return 0;
		}

		// Weight-
		if (KS_LEFT_CENTER == key || KS_WORKLOAD_DOWN == key)
		{
			if (READY_TO_SUMMARY == rt->workout_state)	return 0;
			if (READY_TO_FINISH == rt->workout_state)		return 0;
			if (READY_TO_COOL_DOWN == rt->workout_state)	return 0;

			p = setup->weight * (double)2.2F;
			if ((int)p <= 40)
				p = 40;
			else if (p <= 350 && p > 40)
			{
				p -= 1;
			}
			else												p = 350;
			setup->weight = (double)p / (double)2.2F;
			setup->Weight = (unsigned short)round(setup->weight);
printf("(%s %d) setup->Weight=%d %f\n", __FILE__, __LINE__, setup->Weight, setup->weight);
			return 0;
		}

		// HRC CRUISE
		if (KS_RIGHT_TOP == key)
		{
			if (0 == shift)
			{
				update->Target_heart_rate = 0;
				exception->cmd.hr_cruise = 0;
				setup->Target_heart_rate = saved_target_hr;
				printf("(%s %d) EXIT HRC CRUISE\n", __FILE__, __LINE__);
			}
			else
			{
				saved_target_hr = setup->Target_heart_rate;
				update->Target_heart_rate = setup->Target_heart_rate = rt->target_cruise_heart_rate = update->Heart_rate;

				for (int i = rt->segment_index; i < rt->workLoad_TableUsed; i++)
				{
					if (setup->Work_mode == WP_RANDOM_HILLS_RUN || setup->Work_mode == WP_SINGLE_HILL_RUN 
						|| setup->Work_mode == WP_ROLLING_HILLS_RUN  || setup->Work_mode == WP_GLUTE_BUSTER_RUN || setup->Work_mode == WP_CARDIO_CHALLENGE_RUN)
					{
						rt->watt_calc_mod = CALC_BY_INDEXED_LOAD_LEVEL;
						rt->workLoad_Table_cruise[i] = rt->workLoad_Table[rt->segment_index];
					}
					else
					{
						rt->workLoad_Table_cruise[i] = rt->workLoad.current_load_level;
					}
				}

				if (rt->watt_calc_mod == CALC_BY_CURRENT_LOAD_LEVEL)
					update->Workload_level = rt->workLoad.current_load_level;
				else
					update->Workload_level = rt->workLoad_Table[rt->segment_index];

				if(rt->base_cruise_watt == 0)
					rt->base_cruise_watt = tables->Get_60rpm_Watt_ByLevel(rt->workLoad_Table_cruise[rt->segment_index]);

				exception->cmd.hr_cruise = 1;
				printf("(%s %d) ENTER HRC CRUISE (%d)\n", __FILE__, __LINE__, update->Target_heart_rate);
			}
			return 0;
		}

		// BIKE
		if (KS_RIGHT_BOTTOM == key)
		{
			return 0;
		}

		return 0;
	}


///////////////////////////////////////////////////////////
printf("(%s %d) OnDataScreenChange(NULL)\n", __FILE__, __LINE__);
	return 0;
}


LRESULT CFpcApp::OnLeftThumb(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
	int up = 0;
	struct timeb tb;
	unsigned int now;
	unsigned char top = 30;

	if (0 == wParam)		up = 0;
	else					up = 1;

	if (IN_NORMAL == rt->workout_state)							return 0;
	if (READY_TO_SUMMARY == rt->workout_state)				return 0;
	if (READY_TO_FINISH == rt->workout_state)					return 0;
	if (READY_TO_COOL_DOWN == rt->workout_state)				return 0;
	if (setup->Work_mode == WP_CARDIO_CHALLENGE_RUN)
		return 0;

	if (WP_GLUTE_BUSTER_RUN == setup->Work_mode)			top = 20;
	if (WP_ROLLING_HILLS_RUN == setup->Work_mode)			top = 20;
	if (WP_ROLLING_HILLS_RUN == setup->Work_mode)			top = 20;
	if (WP_HILL_INTVALS_RUN == setup->Work_mode)				top = 20;
	if (WP_SINGLE_HILL_RUN == setup->Work_mode)				top = 20;
	if (WP_RANDOM_HILLS_RUN == setup->Work_mode)			top = 20;
	if (WP_CARDIO_CHALLENGE_RUN == setup->Work_mode)		top = 20;


	ftime(&tb); now = tb.time * 1000 + tb.millitm;
	if (0 == old_thumb_ts || now - old_thumb_ts > 200)
	{
		if (1 == up)
		{
			if (rt->Target_Workload_level >= top)				return 0;
			rt->Target_Workload_level++;
			update->Workload_level = rt->Target_Workload_level;
			exception->cmd.work_load_up = 1;
			//if (rt->Target_Pace_Level > 20)
			//	rt->Target_Pace_Level = 20;
			//exception->cmd.pace_up = 1;
		}
		else
		{
			if (rt->Target_Workload_level < 1)					return 0;
			rt->Target_Workload_level--;
			update->Workload_level = rt->Target_Workload_level;
			exception->cmd.work_load_up = 1;
			//if (rt->Target_Pace_Level > 20)
			//	rt->Target_Pace_Level = 20;
			//exception->cmd.pace_up = 1;
		}
	}
	old_thumb_ts = now;
	return 1;
}

LRESULT CFpcApp::OnRightThumb(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult)
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
	if (setup->Work_mode == WP_CARDIO_CHALLENGE_RUN)
		return 0;

	ftime(&tb); now = tb.time * 1000 + tb.millitm;
	if (0 == old_thumb_ts || now - old_thumb_ts > 200)
	{
		if (1 == up)
		{
			if(data->StrideLength >= MAX_STRIDE_LENGTH)
			{
				data->StrideLength = MAX_STRIDE_LENGTH;
				return 1;
			}
			data->StrideLength++;
			update->Stride_length = rt->Target_Stride = data->StrideLength * 5;
			exception->cmd.stride_up = 1;
		}
		else
		{
			if(data->StrideLength <= MIN_STRIDE_LENGTH)
			{
				data->StrideLength = MIN_STRIDE_LENGTH;
				return 1;
			}
			data->StrideLength--;
			update->Stride_length = rt->Target_Stride = data->StrideLength * 5;
			exception->cmd.stride_up = 1;
		}
	}
	old_thumb_ts = now;
	return 1;
}


LRESULT CFpcApp::OnKeyScanReleased(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{

/*
	if (wParam == KS_RIGHT_TOP)				OnRightUpButton(dwType, wParam, lParam, pResult);
	if (wParam == KS_RIGHT_CENTER)			OnRightCenterButton(dwType, wParam, lParam, pResult);

	if (wParam == KS_LEFT_TOP)				OnLeftUpButton(dwType, wParam, lParam, pResult);
	if (wParam == KS_LEFT_CENTER)			OnLeftCenterButton(dwType, wParam, lParam, pResult);


	if (wParam == KS_LEFT_BOTTOM)			OnLeftDownButton(dwType, wParam, lParam, pResult);
	if (wParam == KS_RIGHT_BOTTOM)			OnRightDownButton(dwType, wParam, lParam, pResult);

	if (wParam == KS_START)					OnStartButton(dwType, wParam, lParam, pResult);
	if (wParam == KS_STOP)					OnStopButton(dwType, wParam, lParam, pResult);
	if (wParam == KS_DELETE)					OnDeleteButton(dwType, wParam, lParam, pResult);
	if (wParam == KS_WORKLOAD_UP)			OnScrollUpButton(dwType, wParam, lParam, pResult);
	if (wParam == KS_WORKLOAD_DOWN)		OnScrollDownButton(dwType, wParam, lParam, pResult);
	if (wParam == KS_STRIDE_UP)				OnStrideUpButton(dwType, wParam, lParam, pResult);
	if (wParam == KS_STRIDE_DOWN)			OnStrideDownButton(dwType, wParam, lParam, pResult);

	if (wParam == KS_NUM0 || wParam == KS_NUM1 || wParam == KS_NUM2 || wParam == KS_NUM3 || wParam == KS_NUM4 || wParam == KS_NUM5 || wParam == KS_NUM6 || wParam == KS_NUM7 || wParam == KS_NUM8 || wParam == KS_NUM9)
	{
		OnNumButton(dwType, (wParam - KS_NUM0), lParam, pResult);
	}
*/


	/*if (wParam == KS_RIGHT_TOP)				printf("KS_RIGHT_TOP, %d\n", wParam);
	if (wParam == KS_RIGHT_CENTER)			printf("KS_RIGHT_CENTER, %d\n", wParam);
	if (wParam == KS_LEFT_TOP)				printf("KS_LEFT_TOP, %d\n", wParam);
	if (wParam == KS_LEFT_CENTER)			printf("KS_LEFT_CENTER, %d\n", wParam);
	if (wParam == KS_LEFT_BOTTOM)			printf("KS_LEFT_BOTTOM, %d\n", wParam);
	if (wParam == KS_RIGHT_BOTTOM)			printf("KS_RIGHT_BOTTOM, %d\n", wParam);
	if (wParam == KS_START)					printf("KS_START, %d\n", wParam);
	if (wParam == KS_STOP)					printf("KS_STOP, %d\n", wParam);
	if (wParam == KS_DELETE)					printf("KS_DELETE, %d\n", wParam);
	if (wParam == KS_WORKLOAD_UP)			printf("KS_WORKLOAD_UP, %d\n", wParam);
	if (wParam == KS_WORKLOAD_DOWN)		printf("KS_WORKLOAD_DOWN, %d\n", wParam);
	if (wParam == KS_STRIDE_UP)				printf("KS_STRIDE_UP, %d\n", wParam);
	if (wParam == KS_STRIDE_DOWN)			printf("KS_STRIDE_DOWN, %d\n", wParam);
	if (wParam == KS_NUM0 || wParam == KS_NUM1 || wParam == KS_NUM2 || wParam == KS_NUM3 || wParam == KS_NUM4 || wParam == KS_NUM5 || wParam == KS_NUM6 || wParam == KS_NUM7 || wParam == KS_NUM8 || wParam == KS_NUM9)
	{
		printf("KS_NUM:%d, %d\n", wParam - KS_NUM0, wParam);
	}*/

	if (wParam == KS_NUM5)
	{
		//printf("update->Workload_level=%d rt->workLoad.current_load_level=%d\n", update->Workload_level, rt->workLoad.current_load_level);
		//printf("update->Target_heart_ratel=%d %d\n", update->Target_heart_rate, setup->Target_heart_rate);
		printf("%d %d %d %d %d\n", 
			setup->Workload_level, update->Workload_level, rt->workLoad.current_load_level, rt->workLoad_Table[rt->segment_index], data->ResistanceLevel);
	}


	return 1;
}


char test_count = 0;
LRESULT CFpcApp::OnKeyScanPressed(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
	if (prob_pca955_1)
	{
		//if (0 == buzz_type)
		SetTimer(BUZZ_ON_TIMER_ID, 31, BuzzOnTimer);
	}

	return 1;
}


unsigned char ADC = 0;

LRESULT CFpcApp::OnWaitFpcEndOfProgram(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
	unsigned char out[1024 * 4];

	struct timeb tb;
	unsigned int now;
	unsigned int old = (unsigned int)lParam;

	ftime(&tb); now = tb.time * 1000 + tb.millitm;
	if (now - old > 30)
	{
		packOurProtocol(CMD_GUI | (1 << 6), 0, 15, (unsigned char *)reply17, out);
		if (write((int)wParam, (const void *)out, 15 + 8) <= 0) {}
	}
	else
	{
//printf("(%s %d) OnWaitFpcEndOfProgram() <= 100\n",  __FILE__, __LINE__);
		PostMessage(M_WAIT_FPC_END_OF_PROGRAM, wParam, lParam);
	}

	return 1;
}


LRESULT CFpcApp::OnTimer(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
	UINT TimerId = (UINT)wParam;


///////////////////////////////////////////////////////////
	// 測試用
	/*if (8100 == TimerId)
	{
		SetTimer(BUZZ_ON_TIMER_ID, 31, BuzzOnTimer);
		//SetTimer(8100, 29003, 0);
		return 1;
	}*/

///////////////////////////////////////////////////////////
	if (CHK_BIKE_MOTO_ERR_TIMER_ID == TimerId)
	{

	SendCmdB(ttyUSB0,0,0x001C ,0);
	//	int D = (int)T_AD - (int)C_AD;

	//	if (D > 3 || D < -3)
	//		key_scan[24] = 1;
	//	else
	//		key_scan[24] = 0;
		return 1;
	}


///////////////////////////////////////////////////////////
	if (DELAY_NEXT_TRACK_TIMER_ID == TimerId)
	{
		SetTimer(NEXT_TRACK_TIMER_ID, 15, IpodNextTrack);
		return 1;
	}
	if (DELAY_PREV_TRACK_TIMER_ID == TimerId)
	{
		SetTimer(PREV_TRACK_TIMER_ID, 15, IpodPrevTrack);
		return 1;
	}
	if (IPOD_SELECT_TIMER_ID == TimerId)
	{
		if (-1 == ipod)			return 1;
		iPod_SendCmd(ipod, iPod_CMD_SELECT);
		return 1;
	}
	if (IPOD_BUTTONRELEASE_TIMER1_ID == TimerId || IPOD_BUTTONRELEASE_TIMER2_ID == TimerId)
	{
		if (-1 == ipod)			return 1;
		iPod_SendCmd(ipod, iPod_CMD_BUTTONRELEASE);
		printf("(%s %d) IPOD_BUTTONRELEASE\n", __FILE__, __LINE__);
		return 1;
	}
	if (IPOD_NEXT_TIMER_ID == TimerId)
	{
		if (-1 == ipod)			return 1;
		iPod_SendCmd(ipod, iPod_CMD_NEXT);
		SetTimer(IPOD_BUTTONRELEASE_TIMER1_ID, 121, 0);
		return 1;
	}
	if (IPOD_PREV_TIMER_ID == TimerId)
	{
		if (-1 == ipod)			return 1;
		iPod_SendCmd(ipod, iPod_CMD_PREV);
		SetTimer(IPOD_BUTTONRELEASE_TIMER1_ID, 121, 0);
		return 1;
	}
	if (IPOD_SKIP_INC_TIMER_ID == TimerId)
	{
		if (-1 == ipod)			return 1;
		iPod_SendCmd(ipod, iPod_CMD_SKIP_INC);
		SetTimer(IPOD_BUTTONRELEASE_TIMER1_ID, 121, 0);
		return 1;
	}
	if (IPOD_SKIP_DEC_TIMER_ID == TimerId)
	{
		if (-1 == ipod)			return 1;
		iPod_SendCmd(ipod, iPod_CMD_SKIP_DEC);
		SetTimer(IPOD_BUTTONRELEASE_TIMER1_ID, 121, 0);
		return 1;
	}
	if (IPOD_SIMPLE_MODE_TIMER_ID == TimerId)
	{
		if (-1 == ipod)			return 1;
		iPod_SendCmd(ipod, iPod_CMD_MODE_2);
		SetTimer(IPOD_BUTTONRELEASE_TIMER1_ID, 121, 0);
		return 1;
	}

	if (IPOD_PREV_FLAG_TIMER_ID == TimerId)
	{
		if (0 == IPOD_PREV_FLAG)
			return 1;
		if (1 == IPOD_PREV_FLAG)
		{
			SetTimer(IPOD_SIMPLE_MODE_TIMER_ID, 1, 0);
			SetTimer(IPOD_SKIP_DEC_TIMER_ID, 23, 0);
			IPOD_PREV_FLAG = 0;
			return 0;
		}
		SetTimer(IPOD_SIMPLE_MODE_TIMER_ID, 1, 0);
		SetTimer(IPOD_PREV_TIMER_ID, 23, 0);
		IPOD_PREV_FLAG = 0;
		return 1;
	}

///////////////////////////////////////////////////////////
	if (FPC_WORKOUT_IN_NORMAL_ID == TimerId)
	{
		rt->workout_state = IN_NORMAL;
		return 1;
	}

///////////////////////////////////////////////////////////
	if (FPC_SCHEDULER_10MS_ID == TimerId)
	{
		data->Clear_Rpm_Delay++;
		//SetTimer(TimerId, 11, 0);
		SetTimer(TimerId, 47, 0);
		return 1;
	}

///////////////////////////////////////////////////////////
	if (SOC_TO_MCU_SAY_HELLO == TimerId)
	{
		printf("SOC_TO_MCU_SAY_HELLO %d",test_count);


		SendCmdB(ttyUSB0,0,0x0030,0);
		switch(test_count)
		{
		case 0 :
		SendCmdB(ttyUSB0,0,0x0000,0);
		break;
		case 1 :
		//SendCmdB(ttyUSB0,0,0x0010,0);
		break;
		case 2 :
		SendCmdB(ttyUSB0,0,0x0011,0);
		break;
		case 3 :
		SendCmdB(ttyUSB0,0,0x0012,0);
		break;
		case 4 :
		SendCmdB(ttyUSB0,0,0x0016,0);
		break;
		case 5:
		SendCmdB(ttyUSB0,1,0x0018,1);
		break;
		case 6 :
		SendCmdB(ttyUSB0,1,0x001b,5);
		break;
		case 7 :
		SendCmdB(ttyUSB0,0,0x0020,0);
		break;
		case 8 :
		SendCmdB(ttyUSB0,0,0x0021,0);
		break;
		case 9 :
		SendCmdB(ttyUSB0,0,0x0022,0);
		break;
		case 10 :
		SendCmdB(ttyUSB0,1,0x0023,5);
		break;
		case 11 :
		SendCmdB(ttyUSB0,1,0x0024,5);
		break;
		
		
		}

		
		
	/*	test_count++;
		if(test_count>11)
		{
		 test_count =0;
		}
		SetTimer(SOC_TO_MCU_SAY_HELLO, 25,0); */
		return 1;
	}

///////////////////////////////////////////////////////////

	if (SD55_CMD_CC_INIT_ID == TimerId)
	{
		lcb->cc->state.New_SPU_Pulse = 0;
		lcb->data->Drive_SPU_Interval = 1;

		if (-1 != ttyUSB0 && 0 == product_type)
		{
			//SendCmd(ttyUSB0, (int)SERIAL_CMD_INIT, 1, &ch);
			//	SendCmd(ttyUSB0, (int)SERIAL_RESET_CONTORLER, 0, 0); chuck modify
			
			SendCmdB(ttyUSB0,0,0x0021,0);

			SetTimer(SD55_SND_CHK_TIMER_ID, 10001, Sd55ChkSndTimer);
		}
		SetTimer(SD55_CMD_RD_SPU_ID, 301, 0);
		return 1;
	}



///////////////////////////////////////////////////////////

	if (SD55_CMD_RD_SPU_ID == TimerId)
	{
		static int once = 0;
		//////////////////////////////////////////////
		// 非 SD55
		if (1 == product_type || 2 == product_type)
		{
			if (0 == once)
			{
				once = 1;
				// MBREAK
				//if (prob_pca955_1)
					SetTimer(SD55_CMD_DEFAULT_STRIDE_ID, 1, 0);

			//	if (prob_pca955_1)// && 2 != product_type)
			//	{
					// Process SPU
					SetTimer(FPC_SCHEDULER_200MS_ID, 499, SpuTimer);
					// HeartRate_Sampling_HGP
// jason note
					//SetTimer(FPC_SCHEDULER_1MS_ID, 1, HrTimer);
			//	}

				// Clear_Rpm_Delay++
				//SetTimer(FPC_SCHEDULER_10MS_ID, 101, ClearRpmTimer);
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
			SetTimer(FPC_SCHEDULER_200MS_ID, 197, SpuTimer);

			// HeartRate_Sampling_HGP
			//if (prob_pca955_1)
			//	SetTimer(FPC_SCHEDULER_1MS_ID, 1, HrTimer);

			// Clear_Rpm_Delay++
			SetTimer(FPC_SCHEDULER_10MS_ID, 101, ClearRpmTimer);

			//SetTimer(SD55_CMD_DEFAULT_STRIDE_ID, 1000 * 30, 0);
			SetTimer(SD55_CMD_DEFAULT_STRIDE_ID, 17001, 0);
		}
		//SendCmdB(ttyUSB0,0,0x0020,0);
		// SendCmd(ttyUSB0, (int)SERIAL_CMD_RD_SPU_INTERVAL, 0, 0); chuck modify
		SetTimer(SD55_CMD_RD_SPU_ID, 443, 0);
		return 1;
	}

///////////////////////////////////////////////////////////
	if (SD55_CMD_DEFAULT_STRIDE_ID == TimerId)
	{
		//exception->cmd.auto_stride = 0;
		data->StrideLength = DEFAULT_STRIDE_LENGTH;
		update->Stride_length = rt->Target_Stride = data->StrideLength * 5;
		Set_Stride_Motor_Position(DEFAULT_STRIDE_LENGTH);
	
		rt->workLoad.current_load_level = update->Workload_level = data->ResistanceLevel = default_work_load_level;
		//rt->Target_Workload_level
		Set_WorkLoad_Motor_Position(default_work_load_level);

		rt->Target_Pace_Level = 1;
		exception->cmd.pace_up = 1;
		return 1;
	}

	return 1;
}

LRESULT CFpcApp::OnInitInstance(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
	//unsigned char ch = 0X0F;
	//gpio_content_t ctx;

	int err;
	//unsigned char v;
	//int res;
	//struct gp_pwm_config_s attr;

	FILE *fp = 0;
	char tmp[256];
	char compare[32];
	struct stat lbuf;

	DIR *dirp;
	struct dirent *dp;

	int fd = -1;
	char buf[32]; 
	char off = '1';



///////////////////////////////////////////////////////////
	fd = open("/sys/class/gpio/export", O_WRONLY);
	sprintf(buf, "%d", 245);
	write(fd, buf, strlen(buf));
	close(fd);

	sprintf(buf, "/sys/class/gpio/gpio%d/direction", 245);
	fd = open(buf, O_WRONLY);
	write(fd, "out", 3);
	close(fd);

	fd = open("/sys/class/gpio/gpio245/value", O_WRONLY);
	if (-1 != fd)
	{
		write(fd, (void *)&off, 1);
		close(fd);
	}
	ipod_vol_tab(i2c0, IPOD_VOL_VAR);


///////////////////////////////////////////////////////////
	fd = open("/sys/class/gpio/export", O_WRONLY);
	sprintf(buf, "%d", 244);
	write(fd, buf, strlen(buf));
	close(fd);

	sprintf(buf, "/sys/class/gpio/gpio%d/direction", 244);
	fd = open(buf, O_WRONLY);
	write(fd, "in", 2);
	close(fd);

	SetTimer(IPOD_DETECT_TIMER_ID, 201, IpodDetectTimer);
// ???
	rt->audio.audio_source = 1;


///////////////////////////////////////////////////////////
	if(0 == stat("/mnt/sdcard/TSC800", &lbuf))
	{
		if (!S_ISDIR(lbuf.st_mode))
			goto sdcard_dir_out;
		if ((dirp = opendir("/mnt/sdcard/TSC800")) == NULL)
			goto sdcard_dir_out;
		while ((dp = readdir(dirp)) != NULL)
		{
			sprintf(tmp, "/mnt/sdcard/TSC800/%s", dp->d_name);
			if(0 == stat(tmp, &lbuf))
			{
				if (S_ISREG(lbuf.st_mode) && 0 == lbuf.st_size)
				{
					unlink(tmp);
					sync();
					printf("(%s %d) DELETE(\"%s\")\n", __FILE__, __LINE__, tmp);
				}
			}
		}
		closedir(dirp);
	}
sdcard_dir_out:



///////////////////////////////////////////////////////////
	CSd55App::OnInitInstance(dwType, wParam, lParam, pResult);

	pthread_attr_t sched_attr;
	int fifo_prio;
	struct sched_param fifo_param;

	pthread_attr_init(&sched_attr);
	pthread_attr_setinheritsched(&sched_attr, PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setscope(&sched_attr, PTHREAD_SCOPE_SYSTEM);
	pthread_attr_setschedpolicy(&sched_attr, SCHED_FIFO);
	fifo_prio = sched_get_priority_min(SCHED_FIFO);

	fifo_param.sched_priority = fifo_prio;
	pthread_attr_setschedparam(&sched_attr, &fifo_param);

///////////////////////////////////////////////////////////
	if(0 == stat("/data/arex/setup.dat", &lbuf))
	{
		if (0 != (fp = fopen("/data/arex/setup.dat", "rb")))
		{
			setbuf(fp, 0);
			fread(compare, sizeof(compare), 1, fp);
			fread(saved_setup, sizeof(saved_setup), 1, fp);
			fclose(fp), fp = 0;
			sync();
			memset(tmp, 0, sizeof(tmp));
			sprintf(tmp, "[%s %s]", __DATE__, __TIME__);
			if (0 != memcmp(compare, tmp, 32))
			{
				unlink("/data/arex/setup.dat");
				sync();
				memset(saved_setup, 0, sizeof(saved_setup));
				printf("(%s %d) UNLINK()\n", __FILE__, __LINE__);
			}
		}
	}
	else
	{
		unlink("/mnt/sdcard/TSC800/saved.xml");		//unlink("/mnt/sdcard/TSC800/settings.xml");
		sync();
	}

///////////////////////////////////////////////////////////
	if (-1 == ipod)
	{
		ipod = open("/dev/ttyS1", O_RDWR | O_NOCTTY | O_NDELAY);
		if (-1 != ipod)												SetBaudRate(ipod, 19200);
	}


///////////////////////////////////////////////////////////
/* if (-1 != (i2c0 = open("/dev/i2c-0", O_RDWR)))
	{
		if (-1 != i2c_read_data(i2c0, 0X44, 0X79, &v, 1))
		{
			prob_ms6257 = 1;
			printf("(%s %d) MS6257 PROBE OK, v=%d\n", __FILE__, __LINE__, v);
		}
		else	
		{
			//printf("(%s %d) MS6257 PROBE FAIL\n", __FILE__, __LINE__);
		}
		if (prob_ms6257)
		{
			ipod_vol_tab(i2c0, IPOD_VOL_VAR);	//ipod_vol_tab(i2c0, 0);
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
			// IN
			v |= BIT4; v |= BIT5; v |= BIT6;
			// OUT
			v &= ~BIT0; v &= ~BIT1; v &= ~BIT2; v &= ~BIT3;
			i2c_write_data(i2c0, PCA_9555_SLAVE_1, PCA_9555_CONFIG_1, &v, 1);

			i2c_read_data(i2c0, PCA_9555_SLAVE_1, PCA_9555_CONFIG_0, &v, 1);
			v |= BIT0; v |= BIT1; v |= BIT4;

			// BIT5=LCD_BL-EN, BIT6=BUZZER_ON/OFF, BIT7=AUDIO_SEL0		// OUTPUT
			v &= ~BIT2; v &= ~BIT3; v &= ~BIT5; v &= ~BIT6; v &= ~BIT7;
			i2c_write_data(i2c0, PCA_9555_SLAVE_1, PCA_9555_CONFIG_0, &v, 1);


			i2c_read_data(i2c0, PCA_9555_SLAVE_1, PCA_9555_INPUT_0, &v, 1);
			v &= ~BIT6;	// BUZZ
			if (0 == product_type || 1 == product_type)
			{
				v |= BIT5;			// BL-EN
				//v &= ~BIT7;		// 800-AS AUDIO_SEL0;
				v |= BIT7;			// 800-AS AUDIO_SEL0;
				rt->audio.audio_source = 1;
			}
			i2c_write_data(i2c0, PCA_9555_SLAVE_1, PCA_9555_OUTPUT_0, &v, 1);

			v = 0X00; res = i2c_write_data(i2c0, PCA_9555_SLAVE_2, PCA_9555_CONFIG_1, &v, 1);
			v = 0XFF; res = i2c_write_data(i2c0, PCA_9555_SLAVE_2, PCA_9555_CONFIG_0, &v, 1);
			if (-1 == res)
				printf("(%s %d) PCA_9555_SLAVE_2 PROBE FAIL\n", __FILE__, __LINE__);
			if (-1 != res)									prob_pca955_2 = 1;

			i2c_read_data(i2c0, PCA_9555_SLAVE_1, PCA_9555_INPUT_0, &v, 1);
			v |= BIT2; v |= BIT3;
			if (0 == product_type || 1 == product_type)
				v |= BIT5;	// LCD_BL-EN
			i2c_write_data(i2c0, PCA_9555_SLAVE_1, PCA_9555_OUTPUT_0, &v, 1);

			
			if (prob_pca955_1)
				hr->Init_HeartRate_Data(this->i2c0);

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
*/
///////////////////////////////////////////////////////////
	memset(setup, 0, sizeof(struct SetupData));
	WorkoutData_initialize();
	setup->Work_mode = WP_HOME;
	setup->Age = 35;
	setup->weight = 150.00F / 2.2F;
	setup->Weight = (unsigned short)(150.00F / 2.2F);
	setup->Workout_time_1000 = (99 * 60) / 1000;
	setup->Workout_time = (99 * 60) % 1000;
	setup->Calorie_goal = 90;
	setup->Segments = 10;
	setup->Workload_level = 1;
	setup->Target_heart_rate = 180;
	setup->Work_heart_rate = 100 * setup->Target_heart_rate / (220 -setup->Age);
	//setup->Rest_heart_rate = 90;
	setup->Pace_level = 1;
	setup->Workout_level = 1;
	setup->Workout_distance = 5000;
	for (int i = 0; i < 10; i++)	setup->Segments_time[i] = 120;
	for (int i = 0; i < 30; i++)	setup->Workload[i] = 1;
	for (int i = 0; i < 30; i++)	setup->Pace[i] = 1;

	//rt->audio.audio_source = 0;
	rt->workout_state = IN_NORMAL;
	update->Workload_level = rt->Target_Workload_level = 1;
	data->ResistanceLevel = 1;
	data->StrideLength = DEFAULT_STRIDE_LENGTH;


///////////////////////////////////////////////////////////
	if (!tUi)
	{
		err = pthread_create(&tUi, NULL, UiThread, this);
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

///////////////////////////////////////////////////////////
	
	//SendCmd(ttyUSB0, (int)SERIAL_CMD_INIT, 1, &ch); chuck modify no SendCmdB
	SetTimer(SD55_CMD_CC_INIT_ID, 301, 0);

	SetTimer(KEY_SCAN_TIMER_ID, 10000, KsTimer);
///////////////////////////////////////////////////////////
	if (0 == product_type || 1 == product_type)
	{
		//if (prob_pca955_1) // chuck modify
			SetTimer(FPC_SCHEDULER_1MS_ID, 1, HrTimer);
	}
	else if (2 == product_type)
	{
		SetTimer(FPC_SCHEDULER_1MS_ID, 1, HrTimer);
	}


///////////////////////////////////////////////////////////
	SetTimer(UDISK_DETECT_TIMER_ID, 2003, UdiskDetectTimer);


///////////////////////////////////////////////////////////
	for (int i = 0; i < 30; i++)
	{
		if (1 == product_type)		printf("pAsResist_Level_Table[%d]=%d\n", i, pAsResist_Level_Table[i]);
	}

	return 1;
}

LRESULT CFpcApp::OnExitInstance(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
	int i;

	for (i = 0; i < 200; i++)		usleep(1), KillTimer(FPC_SCHEDULER_1MS_ID);
	for (i = 0; i < 200; i++)		KillTimer(PWM1_TIMER_ID + i);
	for (i = 0; i < 200; i++)		KillTimer(FPC_SCHEDULER_10MS_ID + i);
	for (i = 0; i < 200; i++)		KillTimer(SD55_CMD_ID + i);

	for (i = 0; i < 200; i++)		KillTimer(PWM1_TIMER_ID + i);
	for (i = 0; i < 200; i++)		KillTimer(SD55_CMD_ID + i);


///////////////////////////////////////////////////////////
	if (prob_pca955_1)
	{
		unsigned char V;

		i2c_read_data(i2c0, PCA_9555_SLAVE_1, PCA_9555_INPUT_0, &V, 1);
		V |= BIT2; V |= BIT3;
		if (0 == product_type || 1 == product_type)
			V |= BIT5;	// LCD_BL-EN
		i2c_write_data(i2c0, PCA_9555_SLAVE_1, PCA_9555_OUTPUT_0, &V, 1);
	}

///////////////////////////////////////////////////////////
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


///////////////////////////////////////////////////////////
	CSd55App::OnExitInstance(dwType, wParam, lParam, pResult);

	return 1;
}


LRESULT CFpcApp::OnUserMessage(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
	unsigned char *frame;
	struct M2M_aHeader *aHeader;
	struct M2M_bHeader *bHeader;
	struct M2M_Message *theMsg;
	unsigned short us;

	if (0 == wParam)													return 1;


///////////////////////////////////////////////////////////
	pthread_cleanup_push(free, (void *)wParam);
	frame = ((unsigned char *)wParam) + 64;
	if (frame[6] != '5')
	{
		ProcessUserMessageB((frame + 6), (int)lParam);
		goto baitout;
	}

///////////////////////////////////////////////////////////
	aHeader = (struct M2M_aHeader *)(frame + 6);
	bHeader = (struct M2M_bHeader *)(frame + 6);

	theMsg = (struct M2M_Message *)malloc(sizeof(struct M2M_Message) + 1024);
	pthread_cleanup_push(free, (void *)theMsg);

	memset((void *)theMsg, 0, sizeof(struct M2M_Message) + 1024);
	theMsg->state.header_error = 0;
	theMsg->state.ascii_type = 0;
	switch(aHeader->head[0])
	{
	//0X35
	case '5':
		if(aHeader->head[1] != '5')										theMsg->state.header_error = 1;
		if(aHeader->head[2] != '5')										theMsg->state.header_error = 1;
		theMsg->state.ascii_type = 1;
		break;
	default:
		break;
	}
	if(theMsg->state.header_error != 1)
	{
		if(theMsg->state.ascii_type == 1)
		{
			theMsg->header.head = 0x55;
			us = aHeader->function[0]-'0'; us *= 10;
			us += aHeader->function[1]-'0'; us *= 10;
			us += aHeader->function[2]-'0';
			theMsg->header.function = (unsigned char)us;
			us = aHeader->len[0]-'0'; us *= 10;
			us += aHeader->len[1]-'0'; us *= 10;
			us += aHeader->len[2]-'0';
			theMsg->header.len = us / 4;
			//theMsg->header.data = (unsigned char *)&theMsg->header.data;
			memcpy((void *)theMsg->header.data, aHeader->data, us + 3);
			ProcessUserMessage(theMsg, (int)lParam);
		}
		else
		{
		}
	}
	pthread_cleanup_pop(1);


baitout:
	pthread_cleanup_pop(1);
	return 1;
}


//////////////////////////////////////////////////////////////////////
// member
//////////////////////////////////////////////////////////////////////

void CFpcApp::TellFpcWait(void)
{
	ResetEvent(hFpcStart);
	return;
}	

void CFpcApp::TellFpcStart(void)
{
	WakeupFpc();
	return;
}

void CFpcApp::FpcWaitForStart(void)
{
	WaitForSingleObject(hFpcStart, INFINITE);
	return;
}

void CFpcApp::WakeupFpc(void)
{
	if(exception->cmd.start != 1)
		return;

	SetEvent(hFpcStart);
	return;
}

void CFpcApp::WaitForProgramTick(void)
{
	WaitForSingleObject(hFpcTick, INFINITE);
	return;
}

unsigned char CFpcApp::UnPackMessageToBinary(struct M2M_Message *theMsg)
{
	unsigned char i,*ucp;//,exe_result;
	unsigned short us,local_checksum,remote_checksum;

	struct SetupDataB *bp;

	//decode the ASC-II mesagae portion to be binary buffer
	ucp = theMsg->header.data;

	bp=(struct SetupDataB *)setup;
	local_checksum = 0;
	remote_checksum = 0;
	for(i=0;i<theMsg->header.len;i++){
		us = (ucp[i*4]-'0')*100+(ucp[i*4+1]-'0')*10+ucp[i*4+2]-'0';
		if(i<25){
			bp->data[i]=us;
		}else{
			bp->data2[i-25]=us;
		}
		local_checksum += us;
	}
	us = (ucp[i*4]-'0')*100+(ucp[i*4+1]-'0')*10+ucp[i*4+2]-'0';
	remote_checksum = us;

	local_checksum &= 0x00FF;
	remote_checksum &= 0x00FF;	

	if(local_checksum == remote_checksum)
	{
		update->Workload_level = setup->Workout_level;
		return CHECKSUM_OK;
	}else{
		return CHECKSUM_FAIL;
	}
}


#define segment_break 27

unsigned char CFpcApp::PackUpdateDataAndSend(struct M2M_Message *theMsg, int fd)
{
	struct RealtimeUpdateDataB *bp;
	unsigned char i;
	unsigned char *update_data;
	unsigned short us = 0, total_data_len;
	unsigned char local_checksum;
	unsigned char out[1024 * 4];
	struct M2M_aHeader *aHeader = 0;


	total_data_len = 	12		// header bytes except the data pointer
				+ (47+2) * 4	// "xxx," 4 byte	//added 2 unsigned short @2013/01/24
				+ 3;// checksum	

	if(theMsg->state.ascii_type == 1)
	{
		aHeader = (struct M2M_aHeader *)&theMsg->header;

		memcpy((void *)aHeader->head, (const void *)"555,", 4);
		memcpy((void *)aHeader->function, (const void *)"002,", 4);
		sprintf((char *)aHeader->len, "%03d,", total_data_len - 15);
		bp = (struct RealtimeUpdateDataB *)update;

		update_data = theMsg->header.data;

		local_checksum = 0;
		for(i = 0;i < (47 + 2); i++)
		{
			if(i < segment_break){
				us = bp->data[i];
			}else{
				us = bp->data1[i - segment_break];
			}
			if(us > 999)
				us = 999;
			local_checksum += us;
			sprintf((char *)update_data, "%03d,", us);
			update_data += 4;
		}
		sprintf((char *)update_data,"%03d",(local_checksum & 0x00ff));
	}
	else
	{

	}

	packOurProtocol(CMD_GUI | (1 << 6), 0, total_data_len, (unsigned char *)&theMsg->header, out);
	if (write(fd, (const void*)out, total_data_len + 8) <= 0) { return 1; }

	return 0;
}

unsigned char CFpcApp::ChecksumCheck(struct M2M_Message *theMsg)
{
	unsigned char i,*ucp;
	unsigned short us,local_checksum,remote_checksum;

	ucp = theMsg->header.data;
	local_checksum = 0;
	remote_checksum = 0;

	for(i=0;i<(theMsg->header.len);i++){
		us = (ucp[i*4]-'0')*100+(ucp[i*4+1]-'0')*10+ucp[i*4+2]-'0';
		if(i == 0){
			if(theMsg->header.len >= 1){
				theMsg->item_one_data = us;
			}
		}
		if(i == 1){
			if(theMsg->header.len >= 2){
				theMsg->item_two_data = us;
			}
		}
		local_checksum += us;
	}
	
	us = (ucp[i*4]-'0')*100+(ucp[i*4+1]-'0')*10+ucp[i*4+2]-'0';
	remote_checksum = us;

	local_checksum &= 0x00FF;
	remote_checksum &= 0x00FF;
	
	if(local_checksum==remote_checksum){
		return CHECKSUM_OK;
	}

	return CHECKSUM_FAIL;	
}

unsigned char CFpcApp::PackSummaryDataAndSend(struct M2M_Message *theMsg, int fd)
{
	struct WorkoutSummaryB *bp;
	unsigned char i;//, exe_result;
	unsigned char *update_data;
	unsigned short us,total_data_len;
	unsigned char local_checksum;//,data;
	unsigned char out[1024 * 4];

	struct M2M_aHeader *aHeader = 0;

// 	switch(rt.summary_state){
// 	case GOT_NO_WORKOUT_SUMMARY:
// 		CollectWorkOutSummaryData();
// 		CollectTotalSummaryData();
// 		CalculateSummaryData();
// 		break;
// 	case GOT_WORKOUT_SUMMARY:
// 		CollectTotalSummaryData();
// 		CalculateSummaryData();
// 		break;
// 	case GOT_TOTAL_SUMMARY:
// 		CalculateSummaryData();
// 		break;
// 	}

//printf("(%s %d) PackSummaryDataAndSend(), rt->summary_state=%d\n",  __FILE__, __LINE__, rt->summary_state);


	//if (rt->summary_state.did_warmup == 1)		{}
	//else										{ CollectWorkOutSummaryData();}
	CalculateSummaryData();


	// header bytes except the data pointer
	// "xxx," 4 byte
	// checksum
	total_data_len = 12 + (4*4 + 6*4 + 1)*4 +3;

	if(theMsg->state.ascii_type==1)
	{
		aHeader = (struct M2M_aHeader *)&theMsg->header;

		memcpy((void *)aHeader->head, (const void *)"555,", 4);
		memcpy((void *)aHeader->function, (const void *)"014,", 4);
		sprintf((char *)aHeader->len, "%03d,", total_data_len - 15);
		bp = (struct WorkoutSummaryB *)summary;

		update_data = theMsg->header.data;
		local_checksum=0;
		for(i = 0; i < 41; i++)
		{
			us = bp->data[i];
			if(us>999)	us = 999;
			local_checksum += us;
			sprintf((char *)update_data, "%03d,", us);
			update_data += 4;
		}
		local_checksum &= 0x00ff;
		sprintf((char *)update_data, "%03d", local_checksum);
	}
	else
	{
	}


	packOurProtocol(CMD_GUI | (1 << 6), 0, total_data_len, (unsigned char *)&theMsg->header, out);
	if (write(fd, (const void*)out, total_data_len + 8) <= 0) { return 1; }

	return 0;
}


void CFpcApp::ReplyTime(struct M2M_Message *theMsg, int fd)
{
	unsigned char i,*ucp;
	unsigned short check_sum, update_time[10];
	unsigned char buff[1024];
	unsigned char out[1024 * 4];

	update_time[0] = update->Segment_time_1000;
	update_time[1] = update->Segment_time;
	update_time[2] = update->Time_elapsed_1000;	
	update_time[3] = update->Time_elapsed;
	update_time[4] = update->Time_remaining_1000;
	update_time[5] = update->Time_remaining;
	update_time[6] = rt->segment_index;
	update_time[7] = rt->workout_state;
	check_sum = 0;

	sprintf((char *)buff, "%s,", reply16);
	ucp = buff;

	for(i = 0; i <= 7; i++)
	{
		if(update_time[i] > 999)update_time[i] = 999;
		check_sum += update_time[i];
		sprintf((char *)(ucp + (i + 3) * 4), "%03d,", update_time[i]);
	}
	check_sum &= 0x00ff;
	sprintf((char *)(ucp+(i+3)*4),"%03d", check_sum);

	packOurProtocol(CMD_GUI | (1 << 6), 0, 47, (unsigned char *)buff, out);
	if (write(fd, (const void*)out, 47 + 8) <= 0) {}
}



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
#define FC_GET_MCU_VERSION					0X9B




unsigned char CFpcApp::ProcessUserMessageB(unsigned char *buff, int fd)
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


//if (F != 149)	printf("(%s %d) FUNCTION:%d\n", __FILE__, __LINE__, F);
	if(FC_WORK_MODE_CHANGE == F)
	{
		struct work_mode_state_t state;

		// 0,777,777,4,data
		state.type = buff[3];
		state.target = BUILD_UINT32(buff[7], buff[6], buff[5], buff[4]);
		state.source = BUILD_UINT32(buff[11], buff[10], buff[9], buff[8]);

		state.key = KS_DELETE;
		if (0 == buff[12])				state.key = KS_LEFT_TOP;

// KS_LEFT_BOTTOM, KS_LEFT_CENTER
		else if (1 == buff[12])			state.key = KS_LEFT_CENTER;
// KS_LEFT_BOTTOM, KS_LEFT_CENTER
		else if (2 == buff[12])			state.key = KS_LEFT_BOTTOM;
		else if (3 == buff[12])			state.key = KS_RIGHT_TOP;
		else if (4 == buff[12])			state.key = KS_RIGHT_CENTER;
		else if (5 == buff[12])			state.key = KS_RIGHT_BOTTOM;
		else if (6 == buff[12])			state.key = KS_START;
		else if (7 == buff[12])			state.key = KS_STOP;
		else if (8 == buff[12])			state.key = KS_WORKLOAD_UP;
		else if (9 == buff[12])			state.key = KS_WORKLOAD_DOWN;
		else if (10 == buff[12])		state.key = KS_STRIDE_UP;
		else if (11 == buff[12])		state.key = KS_STRIDE_DOWN;
		else if (12 == buff[12])		state.key = KS_NUM0;
		else if (13 == buff[12])		state.key = KS_NUM1;
		else if (14 == buff[12])		state.key = KS_NUM2;
		else if (15 == buff[12])		state.key = KS_NUM3;
		else if (16 == buff[12])		state.key = KS_NUM4;
		else if (17 == buff[12])		state.key = KS_NUM5;
		else if (18 == buff[12])		state.key = KS_NUM6;
		else if (19 == buff[12])		state.key = KS_NUM7;
		else if (20 == buff[12])		state.key = KS_NUM8;
		else if (21 == buff[12])		state.key = KS_NUM9;
		else if (22 == buff[12])		state.key = KS_DELETE;
		//else							state.key = KS_DELETE;
		state.shift = buff[13];
//printf("(%s %d) FC_WORK_MODE_CHANGE, type=%d, target=%u, source=%u, key=%d, shift=%d\n", __FILE__, __LINE__, state.type, state.target, state.source, state.key, state.shift);
		if (0 == buff[3])
		{
			OnWorkModeChange(&state);
		}
		else //if (1 == buff[3])
		{

			OnDataScreenChange(&state);
		}

		reply[0] = H; reply[1] = F; L = 0; reply[2] = L;
		packOurProtocol(CMD_GUI | (1 << 6), 0, L + 3, (unsigned char *)reply, out);
		if (write(fd, (const void *)out, L + 3 + 8) <= 0){}
		return 0;
	}



#if 0
	if(FC_SET_WORK_PAGE == F)
	{
//////////////////
// NG
		reply[0] = H; reply[1] = F; L = 0; reply[2] = L;
		packOurProtocol(CMD_GUI | (1 << 6), 0, L + 3, (unsigned char *)reply, out);
		if (write(fd, (const void *)out, L + 3 + 8) <= 0){}
		return 0;


/*
		old_Work_mode = setup->Work_mode;
		setup->Work_mode = BUILD_UINT32(buff[3], buff[4], buff[5], buff[6]);
		key = buff[11];
		if (old_Work_mode != setup->Work_mode)
		{
		}
		OnWorkPageChanged(setup->Work_mode, old_Work_mode, key, 0);
*/

		reply[0] = H; reply[1] = F; L = 0; reply[2] = L;
		packOurProtocol(CMD_GUI | (1 << 6), 0, L + 3, (unsigned char *)reply, out);
		if (write(fd, (const void *)out, L + 3 + 8) <= 0){}
		return 0;
	}

	if(FC_GET_WORK_PAGE == F)
	{
		reply[0] = H; reply[1] = F; L = 4; reply[2] = L; 
		reply[3] = setup->Work_mode & 0X000000FF;
		reply[4] = setup->Work_mode & 0X0000FF00;
		reply[5] = setup->Work_mode & 0X00FF0000;
		reply[6] = setup->Work_mode & 0XFF000000;
		packOurProtocol(CMD_GUI | (1 << 6), 0, L + 3, (unsigned char *)reply, out);
		if (write(fd, (const void *)out, L + 3 + 8) <= 0){}

		return 0;
	}

	if(FC_SET_DS_BTN_TYPE == F)
	{
//////////////////
// NG
		reply[0] = H; reply[1] = F; L = 0; reply[2] = L;
		packOurProtocol(CMD_GUI | (1 << 6), 0, L + 3, (unsigned char *)reply, out);
		if (write(fd, (const void *)out, L + 3 + 8) <= 0){}
		return 0;


		old_ds_btn_type = ds_btn_type;
		ds_btn_type = buff[3];
		key = buff[5];
		if (old_ds_btn_type != ds_btn_type)
		{
		}
		OnDataScreenChanged(buff[3], old_ds_btn_type, key, 0);

		reply[0] = H; reply[1] = F; L = 0; reply[2] = L;
		packOurProtocol(CMD_GUI | (1 << 6), 0, L + 3, (unsigned char *)reply, out);
		if (write(fd, (const void *)out, L + 3 + 8) <= 0){}

		return 0;
	}

	if(FC_GET_DS_BTN_TYPE == F)
	{
		reply[0] = H; reply[1] = F; L = 1; reply[2] = L; reply[3] = ds_btn_type;
		packOurProtocol(CMD_GUI | (1 << 6), 0, L + 3, (unsigned char *)reply, out);
		if (write(fd, (const void *)out, L + 3 + 8) <= 0){}

		return 0;
	}
#endif




	if(FC_SET_WORKLOAD == F)
	{
		rt->Target_Workload_level = buff[3];
		update->Workload_level = rt->Target_Workload_level;
		exception->cmd.work_load_up = 1;

		reply[0] = H; reply[1] = F; L = 1, reply[2] = L; reply[3] = 0;
		packOurProtocol(CMD_GUI | (1 << 6), 0, L + 3, (unsigned char *)reply, out);
		if (write(fd, (const void *)out, L + 3 + 8) <= 0){}
		return 0;
	}
	if(FC_START == F)
	{
		if(
			WP_MANUAL_QUICK_START_RUN != setup->Work_mode
			&& WP_CARDIO_360_QUICK_START_RUN != setup->Work_mode
			&& WP_C360_QUICK_START_RUN != setup->Work_mode
			&& WP_C360_ARM_SCULPTOR_RUN != setup->Work_mode
			&& WP_C360_LEG_SHAPER_RUN != setup->Work_mode
			&& WP_C360_CUSTOM_RUN != setup->Work_mode
			&& WP_CALORIE_GOAL_RUN != setup->Work_mode
			&& WP_GLUTE_BUSTER_RUN	 != setup->Work_mode
			&& WP_LEG_SHAPER_RUN != setup->Work_mode
			&& WP_WL_RUN != setup->Work_mode
			&& WP_TARGET_HRC_RUN != setup->Work_mode
			&& WP_WL_HRC_RUN != setup->Work_mode
			&& WP_AEROBIC_HRC_RUN != setup->Work_mode
			&& WP_INTERVAL_HRC_RUN != setup->Work_mode
			&& WP_CARDIO_CHALLENGE_RUN != setup->Work_mode
			&& WP_WALK_INTERVALS_RUN != setup->Work_mode
			&& WP_PACE_INTERVALS_RUN != setup->Work_mode
			&& WP_PACE_RAMP_RUN != setup->Work_mode
			&& WP_ROLLING_HILLS_RUN	 != setup->Work_mode
			&& WP_HILL_INTVALS_RUN != setup->Work_mode
			&& WP_SINGLE_HILL_RUN != setup->Work_mode
			&& WP_RANDOM_HILLS_RUN	 != setup->Work_mode
			&& WP_DISTANCE_RUN != setup->Work_mode
			&& WP_FITNESS_RUN != setup->Work_mode
			&& WP_CUSTOM_UTRA_RUN != setup->Work_mode
		)
		{

			return 0;
		}
		exception->cmd.start = 1;
		exception->cmd.stop = 0;
		exception->cmd.pause = 0;
		TellFpcStart();

		reply[0] = H; reply[1] = F; L = 1, reply[2] = L; reply[3] = 0;
		packOurProtocol(CMD_GUI | (1 << 6), 0, L + 3, (unsigned char *)reply, out);
		if (write(fd, (const void *)out, L + 3 + 8) <= 0){}
		return 0;
	}

	if(FC_STOP == F)
	{
		exception->cmd.pause = 0;
		exception->cmd.stop = 1;
		rt->workout_state = READY_TO_SUMMARY;

		reply[0] = H; reply[1] = F; L = 1, reply[2] = L; reply[3] = 0;
		packOurProtocol(CMD_GUI | (1 << 6), 0, L + 3, (unsigned char *)reply, out);
		if (write(fd, (const void *)out, L + 3 + 8) <= 0){}
		return 0;
	}


	if(FC_PAUSE == F)
	{
		exception->cmd.pause = 1;

		reply[0] = H; reply[1] = F; L = 1, reply[2] = L; reply[3] = 0;
		packOurProtocol(CMD_GUI | (1 << 6), 0, L + 3, (unsigned char *)reply, out);
		if (write(fd, (const void *)out, L + 3 + 8) <= 0){}
		return 0;
	}

	if(FC_RESUME == F)
	{
		exception->cmd.resume = 1;
		exception->cmd.pause = 0;

		reply[0] = H; reply[1] = F; L = 1, reply[2] = L; reply[3] = 0;
		packOurProtocol(CMD_GUI | (1 << 6), 0, L + 3, (unsigned char *)reply, out);
		if (write(fd, (const void *)out, L + 3 + 8) <= 0){}
		return 0;
	}
    // chuck modify 20140207 

	if(FC_GET_MCU_VERSION == F)
	{

	  reply[0] = H;
	  reply[1] = F; 
	  L = 4; 
	  reply[2] = L; 
	  reply[3] = McuVersion[0];
	  reply[4] = McuVersion[1];
	  reply[5] = McuVersion[2];
	  reply[6] = McuVersion[3];
	  packOurProtocol(CMD_GUI | (1 << 6), 0, L + 3, (unsigned char *)reply, out);
	  if (write(fd, (const void *)out, L + 3 + 8) <= 0){}

	  return 0;
	}
	if(FC_SET_AUDIO_CH == F)
	{
		//i2c_read_data(i2c0, PCA_9555_SLAVE_1, PCA_9555_INPUT_0, &c, 1);
		//if (0 == buff[3])					c &= ~BIT7;
		//else								c |= BIT7;
		//i2c_write_data(i2c0, PCA_9555_SLAVE_1, PCA_9555_OUTPUT_0, &c, 1);
		//rt->audio.audio_source = buff[3];

		reply[0] = H; reply[1] = F; L = 1, reply[2] = L; reply[3] = 0;
		packOurProtocol(CMD_GUI | (1 << 6), 0, L + 3, (unsigned char *)reply, out);
		if (write(fd, (const void *)out, L + 3 + 8) <= 0){}
		return 0;
	}

	if(FC_CHECK_IPOD_DOCK == F)
	{
		//i2c_read_data(i2c0, PCA_9555_SLAVE_1, PCA_9555_INPUT_0, &c, 1);
		//if (c & BIT4)				rt->ipod_in_duck = 1;
		//else						rt->ipod_in_duck = 0;

		reply[0] = H; reply[1] = F; L = 1, reply[2] = L; reply[3] = rt->ipod_in_duck;
		packOurProtocol(CMD_GUI | (1 << 6), 0, L + 3, (unsigned char *)reply, out);
		if (write(fd, (const void *)out, L + 3 + 8) <= 0){}
		return 0;
	}

	if (FC_COOL_DOWN == F)
	{
		exception->cmd.cool_down = 1;
		printf("(%s %d) FC_COOL_DOWN\n", __FILE__, __LINE__);

		reply[0] = H; reply[1] = F; L = 1, reply[2] = L; reply[3] = 0;
		packOurProtocol(CMD_GUI | (1 << 6), 0, L + 3, (unsigned char *)reply, out);
		if (write(fd, (const void *)out, L + 3 + 8) <= 0){}
		return 0;
	}

	if (FC_SET_LOCALTIME == F)
	{
		// system/bin/date -s 200703 25.123456
		struct tm lassie;
		char tmp[64];
		time_t new_time;
		struct timeval garfield;

		memset(tmp, 0, sizeof(tmp));
		tmp[0] = buff[3]; tmp[1] = buff[4]; tmp[2] = buff[5]; tmp[3] = buff[6];
		lassie.tm_year = atoi(tmp) - 1900;

		memset(tmp, 0, sizeof(tmp));
		tmp[0] = buff[7]; tmp[1] = buff[8];
		lassie.tm_mon = atoi(tmp);
		if (atoi(tmp) > 0)			lassie.tm_mon -= 1;
		else if (atoi(tmp) > 11)		lassie.tm_mon = 11;
		else						lassie.tm_mon = 0;

		memset(tmp, 0, sizeof(tmp));
		tmp[0] = buff[9]; tmp[1] = buff[10];
		lassie.tm_mday = atoi(tmp);
		if (atoi(tmp) == 0)			lassie.tm_mday = 1;
		else if (atoi(tmp) > 31)		lassie.tm_mday = 31;

		memset(tmp, 0, sizeof(tmp));
		tmp[0] = buff[12]; tmp[1] = buff[13];
		lassie.tm_hour = atoi(tmp);
		if (atoi(tmp) > 23)			lassie.tm_hour = 23;

		memset(tmp, 0, sizeof(tmp));
		tmp[0] = buff[14]; tmp[1] = buff[15];
		lassie.tm_min = atoi(tmp);
		if (atoi(tmp) > 59)			lassie.tm_min = 59;

		memset(tmp, 0, sizeof(tmp));
		tmp[0] = buff[16]; tmp[1] = buff[17];
		lassie.tm_sec = atoi(tmp);
		if (atoi(tmp) > 59)			lassie.tm_sec = 59;

		new_time = mktime(&lassie);
		//new_time -= 28800;
		  new_time += 3600;	
		//localtime(&t1)

		garfield.tv_sec = new_time;
		garfield.tv_usec = 0;

#if 0
		char sz[128];

		// int settimeofday(const struct timeval *tv, const struct timezone *tz);
		memset(sz, 0, sizeof(sz));
		sprintf(sz, "/system/bin/date -s %s", (char *)(&buff[3]));
		if (0 == system(sz)) {}
#endif

		reply[0] = H; reply[1] = F; L = 1, reply[2] = L; reply[3] = 0;
		packOurProtocol(CMD_GUI | (1 << 6), 0, L + 3, (unsigned char *)reply, out);
		if (write(fd, (const void *)out, L + 3 + 8) <= 0){}

		if (0 != settimeofday(&garfield, 0))
			printf("(%s %d) settimeofday(%04d-%02d-%02d %02d-%02d-%02d), FAIL\n", __FILE__, __LINE__, lassie.tm_year + 1900, lassie.tm_mon + 1, lassie.tm_mday, lassie.tm_hour, lassie.tm_min, lassie.tm_sec);
		else
		{
			int rtc_fd = -1;
			struct rtc_time rtc_tm;

			rtc_fd = open("/dev/rtc0", O_RDWR);
			if (-1 != rtc_fd)
			{
				struct tm *local;

				local = localtime(&new_time);

				rtc_tm.tm_mday = local->tm_mday;
				rtc_tm.tm_mon = local->tm_mon;
				rtc_tm.tm_year = local->tm_year;//104;
				rtc_tm.tm_hour = local->tm_hour;
				rtc_tm.tm_min = local->tm_min;
				rtc_tm.tm_sec = local->tm_sec;

				ioctl(rtc_fd, RTC_SET_TIME, &rtc_tm);
				close(rtc_fd);
			}

			printf("(%s %d) [%04d-%02d-%02d %02d-%02d-%02d]\n", __FILE__, __LINE__, lassie.tm_year + 1900, lassie.tm_mon + 1, lassie.tm_mday, lassie.tm_hour, lassie.tm_min, lassie.tm_sec);
			sync(); sync(); sync();
			sync(); sync(); sync();
			usleep(1000);
			reboot(RB_AUTOBOOT);
		}

		return 0;
	}




///////////////////////////////////////////////
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
		/*static int init = 0;
		if (0 == init)
		{
			init = 1;

			if (0 == product_type || 1 == product_type)
			{
				if (prob_pca955_1)
					SetTimer(FPC_SCHEDULER_1MS_ID, 1, HrTimer);
			}
			else if (2 == product_type)
			{
				SetTimer(FPC_SCHEDULER_1MS_ID, 1, HrTimer);
			}

			SetTimer(UDISK_DETECT_TIMER_ID, 2003, UdiskDetectTimer);
		}*/

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
		for (c = 0; c < SEGMENTS; c++)
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

//////////////////////////
		s = setup->Weight;
#if 1
		s = (unsigned short)round((double)setup->weight * (double)2.2F);
#endif
		reply[5] = HI_UINT16(s); reply[6] = LO_UINT16(s);


		s = setup->Workout_time_1000 * 1000 + setup->Workout_time; reply[7] = HI_UINT16(s); reply[8] = LO_UINT16(s);


/////////////////////////////////////////////
///////////////////////
		s = (unsigned short)((unsigned int)setup->Workout_distance / (unsigned int)10);

		reply[9] = HI_UINT16(s); reply[10] = LO_UINT16(s);

		s = setup->Workload_level; reply[11] = HI_UINT16(s); reply[12] = LO_UINT16(s);
		s = setup->Pace_level; reply[13] = HI_UINT16(s); reply[14] = LO_UINT16(s);
		s = setup->Calorie_goal; reply[15] = HI_UINT16(s); reply[16] = LO_UINT16(s);
		s = setup->Target_heart_rate; reply[17] = HI_UINT16(s); reply[18] = LO_UINT16(s);
		s = setup->Work_heart_rate; reply[19] = HI_UINT16(s); reply[20] = LO_UINT16(s);
		s = setup->Gender; reply[21] = HI_UINT16(s); reply[22] = LO_UINT16(s);
		s = setup->Segments; reply[23] = HI_UINT16(s); reply[24] = LO_UINT16(s);
		for (c = 0; c < SEGMENTS; c++)
		{
			s = setup->Segments_time[c]; reply[25 + c * 2] = HI_UINT16(s); reply[25 + 1 + c * 2] = LO_UINT16(s);
		}
		for (c = 0; c < 30; c++)
		{
			s = setup->Workload[c]; reply[45 + c * 2] = HI_UINT16(s); reply[45 + 1 + c * 2] = LO_UINT16(s);
		}
		for (c = 0; c < 30; c++)
		{
			s = setup->Pace[c]; reply[105 + c * 2] = HI_UINT16(s); reply[105 + 1 + c * 2] = LO_UINT16(s);
		}

//////////////////////////////
		//s = setup->Segments; reply[165] = HI_UINT16(s); reply[166] = LO_UINT16(s);


		reply[0] = H; reply[1] = F; L = 162, reply[2] = L;;
		//reply[0] = H; reply[1] = F; L = 162 + 2, reply[2] = L;;
		packOurProtocol(CMD_GUI | (1 << 6), 0, L + 3, (unsigned char *)reply, out);
		if (write(fd, (const void *)out, L + 3 + 8) <= 0){}
		return 0;
	}

	if(FC_GET_UPDATE == F)
	{
		s = rt->audio.audio_source; reply[3] = HI_UINT16(s); reply[4] = LO_UINT16(s);
		s = update->Time_elapsed_1000 * 1000 + update->Time_elapsed; reply[5] = HI_UINT16(s); reply[6] = LO_UINT16(s);
		s = update->Time_remaining_1000 * 1000 + update->Time_remaining; reply[7] = HI_UINT16(s); reply[8] = LO_UINT16(s);

///////////
		//s = update->Distance_km_i * 1000 + update->Distance_km_f; reply[9] = HI_UINT16(s); reply[10] = LO_UINT16(s);
		s = update->Distance_km_f; reply[9] = HI_UINT16(s); reply[10] = LO_UINT16(s);

		s = update->Distance_remaining_km_i * 1000 + update->Distance_remaining_km_f; reply[11] = HI_UINT16(s); reply[12] = LO_UINT16(s);
		s = update->Heart_rate; reply[13] = HI_UINT16(s); reply[14] = LO_UINT16(s);
		s = update->Target_heart_rate; reply[15] = HI_UINT16(s); reply[16] = LO_UINT16(s);
		s = update->Calories_burned_1000cal * 1000 + update->Calories_burned; reply[17] = HI_UINT16(s); reply[18] = LO_UINT16(s);
		s = update->Calories_per_hour_1000cal * 1000 + update->Calories_per_hour; reply[19] = HI_UINT16(s); reply[20] = LO_UINT16(s);
		s = update->Watts; reply[21] = HI_UINT16(s); reply[22] = LO_UINT16(s);
		s = update->Mets; reply[23] = HI_UINT16(s); reply[24] = LO_UINT16(s);
		s = update->Workload_level; reply[25] = HI_UINT16(s); reply[26] = LO_UINT16(s);

		s = update->Pace_RPM;
		reply[27] = HI_UINT16(s); reply[28] = LO_UINT16(s);

// jason note for test, RPM
		//s = update->Pace_RPM = 60; reply[27] = HI_UINT16(s); reply[28] = LO_UINT16(s);

		s = update->Pace_level; reply[29] = HI_UINT16(s); reply[30] = LO_UINT16(s);

		s = update->Stride_length; reply[31] = HI_UINT16(s); reply[32] = LO_UINT16(s);
		s = update->Segment_time_1000 * 1000 + update->Segment_time; reply[33] = HI_UINT16(s); reply[34] = LO_UINT16(s);
		s = update->workload_index; reply[35] = HI_UINT16(s); reply[36] = LO_UINT16(s);
		for (c = 0; c < GUI_window_size; c++)
		{
			s = (unsigned short)update->Workload_bar[c]; reply[37 + 2 * c] = HI_UINT16(s); reply[38 + 2 * c] = LO_UINT16(s);
		}
		for (c = 0; c < GUI_window_size; c++)
		{
			s = (unsigned short)update->Pace_bar[c]; reply[61 + 2 * c] = HI_UINT16(s); reply[62 + 2 * c] = LO_UINT16(s);
		}
		s = (unsigned short)update->Sports_mode; reply[85] = HI_UINT16(s); reply[86] = LO_UINT16(s);


///////////
		s = (unsigned short)setup->Weight;
		s = (unsigned short)round((double)setup->weight * (double)2.2F);
		reply[87] = HI_UINT16(s); reply[88] = LO_UINT16(s);


///////////
		s = rt->segment_index; reply[89] = HI_UINT16(s); reply[90] = LO_UINT16(s);
		s = (unsigned short)rt->Average_Rpm; reply[91] = HI_UINT16(s); reply[92] = LO_UINT16(s);

///////////
		s = (unsigned short)IPOD_Detect; reply[93] = HI_UINT16(s); reply[94] = LO_UINT16(s);

///////////
		s = update->Distance_km_i; reply[95] = HI_UINT16(s); reply[96] = LO_UINT16(s);

///////////
		s = (unsigned short)(mph * 1.609F * 1000.00F * 100.00F / 3600.00F); reply[97] = HI_UINT16(s); reply[98] = LO_UINT16(s);	// 96 = 98 - 2

		reply[0] = H; reply[1] = F; L = 96, reply[2] = L;

		packOurProtocol(CMD_GUI | (1 << 6), 0, L + 3, (unsigned char *)reply, out);
		if (write(fd, (const void *)out, L + 3 + 8) <= 0){}
		return 0;
	}
	if(FC_GET_SUMMARY == F)
	{
		s = summary->Warmup_time_elapsed_1000 * 1000 + summary->Warmup_time_elapsed; reply[3] = HI_UINT16(s); reply[4] = LO_UINT16(s);
		s = summary->Warmup_avg_heart_rate; reply[5] = HI_UINT16(s); reply[6] = LO_UINT16(s);
		s = summary->Warmup_max_heart_rate; reply[7] = HI_UINT16(s); reply[8] = LO_UINT16(s);
		s = summary->Warmup_average_pace; reply[9] = HI_UINT16(s); reply[10] = LO_UINT16(s);
		s = summary->Warmup_max_pace; reply[11] = HI_UINT16(s); reply[12] = LO_UINT16(s);

		s = summary->Warmup_distance_km_f; reply[13] = HI_UINT16(s); reply[14] = LO_UINT16(s);
		s = summary->Warmup_calories_burned_1000 * 1000 + summary->Warmup_calories_burned; reply[15] = HI_UINT16(s); reply[16] = LO_UINT16(s);
		s = summary->Workout_time_elapsed_1000 * 1000 + summary->Workout_time_elapsed; reply[17] = HI_UINT16(s); reply[18] = LO_UINT16(s);
		s = summary->Workout_avg_heart_rate; reply[19] = HI_UINT16(s); reply[20] = LO_UINT16(s);
		s = summary->Workout_max_heart_rate; reply[21] = HI_UINT16(s); reply[22] = LO_UINT16(s);
		s = summary->Workout_average_pace; reply[23] = HI_UINT16(s); reply[24] = LO_UINT16(s);
		s = summary->Workout_max_pace; reply[25] = HI_UINT16(s); reply[26] = LO_UINT16(s);
		s = summary->Workout_distance_km_f; reply[27] = HI_UINT16(s); reply[28] = LO_UINT16(s);
		s = summary->Workout_calories_burned_1000 * 1000 + summary->Workout_calories_burned; reply[29] = HI_UINT16(s); reply[30] = LO_UINT16(s);
		s = summary->Cooldown_time_elapsed_1000 * 1000 + summary->Cooldown_time_elapsed; reply[31] = HI_UINT16(s); reply[32] = LO_UINT16(s);
		s = summary->Cooldown_avg_heart_rate; reply[33] = HI_UINT16(s); reply[34] = LO_UINT16(s);
		s = summary->Cooldown_max_heart_rate; reply[35] = HI_UINT16(s); reply[36] = LO_UINT16(s);
		s = summary->Cooldown_average_pace; reply[37] = HI_UINT16(s); reply[38] = LO_UINT16(s);
		s = summary->Cooldown_max_pace; reply[39] = HI_UINT16(s); reply[40] = LO_UINT16(s);
		s = summary->Cooldown_distance_km_f; reply[41] = HI_UINT16(s); reply[42] = LO_UINT16(s);
		s = summary->Cooldown_calories_burned_1000 * 1000 + summary->Cooldown_calories_burned; reply[43] = HI_UINT16(s); reply[44] = LO_UINT16(s);
		s = summary->Total_time_elapsed_1000 * 1000 + summary->Total_time_elapsed; reply[45] = HI_UINT16(s); reply[46] = LO_UINT16(s);
		s = summary->Total_avg_heart_rate; reply[47] = HI_UINT16(s); reply[48] = LO_UINT16(s);
		s = summary->Total_max_heart_rate; reply[49] = HI_UINT16(s); reply[50] = LO_UINT16(s);
		s = summary->Total_average_pace; reply[51] = HI_UINT16(s); reply[52] = LO_UINT16(s);
		s = summary->Total_max_pace; reply[53] = HI_UINT16(s); reply[54] = LO_UINT16(s);
		s = summary->Total_distance_km_f; reply[55] = HI_UINT16(s); reply[56] = LO_UINT16(s);
		s = summary->Total_calories_burned_1000 * 1000 + summary->Total_calories_burned; reply[57] = HI_UINT16(s); reply[58] = LO_UINT16(s);
		s = summary->Vo2; reply[59] = HI_UINT16(s); reply[60] = LO_UINT16(s);

////////////////////
		s = (unsigned short)udisk_detect; reply[61] = HI_UINT16(s); reply[62] = LO_UINT16(s);

////////////////////
		s = summary->Warmup_distance_km_i; reply[63] = HI_UINT16(s); reply[64] = LO_UINT16(s);
		s = summary->Workout_distance_km_i; reply[65] = HI_UINT16(s); reply[66] = LO_UINT16(s);
		s = summary->Cooldown_distance_km_i; reply[67] = HI_UINT16(s); reply[68] = LO_UINT16(s);
		s = summary->Total_distance_km_i; reply[69] = HI_UINT16(s); reply[70] = LO_UINT16(s);

		reply[0] = H; reply[1] = F; L = 58 + 2 + 8, reply[2] = L;
		packOurProtocol(CMD_GUI | (1 << 6), 0, L + 3, (unsigned char *)reply, out);
		if (write(fd, (const void *)out, L + 3 + 8) <= 0){}
		return 0;
	}
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
		c |= (BIT6);
		i2c_write_data(i2c0, PCA_9555_SLAVE_1, PCA_9555_OUTPUT_0, &c, 1);
		SetTimer(PWM1_TIMER_ID, 100, 0);
		reply[0] = H; reply[1] = F; L = 0, reply[2] = L;
		packOurProtocol(CMD_GUI | (1 << 6), 0, L + 3, (unsigned char *)reply, out);
		if (write(fd, (const void *)out, L + 3 + 8) <= 0){}
		return 0;
	}
	if(FC_BUTTON == F)
	{
////////////////////////////////////
		reply[0] = H; reply[1] = F; L = 24 + 8, reply[2] = L;
	/*	memcpy(&reply[3 + 0], &key_scan[1], 6);		// S1~6
		reply[3 + 6] = key_scan[9];				// S9
		reply[3 + 7] = key_scan[21];
		reply[3 + 8] = key_scan[7];
		reply[3 + 9] = key_scan[8];
		reply[3 + 10] = key_scan[22];
		reply[3 + 11] = key_scan[23];
		memcpy(&reply[3 + 12], &key_scan[10], 11);    

		reply[3 + 23] = key_scan[24];
		printf("key_scan[24] = %d \n",key_scan[24]);
		if (0 != key_scan[24])
			key_scan[24] = 0;
	*/


		memcpy(&reply[3 + 0], &Update_Key_Scan[0], 24); // chuck  modify

		reply[3 + 23] = key_scan[24];
	//	printf("key_scan[24] = %d \n",key_scan[24]);
		if (0 != key_scan[24])
			key_scan[24] = 0;
////////////////////////////////////
	/*	reply[3 + 24] = key_scan[25];
		reply[3 + 25] = key_scan[26];
		reply[3 + 26] = key_scan[27];
		reply[3 + 27] = key_scan[28];
		reply[3 + 28] = key_scan[29];
		reply[3 + 29] = key_scan[30];
		reply[3 + 30] = key_scan[31];
		reply[3 + 31] = key_scan[32]; */

		reply[3 + 24] = Update_Key_Scan[24];
		reply[3 + 25] = Update_Key_Scan[25];
		reply[3 + 26] = Update_Key_Scan[26];
		reply[3 + 27] = Update_Key_Scan[27];
		reply[3 + 28] = Update_Key_Scan[28];
		reply[3 + 29] = Update_Key_Scan[29];
		reply[3 + 30] = Update_Key_Scan[30];
		reply[3 + 31] = Update_Key_Scan[31];

		packOurProtocol(CMD_GUI | (1 << 6), 0, L + 3, (unsigned char *)reply, out);
		if (write(fd, (const void *)out, L + 3 + 8) <= 0){}

		if(reply[3 + 23]!= 0) printf("error code = %d    \n",reply[3 + 23]);
		return 0;
	}
	if (FC_CHECK_RUNTIME_WORKOUT_SECONDS == F)
	{
		s = update->Time_elapsed_1000 * 1000 + update->Time_elapsed;
		reply[3] = HI_UINT16(s); reply[4] = LO_UINT16(s);
//printf("Time_elapsed=%d, workout_state=%d\n", s, rt->workout_state);

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



// 001,340,401,035,255,255,001,255,255,255,255,255,150,005,940,255,255,060,060,015,015,045,030,060,030,045,060,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,091
unsigned char CFpcApp::ProcessUserMessage(struct M2M_Message *theMsg, int fd)
{
	unsigned exe_result;
	unsigned char out[1024 * 4];
	unsigned char reply[128];
	unsigned char *p;

	//unsigned char us;
	// setup->Work_mode


do {printf("ProcessUserMessage, theMsg->header.function=%u\n", theMsg->header.function);}while(0);



	switch(theMsg->header.function)
	{
	case 102:
		//p = theMsg->header.data;
		//setup->Age = BUILD_UINT16(p[1], p[2]);
		//setup->Weight = BUILD_UINT16(p[1], p[2]);

		return 0;

	case 101:
		if(1)
		{
			int Y, M, D, H, m, S;
			char sz[128];

			p = theMsg->header.data;
			Y = (p[1] - '0') * 10 + (p[2] - '0'); Y += 2000;
			M = (p[5] - '0') * 10 + (p[6] - '0');
			D = (p[9] - '0') * 10 + (p[10] - '0');
			H = (p[13] - '0') * 10 + (p[14] - '0');
			m = (p[17] - '0') * 10 + (p[18] - '0');
			S = (p[21] - '0') * 10 + (p[22] - '0');

			// /system/bin/date -s 20070325.123456
			memset(sz, 0, sizeof(sz));
			sprintf(sz, "/system/bin/date -s %04d%02d%02d.%02d%02d%02d", Y, M, D, H, m, S);
			system(sz);

			packOurProtocol(CMD_GUI | (1 << 6), 0, 15, (unsigned char *)reply101, out);
			if (write(fd, (const void *)out, 15 + 8) <= 0){}
		}
		return 0;

	// Button
	case 100:
		exe_result = ChecksumCheck(theMsg);
		if(exe_result == CHECKSUM_OK)
		{
			memset(reply, 0, sizeof(reply));
			reply[0] = 100; reply[1] = 24;
			memcpy(&reply[2 + 0], &key_scan[1], 6);
			reply[2 + 6] = key_scan[9];
			reply[2 + 7] = key_scan[21];
			reply[2 + 8] = key_scan[7];
			reply[2 + 9] = key_scan[8];
			reply[2 + 10] = key_scan[22];
			reply[2 + 11] = key_scan[23];
			memcpy(&reply[2 + 12], &key_scan[10], 11);
			packOurProtocol(CMD_GUI | (1 << 6), 0, 15, (unsigned char *)reply, out);
			if (write(fd, (const void *)out, 15 + 8) <= 0){}
		}
		return 0;

	case 0x00:
		exe_result = ChecksumCheck(theMsg);
		if(exe_result == CHECKSUM_OK)
		{
			packOurProtocol(CMD_GUI | (1 << 6), 0, 15, (unsigned char *)reply0, out);
			if (write(fd, (const void *)out, 15 + 8) <= 0){}
		}
		return 0;

	// WorkoutData_initialize
	case 0x01:
		WorkoutData_initialize();
		exe_result = UnPackMessageToBinary(theMsg);
		if(exe_result == CHECKSUM_OK)
		{
			packOurProtocol(CMD_GUI | (1 << 6), 0, 15, (unsigned char *)reply1, out);
			if (write(fd, (const void *)out, 15 + 8) <= 0){}
 		}
		break;

	//run state realtime update
	case 0x02:
		exe_result = ChecksumCheck(theMsg);
		if(exe_result == CHECKSUM_OK)
		{
			DataCollection();
			exe_result = PackUpdateDataAndSend(theMsg, fd);
 		}
		break;

	//workload up
	case 0x03:
		exe_result = ChecksumCheck(theMsg);
		if(exe_result == CHECKSUM_OK)
		{
			rt->Target_Workload_level = theMsg->item_one_data;
			update->Workload_level = rt->Target_Workload_level;
			exception->cmd.work_load_up = 1;

			packOurProtocol(CMD_GUI | (1 << 6), 0, 15, (unsigned char *)reply3, out);
			if (write(fd, (const void *)out, 15 + 8) <= 0){}
		}
		break;

	//pace up
	case 0x05:
		exe_result = ChecksumCheck(theMsg);
		if(exe_result == CHECKSUM_OK)
		{
			rt->Target_Pace_Level = theMsg->item_one_data;
			exception->cmd.pace_up = 1;

			packOurProtocol(CMD_GUI | (1 << 6), 0, 15, (unsigned char *)reply5, out);
			if (write(fd, (const void *)out, 15 + 8) <= 0){}
		}
		break;

	//stride_up
	case 0x07:
		exe_result = ChecksumCheck(theMsg);
		if(exe_result == CHECKSUM_OK)
		{
			rt->Target_Stride = theMsg->item_one_data;
			exception->cmd.stride_up = 1;

			packOurProtocol(CMD_GUI | (1 << 6), 0, 15, (unsigned char *)reply7, out);
			if (write(fd, (const void *)out, 15 + 8) <= 0){}
		}
		break;

	//HRC cruise(巡航)
	case 0x09:
		exe_result = ChecksumCheck(theMsg);
		if(exe_result == CHECKSUM_OK)
		{
			if(theMsg->item_one_data == 0)
			{
				FadOutCruiseGUI();
			}
			else
			{
				rt->target_cruise_heart_rate = theMsg->item_one_data;
				FadInCruiseGUI();
			}			

			packOurProtocol(CMD_GUI | (1 << 6), 0, 15, (unsigned char *)reply9, out);
			if (write(fd, (const void *)out, 15 + 8) <= 0){}

		}
		break;

	//Auto stride
	case 10:
		exe_result = ChecksumCheck(theMsg);
		if(exe_result == CHECKSUM_OK)
		{
			if(theMsg->item_one_data == 1)
			{			
				exception->cmd.auto_stride = 1;
			}
			else
			{
				exception->cmd.auto_stride = 0;
			}

			packOurProtocol(CMD_GUI | (1 << 6), 0, 15, (unsigned char *)reply10, out);
			if (write(fd, (const void *)out, 15 + 8) <= 0){}
		}
		break;

	//11 Cool down(A8)
	case 11:
		exe_result = ChecksumCheck(theMsg);
		if(exe_result == CHECKSUM_OK)
		{
			exception->cmd.cool_down = 1;

			packOurProtocol(CMD_GUI | (1 << 6), 0, 15, (unsigned char *)reply11, out);
			if (write(fd, (const void *)out, 15 + 8) <= 0){}
		}
		break;

	//12 Pause(A8)
	case 12:
		exe_result = ChecksumCheck(theMsg);
		if(exe_result == CHECKSUM_OK)
		{
			exception->cmd.pause = 1;

			packOurProtocol(CMD_GUI | (1 << 6), 0, 15, (unsigned char *)reply12, out);
			if (write(fd, (const void *)out, 15 + 8) <= 0){}

		}
		break;

	//13 Resume
	case 13:
		exe_result = ChecksumCheck(theMsg);
		if(exe_result == CHECKSUM_OK)
		{
			exception->cmd.resume = 1;
			exception->cmd.pause = 0;

			packOurProtocol(CMD_GUI | (1 << 6), 0, 15, (unsigned char *)reply13, out);
			if (write(fd, (const void *)out, 15 + 8) <= 0){}
		}
		break;

	//14 Summary Finish(A8)
	case 14:
		exe_result = ChecksumCheck(theMsg);
		if(exe_result == CHECKSUM_OK)
		{
			rt->workout_state = IN_NORMAL;
			exe_result = PackSummaryDataAndSend(theMsg, fd);
		}
		break;

	//Start(from GUI)
 	case 15:
		exe_result = ChecksumCheck(theMsg);
 		if(exe_result == CHECKSUM_OK)
		{
////////////////////////////////////////////////
 			exception->cmd.start = 1;
			TellFpcStart();

			packOurProtocol(CMD_GUI | (1 << 6), 0, 15, (unsigned char *)reply15, out);
			if (write(fd, (const void *)out, 15 + 8) <= 0){}

 		}
 		break;

	//reply time(from GUI)
	case 16:
		exe_result = ChecksumCheck(theMsg);
 		if(exe_result == CHECKSUM_OK)
		{
 			ReplyTime(theMsg, fd);
 		}
 		break;

	//stop(from GUI)
	case 17:
		exe_result = ChecksumCheck(theMsg);
 		if(exe_result == CHECKSUM_OK)
		{
			struct timeb tb;
			unsigned int now;

			//CollectTotalSummaryData();
			exception->cmd.pause = 0;
			exception->cmd.stop = 1;
			rt->workout_state = READY_TO_SUMMARY;

			ftime(&tb); now = tb.time * 1000 + tb.millitm;
			PostMessage(M_WAIT_FPC_END_OF_PROGRAM, (WPARAM)fd, (LPARAM)now);
			break;
 		}
 		break;

	//forward iPod key input, uart
 	case 18:
		exe_result = ChecksumCheck(theMsg);
		if(exe_result == CHECKSUM_OK)
		{

			rt->ipod_key = theMsg->item_one_data;

#if 0
			mach.os.AddIpodKey2Queue(theMsg->item_one_data);
			//ACP_write();
#endif // 0

			packOurProtocol(CMD_GUI | (1 << 6), 0, 15, (unsigned char *)reply18, out);
			if (write(fd, (const void *)out, 15 + 8) <= 0){}
		}
 		break;

	//iPod in duck query
 	case 19:
		exe_result = ChecksumCheck(theMsg);
 		if(exe_result == CHECKSUM_OK)
		{
			//unsigned char UC = 0;

			//if (-1 != i2c_read_data(i2c0, PCA_9555_SLAVE_1, PCA_9555_INPUT_0, &UC, 1))
			//{
			//	if (UC & BIT4)				rt->ipod_in_duck = 1;
			//	else						rt->ipod_in_duck = 0;
			//}

			if(rt->ipod_in_duck == 1)
			{
				packOurProtocol(CMD_GUI | (1 << 6), 0, 19, (unsigned char *)reply19_yes, out);
				if (write(fd, (const void *)out, 19 + 8) <= 0){}

			}
			else
			{
				packOurProtocol(CMD_GUI | (1 << 6), 0, 19, (unsigned char *)reply19_no, out);
				if (write(fd, (const void *)out, 19 + 8) <= 0){}
			}
			//ACP_read();
 		}
		else
		{
		}
 		break;

	// audio control, 0:bv, 1:ipod
 	case 20:
		exe_result = ChecksumCheck(theMsg);
		if(exe_result == CHECKSUM_OK)
		{
			unsigned char UC = 0;

			if (-1 != i2c_read_data(i2c0, PCA_9555_SLAVE_1, PCA_9555_INPUT_0, &UC, 1))
			{
				if (0 == theMsg->item_one_data)	UC &= ~BIT7;
				else								UC |= BIT7;
				i2c_write_data(i2c0, PCA_9555_SLAVE_1, PCA_9555_OUTPUT_0, &UC, 1);
			}
			packOurProtocol(CMD_GUI | (1 << 6), 0, 15, (unsigned char *)reply20, out);
			if (write(fd, (const void *)out, 15 + 8) <= 0){}
		}
 		break;

	//beep
 	case 99:
		exe_result = ChecksumCheck(theMsg);
		if(exe_result == CHECKSUM_OK)
		{
	 		if (-1 != pwm1)
	 		{
	 			if (0 == ioctl(pwm1, PWM_IOCTL_SET_ENABLE, 1))
					SetTimer(PWM1_TIMER_ID, 100, 0);
	 		}
			//sys->beep_time_ms =  theMsg->item_one_data;

			packOurProtocol(CMD_GUI | (1 << 6), 0, 15, (unsigned char *)reply99, out);
			if (write(fd, (const void *)out, 15 + 8) <= 0){}
		}
 		break;


	default:
		return 0;
	}

	//TellFpcStart();

	return 1;
}

void CFpcApp::WorkoutData_initialize(void)
{
	memset((void *)rt, 0, sizeof(struct RunTimeVar));
	rt->workLoad_Table = _workLoad_Table;
	rt->workWatt_Table = _workWatt_Table;
	rt->workPace_Table = _workPace_Table;
	rt->workLoad_Table_cruise = _workLoad_Table_cruise;
	rt->segmentTime_Table = _segmentTime_Table;
	memset((void *)exception, 0, sizeof(struct Exception));
	memset((void *)summary, 0, sizeof(struct WorkoutSummary));
	memset((void *)update, 0, sizeof(struct RealtimeUpdateData));
}


unsigned char CFpcApp::ProgramEngineStart(void)
{
	unsigned int mode;
	char wmName[128];


///////////////////////////////////////////////////////////
	update_cooldown_pace_flag = 0;
	resume_mod_flag = 0;
	rt->workout_mode = MANUAL_MODE;
	rt->base_cruise_watt = 0;
	//ipod_vol_tab(i2c0, IPOD_VOL_VAR);


///////////////////////////////////////////////////////////
	mode = setup->Work_mode;
	if (WP_HOME  == mode)									sprintf(wmName, "WP_HOME");
	else if (WP_MANUAL_QUICK_START  == mode)					sprintf(wmName, "WP_MANUAL_QUICK_START");
	else if (WP_MANUAL_QUICK_START_RUN  == mode)			sprintf(wmName, "WP_MANUAL_QUICK_START_RUN");
	else if (WP_MANUAL_QUICK_START_SUMMARY  == mode)		sprintf(wmName, "WP_MANUAL_QUICK_START_SUMMARY");
	else if (WP_CARDIO_360_QUICK_START  == mode)			sprintf(wmName, "WP_CARDIO_360_QUICK_START");
	else if (WP_CARDIO_360_QUICK_START_RUN  == mode)		sprintf(wmName, "WP_CARDIO_360_QUICK_START_RUN");
	else if (WP_CARDIO_360_QUICK_START_SUMMARY  == mode)	sprintf(wmName, "WP_CARDIO_360_QUICK_START_SUMMARY");
	else if (WP_LANG_SELECT  == mode)							sprintf(wmName, "WP_LANG_SELECT");
	else if (WP_WORKOUT_FINDER  == mode)						sprintf(wmName, "WP_WORKOUT_FINDER");
	else if (WP_C360  == mode)								sprintf(wmName, "WP_C360");
	else if (WP_WL  == mode)									sprintf(wmName, "WP_WL");
	else if (WP_HRC  == mode)									sprintf(wmName, "WP_HRC");
	else if (WP_PERFORMANCE  == mode)						sprintf(wmName, "WP_PERFORMANCE");
	else if (WP_CUSTOM  == mode)								sprintf(wmName, "WP_CUSTOM");
	else if (WP_MANUAL_A  == mode)							sprintf(wmName, "WP_MANUAL_A");
	else if (WP_MANUAL_B  == mode)							sprintf(wmName, "WP_MANUAL_B");
	else if (WP_MANUAL_RUN  == mode)							sprintf(wmName, "WP_MANUAL_RUN");
	else if (WP_MANUAL_SUMMARY  == mode)					sprintf(wmName, "WP_MANUAL_SUMMARY");
	else if (WP_MANUAL_SAVE  == mode)						sprintf(wmName, "WP_MANUAL_SAVE");
	else if (WP_C360_QUICK_START  == mode)					sprintf(wmName, "WP_C360_QUICK_START");
	else if (WP_C360_QUICK_START_RUN  == mode)				sprintf(wmName, "WP_C360_QUICK_START_RUN");
	else if (WP_C360_QUICK_START_SUMMARY  == mode)			sprintf(wmName, "WP_C360_QUICK_START_SUMMARY");
	else if (WP_C360_ARM_SCULPTOR  == mode)					sprintf(wmName, "WP_C360_ARM_SCULPTOR");
	else if (WP_C360_ARM_SCULPTOR_RUN  == mode)				sprintf(wmName, "WP_C360_ARM_SCULPTOR_RUN");
	else if (WP_C360_ARM_SCULPTOR_SUMMARY  == mode)		sprintf(wmName, "WP_C360_ARM_SCULPTOR_SUMMARY");
	else if (WP_C360_ARM_SCULPTOR_SAVE  == mode)			sprintf(wmName, "WP_C360_ARM_SCULPTOR_SAVE");
	else if (WP_C360_LEG_SHAPER  == mode)					sprintf(wmName, "WP_C360_LEG_SHAPER");
	else if (WP_C360_LEG_SHAPER_RUN  == mode)				sprintf(wmName, "WP_C360_LEG_SHAPER_RUN");
	else if (WP_C360_LEG_SHAPER_SUMMARY  == mode)			sprintf(wmName, "WP_C360_LEG_SHAPER_SUMMARY");
	else if (WP_C360_LEG_SHAPER_SAVE  == mode)				sprintf(wmName, "WP_C360_LEG_SHAPER_SAVE");
	else if (WP_C360_CUSTOM_A  == mode)						sprintf(wmName, "WP_C360_CUSTOM_A");
	else if (WP_C360_CUSTOM_B  == mode)						sprintf(wmName, "WP_C360_CUSTOM_B");
	else if (WP_C360_CUSTOM_RUN  == mode)					sprintf(wmName, "WP_C360_CUSTOM_RUN");
	else if (WP_C360_CUSTOM_SUMMARY  == mode)				sprintf(wmName, "WP_C360_CUSTOM_SUMMARY");
	else if (WP_C360_CUSTOM_SAVE  == mode)					sprintf(wmName, "WP_C360_CUSTOM_SAVE");
	else if (WP_CALORIE_GOAL  == mode)						sprintf(wmName, "WP_CALORIE_GOAL");
	else if (WP_CALORIE_GOAL_RUN  == mode)					sprintf(wmName, "WP_CALORIE_GOAL_RUN");
	else if (WP_CALORIE_GOAL_SUMMARY  == mode)				sprintf(wmName, "WP_CALORIE_GOAL_SUMMARY");
	else if (WP_CALORIE_GOAL_SAVE  == mode)					sprintf(wmName, "WP_CALORIE_GOAL_SAVE");
	else if (WP_GLUTE_BUSTER  == mode)						sprintf(wmName, "WP_GLUTE_BUSTER");
	else if (WP_GLUTE_BUSTER_RUN  == mode)					sprintf(wmName, "WP_GLUTE_BUSTER_RUN");
	else if (WP_GLUTE_BUSTER_SUMMARY  == mode)				sprintf(wmName, "WP_GLUTE_BUSTER_SUMMARY");
	else if (WP_GLUTE_BUSTER_SAVE  == mode)					sprintf(wmName, "WP_GLUTE_BUSTER_SAVE");
	else if (WP_LEG_SHAPER_A  == mode)						sprintf(wmName, "WP_LEG_SHAPER_A");
	else if (WP_LEG_SHAPER_B  == mode)						sprintf(wmName, "WP_LEG_SHAPER_B");
	else if (WP_LEG_SHAPER_RUN  == mode)						sprintf(wmName, "WP_LEG_SHAPER_RUN");
	else if (WP_LEG_SHAPER_SUMMARY  == mode)				sprintf(wmName, "WP_LEG_SHAPER_SUMMARY");
	else if (WP_LEG_SHAPER_SAVE  == mode)					sprintf(wmName, "WP_LEG_SHAPER_SAVE");
	else if (WP_WL_A  == mode)								sprintf(wmName, "WP_WL_A");
	else if (WP_WL_B  == mode)								sprintf(wmName, "WP_WL_B");
	else if (WP_WL_RUN  == mode)								sprintf(wmName, "WP_WL_RUN");
	else if (WP_WL_SUMMARY  == mode)							sprintf(wmName, "WP_WL_SUMMARY");
	else if (WP_WL_SAVE  == mode)							sprintf(wmName, "WP_WL_SAVE");
	else if (WP_TARGET_HRC_A  == mode)						sprintf(wmName, "WP_TARGET_HRC_A");
	else if (WP_TARGET_HRC_B  == mode)						sprintf(wmName, "WP_TARGET_HRC_B");
	else if (WP_TARGET_HRC_RUN  == mode)						sprintf(wmName, "WP_TARGET_HRC_RUN");
	else if (WP_TARGET_HRC_SUMMARY  == mode)				sprintf(wmName, "WP_TARGET_HRC_SUMMARY");
	else if (WP_TARGET_HRC_SAVE  == mode)					sprintf(wmName, "WP_TARGET_HRC_SAVE");
	else if (WP_WL_HRC_A  == mode)							sprintf(wmName, "WP_WL_HRC_A");
	else if (WP_WL_HRC_B  == mode)							sprintf(wmName, "WP_WL_HRC_B");
	else if (WP_WL_HRC_RUN  == mode)							sprintf(wmName, "WP_WL_HRC_RUN");
	else if (WP_WL_HRC_SUMMARY  == mode)					sprintf(wmName, "WP_WL_HRC_SUMMARY");
	else if (WP_WL_HRC_SAVE  == mode)						sprintf(wmName, "WP_WL_HRC_SAVE");
	else if (WP_AEROBIC_HRC_A  == mode)						sprintf(wmName, "WP_AEROBIC_HRC_A");
	else if (WP_AEROBIC_HRC_B  == mode)						sprintf(wmName, "WP_AEROBIC_HRC_B");
	else if (WP_AEROBIC_HRC_RUN  == mode)					sprintf(wmName, "WP_AEROBIC_HRC_RUN");
	else if (WP_AEROBIC_HRC_SUMMARY  == mode)				sprintf(wmName, "WP_AEROBIC_HRC_SUMMARY");
	else if (WP_AEROBIC_HRC_SAVE  == mode)					sprintf(wmName, "WP_AEROBIC_HRC_SAVE");
	else if (WP_INTERVAL_HRC_A  == mode)						sprintf(wmName, "WP_INTERVAL_HRC_A");
	else if (WP_INTERVAL_HRC_B  == mode)						sprintf(wmName, "WP_INTERVAL_HRC_B");
	else if (WP_INTERVAL_HRC_RUN  == mode)					sprintf(wmName, "WP_INTERVAL_HRC_RUN");
	else if (WP_INTERVAL_HRC_SUMMARY  == mode)				sprintf(wmName, "WP_INTERVAL_HRC_SUMMARY");
	else if (WP_INTERVAL_HRC_SAVE  == mode)					sprintf(wmName, "WP_INTERVAL_HRC_SAVE");

	else if (WP_DISTANCE_RUN  == mode)						sprintf(wmName, "WP_DISTANCE_RUN");
	else if (WP_PACE_RAMP_RUN == mode)						sprintf(wmName, "WP_PACE_RAMP_RUN");
	else if (WP_RANDOM_HILLS_RUN == mode)					sprintf(wmName, "WP_RANDOM_HILLS_RUN");
	else if (WP_CARDIO_CHALLENGE_RUN == mode)				sprintf(wmName, "WP_CARDIO_CHALLENGE_RUN");
	else if (WP_PACE_INTERVALS_RUN == mode)					sprintf(wmName, "WP_PACE_INTERVALS_RUN");
	else if (WP_FITNESS_RUN == mode)							sprintf(wmName, "WP_FITNESS_RUN");
	else if (WP_CARDIO_360_DEMO_RUN == mode)				sprintf(wmName, "WP_CARDIO_360_DEMO_RUN");

	else														sprintf(wmName, "WP_UNDEF");
	printf("(%s %d) ProgramEngineStart(%s %d)\n", __FILE__, __LINE__, wmName, mode);


///////////////////////////////////////////////////////////
	if (
		WP_MANUAL_QUICK_START_RUN != mode &&
		WP_FITNESS_RUN != mode &&
		WP_DISTANCE_RUN != mode &&
		WP_C360_QUICK_START_RUN != mode &&
		WP_CARDIO_360_QUICK_START_RUN != mode &&
		WP_MANUAL_RUN != mode &&
		WP_C360_ARM_SCULPTOR_RUN != mode &&
		WP_C360_LEG_SHAPER_RUN != mode &&
		WP_C360_CUSTOM_RUN != mode &&
		WP_CALORIE_GOAL_RUN != mode &&
		WP_GLUTE_BUSTER_RUN != mode &&
		WP_LEG_SHAPER_RUN != mode &&
		WP_WL_HRC_RUN != mode &&
		WP_WL_RUN != mode &&
		WP_TARGET_HRC_RUN != mode &&
		WP_AEROBIC_HRC_RUN != mode &&
		WP_INTERVAL_HRC_RUN != mode &&
		WP_CARDIO_CHALLENGE_RUN != mode &&
		WP_CUSTOM_UTRA_RUN != mode &&
		WP_WALK_INTERVALS_RUN != mode &&
		WP_PACE_INTERVALS_RUN != mode &&
		WP_PACE_RAMP_RUN != mode &&
		WP_ROLLING_HILLS_RUN != mode &&
		WP_HILL_INTVALS_RUN != mode &&
		WP_RANDOM_HILLS_RUN != mode &&
		WP_SINGLE_HILL_RUN != mode &&
		WP_CARDIO_360_DEMO_RUN != mode
	)
	{
		printf("(%s %d) CHANGE TO WP_MANUAL_QUICK_START_RUN\n", __FILE__, __LINE__);
		mode = WP_MANUAL_QUICK_START_RUN;
	}


///////////////////////////////////////////////////////////
	WorkoutData_initialize();
	//memset((void *)&rt->total, 0, sizeof(struct WorkOutSummaryData));
	//memset((void *)&rt->cooldown, 0, sizeof(struct WorkOutSummaryData));
	//memset((void *)&rt->workout, 0, sizeof(struct WorkOutSummaryData));
	//memset((void *)&rt->warmup, 0, sizeof(struct WorkOutSummaryData));
	//memset((void *)&rt->summary_state, 0, sizeof(struct WorkOutSummaryState));

	exception->cmd.auto_stride = 0;
	InititalCalculation();
	InitialSummary();

	rt->workout_state = IN_RUNNING;
	rt->Average_Rpm_ACC = 0;
	rt->Average_Heart_Rate_ACC = 0;
	rt->waste_rpm_elapsed_time = 0;
	rt->waste_hr_elapsed_time = 0;
	rt->workLoad.current_load_level = 1;


	if (setup->Workload_level > 0)	update->Workload_level = setup->Workload_level;
	else							update->Workload_level = 1;



///////////////////////////////////////////////////////////
	SetTimer(FPC_SCHEDULER_1000MS_ID, 1, CalculateTimer);

	KillTimer(BL_OFF_TIMER_ID);
	//if (prob_pca955_1 && 1 != product_type)
		//SetTimer(FPC_SCHEDULER_1MS_ID, 1, HrTimer);
	//SetTimer(FPC_SCHEDULER_200MS_ID, 3, 0);


///////////////////////////////////////////////////////////
	sleep(2);

	switch(mode)
	{

#if 1
	//Cardio360 demo	
	case WP_CARDIO_360_DEMO_RUN:
		update->Target_heart_rate = setup->Target_heart_rate = 0;
		setup->Age = 35;
		setup->Weight = (unsigned short)((double)150.00F / (double)2.2F);
		setup->weight = (double)150.00F / (double)2.2F;
		setup->Workout_time_1000 = 0;
		setup->Workout_time = 158;
		setup->Workload_level = 1;

		for (int i = 0; i < 99; i++)			rt->workLoad_Table[i] = 0, rt->workLoad_Table_cruise[i] = 0;
		rt->Target_Workload_level = 1;
		exception->cmd.work_load_up = 1;

		setup->Segments = 9;
		memset(setup->Segments_time, 0, sizeof(setup->Segments_time));
		for (int i = 0; i < setup->Segments; i++)
			setup->Segments_time[i] = 60;
		setup->Segments_time[0] = 3;
		setup->Segments_time[1] = 30;
		setup->Segments_time[2] = 30;
		setup->Segments_time[3] = 15;
		setup->Segments_time[4] = 15;
		setup->Segments_time[5] = 20;
		setup->Segments_time[6] = 20;
		setup->Segments_time[7] = 20;
		setup->Segments_time[8] = 5;

		rt->segmentTime_Table[9] = 0;
		rt->workLoad_Table[9] = 1;
		rt->workPace_Table[9] = 0;

		rt->watt_calc_mod = CALC_BY_INDEXED_LOAD_LEVEL;
		rt->load_adj_mode = INDEXED_TARGET_LOAD_ADJ_WLClass3;
		Cardio360_Demo();
		break;

	default:
		setup->Work_mode = WP_MANUAL_QUICK_START_RUN;
	case WP_MANUAL_QUICK_START_RUN:
		update->Target_heart_rate = setup->Target_heart_rate = 0;
		setup->Age = 35;
		setup->Weight = (unsigned short)((double)150.00F / (double)2.2F);
		setup->weight = (double)150.00F / (double)2.2F;
		// 沒有時間限制
		setup->Workout_time_1000 = (99 * 60 + 59) / 1000;
		setup->Workout_time = (99 * 60 + 59) % 1000;
		setup->Workload_level = 1;
		memset(setup->Workload, 1, sizeof(setup->Workload));

		for (int i = 0; i < 99; i++)			rt->workLoad_Table[i] = 0, rt->workLoad_Table_cruise[i] = 0;//setup->Workload_level;
		rt->Target_Workload_level = 1;
		exception->cmd.work_load_up = 1;

		rt->watt_calc_mod = CALC_BY_CURRENT_LOAD_LEVEL;
		rt->load_adj_mode = SEGMENT_LOAD_ADJ;
		exception->cmd.auto_populate_pace = 1;

		Manual_Quick_Start();
		break;

	case WP_FITNESS_RUN:
		rt->watt_calc_mod = CALC_BY_CURRENT_LOAD_LEVEL;
		rt->load_adj_mode = SEGMENT_LOAD_ADJ;
		exception->cmd.auto_populate_pace = 1;

		Performance_Fitness_Test();
		break;

	case WP_DISTANCE_RUN:
		rt->watt_calc_mod = CALC_BY_CURRENT_LOAD_LEVEL;
		rt->load_adj_mode = SEGMENT_LOAD_ADJ;
		exception->cmd.auto_populate_pace = 1;
		All_Preset_Distance();
		break;

	case WP_C360_QUICK_START_RUN:
	case WP_CARDIO_360_QUICK_START_RUN:
		update->Target_heart_rate = setup->Target_heart_rate = 0;
		setup->Age = 35;
		setup->Weight = (unsigned short)((double)150.00F / (double)2.2F);
		setup->weight = (double)150.00F / (double)2.2F;
		setup->Workout_time_1000 = 1;
		setup->Workout_time = 200;
		setup->Workload_level = 1;

		for (int i = 0; i < 99; i++)			rt->workLoad_Table[i] = 0, rt->workLoad_Table_cruise[i] = 0;
		rt->Target_Workload_level = 1;
		exception->cmd.work_load_up = 1;

		setup->Segments = 10;
		memset(setup->Segments_time, 0, sizeof(setup->Segments_time));
		for (int i = 0; i < setup->Segments; i++)
			setup->Segments_time[i] = 60;
		setup->Segments_time[2] = 15;
		setup->Segments_time[3] = 15;
		setup->Segments_time[4] = 45;
		setup->Segments_time[5] = 30;
		setup->Segments_time[7] = 30;
		setup->Segments_time[8] = 45;

		rt->watt_calc_mod = CALC_BY_INDEXED_LOAD_LEVEL;
		rt->load_adj_mode = INDEXED_TARGET_LOAD_ADJ_WLClass3;
		Cardio360_Quick_Start();
		break;

	case WP_MANUAL_RUN:
		rt->watt_calc_mod = CALC_BY_CURRENT_LOAD_LEVEL;
		rt->load_adj_mode = SEGMENT_LOAD_ADJ;
		exception->cmd.auto_populate_pace = 1;
		Manual_Workout();	
		break;

	case WP_C360_ARM_SCULPTOR_RUN:
		rt->watt_calc_mod	= CALC_BY_CURRENT_LOAD_LEVEL;
		rt->load_adj_mode = SEGMENT_LOAD_ADJ;
		Cardio360_Arm_Sculptor();
		break;
	case WP_C360_LEG_SHAPER_RUN:
		rt->watt_calc_mod	 = CALC_BY_CURRENT_LOAD_LEVEL;
		rt->load_adj_mode = SEGMENT_LOAD_ADJ;
		Cardio360_Leg_Shaper();
		break;	

	case WP_C360_CUSTOM_RUN:
		rt->watt_calc_mod = CALC_BY_INDEXED_LOAD_LEVEL;
		rt->load_adj_mode = INDEXED_TARGET_LOAD_ADJ_WLClass3;
		Cardio360_Customized();
		break;

	case WP_CALORIE_GOAL_RUN:
		rt->watt_calc_mod = CALC_BY_CURRENT_LOAD_LEVEL;
		rt->load_adj_mode = SEGMENT_LOAD_ADJ;
		exception->cmd.auto_populate_pace = 1;
		Weight_Loss_Calorie_Goal();
		break;

// NEW GLUTE BUSTER
	case WP_GLUTE_BUSTER_RUN:
		rt->watt_calc_mod = CALC_BY_INDEXED_LOAD_LEVEL;
		rt->load_adj_mode = INDEXED_TARGET_LOAD_ADJ_WLClass20;
		exception->cmd.auto_populate_pace = 1;
		Weight_Loss_Glute();
		break;

// WP_LEG_SHAPER_RUN
	case WP_LEG_SHAPER_RUN:
		update_cooldown_pace_flag = 1;
		rt->watt_calc_mod = CALC_BY_CURRENT_LOAD_LEVEL;
		rt->load_adj_mode = SEGMENT_LOAD_ADJ;
		rt->pace_adj_mode = INDEXED_TARGET_PACE_ADJ_WLClass20;

		//exception->cmd.auto_populate_pace = 1;

		Weight_Loss_Leg();
		break;

	case WP_WL_HRC_RUN:
	case WP_WL_RUN:
		rt->watt_calc_mod = CALC_BY_CURRENT_LOAD_LEVEL;
		rt->load_adj_mode = SEGMENT_LOAD_ADJ;	
		exception->cmd.auto_populate_pace = 1;
		Hrc_Weight_Loss();
		break;

	case WP_TARGET_HRC_RUN:
		rt->watt_calc_mod = CALC_BY_CURRENT_LOAD_LEVEL;
		rt->load_adj_mode = SEGMENT_LOAD_ADJ;
		exception->cmd.auto_populate_pace = 1;
		Hrc_Target();
		break;

	case WP_AEROBIC_HRC_RUN:
		rt->watt_calc_mod = CALC_BY_CURRENT_LOAD_LEVEL;
		rt->load_adj_mode = SEGMENT_LOAD_ADJ;
		exception->cmd.auto_populate_pace = 1;
		Hrc_Aerobic();
		break;

	case WP_INTERVAL_HRC_RUN:
		rt->watt_calc_mod = CALC_BY_CURRENT_LOAD_LEVEL;
		rt->load_adj_mode = SEGMENT_LOAD_ADJ;
		exception->cmd.auto_populate_pace = 1;
		Hrc_Intervals();
		break;

	case WP_CARDIO_CHALLENGE_RUN:
		update_cooldown_pace_flag = 1;
		rt->watt_calc_mod = CALC_BY_INDEXED_LOAD_LEVEL;
		rt->load_adj_mode = INDEXED_TARGET_LOAD_ADJ_WLClass20;

		rt->pace_adj_mode = INDEXED_TARGET_PACE_ADJ_WLClass20;
		Performance_Cardio_Challenge();
		break;

	case WP_CUSTOM_UTRA_RUN:
		update_cooldown_pace_flag = 1;
		rt->load_adj_mode = SEGMENT_LOAD_ADJ;
		rt->watt_calc_mod = CALC_BY_CURRENT_LOAD_LEVEL;
		Custom_Ultra();
		break;

	case WP_WALK_INTERVALS_RUN:
		update_cooldown_pace_flag = 1;
		rt->watt_calc_mod = CALC_BY_CURRENT_LOAD_LEVEL;
		rt->load_adj_mode = SEGMENT_LOAD_ADJ;

		rt->pace_adj_mode = INDEXED_TARGET_PACE_ADJ_WLClass2;

		Weight_Loss_Walk_and_Run();
		break;

	case WP_PACE_INTERVALS_RUN:
		update_cooldown_pace_flag = 1;
		rt->watt_calc_mod = CALC_BY_CURRENT_LOAD_LEVEL;
		rt->load_adj_mode = SEGMENT_LOAD_ADJ;

		rt->pace_adj_mode = INDEXED_TARGET_PACE_ADJ_WLClass2;

		Performance_Pace_Intervals();
		break;

	case WP_PACE_RAMP_RUN:
		update_cooldown_pace_flag = 1;
		//rt->pace_adj_mode = INDEXED_TARGET_PACE_ADJ_WLClass20;
		//rt->watt_calc_mod = CALC_BY_INDEXED_LOAD_LEVEL;
		//rt->load_adj_mode = INDEXED_TARGET_LOAD_ADJ_WLClass20;

		rt->watt_calc_mod	 = CALC_BY_CURRENT_LOAD_LEVEL;
		rt->load_adj_mode = SEGMENT_LOAD_ADJ;		
		rt->pace_adj_mode = INDEXED_TARGET_PACE_ADJ_WLClass20;

		Performance_Pace_Ramp();
		break;

	case WP_ROLLING_HILLS_RUN:
		update_cooldown_pace_flag = 1;
		rt->watt_calc_mod = CALC_BY_INDEXED_LOAD_LEVEL;
		rt->load_adj_mode = INDEXED_TARGET_LOAD_ADJ_WLClass20;
		exception->cmd.auto_populate_pace = 1;
		Weight_Loss_Rolling_Hills();
		break;

	case WP_HILL_INTVALS_RUN:
		update_cooldown_pace_flag = 1;
		rt->load_adj_mode = INDEXED_TARGET_LOAD_ADJ_WLClass2;
		rt->watt_calc_mod = CALC_BY_INDEXED_LOAD_LEVEL;
		exception->cmd.auto_populate_pace = 1;
		Performance_Hill_Intervals();
		break;

	// NEW RANDOM HILL
	case WP_RANDOM_HILLS_RUN:
		update_cooldown_pace_flag = 1;
		rt->watt_calc_mod = CALC_BY_INDEXED_LOAD_LEVEL;
		rt->load_adj_mode = INDEXED_TARGET_LOAD_ADJ_WLClass20;
		exception->cmd.auto_populate_pace = 1;	
		Random_Hills();
		break;

	case WP_SINGLE_HILL_RUN:
		update_cooldown_pace_flag = 1;
		rt->watt_calc_mod = CALC_BY_INDEXED_LOAD_LEVEL;
		rt->load_adj_mode = INDEXED_TARGET_LOAD_ADJ_WLClass20;
		exception->cmd.auto_populate_pace = 1;	
		Performance_One_Big_Hill();
		break;
#else
	//Manual Manual in the spec sheet
	case 402:
		rt->watt_calc_mod = CALC_BY_CURRENT_LOAD_LEVEL;
		rt->load_adj_mode = SEGMENT_LOAD_ADJ;
		exception->cmd.auto_populate_pace = 1;
		Manual_Workout();	
		break;

	//Cardio360 demo	
	case 403:
		rt->watt_calc_mod = CALC_BY_INDEXED_LOAD_LEVEL;
		rt->load_adj_mode = INDEXED_TARGET_LOAD_ADJ_WLClass3;
		//rt->pace_adj_mode = NO_PACE_ADJ;	
		Cardio360_Demo();
		break;

	//Cardio360 Quick start 
	case 411:
		rt->watt_calc_mod = CALC_BY_INDEXED_LOAD_LEVEL;
		rt->load_adj_mode = INDEXED_TARGET_LOAD_ADJ_WLClass3;
		//rt->pace_adj_mode= NO_PACE_ADJ;
		Cardio360_Quick_Start();
		break;

	case 412:	//Cardio360 Video	
		rt->watt_calc_mod = CALC_BY_CURRENT_LOAD_LEVEL;
		rt->load_adj_mode = SEGMENT_LOAD_ADJ;
		//rt->pace_adj_mode= NO_PACE_ADJ;	
		Cardio360_Video();
		break;

	case 413:	//Cardio360 Arm sculptor	
		rt->watt_calc_mod = CALC_BY_CURRENT_LOAD_LEVEL;
		rt->load_adj_mode = SEGMENT_LOAD_ADJ;
		//rt->pace_adj_mode= NO_PACE_ADJ;
		Cardio360_Arm_Sculptor();
		break;

	case 414:	//Cardio360 Leg shaper
		rt->watt_calc_mod = CALC_BY_CURRENT_LOAD_LEVEL;
		rt->load_adj_mode = SEGMENT_LOAD_ADJ;
		//rt->pace_adj_mode= NO_PACE_ADJ;	
		Cardio360_Leg_Shaper();
		break;	

	case 415:	//Cardio360 Custom	
		rt->watt_calc_mod = CALC_BY_INDEXED_LOAD_LEVEL;
		rt->load_adj_mode = INDEXED_TARGET_LOAD_ADJ_WLClass3;
		//rt->pace_adj_mode= NO_PACE_ADJ;	
		Cardio360_Customized();
		break;

	case 421:	//Calorie goal	
		rt->watt_calc_mod = CALC_BY_CURRENT_LOAD_LEVEL;
		rt->load_adj_mode = SEGMENT_LOAD_ADJ;
		exception->cmd.auto_populate_pace = 1;
		//rt->pace_adj_mode= NO_PACE_ADJ;	
		Weight_Loss_Calorie_Goal();
		break;

	case 423:	//Rolling hills
		rt->watt_calc_mod = CALC_BY_INDEXED_LOAD_LEVEL;
		rt->load_adj_mode = INDEXED_TARGET_LOAD_ADJ_WLClass20;
		exception->cmd.auto_populate_pace = 1;
		Weight_Loss_Rolling_Hills();
		break;

	case 424:	//Walk & run intervals
		rt->watt_calc_mod = CALC_BY_CURRENT_LOAD_LEVEL;
		rt->load_adj_mode = SEGMENT_LOAD_ADJ;
		//rt->pace_adj_mode= NO_PACE_ADJ;	
		Weight_Loss_Walk_and_Run();
		break;

	case 431:	//HRC Target
		rt->watt_calc_mod = CALC_BY_CURRENT_LOAD_LEVEL;
		rt->load_adj_mode = SEGMENT_LOAD_ADJ;
		exception->cmd.auto_populate_pace = 1;
		Hrc_Target();
		break;

	case 432:	//HRC Weight loss	
		rt->watt_calc_mod = CALC_BY_CURRENT_LOAD_LEVEL;
		rt->load_adj_mode = SEGMENT_LOAD_ADJ;	
		exception->cmd.auto_populate_pace = 1;
		Hrc_Weight_Loss();
		break;

	case 433:	//HRC Aerobic	
		rt->watt_calc_mod = CALC_BY_CURRENT_LOAD_LEVEL;
		rt->load_adj_mode = SEGMENT_LOAD_ADJ;
		exception->cmd.auto_populate_pace = 1;	
		Hrc_Aerobic();
		break;

	case 434:	//HRC Distance
		rt->watt_calc_mod = CALC_BY_CURRENT_LOAD_LEVEL;
		rt->load_adj_mode = SEGMENT_LOAD_ADJ;	
		exception->cmd.auto_populate_pace = 1;
		Hrc_Distance();
		break;

	case 435:	//HRC Intervals	
		rt->watt_calc_mod = CALC_BY_CURRENT_LOAD_LEVEL;
		rt->load_adj_mode = SEGMENT_LOAD_ADJ;
		exception->cmd.auto_populate_pace = 1;
		Hrc_Intervals();
		break;

	case 441:	//Cardio challlenge	
		rt->watt_calc_mod = CALC_BY_INDEXED_LOAD_LEVEL;
		rt->load_adj_mode = INDEXED_TARGET_LOAD_ADJ_WLClass20;
		rt->pace_adj_mode = INDEXED_TARGET_PACE_ADJ_WLClass20;
		Performance_Cardio_Challenge();
		break;

	case 445:	//Fitness test
		rt->watt_calc_mod = CALC_BY_CURRENT_LOAD_LEVEL;//CALC_BY_INDEXED_LOAD_LEVEL;
		rt->load_adj_mode = SEGMENT_LOAD_ADJ;//TARGET_WATT_ADJ;	
		exception->cmd.auto_populate_pace = 1;
		Performance_Fitness_Test();
		break;

	case 446:	//Pace ramp	
		rt->watt_calc_mod = CALC_BY_CURRENT_LOAD_LEVEL;
		rt->load_adj_mode = SEGMENT_LOAD_ADJ;
		rt->pace_adj_mode = INDEXED_TARGET_PACE_ADJ_WLClass20;
		Performance_Pace_Ramp();
		break;

	case 447:	//Pace intervals	
		rt->watt_calc_mod = CALC_BY_CURRENT_LOAD_LEVEL;
		rt->load_adj_mode = SEGMENT_LOAD_ADJ;
		Performance_Pace_Intervals();
		break;

	case 448:	//One big hill
		rt->watt_calc_mod = CALC_BY_INDEXED_LOAD_LEVEL;
		rt->load_adj_mode = INDEXED_TARGET_LOAD_ADJ_WLClass20;
		exception->cmd.auto_populate_pace = 1;	
		Performance_One_Big_Hill();
		break;

	case 449:	//Hill  intervals	
// 		rt->load_adj_mode = SEGMENT_LOAD_ADJ;
// 		rt->watt_calc_mod = CALC_BY_CURRENT_LOAD_LEVEL;
		rt->load_adj_mode = INDEXED_TARGET_LOAD_ADJ_WLClass2;
		rt->watt_calc_mod = CALC_BY_INDEXED_LOAD_LEVEL;
		exception->cmd.auto_populate_pace = 1;
		Performance_Hill_Intervals();
		break;

	case 451:	//Custom Pace	
		rt->watt_calc_mod = CALC_BY_CURRENT_LOAD_LEVEL;
		rt->load_adj_mode = SEGMENT_LOAD_ADJ;
		Custom_Pace();
		break;

	case 452:	//Custom Hills
		rt->watt_calc_mod = CALC_BY_CURRENT_LOAD_LEVEL;
		rt->load_adj_mode = SEGMENT_LOAD_ADJ;	
		//exception->cmd.auto_populate_pace = 1;	
		Custom_Hills();
		break;

	case 453:	//Custom Intervals	
		rt->load_adj_mode = SEGMENT_LOAD_ADJ;
		rt->watt_calc_mod = CALC_BY_CURRENT_LOAD_LEVEL;
		Custom_Ultra();
		break;

	case 454:	//Custom HRC intervals
		rt->watt_calc_mod = CALC_BY_CURRENT_LOAD_LEVEL;
		rt->load_adj_mode = SEGMENT_LOAD_ADJ;	
		exception->cmd.auto_populate_pace = 1;	
		Custom_HRC_Intervals();
		break;

	case 461:	//Distance workout(5km,10km,2mi,4mi)
		rt->watt_calc_mod = CALC_BY_CURRENT_LOAD_LEVEL;
		rt->load_adj_mode = SEGMENT_LOAD_ADJ;
		exception->cmd.auto_populate_pace = 1;
		All_Preset_Distance();
		break;
	default:
		break;// 0;
#endif // 1

	}


	//EndofProgram(MANUAL_QUICK_START);


	//CalculateSummaryData();
//printf("(%s %d) Total_time_elapsed=%d, Cooldown_time_elapsed=%d\n",  __FILE__, __LINE__, summary->Total_time_elapsed, summary->Cooldown_time_elapsed);

/*
	rt->summary_state.has_cooldown = 0;
	CollectWorkOutSummaryData();
	CalculateSummaryData();
	if (1)
	{
		unsigned short t;

		t = summary->Total_time_elapsed_1000 * 1000 + summary->Total_time_elapsed;
		t += (summary->Cooldown_time_elapsed_1000 * 1000);
		t += summary->Cooldown_time_elapsed;

		summary->Total_time_elapsed_1000 = t / 1000;
		summary->Total_time_elapsed = t % 1000;
	}
*/

	CalculateSummaryData();
	//key_scan[24] = FPC_FIRE_SUMMARY_READY;
	rt->workout_state = READY_TO_FINISH;


	for(int i = 0; i < 1000; i++)
		KillTimer(FPC_SCHEDULER_1000MS_ID), usleep(1);

	//if (2 != product_type)
		//KillTimer(FPC_SCHEDULER_1MS_ID);

	//KillTimer(FPC_SCHEDULER_200MS_ID);

	SetTimer(SD55_CMD_DEFAULT_STRIDE_ID, 1, 0);
	//SetTimer(FPC_WORKOUT_IN_NORMAL_ID, 5 * 1000, 0);
	SetTimer(FPC_WORKOUT_IN_NORMAL_ID, 100, 0);


printf("\n(%s %d) \nTotal_time=%d, ", __FILE__, __LINE__, 1000 * summary->Total_time_elapsed_1000 + summary->Total_time_elapsed);
printf("Total_distance=%d, ", summary->Total_distance_km_i * 1000 + summary->Total_distance_km_f);
printf("Total_calories=%d \n", summary->Total_calories_burned_1000 * 1000 + summary->Total_calories_burned);

printf("Workout_time=%d, ", summary->Workout_time_elapsed_1000 * 1000 + summary->Workout_time_elapsed);
printf("Workout_distance=%d, ", summary->Workout_distance_km_i * 1000 + summary->Workout_distance_km_f);
printf("Workout_calories=%d \n", summary->Workout_calories_burned_1000 * 1000 + summary->Workout_calories_burned);

printf("Cooldown_time=%d, ", summary->Cooldown_time_elapsed_1000 * 1000 + summary->Cooldown_time_elapsed);
printf("Cooldown_distance=%d, ", summary->Cooldown_distance_km_i * 1000 + summary->Cooldown_distance_km_f);
printf("Cooldown_calories=%d \n", summary->Cooldown_calories_burned_1000 * 1000 + summary->Cooldown_calories_burned);

printf("Warmup_time=%d, ", summary->Warmup_time_elapsed_1000 * 1000 + summary->Warmup_time_elapsed);
printf("Warmup_time=%d, ", summary->Warmup_distance_km_i * 1000 + summary->Warmup_distance_km_f);
printf("Warmup_time=%d \n", summary->Warmup_calories_burned_1000 * 1000 + summary->Warmup_calories_burned);
printf("\n");


//////////////////////
	//ipod_vol_tab(i2c0, 0);
	if (1 == bl_sleep_type)
		SetTimer(BL_OFF_TIMER_ID, 1000 * 60 * 15 + 1, BlOffTimer);
	else if (2 == bl_sleep_type)
		SetTimer(BL_OFF_TIMER_ID, 1000 * 60 * 30 + 1, BlOffTimer);
	else if (3 == bl_sleep_type)
		SetTimer(BL_OFF_TIMER_ID, 1000 * 60 * 40 + 1, BlOffTimer);
	else if (4 == bl_sleep_type)
		SetTimer(BL_OFF_TIMER_ID, 1000 * 60 * 60 + 1, BlOffTimer);


	TellFpcWait();
	return 0;
}



void CFpcApp::EndofProgram(unsigned char for_program)
{
printf("(%s %d) EndofProgram(%d)\n", __FILE__, __LINE__, rt->exception_result);


///////////////////////////////////////////////////////////
	// USER OVER HR
	if(rt->exception_result == EXCEPTION_OVERHR_BREAK)
		rt->workout_state = READY_TO_COOL_DOWN;

	// USER COOL
	if(rt->exception_result == EXCEPTION_COOLDOWN)
		rt->workout_state = READY_TO_COOL_DOWN;

	// USER STOP
	if(rt->exception_result == EXCEPTION_BREAK)
		rt->workout_state = READY_TO_FINISH;


///////////////////////////////////////////////////////////
	// NORMAL TIMEOUT
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


///////////////////////////////////////////////////////////
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


///////////////////////////////////////////////////////////
	//exception->cmd.auto_stride = 0;
	//exception->cmd.hr_cruise = 0;
 	exception->cmd.pause = 0;

 	//rt->summary_state.has_cooldown = 0;
 	//CollectWorkOutSummaryData();



///////////////////////////////////////////////////////////
	switch(rt->exception_result)
	{
	case EXCEPTION_CONTINUE:
		break;

	default:
printf("(%s %d) BUG(), EXCEPTION_DEFAULT, has_cooldown=%d\n", __FILE__, __LINE__, rt->summary_state.has_cooldown);

	case EXCEPTION_BREAK:
printf("(%s %d) EXCEPTION_BREAK, has_cooldown=%d\n", __FILE__, __LINE__, rt->summary_state.has_cooldown);
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
// jason note
			if (!rt->summary_state.has_cooldown)
				CollectWorkOutSummaryData();
		}

		rt->workout_state = READY_TO_SUMMARY;

		break;

	case EXCEPTION_OVERHR_BREAK:
//printf("(%s %d) EXCEPTION_OVERHR_BREAK\n", __FILE__, __LINE__);

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

	case EXCEPTION_COOLDOWN:
printf("(%s %d) EXCEPTION_COOLDOWN, rt->summary_state.did_warmup=%d rt->summary_state.has_warmup=%d\n", __FILE__, __LINE__, rt->summary_state.did_warmup, rt->summary_state.has_warmup);
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

#if 1
		Default_CoolDown_pace_process(for_program);
#else
		Default_CoolDown();
		CalculateCoolDownSummaryWithWorkout();
#endif

		break;

/*
	default:

printf("(%s %d) EXCEPTION_DEFAULT\n", __FILE__, __LINE__);

		//Wait 10 second
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
			WaitForProgramTick();//timer tick will com once 100ms		
		}
		if(rt->exception_result == EXCEPTION_COOLDOWN)
		{	
			CollectWorkOutSummaryData();
			Default_CoolDown_pace_process(for_program);
		}
		else
		{
			//rt->summary_state.has_cooldown = 0;
			CollectWorkOutSummaryData();
			//CollectTotalSummaryData();

			SetTimer(SD55_CMD_DEFAULT_STRIDE_ID, 1, 0);
		}
		break;
*/
	}


	for (int i = 0; i < 99; i++)			rt->workLoad_Table[i] = 0;//setup->Workload_level;

	update->Workload_level = rt->Target_Workload_level = 1;
	exception->cmd.work_load_up = 1;


///////////////////////////////////////////////////////////
	//rt->summary_state.has_workout = 0;
	SetTimer(SD55_CMD_DEFAULT_STRIDE_ID, 1, 0);

}
//#endif


void CFpcApp::Default_CoolDown(void)
{     
printf("(%s %d) Default_CoolDown()\n", __FILE__, __LINE__);

// jason
	exception->cmd.hr_cruise = 0;
	for(int i = rt->segment_index + 1; i <= rt->workLoad_TableUsed; i++)
		rt->workLoad_Table[i] = 1;

	//rt->workout_state = IN_NORMAL;
	rt->workout_mode = MANUAL_MODE;
	rt->workout_state = READY_TO_COOL_DOWN;

	//Clean up the variables for Cool down Part
	rt->Average_Heart_Rate = 0;
	rt->Max_HeartRate = 0;
	rt->Average_Rpm = 0;
	rt->Max_Rpm = 0;
	
 	update->Segment_time	 = 120;
 	update->Segment_time_1000 = 0;
	/*if(rt->segment_index == segment_index_max + 2)//$$$$$$$$$
 	       update->Time_remaining =0; //0秒
 	else
 	{
		update->Time_remaining = 120;//120秒
		printf("(%s %d) update->Time_remaining=120\n", __FILE__, __LINE__);
 	}*/
 	update->Time_remaining_1000 = 0;
	update->Time_remaining = 120;//120秒

	rt->total_workout_time_tick = FP_TICKS_PER_SECOND * default_cool_down_time * 60;

// jason
	rt->watt_calc_mod = CALC_BY_CURRENT_LOAD_LEVEL;
	rt->load_adj_mode = SEGMENT_LOAD_ADJ;

	update->Workload_level = rt->workLoad.current_load_level = default_work_load_level;
	AdjustMachineWorkLoad(SEGMENT_LOAD_ADJ, 0);

	rt->tick_1sec_per100ms = rt->tick_1sec_per100ms_reload;
	rt->exception_result = EXCEPTION_CONTINUE;
	rt->segment_index = rt->workLoad_TableUsed;

	while(rt->total_workout_time_tick > 0 && rt->exception_result == EXCEPTION_CONTINUE)
	//while(rt->exception_result == EXCEPTION_CONTINUE)
	{
		//handling the Exceptions		
		rt->exception_result = ExceptionHandler(DEFAULT_COOL_DOWN);
//printf("(%s %d) rt->exception_result=%d\n", __FILE__, __LINE__, rt->exception_result);

		//end of handling the Exceptions	
		if(exception->cmd.pause == 0)
		{
			///////////////////////////////////////////////////////////////////////////
			//Workout Time Update
			//if(rt->segment_time_tick > 0)
			//	rt->segment_time_tick--;
			if(rt->total_workout_time_tick > 0)
				rt->total_workout_time_tick --;

			//////////////////////////////////////////////////////
			//DataScreen Time / Bar Update
			//timer elapsed 1 sec after 100 tick count;

			if(rt->tick_1sec_per100ms > 0)
				rt->tick_1sec_per100ms--;	
			if(rt->tick_1sec_per100ms == 0)
			{
				rt->tick_1sec_per100ms = rt->tick_1sec_per100ms_reload;

				Update_Workout_Time_Elapsed();
				Update_Workout_Time_Remaining();

				if (
					update_cooldown_pace_flag
					/*setup->Work_mode == WP_LEG_SHAPER_RUN
					|| setup->Work_mode == WP_WALK_INTERVALS_RUN
					|| setup->Work_mode == WP_PACE_INTERVALS_RUN
					|| setup->Work_mode == WP_PACE_RAMP_RUN
					|| setup->Work_mode == WP_CUSTOM_UTRA_RUN
					|| setup->Work_mode == WP_HILL_INTVALS_RUN
					|| setup->Work_mode == WP_SINGLE_HILL_RUN
					|| setup->Work_mode == WP_RANDOM_HILLS_RUN
					|| setup->Work_mode == WP_ROLLING_HILLS_RUN*/
				)
				{
					//UpdateCoolDownPace(CUSTOM_PACE);
				}

// jason
//CollectCooldownSummaryData();
				rt->workLoad_Table[rt->workLoad_TableUsed] = update->Workload_level;

				Update_GUI_bar();
			}
			//////////////////////////////////////////////////////
		}


	//rt->cooldown.Time_elapsed = rt->elapsed_time;
	//rt->cooldown.Time_elapsed_1000 = rt->elapsed_time_1000;
//printf("(%s %d) rt->elapsed_time=%d\n", __FILE__, __LINE__, rt->elapsed_time_1000 * 1000 + rt->elapsed_time);



		//if (EXCEPTION_BREAK != rt->exception_result)
		WaitForProgramTick();
	}	


//printf("(%s %d) rt->exception_result=%d\n", __FILE__, __LINE__, rt->exception_result);

	//Summary data collection
	rt->workout_state = READY_TO_FINISH;
	CollectCooldownSummaryData();
	//rt->summary_state.has_workout = 0;

	//Set mechanical state to the default state
	SetTimer(SD55_CMD_DEFAULT_STRIDE_ID, 1, 0);
}

void CFpcApp::Default_CoolDown_pace_process(unsigned char for_program)
{
	unsigned char i;
	float fl = 0.00F;

	switch(for_program)
	{
	case CUSTOM_PACE:	
	case CUSTOM_ULTRA:		
	case CUSTOM_HILLS:		
	case PERFORMANCE_PACE_RAMP:
	case PERFORMANCE_PACE_INTERVAL:
	case WEIGHT_LOSS_WALK_AND_RUN:
	case WEIGHT_LOSS_LEG_SHAPTER:
		for(i = rt->segment_index; i < rt->workLoad_TableUsed ; i++)
			rt->workPace_Table[i] = rt->workPace_Table[rt->segment_index];

		//set pace to average 40% pace 
		fl = (float)rt->Average_Rpm;
		fl *= 0.4F;
		//update->Pace_level = 20;//GetWorkPaceLevel_of_ppi((unsigned char) fl);
		setup->Pace_level = GetWorkPaceLevel_of_ppi((unsigned char)fl);	
		update->Pace_level = setup->Pace_level;
		break;
	}

	Default_CoolDown();
}

void CFpcApp::All_Preset_Distance(void)
{  
	int distance_out = 0;

	//Set up screen
	//‧	Age			(default 35)  Adjustable from 10-99
	//‧	Weight	  		(default 150) Adjustable from 50-350(weight_max)
	//‧	Workout Distance:	(default 5K) this block will allow 4 options to chose from 5K, 10K, 2mi and 4mi. 
	//‧	Workload		(default Level 1) Adjustable from 1-20
	
	//Workout time Description:
	//‧	Time:		This work out does not have a set time, it will start a 0 and count elapsed time until the end of the set workout distance.
#if 1
	InitWorkoutPhase01_distance(90); //in HRC Distance.c
	InitWorkoutPhase02_apd(1);
#endif


printf("(%s %d) All_Preset_Distance(%f %d)\n", __FILE__, __LINE__, target_workout_distance, setup->Workout_distance);



	rt->watt_calc_mod = CALC_BY_CURRENT_LOAD_LEVEL;
	rt->load_adj_mode = SEGMENT_LOAD_ADJ;	

	exception->cmd.distance_started = 1;

	rt->exception_result = EXCEPTION_CONTINUE;
	//WorkOutDebugMessgae(3);	

	//while(rt->Distance_metric < rt->target_workout_distance && rt->exception_result == EXCEPTION_CONTINUE)
	while(rt->exception_result == EXCEPTION_CONTINUE)
	{
		//for each segment
		update->Segment_time = 60;//rt->segmentTime_Table[rt->segment_index];
 		rt->segment_time_tick = FP_TICKS_PER_SECOND * update->Segment_time;
		rt->workLoad.current_load_level = rt->workLoad_Table[rt->segment_index];

		//update->Workload_level = rt->workLoad.current_load_level;
		if(exception->cmd.hr_cruise == 0)
		{
			AdjustMachineWorkLoad(SEGMENT_LOAD_ADJ, 0);
		}

		//if(update->Segment_time == 0)rt->segment_index++;
		if(update->Segment_time == 0)
			rt->segment_index++;

		while(rt->segment_time_tick > 0 && rt->exception_result == EXCEPTION_CONTINUE)
		{
			//get realtime heartrate
			//rt->current_heart_rate = lcb->hr->DisplayHeartRate;
			rt->current_heart_rate = update->Heart_rate;

			//handling the Exceptions		
			rt->exception_result = ExceptionHandler(ALLPRESET_DISTANCE);
			if(rt->exception_result == EXCEPTION_COOLDOWN)
				break;
			if(rt->exception_result == EXCEPTION_BREAK)
				break;
			//end of handling the Exceptions	
			
			if(exception->cmd.pause == 0)
			{	//break this loop;
				//real time update machine realtime data
				/////20130409/////////
				//Workout Time Update
				if(rt->segment_time_tick > 0){
					rt->segment_time_tick--;
				}
				if(rt->segment_time_tick==0){
					rt->segment_index++;
					exception->cmd.segment_shift = 1;	//for Update_GUI_bar_distance()
				}
				if(rt->total_workout_time_tick > 0){
					rt->total_workout_time_tick --;
				}
				else
				{
					//InitWorkoutPhase01(99);
					rt->total_workout_time_tick = 99 * 60 * FP_TICKS_PER_SECOND;


/*
				if(rt->total_workout_time_tick > 0)
				{
					rt->total_workout_time_tick --;
				}
				else
				{
					InitWorkoutPhase01(99);
					rt->total_workout_time_tick = rt->total_workout_time * FP_TICKS_PER_SECOND;

					//rt->total_workout_time = setup->Workout_time_1000 * 1000 + setup->Workout_time;
					//update->Time_remaining  = setup->Workout_time;
					//update->Time_remaining_1000 = setup->Workout_time_1000;


					//rt->total_workout_time += rt->total_workout_time;
				}

*/



				}


				/////////////////////
				//DataScreen Time / Bar Update
				//timer elapsed 1 sec after 100 tick count;
				if(rt->tick_1sec_per100ms >0)
					rt->tick_1sec_per100ms--;	
				if(rt->tick_1sec_per100ms == 0)
				{
					rt->tick_1sec_per100ms = rt->tick_1sec_per100ms_reload;

					Update_Workout_Time_Elapsed();

// jason note
//					Update_Workout_Time_Remaining();
					update->Time_remaining_1000 = 0;
					update->Time_remaining = 1;


					Update_GUI_bar_distance();
				}
				/////20130409/////////

				if(exception->cmd.hr_cruise == 1)
				{
					rt->watt_calc_mod = CALC_BY_CURRENT_LOAD_LEVEL;
					HR_Cruise();
				}
			}

			WaitForProgramTick();	//timer tick will com once 100ms

			if(rt->Distance_metric >= rt->target_workout_distance)
			{
				update->Time_remaining_1000 = 0;
				update->Time_remaining = 0;
				distance_out = 1;
				goto DISTANCE_OUT;
			}			
		}
	}


DISTANCE_OUT:
	if(distance_out)
	{
printf("(%s %d) DISTANCE_OUT, %s(%f %f)\n", __FILE__, __LINE__, __FUNCTION__, rt->Distance_metric, rt->target_workout_distance);

		for(int i = 0; i < 1000; i++)
		{
			rt->exception_result = ExceptionHandler(ALLPRESET_DISTANCE);
			if(rt->exception_result == EXCEPTION_COOLDOWN)
				break;
			if(rt->exception_result == EXCEPTION_BREAK)
				break;
			usleep(1000);
		}
		rt->summary_state.has_warmup = 0;
		rt->summary_state.has_test_load = 0;
		rt->summary_state.has_workout = 0;
		rt->summary_state.has_cooldown = 0;
		rt->summary_state.did_warmup = 0;
		rt->summary_state.did_test_load = 0;
		rt->summary_state.did_workout = 0;
		rt->summary_state.did_cooldown = 0;
	}

#if 0

(FpcData.cpp 450) workout Time_elapsed=21
(FpcApp.cpp 6324) Default_CoolDown()
(FpcApp.cpp 6344) update->Time_remaining=120
(FpcApp.cpp 2962) source != target
(FpcApp.cpp 2995) OnDataScreen(), 3->3  KS_LEFT_TOP
(FpcApp.cpp 2962) source != target
(FpcData.cpp 483) CollectCooldownSummaryData(141)
(FpcApp.cpp 6143) EndofProgram(2)
(FpcApp.cpp 6282) EXCEPTION_DEFAULT
(FpcData.cpp 450) workout Time_elapsed=141
(FpcData.cpp 608) CalculateSummaryData(0, 141, 1)
(FpcData.cpp 626) rt->summary_state.has_workout == 1
(FpcData.cpp 526) CalculateCoolDownSummaryWithWorkout()
(FpcData.cpp 630) rt->cooldown.Time_elapsed=0
(FpcData.cpp 756) CalculateSummaryData(4)
(FpcData.cpp 801) CalculateSummaryData(5), Workout_time_elapsed=141
(FpcData.cpp 848) CalculateSummaryData(7), rt->cooldown.Time_elapsed=0




(FpcData.cpp 450) workout Time_elapsed=10
(FpcApp.cpp 6334) Default_CoolDown()
(FpcApp.cpp 6354) update->Time_remaining=120



rt->summary_state.did_warmup=0;
rt->summary_state.has_warmup=0;



#endif






	EndofProgram(ALLPRESET_DISTANCE);
}

void CFpcApp::Manual_Quick_Start(void)
{
//do {printf("(%s %d) DBG\n", __FILE__, __LINE__);} while(0);

	InitWorkoutPhase01(99);
	InitWorkoutPhase02_mqs(1);

	AdjustMachineWorkLoad(SEGMENT_LOAD_ADJ, 0);
	rt->exception_result = EXCEPTION_CONTINUE;


	if (0 == update->Segment_time)		update->Segment_time = 60;


	//while(rt->total_workout_time_tick > 0 && rt->exception_result == EXCEPTION_CONTINUE)
	while(rt->exception_result == EXCEPTION_CONTINUE)
	{
		//for each segment
		update->Segment_time = 60;

		// 10 * 60
 		rt->segment_time_tick = FP_TICKS_PER_SECOND * update->Segment_time;

		if(exception->cmd.hr_cruise == 0)
		{
			rt->workLoad.current_load_level = rt->workLoad_Table[rt->segment_index];
			//update->Workload_level = rt->workLoad.current_load_level;

//////////////////
// XXXXXX jason note
			//AdjustMachineWorkLoad(SEGMENT_LOAD_ADJ, 0);
		}

		// 600
		while(rt->segment_time_tick > 0 && rt->exception_result == EXCEPTION_CONTINUE)
		{
			//get realtime heartrate
			//rt->current_heart_rate = lcb->hr->DisplayHeartRate;
			rt->current_heart_rate = update->Heart_rate;


			//handling the Exceptions		
/////////////////////////////////////////////////////
// jason note
			rt->exception_result = ExceptionHandler(MANUAL_QUICK_START);

			if(rt->exception_result == EXCEPTION_COOLDOWN)				{ break; }
			if(rt->exception_result == EXCEPTION_BREAK)						{ break; }

			if(exception->cmd.pause == 0)
			{
				/////20130409/////////
				//Workout Time Update
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
				else
				{
					//InitWorkoutPhase01(99);
					rt->total_workout_time_tick = rt->total_workout_time * FP_TICKS_PER_SECOND;

					//rt->total_workout_time = setup->Workout_time_1000 * 1000 + setup->Workout_time;
					//update->Time_remaining  = setup->Workout_time;
					//update->Time_remaining_1000 = setup->Workout_time_1000;


					//rt->total_workout_time += rt->total_workout_time;
				}

				/////////////////////
				//DataScreen Time / Bar Update
				//timer elapsed 1 sec after 100 tick count;
				if(rt->tick_1sec_per100ms > 0)
					rt->tick_1sec_per100ms--;	

				if(rt->tick_1sec_per100ms == 0)
				{
					rt->tick_1sec_per100ms = rt->tick_1sec_per100ms_reload;

/////////////////////
// jason note

					Update_Workout_Time_Elapsed();
					//Update_Workout_Time_Remaining();
					update->Time_remaining_1000 = 0;
					update->Time_remaining = 1;

					//specified to Manual workout/distance/quickstart
					//AutoPopulatePace();
					//Update_GUI_bar_distance();
					Update_GUI_bar();
				}

				/////20130409/////////
				if(exception->cmd.hr_cruise == 1)
				{
					rt->watt_calc_mod = CALC_BY_CURRENT_LOAD_LEVEL;
					HR_Cruise();
				}
			}

			//timer tick will com once 100ms
			WaitForProgramTick();
		}
	}

	EndofProgram(MANUAL_QUICK_START);
}

void CFpcApp::Manual_Workout_time(void)
{
	printf("(%s %d) Manual_Workout_time()\n", __FILE__, __LINE__);

	if (0 == update->Segment_time)		update->Segment_time = 60;

	//while(rt->total_workout_time_tick > 0 && rt->exception_result == EXCEPTION_CONTINUE)
	while(rt->exception_result == EXCEPTION_CONTINUE)
	{
		//for each segment
		update->Segment_time = rt->segmentTime_Table[rt->segment_index];
		rt->segment_time_tick = FP_TICKS_PER_SECOND * update->Segment_time;

		if(exception->cmd.hr_cruise == 0)
		{
			rt->workLoad.current_load_level = rt->workLoad_Table[rt->segment_index];	
			//update->Workload_level = rt->workLoad.current_load_level;
			AdjustMachineWorkLoad(SEGMENT_LOAD_ADJ, 0);
		}
		while(rt->segment_time_tick > 0 && rt->exception_result == EXCEPTION_CONTINUE)
		{
			//get realtime heartrate
			//rt->current_heart_rate = lcb->hr->DisplayHeartRate;
			rt->current_heart_rate = update->Heart_rate;

			//handling the Exceptions		
			rt->exception_result = ExceptionHandler(MANUAL_MANUAL);
			if(rt->exception_result == EXCEPTION_COOLDOWN)
				break;
			if(rt->exception_result == EXCEPTION_BREAK)
				break;
			//end of handling the Exceptions				

			if(exception->cmd.pause == 0){
				/////20130409/////////
				//Workout Time Update
				if(rt->segment_time_tick > 0){
					rt->segment_time_tick--;
				}
				if(rt->segment_time_tick==0){
					rt->segment_index++;
				}
				if(rt->total_workout_time_tick > 0){
					rt->total_workout_time_tick --;
				}
				/////////////////////
				//DataScreen Time / Bar Update
				//timer elapsed 1 sec after 100 tick count;
				if(rt->tick_1sec_per100ms >0)
					rt->tick_1sec_per100ms--;	
				if(rt->tick_1sec_per100ms == 0){
					rt->tick_1sec_per100ms = 
						rt->tick_1sec_per100ms_reload;
					Update_Workout_Time_Elapsed();
					Update_Workout_Time_Remaining();
					
					//specified to Manual workout/distance/quickstart
					//AutoPopulatePace();
					Update_GUI_bar();
				}
				/////20130409/////////
				if(exception->cmd.hr_cruise == 1)HR_Cruise();
			}	

			//WorkOutDebugMessgae(1);
			WaitForProgramTick();//timer tick will com once 100ms
		}
	}	
}

void CFpcApp::Manual_Workout(void)
{

#if 1
	InitWorkoutPhase01_mw();
	InitWorkoutPhase02_mw(1);	
#endif

	//if(setup->Workout_distance < 159 + 1 || (unsigned short)setup->distance < 159 + 1)
	if(0 == manual_distance_flag)
	{
		Manual_Workout_time();
	}
	else
	{
		Manual_Workout_distance();	
	}

	EndofProgram(MANUAL_MANUAL);
}

void CFpcApp::Manual_Workout_distance(void)
{
	int distance_out = 0;

printf("(%s %d) Manual_Workout_distances()\n", __FILE__, __LINE__);

	//by Distance
	exception->cmd.distance_started = 1;

	//while(rt->Distance_metric < rt->target_workout_distance && rt->exception_result == EXCEPTION_CONTINUE)
	while(rt->exception_result == EXCEPTION_CONTINUE)
	{
		//for each segment
		update->Segment_time = 60;//rt->segmentTime_Table[rt->segment_index];
 		rt->segment_time_tick = FP_TICKS_PER_SECOND * update->Segment_time;
		
		if(exception->cmd.hr_cruise == 0)
		{
			rt->workLoad.current_load_level = rt->workLoad_Table[rt->segment_index];
			//update->Workload_level = rt->workLoad.current_load_level;
			AdjustMachineWorkLoad(SEGMENT_LOAD_ADJ, 0);
		}

		while(rt->segment_time_tick > 0 && rt->exception_result == EXCEPTION_CONTINUE)
		{
			//get realtime heartrate
			//rt->current_heart_rate = lcb->hr->DisplayHeartRate;
			rt->current_heart_rate = update->Heart_rate;

			//handling the Exceptions
			rt->exception_result = ExceptionHandler(MANUAL_MANUAL);
			if(rt->exception_result == EXCEPTION_COOLDOWN)
				break;
			if(rt->exception_result == EXCEPTION_BREAK)
				break;
			//end of handling the Exceptions	
			
			if(exception->cmd.pause == 0){	//break this loop;
				//real time update machine realtime data
				/////20130409/////////
				//Workout Time Update
				if(rt->segment_time_tick > 0)
				{
					rt->segment_time_tick--;
				}
				if(rt->segment_time_tick==0){
					rt->segment_index++;
					exception->cmd.segment_shift = 1;	//for Update_GUI_bar_distance()					
				}
				if(rt->total_workout_time_tick > 0){
					rt->total_workout_time_tick --;
				}
				/////////////////////
				//DataScreen Time / Bar Update
				//timer elapsed 1 sec after 100 tick count;
				if(rt->tick_1sec_per100ms >0)
					rt->tick_1sec_per100ms--;	
				if(rt->tick_1sec_per100ms == 0)
				{
					rt->tick_1sec_per100ms = rt->tick_1sec_per100ms_reload;

					Update_Workout_Time_Elapsed();

					//specified to Manual workout/distance/quickstart
					//AutoPopulatePace();
// jason note
//					Update_Workout_Time_Remaining();
					update->Time_remaining_1000 = 0;
					update->Time_remaining = 1;

					Update_GUI_bar_distance();
				}
				/////20130409/////////
				if(exception->cmd.hr_cruise == 1)
					HR_Cruise();
			}			
			//loop back
			//WorkOutDebugMessgae(1);
			WaitForProgramTick();	//timer tick will com once 100ms

			if(rt->Distance_metric >= rt->target_workout_distance)
			{
				update->Time_remaining_1000 = 0;
				update->Time_remaining = 0;
				distance_out = 1;
				goto DISTANCE_OUT;
			}
		}
	}


DISTANCE_OUT:
	if(distance_out)
	{
printf("(%s %d) DISTANCE_OUT, %s(%f %f)\n", __FILE__, __LINE__, __FUNCTION__, rt->Distance_metric, rt->target_workout_distance);

		for(int i = 0; i < 1000; i++)
		{
			rt->exception_result = ExceptionHandler(ALLPRESET_DISTANCE);
			if(rt->exception_result == EXCEPTION_COOLDOWN)
				break;
			if(rt->exception_result == EXCEPTION_BREAK)
				break;
			usleep(1000);
		}
		rt->summary_state.has_warmup = 0;
		rt->summary_state.has_test_load = 0;
		rt->summary_state.has_workout = 0;
		rt->summary_state.has_cooldown = 0;
		rt->summary_state.did_warmup = 0;
		rt->summary_state.did_test_load = 0;
		rt->summary_state.did_workout = 0;
		rt->summary_state.did_cooldown = 0;
	}

	EndofProgram(MANUAL_MANUAL);
}


#define C3LS_SEGMENT_COUNT	4
struct Cardio360_exercise c3ls_Table[C3LS_SEGMENT_COUNT] =
{
	{0,0,60,01,00},//oSTRIDE FORWARD (1:00)
	{7,0,60,01,00},//oHOLD SIDE RAILS, REVERSE STRIDE & LIFT TOES(1:00)
	{5,0,60,01,00},//oSTRIDE FORWARD & LIFT HEELS (1:00)
	{4,0,60,01,00},//oREVERSE STRIDE & BEND KNEES (1:00)
};


#define C3AS_SEGMENT_COUNT 4
struct Cardio360_exercise c3as_Table[C3AS_SEGMENT_COUNT] =
{
	{8, 0, 60, 01, 00},//oSTAND ON SIDES, CHANGE GRIP, PUSH & PULL ARMS (1:00)
	{2, 0, 30, 01, 00},//oSTAND ON SIDES LEFT ARM ONLY  (0:30)
	{8, 0, 60, 01, 00},//oSTAND ON SIDES, CHANGE GRIP, PUSH & PULL ARMS (1:00)
	{3, 0, 30, 01, 00} //oSTAND ON SIDES RIGHT ARM ONLY (0:30)
};


#define C360QS_SEGMENT_COUNT 10


/*
	unsigned char sport_mode;
	unsigned char customized_time;
	unsigned char default_segment_time;
	unsigned char work_load_class;
	unsigned char reserved;	
*/


//struct Cardio360_exercise c3qs_Table[C360QS_SEGMENT_COUNT] =
struct Cardio360_exercise c3qs_Table[] =
{
	{ 0, 0, 60, 02, 00},		// 1.	START UP, STRIDE FORWARD (1:00)                     	
	{ 1, 0, 60, 02, 00},		// 2.	STRIDE FORWARD & PUSH ARMS (1:00)                   	
	{ 2, 0, 15, 00, 00},		// 3.	STAND ON SIDES, LEFT ARM ONLY (0:15)                	
	{ 3, 0, 15, 00, 00},		// 4.	STAND ON SIDES, RIGHT ARM ONLY (0:15)               	
	{ 4, 0, 45, 01, 00},		// 5.	REVERSE STRIDE & BEND KNEES (0:45)                  	
	{ 5, 0, 30, 01, 00},		// 6.	STRIDE FORWARD & LIFT HEELS (0:30)                  	
	{ 6, 0, 60, 02, 00},		// 7.	STRIDE FORWARD & PULL ARMS(1:00)                    	
	{ 7, 0, 30, 01, 00},		// 8.	HOLD SIDE RAILS, REVERSE STRIDE & LIFT TOES (0:30)  	
	{ 8, 0, 45, 00, 00},		// 9.	STAND ON SIDES, CHANGE GRIP, PUSH & PULL ARMS (0:45)	
	{ 9, 0, 60, 02, 00}, 	// 10. RECOVER, STRIDE FORWARD (1:00)

	{ 9, 0, 60, 02, 00}, 		// 10. RECOVER, STRIDE FORWARD (1:00)
	{ 9, 0, 60, 02, 00}, 		// 10. RECOVER, STRIDE FORWARD (1:00)
	{ 9, 0, 60, 02, 00}, 		// 10. RECOVER, STRIDE FORWARD (1:00)
	{ 9, 0, 60, 02, 00}, 		// 10. RECOVER, STRIDE FORWARD (1:00)
	{ 9, 0, 60, 02, 00}, 		// 10. RECOVER, STRIDE FORWARD (1:00)
	{ 9, 0, 60, 02, 00}, 		// 10. RECOVER, STRIDE FORWARD (1:00)
	{ 9, 0, 60, 02, 00}, 		// 10. RECOVER, STRIDE FORWARD (1:00)
	{ 9, 0, 60, 02, 00}, 		// 10. RECOVER, STRIDE FORWARD (1:00)
	{ 9, 0, 60, 02, 00}, 		// 10. RECOVER, STRIDE FORWARD (1:00)
	{ 9, 0, 60, 02, 00}, 		// 10. RECOVER, STRIDE FORWARD (1:00)
	{ 9, 0, 60, 02, 00}, 		// 10. RECOVER, STRIDE FORWARD (1:00)
	{ 9, 0, 60, 02, 00}, 		// 10. RECOVER, STRIDE FORWARD (1:00)
	{ 9, 0, 60, 02, 00}, 		// 10. RECOVER, STRIDE FORWARD (1:00)
	{ 9, 0, 60, 02, 00}, 		// 10. RECOVER, STRIDE FORWARD (1:00)
	{ 9, 0, 60, 02, 00}, 		// 10. RECOVER, STRIDE FORWARD (1:00)
	{ 9, 0, 60, 02, 00}, 		// 10. RECOVER, STRIDE FORWARD (1:00)
	{ 9, 0, 60, 02, 00}, 		// 10. RECOVER, STRIDE FORWARD (1:00)
	{ 9, 0, 60, 02, 00}, 		// 10. RECOVER, STRIDE FORWARD (1:00)
	{ 9, 0, 60, 02, 00}, 		// 10. RECOVER, STRIDE FORWARD (1:00)
	{ 9, 0, 60, 02, 00}, 		// 10. RECOVER, STRIDE FORWARD (1:00)
	{ 9, 0, 60, 02, 00}, 		// 10. RECOVER, STRIDE FORWARD (1:00)
	{ 9, 0, 60, 02, 00}, 		// 10. RECOVER, STRIDE FORWARD (1:00)
	{ 9, 0, 60, 02, 00}, 		// 10. RECOVER, STRIDE FORWARD (1:00)
};


#define C360D_SEGMENT_COUNT 9
//struct Cardio360_exercise c3d_Table[C360D_SEGMENT_COUNT] =
struct Cardio360_exercise c3d_Table[] =
{
	{0 ,0 ,3  ,2 ,0},	//‧	CARDIO 360 DEMO (3 seconds)
	{6 ,0 ,30 ,2 ,0},	//‧	TOTAL BODY/STRIDE FORWARD & PULL ARMS (0:30) (4 DOTS)
	{4 ,0 ,30 ,1 ,0},	//‧	LOWER BODY/REVERSE STRIDE & BEND KNEES (0:30) (3 DOTS)
	{2 ,0 ,15 ,0 ,0},	//‧	UPPER BODY/STAND ON SIDES LEFT ARM ONLY (0:15) (2 DOTS)
	{3 ,0 ,15 ,0 ,0},	//‧	STAND ON SIDES RIGHT ARM ONLY (0:15) (2 DOTS)
	{8 ,0 ,20 ,0 ,0},	//‧	UPPER BODY/STAND ON SIDES/CHANGE GRIP/PUSH & PULL ARMS (:20) (2 DOTS)
	{5 ,0 ,20 ,1 ,0},	//‧	LOWER BODY/STRIDE FORWARD & LIFT HEELS (0:20) (3 DOTS)
	{1 ,0 ,20 ,2 ,0},	//‧	TOTAL BODY/STRIDE FORWARD & PUSH ARMS (0:20) (4 DOTS)
	{9 ,0 ,5  ,2 ,0},	//‧	DEMO COMPLETE(5 seconds)	

	{9 ,0 ,5  ,2 ,0},	//‧	DEMO COMPLETE(5 seconds)
	{9 ,0 ,5  ,2 ,0},	//‧	DEMO COMPLETE(5 seconds)
	{9 ,0 ,5  ,2 ,0},	//‧	DEMO COMPLETE(5 seconds)
	{9 ,0 ,5  ,2 ,0},	//‧	DEMO COMPLETE(5 seconds)
	{9 ,0 ,5  ,2 ,0},	//‧	DEMO COMPLETE(5 seconds)
	{9 ,0 ,5  ,2 ,0},	//‧	DEMO COMPLETE(5 seconds)
	{9 ,0 ,5  ,2 ,0},	//‧	DEMO COMPLETE(5 seconds)
	{9 ,0 ,5  ,2 ,0},	//‧	DEMO COMPLETE(5 seconds)
	{9 ,0 ,5  ,2 ,0},	//‧	DEMO COMPLETE(5 seconds)
	{9 ,0 ,5  ,2 ,0},	//‧	DEMO COMPLETE(5 seconds)
	{9 ,0 ,5  ,2 ,0},	//‧	DEMO COMPLETE(5 seconds)
	{9 ,0 ,5  ,2 ,0},	//‧	DEMO COMPLETE(5 seconds)
	{9 ,0 ,5  ,2 ,0},	//‧	DEMO COMPLETE(5 seconds)
	{9 ,0 ,5  ,2 ,0},	//‧	DEMO COMPLETE(5 seconds)
	{9 ,0 ,5  ,2 ,0},	//‧	DEMO COMPLETE(5 seconds)
	{9 ,0 ,5  ,2 ,0},	//‧	DEMO COMPLETE(5 seconds)
	{9 ,0 ,5  ,2 ,0},	//‧	DEMO COMPLETE(5 seconds)
	{9 ,0 ,5  ,2 ,0},	//‧	DEMO COMPLETE(5 seconds)
	{9 ,0 ,5  ,2 ,0},	//‧	DEMO COMPLETE(5 seconds)
	{9 ,0 ,5  ,2 ,0},	//‧	DEMO COMPLETE(5 seconds)
	{9 ,0 ,5  ,2 ,0},	//‧	DEMO COMPLETE(5 seconds)
	{9 ,0 ,5  ,2 ,0},	//‧	DEMO COMPLETE(5 seconds)
	{9 ,0 ,5  ,2 ,0},	//‧	DEMO COMPLETE(5 seconds)
	{9 ,0 ,5  ,2 ,0},	//‧	DEMO COMPLETE(5 seconds)
};

void CFpcApp::WorkoutLoopType_WLClass3(unsigned char for_program)
{
	unsigned short last_segment_time_tick;

	last_segment_time_tick = FP_TICKS_PER_SECOND * rt->segmentTime_Table[rt->total_segment - 1];
	rt->exception_result = EXCEPTION_CONTINUE;

	if (0 == update->Segment_time)
		update->Segment_time = 60;

#if 0
Cardio360_Quick_Start();
	InitWorkoutPhase01_sec(158);
	InitWorkoutPhase02_C360(C360D_SEGMENT_COUNT, CARDIO360_DEMO, c3d_Table);
#endif


	//while(rt->total_workout_time_tick > 0 && rt->exception_result == EXCEPTION_CONTINUE)
	while(rt->exception_result == EXCEPTION_CONTINUE)
	{
		update->Segment_time = rt->segmentTime_Table[rt->segment_index];
		if(update->Segment_time > 0)
		{
			rt->segment_time_tick = FP_TICKS_PER_SECOND * update->Segment_time;

			//rt->workLoad.current_load_level = rt->workLoad_Table[rt->segment_index];
			//update->Workload_level = rt->workLoad.current_load_level;

			//update->Sports_mode = rt->C360_Table[rt->random_index[rt->segment_index]].sport_mode;

			GetIndexedWorkload_C360();
			if(exception->cmd.hr_cruise == 0)
			{
				AdjustMachineWorkLoad(INDEXED_TARGET_LOAD_ADJ_WLClass3, 0);
			}
		}
		else
		{
			rt->segment_time_tick = 0;
		}
		if(rt->segment_time_tick == 0)
			rt->segment_index++;
		while(rt->segment_time_tick > 0 && rt->exception_result == EXCEPTION_CONTINUE)
		{		
			//handling the Exceptions		
			rt->exception_result = ExceptionHandler(for_program);
			if(rt->exception_result == EXCEPTION_COOLDOWN)
			{
				update->Sports_mode = 0x09;	//mode= 0~9 
				break;
			}
			if(rt->exception_result == EXCEPTION_BREAK)
				break;
			//end of handling the Exceptions	

			if(exception->cmd.pause == 0)
			{
				if(rt->segment_time_tick > 0)
					rt->segment_time_tick--;
				if(rt->segment_time_tick == 0)
					rt->segment_index++;
				if(rt->total_workout_time_tick > 0)
					rt->total_workout_time_tick --;

				/////////////////////
				//DataScreen Time / Bar Update
				//timer elapsed 1 sec after 100 tick count;
				if(rt->tick_1sec_per100ms > 0)
					rt->tick_1sec_per100ms--;
				if(rt->tick_1sec_per100ms == 0)
				{
					rt->tick_1sec_per100ms = rt->tick_1sec_per100ms_reload;
					Update_Workout_Time_Elapsed();
					Update_Workout_Time_Remaining();
					Update_GUI_bar();
				}
				/////20130409/////////
				if(exception->cmd.hr_cruise == 1)
					HR_Cruise();
				
				//specific to Cardio360
				if(rt->segment_index < rt->total_segment - 1)
				{
					if(rt->total_workout_time_tick <= last_segment_time_tick)
					{
						rt->segment_time_tick = 0;
						rt->segment_index++;
						break;
					}
				}				
			}			
			WaitForProgramTick();//timer tick will com once 100ms
		}		
		if(rt->segment_index == rt->total_segment - 1)
		{
			if(rt->total_workout_time_tick > last_segment_time_tick)
				rt->segment_index = 1;
		}
	}

	EndofProgram(for_program);
}

void CFpcApp::Cardio360_Demo(void)
{
	InitWorkoutPhase01_sec(158);
	InitWorkoutPhase02_C360(C360D_SEGMENT_COUNT, CARDIO360_DEMO, c3d_Table);

	WorkoutLoopType_WLClass3(CARDIO360_DEMO);
}

void CFpcApp::Cardio360_Quick_Start(void)
{
	InitWorkoutPhase01(20);
	InitWorkoutPhase02_C360(C360QS_SEGMENT_COUNT, CARDIO360_QUICK_START, c3qs_Table);
	WorkoutLoopType_WLClass3(CARDIO360_QUICK_START);
}

void CFpcApp::Cardio360_Video(void)
{
	InitWorkoutPhase01(20);
	InitWorkoutPhase02_C360V(10);//apas_DefaultWorkload_Table);

	rt->watt_calc_mod = CALC_BY_CURRENT_LOAD_LEVEL;
	rt->load_adj_mode = SEGMENT_LOAD_ADJ;

	if (0 == update->Segment_time)		update->Segment_time = 60;

	rt->exception_result = EXCEPTION_CONTINUE;
	while(rt->total_workout_time_tick > 0 && rt->exception_result == EXCEPTION_CONTINUE)
	{
		update->Segment_time = rt->segmentTime_Table[rt->segment_index];

		if(update->Segment_time > 0)
		{
			rt->segment_time_tick = FP_TICKS_PER_SECOND * update->Segment_time;
			update->Workload_level = rt->workLoad.current_load_level;
			if(exception->cmd.hr_cruise == 0)
			{
				AdjustMachineWorkLoad(SEGMENT_LOAD_ADJ, 0);
			}
		}
		else
		{
			rt->segment_time_tick = 0;
		}

		
		if(rt->segment_time_tick == 0)		rt->segment_index++;	

		while(rt->segment_time_tick > 0 && rt->exception_result == EXCEPTION_CONTINUE)
		{
			//handling the Exceptions		
			rt->exception_result = ExceptionHandler(CARDIO360_VIDEO);
			if(rt->exception_result == EXCEPTION_COOLDOWN){
				break;
			}
			if(rt->exception_result == EXCEPTION_BREAK)		break;
			//end of handling the Exceptions	
			if(exception->cmd.pause == 0)
			{
				/////20130409/////////
				//Workout Time Update
				if(rt->segment_time_tick > 0)
				{
					rt->segment_time_tick--;
				}
				if(rt->segment_time_tick==0)
				{
					rt->segment_index++;
				}
				if(rt->total_workout_time_tick > 0)
				{
					rt->total_workout_time_tick --;
				}
				/////////////////////
				//DataScreen Time / Bar Update
				//timer elapsed 1 sec after 100 tick count;
				if(rt->tick_1sec_per100ms >0)
					rt->tick_1sec_per100ms--;	
				if(rt->tick_1sec_per100ms == 0)
				{
					rt->tick_1sec_per100ms = rt->tick_1sec_per100ms_reload;
					Update_Workout_Time_Elapsed();
					Update_Workout_Time_Remaining();
					Update_GUI_bar();
				}
				/////20130409/////////
				if(exception->cmd.hr_cruise == 1)HR_Cruise();
			}

			WaitForProgramTick();//timer tick will com once 100ms
		}
	}

	EndofProgram(CARDIO360_VIDEO);
}

void CFpcApp::Cardio360_Arm_Sculptor(void)
{
	InitWorkoutPhase01(10);
	InitWorkoutPhase02_C360_V1(C3AS_SEGMENT_COUNT, CARDIO360_ARM_SCULPTOR, c3as_Table); //in Cardio360_Quick_Start.c


printf("(%s %d) Cardio360_Arm_Sculptor()\n", __FILE__, __LINE__);
	WorkoutLoopType_WLClass3_V1(CARDIO360_ARM_SCULPTOR);
}

void CFpcApp::WorkoutLoopType_WLClass3_V1(unsigned char for_program)
{
	//unsigned short last_segment_time_tick;
	//last_segment_time_tick = FP_TICKS_PER_SECOND * rt->segmentTime_Table[rt->total_segment-1];
	//exception->cmd.work_load_up = 1;

	if (0 == update->Segment_time)		update->Segment_time = 60;


	rt->exception_result = EXCEPTION_CONTINUE;
	//while(rt->total_workout_time_tick > 0 && rt->exception_result == EXCEPTION_CONTINUE)
	while(rt->exception_result == EXCEPTION_CONTINUE)
	{
		if (rt->segmentTime_Table[rt->segment_index] > 0)
			update->Segment_time = rt->segmentTime_Table[rt->segment_index];
		else
			update->Segment_time = 60;

		if(update->Segment_time > 0)
		{
			rt->segment_time_tick = FP_TICKS_PER_SECOND * update->Segment_time;

			rt->workLoad.current_load_level = rt->workLoad_Table[rt->segment_index];

			update->Sports_mode =  rt->C360_Table[rt->random_index[rt->segment_index]].sport_mode;

			//GetIndexedWorkload_C360();
			if(exception->cmd.hr_cruise == 0)
			{
				AdjustMachineWorkLoad(SEGMENT_LOAD_ADJ, 0);
			}
		}
		else
		{
			rt->segment_time_tick = 0;
		}
		if(rt->segment_time_tick == 0)
			rt->segment_index++;	

		//begin of each segment
		while(rt->segment_time_tick > 0 && rt->exception_result == EXCEPTION_CONTINUE)
		{		
			//handling the Exceptions		
			rt->exception_result = ExceptionHandler(for_program);
			if(rt->exception_result == EXCEPTION_COOLDOWN)
			{
				update->Sports_mode = 0x09;	//mode= 0~9 
				break;
			}
			if(rt->exception_result == EXCEPTION_BREAK)
				break;
			//end of handling the Exceptions	
			
			if(exception->cmd.pause == 0){
				/////20130409/////////
				//Workout Time Update
				if(rt->segment_time_tick > 0){
					rt->segment_time_tick--;
				}
				if(rt->segment_time_tick==0){
					rt->segment_index++;
				}				
				if(rt->total_workout_time_tick > 0){
					rt->total_workout_time_tick --;
				}
				/////////////////////
				//DataScreen Time / Bar Update
				//timer elapsed 1 sec after 100 tick count;
				if(rt->tick_1sec_per100ms >0)
					rt->tick_1sec_per100ms--;	
				if(rt->tick_1sec_per100ms == 0)
				{
					rt->tick_1sec_per100ms = 
						rt->tick_1sec_per100ms_reload;
					Update_Workout_Time_Elapsed();
					Update_Workout_Time_Remaining();
					Update_GUI_bar();
				}
				/////20130409/////////
				if(exception->cmd.hr_cruise == 1)HR_Cruise();
			}			
			WaitForProgramTick();//timer tick will com once 100ms
		}		
		if(rt->segment_index == rt->total_segment)
		{
			//if(rt->total_workout_time_tick > last_segment_time_tick) 
				rt->segment_index = 0;
		}
	}


	//for cool down display
	update->Sports_mode = 0x09;	//mode= 0~9 
	EndofProgram(for_program);
}

void CFpcApp::Cardio360_Leg_Shaper(void)
{

#if 1
	InitWorkoutPhase01(20);
	InitWorkoutPhase02_C360_V1(C3LS_SEGMENT_COUNT,CARDIO360_LEG_SHAPER, c3ls_Table);
#endif

	WorkoutLoopType_WLClass3_V1(CARDIO360_LEG_SHAPER);
}

void CFpcApp::Cardio360_Customized(void)
{

#if 1
	InitWorkoutPhase01(20);
	InitWorkoutPhase02_C360(C360QS_SEGMENT_COUNT, CARDIO360_CUSTOMIZED, c3qs_Table);
#endif



	WorkoutLoopType_WLClass3(CARDIO360_CUSTOMIZED);
}

void CFpcApp::WorkoutLoopType1(unsigned char for_program, unsigned char load_adj, unsigned char watt_calc_mod)
{
	rt->load_adj_mode = load_adj;
	rt->watt_calc_mod = watt_calc_mod;
	rt->exception_result = EXCEPTION_CONTINUE;	

	if (0 == update->Segment_time)		update->Segment_time = 60;


	//while(rt->total_workout_time_tick > 0 && rt->exception_result == EXCEPTION_CONTINUE)
	while(rt->exception_result == EXCEPTION_CONTINUE)
	{
		//for each segment
		update->Segment_time = rt->segmentTime_Table[rt->segment_index];
		if(update->Segment_time > 0)
		{
			rt->segment_time_tick = FP_TICKS_PER_SECOND * update->Segment_time;
			if(exception->cmd.hr_cruise == 0)
			{
				//rt->workPace_Table
				rt->workLoad.current_load_level = rt->workLoad_Table[rt->segment_index];
				update->Workload_level = rt->workLoad.current_load_level;
				AdjustMachineWorkLoad(load_adj, 0);
			}
		}
		while(rt->segment_time_tick > 0 && rt->exception_result == EXCEPTION_CONTINUE)
		{
			//get realtime heartrate
			//rt->current_heart_rate = lcb->hr->DisplayHeartRate;
			rt->current_heart_rate = update->Heart_rate;

			//handling the Exceptions		
			rt->exception_result = ExceptionHandler(for_program);
			//end of handling the Exceptions			
			if(rt->exception_result == EXCEPTION_COOLDOWN)
				break;
			if(rt->exception_result == EXCEPTION_BREAK)
				break;
			//end of handling the Exceptions
			
			if(exception->cmd.pause == 0){
				/////20130409/////////
				//Workout Time Update
				if(rt->segment_time_tick > 0){
					rt->segment_time_tick--;
				}
				if(rt->segment_time_tick==0){
					rt->segment_index++;
				}
				if(rt->total_workout_time_tick > 0){
					rt->total_workout_time_tick --;
				}
				/////////////////////
				//DataScreen Time / Bar Update
				//timer elapsed 1 sec after 100 tick count;
				if(rt->tick_1sec_per100ms >0)
					rt->tick_1sec_per100ms--;	
				if(rt->tick_1sec_per100ms == 0)
				{
					rt->tick_1sec_per100ms = rt->tick_1sec_per100ms_reload;
					Update_Workout_Time_Elapsed();
					Update_Workout_Time_Remaining();

					UpdateCoolDownPace(for_program);

					Update_GUI_bar();
				}
				/////20130409/////////
				if(exception->cmd.hr_cruise == 1)
					HR_Cruise();
			}
			WaitForProgramTick();//timer tick will com once 100ms
		}
	}
	EndofProgram(for_program);
}

void CFpcApp::Weight_Loss_Calorie_Goal(void) 
{

#if 1
	InitWorkoutPhase01(20);
	InitWorkoutPhase02_wlcg(1);
#endif

	WorkoutLoopType1(WEIGHT_LOSS_CALORIE_GOAL, SEGMENT_LOAD_ADJ, CALC_BY_CURRENT_LOAD_LEVEL);
}

struct WLClass20_Load weight_loss_rolling_hill_LevelTable[]=
{
	{{1,1, 4, 7, 8, 7, 4, 7, 8, 7, 4, 7, 8, 7, 4, 7, 8, 7, 4,1}},
	{{1,1, 4, 7, 9, 7, 4, 7, 9, 7, 4, 7, 9, 7, 4, 7, 9, 7, 4,1}},
	{{1,1, 4, 7,10, 7, 4, 7,10, 7, 4, 7,10, 7, 4, 7,10, 7, 4,1}},
	{{1,1, 5, 7,10, 7, 5, 7,10, 7, 5, 7,10, 7, 5, 7,10, 7, 5,1}},
	{{4,4, 7, 8,11, 8, 7, 8,11, 8, 7, 8,11, 8, 7, 8,11, 8, 7,4}},
	{{4,4, 7, 8,11, 8, 7, 8,11, 8, 7, 8,11, 8, 7, 8,11, 8, 7,4}},
	{{4,4, 8,10,12,10, 8,10,12,10, 8,10,12,10, 8,10,12,10, 8,4}},
	{{4,4, 8,10,12,10, 8,10,12,10, 8,10,12,10, 8,10,12,10, 8,4}},
	{{5,5, 9,11,13,11, 9,11,13,11, 9,11,13,11, 9,11,13,11, 9,5}},
	{{5,5,10,11,13,11,10,11,13,11,10,11,13,11,10,11,13,11,10,5}},
	{{5,5, 9,11,14,11, 9,11,14,11, 9,11,14,11, 9,11,14,11, 9,5}},
	{{5,5,10,12,14,12,10,12,14,12,10,12,14,12,10,12,14,12,10,5}},
	{{7,7,10,12,14,12,10,12,14,12,10,12,14,12,10,12,14,12,10,7}},
	{{7,7,10,13,15,13,10,13,15,13,10,13,15,13,10,13,15,13,10,7}},
	{{7,7,11,13,15,13,11,13,15,13,11,13,15,13,11,13,15,13,11,7}},
	{{7,7,11,14,15,14,11,14,15,14,11,14,15,14,11,14,15,14,11,7}},
	{{8,8,11,14,15,14,11,14,15,14,11,14,15,14,11,14,15,14,11,8}},
	{{8,8,11,14,16,14,11,14,16,14,11,14,16,14,11,14,16,14,11,8}},
	{{8,8,12,15,16,15,12,15,16,15,12,15,16,15,12,15,16,15,12,8}},
	{{8,8,12,15,16,15,12,15,16,15,12,15,16,15,12,15,16,15,12,8}}
};

void CFpcApp::Weight_Loss_Rolling_Hills(void)
{

#if 1
	InitWorkoutPhase01(20);
	InitWorkoutPhase02_wlrh(20, weight_loss_rolling_hill_LevelTable);//(20 segment, no timing table, indexed workload table)
#endif

	WorkoutLoopType_WLClass20(WEIGHT_LOSS_ROLLING_HILLS);
}

void CFpcApp::WorkoutLoopType_WLClass20(unsigned char for_program)
{
	rt->exception_result = EXCEPTION_CONTINUE;	

	if (0 == update->Segment_time)		update->Segment_time = 60;


	//while(rt->total_workout_time_tick > 0 && rt->exception_result == EXCEPTION_CONTINUE)
	while(rt->exception_result == EXCEPTION_CONTINUE)
	{
		//for each segment
		update->Segment_time = rt->segmentTime_Table[rt->segment_index];
		if(update->Segment_time >0)
		{
			rt->segment_time_tick = FP_TICKS_PER_SECOND * update->Segment_time;
			//GetIndexedWorkload_WLClass20_single();
			if(exception->cmd.hr_cruise == 0)
			{
				AdjustMachineWorkLoad(INDEXED_TARGET_LOAD_ADJ_WLClass20, 0);
			}
		}

		while(rt->segment_time_tick > 0 && rt->exception_result == EXCEPTION_CONTINUE)
		{
			//get realtime heartrate
			//rt->current_heart_rate = lcb->hr->DisplayHeartRate;
			rt->current_heart_rate = update->Heart_rate;

			//handling the Exceptions		
			rt->exception_result = ExceptionHandler(for_program);
			//end of handling the Exceptions			
			if(rt->exception_result == EXCEPTION_COOLDOWN)
				break;
			if(rt->exception_result == EXCEPTION_BREAK)
				break;
			//end of handling the Exceptions
			
			if(exception->cmd.pause == 0)
			{
				/////20130409/////////
				//Workout Time Update
				if(rt->segment_time_tick > 0)
				{
					rt->segment_time_tick--;
				}
				if(rt->segment_time_tick==0)
				{
					rt->segment_index++;
				}
				if(rt->total_workout_time_tick > 0)
				{
					rt->total_workout_time_tick --;
				}
				/////////////////////
				//DataScreen Time / Bar Update
				//timer elapsed 1 sec after 100 tick count;
				if(rt->tick_1sec_per100ms >0)
					rt->tick_1sec_per100ms--;	
				if(rt->tick_1sec_per100ms == 0)
				{
					rt->workPace_Table[rt->workLoad_TableUsed] = update->Pace_RPM;

					rt->tick_1sec_per100ms = rt->tick_1sec_per100ms_reload;
					Update_Workout_Time_Elapsed();
					Update_Workout_Time_Remaining();
					Update_GUI_bar();
				}
				/////20130409/////////
				if(exception->cmd.hr_cruise == 1)
					HR_Cruise();
			}
			WaitForProgramTick();//timer tick will com once 100ms
		}

	

		printf("rt->exception_result = %d\n",rt->exception_result);
	}
	EndofProgram(for_program);	
}

struct BiPace weight_loss_walk_and_run_PaceTable[]=//wlwarPaceTable[]=
{
	//Walk Run Chart
	//WALK PACE RUN PACE	
	{50, 60},
	{50, 65},
	{50, 68},
	{50, 70},
	{50, 73},
	{55, 75},
	{55, 78},
	{55, 80},
	{55, 83},
	{55, 85},
	{60, 88},
	{60, 90},
	{60, 93},
	{60, 95},
	{60, 98},
	{65,100},
	{65,103},
	{65,105},
	{65,108},
	{65,110}
};

void CFpcApp::Weight_Loss_Walk_and_Run(void)
{

#if 1
	InitWorkoutPhase01(20);
	InitWorkoutPhase02_segTime_pace(1, weight_loss_walk_and_run_PaceTable);
#endif

	WorkoutLoopType1(WEIGHT_LOSS_WALK_AND_RUN, SEGMENT_LOAD_ADJ, CALC_BY_CURRENT_LOAD_LEVEL);
}

void CFpcApp::Hrc_Target(void)
{      
	WorkoutLoopType_HRC(HRC_TARGET, SEGMENT_LOAD_ADJ, CALC_BY_CURRENT_LOAD_LEVEL);
}

void CFpcApp::WorkoutLoopType_HRC(unsigned char for_program, unsigned char load_adj, unsigned char watt_calc_mod)
{
	InitWorkoutPhase01(20);
	InitWorkoutPhase02_1pre_segTime(1);
	if (0 == setup->Target_heart_rate)
		setup->Target_heart_rate = 180;
	update->Target_heart_rate = setup->Target_heart_rate;


	//FPC machine state/mode
	rt->watt_calc_mod = watt_calc_mod;
	rt->load_adj_mode = load_adj;

	rt->workout_mode = MANUAL_MODE;	//start with warm up mode /manual mode
	rt->segment_index = 0;
	AdjustMachineWorkLoad(SEGMENT_LOAD_ADJ, 0);

	if (0 == update->Segment_time)		update->Segment_time = 60;

	rt->exception_result = EXCEPTION_CONTINUE;	

	//while(rt->total_workout_time_tick > 0 && rt->exception_result == EXCEPTION_CONTINUE)
	while(rt->exception_result == EXCEPTION_CONTINUE)
	{
		update->Segment_time = rt->segmentTime_Table[rt->segment_index];

		if(update->Segment_time > 0)
		{
			rt->segment_time_tick = FP_TICKS_PER_SECOND * update->Segment_time;

			rt->workLoad.current_load_level = rt->workLoad_Table[rt->segment_index];
			update->Workload_level = rt->workLoad.current_load_level;
		}

/*
		//for each segment
		update->Segment_time = rt->segmentTime_Table[rt->segment_index];
 		rt->segment_time_tick = FP_TICKS_PER_SECOND * update->Segment_time;

		//rt->workLoad.current_load_level = rt->workLoad_Table[rt->segment_index];
		//update->Workload_level = rt->workLoad.current_load_level;
		rt->workLoad.current_load_level = rt->workLoad_Table[rt->segment_index];
		setup->Workload_level = update->Workload_level = rt->workLoad.current_load_level;
*/

		//if(rt->segment_index > 1)
		if(rt->segment_index > 0)
		{
			if(rt->workout_mode == PROGRESS_MODE || rt->workout_mode == RESUME_MODE)
			{
				AdjustMachineWorkLoad(SEGMENT_LOAD_ADJ, 0);
			}				
		}

		if(update->Segment_time == 0)
			rt->segment_index++;


		while(rt->segment_time_tick > 0 && rt->exception_result == EXCEPTION_CONTINUE)
		{
			//get realtime heartrate
			//rt->current_heart_rate = lcb->hr->DisplayHeartRate;
			rt->current_heart_rate = update->Heart_rate;

			//handling the Exceptions		
			rt->exception_result = ExceptionHandler(for_program);
			//end of handling the Exceptions			
			if(rt->exception_result == EXCEPTION_COOLDOWN)
				break;
			if(rt->exception_result == EXCEPTION_BREAK)
				break;
			//end of handling the Exceptions

			if (rt->workout_mode != RESUME_MODE)
			{
				if(rt->current_heart_rate >= setup->Target_heart_rate)
				{
					rt->deltaHR = rt->current_heart_rate - setup->Target_heart_rate;
					if(rt->deltaHR > 12)
					{
						rt->exception_result = EXCEPTION_OVERHR_BREAK;
					}
				}
			}

			switch(rt->workout_mode)
			{
			case RESUME_MODE:
				rt->watt_calc_mod = CALC_BY_CURRENT_LOAD_LEVEL;
				rt->load_adj_mode = SEGMENT_LOAD_ADJ;

				if(exception->cmd.pause != 0)
					break;
				if (resume_mod_flag)
				{
					resume_mod_flag = 0;
					for(int i = rt->segment_index; i < rt->workLoad_TableUsed; i++)
					{
						rt->workLoad_Table[i] = update->Workload_level;
						rt->workPace_Table[i] = 0;
					}
				}
				if(rt->segment_time_tick > 0)
					rt->segment_time_tick--;
				if(rt->segment_time_tick==0)
					rt->segment_index++;
				if(rt->total_workout_time_tick > 0)
					rt->total_workout_time_tick--;
				if(rt->tick_1sec_per100ms >0)
					rt->tick_1sec_per100ms--;	
				if(rt->tick_1sec_per100ms == 0)
				{
					rt->workLoad.current_load_level = rt->workLoad_Table[rt->segment_index];
					setup->Workload_level = update->Workload_level = rt->workLoad.current_load_level;

					rt->tick_1sec_per100ms = rt->tick_1sec_per100ms_reload;
					Update_Workout_Time_Elapsed();
					Update_Workout_Time_Remaining();
					Update_GUI_bar();
				}
				break;

			case MANUAL_MODE:
				rt->watt_calc_mod = CALC_BY_CURRENT_LOAD_LEVEL;
				rt->load_adj_mode = SEGMENT_LOAD_ADJ;

				//WARM UP mode tring to approach to target hr
				rt->summary_state.did_warmup = 1;
				if(rt->current_heart_rate == 0)
				{
					goto MMT;
					//break;
				}
				if(rt->current_heart_rate > setup->Target_heart_rate)
				{
					rt->deltaHR = rt->current_heart_rate - setup->Target_heart_rate;
					if(rt->deltaHR < 12)
					{
						CollectWarmUpSummaryData();
						//rt->has_warmup = 1;	
						rt->workout_mode = PROGRESS_MODE;
						rt->exception_result = EXCEPTION_STAGE_ADVANCE;
						rt->tick_1sec_per100ms = rt->tick_1sec_per100ms_reload;
						rt->tick_30sec = rt->tick_30sec_reload;
						rt->segment_index ++;
					}
					else
					{
						rt->exception_result = EXCEPTION_OVERHR_BREAK;
					}
				}
				else
				{
					rt->deltaHR = setup->Target_heart_rate - rt->current_heart_rate;
					if(rt->deltaHR < 10)
					{
						CollectWarmUpSummaryData();
						//rt->has_warmup = 1;	
						rt->workout_mode = PROGRESS_MODE;
						rt->exception_result = EXCEPTION_STAGE_ADVANCE;
						rt->tick_1sec_per100ms = rt->tick_1sec_per100ms_reload;
						rt->tick_30sec = rt->tick_30sec_reload;
						rt->segment_index ++;
					}
					else
					{
						rt->tick_30sec --;	
						if(rt->tick_30sec==0)
						{
							rt->tick_30sec = rt->tick_30sec_reload;
							//if(rt->current_heart_rate < setup->Target_heart_rate){							
							//	calc_level = GetHiger10WattWorkLoad();
							//}
							//rt->adjusted_Target_Workload = calc_level;
							//PopulateWorkLoad();
							//AdjustMachineWorkLoad(SEGMENT_LOAD_ADJ,0);
							//Update_GUI_bar();			
						}					
					}
				}	
				//////////////////////////////////////////////////////
				//DataScreen Time / Bar Update
				//timer elapsed 1 sec after 100 tick count;
MMT:
				if(rt->tick_1sec_per100ms > 0)
					rt->tick_1sec_per100ms--;	
				if(rt->tick_1sec_per100ms == 0)
				{
					rt->tick_1sec_per100ms = rt->tick_1sec_per100ms_reload;
					Update_Workout_Time_Elapsed();
					//if (resume_mod_flag)						Update_Workout_Time_Remaining();
					Update_GUI_bar();
				}
				//////////////////////////////////////////////////////					
				break;
		
			case PROGRESS_MODE:
//printf("(%s %d) PROGRESS_MODE\n", __FILE__, __LINE__);
				rt->summary_state.did_workout = 1;
				if(rt->current_heart_rate > setup->Target_heart_rate)
				{
					rt->deltaHR = rt->current_heart_rate - setup->Target_heart_rate;
					if(rt->deltaHR > 12)
					{
						rt->exception_result = EXCEPTION_OVERHR_BREAK;	
					}
				}
				else
				{
					rt->deltaHR = setup->Target_heart_rate - rt->current_heart_rate;
				}
				if(exception->cmd.pause == 0)
				{
					/////20130409/////////
					//Workout Time Update
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
					/////////////////////
					//DataScreen Time / Bar Update
					//timer elapsed 1 sec after 100 tick count;
					if(rt->tick_1sec_per100ms >0)
						rt->tick_1sec_per100ms--;	
					if(rt->tick_1sec_per100ms == 0)
					{
						rt->tick_1sec_per100ms = 
							rt->tick_1sec_per100ms_reload;
						Update_Workout_Time_Elapsed();
						Update_Workout_Time_Remaining();
						//AutoPopulatePace();
						Update_GUI_bar();
					}
					/////20130409/////////
					if(exception->cmd.hr_cruise == 1)
					{
						HR_Cruise();
					}
					else
					{
						HRC_conditioning();
					}
				}

				if (resume_mod_flag)
				{
					resume_mod_flag = 0;
					rt->workout_mode = RESUME_MODE;
					for(int i = rt->segment_index; i < rt->workLoad_TableUsed; i++)
					{
						rt->workLoad_Table[i] = update->Workload_level;
						rt->workPace_Table[i] = 0;
					}
				}
				break;
				
			default:
printf("(%s %d) ERR\n", __FILE__, __LINE__);
				//error handling
				break;
			}	

			WaitForProgramTick();//timer tick will com once 100ms

			if(rt->exception_result == EXCEPTION_STAGE_ADVANCE)
			{
				rt->exception_result = EXCEPTION_CONTINUE;
				break;
			}
		}
	}	

	EndofProgram(for_program);
}

void CFpcApp::Hrc_Weight_Loss(void)
{

#if 1
	InitWorkoutPhase01(20);
	InitWorkoutPhase02_1pre_segTime(1);
	if (0 == setup->Target_heart_rate)
		setup->Target_heart_rate = 180;
	update->Target_heart_rate = setup->Target_heart_rate;
#endif

	//FPC machine state/mode
	rt->watt_calc_mod = CALC_BY_CURRENT_LOAD_LEVEL;
	rt->load_adj_mode = SEGMENT_LOAD_ADJ;
	rt->workout_mode = MANUAL_MODE;	//start with warm up mode /manual mode
	AdjustMachineWorkLoad(SEGMENT_LOAD_ADJ,0);
	rt->exception_result = EXCEPTION_CONTINUE;	

	if (0 == update->Segment_time)		update->Segment_time = 60;

	//while(rt->total_workout_time_tick > 0 && rt->exception_result == EXCEPTION_CONTINUE)
	while(rt->exception_result == EXCEPTION_CONTINUE)
	{
		update->Segment_time = rt->segmentTime_Table[rt->segment_index];

		if(update->Segment_time > 0)
		{
			rt->segment_time_tick = FP_TICKS_PER_SECOND * update->Segment_time;

			rt->workLoad.current_load_level = rt->workLoad_Table[rt->segment_index];
			update->Workload_level = rt->workLoad.current_load_level;
		}

/*
		//for each segment
		update->Segment_time = rt->segmentTime_Table[rt->segment_index];

 		rt->segment_time_tick = FP_TICKS_PER_SECOND * update->Segment_time;

		//rt->workLoad.current_load_level = rt->workLoad_Table[rt->segment_index];
		//update->Workload_level = rt->workLoad.current_load_level;
		rt->workLoad.current_load_level = rt->workLoad_Table[rt->segment_index];
		setup->Workload_level = update->Workload_level = rt->workLoad.current_load_level;
*/

		//if(rt->segment_index > 1)
		if(rt->segment_index > 0)
		{
			if(rt->workout_mode == PROGRESS_MODE || rt->workout_mode == RESUME_MODE)
			{
				AdjustMachineWorkLoad(SEGMENT_LOAD_ADJ, 0);
			}				
		}

		if(update->Segment_time == 0)
			rt->segment_index++;

		while(rt->segment_time_tick > 0 && rt->exception_result == EXCEPTION_CONTINUE)
		{			
			//get realtime heartrate
			//rt->current_heart_rate = lcb->hr->DisplayHeartRate;
			rt->current_heart_rate = update->Heart_rate;

			//handling the Exceptions		
			rt->exception_result = ExceptionHandler(HRC_WEIGHT_LOSS);
			//end of handling the Exceptions			
			if(rt->exception_result == EXCEPTION_COOLDOWN)
				break;
			if(rt->exception_result == EXCEPTION_BREAK)
				break;
			//end of handling the Exceptions

			if (rt->workout_mode != RESUME_MODE)
			{
				if(rt->current_heart_rate >= setup->Target_heart_rate)
				{
					rt->deltaHR = rt->current_heart_rate - setup->Target_heart_rate;
					if(rt->deltaHR > 12)
					{
						printf("(%s %d) EXCEPTION_OVERHR_BREAK\n", __FILE__, __LINE__);
						rt->exception_result = EXCEPTION_OVERHR_BREAK;
					}
				}
			}

			switch(rt->workout_mode)
			{
			case RESUME_MODE:
				rt->watt_calc_mod = CALC_BY_CURRENT_LOAD_LEVEL;
				rt->load_adj_mode = SEGMENT_LOAD_ADJ;

				if(exception->cmd.pause != 0)
					break;
				if (resume_mod_flag)
				{
					resume_mod_flag = 0;
					for(int i = rt->segment_index; i < rt->workLoad_TableUsed; i++)
					{
						rt->workLoad_Table[i] = update->Workload_level;
						rt->workPace_Table[i] = 0;
					}
				}
				if(rt->segment_time_tick > 0)
					rt->segment_time_tick--;
				if(rt->segment_time_tick==0)
					rt->segment_index++;
				if(rt->total_workout_time_tick > 0)
					rt->total_workout_time_tick--;
				if(rt->tick_1sec_per100ms >0)
					rt->tick_1sec_per100ms--;	
				if(rt->tick_1sec_per100ms == 0)
				{
					rt->workLoad.current_load_level = rt->workLoad_Table[rt->segment_index];
					setup->Workload_level = update->Workload_level = rt->workLoad.current_load_level;

					rt->tick_1sec_per100ms = rt->tick_1sec_per100ms_reload;
					Update_Workout_Time_Elapsed();
					Update_Workout_Time_Remaining();
					Update_GUI_bar();
				}
				break;

			case MANUAL_MODE:
				rt->watt_calc_mod = CALC_BY_CURRENT_LOAD_LEVEL;
				rt->load_adj_mode = SEGMENT_LOAD_ADJ;

				//WARM UP mode tring to approach to target hr
				rt->summary_state.did_warmup = 1;
			
				if(rt->current_heart_rate == 0)
				{
					goto MMT;
					//break;
				}
				if(rt->current_heart_rate > setup->Target_heart_rate)
				{
					rt->deltaHR = rt->current_heart_rate - setup->Target_heart_rate;
					if(rt->deltaHR < 12)
					{
						CollectWarmUpSummaryData();
						//rt->has_warmup = 1;	
						rt->workout_mode = PROGRESS_MODE;
						rt->exception_result = EXCEPTION_STAGE_ADVANCE;
						rt->tick_30sec = rt->tick_30sec_reload;
						rt->segment_index ++;
						rt->tick_1sec_per100ms = rt->tick_1sec_per100ms_reload;
					}
					else
					{
						rt->exception_result = EXCEPTION_OVERHR_BREAK;	
					}
				}
				else
				{
					rt->deltaHR = setup->Target_heart_rate - rt->current_heart_rate;
					if(rt->deltaHR < 10){
						CollectWarmUpSummaryData();
						//rt->has_warmup = 1;	
						rt->workout_mode = PROGRESS_MODE;
						rt->exception_result = EXCEPTION_STAGE_ADVANCE;
						rt->tick_30sec = rt->tick_30sec_reload;
						rt->segment_index ++;
						rt->tick_1sec_per100ms = rt->tick_1sec_per100ms_reload;

					}else{
						//rt->tick_30sec --;	
						//if(rt->tick_30sec==0){
						//	rt->tick_30sec = rt->tick_30sec_reload;
						//	if(rt->current_heart_rate < setup->Target_heart_rate){							
						//		calc_level = GetHiger10WattWorkLoad();
						//	}
						//	rt->adjusted_Target_Workload = calc_level;
						//	PopulateWorkLoad();
						//	AdjustMachineWorkLoad(SEGMENT_LOAD_ADJ,0);
						//	Update_GUI_bar();			
						//}					
					}
				}	
				//////////////////////////////////////////////////////
				//DataScreen Time / Bar Update
				//timer elapsed 1 sec after 100 tick count;
MMT:
				if(rt->tick_1sec_per100ms >0)
					rt->tick_1sec_per100ms--;	
				if(rt->tick_1sec_per100ms == 0)
				{
					rt->tick_1sec_per100ms = rt->tick_1sec_per100ms_reload;
					Update_Workout_Time_Elapsed();
					Update_GUI_bar();
				}
				//////////////////////////////////////////////////////				
				break;
		
			case PROGRESS_MODE:
				rt->summary_state.did_workout = 1;
				if(rt->current_heart_rate > setup->Target_heart_rate)
				{
					rt->deltaHR = rt->current_heart_rate - setup->Target_heart_rate;
					if(rt->deltaHR > 12)
					{
						rt->exception_result = EXCEPTION_OVERHR_BREAK;	
					}
				}
				else
				{
					rt->deltaHR = setup->Target_heart_rate - rt->current_heart_rate;
				}
				if(exception->cmd.pause == 0)
				{
					/////20130409/////////
					//Workout Time Update
					if(rt->segment_time_tick > 0)
					{
						rt->segment_time_tick--;
					}
					if(rt->segment_time_tick==0)
					{
						rt->segment_index++;
					}
					if(rt->total_workout_time_tick > 0){
						rt->total_workout_time_tick --;
					}
					/////////////////////
					//DataScreen Time / Bar Update
					//timer elapsed 1 sec after 100 tick count;
					if(rt->tick_1sec_per100ms > 0)
						rt->tick_1sec_per100ms--;	
					if(rt->tick_1sec_per100ms == 0)
					{
						rt->tick_1sec_per100ms = rt->tick_1sec_per100ms_reload;
						Update_Workout_Time_Elapsed();
						Update_Workout_Time_Remaining();
						//AutoPopulatePace();
						Update_GUI_bar();
					}
					/////20130409/////////
					if(exception->cmd.hr_cruise == 1)
					{
						HR_Cruise();
					}
					else
					{
						HRC_conditioning();
					}
				}

				if (resume_mod_flag)
				{
					resume_mod_flag = 0;
					rt->workout_mode = RESUME_MODE;
					for(int i = rt->segment_index; i < rt->workLoad_TableUsed; i++)
					{
						rt->workLoad_Table[i] = update->Workload_level;
						rt->workPace_Table[i] = 0;
					}
				}
				break;
				
			default:
printf("(%s %d) ERR\n", __FILE__, __LINE__);
				//error handling
				break;
			}	

			WaitForProgramTick();//timer tick will com once 100ms
			if(rt->exception_result == EXCEPTION_STAGE_ADVANCE)
			{
				rt->exception_result = EXCEPTION_CONTINUE;
				break;
			}
		}
	}	

	EndofProgram(HRC_WEIGHT_LOSS);
}

void CFpcApp::Hrc_Aerobic(void)
{      
	WorkoutLoopType_HRC(HRC_AEROBIC, SEGMENT_LOAD_ADJ, CALC_BY_CURRENT_LOAD_LEVEL);
}

void CFpcApp::Hrc_Distance(void)
{      
	WorkoutLoopType_HRC_distance(HRC_DISTANCE, SEGMENT_LOAD_ADJ, CALC_BY_CURRENT_LOAD_LEVEL);
}

void CFpcApp::WorkoutLoopType_HRC_distance(unsigned char for_program, unsigned char load_adj, unsigned char watt_calc_mod)
{

#if 1
	InitWorkoutPhase01_distance(90);
	InitWorkoutPhase02_hrcd(1);
#endif


	//FPC machine state/mode

	rt->workout_mode = MANUAL_MODE;	//start with warm up mode /manual mode
	AdjustMachineWorkLoad(SEGMENT_LOAD_ADJ, 0);
	rt->exception_result = EXCEPTION_CONTINUE;	


	//while(rt->Distance_metric < rt->target_workout_distance && rt->exception_result == EXCEPTION_CONTINUE)
	while(rt->exception_result == EXCEPTION_CONTINUE)
	{
		//for each segment
		update->Segment_time = 60;//rt->segmentTime_Table[rt->segment_index];
 		rt->segment_time_tick = FP_TICKS_PER_SECOND * update->Segment_time;
		rt->workLoad.current_load_level = 
			rt->workLoad_Table[rt->segment_index];
		update->Workload_level = rt->workLoad.current_load_level;
		
		//if(update->Segment_time == 0)rt->segment_index++;
		while((rt->segment_time_tick >0) && (rt->exception_result == EXCEPTION_CONTINUE))
		{			
			//get realtime heartrate
			//rt->current_heart_rate = lcb->hr->DisplayHeartRate;	
			rt->current_heart_rate = update->Heart_rate;

			//handling the Exceptions		
			rt->exception_result = ExceptionHandler(for_program);
			//end of handling the Exceptions			
			if(rt->exception_result == EXCEPTION_COOLDOWN)break;
			if(rt->exception_result == EXCEPTION_BREAK)break;
			//end of handling the Exceptions
			if(rt->current_heart_rate >= setup->Target_heart_rate){
				rt->deltaHR = rt->current_heart_rate - setup->Target_heart_rate;
				if(rt->deltaHR > 12) rt->exception_result = EXCEPTION_OVERHR_BREAK;
			}
			switch(rt->workout_mode)
			{
			case MANUAL_MODE:
				rt->watt_calc_mod = CALC_BY_CURRENT_LOAD_LEVEL;
				rt->load_adj_mode = SEGMENT_LOAD_ADJ;

				//WARM UP mode tring to approach to target hr
				rt->summary_state.did_warmup = 1;
				if(rt->current_heart_rate == 0)
				{
					goto MMT;
					//break;
				}
				if(rt->current_heart_rate > setup->Target_heart_rate){
					rt->deltaHR = rt->current_heart_rate - setup->Target_heart_rate;
					if(rt->deltaHR < 12){
						CollectWarmUpSummaryData();
						//rt->has_warmup = 1;	
						rt->workout_mode = PROGRESS_MODE;
						rt->exception_result = EXCEPTION_STAGE_ADVANCE;
						rt->tick_1sec_per100ms = rt->tick_1sec_per100ms_reload;
						rt->tick_30sec = rt->tick_30sec_reload;
						rt->segment_index ++;
						exception->cmd.segment_shift = 1;
					}
				}else{
					rt->deltaHR = setup->Target_heart_rate - rt->current_heart_rate;
					if(rt->deltaHR < 10){
						CollectWarmUpSummaryData();
						//rt->has_warmup = 1;	
						
						rt->workout_mode = PROGRESS_MODE;
						rt->exception_result = EXCEPTION_STAGE_ADVANCE;
						rt->tick_1sec_per100ms = rt->tick_1sec_per100ms_reload;
						rt->tick_30sec = rt->tick_30sec_reload;
						rt->segment_index ++;
						exception->cmd.segment_shift = 1;
						
					}else{
						//rt->tick_30sec --;	
						//if(rt->tick_30sec==0){
						//	rt->tick_30sec = rt->tick_30sec_reload;
						//	if(rt->current_heart_rate < setup->Target_heart_rate){							
						//		calc_level = GetHiger10WattWorkLoad();
						//	}
						//	rt->adjusted_Target_Workload = calc_level;
						//	PopulateWorkLoad();
						//	AdjustMachineWorkLoad(SEGMENT_LOAD_ADJ,0);
						//	Update_GUI_bar_distance();	
						//}					
					}
					
				}	
				//////////////////////////////////////////////////////
MMT:
				//DataScreen Time / Bar Update
				//timer elapsed 1 sec after 100 tick count;
				if(rt->tick_1sec_per100ms >0)
					rt->tick_1sec_per100ms--;	
				if(rt->tick_1sec_per100ms == 0){
					rt->tick_1sec_per100ms = rt->tick_1sec_per100ms_reload;
					Update_Workout_Time_Elapsed();
					Update_GUI_bar_distance();
				}
				//////////////////////////////////////////////////////			
				break;
		
			case PROGRESS_MODE:
				rt->summary_state.did_workout = 1;
				exception->cmd.distance_started = 1;
				if(rt->current_heart_rate > setup->Target_heart_rate){
					rt->deltaHR = rt->current_heart_rate - setup->Target_heart_rate;
					if(rt->deltaHR > 12){
						rt->exception_result = EXCEPTION_OVERHR_BREAK;	
					}
				}else{
					rt->deltaHR = setup->Target_heart_rate - rt->current_heart_rate;
				}
				if(exception->cmd.pause == 0){
					/////20130409/////////
					//Workout Time Update
					if(rt->segment_time_tick > 0){
						rt->segment_time_tick--;
					}
					if(rt->segment_time_tick==0){
						rt->segment_index++;
						exception->cmd.segment_shift = 1;	//for Update_GUI_bar_distance()
					}
					if(rt->total_workout_time_tick > 0){
						rt->total_workout_time_tick --;
					}
					/////////////////////
					//DataScreen Time / Bar Update
					//timer elapsed 1 sec after 100 tick count;
					if(rt->tick_1sec_per100ms >0)
						rt->tick_1sec_per100ms--;	
					if(rt->tick_1sec_per100ms == 0){
						rt->tick_1sec_per100ms = 
							rt->tick_1sec_per100ms_reload;
						Update_Workout_Time_Elapsed();
						Update_Workout_Time_Remaining();
						//AutoPopulatePace();
						Update_GUI_bar_distance();
					}
					/////20130409/////////
					if(exception->cmd.hr_cruise == 1){
						HR_Cruise_distance();
					}else{
						HRC_conditioning();
					}					
				}
				break;
			}	
			WaitForProgramTick();//timer tick will com once 100ms
			if(rt->exception_result == EXCEPTION_STAGE_ADVANCE){
				rt->exception_result = EXCEPTION_CONTINUE;
				break;
			}
			if(rt->Distance_metric >= rt->target_workout_distance)
			{
				rt->exception_result = EXCEPTION_COOLDOWN;
				break;
			}
		}
	}
	EndofProgram(for_program);
}


extern struct HRC_Intervals_exercise hi_Table[];

void CFpcApp::Hrc_Intervals(void)
{
	InitWorkoutPhase01(20);
	InitWorkoutPhase02_harci(8);
	if (0 == setup->Target_heart_rate)
		setup->Target_heart_rate = 180;
	update->Target_heart_rate = setup->Target_heart_rate;


	rt->watt_calc_mod = CALC_BY_CURRENT_LOAD_LEVEL;
	rt->load_adj_mode = SEGMENT_LOAD_ADJ;

	//FPC machine state/mode
	rt->workout_mode = MANUAL_MODE;	//start with warm up mode /manual mode
	AdjustMachineWorkLoad(SEGMENT_LOAD_ADJ, 0);
	rt->exception_result = EXCEPTION_CONTINUE;


	if (0 == update->Segment_time)		update->Segment_time = 60;

	//while(rt->total_workout_time_tick > 0 && rt->exception_result == EXCEPTION_CONTINUE)
	while(rt->exception_result == EXCEPTION_CONTINUE)
	{
		//for each segment
		update->Segment_time = rt->segmentTime_Table[rt->segment_index];

		if(update->Segment_time > 0)
		{
			rt->segment_time_tick = FP_TICKS_PER_SECOND * update->Segment_time;

			rt->workLoad.current_load_level = rt->workLoad_Table[rt->segment_index];
			update->Workload_level = rt->workLoad.current_load_level;
		}

		//if(rt->segment_index > 1)
		if(rt->segment_index > 0)
		{
			if(rt->workout_mode == PROGRESS_MODE || rt->workout_mode == RESUME_MODE)
			{
				AdjustMachineWorkLoad(SEGMENT_LOAD_ADJ, 0);
			}
		}

		if(update->Segment_time == 0)
			rt->segment_index++;

		while(rt->segment_time_tick > 0 && rt->exception_result == EXCEPTION_CONTINUE)
		{
			//get realtime heartrate
			//rt->current_heart_rate = lcb->hr->DisplayHeartRate;
			rt->current_heart_rate = update->Heart_rate;

			//handling the Exceptions
			rt->exception_result = ExceptionHandler(HRC_INTERVALS);
			//end of handling the Exceptions			
			if(rt->exception_result == EXCEPTION_COOLDOWN)
				break;
			if(rt->exception_result == EXCEPTION_BREAK)
				break;
			//end of handling the Exceptions
			//get realtime heartrate

			if(rt->workout_mode != RESUME_MODE)
			{
				if(rt->current_heart_rate >= setup->Target_heart_rate)
				{
					rt->deltaHR = rt->current_heart_rate - setup->Target_heart_rate;
					if(rt->deltaHR > 12)
						rt->exception_result = EXCEPTION_OVERHR_BREAK;
				}
			}

			switch(rt->workout_mode)
			{
			case RESUME_MODE:
				rt->watt_calc_mod = CALC_BY_CURRENT_LOAD_LEVEL;
				rt->load_adj_mode = SEGMENT_LOAD_ADJ;

				if(exception->cmd.pause != 0)
					break;
				if (resume_mod_flag)
				{
					resume_mod_flag = 0;
					for(int i = rt->segment_index; i < rt->workLoad_TableUsed; i++)
					{
						rt->workLoad_Table[i] = update->Workload_level;
						rt->workPace_Table[i] = 0;
					}
				}
				if(rt->segment_time_tick > 0)
					rt->segment_time_tick--;
				if(rt->segment_time_tick == 0)
					rt->segment_index++;
				if(rt->total_workout_time_tick > 0)
					rt->total_workout_time_tick--;
				if(rt->tick_1sec_per100ms > 0)
					rt->tick_1sec_per100ms--;	
				if(rt->tick_1sec_per100ms == 0)
				{
					rt->workLoad.current_load_level = rt->workLoad_Table[rt->segment_index];
					setup->Workload_level = update->Workload_level = rt->workLoad.current_load_level;

					rt->tick_1sec_per100ms = rt->tick_1sec_per100ms_reload;
					Update_Workout_Time_Elapsed();
					Update_Workout_Time_Remaining();
					Update_GUI_bar();
				}
				break;

			case MANUAL_MODE:
				rt->watt_calc_mod = CALC_BY_CURRENT_LOAD_LEVEL;
				rt->load_adj_mode = SEGMENT_LOAD_ADJ;

				//WARM UP mode tring to approach to target hr
				rt->summary_state.did_warmup = 1;
				if(rt->current_heart_rate == 0)
				{
					//printf("(%s %d) MMT\n", __FILE__, __LINE__);
					goto MMT;
					//break;
				}
				if(rt->current_heart_rate > setup->Target_heart_rate)
				{
					rt->deltaHR = rt->current_heart_rate - setup->Target_heart_rate;
					if(rt->deltaHR < 12)
					{
						CollectWarmUpSummaryData();
						//rt->has_warmup = 1;	
						rt->workout_mode = PROGRESS_MODE;
						rt->exception_result = EXCEPTION_STAGE_ADVANCE;
						rt->tick_30sec = rt->tick_30sec_reload;
						rt->segment_index ++;
						rt->tick_1sec_per100ms = rt->tick_1sec_per100ms_reload;
					}
				}
				else
				{
					rt->deltaHR = setup->Target_heart_rate - rt->current_heart_rate;
					if(rt->deltaHR < 10)
					{
						CollectWarmUpSummaryData();
						//rt->has_warmup = 1;
						rt->workout_mode = PROGRESS_MODE;
						rt->exception_result = EXCEPTION_STAGE_ADVANCE;
						rt->tick_30sec = rt->tick_30sec_reload;
						rt->segment_index ++;
						rt->tick_1sec_per100ms = rt->tick_1sec_per100ms_reload;
					}
					else
					{
						rt->tick_30sec --;	
						if(rt->tick_30sec==0)
						{
							rt->tick_30sec = rt->tick_30sec_reload;
							//if(rt->current_heart_rate < setup->Target_heart_rate)
							//{							
							//	calc_level = GetHiger10WattWorkLoad();
							//}
							//rt->adjusted_Target_Workload = calc_level;
							//PopulateWorkLoad_Intervals();
							//AdjustMachineWorkLoad(SEGMENT_LOAD_ADJ,0);
							//Update_GUI_bar();			
						}					
					}
				}	
MMT:
				//////////////////////////////////////////////////////
				//DataScreen Time / Bar Update
				//timer elapsed 1 sec after 100 tick count;
				if(rt->tick_1sec_per100ms > 0)
				{
					rt->tick_1sec_per100ms--;
				}
				if(rt->tick_1sec_per100ms == 0)
				{
					rt->tick_1sec_per100ms = rt->tick_1sec_per100ms_reload;
					Update_Workout_Time_Elapsed();
					//if (resume_mod_flag)						Update_Workout_Time_Remaining();
					Update_GUI_bar();
				}
				//////////////////////////////////////////////////////				
				break;

			case PROGRESS_MODE:
				rt->summary_state.did_workout = 1;
				if(rt->current_heart_rate > setup->Target_heart_rate)
				{
					rt->deltaHR = rt->current_heart_rate - setup->Target_heart_rate;
					if(rt->deltaHR > 12)
					{
						rt->exception_result = EXCEPTION_OVERHR_BREAK;
					}
				}
				else
				{
					rt->deltaHR = setup->Target_heart_rate - rt->current_heart_rate;
				}

				if(exception->cmd.pause == 0)
				{
					//Workout time Description:
					//‧	Time:	After the START button is pressed the bike or elliptical will be in a MANUAL or WARM UP mode until the users 
					//	HR reaches within 10 BPM of the set WORK HR.  Once the user reaches within 10 BPM of the set WORK HR the workout will 
					//	officially start from the WORKOUT TIME set in the set up screen.  If the BPM exceeds the set Target HR by 12 the 
					//	program will end.
					//‧	Work/rest:  Once the workout begins the bike or elliptical will make workload adjustments to meet the set WORK HR, 
					//	once the WORK HR is reached the bike or elliptical will begin to decrease workload to reach the REST HR.  Once the 
					//	REST HR is achieved the workload will then step-up to the previous WORK HR workload for 1:00 then step-down to the 
					//	previous REST HR workload for 1:00.  This process will continue through the end of the workout.
					switch(hi_Table[rt->segment_index-1].work_type)
					{
					case WORK_SEGMENT:
						HRC_conditioning_hrci();
						if(exception->cmd.hr_cruise == 1)
						{
							HR_Cruise();
						}
						else
						{
							HRC_conditioning_hrci();
						}
						break;
					case REST_SEGMENT:

// 						if(rest_watt_set == 0)
//						{
// 							calc_level = GetLower065WattWorkLoad();
// 							AdjustMachineWorkLoad(load_adj,0);
// 							rt->workLoad.current_load_level = calc_level;
// 							Update_GUI_bar();
// 							rest_watt_set = 1;
// 						}						
						break;
					}
					/////20130409/////////
					//Workout Time Update
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
						rt->total_workout_time_tick--;
					}
					/////////////////////
					//DataScreen Time / Bar Update
					//timer elapsed 1 sec after 100 tick count;
					if(rt->tick_1sec_per100ms > 0)
						rt->tick_1sec_per100ms--;	
					if(rt->tick_1sec_per100ms == 0)
					{
						rt->workLoad.current_load_level = rt->workLoad_Table[rt->segment_index];
						setup->Workload_level = update->Workload_level = rt->workLoad.current_load_level;

						rt->tick_1sec_per100ms = rt->tick_1sec_per100ms_reload;
						Update_Workout_Time_Elapsed();
						Update_Workout_Time_Remaining();
						//AutoPopulatePace();
						Update_GUI_bar();
					}
					/////20130409/////////
				}

				if (resume_mod_flag)
				{
					resume_mod_flag = 0;
					rt->workout_mode = RESUME_MODE;
					for(int i = rt->segment_index; i < rt->workLoad_TableUsed; i++)
					{
						rt->workLoad_Table[i] = update->Workload_level;
						rt->workPace_Table[i] = 0;
					}
				}
				break;
		
			default:
				;//error handling
				break;
			}	

			WaitForProgramTick();//timer tick will com once 100ms

			if(rt->exception_result == EXCEPTION_STAGE_ADVANCE)
			{
				rt->exception_result = EXCEPTION_CONTINUE;
				break;
			}
		}
	}	

	EndofProgram(HRC_INTERVALS);
}

struct WLClass20_Load performance_cardio_challenge_LeveTable[]=	//indexd by Level, each have 20 segments
{
	{{ 1,	 1,	 4,	 4,	 7,	 7,	 8,	 8,	10,	10,	10,	10,	 8,	 8,	 7,	 7,	 4,	 4,	 1,	 1}},
	{{ 1,	 1,	 4,	 4,	 8,	 8,	10,	10,	11,	11,	11,	11,	10,	10,	 8,	 8,	 4,	 4,	 1,	 1}},
	{{ 1,	 1,	 7,	 7,	10,	10,	11,	11,	12,	12,	12,	12,	11,	11,	10,	10,	 7,	 7,	 1,	 1}},
	{{ 4,	 4,	 7,	 7,	10,	10,	12,	12,	13,	13,	13,	13,	12,	12,	10,	10,	 7,	 7,	 4,	 4}},
	{{ 4,	 4,	 8,	 8,	11,	11,	13,	13,	14,	14,	14,	14,	13,	13,	11,	11,	 8,	 8,	 4,	 4}},
	{{ 4,	 4,	 8,	 8,	11,	11,	13,	13,	14,	14,	14,	14,	13,	13,	11,	11,	 8,	 8,	 4,	 4}},
	{{ 7,	 7,	10,	10,	11,	11,	13,	13,	15,	15,	15,	15,	13,	13,	11,	11,	10,	10,	 7,	 7}},
	{{ 7,	 7,	10,	10,	12,	12,	14,	14,	15,	15,	15,	15,	14,	14,	12,	12,	10,	10,	 7,	 7}},
	{{ 7,	 7,	10,	10,	12,	12,	14,	14,	16,	16,	16,	16,	14,	14,	12,	12,	10,	10,	 7,	 7}},
	{{ 7,	 7,	11,	11,	14,	14,	15,	15,	16,	16,	16,	16,	15,	15,	14,	14,	11,	11,	 7,	 7}},
	{{ 8,	 8,	11,	11,	13,	13,	15,	15,	17,	17,	17,	17,	15,	15,	13,	13,	11,	11,	 8,	 8}},
	{{ 8,	 8,	12,	12,	14,	14,	15,	15,	18,	18,	18,	18,	15,	15,	14,	14,	12,	12,	 8,	 8}},
	{{ 8,	 8,	12,	12,	14,	14,	16,	16,	18,	18,	18,	18,	16,	16,	14,	14,	12,	12,	 8,	 8}},
	{{ 8,	 8,	13,	13,	15,	15,	16,	16,	19,	19,	19,	19,	16,	16,	15,	15,	13,	13,	 8,	 8}},
	{{10,	10,	13,	13,	15,	15,	17,	17,	19,	19,	19,	19,	17,	17,	15,	15,	13,	13,	10,	10}},
	{{10,	10,	14,	14,	15,	15,	18,	18,	20,	20,	20,	20,	18,	18,	15,	15,	14,	14,	10,	10}},
	{{11,	11,	14,	14,	16,	16,	18,	18,	21,	21,	21,	21,	18,	18,	16,	16,	14,	14,	11,	11}},
	{{11,	11,	15,	15,	18,	18,	20,	20,	24,	24,	24,	24,	20,	20,	18,	18,	15,	15,	11,	11}},
	{{12,	12,	15,	15,	18,	18,	20,	20,	30,	30,	30,	30,	20,	20,	18,	18,	15,	15,	12,	12}},
	{{12,	12,	16,	16,	19,	19,	22,	22,	30,	30,	30,	30,	22,	22,	19,	19,	16,	16,	12,	12}}
};

struct WLClass20_Pace performance_cardio_challenge_PaceTable[]=
{
	{{50,50,54,54,58,58, 62, 62, 65, 65, 65, 65, 62, 62,58,58,54,54,50,50}},	// 1,
	{{50,50,55,55,59,59, 63, 63, 68, 68, 68, 68, 63, 63,59,59,55,55,50,50}},	// 2,
	{{52,52,55,55,60,60, 65, 65, 70, 70, 70, 70, 65, 65,60,60,55,55,52,52}},	// 3,	
	{{52,52,56,56,61,61, 66, 66, 73, 73, 73, 73, 66, 66,61,61,56,56,52,52}},	// 4,
	{{52,52,56,56,62,62, 67, 67, 75, 75, 75, 75, 67, 67,62,62,56,56,52,52}},	// 5,
	{{54,54,58,58,63,63, 69, 69, 78, 78, 78, 78, 69, 69,63,63,58,58,54,54}},	// 6,
	{{56,56,61,61,66,66, 72, 72, 80, 80, 80, 80, 72, 72,66,66,61,61,56,56}},	// 7,
	{{58,58,64,64,69,69, 75, 75, 83, 83, 83, 83, 75, 75,69,69,64,64,58,58}},	// 8,
	{{58,58,65,65,70,70, 76, 76, 85, 85, 85, 85, 76, 76,70,70,65,65,58,58}},	// 9,
	{{60,60,65,65,72,72, 80, 80, 88, 88, 88, 88, 80, 80,72,72,65,65,60,60}},	//10,
	{{60,60,66,66,73,73, 82, 82, 90, 90, 90, 90, 82, 82,73,73,66,66,60,60}},	//11,
	{{62,62,68,68,75,75, 84, 84, 93, 93, 93, 93, 84, 84,75,75,68,68,62,62}},	//12,
	{{64,64,70,70,78,78, 85, 85, 95, 95, 95, 95, 85, 85,78,78,70,70,64,64}},	//13,
	{{64,64,74,74,82,82, 90, 90, 98, 98, 98, 98, 90, 90,82,82,74,74,64,64}},	//14,
	{{66,66,74,74,84,84, 92, 92,100,100,100,100, 92, 92,84,84,74,74,66,66}},	//15,
	{{66,66,75,75,84,84, 94, 94,103,103,103,103, 94, 94,84,84,75,75,66,66}},	//16,
	{{68,68,78,78,85,85, 95, 95,105,105,105,105, 95, 95,85,85,78,78,68,68}},	//17,
	{{68,68,78,78,88,88, 98, 98,108,108,108,108, 98, 98,88,88,78,78,68,68}},	//18,
	{{70,70,80,80,90,90,100,100,110,110,110,110,100,100,90,90,80,80,70,70}},	//19,
	{{70,70,80,80,90,90,100,100,115,115,115,115,100,100,90,90,80,80,70,70}} 	//20,
};


void CFpcApp::Performance_Cardio_Challenge(void)
{

#if 1
	InitWorkoutPhase01(20);
	InitWorkoutPhase02_pcc(20, performance_cardio_challenge_LeveTable, performance_cardio_challenge_PaceTable);//(20 segment, no timing table, indexed workload table)
#endif

	WorkoutLoopType_WLClass20(PERFORMANCE_CARDIO_CHALLENGE);	
}

#define NONE_HR_RETRY_TIMES 10

void CFpcApp::Performance_Fitness_Test(void)
{
	int hasVo2 = 1;
	int fitness_out = 0;

	//unsigned char old_stage_index = 0;
	unsigned short deltaHr = 0;
	unsigned short sample_hr1 = 0;
	unsigned short sample_hr2 = 0;
	unsigned short StageHR[] = {0,0,0,0};
	unsigned short StageWatt[] = {0,0,0,0};
	unsigned char i = 0;
	unsigned char last_stage_hr = 0;
	float new_target_watt = 0.00F;

	//for VO2
	float hpw1 = 0.00F;
	float hpw2 = 0.00F;
	float watts_headroom = 0.00F;
	float Max_Continuos_watts = 0.00F;
	float Max_VO2 = 0.00F;
	float Max_HR = 0.00F;

	//Set up screen:
	//‧	Age			(default 35) Adjustable from 10-99
	//‧	Target Heart Rate	(default 157/85%) This is not adjustable.  HR is variable based on age and

	// Workout time:
	//‧	Time:
	//		Is determined two different ways for this test.
	//		First-if the user has not reached their 85% of maximum HR within 12 minuets the test will comes to an
	//		end and gives VO2max score in the message center.  
	//		Second- if the user reaches their 85% maximum HR the test will end and display the V02 max score 0.   
	//		At any time the HR is lost the Test will end and display NO HEART RATE DETECTED in the message center.
#if 1
	InitWorkoutPhase01(12);
	InitWorkoutPhase02_pft(1);
//rt->total_workout_time = 72;
#endif

	//Segment description:
	//‧	Both workload and pace will be displayed as one minute segments.	
	//‧	Graph will be the only available screen shot and the attached formula should display a fixed chart in the 20 segment display.
	//
	//Estimated V02 Max reading:
	//‧	At the end of the test the V02 Max # associated with the completion time should scroll through the message center.
	rt->Target_Watts = 25;
	rt->stage_extended = 0;
	deltaHr = sample_hr1 = sample_hr2 = 0;

	rt->no_heart_rate_detect = NONE_HR_RETRY_TIMES;
	rt->exception_result = EXCEPTION_CONTINUE;	
	last_stage_hr = 0;
	//Max_HR = 0;

	if (0 == update->Segment_time)
		update->Segment_time = 60;

	//while(rt->total_workout_time_tick > 0 && rt->exception_result == EXCEPTION_CONTINUE)
	while(rt->exception_result == EXCEPTION_CONTINUE)
	{
		//for each segment
		update->Segment_time = rt->segmentTime_Table[rt->segment_index];
		if(update->Segment_time > 0)
		{
			rt->segment_time_tick = FP_TICKS_PER_SECOND * update->Segment_time;
			if(exception->cmd.hr_cruise == 0)
			{
				rt->workLoad.current_load_level = rt->workLoad_Table[rt->segment_index];
				update->Workload_level = rt->workLoad.current_load_level;
				AdjustMachineWorkLoad(SEGMENT_LOAD_ADJ, 0);
			}
		}

		//rt->stage_index = rt->segment_index%4;
		rt->stage_index = rt->segment_index / 3;
		//old_stage_index = rt->stage_index;

		if(rt->stage_index > 0)
		{
			last_stage_hr = (unsigned char)StageHR[rt->stage_index - 1];
		}

		new_target_watt = GetNewTargeWatt_stage_hr((unsigned char)rt->stage_index, last_stage_hr);
printf("(%s %d) new_target_watt=%f %d, %d\n", __FILE__, __LINE__, new_target_watt, rt->stage_index, last_stage_hr);


		StageWatt[rt->stage_index] = (unsigned short)new_target_watt;//rt->Target_Watts;
		for(i = rt->stage_index; i < rt->workLoad_TableUsed; i++)
		{
			new_target_watt = GetNewTargeWatt_stage_hr((unsigned char)(i / 3), last_stage_hr);

// jason
			//rt->workLoad_Table[i] = tables->Get_60rpm_Level_ByWatt((unsigned short)new_target_watt);
		}



		while(rt->segment_time_tick > 0 && rt->exception_result == EXCEPTION_CONTINUE)
		{
			//get realtime heartrate
			//rt->current_heart_rate = lcb->hr->DisplayHeartRate;
			rt->current_heart_rate = update->Heart_rate;

			if(rt->current_heart_rate > Max_HR)
				Max_HR = (float)rt->current_heart_rate;

			//handling the Exceptions
			rt->exception_result = ExceptionHandler(PERFORMANCE_FITNESS_TEST);

			//end of handling the Exceptions
			if(rt->exception_result == EXCEPTION_COOLDOWN)
			{
				printf("(%s %d) EXCEPTION_COOLDOWN\n", __FILE__, __LINE__);
				break;
			}
			if(rt->exception_result == EXCEPTION_BREAK)
			{
				printf("(%s %d) EXCEPTION_BREAK\n", __FILE__, __LINE__);
				break;
			}
			//end of handling the Exceptions

			switch(rt->segment_index % 3)
			{
				//every 2nd,3rd minutes the highest heartrate is sampled
				//according to the [fit test.xls] spread sheet.
			case 0:
// jason
				rt->stage_extended = 0;
				sample_hr1 = 0;
				sample_hr2 = 0;
				break;
			case 1:
				if(rt->segment_time_tick <= 150)
				{
					//sample heart rate hr1
					//rt->current_heart_rate = lcb->hr->DisplayHeartRate;
					rt->current_heart_rate = update->Heart_rate;

					if(sample_hr1 == 0)
						sample_hr1 = rt->current_heart_rate;
					else
						sample_hr1 = (sample_hr1 + rt->current_heart_rate) / 2;
				}
				break;
			case 2:
				if(rt->segment_time_tick <= 150)
				{
					//sample heart rate hr2
					//rt->current_heart_rate = lcb->hr->DisplayHeartRate;
					rt->current_heart_rate = update->Heart_rate;

					if(sample_hr2 == 0)
						sample_hr2 = rt->current_heart_rate;
					else
						sample_hr2 = (sample_hr2 + rt->current_heart_rate) / 2;
				}
				if(rt->segment_time_tick == 1)
				{
					//check if the stage is extended @ the final 100ms moment
					if(sample_hr2 > sample_hr1)
						deltaHr = sample_hr2 - sample_hr1;
					else
						deltaHr = sample_hr1 - sample_hr2;

					//Each stage is 3 minutes long.
					//Heart rates are sampled during the final 15 seconds of the second and third minute of each stage.
					//If the heart rates are not within 6 bpm of each other, the stage is extended another minute. 
// jason
					if (0 == rt->stage_index || 3 == rt->stage_index)
						deltaHr = 0;
					if(deltaHr > 6)
					{
						if(rt->stage_extended == 0)
						{
							rt->stage_extended = 1;

							sample_hr1 = (sample_hr1 + sample_hr2) / 2;

// jason
							update->Segment_time += 60;
							update->Time_remaining += 60;
							rt->total_workout_time_tick += 601;
							rt->segmentTime_Table[rt->segment_index] += 60;
							rt->segment_time += 60;
							rt->segment_time_tick += 601;
						}
						else
						{
							StageHR[rt->stage_index] = (sample_hr2 + sample_hr1) / 2;
						}
					}
					else
					{
						StageHR[rt->stage_index] = (sample_hr2 + sample_hr1) / 2;
					}
				}
				break;
			}

			/////20130409/////////
			//Workout Time Update
			if(rt->segment_time_tick > 0)
				rt->segment_time_tick--;
			if(rt->segment_time_tick == 0)
				rt->segment_index++;
			if(rt->total_workout_time_tick > 0)
				rt->total_workout_time_tick--;
			/////////////////////
			//DataScreen Time / Bar Update
			//timer elapsed 1 sec after 100 tick count;
			if(rt->tick_1sec_per100ms > 0)
				rt->tick_1sec_per100ms--;
			if(rt->tick_1sec_per100ms == 0)
			{
				rt->tick_1sec_per100ms = rt->tick_1sec_per100ms_reload;
				Update_Workout_Time_Elapsed();
				Update_Workout_Time_Remaining();
				Update_GUI_bar();
				//if((segment_index_max + 1) > rt->segment_index)					Update_GUI_bar_Fitness();
			}
			/////20130409/////////
			if(exception->cmd.hr_cruise == 1)
				HR_Cruise();

			//if(rt->current_heart_rate >= setup->Target_heart_rate)
			if(rt->current_heart_rate >= 185)
			{
				hasVo2 = 0;
				rt->exception_result = EXCEPTION_COOLDOWN;
				printf("(%s %d) EXCEPTION_COOLDOWN, rt->current_heart_rate >= 185\n", __FILE__, __LINE__);
				break;
			}
			/*if(rt->current_heart_rate == 0)
			{
				hasVo2 = 0;
				rt->exception_result = EXCEPTION_COOLDOWN;
				printf("(%s %d) EXCEPTION_COOLDOWN, rt->current_heart_rate == 0\n", __FILE__, __LINE__);
				break;
			}*/

			/*if(rt->segment_index == (segment_index_max + 2))
			{
				rt->exception_result = EXCEPTION_COOLDOWN;
			//break;
			}*/

			WaitForProgramTick();//timer tick will com once 100ms


//////////////////////////////////////////////
			if(rt->total_workout_time_tick == 0)
			{
				update->Time_remaining_1000 = 0;
				update->Time_remaining = 0;
				fitness_out = 1;
				goto FITNESS_OUT;
			}			

		}		
	}



//////////////////////////////////////////////
FITNESS_OUT:
	if(fitness_out)
	{
		for(int i = 0; i < 1000; i++)
		{
			rt->exception_result = ExceptionHandler(ALLPRESET_DISTANCE);
			if(rt->exception_result == EXCEPTION_COOLDOWN)
				break;
			if(rt->exception_result == EXCEPTION_BREAK)
				break;
			usleep(1000);
		}
		rt->summary_state.has_warmup = 0;
		rt->summary_state.has_test_load = 0;
		rt->summary_state.has_workout = 0;
		rt->summary_state.has_cooldown = 0;
		rt->summary_state.did_warmup = 0;
		rt->summary_state.did_test_load = 0;
		rt->summary_state.did_workout = 0;
		rt->summary_state.did_cooldown = 0;

		rt->segment_index = 12;
	}

	//if(rt->exception_result == EXCEPTION_COOLDOWN)
	if(0 == hasVo2)
	{
		summary->Vo2 = 0;
		printf("(%s %d) EXCEPTION_COOLDOWN, summary->Vo2=0\n", __FILE__, __LINE__);
	}
	else
	{
		// 臺階的總氧氣消耗量

		//HR1, HR2, HR3, HR4 ==> 每stage最後15秒平均心跳
		//StageHR[0],	StageHR[1],	StageHR[2],	StageHR[3]
		//StageWatt[0],	StageWatt[1],	StageWatt[2],	StageWatt[3]
//		if((StageWatt[1] > 0)&&(StageWatt[2] > 0)&&(StageWatt[3] > 0))
//		{
		// HPW1= (HR3-HR2)/(watt3-watt2) ==> 每watt心跳增加率(stage 2 to 3)

		if (StageWatt[2] - StageWatt[1] > 0)
		{
			hpw1 = (float)(StageHR[2] - StageHR[1]) / (float)(StageWatt[2] - StageWatt[1]);
			printf("(%s %d) %d %d %f\n", __FILE__, __LINE__, StageHR[1], StageHR[2], hpw1);
		}
		else
		{
			printf("(%s %d) %d %d\n", __FILE__, __LINE__, StageHR[1], StageHR[2]);
		}


		
		// HPW2= (HR4-HR3)/(watt4-watt3) ==> 每watt心跳增加率(stage 3 to 4)

		if (StageWatt[3] - StageWatt[2] > 0)
		{
			hpw2 = (float)(StageHR[3] - StageHR[2]) / (float)(StageWatt[3] - StageWatt[2]);
			printf("(%s %d) %d %d %f\n", __FILE__, __LINE__, StageHR[2], StageHR[3], hpw1);
		}
		else
		{
			printf("(%s %d) %d %d\n", __FILE__, __LINE__, StageHR[2], StageHR[3]);
		}


		// Watts Headroom = (Max_HR - HR4)/((HPW1 + HPW2) / 2)
		if (hpw1 + hpw2 > 0.00F)
		{
			watts_headroom = (Max_HR - (float)StageHR[3]) / ((hpw1 + hpw2) / 2);
			printf("(%s %d) %d %f %f\n", __FILE__, __LINE__, StageHR[3], Max_HR, watts_headroom);
		}
		else
		{
			printf("(%s %d) 0 == (hpw1 + hpw2)\n", __FILE__, __LINE__);
		}


//		}

		// Max Continuos watts = Watt4 + Watts Headroom
		Max_Continuos_watts = (float)StageWatt[3] + watts_headroom;
		
		// Max_VO2 = Max Continus watts / ((weight_lb/2.2)*0.083)
		if (setup->Weight > 0)
		{
			Max_VO2 = Max_Continuos_watts / (((float)setup->Weight / 2.2F) * 0.083F);
			printf("(%s %d) Max_Continuos_watts=%f %d %f %d\n", __FILE__, __LINE__, Max_Continuos_watts, StageWatt[3], watts_headroom, setup->Weight);
		}

		summary->Vo2 = (unsigned short)Max_VO2;
		printf("(%s %d) summary->Vo2=%d\n", __FILE__, __LINE__, summary->Vo2);
	}


	EndofProgram(PERFORMANCE_FITNESS_TEST);
}



void CFpcApp::Performance_Pace_Ramp(void)
{

#if 1
	InitWorkoutPhase01(20);
	InitWorkoutPhase02_ppr(20);
	//InitWorkoutPhase02_pcc(20, wpr_Table, ppr_Table);
#endif


	if (0 == update->Segment_time)		update->Segment_time = 60;

	rt->exception_result = EXCEPTION_CONTINUE;	


	//while(rt->total_workout_time_tick > 0 && rt->exception_result == EXCEPTION_CONTINUE)
	while(rt->exception_result == EXCEPTION_CONTINUE)
	{
		//for each segment
		update->Segment_time = rt->segmentTime_Table[rt->segment_index];
 		rt->segment_time_tick = FP_TICKS_PER_SECOND * update->Segment_time;
		if(exception->cmd.hr_cruise == 0)
		{
			rt->workLoad.current_load_level = rt->workLoad_Table[rt->segment_index];
			//update->Workload_level = rt->workLoad.current_load_level;
			//AdjustMachineWorkLoad(INDEXED_TARGET_LOAD_ADJ_WLClass20, 0);
			AdjustMachineWorkLoad(SEGMENT_LOAD_ADJ, 0);
		}

		while(rt->segment_time_tick > 0 && rt->exception_result == EXCEPTION_CONTINUE)
		{
			//get realtime heartrate
			//rt->current_heart_rate = lcb->hr->DisplayHeartRate;
			rt->current_heart_rate = update->Heart_rate;

			//handling the Exceptions		
			rt->exception_result = ExceptionHandler(PERFORMANCE_PACE_RAMP);
			//end of handling the Exceptions			
			if(rt->exception_result == EXCEPTION_COOLDOWN)
				break;
			if(rt->exception_result == EXCEPTION_BREAK)
				break;
			//end of handling the Exceptions
		
			if(exception->cmd.pause == 0)
			{
				//Data Displays: real time update data
				{
					//Data Displays:
					//‧	TIME ELAPSED
					//‧	TIME REMAINING
					//‧	RPM/PACE SETTER
					//‧	DISTANCE
					//‧	DISTANCE REMAINING (disabled)
					//‧	CALORIES BURNED
					//‧	CALORIES/HR
					//‧	WATTs
					//‧	METs
					//‧	BEATS PER MINUTE
					//‧	TARGET HR (disabled)
					//‧	Graph
					//‧	Track
					//‧	Scene
					//‧	Simple
				}
				//Message center:  
				//(repeat scroll of messages as listed below every 2 minutes)
				//      
				//‧	You are doing the PACE RAMP workout.
				//‧	To change intensity of PACE RAMP, adjust PACE SETTER at any time.
				//‧	Change WORKLOAD at any time.
				//‧	PAUSE workout at any time by pressing STOP/PAUSE. 
				//‧	Care should be used when stepping on or stepping off the elliptical.
				//‧	To learn more about this product, go to TRUEFITNESS.com 
				/////20130409/////////
				//Workout Time Update
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
				/////////////////////
				//DataScreen Time / Bar Update
				//timer elapsed 1 sec after 100 tick count;
				if(rt->tick_1sec_per100ms > 0)
					rt->tick_1sec_per100ms--;	
				if(rt->tick_1sec_per100ms == 0)
				{
					rt->tick_1sec_per100ms = rt->tick_1sec_per100ms_reload;
					Update_Workout_Time_Elapsed();
					Update_Workout_Time_Remaining();

					UpdateCoolDownPace(PERFORMANCE_PACE_RAMP);

					Update_GUI_bar();
				}
				/////20130409/////////
				if(exception->cmd.hr_cruise == 1)HR_Cruise();
			}
			WaitForProgramTick();//timer tick will com once 100ms
		}
	}

	EndofProgram(PERFORMANCE_PACE_RAMP);
}


extern struct BiPace ppiPaceTable[];

void CFpcApp::Performance_Pace_Intervals(void)
{

#if 1
	InitWorkoutPhase01(20);
	InitWorkoutPhase02_segTime_pace(1, ppiPaceTable);
#endif

	if (0 == update->Segment_time)		update->Segment_time = 60;

	rt->exception_result = EXCEPTION_CONTINUE;	
	//while((rt->total_workout_time_tick > 0 && rt->exception_result == EXCEPTION_CONTINUE)
	while(rt->exception_result == EXCEPTION_CONTINUE)
	{	
		//for each segment
		update->Segment_time = rt->segmentTime_Table[rt->segment_index];
 		rt->segment_time_tick = FP_TICKS_PER_SECOND * update->Segment_time;

		if(exception->cmd.hr_cruise == 0)
		{
			rt->workLoad.current_load_level = rt->workLoad_Table[rt->segment_index];
			//update->Workload_level = rt->workLoad.current_load_level;
			AdjustMachineWorkLoad(SEGMENT_LOAD_ADJ, 0);
		}

		while(rt->segment_time_tick > 0 && rt->exception_result == EXCEPTION_CONTINUE)
		{
			//get realtime heartrate
			//rt->current_heart_rate = lcb->hr->DisplayHeartRate;
			rt->current_heart_rate = update->Heart_rate;

			//handling the Exceptions		
			rt->exception_result = ExceptionHandler(PERFORMANCE_PACE_INTERVAL);
			//end of handling the Exceptions			
			if(rt->exception_result == EXCEPTION_COOLDOWN)
				break;
			if(rt->exception_result == EXCEPTION_BREAK)
				break;
			//end of handling the Exceptions

			if(exception->cmd.pause == 0){
				/////20130409/////////
				//Workout Time Update
				if(rt->segment_time_tick > 0){
					rt->segment_time_tick--;
				}
				if(rt->segment_time_tick==0){
					rt->segment_index++;
				}
				if(rt->total_workout_time_tick > 0){
					rt->total_workout_time_tick --;
				}
				/////////////////////
				//DataScreen Time / Bar Update
				//timer elapsed 1 sec after 100 tick count;
				if(rt->tick_1sec_per100ms >0)
					rt->tick_1sec_per100ms--;	
				if(rt->tick_1sec_per100ms == 0)
				{
					rt->tick_1sec_per100ms = rt->tick_1sec_per100ms_reload;
					Update_Workout_Time_Elapsed();
					Update_Workout_Time_Remaining();
					UpdateCoolDownPace(PERFORMANCE_PACE_INTERVAL);
					Update_GUI_bar();
				}
				/////20130409/////////
				if(exception->cmd.hr_cruise == 1)
					HR_Cruise();
			}

			WaitForProgramTick();//timer tick will com once 100ms
		}
	}

	EndofProgram(PERFORMANCE_PACE_INTERVAL);

printf("(%s %d) rt->workPace_Table[]=%d rt->total_segment=%d\n", __FILE__, __LINE__, rt->workPace_Table[rt->total_segment], rt->total_segment);
}


struct WLClass20_Pace legShapePaceTable[] =
{
{{40,	40,	72,	52,	52,	72,	40,	40,	48,	72,	72,	48,	40,	40,	72,	52,	52,	72,	40,	40}},
{{42,	42,	74,	54,	54,	74,	42,	42,	50,	74,	74,	50,	42,	42,	74,	54,	54,	74,	42,	42}},
{{44,	44,	76,	56,	56,	76,	44,	44,	52,	76,	76,	52,	44,	44,	76,	56,	56,	76,	44,	44}},
{{46,	46,	78,	58,	58,	78,	46,	46,	54,	78,	78,	54,	46,	46,	78,	58,	58,	78,	46,	46}},
{{48,	48,	80,	60,	60,	80,	48,	48,	56,	80,	80,	56,	48,	48,	80,	60,	60,	80,	48,	48}},
{{50,	50,	82,	62,	62,	82,	50,	50,	58,	82,	82,	58,	50,	50,	82,	62,	62,	82,	50,	50}},
{{52,	52,	84,	64,	64,	84,	52,	52,	60,	84,	84,	60,	52,	52,	84,	64,	64,	84,	52,	52}},
{{54,	54,	86,	66,	66,	86,	54,	54,	62,	86,	86,	62,	54,	54,	86,	66,	66,	86,	54,	54}},
{{56,	56,	88,	68,	68,	88,	56,	56,	64,	88,	88,	64,	56,	56,	88,	68,	68,	88,	56,	56}},
{{58,	58,	90,	70,	70,	90,	58,	58,	66,	90,	90,	66,	58,	58,	90,	70,	70,	90,	58,	58}},
{{60,	60,	92,	72,	72,	92,	60,	60,	68,	92,	92,	68,	60,	60,	92,	72,	72,	92,	60,	60}},
{{62,	62,	94,	74,	74,	94,	62,	62,	70,	94,	94,	70,	62,	62,	94,	74,	74,	94,	62,	62}},
{{64,	64,	96,	76,	76,	96,	64,	64,	72,	96,	96,	72,	64,	64,	96,	76,	76,	96,	64,	64}},
{{66,	66,	98,	78,	78,	98,	66,	66,	74,	98,	98,	74,	66,	66,	98,	78,	78,	98,	66,	66}},
{{68,	68,	100,	80,	80,	100,	68,	68,	76,	100,	100,	76,	68,	68,	100,	80,	80,	100,	68,	68}},
{{70,	70,	102,	82,	82,	102,	70,	70,	78,	102,	102,	78,	70,	70,	102,	82,	82,	102,	70,	70}},
{{72,	72,	104,	84,	84,	104,	72,	72,	80,	104,	104,	80,	72,	72,	104,	84,	84,	104,	72,	72}},
{{74,	74,	106,	86,	86,	106,	74,	74,	82,	106,	106,	82,	74,	74,	106,	86,	86,	106,	74,	74}},
{{76,	76,	108,	88,	88,	108,	76,	76,	84,	108, 108,	84,	76,	76,	108,	88,	88,	108,	76,	76}},
{{78,	78,	110,	90,	90,	110,	78,	78,	86,	110,	110,	86,	78,	78,	110,	90,	90,	110,	78,	78}},

};


struct WLClass20_Load gluteWattTable[] =
{

#if 0
{{130,	130,	260,	200,	200,	260,	130,	130,	230,	260,	260,	230,	130,	130,	260,	200,	200,	260,	130,	130}},
{{135,	135,	270,	200,	200,	270,	135,	135,	240,	270,	270,	240,	135,	135,	270,	200,	200,	270,	135,	135}},
{{40,	40,	80,	60,	60,	80,	40,	40,	50,	80,	80,	50,	40,	40,	80,	60,	60,	80,	40,	40}},
{{45,	45,	90,	60,	60,	90,	45,	45,	60,	90,	90,	60,	45,	45,	90,	60,	60,	90,	45,	45}},
{{50,	50,	100,	80,	80,	100,	50,	50,	70,	100,	100,	70,	50,	50,	100,	80,	80,	100,	50,	50}},
{{55,	55,	110,	80,	80,	110,	55,	55,	80,	110,	110,	80,	55,	55,	110,	80,	80,	110,	55,	55}},
{{60,	60,	120,	100,	100,	120,	60,	60,	90,	120,	120,	90,	60,	60,	120,	100,	100,	120,	60,	60}},
{{65,	65,	130,	100,	100,	130,	65,	65,	100,	130,	130,	100,	65,	65,	130,	100,	100,	130,	65,	65}},
{{70,	70,	140,	110,	110,	140,	70,	70,	110,	140,	140,	110,	70,	70,	140,	110,	110,	140,	70,	70}},
{{75,	75,	150,	110,	110,	150,	75,	75,	120,	150,	150,	120,	75,	75,	150,	110,	110,	150,	75,	75}},
{{80,	80,	160,	120,	120,	160,	80,	80,	130,	160,	160,	130,	80,	80,	160,	120,	120,	160,	80,	80}},
{{85,	85,	170,	120,	120,	170,	85,	85,	140,	170,	170,	140,	85,	85,	170,	120,	120,	170,	85,	85}},
{{90,	90,	180,	130,	130,	180,	90,	90,	150,	180,	180,	150,	90,	90,	180,	130,	130,	180,	90,	90}},
{{95,	95,	190,	130,	130,	190,	95,	95,	160,	190,	190,	160,	95,	95,	190,	130,	130,	190,	95,	95}},
{{100,	100,	200,	150,	150,	200,	100,	100,	170,	200,	200,	170,	100,	100,	200,	150,	150,	200,	100,	100}},
{{105,	105,	210,	150,	150,	210,	105,	105,	180,	210,	210,	180,	105,	105,	210,	150,	150,	210,	105,	105}},
{{110,	110,	220,	160,	160,	220,	110,	110,	190,	220,	220,	190,	110,	110,	220,	160,	160,	220,	110,	110}},
{{115,	115,	230,	160,	160,	230,	115,	115,	200,	230,	230,	200,	115,	115,	230,	160,	160,	230,	115,	115}},
{{120,	120,	240,	180,	180,	240,	120,	120,	210,	240,	240,	210,	120,	120,	240,	180,	180,	240,	120,	120}},
{{125,	125,	250,	180,	180,	250,	125,	125,	220,	250,	250,	220,	125,	125,	250,	180,	180,	250,	125,	125}},
#else
// 1
	{{ 1,  	1,	 10,	 7,	 7,	 10,	 1,	 1,	5,	10,	10,	5,	 1,	 1,	 10,	 5,	 5,	 10,	 1,	 1 }},
// 2
	{{ 3,  	3,	 11,	 7,	 7,	 11,	3,	3,	8,	11,	11,	8,	3,	3,	 11,	 8,	 8,	 11,	 3,	 3 }},
// 3
	{{ 4, 	4,	 13,	 10,	10,	13,	4,	4,	9,	13,	13,	9,	4,	4,	13,	9,	 9,	 13,	 4,	 4 }},
// 4
	{{ 6, 	6,	 14,	 10,	10,	14,	6,	6,	11,	14,	14,	11,	6,	6,	14,	11,	 11,	 14,	 6,	 6 }},
// 5
	{{ 7, 	7,	 15,	 13,	13,	15,	7,	7,	12,	15,	15,	12,	7,	7,	15,	12,	 12,	 15,	 7,	 7 }},
// 6
	{{ 8, 	8,	 15,	 13,	13,	15,	8,	8,	13,	15,	15,	13,	8,	8,	15,	13,	 13,	 15,	 8,	 8 }},
// 7
	{{ 8, 	8,	16,	14,	14,	16,	8,	8,	14,	16,	16,	14,	8,	8,	16,	14,	14,	16,	 8,	 8 }},
// 8
	{{ 9, 	9,	16,	14,	14,	16,	9,	9,	15,	16,	16,	15,	9,	9,	16,	15,	15,	16,	 9,	 9 }},
// 9
	{{ 10, 	10,	17,	15,	15,	17,	10,	10,	15,	17,	17,	15,	10,	10,	17,	15,	15,	17,	 10,	 10 }},
// 10
	{{ 10, 	10,	17,	15,	15,	17,	10,	10,	16,	17,	17,	16,	10,	10,	17,	16,	16,	17,	 10,	 10 }},
// 11
	{{ 11,	11,	18,	15,	15,	18,	11,	11,	16,	18,	18,	16,	11,	11,	18,	16,	16,	18,	 11,	 11 }},
// 12
	{{ 12, 	12,	19,	15,	15,	19,	12,	12,	17,	19,	19,	17,	12,	12,	19,	17,	17,	19,	 12,	 12 }},
// 13
	{{ 12, 	12,	19,	16,	16,	19,	12,	12,	17,	19,	19,	17,	12,	12,	19,	17,	17,	19,	 12,	 12 }},
// 14
	{{ 13, 	13,	20,	16,	16,	20,	13,	13,	18,	20,	20,	18,	13,	13,	20,	18,	18,	20,	 13,	 13 }},
// 15
	{{ 14, 	14,	20,	17,	17,	20,	14,	14,	19,	20,	20,	19,	14,	14,	20,	19,	19,	20,	14,	14 }},
// 16
	{{ 14, 	14,	21,	17,	17,	21,	14,	14,	19,	21,	21,	19,	14,	14,	21,	19,	19,	21,	14,	14 }},
// 17
	{{ 15, 	15,	21,	18,	18,	21,	15,	15,	20,	21,	21,	20,	15,	15,	21,	20,	20,	21,	15,	15 }},
// 18
	{{ 15, 	15,	21,	18,	18,	21,	15,	15,	20,	21,	21,	20,	15,	15,	21,	20,	20,	21,	15,	15 }},
// 19
	{{ 15,	15,	21,	19,	19,	21,	15,	15,	21,	21,	21,	21,	15,	15,	21,	21,	21,	21,	15,	15 }},
// 20
	{{ 16,	16,	22,	19,	19,	22,	16,	16,	21,	22,	22,	21,	16,	16,	22,	21,	21,	22,	16,	16 }}
#endif

};


struct WLClass20_Load obhWattTable[]=
{
	{{ 1,	 1,	 4,	 4,	 7,	 7,	 8,	 8,	10,	10,	10,	10,	 8,	 8,	 7,	 7,	 4,	 4,	 1,	 1}},
	{{ 1,	 1,	 4,	 4,	 8,	 8,	10,	10,	11,	11,	11,	11,	10,	10,	 8,	 8,	 4,	 4,	 1,	 1}},
	{{ 1,	 1,	 7,	 7,	10,	10,	11,	11,	12,	12,	12,	12,	11,	11,	10,	10,	 7,	 7,	 1,	 1}},
	{{ 4,	 4,	 7,	 7,	10,	10,	12,	12,	13,	13,	13,	13,	12,	12,	10,	10,	 7,	 7,	 4,	 4}},
	{{ 4,	 4,	 8,	 8,	11,	11,	13,	13,	14,	14,	14,	14,	13,	13,	11,	11,	 8,	 8,	 4,	 4}},
	{{ 4,	 4,	 8,	 8,	11,	11,	13,	13,	14,	14,	14,	14,	13,	13,	11,	11,	 8,	 8,	 4,	 4}},
	{{ 7,	 7,	10,	10,	11,	11,	13,	13,	15,	15,	15,	15,	13,	13,	11,	11,	10,	10,	 7,	 7}},
	{{ 7,	 7,	10,	10,	12,	12,	14,	14,	15,	15,	15,	15,	14,	14,	12,	12,	10,	10,	 7,	 7}},
	{{ 7,	 7,	10,	10,	12,	12,	14,	14,	16,	16,	16,	16,	14,	14,	12,	12,	10,	10,	 7,	 7}},
	{{ 7,	 7,	11,	11,	14,	14,	15,	15,	16,	16,	16,	16,	15,	15,	14,	14,	11,	11,	 7,	 7}},
	{{ 8,	 8,	11,	11,	13,	13,	15,	15,	17,	17,	17,	17,	15,	15,	13,	13,	11,	11,	 8,	 8}},
	{{ 8,	 8,	12,	12,	14,	14,	15,	15,	18,	18,	18,	18,	15,	15,	14,	14,	12,	12,	 8,	 8}},
	{{ 8,	 8,	12,	12,	14,	14,	16,	16,	18,	18,	18,	18,	16,	16,	14,	14,	12,	12,	 8,	 8}},
	{{ 8,	 8,	13,	13,	15,	15,	16,	16,	19,	19,	19,	19,	16,	16,	15,	15,	13,	13,	 8,	 8}},
	{{10,	10,	13,	13,	15,	15,	17,	17,	19,	19,	19,	19,	17,	17,	15,	15,	13,	13,	10,	10}},
	{{10,	10,	14,	14,	15,	15,	18,	18,	20,	20,	20,	20,	18,	18,	15,	15,	14,	14,	10,	10}},
	{{11,	11,	14,	14,	16,	16,	18,	18,	21,	21,	21,	21,	18,	18,	16,	16,	14,	14,	11,	11}},
	{{11,	11,	15,	15,	18,	18,	20,	20,	24,	24,	24,	24,	20,	20,	18,	18,	15,	15,	11,	11}},
	{{12,	12,	15,	15,	18,	18,	20,	20,	30,	30,	30,	30,	20,	20,	18,	18,	15,	15,	12,	12}},
	{{12,	12,	16,	16,	19,	19,	22,	22,	30,	30,	30,	30,	22,	22,	19,	19,	16,	16,	12,	12}}
};

void CFpcApp::Performance_One_Big_Hill(void)
{
	InitWorkoutPhase01(20);
	InitWorkoutPhase02_pobh(setup->Segments, obhWattTable);//(20 segment, no timing table, indexed workload table)

	if (0 == update->Segment_time)		update->Segment_time = 60;

	rt->exception_result = EXCEPTION_CONTINUE;	
	//while(rt->total_workout_time_tick > 0 && rt->exception_result == EXCEPTION_CONTINUE)
	while(rt->exception_result == EXCEPTION_CONTINUE)
	{
		//for each segment
		update->Segment_time = rt->segmentTime_Table[rt->segment_index];
 		rt->segment_time_tick = FP_TICKS_PER_SECOND * update->Segment_time;
		if(exception->cmd.hr_cruise == 0)
		{
			rt->workLoad.current_load_level = rt->workLoad_Table[rt->segment_index];
			//can only be one of both
			//update->Workload_level = rt->workLoad.current_load_level;
			//update->Workload_level = setup->Workload_level;
			AdjustMachineWorkLoad(INDEXED_TARGET_LOAD_ADJ_WLClass20, 0);
		}
		
		while(rt->segment_time_tick > 0 && rt->exception_result == EXCEPTION_CONTINUE)
		{
			//get realtime heartrate
			//rt->current_heart_rate = lcb->hr->DisplayHeartRate;
			rt->current_heart_rate = update->Heart_rate;

			//handling the Exceptions		
			rt->exception_result = ExceptionHandler(PERFORMANCE_ONE_BIG_HILL);
			//end of handling the Exceptions			
			if(rt->exception_result == EXCEPTION_COOLDOWN)break;
			if(rt->exception_result == EXCEPTION_BREAK)break;
			//end of handling the Exceptions
			
			if(exception->cmd.pause == 0){
				/////20130409/////////
				//Workout Time Update
				if(rt->segment_time_tick > 0){
					rt->segment_time_tick--;
				}
				if(rt->segment_time_tick==0){
					rt->segment_index++;
				}
				if(rt->total_workout_time_tick > 0){
					rt->total_workout_time_tick --;
				}
				/////////////////////
				//DataScreen Time / Bar Update
				//timer elapsed 1 sec after 100 tick count;
				if(rt->tick_1sec_per100ms >0)
					rt->tick_1sec_per100ms--;	
				if(rt->tick_1sec_per100ms == 0){
					rt->tick_1sec_per100ms = 
						rt->tick_1sec_per100ms_reload;
					Update_Workout_Time_Elapsed();
					Update_Workout_Time_Remaining();
					Update_GUI_bar();
				}
				/////20130409/////////
				if(exception->cmd.hr_cruise == 1)HR_Cruise();
			}
			WaitForProgramTick();//timer tick will com once 100ms
		}
	}

	EndofProgram(PERFORMANCE_ONE_BIG_HILL);	
}


struct WLClass2_load wr_Table[]=
{
	{{ 1,  1,}},
	{{ 1,  4,}},
	{{ 1,  7,}},
	{{ 1,  8,}},
	{{ 1, 10,}},
	{{ 3, 11,}},
	{{ 4, 12,}},
	{{ 5, 13,}},
	{{ 7, 14,}},
	{{ 7, 14,}},
	{{ 8, 15,}},
	{{ 9, 15,}},
	{{10, 16,}},
	{{10, 16,}},
	{{11, 17,}},
	{{11, 18,}},
	{{12, 18,}},
	{{12, 19,}},
	{{13, 19,}},
	{{14, 20,}}
};

void CFpcApp::Performance_Hill_Intervals(void)
{

#if 1
	InitWorkoutPhase01(20);
	InitWorkoutPhase02_hi(2, wr_Table);
#endif


	if (0 == update->Segment_time)		update->Segment_time = 60;

	rt->exception_result = EXCEPTION_CONTINUE;	
	//while(rt->total_workout_time_tick > 0 && rt->exception_result == EXCEPTION_CONTINUE)
	while(rt->exception_result == EXCEPTION_CONTINUE)
	{	
		//for each segment
		update->Segment_time = rt->segmentTime_Table[rt->segment_index];
 		rt->segment_time_tick = FP_TICKS_PER_SECOND * update->Segment_time;
		if(exception->cmd.hr_cruise == 0){
			rt->workLoad.current_load_level = //this is for load adjustment
				rt->workLoad_Table[rt->segment_index];
			//can only be one of both
			//update->Workload_level = rt->workLoad.current_load_level;
			//update->Workload_level = setup->Workload_level;
			AdjustMachineWorkLoad(INDEXED_TARGET_LOAD_ADJ_WLClass2,0);
		}

		while((rt->segment_time_tick > 0) && (rt->exception_result == EXCEPTION_CONTINUE)){			
			//get realtime heartrate
			//rt->current_heart_rate = lcb->hr->DisplayHeartRate;
			rt->current_heart_rate = update->Heart_rate;

			//handling the Exceptions		
			rt->exception_result = ExceptionHandler(PERFORMANCE_HILL_INTERVALS);
			//end of handling the Exceptions			
			if(rt->exception_result == EXCEPTION_COOLDOWN)break;
			if(rt->exception_result == EXCEPTION_BREAK)break;
			//end of handling the Exceptions
			
			if(exception->cmd.pause == 0){
				/////20130409/////////
				//Workout Time Update
				if(rt->segment_time_tick > 0){
					rt->segment_time_tick--;
				}
				if(rt->segment_time_tick==0){
					rt->segment_index++;
				}
				if(rt->total_workout_time_tick > 0){
					rt->total_workout_time_tick --;
				}
				/////////////////////
				//DataScreen Time / Bar Update
				//timer elapsed 1 sec after 100 tick count;
				if(rt->tick_1sec_per100ms >0)
					rt->tick_1sec_per100ms--;	
				if(rt->tick_1sec_per100ms == 0){
					rt->tick_1sec_per100ms = 
						rt->tick_1sec_per100ms_reload;
					Update_Workout_Time_Elapsed();
					Update_Workout_Time_Remaining();
					Update_GUI_bar();
				}
				/////20130409/////////
				if(exception->cmd.hr_cruise == 1)HR_Cruise();
			}
			//WorkOutDebugMessgae(1);
			WaitForProgramTick();//timer tick will com once 100ms
		}
	}

	EndofProgram(PERFORMANCE_HILL_INTERVALS);
}

void CFpcApp::Custom_Pace(void)
{
#if 1
	InitWorkoutPhase01(30);
	InitWorkoutPhase02_cp();
#endif

	WorkoutLoopType1(CUSTOM_PACE, SEGMENT_LOAD_ADJ, CALC_BY_CURRENT_LOAD_LEVEL);
}

void CFpcApp::Custom_Hills(void)
{  

#if 1
	InitWorkoutPhase01(30);
	InitWorkoutPhase02_ch();
#endif

	WorkoutLoopType1(CUSTOM_HILLS, SEGMENT_LOAD_ADJ, CALC_BY_CURRENT_LOAD_LEVEL);
}

void CFpcApp::Custom_Ultra(void)
{

#if 1
	InitWorkoutPhase01(20);
	InitWorkoutPhase02_cu();
#endif


	if (0 == update->Segment_time)		update->Segment_time = 60;

	rt->load_adj_mode = SEGMENT_LOAD_ADJ;
	rt->watt_calc_mod = CALC_BY_CURRENT_LOAD_LEVEL;
	rt->exception_result = EXCEPTION_CONTINUE;	

	//while(rt->total_workout_time_tick > 0 && rt->exception_result == EXCEPTION_CONTINUE)
	while(rt->exception_result == EXCEPTION_CONTINUE)
	//while((rt->Distance_metric < rt->target_workout_distance) && (rt->exception_result == EXCEPTION_CONTINUE))
	{
		//for each segment
		update->Segment_time = rt->segmentTime_Table[rt->segment_index];
 		rt->segment_time_tick = FP_TICKS_PER_SECOND * update->Segment_time;
		rt->workLoad.current_load_level = rt->workLoad_Table[rt->segment_index];
		//this will update the WorkLoad Diasplay
		update->Workload_level = rt->workLoad.current_load_level;
		if(exception->cmd.hr_cruise == 0)
		{
			AdjustMachineWorkLoad(SEGMENT_LOAD_ADJ, 0);
		}
		while(rt->segment_time_tick > 0 && rt->exception_result == EXCEPTION_CONTINUE)
		{	
			//get realtime heartrate
			//rt->current_heart_rate = lcb->hr->DisplayHeartRate;	
			rt->current_heart_rate = update->Heart_rate;

			//handling the Exceptions		
			rt->exception_result = ExceptionHandler(CUSTOM_ULTRA);
			//end of handling the Exceptions			
			if(rt->exception_result == EXCEPTION_COOLDOWN)
				break;
			if(rt->exception_result == EXCEPTION_BREAK)
				break;
			//end of handling the Exceptions
			
			//Message center:
			//‧	You are doing a CUSTOM ULTRA workout.
			//‧	Changes to PACE or WORKLOAD affect the current segment only.
			//‧	PAUSE workout at any time by pressing STOP/PAUSE. 
			//‧	Care should be used when stepping on or stepping off the equipment.
			//‧	To learn more about this product, go to TRUEFITNESS.com 
		
			// Workout time:
			//‧	This is decided by the entered Workout time in set up screen #322-#323. 

			//Segment description:
			//‧	Workload:	Workload changes only affect the current segment.
			//‧	Pace:		Pace changes only affect the current segment.	

			//todo: get work_load through looking up the table with segment_index and level been setup
			//set the work_load and the display the pace	
			if(exception->cmd.pause == 0)
			{	
				//real time update data

				/////20130409/////////
				//Workout Time Update
				if(rt->segment_time_tick > 0){
					rt->segment_time_tick--;
				}
				if(rt->segment_time_tick==0){
					rt->segment_index++;
				}
				if(rt->total_workout_time_tick > 0){
					rt->total_workout_time_tick --;
				}
				/////////////////////
				//DataScreen Time / Bar Update
				//timer elapsed 1 sec after 100 tick count;
				if(rt->tick_1sec_per100ms >0)
					rt->tick_1sec_per100ms--;	
				if(rt->tick_1sec_per100ms == 0)
				{
					rt->tick_1sec_per100ms = 
						rt->tick_1sec_per100ms_reload;
					Update_Workout_Time_Elapsed();
					Update_Workout_Time_Remaining();
					UpdateCoolDownPace(CUSTOM_ULTRA);
					Update_GUI_bar();
				}
				/////20130409/////////
				if(exception->cmd.hr_cruise == 1)
					HR_Cruise();
			}
			//if(rt->Distance_metric >= rt->target_workout_distance)break;
			WaitForProgramTick();//timer tick will com once 100ms	
		}		
	}

	EndofProgram(CUSTOM_ULTRA);
}

void CFpcApp::Custom_HRC_Intervals(void)
{      
	unsigned char i;

#if 1
	InitWorkoutPhase01(20);
	InitWorkoutPhase02_1pre_segSegment_chrci();
#endif

	//FPC machine state/mode
	rt->watt_calc_mod = CALC_BY_CURRENT_LOAD_LEVEL;
	rt->load_adj_mode = SEGMENT_LOAD_ADJ;
	rt->segment_index = 0;
	AdjustMachineWorkLoad(SEGMENT_LOAD_ADJ,0);
			
	//start with warm up mode
	rt->workout_mode = WARM_UP_MODE;
	rt->exception_result = EXCEPTION_CONTINUE;	
	exception->cmd.work_hr_reached = 0;
	exception->cmd.rest_hr_reached = 0;

	if (0 == update->Segment_time)		update->Segment_time = 60;


	while(rt->total_workout_time_tick > 0 && rt->exception_result == EXCEPTION_CONTINUE)
	{
		//for each segment
		update->Segment_time = rt->segmentTime_Table[rt->segment_index];
 		rt->segment_time_tick = FP_TICKS_PER_SECOND * update->Segment_time;
		rt->workLoad.current_load_level = 
			rt->workLoad_Table[rt->segment_index];
		update->Workload_level = rt->workLoad.current_load_level;

		if(rt->segment_index == 0){
			//update->Target_heart_rate = setup->Work_heart_rate;
			setup->Target_heart_rate = setup->Work_heart_rate;
		}else{
			if(exception->cmd.hr_cruise == 0){
				if(rt->workout_mode == PROGRESS_MODE){
					switch(rt->segment_index%2){
					case 1://WORK_SEGMENT
						setup->Target_heart_rate = setup->Work_heart_rate;
						//AdjustMachineWorkLoad(SEGMENT_LOAD_ADJ,0);	
						break;
					case 0://REST_SEGMENT
						setup->Target_heart_rate = setup->Rest_heart_rate;
						//AdjustMachineWorkLoad(SEGMENT_LOAD_ADJ,0);
						break;
					}
					if(exception->cmd.hr_cruise == 0){
						AdjustMachineWorkLoad(SEGMENT_LOAD_ADJ,0);
					}
				}
			}
		}			

		while((rt->segment_time_tick >0) && (rt->exception_result == EXCEPTION_CONTINUE)){
			//get realtime heartrate
			//rt->current_heart_rate = lcb->hr->DisplayHeartRate;	
			rt->current_heart_rate = update->Heart_rate;
			
			//handling the Exceptions		
			rt->exception_result = ExceptionHandler(CUSTOM_HRC_INTERVALS);

			//end of handling the Exceptions			
			if(rt->exception_result == EXCEPTION_COOLDOWN)break;
			if(rt->exception_result == EXCEPTION_BREAK)break;
			//end of handling the Exceptions
			
			if(rt->current_heart_rate >= setup->Work_heart_rate)
			{
				rt->deltaHR = rt->current_heart_rate - setup->Work_heart_rate;
				if(rt->deltaHR > 12){
					rt->exception_result = EXCEPTION_COOLDOWN;
					break;
				}
			}

// 		sprintf(debug_temp,"[%02d][%03d][%03d][%03d][%03d][%01d:%01d][%03d/%03d][%03d:%03d]",
// 			rt->workout_mode,
// 			update->Target_heart_rate,
// 			setup->Work_heart_rate,
// 			setup->Rest_heart_rate,
// 			rt->current_heart_rate,
// 			exception->cmd.work_hr_reached,
// 			exception->cmd.rest_hr_reached,
// 			rt->workLoad.current_load_level,
// 			update->Workload_level,
// 			rt->segment_index,
// 			rt->tick_10sec
// 		);
// 		DisplayString(1,50, debug_temp);

			switch(rt->workout_mode){
			case WARM_UP_MODE:
				//WARM UP mode tring to approach to target hr
				rt->summary_state.did_warmup = 1;
				if(rt->current_heart_rate == 0)break;
				if(rt->current_heart_rate > setup->Work_heart_rate){
					rt->deltaHR = rt->current_heart_rate - setup->Work_heart_rate;
					if(rt->deltaHR < 12){
						CollectWarmUpSummaryData();
						//rt->has_warmup = 1;	
						rt->workout_mode = HR_LOAD_TEST_MODE;
						rt->exception_result = EXCEPTION_STAGE_ADVANCE;
						rt->tick_10sec = rt->tick_10sec_reload;
						rt->segment_index++;
					}else{
						rt->exception_result = EXCEPTION_OVERHR_BREAK;
					}
				}else{
					rt->deltaHR = setup->Work_heart_rate - rt->current_heart_rate;
					if(rt->deltaHR < 10){
						CollectWarmUpSummaryData();
						//rt->has_warmup = 1;	
						rt->workout_mode = HR_LOAD_TEST_MODE;
						rt->exception_result = EXCEPTION_STAGE_ADVANCE;
						rt->tick_10sec = rt->tick_10sec_reload;
						rt->segment_index++;
					}else{
						//adjustment the target HR every 10 second
						///////////////////////////////////////////////////////////////////////////
						//rt->tick_10sec --;	
						//if(rt->tick_10sec==0){
						//	rt->tick_10sec = rt->tick_10sec_reload;
						//	//AdjustMachineWorkLoad(INCREASE1_LOAD_ADJ,0);
						//}
						///////////////////////////////////////////////////////////////////////////						
					}
				}	
				//////////////////////////////////////////////////////
				//DataScreen Time / Bar Update
				//timer elapsed 1 sec after 100 tick count;
				if(rt->tick_1sec_per100ms >0)
					rt->tick_1sec_per100ms--;	
				if(rt->tick_1sec_per100ms == 0){
					rt->tick_1sec_per100ms = 
						rt->tick_1sec_per100ms_reload;
					Update_Workout_Time_Elapsed();
					Update_GUI_bar();
				}
				//////////////////////////////////////////////////////	

				break;
			case HR_LOAD_TEST_MODE:
				rt->summary_state.did_test_load = 1;
				if(exception->cmd.work_hr_reached == 0){
					setup->Target_heart_rate = setup->Work_heart_rate;
					if(rt->current_heart_rate >= setup->Work_heart_rate){
						//mark the work_load needed to reach the work HR						
						rt->deltaHR = rt->current_heart_rate - setup->Work_heart_rate;
						if(rt->deltaHR < 12){
							rt->workLoad.reached_work_hr_level = rt->workLoad.current_load_level;
							exception->cmd.work_hr_reached=1;
							rt->tick_10sec = rt->tick_10sec_reload;
							rt->segment_index++;		
							//PopulateWorkSegment();
						}else{
							rt->exception_result = EXCEPTION_OVERHR_BREAK;
						}
					}else{
						///////////////////////////////////////////////////////////////////////////
						rt->tick_10sec --;	
						if(rt->tick_10sec==0){
							rt->tick_10sec = rt->tick_10sec_reload;
							if(rt->workLoad.current_load_level<30)
								rt->workLoad.current_load_level++;
							update->Workload_level = rt->workLoad.current_load_level;
							
							AdjustMachineWorkLoad(SEGMENT_LOAD_ADJ,0);
							rt->workLoad.reached_work_hr_level = rt->workLoad.current_load_level;
							PopulateBothSegmentLoad();
						}
						///////////////////////////////////////////////////////////////////////////						
					}
				}else{
					//exception->cmd.work_hr_reached = 1
					if(exception->cmd.rest_hr_reached == 0){
						setup->Target_heart_rate = setup->Rest_heart_rate;
						if(rt->current_heart_rate <= setup->Rest_heart_rate){
							//mark the work_load needed to reach the rest HR
							rt->workLoad.reached_rest_hr_level = rt->workLoad.current_load_level;
							exception->cmd.rest_hr_reached=1;
							rt->tick_10sec = rt->tick_10sec_reload;
							rt->segment_index++;
						}else{
							///////////////////////////////////////////////////////////////////////////
							rt->tick_10sec --;	
							if(rt->tick_10sec==0){
								rt->tick_10sec = rt->tick_10sec_reload;
								if(rt->workLoad.current_load_level>2)
									rt->workLoad.current_load_level--;
								update->Workload_level = rt->workLoad.current_load_level;
								AdjustMachineWorkLoad(SEGMENT_LOAD_ADJ,0);
								rt->workLoad.reached_rest_hr_level = rt->workLoad.current_load_level;
								PopulateRestSegmentLoad();			
							}
							///////////////////////////////////////////////////////////////////////////							
						}
					}
				}
				if(exception->cmd.work_hr_reached == 1){
					if(exception->cmd.rest_hr_reached == 1){
						exception->cmd.work_hr_reached = 0;
						exception->cmd.rest_hr_reached = 0;

						rt->workout_mode 	 = PROGRESS_MODE;
						rt->exception_result = EXCEPTION_STAGE_ADVANCE;
						rt->tick_10sec	 = rt->tick_10sec_reload;

						rt->timer_tick 	 = rt->timer_tick_reload;
						rt->tick_10sec	 = rt->tick_10sec_reload;

						for(i=rt->segment_index;i < rt->workLoad_TableUsed;i++){
							switch(rt->segment_index %2){
							case 1://WORK_SEGMENT
								//rt->workLoad_Table[i] = rt->workLoad.reached_work_hr_level;
								break;
							case 0://REST_SEGMENT:
								rt->workLoad_Table[i] = rt->workLoad.reached_rest_hr_level;
								break;
							}
						}
					}
				}
				//////////////////////////////////////////////////////
				//DataScreen Time / Bar Update
				//timer elapsed 1 sec after 100 tick count;
				if(rt->tick_1sec_per100ms >0)
					rt->tick_1sec_per100ms--;	
				if(rt->tick_1sec_per100ms == 0){
					rt->tick_1sec_per100ms = 
						rt->tick_1sec_per100ms_reload;
					Update_Workout_Time_Elapsed();
					Update_Workout_Time_Remaining();
					Update_GUI_bar();
				}
				//////////////////////////////////////////////////////					
				break;
			
			case PROGRESS_MODE:
				rt->summary_state.did_workout = 1;
				if(exception->cmd.pause == 0){
					if(exception->cmd.hr_cruise == 0){
						switch(rt->segment_index %2){
						case 1://WORK_SEGMENT
							///////////////////////////////////////////////////////////////////////////	
							setup->Target_heart_rate = setup->Work_heart_rate;
							rt->tick_10sec --;	
							if(rt->tick_10sec==0){
								if(rt->current_heart_rate < setup->Work_heart_rate){
									rt->tick_10sec = rt->tick_10sec_reload;
									if(rt->workLoad.current_load_level<30)
										rt->workLoad.current_load_level++;
									update->Workload_level = rt->workLoad.current_load_level;
									AdjustMachineWorkLoad(SEGMENT_LOAD_ADJ,0);
									rt->workLoad.reached_work_hr_level = rt->workLoad.current_load_level;
									PopulateWorkSegmentLoad();
								}
							}
							///////////////////////////////////////////////////////////////////////////							
							break;
						case 0://REST_SEGMENT:
							///////////////////////////////////////////////////////////////////////////	
							setup->Target_heart_rate = setup->Rest_heart_rate;
							rt->tick_10sec --;	
							if(rt->tick_10sec==0){
								rt->tick_10sec = rt->tick_10sec_reload;
								if(rt->current_heart_rate > setup->Rest_heart_rate){
									if(rt->workLoad.current_load_level>2)
										rt->workLoad.current_load_level--;
									update->Workload_level = rt->workLoad.current_load_level;
									AdjustMachineWorkLoad(SEGMENT_LOAD_ADJ,0);
									rt->workLoad.reached_rest_hr_level = rt->workLoad.current_load_level;
									PopulateRestSegmentLoad();
								}								
							}
							///////////////////////////////////////////////////////////////////////////							
							break;
						}
					}					
					/////20130409/////////
					//Workout Time Update
					if(rt->segment_time_tick > 0){
						rt->segment_time_tick--;
					}
					if(rt->segment_time_tick==0){
						rt->segment_index++;
					}
					if(rt->total_workout_time_tick > 0){
						rt->total_workout_time_tick --;
					}
					/////////////////////
					//DataScreen Time / Bar Update
					//timer elapsed 1 sec after 100 tick count;
					if(rt->tick_1sec_per100ms >0)
						rt->tick_1sec_per100ms--;	
					if(rt->tick_1sec_per100ms == 0){
						rt->tick_1sec_per100ms = 
							rt->tick_1sec_per100ms_reload;
						Update_Workout_Time_Elapsed();
						Update_Workout_Time_Remaining();
						Update_GUI_bar();
					}
					/////20130409/////////
					if(exception->cmd.hr_cruise == 1)
						HR_Cruise();
				}				
				break;
			default:
				//error handling
				break;
			}
			WaitForProgramTick();//timer tick will com once 100ms
			if(rt->exception_result == EXCEPTION_STAGE_ADVANCE){
				rt->exception_result = EXCEPTION_CONTINUE;
				break;
			}			
		}
	}

	EndofProgram(CUSTOM_HRC_INTERVALS);
}


unsigned char CFpcApp::ExceptionHandler(unsigned char for_program)
{
	unsigned char i;


///////////////////////	
	//if a stop cmd is issued from GUI
	if(exception->cmd.stop == 1)
	{
		exception->cmd.stop = 0;
		return EXCEPTION_BREAK;
	}


///////////////////////	
	//if a cool down cmd is issued from GUI
	if(exception->cmd.cool_down == 1)
	{
		printf("(%s %d) exception->cmd.cool_down == 1\n", __FILE__, __LINE__);
		exception->cmd.cool_down = 0;
		return EXCEPTION_COOLDOWN;
	}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
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
				AdjustMachineWorkLoad(INDEXED_TARGET_LOAD_ADJ_WLClass2, 0);
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

			//for CARDIO360_xx
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
					
				//case CUSTOM_PACE:
				//	for(i=rt->segment_index;i<rt->total_segment;i++){
				//		rt->workLoad_Table[i] = rt->workLoad.current_load_level;
				//	}
				//	//rt->workLoad_Table[rt->segment_index] = rt->workLoad.current_load_level;
				//	AdjustMachineWorkLoad(SEGMENT_LOAD_ADJ, rt->segment_index);
				//	break;

				case HRC_INTERVALS:
					rt->adjusted_Target_Workload = rt->workLoad.current_load_level;
					PopulateWorkLoad_Intervals_hrci3();
					AdjustMachineWorkLoad(SEGMENT_LOAD_ADJ, 0);
					break;

				//case HRC_DISTANCE:
				//	rt->adjusted_Target_Workload = rt->workLoad.current_load_level;
				//	PopulateWorkLoad_Intervals3();
				//	AdjustMachineWorkLoad(SEGMENT_LOAD_ADJ,0);
				//	break;	

				case MANUAL_MANUAL:
					for(i=rt->segment_index; i < rt->total_segment;i++)
					{
						rt->workLoad_Table[i] = rt->workLoad.current_load_level;
					}
					//rt->workLoad_Table[rt->segment_index] = rt->workLoad.current_load_level;
					AdjustMachineWorkLoad(SEGMENT_LOAD_ADJ, 0);
					break;

				default:
					//updated and auto populate the load segments
					for(i = rt->segment_index; i <rt->total_segment; i++)
					{
						rt->workLoad_Table[i] = rt->workLoad.current_load_level;
					}
					AdjustMachineWorkLoad(SEGMENT_LOAD_ADJ, 0);
					break;
				}
				break;	
				//AdjustMachineWorkLoad(INCREASE_LOAD_ADJ, rt->Target_Workload_level);
				//break;
			}			
			//sprintf(mach.debug_buffer,"Stride UP %d",rt->Target_Workload);
			//DisplayString(190,200, mach.debug_buffer);
			//delay_100us(50);
			//rt->Target_Workload = 0;
		}//else{
		//	AdjustMachineWorkLoad(INCREASE1_LOAD_ADJ, 0);
		//}
	}



///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
	if(exception->cmd.stride_up == 1)
	{
 		exception->cmd.stride_up = 0;
		if(rt->Target_Stride > 0)
		{
			lcb->data->StrideLength = rt->Target_Stride / 5;
			rt->Target_Stride = 0;
		}
		else
		{
			if(lcb->data->StrideLength < MAX_STRIDE_LENGTH)	lcb->data->StrideLength += 1;
		}
		if(lcb->data->StrideLength > 0)
		{
			Set_Stride_Motor_Position(lcb->data->StrideLength);
		}
	}



///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
	//Pace	Pace changes affect the current segment only.
	if(exception->cmd.pace_up == 1)
	{
		exception->cmd.pace_up = 0;
		if(rt->Target_Pace_Level > 0)
		{
			//exception->cmd.work_load_up = 0;
			setup->Pace_level = rt->Target_Pace_Level;
			switch(rt->pace_adj_mode)
			{
			case INDEXED_TARGET_PACE_ADJ_WLClass2:
				setup->Pace_level = rt->Target_Pace_Level;
				UpdateIndexedPaceTable();
				//printf("(%s %d) INDEXED_TARGET_PACE_ADJ_WLClass2\n", __FILE__, __LINE__);
				break;

			case INDEXED_TARGET_PACE_ADJ_WLClass20:
				//rt->workLoad.current_pace_level = rt->Target_Pace;
				setup->Pace_level = rt->Target_Pace_Level;
				UpdateIndexedPaceTable_WLCLASS20();
				//AdjustMachinePace(INDEXED_PACE_ADJ, rt->segment_index);
				break;

			default:
				switch(for_program)
				{
				case CUSTOM_HILLS:
					for(i=rt->segment_index;i < rt->workLoad_TableUsed;i++)
					{
						rt->workPace_Table[i] = GetWorkPace_of_ppi(rt->Target_Pace_Level);
					}
					//update->Pace_level = rt->Target_Pace_Level;
					break;
					
				case CUSTOM_PACE:
					rt->workPace_Table[rt->segment_index] = GetWorkPace_of_ppi(rt->Target_Pace_Level);
					//update->Pace_level = rt->Target_Pace_Level;
					break;
				
				case CUSTOM_ULTRA:
					rt->workPace_Table[rt->segment_index] = GetWorkPace_of_ppi(rt->Target_Pace_Level);
					//update->Pace_level = rt->Target_Pace_Level;
					break;
				}
				//AdjustMachinePace(INCREASE_PACE_ADJ, rt->Target_Pace_Level);
			}

			rt->Target_Pace_Level = 0;
		}
		else
		{
			//AdjustMachinePace(INCREASE_PACE_ADJ, 0);
		}
	}

	return EXCEPTION_CONTINUE;
}	

void CFpcApp::AdjustMachineWorkLoad(unsigned char adj_type, unsigned char specify_level)
{
	if (1 == exception->cmd.hr_cruise)
	{
		printf("(%s %d) AdjustMachineWorkLoad(%d %d %d)\n", __FILE__, __LINE__, adj_type, rt->workLoad.current_load_level, rt->workLoad_Table[rt->segment_index]);
	}


	switch(adj_type)
	{
	case SEGMENT_LOAD_ADJ:
		//according to the update->Workload_level 
		Set_WorkLoad_Motor_Position(rt->workLoad.current_load_level);
		break;
		
 	case SEGMENT_INDEX_LOAD_ADJ:
 		//according to the update->Workload_level 
 		Set_WorkLoad_Motor_Position(rt->workLoad_Table[rt->segment_index]);
 		break;

	//add for Weight loss Rolling Hills by Simon@20120325
 	case INDEXED_TARGET_LOAD_ADJ_WLClass2:
 		Set_WorkLoad_Motor_Position(rt->workLoad_Table[rt->segment_index]);
 		break;

	//add for Weight loss Rolling Hills by Simon@20120325
 	case INDEXED_TARGET_LOAD_ADJ_WLClass3:
 		Set_WorkLoad_Motor_Position(rt->workLoad_Table[rt->segment_index]);
 		break;

	//add for Weight loss Rolling Hills by Simon@20120325
 	case INDEXED_TARGET_LOAD_ADJ_WLClass20:
 		Set_WorkLoad_Motor_Position(rt->workLoad_Table[rt->segment_index]);
 		break;

	default:
		printf("(%s %d) AdjustMachineWorkLoad(%d %d)\n", __FILE__, __LINE__, adj_type, specify_level);
		break;
	}

}



//rt->Watts = tables->Get_Watt(rt->workLoad_Table[rt->segment_index], rpm);
//rt->Watts = tables->Get_Watt(rt->workLoad.current_load_level, rpm);



void CFpcApp::HR_Cruise(void)
{
#define DELTA_CROUSE_HR	2

	unsigned char i;
	unsigned char calc_level = 1;
 	unsigned char cruise_hr = 0;
	unsigned char adjust_required = 0;	


///////////////////////////////////////////////////////////
	if (setup->Work_mode == WP_RANDOM_HILLS_RUN || setup->Work_mode == WP_SINGLE_HILL_RUN 
		|| setup->Work_mode == WP_ROLLING_HILLS_RUN  || setup->Work_mode == WP_GLUTE_BUSTER_RUN || setup->Work_mode == WP_CARDIO_CHALLENGE_RUN)
	{
		rt->watt_calc_mod = CALC_BY_INDEXED_LOAD_LEVEL;
	}
	else
	{
		rt->watt_calc_mod = CALC_BY_CURRENT_LOAD_LEVEL;
	}


	if (0 == setup->Target_heart_rate)
		setup->Target_heart_rate = 180;

	rt->current_heart_rate = update->Heart_rate;
	update->Target_heart_rate = setup->Target_heart_rate;
	cruise_hr = setup->Target_heart_rate;

	if(rt->current_heart_rate > cruise_hr)
	{
		rt->deltaHR = rt->current_heart_rate - cruise_hr;
		if(rt->deltaHR > 12)
		{
			printf("(%s %d) EXCEPTION_OVERHR_BREAK\n", __FILE__, __LINE__);
			rt->exception_result = EXCEPTION_OVERHR_BREAK;
			return;
		}
	}

	adjust_required = 0;
	if(rt->current_heart_rate > cruise_hr + DELTA_CROUSE_HR)		adjust_required = 1;
	if(rt->current_heart_rate < cruise_hr - DELTA_CROUSE_HR)		adjust_required = 1;


/*
	if(adjust_required == 1)
	{
		if(rt->tick_30sec > 0)									rt->tick_30sec --;
	}
	else
	{
		rt->tick_30sec = rt->tick_30sec_reload;
	}
*/
	if(rt->tick_30sec > 0)		rt->tick_30sec --;
	else						rt->tick_30sec = rt->tick_30sec_reload;

	if(rt->tick_30sec == 0)
	{
		rt->tick_30sec = rt->tick_30sec_reload;
		adjust_required = 0;
		calc_level = 0;

		if(rt->current_heart_rate > cruise_hr + DELTA_CROUSE_HR)
		{
			//rt.base_cruise_watt = Watts_Calc_cruise();
			//rt->base_cruise_watt 
			calc_level = GetLower10WattWorkLoad_cruise();
			adjust_required = 1;
		}
		else if(rt->current_heart_rate < cruise_hr - DELTA_CROUSE_HR)
		{
			calc_level = GetHiger10WattWorkLoad_cruise();
			adjust_required = 1;
		}

#if 0
		if(rt->current_heart_rate < cruise_hr - DELTA_CROUSE_HR)
		{
			calc_level = GetHiger10WattWorkLoad_cruise();
			//calc_level = GetHiger10WattWorkLoad();
			adjust_required = 1;
		}
#endif // 0



printf("(%s %d) HR_Cruise(), adjust_required=%d calc_level=%d\n", __FILE__, __LINE__, adjust_required, calc_level);
		if(adjust_required == 1 && calc_level > 0)
		{
			for(i = rt->segment_index; i < rt->workLoad_TableUsed; i++)
			{
				rt->workLoad_Table_cruise[i] = calc_level;
			}
			rt->workLoad.current_load_level = calc_level;

// jason note 可能有誤
			//rt->workLoad_Table[rt->segment_index]
			update->Workload_level = rt->workLoad.current_load_level; //added by simon@20130509

			// SEGMENT_INDEX_LOAD_ADJ
			AdjustMachineWorkLoad(SEGMENT_LOAD_ADJ, 0);
			Update_GUI_bar();
		}

		if (!adjust_required)
			printf("(%s %d) 0 AdjustMachineWorkLoad(SEGMENT_LOAD_ADJ)\n", __FILE__, __LINE__);
		else	
			printf("(%s %d) 1 AdjustMachineWorkLoad(SEGMENT_LOAD_ADJ, %d)\n", __FILE__, __LINE__, rt->workLoad.current_load_level);
	}

	///////////////////////////////////////////////////////////////////////////
}




		//rt->Watts = tables->Get_Watt(rt->workLoad_Table[rt->segment_index], rpm);
		//rt->Watts = tables->Get_Watt(rt->workLoad.current_load_level, rpm);



#define MAG 1000.00F

//


//
// [JayLee 20214-11-13] define for auto stride funstion
//
#define TX2_MAX_RPM				200
#define STRIDE_MOVING_1_LEVEL_TIME_SEC		1.1
#define AUTO_STRIDE_SAMPLE_RATE_1Sx5CNT		5
#define AUTO_STRIDE_SAMPLE_RATE_1Sx10CNT	10
//
#define RPM_CHANGE_RANGE_LEVEL_1_VALUE		11
#define RPM_CHANGE_RANGE_LEVEL_2_VALUE		8
#define CHANGE_STRIDE_COUNT_FOR_RPM_LEVEL_1	8
#define CHANGE_STRIDE_COUNT_FOR_RPM_LEVEL_2	9


//total rpm sample rate
#define AUTO_STRIDE_TOTAL_SAMPLE_RATE		AUTO_STRIDE_SAMPLE_RATE_1Sx10CNT
//
// average first got rpm, and this result to average next all rpm, ex.(10 sampled rpm, and setting this value=5):
// average = ( [ (1+2+3+4+5) / 5] + [6 ] + [7] + [8] + [9] + [10]) / 6
#define AUTO_STRIDE_FIRST_AVERAGE_VALUE		AUTO_STRIDE_SAMPLE_RATE_1Sx5CNT
//
//
#define TOTAL_AUTO_STRIDE_LEVEL_LIST		11
#define MAX_RPM_AUTO_STRIDE_VALUE		54

const struct AUTO_STRIDE_TB{
	unsigned char rpm_limit;
	unsigned char stride;
} NewAutoStrideTable[TOTAL_AUTO_STRIDE_LEVEL_LIST] = {
	//rpm	stride
	{ 39,	32 },	// less than 39 or more
	{ 44,	34 },	// 44 -40
	{ 49,	36 },	// 49 - 45
	{ 54,	38 },	// 54 - 50
	{ 59,	40 },	// 59 - 55
	{ 64,	42 },	// 64 - 60
	{ 69,	44 },	// 69 - 65
	{ 74,	46 },	// 74 - 70
	{ 79,	48 },	// 79 - 75
	{ 84,	50 },	// 84 - 80
	{ 90,	52 }	// 90 - 85
	// higher than 91 or more
};
//
static unsigned short		Sports_Auto_Stride_Rpm[AUTO_STRIDE_TOTAL_SAMPLE_RATE];
static unsigned char		Sports_Auto_Stride_Sample_Cnt = 0;
static unsigned char		BeforeNonAutoStride = TRUE;
static unsigned char		Stride_Moving = 0;
static unsigned char		Sports_Auto_Stride_Calc_Flag = FALSE;
static unsigned short		BeforeAveragerRpm = 0;
//
//
unsigned short GetAveragerRpm( unsigned short *RpmArray, unsigned char TotalRpm)
{
    unsigned short	rpm = RpmArray[0];
    unsigned char	a;
    for( a = 1; a < TotalRpm; a++)
    	rpm += RpmArray[a];
    return rpm / TotalRpm;
}
//
//
unsigned short CheckRpmDifferenceValue( unsigned short rpm1, unsigned short rpm2)
{
    if( rpm1 > rpm2 )
    	return rpm1 - rpm2;
    return rpm2 - rpm1;
}
//
// get stride value by rpm
unsigned char GetStrideValue( unsigned short rpm)
{
    unsigned char a;

    // reference rpm value to get stride value 
    for( a = 0; a < TOTAL_AUTO_STRIDE_LEVEL_LIST; a++)
    {
	if( (unsigned short)rpm <= NewAutoStrideTable[a].rpm_limit )
	    return NewAutoStrideTable[a].stride;
    }
    return MAX_RPM_AUTO_STRIDE_VALUE;
}
//
// get stride level range from stride-1 to stride-2
unsigned char GetStrideMovingTimeSec( unsigned char stride1, unsigned char stride2)
{
    unsigned char a, inx1 = 0, inx2 = 0;

    if( stride1 == stride2 )
    	return 0;
    
    for( a = 0; a < TOTAL_AUTO_STRIDE_LEVEL_LIST; a++)
    {
	if( NewAutoStrideTable[a].stride == stride1 )
	    inx1 = a+1;
	if( NewAutoStrideTable[a].stride == stride2 )
	    inx2 = a+1;
    }
    if( stride1 >= MAX_RPM_AUTO_STRIDE_VALUE )
	inx1 = TOTAL_AUTO_STRIDE_LEVEL_LIST + 1;
    if( stride2 >= MAX_RPM_AUTO_STRIDE_VALUE )
    	inx2 = TOTAL_AUTO_STRIDE_LEVEL_LIST + 1;

    if( inx1 > inx2 )
	return (unsigned char)((float)((inx1 - inx2) * STRIDE_MOVING_1_LEVEL_TIME_SEC));
    //
    return (unsigned char)((float)((inx2 - inx1) * STRIDE_MOVING_1_LEVEL_TIME_SEC));
}





// [JayLee 2014-11-12] 1 sec to run this function
void	CFpcApp::Calculate(void)
{
	// 代謝當量 MET, Metabolic Equivalent
	unsigned char i;
    unsigned short	temp_short = 0;
	unsigned long integer, total_base = 0;
	float delta_distance = 0.00F;
	unsigned short rpm = update->Pace_RPM;

	float correction_factor = 0.00F;
	float calc_speed = 0.00F;
	float VO2 = 0.00F;
	float weight_factor = 0.00F;


	if (setup->Weight == 0)
		setup->Weight = (unsigned short)(150.00F / 2.2F), setup->weight = (float)setup->Weight;

    if( (0 == rpm) || exception->cmd.pause )
	{
		update->Watts = 0;
		rt->Calories_HR = 0.00F;
	}
	else
	{
		if (1 == product_type)
		{
	    if( rpm > 120 )
	    	rpm = 11;
	    else if (rpm >= 10)
	    	rpm = (rpm - 10) / 10;
	    else
		rpm = 0;
		}
		else
		{
	    if( rpm > 120 )
		rpm = 11;
	    else if( rpm >= 30 )
		rpm = (rpm - 10) / 10;
	    else
		rpm = 0;
		}

		if (rt->watt_calc_mod)
			rt->Watts = tables->Get_Watt(rt->workLoad_Table[rt->segment_index], rpm);
		else	
			rt->Watts = tables->Get_Watt(rt->workLoad.current_load_level, rpm);
		update->Watts = (unsigned short)rt->Watts;

		if (1 == product_type)
		{
			VO2 = 3.5F + (float)update->Watts / ((float)setup->Weight * 0.07F);
			rt->Calories_HR = VO2 * (float)setup->Weight * 0.3F;
		}
		else
			rt->Calories_HR  = (9.00F * (float)rt->Watts) + 120.00F;
	}

	rt->Calories += rt->Calories_HR / 3600.00F;	// 卡路里


	update->Calories_per_hour_1000cal = ((unsigned short)rt->Calories_HR) / 1000;
	update->Calories_per_hour = ((unsigned short)rt->Calories_HR) - update->Calories_per_hour_1000cal * 1000;
	if(rt->Calories > 1000.00F)
	{
		update->Calories_burned_1000cal = (unsigned short)rt->Calories / 1000;
		update->Calories_burned = (unsigned short)rt->Calories - update->Calories_burned_1000cal * 1000;
	}
	else
	{
		update->Calories_burned_1000cal = 0;
		update->Calories_burned = (unsigned short)rt->Calories;
	}

	if (1 == product_type)
		rt->Mets_metric = (float)VO2 / 3.50F;
	else
		rt->Mets_metric = rt->Calories_HR / (float)setup->Weight;

	rt->Mets_imperial = rt->Mets_metric / 0.45F;


// jason note	
	//if(sys->unit_mode == METRIC)
	if(1)
	{
		update->Mets = (unsigned short)(rt->Mets_metric);
	}
	else
	{
		// 英制
		update->Mets = (unsigned short)rt->Mets_imperial;
	}


///////////////////////////////////////////////////////////
// 計算 delta_distance
	if (1 == product_type)
	{
		if (update->Watts > 0)
		{
			calc_speed = 0.000000101117F * powf((float)update->Watts, 3.00F) - 0.00014F * powf((float)update->Watts, 2.00F) + 0.08461F * (float)update->Watts + 7.53913781F;

			if (150.00F >= (float)setup->Weight * 2.2F)
				weight_factor = powf((150.00F - (float)setup->Weight * 2.2F) / 100, 2.00F / 3.00F);
			else
				weight_factor = powf(((float)setup->Weight * 2.2F - 150.00F) / 100, 2.00F / 3.00F);

			correction_factor = calc_speed / (float)update->Watts;

			if ((unsigned short)((float)setup->Weight * 2.2F) > 150)
				mph = calc_speed - weight_factor + correction_factor;
			else if ((unsigned short)((float)setup->Weight * 2.2F) < 150)
				mph = calc_speed + weight_factor + correction_factor;
			else
				mph = calc_speed + correction_factor;

			delta_distance = mph / 3600.00F * Mile_KM_ratio;
		}
		else
		{
			delta_distance = 0.00F;
		}
	}
	else
	{
		//distance incrementation
		delta_distance = rt->Calories_HR * Mile_KM_ratio / (107.00F * 3600.00F);
	}

	rt->Distance_metric += delta_distance;
	rt->Distance_imperial = rt->Distance_metric / Mile_KM_ratio;

	//unit convertion, KM
	integer = (unsigned short)(rt->Distance_metric * MAG);

	//integer = (unsigned short)(3.85*1000);
	update->Distance_km_i  = integer / (unsigned short)MAG;
	update->Distance_km_f = integer-update->Distance_km_i * (unsigned short)MAG;

	// Mile
	integer = (unsigned short)(rt->Distance_imperial * MAG);
	//integer = (unsigned short)(3.85 * 1000);
	update->Distance_mi_i  = integer / (unsigned short)MAG;
	update->Distance_mi_f  = integer - update->Distance_mi_i * (unsigned short)MAG;


///////////////////////////////////////////////////////////
	//printf("(%s %d) VO2=%f\n", __FILE__, __LINE__, VO2);
	//printf("(%s %d) calc_speed=%f\n", __FILE__, __LINE__, calc_speed);
	//printf("(%s %d) update->Watts=%d\n", __FILE__, __LINE__, update->Watts);
	//printf("(%s %d) Distance_m=%d\n", __FILE__, __LINE__, update->Distance_km_i * 1000 + update->Distance_km_f);



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

		//if(rt->Distance_metric < MIN_DISTANCE)	rt->Distance_metric = MIN_DISTANCE;
		//if(rt->Distance_metric > MAX_DISTANCE)	rt->Distance_metric = MAX_DISTANCE;
		rt->RemainDistance_metric = rt->target_workout_distance - (rt->Distance_metric - rt->start_Distance_metric);
	}
	else
	{
		rt->RemainDistance_metric = rt->target_workout_distance;
	}

	//unit conversion
	rt->RemainDistance_imperial = rt->RemainDistance_metric / Mile_KM_ratio;
	//unit convertion
	integer = (unsigned short)(rt->RemainDistance_metric * MAG);	
	//integer = (unsigned short)(3.85*1000);
	update->Distance_remaining_km_i  = integer / (unsigned short)MAG;
	update->Distance_remaining_km_f = integer - update->Distance_remaining_km_i * (unsigned short)MAG;
	//unit convertion
	integer = (unsigned short)(rt->RemainDistance_imperial * MAG);
	//integer = (unsigned short)(3.85*1000);
	update->Distance_remaining_mi_i = integer / (unsigned short)MAG;
	update->Distance_remaining_mi_f = integer - update->Distance_remaining_mi_i * (unsigned short)MAG;


// update->Heart_rate = lcb->hr->DisplayHeartRate;
	temp_short = lcb->hr->DisplayHeartRate;


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
			total_base = (rt->elapsed_time + rt->elapsed_time_1000 * 1000) - rt->waste_hr_elapsed_time + 1;

			rt->Average_Heart_Rate_base = total_base;
			if (0 != total_base)
				rt->Average_Heart_Rate = (unsigned short)(rt->Average_Heart_Rate_ACC / total_base);
			else
				printf("(%s %d) BUG()\n", __FILE__, __LINE__);
		}
	}


	temp_short = lcb->Get_RPM();
	if(temp_short > rt->Max_Rpm)
		rt->Max_Rpm = temp_short;


	if(exception->cmd.pause == 0)
	{
		if(temp_short == 0)
		{
			rt->waste_rpm_elapsed_time ++;
		}
		else
		{
			rt->Average_Rpm_ACC += temp_short;
			total_base = rt->elapsed_time + rt->elapsed_time_1000 * 1000 - rt->waste_rpm_elapsed_time + 1;
			rt->Average_Rpm_base = total_base;
			if (0 != total_base)
				rt->Average_Rpm = (unsigned short)(rt->Average_Rpm_ACC / total_base);
			else
				printf("(%s %d) BUG()\n", __FILE__, __LINE__);
		}
	}

    // [JayLee 2014-11-13] add new auto stride code flow
	if(exception->cmd.auto_stride == 1)
	{
	float		curr_rpm;
	unsigned short	diff_rpm_value;
	unsigned char	AveragerRpmTotalAmount;
	unsigned char	Sports_Auto_Stride_Len;
	unsigned char	NowStrideLen = lcb->Get_Stride_Motor_Position();

	if( BeforeNonAutoStride == TRUE )
		{
	    // first into auto stride
	    BeforeNonAutoStride = FALSE;
	    rt->Auto_Stride = NowStrideLen;
	    Sports_Auto_Stride_Calc_Flag = FALSE;
	    Sports_Auto_Stride_Sample_Cnt = 0;
	    Stride_Moving = 0;
	    BeforeAveragerRpm = lcb->Get_RPM();
	}

	if( Stride_Moving )
			{
	    Stride_Moving--;
			}		
	else
	{
	    // get current RPM
	    Sports_Auto_Stride_Rpm[Sports_Auto_Stride_Sample_Cnt] = lcb->Get_RPM();

	    if( Sports_Auto_Stride_Rpm[Sports_Auto_Stride_Sample_Cnt] > TX2_MAX_RPM )
		Sports_Auto_Stride_Rpm[Sports_Auto_Stride_Sample_Cnt] = TX2_MAX_RPM;

	    if( Sports_Auto_Stride_Calc_Flag == FALSE )
	    {
		if( GetStrideValue(Sports_Auto_Stride_Rpm[Sports_Auto_Stride_Sample_Cnt]) != rt->Auto_Stride )
		    Sports_Auto_Stride_Calc_Flag = TRUE;
	    }
	    else
	    {
		Sports_Auto_Stride_Sample_Cnt++;

		AveragerRpmTotalAmount = AUTO_STRIDE_TOTAL_SAMPLE_RATE;
		if( Sports_Auto_Stride_Sample_Cnt == CHANGE_STRIDE_COUNT_FOR_RPM_LEVEL_1 )
		{
		    diff_rpm_value = CheckRpmDifferenceValue( GetAveragerRpm( Sports_Auto_Stride_Rpm, CHANGE_STRIDE_COUNT_FOR_RPM_LEVEL_1), BeforeAveragerRpm);
		    if(	diff_rpm_value > RPM_CHANGE_RANGE_LEVEL_1_VALUE )
		    {
			Sports_Auto_Stride_Sample_Cnt = AUTO_STRIDE_TOTAL_SAMPLE_RATE;
			AveragerRpmTotalAmount = CHANGE_STRIDE_COUNT_FOR_RPM_LEVEL_1;
		    }
		}
		else if( Sports_Auto_Stride_Sample_Cnt == CHANGE_STRIDE_COUNT_FOR_RPM_LEVEL_2 )
		{
		    diff_rpm_value = CheckRpmDifferenceValue( GetAveragerRpm( Sports_Auto_Stride_Rpm, CHANGE_STRIDE_COUNT_FOR_RPM_LEVEL_2), BeforeAveragerRpm);
		    if( diff_rpm_value > RPM_CHANGE_RANGE_LEVEL_2_VALUE )
		    {
			Sports_Auto_Stride_Sample_Cnt = AUTO_STRIDE_TOTAL_SAMPLE_RATE;
			AveragerRpmTotalAmount = CHANGE_STRIDE_COUNT_FOR_RPM_LEVEL_2;
		    }
		}

		if( Sports_Auto_Stride_Sample_Cnt >= AUTO_STRIDE_TOTAL_SAMPLE_RATE )
		{

		    Sports_Auto_Stride_Sample_Cnt = 0;

		    // average consecutive rpm
		    curr_rpm = 0;

		    if( AUTO_STRIDE_FIRST_AVERAGE_VALUE > 0 )
		    {
			for( i = 0; i < AUTO_STRIDE_FIRST_AVERAGE_VALUE; i++)
			{
			    if( !i )
				curr_rpm = (float)Sports_Auto_Stride_Rpm[i];
			    else
		    		curr_rpm += (float)Sports_Auto_Stride_Rpm[i];
			}
			curr_rpm = curr_rpm/AUTO_STRIDE_FIRST_AVERAGE_VALUE;
		    }

		    //for( i = AUTO_STRIDE_FIRST_AVERAGE_VALUE; i < AUTO_STRIDE_TOTAL_SAMPLE_RATE; i++)
		    for( i = AUTO_STRIDE_FIRST_AVERAGE_VALUE; i < AveragerRpmTotalAmount; i++)
			curr_rpm += (float)Sports_Auto_Stride_Rpm[i];

		    if( AUTO_STRIDE_FIRST_AVERAGE_VALUE > 0 )
			curr_rpm = curr_rpm / ( (AveragerRpmTotalAmount - AUTO_STRIDE_FIRST_AVERAGE_VALUE) + 1);
		    else
			curr_rpm = curr_rpm / AveragerRpmTotalAmount;

		    Sports_Auto_Stride_Len = GetStrideValue( (unsigned short)curr_rpm);

		    // setting new stride
		    if( rt->Auto_Stride != Sports_Auto_Stride_Len )
		    {
			Set_Stride_Motor_Position(Sports_Auto_Stride_Len);
			Stride_Moving = GetStrideMovingTimeSec( rt->Auto_Stride, Sports_Auto_Stride_Len) - 1;
			rt->Auto_Stride = Sports_Auto_Stride_Len;
			Sports_Auto_Stride_Calc_Flag = FALSE;
			BeforeAveragerRpm = (unsigned short)curr_rpm;
		    }
		}
	    }
	}
    }
    else
    {
	BeforeNonAutoStride = TRUE;
	}
}


void CFpcApp::HR_Cruise_distance(void)
{
	unsigned char i,calc_level;
 	unsigned char cruise_hr;
	unsigned char adjust_required = 0;	

	//rt->current_heart_rate = lcb->hr->DisplayHeartRate;
	rt->current_heart_rate = update->Heart_rate;

	update->Target_heart_rate = setup->Target_heart_rate;
	
	cruise_hr = setup->Target_heart_rate;

// 	sprintf(thisMachine.debug_buffer,"In CRUISE %03d:%03d ",rt->current_heart_rate,cruise_hr);
// 	DisplayString(1,40, thisMachine.debug_buffer);
// 	Set_Stride_Motor_Position(DEFAULT_STRIDE_LENGTH);

	if(rt->current_heart_rate > cruise_hr){
		rt->deltaHR = rt->current_heart_rate - cruise_hr;
		if(rt->deltaHR > 12){
			rt->exception_result = EXCEPTION_OVERHR_BREAK;
			//rt->exception_result == EXCEPTION_COOLDOWN
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
	if(adjust_required == 1){
		if(rt->tick_30sec >0)rt->tick_30sec --;
	}else{
		rt->tick_30sec = rt->tick_30sec_reload;
	}
	if(rt->tick_30sec==0){
		calc_level = 0;
		rt->tick_30sec = rt->tick_30sec_reload;
		adjust_required = 0;
		if(rt->current_heart_rate > (cruise_hr+2)){
			calc_level = GetLower10WattWorkLoad();
			adjust_required = 1;
		}
		if(rt->current_heart_rate < (cruise_hr-2)){							
			calc_level = GetHiger10WattWorkLoad();
			adjust_required = 1;
		}
		if(adjust_required == 1){
			for(i=rt->segment_index;i<rt->workLoad_TableUsed;i++){
				rt->workLoad_Table_cruise[i] = calc_level;
			}
			rt->workLoad.current_load_level = calc_level;

// jason note
			update->Workload_level = rt->workLoad.current_load_level; //added by simon@20130509

			AdjustMachineWorkLoad(SEGMENT_LOAD_ADJ,0);

			Update_GUI_bar_distance();
		}
	}
	///////////////////////////////////////////////////////////////////////////
}


void CFpcApp::Set_Stride_Motor_Position(unsigned char position)
{
	unsigned char ch[2];
	unsigned int x =0;
	if(position < MIN_STRIDE_LENGTH || position > MAX_STRIDE_LENGTH)
		return;
	data->StrideLength = position;
	ch[0] = 0x0F;
	ch[1] = Tables->Get_Stride_Target(data->StrideLength-MIN_STRIDE_LENGTH+1);
	//ch[1] = position;
	x = BUILD_UINT16(ch[1],ch[0]);
	printf("Set_Stride_Motor_Position  = %d\n",x);
	if (-1 != ttyUSB0)
	{
		SendCmdB(ttyUSB0,2,0x0024 ,x);
	}
		//SendCmd(ttyUSB0, (int)SERIAL_CMD_WR_STRIDE_POS, 2, ch); chuck modify
}

char BkieWorkloadMaxFlag =0;
char BkieWorkloadMinFlag =0;



void CFpcApp::Set_WorkLoad_Motor_Position(unsigned char position)
{
	unsigned char ch;
			  int adValue;

	if(position < MIN_RESISTANCE_LEVEL || position > MAX_RESISTANCE_LEVEL)
		return;
	//data->ResistanceLevel = position;
	//update->Workload_level = rt->workLoad.current_load_level = position;


	// 發電機
	if (2 == product_type)
	{
		struct gp_pwm_config_s attr;

		if (-1 == pwm1)
			return;
		ch = pGsResist_Level_Table[position - 1];
		attr.duty = ch;
		attr.freq = 20;
		attr.pin_index = 0;
		if (-1 == ioctl(pwm1, PWM_IOCTL_SET_ATTRIBUTE, &attr))				{ printf("PWM_IOCTL_SET_ATTRIBUTE, IOCTL() FAIL !!\n"); }
		if (-1 == ioctl(pwm1, PWM_IOCTL_SET_ENABLE, 1))						{ printf("PWM_IOCTL_SET_ENABLE(1) FAIL\n"); }
		return;
	}

	// SD55
	if (0 == product_type)
	{
		ch = pSD55_Resist_Level_Table[position - 1];

		SendCmdB(ttyUSB0,1,0x0023 ,ch);
		// SendCmd(ttyUSB0, (int)SERIAL_CMD_WR_RESIST_POS, 1, &ch); chuck modify
		return;
	}


	// 拉馬達

	ch = pAsResist_Level_Table[position - 1];
	adValue = ch << 2;
	printf("adValue = %d \n",adValue);
	

	if(position != 30)
	{
	   BkieWorkloadMaxFlag = 0;
	}
	if(position != 1)
	{
	   BkieWorkloadMinFlag = 0;
	}
	
	if(BkieWorkloadMaxFlag == 1 || BkieWorkloadMinFlag == 1)
	{
	    return;
	}
	
	SendCmdB(ttyUSB0,2,0x001B ,adValue);
	
	if(position == 30)
	{
	    BkieWorkloadMaxFlag = 1;
	}
		if(position == 1)
	{
	    BkieWorkloadMinFlag = 1;
	}
	

		
  	SetTimer(CHK_BIKE_MOTO_ERR_TIMER_ID, 5003, 0); //chuck modify 
}

void CFpcApp::HRC_conditioning(void)
{
	unsigned char calc_level = 0;
	unsigned char adjust_required = 0;	
	
	//rt->current_heart_rate = lcb->hr->DisplayHeartRate;
	rt->current_heart_rate = update->Heart_rate;

	update->Target_heart_rate = setup->Target_heart_rate;
	///////////////////////////////////////////////////////////////////////////
	adjust_required = 0;
	if(rt->current_heart_rate > (2+setup->Target_heart_rate)){
		adjust_required = 1;
	}							
	if(rt->current_heart_rate < (setup->Target_heart_rate-2)){							
		adjust_required = 1;
	}
	if(adjust_required == 1){
		if(rt->tick_30sec >0)rt->tick_30sec --;
	}else{
		rt->tick_30sec = rt->tick_30sec_reload;
	}

	if(rt->tick_30sec==0){
		rt->tick_30sec = rt->tick_30sec_reload;
		adjust_required = 0;
		if(rt->current_heart_rate > (2+setup->Target_heart_rate)){
			calc_level = GetLower10WattWorkLoad();
			adjust_required = 1;
		}							
		if(rt->current_heart_rate < (setup->Target_heart_rate-2)){							
			calc_level = GetHiger10WattWorkLoad();
			adjust_required = 1;
		}
		if(adjust_required == 1){
			rt->adjusted_Target_Workload = calc_level;			
			PopulateWorkLoad();

			AdjustMachineWorkLoad(SEGMENT_LOAD_ADJ, 0);

			Update_GUI_bar();
		}
	}
// 	///////////////////////////////////////////////////////////////////////////
// 	rt->tick_30sec --;	
// 	if(rt->tick_30sec==0){
// 		rt->tick_30sec = rt->tick_30sec_reload;
// 		if(rt->current_heart_rate == setup->Target_heart_rate){
// 			;
// 		}else{
// 			adjust_required = 0;
// 			if(rt->current_heart_rate > (2+setup->Target_heart_rate)){
// 				calc_level = GetLower10WattWorkLoad();
// 				adjust_required = 1;
// 			}							
// 			if(rt->current_heart_rate < (setup->Target_heart_rate-2)){							
// 				calc_level = GetHiger10WattWorkLoad();
// 				adjust_required = 1;
// 			}
// 			if(adjust_required == 1){
// 				rt->adjusted_Target_Workload = calc_level;			
// 				PopulateWorkLoad();
// 				AdjustMachineWorkLoad(SEGMENT_LOAD_ADJ,0);
// 				Update_GUI_bar();
// 			}
// 		}
// 		
// 	}
// 	///////////////////////////////////////////////////////////////////////////	
}

void CFpcApp::FadOutCruiseGUI(void)
{
	unsigned short rpm = update->Pace_RPM;


	exception->cmd.hr_cruise = 0;
	rt->base_cruise_watt = 0;
	
	//restore settings
	rt->watt_calc_mod	 = rt->watt_calc_mod_org;
	rt->workLoad.current_load_level = rt->workLoad_Table[rt->segment_index];
		
	setup->Target_heart_rate	 = rt->before_cruise_Target_heart_rate;//rt->normal_Target_heart_rate;
	update->Target_heart_rate	 = setup->Target_heart_rate;
	setup->Workload_level	 = rt->before_cruise_setup_level;

	update->Workload_level = rt->before_cruise_level;

#if 1
	if (rpm >= 30)			rpm = (rpm - 30) / 10;
	else						rpm = 0;
	if (rt->watt_calc_mod)		rt->Watts = tables->Get_Watt(rt->segment_index, rpm);
	else						rt->Watts = tables->Get_Watt(rt->workLoad.current_load_level, rpm);
	update->Watts = rt->Watts;
#else
	update->Watts = (unsigned short)Watts_Calc();	
#endif // 1

	AdjustMachineWorkLoad(SEGMENT_INDEX_LOAD_ADJ, 0);

	//endof restore settings
	Update_GUI_bar();
}

void CFpcApp::HRC_conditioning_hrci(void)
{
	unsigned char calc_level = 0;
	unsigned char adjust_required = 0;	
	///////////////////////////////////////////////////////////////////////////
	adjust_required = 0;
	if(rt->current_heart_rate > (setup->Target_heart_rate+2))
	{
		adjust_required = 1;
	}							
	if(rt->current_heart_rate < (setup->Target_heart_rate-2))
	{
		adjust_required = 1;
	}
	if(adjust_required == 1)
	{
		rt->tick_30sec --;
	}
	else
	{
		rt->tick_30sec = rt->tick_30sec_reload;
	}

	if(rt->tick_30sec==0)
	{
		rt->tick_30sec = rt->tick_30sec_reload;
		adjust_required = 0;
		if(rt->current_heart_rate > (2+setup->Target_heart_rate))
		{
			calc_level = GetLower10WattWorkLoad();
			adjust_required = 1;
		}
		if(rt->current_heart_rate < (setup->Target_heart_rate-2))
		{
			calc_level = GetHiger10WattWorkLoad();
			adjust_required = 1;
		}
		if(adjust_required == 1)
		{
			rt->adjusted_Target_Workload = calc_level;			
			PopulateWorkLoad_Intervals_hrci3();

			AdjustMachineWorkLoad(SEGMENT_LOAD_ADJ,0);

			Update_GUI_bar();
		}
	}
}


unsigned char CFpcApp::OnWorkModeChange(struct work_mode_state_t *state)
{
	static int segments_adjust = 1;

	int i = 0;
	unsigned int target;
	unsigned int source;
	unsigned char key;
	unsigned char shift;

	char vkeyName[32];
	char sourceName[32];
	char targetName[32];
	unsigned char karr;

	source = state->source;
	target = state->target;
	key = state->key;
	shift = state->shift;

	if (target == source && 777 == target)
	{
		if (KS_NUM4 == key)
		{
			buzz_type = shift;
			return 1;
		}
		if (KS_NUM2 == key)
		{
			bl_sleep_type = shift;
			if (1 == bl_sleep_type)
				SetTimer(BL_OFF_TIMER_ID, 1000 * 60 * 15 + 1, BlOffTimer);
			else if (2 == bl_sleep_type)
				SetTimer(BL_OFF_TIMER_ID, 1000 * 60 * 30 + 1, BlOffTimer);
			else if (3 == bl_sleep_type)
				SetTimer(BL_OFF_TIMER_ID, 1000 * 60 * 40 + 1, BlOffTimer);
			else if (4 == bl_sleep_type)
				SetTimer(BL_OFF_TIMER_ID, 1000 * 60 * 60 + 1, BlOffTimer);
			return 1;
		}
		if (KS_NUM1 == key)
		{
			hr_pirority_type = shift;
			return 1;
		}

		return 1;
	}



	if (IN_RUNNING == rt->workout_state)						return 0;
	if (READY_TO_COOL_DOWN == rt->workout_state)				return 0;
	if (READY_TO_SUMMARY == rt->workout_state)				return 0;
	if (0XFF == key)											return 0;
	if (0 != shift)
		{}

///////////////////////////////////////////////////////////
	if (KS_LEFT_TOP == key)									sprintf(vkeyName, "KS_LEFT_TOP"), karr = 0;
	else if (KS_LEFT_CENTER == key)							sprintf(vkeyName, "KS_LEFT_CENTER"), karr = 1;
	else if (KS_LEFT_BOTTOM == key)							sprintf(vkeyName, "KS_LEFT_BOTTOM"), karr = 2;
	else if (KS_RIGHT_TOP == key)								sprintf(vkeyName, "KS_RIGHT_TOP"), karr = 3;
	else if (KS_RIGHT_CENTER == key)							sprintf(vkeyName, "KS_RIGHT_CENTER"), karr = 4;
	else if (KS_RIGHT_BOTTOM == key)							sprintf(vkeyName, "KS_RIGHT_BOTTOM"), karr = 5;
	else if (KS_START == key)									sprintf(vkeyName, "KS_START"), karr = 6;
	else if (KS_STOP == key)									sprintf(vkeyName, "KS_STOP"), karr = 7;
	else if (KS_WORKLOAD_UP == key)							sprintf(vkeyName, "KS_WORKLOAD_UP"), karr = 8;
	else if (KS_WORKLOAD_DOWN == key)						sprintf(vkeyName, "KS_WORKLOAD_DOWN"), karr = 9;
	else if (KS_STRIDE_UP == key)								sprintf(vkeyName, "KS_STRIDE_UP"), karr = 10;
	else if (KS_STRIDE_DOWN == key)							sprintf(vkeyName, "KS_STRIDE_DOWN"), karr = 11;
	else if (KS_NUM0 == key)									sprintf(vkeyName, "KS_NUM0"), karr = 12;
	else if (KS_NUM1 == key)									sprintf(vkeyName, "KS_NUM1"), karr = 13;
	else if (KS_NUM2 == key)									sprintf(vkeyName, "KS_NUM2"), karr = 14;
	else if (KS_NUM3 == key)									sprintf(vkeyName, "KS_NUM3"), karr = 15;
	else if (KS_NUM4 == key)									sprintf(vkeyName, "KS_NUM4"), karr = 16;
	else if (KS_NUM5 == key)									sprintf(vkeyName, "KS_NUM5"), karr = 17;
	else if (KS_NUM6 == key)									sprintf(vkeyName, "KS_NUM6"), karr = 18;
	else if (KS_NUM7 == key)									sprintf(vkeyName, "KS_NUM7"), karr = 19;
	else if (KS_NUM8 == key)									sprintf(vkeyName, "KS_NUM8"), karr = 20;
	else if (KS_NUM9 == key)									sprintf(vkeyName, "KS_NUM9"), karr = 21;
	else if (KS_DELETE == key)									sprintf(vkeyName, "KS_DELETE"), karr = 22;
	else														sprintf(vkeyName, "KS_UNDEF"), karr = 255;

///////////////////////////////////////////////////////////
	if (WP_HOME == source)									sprintf(sourceName, "WP_HOME");
	else if (WP_PACE == source)								sprintf(sourceName, "WP_PACE");
	else if (WP_MANUAL_QUICK_START == source)				sprintf(sourceName, "WP_MANUAL_QUICK_START");
	else if (WP_MANUAL_QUICK_START_RUN == source)			sprintf(sourceName, "WP_MANUAL_QUICK_START_RUN");
	else if (WP_MANUAL_QUICK_START_SUMMARY == source)		sprintf(sourceName, "WP_MANUAL_QUICK_START_SUMMARY");
	else if (WP_CARDIO_360_QUICK_START == source)			sprintf(sourceName, "WP_CARDIO_360_QUICK_START");
	else if (WP_CARDIO_360_QUICK_START_RUN == source)		sprintf(sourceName, "WP_CARDIO_360_QUICK_START_RUN");
	else if (WP_CARDIO_360_QUICK_START_SUMMARY == source)	sprintf(sourceName, "WP_CARDIO_360_QUICK_START_SUMMARY");
	else if (WP_LANG_SELECT == source)						sprintf(sourceName, "WP_LANG_SELECT");
	else if (WP_WORKOUT_FINDER == source)					sprintf(sourceName, "WP_WORKOUT_FINDER");
	else if (WP_C360 == source)								sprintf(sourceName, "WP_C360");
	else if (WP_WL == source)									sprintf(sourceName, "WP_WL");
	else if (WP_HRC == source)								sprintf(sourceName, "WP_HRC");
	else if (WP_PERFORMANCE == source)						sprintf(sourceName, "WP_PERFORMANCE");
	else if (WP_CUSTOM == source)								sprintf(sourceName, "WP_CUSTOM");
	else if (WP_MANUAL_A == source)							sprintf(sourceName, "WP_MANUAL_A");
	else if (WP_MANUAL_B == source)							sprintf(sourceName, "WP_MANUAL_B");
	else if (WP_MANUAL_RUN == source)							sprintf(sourceName, "WP_MANUAL_RUN");
	else if (WP_MANUAL_SUMMARY == source)					sprintf(sourceName, "WP_MANUAL_SUMMARY");
	else if (WP_MANUAL_SAVE == source)						sprintf(sourceName, "WP_MANUAL_SAVE");
	else if (WP_C360_QUICK_START == source)					sprintf(sourceName, "WP_C360_QUICK_START");
	else if (WP_C360_QUICK_START_RUN == source)				sprintf(sourceName, "WP_C360_QUICK_START_RUN");
	else if (WP_C360_QUICK_START_SUMMARY == source)			sprintf(sourceName, "WP_C360_QUICK_START_SUMMARY");
	else if (WP_C360_ARM_SCULPTOR == source)					sprintf(sourceName, "WP_C360_ARM_SCULPTOR");
	else if (WP_C360_ARM_SCULPTOR_RUN == source)				sprintf(sourceName, "WP_C360_ARM_SCULPTOR_RUN");
	else if (WP_C360_ARM_SCULPTOR_SUMMARY == source)		sprintf(sourceName, "WP_C360_ARM_SCULPTOR_SUMMARY");
	else if (WP_C360_ARM_SCULPTOR_SAVE == source)			sprintf(sourceName, "WP_C360_ARM_SCULPTOR_SAVE");
	else if (WP_C360_LEG_SHAPER == source)					sprintf(sourceName, "WP_C360_LEG_SHAPER");
	else if (WP_C360_LEG_SHAPER_RUN == source)				sprintf(sourceName, "WP_C360_LEG_SHAPER_RUN");
	else if (WP_C360_LEG_SHAPER_SUMMARY == source)			sprintf(sourceName, "WP_C360_LEG_SHAPER_SUMMARY");
	else if (WP_C360_LEG_SHAPER_SAVE == source)				sprintf(sourceName, "WP_C360_LEG_SHAPER_SAVE");
	else if (WP_C360_CUSTOM_A == source)						sprintf(sourceName, "WP_C360_CUSTOM_A");
	else if (WP_C360_CUSTOM_B == source)						sprintf(sourceName, "WP_C360_CUSTOM_B");
	else if (WP_C360_CUSTOM_RUN == source)					sprintf(sourceName, "WP_C360_CUSTOM_RUN");
	else if (WP_C360_CUSTOM_SUMMARY == source)				sprintf(sourceName, "WP_C360_CUSTOM_SUMMARY");
	else if (WP_C360_CUSTOM_SAVE == source)					sprintf(sourceName, "WP_C360_CUSTOM_SAVE");
	else if (WP_CALORIE_GOAL == source)						sprintf(sourceName, "WP_CALORIE_GOAL");
	else if (WP_CALORIE_GOAL_RUN == source)					sprintf(sourceName, "WP_CALORIE_GOAL_RUN");
	else if (WP_CALORIE_GOAL_SUMMARY == source)				sprintf(sourceName, "WP_CALORIE_GOAL_SUMMARY");
	else if (WP_CALORIE_GOAL_SAVE == source)					sprintf(sourceName, "WP_CALORIE_GOAL_SAVE");
	else if (WP_GLUTE_BUSTER == source)						sprintf(sourceName, "WP_GLUTE_BUSTER");
	else if (WP_GLUTE_BUSTER_RUN == source)					sprintf(sourceName, "WP_GLUTE_BUSTER_RUN");
	else if (WP_GLUTE_BUSTER_SUMMARY == source)				sprintf(sourceName, "WP_GLUTE_BUSTER_SUMMARY");
	else if (WP_GLUTE_BUSTER_SAVE == source)					sprintf(sourceName, "WP_GLUTE_BUSTER_SAVE");
	else if (WP_LEG_SHAPER_A == source)						sprintf(sourceName, "WP_LEG_SHAPER_A");
	else if (WP_LEG_SHAPER_B == source)						sprintf(sourceName, "WP_LEG_SHAPER_B");
	else if (WP_LEG_SHAPER_RUN == source)						sprintf(sourceName, "WP_LEG_SHAPER_RUN");
	else if (WP_LEG_SHAPER_SUMMARY == source)				sprintf(sourceName, "WP_LEG_SHAPER_SUMMARY");
	else if (WP_LEG_SHAPER_SAVE == source)					sprintf(sourceName, "WP_LEG_SHAPER_SAVE");
	else if (WP_WL_A == source)								sprintf(sourceName, "WP_WL_A");
	else if (WP_WL_B == source)								sprintf(sourceName, "WP_WL_B");
	else if (WP_WL_RUN == source)								sprintf(sourceName, "WP_WL_RUN");
	else if (WP_WL_SUMMARY == source)						sprintf(sourceName, "WP_WL_SUMMARY");
	else if (WP_WL_SAVE == source)							sprintf(sourceName, "WP_WL_SAVE");
	else if (WP_TARGET_HRC_A == source)						sprintf(sourceName, "WP_TARGET_HRC_A");
	else if (WP_TARGET_HRC_B == source)						sprintf(sourceName, "WP_TARGET_HRC_B");
	else if (WP_TARGET_HRC_RUN == source)					sprintf(sourceName, "WP_TARGET_HRC_RUN");
	else if (WP_TARGET_HRC_SUMMARY == source)				sprintf(sourceName, "WP_TARGET_HRC_SUMMARY");
	else if (WP_TARGET_HRC_SAVE == source)					sprintf(sourceName, "WP_TARGET_HRC_SAVE");
	else if (WP_WL_HRC_A == source)							sprintf(sourceName, "WP_WL_HRC_A");
	else if (WP_WL_HRC_B == source)							sprintf(sourceName, "WP_WL_HRC_B");
	else if (WP_WL_HRC_RUN == source)							sprintf(sourceName, "WP_WL_HRC_RUN");
	else if (WP_WL_HRC_SUMMARY == source)					sprintf(sourceName, "WP_WL_HRC_SUMMARY");
	else if (WP_WL_HRC_SAVE == source)						sprintf(sourceName, "WP_WL_HRC_SAVE");
	else if (WP_AEROBIC_HRC_A == source)						sprintf(sourceName, "WP_AEROBIC_HRC_A");
	else if (WP_AEROBIC_HRC_B == source)						sprintf(sourceName, "WP_AEROBIC_HRC_B");
	else if (WP_AEROBIC_HRC_RUN == source)					sprintf(sourceName, "WP_AEROBIC_HRC_RUN");
	else if (WP_AEROBIC_HRC_SUMMARY == source)				sprintf(sourceName, "WP_AEROBIC_HRC_SUMMARY");
	else if (WP_AEROBIC_HRC_SAVE == source)					sprintf(sourceName, "WP_AEROBIC_HRC_SAVE");
	else if (WP_INTERVAL_HRC_A == source)						sprintf(sourceName, "WP_INTERVAL_HRC_A");
	else if (WP_INTERVAL_HRC_B == source)						sprintf(sourceName, "WP_INTERVAL_HRC_B");
	else if (WP_INTERVAL_HRC_RUN == source)					sprintf(sourceName, "WP_INTERVAL_HRC_RUN");
	else if (WP_INTERVAL_HRC_SUMMARY == source)				sprintf(sourceName, "WP_INTERVAL_HRC_SUMMARY");
	else if (WP_INTERVAL_HRC_SAVE == source)					sprintf(sourceName, "WP_INTERVAL_HRC_SAVE");
	else if (WP_DISTANCE_RUN == source)						sprintf(sourceName, "WP_DISTANCE_RUN");
	else if (WP_DISTANCE_A == source)							sprintf(sourceName, "WP_DISTANCE_A");
	else if (WP_DISTANCE_B == source)							sprintf(sourceName, "WP_DISTANCE_B");
	else if (WP_DISTANCE_SAVE == source)						sprintf(sourceName, "WP_DISTANCE_SAVE");
	else if (WP_DISTANCE_SUMMARY == source)					sprintf(sourceName, "WP_DISTANCE_SUAAMAY");
	else if (WP_PACE_RAMP_A == source)						sprintf(sourceName, "WP_PACE_RAMP_A");
	else if (WP_PACE_RAMP_B == source)						sprintf(sourceName, "WP_PACE_RAMP_B");
	else if (WP_PACE_RAMP_SAVE == source)						sprintf(sourceName, "WP_PACE_RAMP_SAVE");
	else if (WP_PACE_RAMP_SUMMARY == source)					sprintf(sourceName, "WP_PACE_RAMP_SUMMARY");
	else if (WP_PACE_RAMP_RUN == source)						sprintf(sourceName, "WP_PACE_RAMP_RUN");
	else if (WP_PACE_INTERVALS_A == source)					sprintf(sourceName, "WP_PACE_INTERVALS_A");
	else if (WP_PACE_INTERVALS_B == source)					sprintf(sourceName, "WP_PACE_INTERVALS_B");
	else if (WP_PACE_INTERVALS_SAVE == source)				sprintf(sourceName, "WP_PACE_INTERVALS_SAVE");
	else if (WP_PACE_INTERVALS_SUMMARY == source)			sprintf(sourceName, "WP_PACE_INTERVALS_SUMMARY");
	else if (WP_PACE_INTERVALS_RUN == source)					sprintf(sourceName, "WP_PACE_INTERVALS_RUN");
	else if (WP_WALK_INTERVALS_A == source)					sprintf(sourceName, "WP_WALK_INTERVALS_A");
	else if (WP_WALK_INTERVALS_B == source)					sprintf(sourceName, "WP_WALK_INTERVALS_B");
	else if (WP_WALK_INTERVALS_SAVE == source)				sprintf(sourceName, "WP_WALK_INTERVALS_SAVE");
	else if (WP_WALK_INTERVALS_RUN == source)					sprintf(sourceName, "WP_WALK_INTERVALS_RUN");
	else if (WP_WALK_INTERVALS_SUMMARY == source)			sprintf(sourceName, "WP_WALK_INTERVALS_SUMMARY");
	else if (WP_CUSTOM_UTRA_A == source)						sprintf(sourceName, "WP_CUSTOM_UTRA_A");
	else if (WP_CUSTOM_UTRA_B == source)						sprintf(sourceName, "WP_CUSTOM_UTRA_B");
	else if (WP_CUSTOM_UTRA_C == source)						sprintf(sourceName, "WP_CUSTOM_UTRA_C");
	else if (WP_CUSTOM_UTRA_RUN == source)					sprintf(sourceName, "WP_CUSTOM_UTRA_RUN");
	else if (WP_CUSTOM_UTRA_SAVE == source)					sprintf(sourceName, "WP_CUSTOM_UTRA_SAVE");
	else if (WP_CUSTOM_UTRA_SUMMARY == source)				sprintf(sourceName, "WP_CUSTOM_UTRA_SUMMARY");
	else if (WP_RANDOM_HILLS == source)						sprintf(sourceName, "WP_WP_RANDOM_HILLS");
	else if (WP_RANDOM_HILLS_RUN == source)					sprintf(sourceName, "WP_RANDOM_HILLS_RUN");
	else if (WP_RANDOM_HILLS_SAVE == source)					sprintf(sourceName, "WP_RANDOM_HILLS_SAVE");
	else if (WP_RANDOM_HILLS_SUMMARY == source)				sprintf(sourceName, "WP_RANDOM_HILLS_SUMMARY");
	else														sprintf(sourceName, "WP_UNDEF");
///////////////////////////////////////////////////////////
	if (WP_HOME  == target)									sprintf(targetName, "WP_HOME");
	else if (WP_PACE == target)								sprintf(targetName, "WP_PACE");
	else if (WP_MANUAL_QUICK_START  == target)				sprintf(targetName, "WP_MANUAL_QUICK_START");
	else if (WP_MANUAL_QUICK_START_RUN  == target)			sprintf(targetName, "WP_MANUAL_QUICK_START_RUN");
	else if (WP_MANUAL_QUICK_START_SUMMARY  == target)		sprintf(targetName, "WP_MANUAL_QUICK_START_SUMMARY");
	else if (WP_CARDIO_360_QUICK_START  == target)			sprintf(targetName, "WP_CARDIO_360_QUICK_START");
	else if (WP_CARDIO_360_QUICK_START_RUN  == target)		sprintf(targetName, "WP_CARDIO_360_QUICK_START_RUN");
	else if (WP_CARDIO_360_QUICK_START_SUMMARY  == target)	sprintf(targetName, "WP_CARDIO_360_QUICK_START_SUMMARY");
	else if (WP_LANG_SELECT  == target)						sprintf(targetName, "WP_LANG_SELECT");
	else if (WP_WORKOUT_FINDER  == target)					sprintf(targetName, "WP_WORKOUT_FINDER");
	else if (WP_C360  == target)								sprintf(targetName, "WP_C360");
	else if (WP_WL  == target)									sprintf(targetName, "WP_WL");
	else if (WP_HRC  == target)								sprintf(targetName, "WP_HRC");
	else if (WP_PERFORMANCE  == target)						sprintf(targetName, "WP_PERFORMANCE");
	else if (WP_CUSTOM  == target)								sprintf(targetName, "WP_CUSTOM");
	else if (WP_MANUAL_A  == target)							sprintf(targetName, "WP_MANUAL_A");
	else if (WP_MANUAL_B  == target)							sprintf(targetName, "WP_MANUAL_B");
	else if (WP_MANUAL_RUN  == target)							sprintf(targetName, "WP_MANUAL_RUN");
	else if (WP_MANUAL_SUMMARY  == target)					sprintf(targetName, "WP_MANUAL_SUMMARY");
	else if (WP_MANUAL_SAVE  == target)						sprintf(targetName, "WP_MANUAL_SAVE");
	else if (WP_C360_QUICK_START  == target)					sprintf(targetName, "WP_C360_QUICK_START");
	else if (WP_C360_QUICK_START_RUN  == target)				sprintf(targetName, "WP_C360_QUICK_START_RUN");
	else if (WP_C360_QUICK_START_SUMMARY  == target)			sprintf(targetName, "WP_C360_QUICK_START_SUMMARY");
	else if (WP_C360_ARM_SCULPTOR  == target)					sprintf(targetName, "WP_C360_ARM_SCULPTOR");
	else if (WP_C360_ARM_SCULPTOR_RUN  == target)				sprintf(targetName, "WP_C360_ARM_SCULPTOR_RUN");
	else if (WP_C360_ARM_SCULPTOR_SUMMARY  == target)		sprintf(targetName, "WP_C360_ARM_SCULPTOR_SUMMARY");
	else if (WP_C360_ARM_SCULPTOR_SAVE  == target)			sprintf(targetName, "WP_C360_ARM_SCULPTOR_SAVE");
	else if (WP_C360_LEG_SHAPER  == target)					sprintf(targetName, "WP_C360_LEG_SHAPER");
	else if (WP_C360_LEG_SHAPER_RUN  == target)				sprintf(targetName, "WP_C360_LEG_SHAPER_RUN");
	else if (WP_C360_LEG_SHAPER_SUMMARY  == target)			sprintf(targetName, "WP_C360_LEG_SHAPER_SUMMARY");
	else if (WP_C360_LEG_SHAPER_SAVE  == target)				sprintf(targetName, "WP_C360_LEG_SHAPER_SAVE");
	else if (WP_C360_CUSTOM_A  == target)						sprintf(targetName, "WP_C360_CUSTOM_A");
	else if (WP_C360_CUSTOM_B  == target)						sprintf(targetName, "WP_C360_CUSTOM_B");
	else if (WP_C360_CUSTOM_RUN  == target)					sprintf(targetName, "WP_C360_CUSTOM_RUN");
	else if (WP_C360_CUSTOM_SUMMARY  == target)				sprintf(targetName, "WP_C360_CUSTOM_SUMMARY");
	else if (WP_C360_CUSTOM_SAVE  == target)					sprintf(targetName, "WP_C360_CUSTOM_SAVE");
	else if (WP_CALORIE_GOAL  == target)						sprintf(targetName, "WP_CALORIE_GOAL");
	else if (WP_CALORIE_GOAL_RUN  == target)					sprintf(targetName, "WP_CALORIE_GOAL_RUN");
	else if (WP_CALORIE_GOAL_SUMMARY  == target)				sprintf(targetName, "WP_CALORIE_GOAL_SUMMARY");
	else if (WP_CALORIE_GOAL_SAVE  == target)					sprintf(targetName, "WP_CALORIE_GOAL_SAVE");
	else if (WP_GLUTE_BUSTER  == target)						sprintf(targetName, "WP_GLUTE_BUSTER");
	else if (WP_GLUTE_BUSTER_RUN  == target)					sprintf(targetName, "WP_GLUTE_BUSTER_RUN");
	else if (WP_GLUTE_BUSTER_SUMMARY  == target)				sprintf(targetName, "WP_GLUTE_BUSTER_SUMMARY");
	else if (WP_GLUTE_BUSTER_SAVE  == target)					sprintf(targetName, "WP_GLUTE_BUSTER_SAVE");
	else if (WP_LEG_SHAPER_A  == target)						sprintf(targetName, "WP_LEG_SHAPER_A");
	else if (WP_LEG_SHAPER_B  == target)						sprintf(targetName, "WP_LEG_SHAPER_B");
	else if (WP_LEG_SHAPER_RUN  == target)						sprintf(targetName, "WP_LEG_SHAPER_RUN");
	else if (WP_LEG_SHAPER_SUMMARY  == target)				sprintf(targetName, "WP_LEG_SHAPER_SUMMARY");
	else if (WP_LEG_SHAPER_SAVE  == target)					sprintf(targetName, "WP_LEG_SHAPER_SAVE");
	else if (WP_WL_A  == target)								sprintf(targetName, "WP_WL_A");
	else if (WP_WL_B  == target)								sprintf(targetName, "WP_WL_B");
	else if (WP_WL_RUN  == target)								sprintf(targetName, "WP_WL_RUN");
	else if (WP_WL_SUMMARY  == target)						sprintf(targetName, "WP_WL_SUMMARY");
	else if (WP_WL_SAVE  == target)							sprintf(targetName, "WP_WL_SAVE");
	else if (WP_TARGET_HRC_A  == target)						sprintf(targetName, "WP_TARGET_HRC_A");
	else if (WP_TARGET_HRC_B  == target)						sprintf(targetName, "WP_TARGET_HRC_B");
	else if (WP_TARGET_HRC_RUN  == target)					sprintf(targetName, "WP_TARGET_HRC_RUN");
	else if (WP_TARGET_HRC_SUMMARY  == target)				sprintf(targetName, "WP_TARGET_HRC_SUMMARY");
	else if (WP_TARGET_HRC_SAVE  == target)					sprintf(targetName, "WP_TARGET_HRC_SAVE");
	else if (WP_WL_HRC_A  == target)							sprintf(targetName, "WP_WL_HRC_A");
	else if (WP_WL_HRC_B  == target)							sprintf(targetName, "WP_WL_HRC_B");
	else if (WP_WL_HRC_RUN  == target)							sprintf(targetName, "WP_WL_HRC_RUN");
	else if (WP_WL_HRC_SUMMARY  == target)					sprintf(targetName, "WP_WL_HRC_SUMMARY");
	else if (WP_WL_HRC_SAVE  == target)						sprintf(targetName, "WP_WL_HRC_SAVE");
	else if (WP_AEROBIC_HRC_A  == target)						sprintf(targetName, "WP_AEROBIC_HRC_A");
	else if (WP_AEROBIC_HRC_B  == target)						sprintf(targetName, "WP_AEROBIC_HRC_B");
	else if (WP_AEROBIC_HRC_RUN  == target)					sprintf(targetName, "WP_AEROBIC_HRC_RUN");
	else if (WP_AEROBIC_HRC_SUMMARY  == target)				sprintf(targetName, "WP_AEROBIC_HRC_SUMMARY");
	else if (WP_AEROBIC_HRC_SAVE  == target)					sprintf(targetName, "WP_AEROBIC_HRC_SAVE");
	else if (WP_INTERVAL_HRC_A  == target)						sprintf(targetName, "WP_INTERVAL_HRC_A");
	else if (WP_INTERVAL_HRC_B  == target)						sprintf(targetName, "WP_INTERVAL_HRC_B");
	else if (WP_INTERVAL_HRC_RUN  == target)					sprintf(targetName, "WP_INTERVAL_HRC_RUN");
	else if (WP_INTERVAL_HRC_SUMMARY  == target)				sprintf(targetName, "WP_INTERVAL_HRC_SUMMARY");
	else if (WP_INTERVAL_HRC_SAVE  == target)					sprintf(targetName, "WP_INTERVAL_HRC_SAVE");
	else if (WP_DISTANCE_RUN == target)						sprintf(targetName, "WP_DISTANCE_RUN");
	else if (WP_DISTANCE_A == target)							sprintf(targetName, "WP_DISTANCE_A");
	else if (WP_DISTANCE_B == target)							sprintf(targetName, "WP_DISTANCE_B");
	else if (WP_DISTANCE_SAVE == target)						sprintf(targetName, "WP_DISTANCE_SAVE");
	else if (WP_DISTANCE_SUMMARY == target)					sprintf(targetName, "WP_DISTANCE_SUAAMAY");
	else if (WP_PACE_RAMP_A == target)						sprintf(targetName, "WP_PACE_RAMP_A");
	else if (WP_PACE_RAMP_B == target)						sprintf(targetName, "WP_PACE_RAMP_B");
	else if (WP_PACE_RAMP_SAVE == target)						sprintf(targetName, "WP_PACE_RAMP_SAVE");
	else if (WP_PACE_RAMP_SUMMARY == target)					sprintf(targetName, "WP_PACE_RAMP_SUMMARY");
	else if (WP_PACE_RAMP_RUN == target)						sprintf(targetName, "WP_PACE_RAMP_RUN");
	else if (WP_PACE_INTERVALS_A == target)					sprintf(targetName, "WP_PACE_INTERVALS_A");
	else if (WP_PACE_INTERVALS_B == target)					sprintf(targetName, "WP_PACE_INTERVALS_B");
	else if (WP_PACE_INTERVALS_SAVE == target)				sprintf(targetName, "WP_PACE_INTERVALS_SAVE");
	else if (WP_PACE_INTERVALS_SUMMARY == target)				sprintf(targetName, "WP_PACE_INTERVALS_SUMMARY");
	else if (WP_PACE_INTERVALS_RUN == target)					sprintf(targetName, "WP_PACE_INTERVALS_RUN");
	else if (WP_WALK_INTERVALS_A == target)					sprintf(targetName, "WP_WALK_INTERVALS_A");
	else if (WP_WALK_INTERVALS_B == target)					sprintf(targetName, "WP_WALK_INTERVALS_B");
	else if (WP_WALK_INTERVALS_SAVE == target)				sprintf(targetName, "WP_WALK_INTERVALS_SAVE");
	else if (WP_WALK_INTERVALS_RUN == target)					sprintf(targetName, "WP_WALK_INTERVALS_RUN");
	else if (WP_WALK_INTERVALS_SUMMARY == target)			sprintf(targetName, "WP_WALK_INTERVALS_SUMMARY");
	else if (WP_CUSTOM_UTRA_A == target)						sprintf(targetName, "WP_CUSTOM_UTRA_A");
	else if (WP_CUSTOM_UTRA_B == target)						sprintf(targetName, "WP_CUSTOM_UTRA_B");
	else if (WP_CUSTOM_UTRA_C == target)						sprintf(targetName, "WP_CUSTOM_UTRA_C");
	else if (WP_CUSTOM_UTRA_RUN == target)					sprintf(targetName, "WP_CUSTOM_UTRA_RUN");
	else if (WP_CUSTOM_UTRA_SAVE == target)					sprintf(targetName, "WP_CUSTOM_UTRA_SAVE");
	else if (WP_CUSTOM_UTRA_SUMMARY == target)				sprintf(targetName, "WP_CUSTOM_UTRA_SUMMARY");
	else if (WP_RANDOM_HILLS == target)						sprintf(targetName, "WP_RANDOM_HILLS");
	else if (WP_RANDOM_HILLS_RUN == target)					sprintf(targetName, "WP_RANDOM_HILLS_RUN");
	else if (WP_RANDOM_HILLS_SAVE == target)					sprintf(targetName, "WP_RANDOM_HILLS_SAVE");
	else if (WP_RANDOM_HILLS_SUMMARY == target)				sprintf(targetName, "WP_RANDOM_HILLS_SUMMARY");
	else if (WP_CARDIO_360_DEMO_RUN == target)				sprintf(targetName, "WP_CARDIO_360_DEMO_RUN");
	else														sprintf(targetName, "WP_UNDEF");
	printf("OnWorkMode(%d->%d), %s->%s  %s(%d)\n", source, target, sourceName, targetName, vkeyName, karr);


///////////////////////////////////////////////////////////
	if (key == KS_RIGHT_TOP && (WP_HOME == source || WP_CARDIO_360_DEMO_RUN == target))
	{
		memset(summary, 0, sizeof(struct WorkoutSummary));
		memset(update, 0, sizeof(struct RealtimeUpdateData));
		update->Target_heart_rate = setup->Target_heart_rate;

		setup->Work_mode = target;
		exception->cmd.start = 1;
		exception->cmd.stop = 0;
		exception->cmd.pause = 0;
		exception->cmd.resume = 0;
		TellFpcStart();
printf("(%s %d) WM_DBG(START)\n", __FILE__, __LINE__);
		return 1;
	}



///////////////////////////////////////////////////////////
	if (WP_HOME == target || WP_HOME == source)
	{
		memset(summary, 0, sizeof(struct WorkoutSummary));
		memset(update, 0, sizeof(struct RealtimeUpdateData));
		update->Target_heart_rate = setup->Target_heart_rate;

		rt->warmup_Time_elapsed = 0;
		rt->waste_hr_elapsed_time = 0;
		rt->waste_rpm_elapsed_time = 0;
		rt->workout_time = 0;
		rt->workout_time_1000 = 0;
		rt->cooldown.Time_elapsed = 0;
		rt->cooldown.Time_elapsed_1000 = 0;
	}

	if (source != target && target == WP_WORKOUT_FINDER)
	{
		read_setup_ok = 0;
	}
	if (source != target || WP_HOME == target)
	{
		setup->Work_mode = target;
		memset(focus_key, 0XFF, sizeof(focus_key));
		age_focused = 0; weight_focused = 0; workload_focused = 0; distance_focused = 0; time_focused = 0; calorie_focused = 0; gender_focused = 0; pace_focused = 0; segments_focused = 0; hr_focused = 0;
	}

///////////////////////////////////////////////////////////
	if (source != target)
	{
		focus_ok = 1; scroll_idx = 0;
	}
	if (0 == focus_ok && (key == KS_RIGHT_BOTTOM || key == KS_LEFT_BOTTOM) && source == target)
	{
printf("(%s %d) WM_DBG(0==focus_ok)\n", __FILE__, __LINE__);
		return 1;
	}


///////////////////////////////////////////////////////////
	if (target != source &&
		(
			target == WP_MANUAL_A
			|| target == WP_C360_ARM_SCULPTOR
			|| target == WP_C360_LEG_SHAPER
			|| target == WP_C360_CUSTOM_A
			|| target == WP_CALORIE_GOAL
			|| target == WP_GLUTE_BUSTER
			|| target == WP_LEG_SHAPER_A
			|| target == WP_WL_A
			|| target == WP_TARGET_HRC_A
			|| target == WP_WL_HRC_A
			|| target == WP_TARGET_HRC_A
			|| target == WP_AEROBIC_HRC_A
			|| target == WP_INTERVAL_HRC_A
			|| target == WP_CARDIO_CHALLENGE
			|| target == WP_WALK_INTERVALS_A
			|| target == WP_PACE_INTERVALS_A
			|| target == WP_PACE_RAMP_A
			|| target == WP_ROLLING_HILLS
			|| target == WP_HILL_INTVALS
			|| target == WP_SINGLE_HILL
			|| target == WP_RANDOM_HILLS
			|| target == WP_DISTANCE_A
			|| target == WP_FITNESS_A
			|| target == WP_CUSTOM_UTRA_A
		)
	)
	{
		manual_distance_flag = 0;
		age_focused = 0; weight_focused = 0; workload_focused = 0; distance_focused = 0; time_focused = 0; calorie_focused = 0; gender_focused = 0; pace_focused = 0; segments_focused = 0; hr_focused = 0;
		age_focused = 1;
	}
	if (source != target && target == WP_MANUAL_B)
	{
		age_focused = 0; weight_focused = 0; workload_focused = 0; distance_focused = 0; time_focused = 0; calorie_focused = 0; gender_focused = 0; pace_focused = 0; segments_focused = 0; hr_focused = 0;
		time_focused = 1;
	}
	if (source != target && target == WP_LEG_SHAPER_B)
	{
		age_focused = 0; weight_focused = 0; workload_focused = 0; distance_focused = 0; time_focused = 0; calorie_focused = 0; gender_focused = 0; pace_focused = 0; segments_focused = 0; hr_focused = 0;
		pace_focused = 1;
	}
	if (source != target && target == WP_WALK_INTERVALS_B)
	{
		age_focused = 0; weight_focused = 0; workload_focused = 0; distance_focused = 0; time_focused = 0; calorie_focused = 0; gender_focused = 0; pace_focused = 0; segments_focused = 0; hr_focused = 0;
		pace_focused = 1;
	}
	if (source != target && target == WP_PACE_INTERVALS_B)
	{
		age_focused = 0; weight_focused = 0; workload_focused = 0; distance_focused = 0; time_focused = 0; calorie_focused = 0; gender_focused = 0; pace_focused = 0; segments_focused = 0; hr_focused = 0;
		pace_focused = 1;
	}
	if (source != target && target == WP_PACE_RAMP_B)
	{
		age_focused = 0; weight_focused = 0; workload_focused = 0; distance_focused = 0; time_focused = 0; calorie_focused = 0; gender_focused = 0; pace_focused = 0; segments_focused = 0; hr_focused = 0;
		pace_focused = 1;
	}



///////////////////////////////////////////////////////////
// RUN
	if (
		(source == WP_MANUAL_B && target == WP_MANUAL_RUN)
		|| (source == WP_C360_ARM_SCULPTOR && target == WP_C360_ARM_SCULPTOR_RUN)
		|| (source == WP_C360_LEG_SHAPER && target == WP_C360_LEG_SHAPER_RUN)
		|| (source == WP_C360_CUSTOM_B && target == WP_C360_CUSTOM_RUN)
		|| (source == WP_CALORIE_GOAL && target == WP_CALORIE_GOAL_RUN)
		|| (source == WP_GLUTE_BUSTER && target == WP_GLUTE_BUSTER_RUN)
		|| (source == WP_LEG_SHAPER_B && target == WP_LEG_SHAPER_RUN)
		|| (source == WP_WL_B && target == WP_WL_RUN)
		|| (source == WP_TARGET_HRC_B && target == WP_TARGET_HRC_RUN)
		|| (source == WP_WL_HRC_B && target == WP_WL_HRC_RUN)
		|| (source == WP_AEROBIC_HRC_B && target == WP_AEROBIC_HRC_RUN)
		|| (source == WP_INTERVAL_HRC_B && target == WP_INTERVAL_HRC_RUN)
		|| (source == WP_CARDIO_CHALLENGE && target == WP_CARDIO_CHALLENGE_RUN)
		|| (source == WP_WALK_INTERVALS_B && target == WP_WALK_INTERVALS_RUN)
		|| (source == WP_PACE_INTERVALS_B && target == WP_PACE_INTERVALS_RUN)
		|| (source == WP_PACE_RAMP_B && target == WP_PACE_RAMP_RUN)
		|| (source == WP_ROLLING_HILLS && target == WP_ROLLING_HILLS_RUN)
		|| (source == WP_HILL_INTVALS && target == WP_HILL_INTVALS_RUN)
		|| (source == WP_SINGLE_HILL && target == WP_SINGLE_HILL_RUN)
		|| (source == WP_RANDOM_HILLS && target == WP_RANDOM_HILLS_RUN)
		|| (source == WP_DISTANCE_B && target == WP_DISTANCE_RUN)
		|| (source == WP_FITNESS_B && target == WP_FITNESS_RUN)
		|| (source == WP_CUSTOM_UTRA_C && target == WP_CUSTOM_UTRA_RUN)
	)
	{
		setup->Work_mode = target;
		exception->cmd.start = 1;
		exception->cmd.stop = 0;
		exception->cmd.pause = 0;
		exception->cmd.resume = 0;
		TellFpcStart();
printf("(%s %d) WM_DBG(START)\n", __FILE__, __LINE__);
		return 1;
	}


///////////////////////////////////////////////////////////
// SAVE

// OnWorkMode(13012->1301), WP_C360_ARM_SCULPTOR_SAVE->WP_C360_ARM_SCULPTOR  KS_LEFT_BOTTOM(2)
// OnWorkMode(13112->13112), WP_GLUTE_BUSTER_SAVE->WP_GLUTE_BUSTER_SAVE  KS_LEFT_TOP(0)

	if (source == target &&
		(
			target == WP_MANUAL_SAVE
			|| target == WP_C360_ARM_SCULPTOR_SAVE
			|| target == WP_C360_LEG_SHAPER_SAVE
			|| target == WP_C360_CUSTOM_SAVE
			|| target == WP_CALORIE_GOAL_SAVE
			|| target == WP_GLUTE_BUSTER_SAVE
			|| target == WP_LEG_SHAPER_SAVE
			|| target == WP_WL_SAVE
			|| target == WP_TARGET_HRC_SAVE
			|| target == WP_WL_HRC_SAVE
			|| target == WP_AEROBIC_HRC_SAVE
			|| target == WP_INTERVAL_HRC_SAVE
			|| target == WP_CARDIO_CHALLENGE_SAVE
			|| target == WP_WALK_INTERVALS_SAVE
			|| target == WP_PACE_INTERVALS_SAVE
			|| target == WP_PACE_RAMP_SAVE
			|| target == WP_ROLLING_HILLS_SAVE
			|| target == WP_HILL_INTVALS_SAVE
			|| target == WP_SINGLE_HILL_SAVE
			|| target == WP_RANDOM_HILLS_SAVE
			|| target == WP_DISTANCE_SAVE
			|| target == WP_FITNESS_SAVE
			|| target == WP_CUSTOM_UTRA_SAVE
		)
	)
	{
		FILE *fp = 0;
		char tmp[32];

		if (key == KS_LEFT_TOP)
		{
			memcpy(&saved_setup[0], setup, sizeof(struct SetupData));
			saved_setup[0].Work_mode = target / 10;

			fp = fopen("/data/arex/setup.dat", "wb+");
			if (!fp)
			{
				printf("(%s %d) WM_DBG(SAVE 0) FAIL\n", __FILE__, __LINE__);
				return 1;
			}
			setbuf(fp, 0);
			memset(tmp, 0, sizeof(tmp));
			sprintf(tmp, "[%s %s]", __DATE__, __TIME__);
			fwrite(tmp, sizeof(tmp), 1, fp);
			fwrite(saved_setup, sizeof(saved_setup), 1, fp);
			fclose(fp);
			sync();
printf("(%s %d) WM_DBG(SAVE 0, %d %d)\n", __FILE__, __LINE__, setup->Work_mode, saved_setup[0].Work_mode);
			return 1;
		}
		if (key == KS_LEFT_CENTER)
		{
			memcpy(&saved_setup[1], setup, sizeof(struct SetupData));
			saved_setup[1].Work_mode = target / 10;

			fp = fopen("/data/arex/setup.dat", "wb+");
			if (!fp)
			{
				printf("(%s %d) WM_DBG(SAVE 1) FAIL\n", __FILE__, __LINE__);
				return 1;
			}
			setbuf(fp, 0);
			memset(tmp, 0, sizeof(tmp));
			sprintf(tmp, "[%s %s]", __DATE__, __TIME__);
			fwrite(tmp, sizeof(tmp), 1, fp);
			fwrite(saved_setup, sizeof(saved_setup), 1, fp);
			fclose(fp);
			sync();
printf("(%s %d) WM_DBG(SAVE 1, %d %d)\n", __FILE__, __LINE__, setup->Work_mode, saved_setup[1].Work_mode);
			return 1;
		}
		if (key == KS_RIGHT_TOP)
		{
			memcpy(&saved_setup[2], setup, sizeof(struct SetupData));
			saved_setup[2].Work_mode = target / 10;

			fp = fopen("/data/arex/setup.dat", "wb+");
			if (!fp)
			{
				printf("(%s %d) WM_DBG(SAVE 2) FAIL\n", __FILE__, __LINE__);
				return 1;
			}
			setbuf(fp, 0);
			memset(tmp, 0, sizeof(tmp));
			sprintf(tmp, "[%s %s]", __DATE__, __TIME__);
			fwrite(tmp, sizeof(tmp), 1, fp);
			fwrite(saved_setup, sizeof(saved_setup), 1, fp);
			fclose(fp);
			sync();
printf("(%s %d) WM_DBG(SAVE 2, %d %d)\n", __FILE__, __LINE__, setup->Work_mode, saved_setup[2].Work_mode);
			return 1;
		}
		if (key == KS_RIGHT_CENTER)
		{
			memcpy(&saved_setup[3], setup, sizeof(struct SetupData));
			saved_setup[3].Work_mode = target / 10;

			fp = fopen("/data/arex/setup.dat", "wb+");
			if (!fp)
			{
				printf("(%s %d) WM_DBG(SAVE 3) FAIL\n", __FILE__, __LINE__);
				return 1;
			}
			setbuf(fp, 0);
			memset(tmp, 0, sizeof(tmp));
			sprintf(tmp, "[%s %s]", __DATE__, __TIME__);
			fwrite(tmp, sizeof(tmp), 1, fp);
			fwrite(saved_setup, sizeof(saved_setup), 1, fp);
			fclose(fp);
			sync();
printf("(%s %d) WM_DBG(SAVE 3, %d %d)\n", __FILE__, __LINE__, setup->Work_mode, saved_setup[3].Work_mode);
			return 1;
		}
	}
/*
	if (
		(source == WP_MANUAL_B && target == WP_MANUAL_SAVE)
		|| (source == WP_C360_ARM_SCULPTOR && target == WP_C360_ARM_SCULPTOR_SAVE)
		|| (source == WP_C360_LEG_SHAPER && target == WP_C360_LEG_SHAPER_SAVE)
		|| (source == WP_C360_CUSTOM_B && target == WP_C360_CUSTOM_SAVE)
		|| (source == WP_CALORIE_GOAL && target == WP_CALORIE_GOAL_SAVE)
		|| (source == WP_GLUTE_BUSTER && target == WP_GLUTE_BUSTER_SAVE)
		|| (source == WP_LEG_SHAPER_B && target == WP_LEG_SHAPER_SAVE)
		|| (source == WP_WL_B && target == WP_WL_SAVE)
		|| (source == WP_TARGET_HRC_B && target == WP_TARGET_HRC_SAVE)
		|| (source == WP_WL_HRC_B && target == WP_WL_HRC_SAVE)
		|| (source == WP_AEROBIC_HRC_B && target == WP_AEROBIC_HRC_SAVE)
		|| (source == WP_INTERVAL_HRC_B && target == WP_INTERVAL_HRC_SAVE)
		|| (source == WP_CARDIO_CHALLENGE && target == WP_CARDIO_CHALLENGE_SAVE)
		|| (source == WP_WALK_INTERVALS_B && target == WP_WALK_INTERVALS_SAVE)
		|| (source == WP_PACE_INTERVALS_B && target == WP_PACE_INTERVALS_SAVE)
		|| (source == WP_PACE_RAMP_B && target == WP_PACE_RAMP_SAVE)
		|| (source == WP_ROLLING_HILLS && target == WP_ROLLING_HILLS_SAVE)
		|| (source == WP_HILL_INTVALS && target == WP_HILL_INTVALS_SAVE)
		|| (source == WP_SINGLE_HILL && target == WP_SINGLE_HILL_SAVE)
		|| (source == WP_RANDOM_HILLS && target == WP_RANDOM_HILLS_SAVE)
		|| (source == WP_DISTANCE_B && target == WP_DISTANCE_SAVE)
		|| (source == WP_FITNESS_B && target == WP_FITNESS_SAVE)
		|| (source == WP_CUSTOM_UTRA_C && target == WP_CUSTOM_UTRA_SAVE)
	)
	{
		// [Aug 28 2013 14:31:54]
		if (key == KS_LEFT_TOP)
		{
			memcpy(&saved_setup[0], setup, sizeof(struct SetupData));
			saved_setup[0].Work_mode = target / 10;
		}
		if (key == KS_LEFT_CENTER)
		{
			memcpy(&saved_setup[1], setup, sizeof(struct SetupData));
			saved_setup[1].Work_mode = target / 10;
		}
		if (key == KS_RIGHT_TOP)
		{
			memcpy(&saved_setup[2], setup, sizeof(struct SetupData));
			saved_setup[2].Work_mode = target / 10;
		}
		if (key == KS_RIGHT_CENTER)
		{
			memcpy(&saved_setup[3], setup, sizeof(struct SetupData));
			saved_setup[3].Work_mode = target / 10;
		}
		if (key == KS_LEFT_TOP || key == KS_LEFT_CENTER || key == KS_RIGHT_TOP || key == KS_RIGHT_CENTER)
		{
			FILE *fp = 0;
			char tmp[32];

			fp = fopen("/data/arex/setup.dat", "wb+");
			if (!fp)			return 1;
			memset(tmp, 0, sizeof(tmp));
			sprintf(tmp, "[%s %s]", __DATE__, __TIME__);
			fwrite(tmp, sizeof(tmp), 1, fp);
			fwrite(saved_setup, sizeof(saved_setup), 1, fp);
			fclose(fp);

printf("(%s %d) WM_DBG(SAVE)\n", __FILE__, __LINE__);
			return 1;
		}
	}
*/
///////////////////////////////////////////////////////////
// READ SAVE
	if (
		(source == WP_READ_SAVE_0 && target == WP_CUSTOM)
	)
	{

		if (
			setup->Work_mode == WP_CUSTOM_UTRA_A
			|| setup->Work_mode == WP_CUSTOM_UTRA_B
			|| setup->Work_mode == WP_CUSTOM_UTRA_C
			|| setup->Work_mode == WP_CUSTOM_UTRA_RUN
			|| setup->Work_mode == WP_CUSTOM_UTRA_SAVE
		)
		{

			printf("(%s %d) WM_DBG(Work_mode == WP_CUSTOM_UTRA %d)\n", __FILE__, __LINE__, setup->Work_mode);
			return 1;
		}

		setup->Target_heart_rate = update->Target_heart_rate = 0;
		setup->Workout_time_1000 = 1;
		setup->Workout_time = 200;
		setup->Age = 35;
		setup->Weight = (unsigned short)((double)150.00F / (double)2.2F);
		setup->weight = (double)150.00F / (double)2.2F;
		setup->Workout_distance = (unsigned int)((double)0.5F * (double)1.609344F * (double)1000.00F);
		setup->distance = (double)0.5F * (double)1.609344F * (double)1000.00F;
		setup->Workload_level = 1;

		setup->Segments = 10;
		memset(setup->Segments_time, 0, sizeof(setup->Segments_time));
		for (i = 0; i < setup->Segments; i++)
		{
			setup->Workload[i] = 1;
			setup->Segments_time[i] = 60;
			setup->Pace[i] = 1;
		}

		printf("(%s %d) WM_DBG(WP_READ_SAVE->WP_CUSTOM %d)\n", __FILE__, __LINE__, setup->Work_mode);
		return 1;
	}

	if (source == WP_CUSTOM && target == WP_CUSTOM)
	{
		if (key == KS_LEFT_BOTTOM)
		{
			setup->Target_heart_rate = update->Target_heart_rate = 0;
			setup->Workout_time_1000 = 1;
			setup->Workout_time = 200;
			setup->Age = 35;
			setup->Weight = (unsigned short)((double)150.00F / (double)2.2F);
			setup->weight = (double)150.00F / (double)2.2F;
			setup->Workout_distance = (unsigned int)((double)0.5F * (double)1.609344F * (double)1000.00F);
			setup->distance = (double)0.5F * (double)1.609344F * (double)1000.00F;
			setup->Workload_level = 1;

			setup->Segments = 10;
			memset(setup->Segments_time, 0, sizeof(setup->Segments_time));
			for (i = 0; i < setup->Segments; i++)
			{
				setup->Workload[i] = 1;
				setup->Segments_time[i] = 60;
				setup->Pace[i] = 1;
			}

			return 1;
		}

		if (key == KS_LEFT_TOP || key == KS_LEFT_CENTER || key == KS_RIGHT_TOP || key == KS_RIGHT_CENTER)
		{
			read_setup_ok = 0;
		}

		if (key == KS_LEFT_TOP)
		{
			if (saved_setup[0].Age > 0)
			{
				memcpy(setup, &saved_setup[0], sizeof(struct SetupData));
				read_setup_ok = 1;
			}
		}
		if (key == KS_LEFT_CENTER)
		{
			if (saved_setup[1].Age > 0)
			{
				memcpy(setup, &saved_setup[1], sizeof(struct SetupData));
				read_setup_ok = 1;
			}
		}
		if (key == KS_RIGHT_TOP)
		{
			if (saved_setup[2].Age > 0)
			{
				memcpy(setup, &saved_setup[2], sizeof(struct SetupData));
				read_setup_ok = 1;
			}
		}
		if (key == KS_RIGHT_CENTER)
		{
			if (saved_setup[3].Age > 0)
			{
				memcpy(setup, &saved_setup[3], sizeof(struct SetupData));
				read_setup_ok = 1;
			}
		}
		if (key == KS_LEFT_TOP || key == KS_LEFT_CENTER || key == KS_RIGHT_TOP || key == KS_RIGHT_CENTER)
		{
			if (read_setup_ok)
			{
				printf("(%s %d) WM_DBG(READ SAVED OK, %d)\n", __FILE__, __LINE__, setup->Work_mode);
				printf("(%s %d) A=%d)\n", __FILE__, __LINE__, setup->Age);
				printf("(%s %d) W=%d)\n", __FILE__, __LINE__, setup->Weight);
			}
			else
			{
				printf("(%s %d) WM_DBG(READ SAVED FAIL)\n", __FILE__, __LINE__);
			}
			return 1;
		}
	}


///////////////////////////////////////////////////////////
	if (source == WP_HOME && target == WP_CARDIO_360_QUICK_START_RUN && key == KS_LEFT_TOP)
	{
		setup->Work_mode = WP_CARDIO_360_QUICK_START_RUN;
		exception->cmd.start = 1;
		exception->cmd.stop = 0;
		exception->cmd.pause = 0;
		exception->cmd.resume = 0;
		TellFpcStart();
printf("(%s %d) WM_DBG(WP_CARDIO_360_QUICK_START_RUN)\n", __FILE__, __LINE__);
		return 1;
	}
	if (source == WP_HOME && target == WP_MANUAL_QUICK_START_RUN && (key == KS_RIGHT_BOTTOM || key == KS_START))
	{
printf("(%s %d) WP_MANUAL_QUICK_START_RUN\n", __FILE__, __LINE__);
		setup->Work_mode = WP_MANUAL_QUICK_START_RUN;
		exception->cmd.start = 1;
		exception->cmd.stop = 0;
		exception->cmd.pause = 0;
		exception->cmd.resume = 0;
		TellFpcStart();
printf("(%s %d) WM_DBG(WP_MANUAL_QUICK_START_RUN)\n", __FILE__, __LINE__);
		return 1;
	}
	if (source == WP_HOME && target == WP_WORKOUT_FINDER && key == KS_LEFT_BOTTOM)
	{
		setup->Work_mode = WP_WORKOUT_FINDER;
printf("(%s %d) WM_DBG(WP_WORKOUT_FINDER)\n", __FILE__, __LINE__);
		return 1;
	}

///////////////////////////////////////////////////////////
	if (source == WP_C360 && target == WP_C360_QUICK_START_RUN && key == KS_RIGHT_BOTTOM)
	{
		setup->Work_mode = WP_C360_QUICK_START_RUN;
		exception->cmd.start = 1;
		exception->cmd.stop = 0;
		exception->cmd.pause = 0;
		exception->cmd.resume = 0;
		TellFpcStart();
printf("(%s %d) WM_DBG(WP_C360_QUICK_START_RUN)\n", __FILE__, __LINE__);
		return 1;
	}

#if 0
///////////////////////////////////////////////////////////
// NEXT
	if (source == WP_C360 && key == KS_RIGHT_BOTTOM)
	{
		setup->Work_mode = target;
		return 1;
	}
	if (source == WP_WL
		&& (key == KS_LEFT_TOP || key == KS_LEFT_CENTER || key == KS_LEFT_BOTTOM || key == KS_RIGHT_TOP || key == KS_RIGHT_BOTTOM)
	)
	{
		setup->Work_mode = target;
		return 1;
	}
	if (source == WP_HRC
		&& (key == KS_LEFT_TOP || key == KS_LEFT_CENTER || key == KS_LEFT_BOTTOM || key == KS_RIGHT_TOP || key == KS_RIGHT_BOTTOM)
	)
	{
		setup->Work_mode = target;
		return 1;
	}
	if (source == WP_PERFORMANCE
		&& (key == KS_LEFT_TOP || key == KS_LEFT_CENTER || key == KS_LEFT_BOTTOM || key == KS_RIGHT_TOP || key == KS_RIGHT_BOTTOM || key == KS_RIGHT_CENTER)
	)
	{
		setup->Work_mode = target;
		return 1;
	}
#endif // 0


///////////////////////////////////////////////////////////
// INIT WORKOUT
	if (source == WP_WORKOUT_FINDER && target == WP_MANUAL_A)// && source != WP_CUSTOM  && source != WP_READ_SAVE_0 && source != WP_READ_SAVE_1 && source != WP_READ_SAVE_2 && source != WP_READ_SAVE_3)
	{
		setup->Target_heart_rate = update->Target_heart_rate = 0;
		setup->Workout_time_1000 = 1;
		setup->Workout_time = 200;
		setup->Age = 35;
		setup->Weight = (unsigned short)((double)150.00F / (double)2.2F);
		setup->weight = (double)150.00F / (double)2.2F;

		// (於Manual, distance和time 是二擇一,互斥)
		setup->Workout_distance = (unsigned int)((double)0.5F * (double)1.609344F * (double)1000.00F);
		setup->distance = (double)0.5F * (double)1.609344F * (double)1000.00F;
		//setup->Workout_distance = 159;
		//setup->distance = (double)setup->Workout_distance;
		setup->Workload_level = 1;

		setup->Segments = 20;
		//memset(setup->Segments_time, 0, sizeof(setup->Segments_time));
		for (i = 0; i < setup->Segments; i++)			setup->Segments_time[i] = 60;

		InitWorkoutPhase01_mw();
		InitWorkoutPhase02_mw(1);
printf("(%s %d) WM_DBG(InitWorkoutPhase)\n", __FILE__, __LINE__);
		return 1;
	}
	if (source == WP_C360 && target == WP_C360_ARM_SCULPTOR)// && source != WP_CUSTOM && source != WP_READ_SAVE_0 && source != WP_READ_SAVE_1 && source != WP_READ_SAVE_2 && source != WP_READ_SAVE_3)
	{
		setup->Target_heart_rate = update->Target_heart_rate = 0;
		// default 10 分鐘
		setup->Workout_time_1000 = 0;
		setup->Workout_time = 600;

		setup->Age = 35;
		setup->Weight = (unsigned short)((double)150.00F / (double)2.2F);
		setup->weight = (double)150.00F / (double)2.2F;

		setup->Workout_distance = (unsigned int)((double)0.5F * (double)1.609344F * (double)1000.00F);
		setup->distance = (double)0.5F * (double)1.609344F * (double)1000.00F;
		setup->Workload_level = 1;

		setup->Segments = C3AS_SEGMENT_COUNT;
		//memset(setup->Segments_time, 0, sizeof(setup->Segments_time));
		for (i = 0; i < setup->Segments; i++)			setup->Segments_time[i] = 60;
		setup->Segments_time[1] = 30; setup->Segments_time[3] = 30;

		// default 10 分鐘
		update->Sports_mode = 8;

		InitWorkoutPhase01(10);
		InitWorkoutPhase02_C360_V1(C3AS_SEGMENT_COUNT, CARDIO360_ARM_SCULPTOR, c3as_Table);
printf("(%s %d) WM_DBG(InitWorkoutPhase=10)\n", __FILE__, __LINE__);
		return 1;
	}
	if (source == WP_C360 && target == WP_C360_LEG_SHAPER)// && source != WP_CUSTOM  && source != WP_READ_SAVE_0 && source != WP_READ_SAVE_1 && source != WP_READ_SAVE_2 && source != WP_READ_SAVE_3)
	{
		setup->Target_heart_rate = update->Target_heart_rate = 0;
		setup->Workout_time_1000 = 1;
		setup->Workout_time = 200;
		setup->Age = 35;
		setup->Weight = (unsigned short)((double)150.00F / (double)2.2F);
		setup->weight = (double)150.00F / (double)2.2F;
		setup->Workout_distance = (unsigned int)((double)0.5F * (double)1.609344F * (double)1000.00F);
		setup->distance = (double)0.5F * (double)1.609344F * (double)1000.00F;
		setup->Workload_level = 1;
		setup->Pace_level = 1;

		setup->Segments = C3LS_SEGMENT_COUNT;
		memset(setup->Segments_time, 0, sizeof(setup->Segments_time));
		for (i = 0; i < setup->Segments; i++)			setup->Segments_time[i] = 60;
		//setup->Segments_time[2] = 15; setup->Segments_time[3] = 15; setup->Segments_time[4] = 45; setup->Segments_time[5] = 30; 
		//setup->Segments_time[7] = 30; setup->Segments_time[8] = 45;

		InitWorkoutPhase01(20);
		InitWorkoutPhase02_C360_V1(C3LS_SEGMENT_COUNT, CARDIO360_LEG_SHAPER, c3ls_Table);
printf("(%s %d) WM_DBG(InitWorkoutPhase)\n", __FILE__, __LINE__);
		return 1;
	}
	if (source == WP_C360 && target == WP_C360_CUSTOM_A)// && source != WP_CUSTOM  && source != WP_READ_SAVE_0 && source != WP_READ_SAVE_1 && source != WP_READ_SAVE_2 && source != WP_READ_SAVE_3)
	{
		setup->Target_heart_rate = update->Target_heart_rate = 0;
		setup->Workout_time_1000 = 1;
		setup->Workout_time = 200;
		setup->Age = 35;
		setup->Weight = (unsigned short)((double)150.00F / (double)2.2F);
		setup->weight = (double)150.00F / (double)2.2F;
		setup->Workout_distance = (unsigned int)((double)0.5F * (double)1.609344F * (double)1000.00F);
		setup->distance = (double)0.5F * (double)1.609344F * (double)1000.00F;
		setup->Workload_level = 1;
/*
"	START UP, STRIDE FORWARD (1:00)
"	STRIDE FORWARD & PUSH ARMS (1:00)
"	STAND ON SIDES, LEFT ARM ONLY (0:15)
"	STAND ON SIDES, RIGHT ARM ONLY (0:15)
"	REVERSE STRIDE & BEND KNEES (0:45)
"	STRIDE FORWARD & LIFT HEELS (0:30)
"	STRIDE FORWARD & PULL ARMS(1:00)
"	HOLD SIDE RAILS, REVERSE STRIDE & LIFT TOES (0:30)
"	STAND ON SIDES, CHANGE GRIP, PUSH & PULL ARMS (0:45)
"	RECOVER, STRIDE FORWARD (1:00)

*/
		setup->Segments = C360QS_SEGMENT_COUNT;
		//memset(setup->Segments_time, 0, sizeof(setup->Segments_time));
		for (i = 0; i < setup->Segments; i++)			setup->Segments_time[i] = 60;
		setup->Segments_time[2] = 15; setup->Segments_time[3] = 15;  setup->Segments_time[4] = 45;  setup->Segments_time[5] = 30;
		setup->Segments_time[7] = 30; setup->Segments_time[8] = 45; //setup->Segments_time[9] = 60; setup->Segments_time[10] = 30;

		InitWorkoutPhase01(20);
		InitWorkoutPhase02_C360(C360QS_SEGMENT_COUNT, CARDIO360_CUSTOMIZED, c3qs_Table);
		printf("(%s %d) WM_DBG(InitWorkoutPhase)\n", __FILE__, __LINE__);
		return 1;
	}
	if (source == WP_WL && target == WP_CALORIE_GOAL)
	{
		//static unsigned short watt_table[] = { 39, 41,  44,  48,  51,  55,  59,  65,  71,  78,  88,  96,  106, 116,  133,  150,  170,  180, 204, 228, 252, 265, 273, 278, 280, 280, 280, 280, 280, 280, 280 };
		unsigned short Watt_workload = 0;
		unsigned short minutes = 0;

		setup->Target_heart_rate = update->Target_heart_rate = 0;
		setup->Workout_time_1000 = 1;
		setup->Workout_time = 200;
		setup->Age = 35;
		setup->Weight = (unsigned short)((double)150.00F / (double)2.2F);
		setup->weight = (double)150.00F / (double)2.2F;
		setup->Workout_distance = (unsigned int)((double)0.5F * (double)1.609344F * (double)1000.00F);
		setup->distance = (double)0.5F * (double)1.609344F * (double)1000.00F;
		setup->Calorie_goal = 90;

		minutes = (1000 * setup->Workout_time_1000 + setup->Workout_time) / 60;

		if (1 == product_type)
		{
			Watt_workload = setup->Calorie_goal / minutes * 60 / 3 * 10 * 7 / 100 - setup->Weight * 7 / 100 * 35 / 10;
			setup->Workload_level = tables->Get_70rpm_Level_ByWatt(Watt_workload);
		}
		else
		{
/*
			for (i = 0; i < (int)(sizeof(watt_table) / sizeof(unsigned short)) - 1; i++)
			{
				if (Watt_workload < watt_table[i + 1] && Watt_workload >= watt_table[i])
					break;
			}
			if((int)(sizeof(watt_table) / sizeof(unsigned short)) - 1 == i)
				setup->Workload_level = 1;
			else
			{
				setup->Workload_level = i + 1;
				if (setup->Workload_level > 30)
					setup->Workload_level = 30;
			}
*/
			Watt_workload = ((setup->Calorie_goal / minutes) * 60 - 120) / 9;
			setup->Workload_level = tables->Get_60rpm_Level_ByWatt(Watt_workload);
		}

		//memset(setup->Segments_time, 0, sizeof(setup->Segments_time));
		setup->Segments = (setup->Workout_time_1000 * 1 + setup->Workout_time) / (minutes * 60);
		if (setup->Segments > 30)				setup->Segments = 30;
		if (setup->Segments == 0)				setup->Segments = 20;
		for (i = 0; i < 30; i++)
		{
			if (i < setup->Segments)			setup->Segments_time[i] = 60;
			else								setup->Segments_time[i] = 0;
		}

		InitWorkoutPhase01(20);
		InitWorkoutPhase02_wlcg(1);
printf("(%s %d) WM_DBG(InitWorkoutPhase)\n", __FILE__, __LINE__);
		return 1;
	}
	if (source ==WP_WL && target == WP_GLUTE_BUSTER)// && source != WP_CUSTOM  && source != WP_READ_SAVE_0 && source != WP_READ_SAVE_1 && source != WP_READ_SAVE_2 && source != WP_READ_SAVE_3)
	{
		setup->Target_heart_rate = update->Target_heart_rate = 0;
		setup->Workout_time_1000 = 1;
		setup->Workout_time = 200;
		setup->Age = 35;
		setup->Weight = (unsigned short)((double)150.00F / (double)2.2F);
		setup->weight = (double)150.00F / (double)2.2F;
		setup->Workout_distance = (unsigned int)((double)0.5F * (double)1.609344F * (double)1000.00F);
		setup->distance = (double)0.5F * (double)1.609344F * (double)1000.00F;
		setup->Workload_level = 1;

		setup->Segments = 20;
		memset(setup->Segments_time, 0, sizeof(setup->Segments_time));
		for (i = 0; i < setup->Segments; i++)			setup->Segments_time[i] = 60;

		InitWorkoutPhase01(20);
		InitWorkoutPhase02_pobh(setup->Segments, gluteWattTable);//(20 segment, no timing table, indexed workload table)
printf("(%s %d) WM_DBG(InitWorkoutPhase)\n", __FILE__, __LINE__);
		return 1;
	}
	if (source ==WP_WL && target == WP_LEG_SHAPER_A)// && source != WP_CUSTOM  && source != WP_READ_SAVE_0 && source != WP_READ_SAVE_1 && source != WP_READ_SAVE_2 && source != WP_READ_SAVE_3)
	{
		setup->Target_heart_rate = update->Target_heart_rate = 0;
		setup->Workout_time_1000 = 1;
		setup->Workout_time = 200;
		setup->Age = 35;
		setup->Weight = (unsigned short)((double)150.00F / (double)2.2F);
		setup->weight = (double)150.00F / (double)2.2F;
		setup->Workout_distance = (unsigned int)((double)0.5F * (double)1.609344F * (double)1000.00F);
		setup->distance = (double)0.5F * (double)1.609344F * (double)1000.00F;
		setup->Workload_level = 1;
		setup->Pace_level = 1;

		setup->Segments = 20;
		//memset(setup->Segments_time, 0, sizeof(setup->Segments_time));
		for (i = 0; i < setup->Segments; i++)			setup->Segments_time[i] = 60;

		InitWorkoutPhase01(20);
		InitWorkoutPhase02_Leg(20);
printf("(%s %d) WM_DBG(InitWorkoutPhase)\n", __FILE__, __LINE__);
		return 1;
	}
	if (source ==WP_WL  && target == WP_WL_A)// && source != WP_CUSTOM  && source != WP_READ_SAVE_0 && source != WP_READ_SAVE_1 && source != WP_READ_SAVE_2 && source != WP_READ_SAVE_3)
	{
		setup->Target_heart_rate = update->Target_heart_rate = 0;
		setup->Workout_time_1000 = 1;
		setup->Workout_time = 200;
		setup->Age = 35;
		setup->Weight = (unsigned short)((double)150.00F / (double)2.2F);
		setup->weight = (double)150.00F / (double)2.2F;
		setup->Workout_distance = (unsigned int)((double)0.5F * (double)1.609344F * (double)1000.00F);
		setup->distance = (double)0.5F * (double)1.609344F * (double)1000.00F;
		setup->Workload_level = 1;

		setup->Work_heart_rate = 65;
		update->Target_heart_rate = setup->Target_heart_rate = setup->Work_heart_rate * (220 - setup->Age) / 100;

		setup->Segments = 20;
		//memset(setup->Segments_time, 0, sizeof(setup->Segments_time));
		for (i = 0; i < setup->Segments; i++)			setup->Segments_time[i] = 60;

		InitWorkoutPhase01(20);
		InitWorkoutPhase02_1pre_segTime(1);
printf("(%s %d) WM_DBG(InitWorkoutPhase)\n", __FILE__, __LINE__);
		return 1;
	}
	if (source == WP_HRC && target == WP_TARGET_HRC_A)// && source != WP_CUSTOM  && source != WP_READ_SAVE_0 && source != WP_READ_SAVE_1 && source != WP_READ_SAVE_2 && source != WP_READ_SAVE_3)
	{
		setup->Target_heart_rate = update->Target_heart_rate = 0;
		setup->Workout_time_1000 = 1;
		setup->Workout_time = 200;
		setup->Age = 35;
		setup->Weight = (unsigned short)((double)150.00F / (double)2.2F);
		setup->weight = (double)150.00F / (double)2.2F;
		setup->Workout_distance = (unsigned int)((double)0.5F * (double)1.609344F * (double)1000.00F);
		setup->distance = (double)0.5F * (double)1.609344F * (double)1000.00F;
		setup->Workload_level = 1;
		update->Target_heart_rate = setup->Target_heart_rate = 80;
		setup->Work_heart_rate = 100 * setup->Target_heart_rate / (220 - setup->Age);

		setup->Segments = 20;
		//memset(setup->Segments_time, 0, sizeof(setup->Segments_time));
		for (i = 0; i < setup->Segments; i++)			setup->Segments_time[i] = 60;

		InitWorkoutPhase01(20);
		InitWorkoutPhase02_1pre_segTime(1);
printf("(%s %d) WM_DBG(InitWorkoutPhase)\n", __FILE__, __LINE__);
		return 1;
	}
	if (source == WP_HRC && target == WP_WL_HRC_A)// && source != WP_CUSTOM  && source != WP_READ_SAVE_0 && source != WP_READ_SAVE_1 && source != WP_READ_SAVE_2 && source != WP_READ_SAVE_3)
	{
		setup->Target_heart_rate = update->Target_heart_rate = 0;
		setup->Workout_time_1000 = 1;
		setup->Workout_time = 200;
		setup->Age = 35;
		setup->Weight = (unsigned short)((double)150.00F / (double)2.2F);
		setup->weight = (double)150.00F / (double)2.2F;
		setup->Workout_distance = (unsigned int)((double)0.5F * (double)1.609344F * (double)1000.00F);
		setup->distance = (double)0.5F * (double)1.609344F * (double)1000.00F;
		setup->Workload_level = 1;

		//setup->Target_heart_rate = 120;
		//setup->Work_heart_rate = 100 * setup->Target_heart_rate / (220 - setup->Age);

		setup->Work_heart_rate = 65;
		update->Target_heart_rate = setup->Target_heart_rate = setup->Work_heart_rate * (220 - setup->Age) / 100;

		setup->Segments = 20;
		//memset(setup->Segments_time, 0, sizeof(setup->Segments_time));
		for (i = 0; i < setup->Segments; i++)			setup->Segments_time[i] = 60;

		InitWorkoutPhase01(20);
		InitWorkoutPhase02_1pre_segTime(1);
printf("(%s %d) WM_DBG(InitWorkoutPhase)\n", __FILE__, __LINE__);
		return 1;
	}
	if (source == WP_HRC && target == WP_AEROBIC_HRC_A)// && source != WP_CUSTOM  && source != WP_READ_SAVE_0 && source != WP_READ_SAVE_1 && source != WP_READ_SAVE_2 && source != WP_READ_SAVE_3)
	{
		setup->Target_heart_rate = update->Target_heart_rate = 0;
		setup->Workout_time_1000 = 1;
		setup->Workout_time = 200;
		setup->Age = 35;
		setup->Weight = (unsigned short)((double)150.00F / (double)2.2F);
		setup->weight = (double)150.00F / (double)2.2F;
		setup->Workout_distance = (unsigned int)((double)0.5F * (double)1.609344F * (double)1000.00F);
		setup->distance = (double)0.5F * (double)1.609344F * (double)1000.00F;
		setup->Workload_level = 1;

		//setup->Target_heart_rate = 148;
		//setup->Work_heart_rate = 100 * setup->Target_heart_rate / (220 - setup->Age);

		setup->Work_heart_rate = 80;
		update->Target_heart_rate = setup->Target_heart_rate = setup->Work_heart_rate * (220 - setup->Age) / 100;

		setup->Segments = 20;
		//memset(setup->Segments_time, 0, sizeof(setup->Segments_time));
		for (i = 0; i < setup->Segments; i++)			setup->Segments_time[i] = 60;

		InitWorkoutPhase01(20);
		InitWorkoutPhase02_1pre_segTime(1);
printf("(%s %d) WM_DBG(InitWorkoutPhase)\n", __FILE__, __LINE__);
		return 1;
	}
	if (source == WP_HRC && target == WP_INTERVAL_HRC_A)// && source != WP_CUSTOM  && source != WP_READ_SAVE_0 && source != WP_READ_SAVE_1 && source != WP_READ_SAVE_2 && source != WP_READ_SAVE_3)
	{
		setup->Target_heart_rate = update->Target_heart_rate = 0;
		setup->Workout_time_1000 = 1;
		setup->Workout_time = 200;
		setup->Age = 35;
		setup->Weight = (unsigned short)((double)150.00F / (double)2.2F);
		setup->weight = (double)150.00F / (double)2.2F;
		setup->Workout_distance = (unsigned int)((double)0.5F * (double)1.609344F * (double)1000.00F);
		setup->distance = (double)0.5F * (double)1.609344F * (double)1000.00F;
		setup->Workload_level = 1;
		update->Target_heart_rate = setup->Target_heart_rate = 80;
		setup->Work_heart_rate = 100 * setup->Target_heart_rate / (220 - setup->Age);

		setup->Segments = 20;
		//memset(setup->Segments_time, 0, sizeof(setup->Segments_time));
		for (i = 0; i < setup->Segments; i++)			setup->Segments_time[i] = 60;

		InitWorkoutPhase01(20);
		InitWorkoutPhase02_harci(8);
printf("(%s %d) WM_DBG(InitWorkoutPhase)\n", __FILE__, __LINE__);
		return 1;
	}
	if (source == WP_PERFORMANCE && target == WP_CARDIO_CHALLENGE)// && source != WP_CUSTOM  && source != WP_READ_SAVE_0 && source != WP_READ_SAVE_1 && source != WP_READ_SAVE_2 && source != WP_READ_SAVE_3)
	{
		setup->Target_heart_rate = update->Target_heart_rate = 0;
		setup->Workout_time_1000 = 1;
		setup->Workout_time = 200;
		setup->Age = 35;
		setup->Weight = (unsigned short)((double)150.00F / (double)2.2F);
		setup->weight = (double)150.00F / (double)2.2F;
		setup->Workout_distance = (unsigned int)((double)0.5F * (double)1.609344F * (double)1000.00F);
		setup->distance = (double)0.5F * (double)1.609344F * (double)1000.00F;
		setup->Workload_level = 1;

		setup->Segments = 20;
		//memset(setup->Segments_time, 0, sizeof(setup->Segments_time));
		for (i = 0; i < setup->Segments + 1; i++)			setup->Segments_time[i] = 60;

		InitWorkoutPhase01(20);
		InitWorkoutPhase02_pcc(20, performance_cardio_challenge_LeveTable, performance_cardio_challenge_PaceTable);
printf("(%s %d) WM_DBG(InitWorkoutPhase)\n", __FILE__, __LINE__);
		return 1;
	}
	if (source == WP_PACE && target == WP_WALK_INTERVALS_A)// && source != WP_CUSTOM  && source != WP_READ_SAVE_0 && source != WP_READ_SAVE_1 && source != WP_READ_SAVE_2 && source != WP_READ_SAVE_3)
	{
		setup->Target_heart_rate = update->Target_heart_rate = 0;
		setup->Workout_time_1000 = 1;
		setup->Workout_time = 200;
		setup->Age = 35;
		setup->Weight = (unsigned short)((double)150.00F / (double)2.2F);
		setup->weight = (double)150.00F / (double)2.2F;
		setup->Workout_distance = (unsigned int)((double)0.5F * (double)1.609344F * (double)1000.00F);
		setup->distance = (double)0.5F * (double)1.609344F * (double)1000.00F;
		setup->Workload_level = 1;
		setup->Pace_level = 1;


// JC NOTE
// JC NOTE
// JC NOTE
// JC NOTE
		setup->Segments = 20;
		//memset(setup->Segments_time, 0, sizeof(setup->Segments_time));
		//for (i = 0; i < setup->Segments; i++)			setup->Segments_time[i] = 60;
		for (i = 0; i < 64; i++)			setup->Segments_time[i] = 60;

		InitWorkoutPhase01(20);
		InitWorkoutPhase02_segTime_pace(1, weight_loss_walk_and_run_PaceTable);
printf("(%s %d) WM_DBG(InitWorkoutPhase)\n", __FILE__, __LINE__);
		return 1;
	}
	if (source == WP_PACE && target == WP_PACE_INTERVALS_A)// && source != WP_CUSTOM && source != WP_READ_SAVE_0 && source != WP_READ_SAVE_1 && source != WP_READ_SAVE_2 && source != WP_READ_SAVE_3)
	{
		setup->Target_heart_rate = update->Target_heart_rate = 0;
		setup->Workout_time_1000 = 1;
		setup->Workout_time = 200;
		setup->Age = 35;
		setup->Weight = (unsigned short)((double)150.00F / (double)2.2F);
		setup->weight = (double)150.00F / (double)2.2F;
		setup->Workout_distance = (unsigned int)((double)0.5F * (double)1.609344F * (double)1000.00F);
		setup->distance = (double)0.5F * (double)1.609344F * (double)1000.00F;
		setup->Workload_level = 1;
		setup->Pace_level = 1;

// JC NOTE
// JC NOTE
// JC NOTE
// JC NOTE
		setup->Segments = 20;
		memset(setup->Segments_time, 0, sizeof(setup->Segments_time));
		//for (i = 0; i < setup->Segments; i++)			setup->Segments_time[i] = 60;
		for (i = 0; i < 64; i++)			setup->Segments_time[i] = 60;

		InitWorkoutPhase01(20);
		InitWorkoutPhase02_segTime_pace(1, ppiPaceTable);
printf("(%s %d) WM_DBG(InitWorkoutPhase)\n", __FILE__, __LINE__);
		return 1;
	}
	if (source == WP_PACE && target == WP_PACE_RAMP_A)// && source != WP_CUSTOM && source != WP_READ_SAVE_0 && source != WP_READ_SAVE_1 && source != WP_READ_SAVE_2 && source != WP_READ_SAVE_3)
	{
		setup->Target_heart_rate = update->Target_heart_rate = 0;
		setup->Workout_time_1000 = 1;
		setup->Workout_time = 200;
		setup->Age = 35;
		setup->Weight = (unsigned short)((double)150.00F / (double)2.2F);
		setup->weight = (double)150.00F / (double)2.2F;
		setup->Workout_distance = (unsigned int)((double)0.5F * (double)1.609344F * (double)1000.00F);
		setup->distance = (double)0.5F * (double)1.609344F * (double)1000.00F;
		setup->Workload_level = 1;
		setup->Pace_level = 1;

		setup->Segments = 20;
		memset(setup->Segments_time, 0, sizeof(setup->Segments_time));
		for (i = 0; i < setup->Segments; i++)			setup->Segments_time[i] = 60;


		/*for(i = 0; i < 20; i++)
		{
			for(j = 0; j < 20; j++)
			{
				wpr_Table[i].work_load[j] = tables->Get_Level_ByRpmWatt(60, wpr_Table[i].work_load[j]);
			}
		}*/

		InitWorkoutPhase01(20);
		InitWorkoutPhase02_ppr(20);
		//InitWorkoutPhase02_pcc(20, wpr_Table, ppr_Table);
printf("(%s %d) WM_DBG(InitWorkoutPhase)\n", __FILE__, __LINE__);
		return 1;
	}
	if (source == WP_HILLS && target == WP_ROLLING_HILLS)// && source != WP_CUSTOM && source != WP_READ_SAVE_0 && source != WP_READ_SAVE_1 && source != WP_READ_SAVE_2 && source != WP_READ_SAVE_3)
	{
		setup->Target_heart_rate = update->Target_heart_rate = 0;
		setup->Workout_time_1000 = 1;
		setup->Workout_time = 200;
		setup->Age = 35;
		setup->Weight = (unsigned short)((double)150.00F / (double)2.2F);
		setup->weight = (double)150.00F / (double)2.2F;
		setup->Workout_distance = (unsigned int)((double)0.5F * (double)1.609344F * (double)1000.00F);
		setup->distance = (double)0.5F * (double)1.609344F * (double)1000.00F;
		setup->Workload_level = 1;

		setup->Segments = 20;
		//memset(setup->Segments_time, 0, sizeof(setup->Segments_time));
		for (i = 0; i < setup->Segments; i++)			setup->Segments_time[i] = 60;

		InitWorkoutPhase01(20);
		InitWorkoutPhase02_wlrh(20, weight_loss_rolling_hill_LevelTable);//(20 segment, no timing table, indexed workload table)
printf("(%s %d) WM_DBG(InitWorkoutPhase)\n", __FILE__, __LINE__);
		return 1;
	}
	if (source == WP_HILLS && target == WP_HILL_INTVALS)// && source != WP_CUSTOM && source != WP_READ_SAVE_0 && source != WP_READ_SAVE_1 && source != WP_READ_SAVE_2 && source != WP_READ_SAVE_3)
	{
		setup->Target_heart_rate = update->Target_heart_rate = 0;
		setup->Workout_time_1000 = 1;
		setup->Workout_time = 200;
		setup->Age = 35;
		setup->Weight = (unsigned short)((double)150.00F / (double)2.2F);
		setup->weight = (double)150.00F / (double)2.2F;
		setup->Workout_distance = (unsigned int)((double)0.5F * (double)1.609344F * (double)1000.00F);
		setup->distance = (double)0.5F * (double)1.609344F * (double)1000.00F;
		setup->Workload_level = 1;

		setup->Segments = 20;
		memset(setup->Segments_time, 0, sizeof(setup->Segments_time));
		for (i = 0; i < setup->Segments; i++)			setup->Segments_time[i] = 60;

		InitWorkoutPhase01(20);
		InitWorkoutPhase02_hi(2, wr_Table);
printf("(%s %d) WM_DBG(InitWorkoutPhase)\n", __FILE__, __LINE__);
		return 1;
	}
	if (source == WP_HILLS && target == WP_SINGLE_HILL)// && source != WP_CUSTOM && source != WP_READ_SAVE_0 && source != WP_READ_SAVE_1 && source != WP_READ_SAVE_2 && source != WP_READ_SAVE_3)
	{
		setup->Target_heart_rate = update->Target_heart_rate = 0;
		setup->Workout_time_1000 = 1;
		setup->Workout_time = 200;
		setup->Age = 35;
		setup->Weight = (unsigned short)((double)150.00F / (double)2.2F);
		setup->weight = (double)150.00F / (double)2.2F;
		setup->Workout_distance = (unsigned int)((double)0.5F * (double)1.609344F * (double)1000.00F);
		setup->distance = (double)0.5F * (double)1.609344F * (double)1000.00F;
		setup->Workload_level = 1;

		setup->Segments = 20;
		memset(setup->Segments_time, 0, sizeof(setup->Segments_time));
		for (i = 0; i < setup->Segments; i++)			setup->Segments_time[i] = 60;

		InitWorkoutPhase01(20);
		InitWorkoutPhase02_pobh(setup->Segments, obhWattTable);//(20 segment, no timing table, indexed workload table)
printf("(%s %d) WM_DBG(InitWorkoutPhase)\n", __FILE__, __LINE__);
		return 1;
	}
	if (source == WP_HILLS && target == WP_RANDOM_HILLS)// && source != WP_CUSTOM && source != WP_READ_SAVE_0 && source != WP_READ_SAVE_1 && source != WP_READ_SAVE_2 && source != WP_READ_SAVE_3)
	{
		setup->Target_heart_rate = update->Target_heart_rate = 0;
		setup->Workout_time_1000 = 1;
		setup->Workout_time = 200;
		setup->Age = 35;
		setup->Weight = (unsigned short)((double)150.00F / (double)2.2F);
		setup->weight = (double)150.00F / (double)2.2F;
		setup->Workout_distance = (unsigned int)((double)0.5F * (double)1.609344F * (double)1000.00F);
		setup->distance = (double)0.5F * (double)1.609344F * (double)1000.00F;
		setup->Workload_level = 1;

		setup->Segments = 20;
		//memset(setup->Segments_time, 0, sizeof(setup->Segments_time));
		for (i = 0; i < setup->Segments; i++)			setup->Segments_time[i] = 60;

		InitWorkoutPhase01(20);
		srand((unsigned)time(NULL));
		for (i = 0; i < 20; i++)		randomWattTable[0].work_load[i] = 1 + (rand() % 8);
		for (i = 0; i < 20; i++)		randomWattTable[1].work_load[i] = randomWattTable[0].work_load[i] + 1;
		for (i = 0; i < 20; i++)		randomWattTable[2].work_load[i] = randomWattTable[0].work_load[i] + 2;
		for (i = 0; i < 20; i++)		randomWattTable[3].work_load[i] = randomWattTable[0].work_load[i] + 3;
		for (i = 0; i < 20; i++)		randomWattTable[4].work_load[i] = randomWattTable[0].work_load[i] + 4;
		for (i = 0; i < 20; i++)		randomWattTable[5].work_load[i] = randomWattTable[0].work_load[i] + 5;
		for (i = 0; i < 20; i++)		randomWattTable[6].work_load[i] = randomWattTable[0].work_load[i] + 6;
		for (i = 0; i < 20; i++)		randomWattTable[7].work_load[i] = randomWattTable[0].work_load[i] + 7;
		for (i = 0; i < 20; i++)		randomWattTable[8].work_load[i] = randomWattTable[0].work_load[i] + 8;
		for (i = 0; i < 20; i++)		randomWattTable[9].work_load[i] = randomWattTable[0].work_load[i] + 9;
		for (i = 0; i < 20; i++)		randomWattTable[10].work_load[i] = randomWattTable[0].work_load[i] + 10;
		for (i = 0; i < 20; i++)		randomWattTable[11].work_load[i] = randomWattTable[0].work_load[i] + 11;
		for (i = 0; i < 20; i++)		randomWattTable[12].work_load[i] = randomWattTable[0].work_load[i] + 12;
		for (i = 0; i < 20; i++)		randomWattTable[13].work_load[i] = randomWattTable[0].work_load[i] + 13;
		for (i = 0; i < 20; i++)		randomWattTable[14].work_load[i] = randomWattTable[0].work_load[i] + 14;
		for (i = 0; i < 20; i++)		randomWattTable[15].work_load[i] = randomWattTable[0].work_load[i] + 15;
		for (i = 0; i < 20; i++)		randomWattTable[16].work_load[i] = randomWattTable[0].work_load[i] + 16;
		for (i = 0; i < 20; i++)		randomWattTable[17].work_load[i] = randomWattTable[0].work_load[i] + 17;
		for (i = 0; i < 20; i++)		randomWattTable[18].work_load[i] = randomWattTable[0].work_load[i] + 18;
		for (i = 0; i < 20; i++)		randomWattTable[19].work_load[i] = randomWattTable[0].work_load[i] + 19;

		InitWorkoutPhase02_pobh(setup->Segments, randomWattTable);//(20 segment, no timing table, indexed workload table)
printf("(%s %d) WM_DBG(InitWorkoutPhase)\n", __FILE__, __LINE__);
		return 1;
	}
	if (source ==WP_PERFORMANCE && target == WP_DISTANCE_A)// && source != WP_CUSTOM && source != WP_READ_SAVE_0 && source != WP_READ_SAVE_1 && source != WP_READ_SAVE_2 && source != WP_READ_SAVE_3)
	{
		setup->Target_heart_rate = update->Target_heart_rate = 0;
		setup->Workout_time_1000 = 1;
		setup->Workout_time = 200;
		setup->Age = 35;
		setup->Weight = (unsigned short)((double)150.00F / (double)2.2F);
		setup->weight = (double)150.00F / (double)2.2F;
		setup->Workout_distance = 5000;
		setup->distance = 5000.00F;
		setup->Workload_level = 1;

		setup->Segments = 20;
		memset(setup->Segments_time, 0, sizeof(setup->Segments_time));
		for (i = 0; i < setup->Segments; i++)			setup->Segments_time[i] = 60;

		InitWorkoutPhase01_distance(90); //in HRC Distance.c
		InitWorkoutPhase02_apd(1);

printf("(%s %d) WM_DBG(InitWorkoutPhase, %f %d %d)\n", __FILE__, __LINE__, rt->target_workout_distance, setup->Workout_distance, rt->workout_state);
		return 1;
	}
	if (source == WP_PERFORMANCE && target == WP_FITNESS_A)// && source != WP_CUSTOM && source != WP_READ_SAVE_0 && source != WP_READ_SAVE_1 && source != WP_READ_SAVE_2 && source != WP_READ_SAVE_3)
	{
		setup->Target_heart_rate = update->Target_heart_rate = 0;
		setup->Workout_time_1000 = 0;
		setup->Workout_time = 720;
		setup->Age = 35;
		setup->Weight = (unsigned short)((double)150.00F / (double)2.2F);
		setup->weight = (double)150.00F / (double)2.2F;
		setup->Workout_distance = (unsigned int)((double)0.5F * (double)1.609344F * (double)1000.00F);
		setup->distance = (double)0.5F * (double)1.609344F * (double)1000.00F;
		setup->Workload_level = 1;
		setup->Target_heart_rate = 157;//185;//freeman, default 85%
		setup->Gender = 0;

		setup->Work_heart_rate = 100 * setup->Target_heart_rate / (220 - setup->Age);
		setup->Segments = 12;
		memset(setup->Segments_time, 0, sizeof(setup->Segments_time));
		for (i = 0; i < setup->Segments; i++)			setup->Segments_time[i] = 60;

		InitWorkoutPhase01(12);
		InitWorkoutPhase02_pft(1);
printf("(%s %d) WM_DBG(InitWorkoutPhase)\n", __FILE__, __LINE__);
		return 1;
	}
	if (source == WP_CUSTOM && target == WP_CUSTOM_UTRA_A)
	{
		if (0 == read_setup_ok)
		{
			setup->Target_heart_rate = update->Target_heart_rate = 0;
			setup->Workout_time_1000 = 1;
			setup->Workout_time = 200;
			setup->Age = 35;
			setup->Weight = (unsigned short)((double)150.00F / (double)2.2F);
			setup->weight = (double)150.00F / (double)2.2F;
			setup->Workout_distance = (unsigned int)((double)0.5F * (double)1.609344F * (double)1000.00F);
			setup->distance = (double)0.5F * (double)1.609344F * (double)1000.00F;
			setup->Workload_level = 1;

			setup->Segments = 10;
			//memset(setup->Segments_time, 0, sizeof(setup->Segments_time));
			for (i = 0; i < setup->Segments; i++)
			{
				setup->Workload[i] = 1;
				setup->Segments_time[i] = 60;
				setup->Pace[i] = 1;
			}
		}

		InitWorkoutPhase01(20);
		InitWorkoutPhase02_cu();	// custom ultra
printf("(%s %d) WM_DBG(WP_CUSTOM_UTRA INIT=%d Age=%d)\n", __FILE__, __LINE__, read_setup_ok, setup->Age);
		return 1;
	}
	//if (source == WP_CUSTOM_UTRA_A && target == WP_CUSTOM)
	if (source != target && target == WP_CUSTOM)
	{
		setup->Target_heart_rate = update->Target_heart_rate = 0;
		setup->Workout_time_1000 = 1;
		setup->Workout_time = 200;
		setup->Age = 35;
		setup->Weight = (unsigned short)((double)150.00F / (double)2.2F);
		setup->weight = (double)150.00F / (double)2.2F;
		setup->Workout_distance = (unsigned int)((double)0.5F * (double)1.609344F * (double)1000.00F);
		setup->distance = (double)0.5F * (double)1.609344F * (double)1000.00F;
		setup->Workload_level = 1;

		setup->Segments = 10;
		//memset(setup->Segments_time, 0, sizeof(setup->Segments_time));
		for (i = 0; i < setup->Segments; i++)
		{
			setup->Workload[i] = 1;
			setup->Segments_time[i] = 60;
			setup->Pace[i] = 1;
		}

		InitWorkoutPhase01(20);
		InitWorkoutPhase02_cu();	// custom ultra
printf("(%s %d) WM_DBG(WP_CUSTOM_UTRA INIT=%d Age=%d)\n", __FILE__, __LINE__, read_setup_ok, setup->Age);
		return 1;
	}



///////////////////////////////////////////////////////////
	if (
		(source != target && target == WP_PACE)
		|| (source != target && target == WP_PERFORMANCE)
		|| (source != target && target == WP_HOME)
	)
	{
		setup->Pace_level = 1;
		//printf("(%s %d) WM_DBG(RESET Pace_level=1)\n", __FILE__, __LINE__);
		return 1;
	}

	if (source == WP_C360_CUSTOM_B && target == WP_C360_CUSTOM_B)
	{
		if (key == KS_LEFT_TOP)
		{
			if (scroll_idx > 0)		scroll_idx--;
			printf("(%s %d) WM_DBG(%d)\n", __FILE__, __LINE__, scroll_idx);
			return 1;
		}
		if (key == KS_LEFT_CENTER)
		{
			if (scroll_idx < 9)		scroll_idx++;
			printf("(%s %d) WM_DBG(%d)\n", __FILE__, __LINE__, scroll_idx);
			return 1;
		}
		if (key == KS_RIGHT_TOP)
		{
			if (setup->Segments_time[scroll_idx] > 120)		// 120 sec
				setup->Segments_time[scroll_idx] = 120;
			else if (setup->Segments_time[scroll_idx] <= 120 && setup->Segments_time[scroll_idx] > 105)
				setup->Segments_time[scroll_idx] = 120;
			else //if (setup->Segments_time[scroll_idx] <= 105)
				setup->Segments_time[scroll_idx] += 15;
			printf("(%s %d) WM_DBG(%d %d)\n", __FILE__, __LINE__, scroll_idx, setup->Segments_time[scroll_idx]);
			return 1;
		}
		if (key == KS_RIGHT_CENTER)
		{
			int i;
			int zero = 0;		// 為 0 之 個數

			// 計算中間 0 之 個數, 1~8
			for (i = 0; i < 8; i++)
			{
				if (0 == setup->Segments_time[1 + i])
					zero++;
			}
			if (zero >= 7)
			{
				if (setup->Segments_time[scroll_idx] <= 15)
				{
					//setup->Segments_time[scroll_idx] = 15;
					printf("(%s %d) WM_DBG(%d %d), ALL ZERO NOT ALLOWED \n", __FILE__, __LINE__, scroll_idx, setup->Segments_time[scroll_idx]);
					return 1;
				}
			}
			if (0 == scroll_idx || 9 == scroll_idx)
			{
				if (setup->Segments_time[scroll_idx] <= 15)
				{
					setup->Segments_time[scroll_idx] = 15;
					printf("(%s %d) WM_DBG(%d %d), ZERO NOT ALLOWED \n", __FILE__, __LINE__, scroll_idx, setup->Segments_time[scroll_idx]);
					return 1;
				}
			}

			if (setup->Segments_time[scroll_idx] >= 0 && setup->Segments_time[scroll_idx] < 15)
			{
				setup->Segments_time[scroll_idx] = 0;
			}
			else
			{
				int to;

				for(i = 1; i < 9; i++)
					to += (int)setup->Segments_time[i];
				if (
					to <= 15
					&& (1 == scroll_idx || 2 == scroll_idx || 3 == scroll_idx || 4 == scroll_idx || 5 == scroll_idx || 6 == scroll_idx || 7 == scroll_idx || 8 == scroll_idx)
				)
				{
					printf("(%s %d) WM_DBG(%d %d)\n", __FILE__, __LINE__, scroll_idx, setup->Segments_time[scroll_idx]);
					return 1;
				}

				setup->Segments_time[scroll_idx] -= 15;
			}
			printf("(%s %d) WM_DBG(%d %d)\n", __FILE__, __LINE__, scroll_idx, setup->Segments_time[scroll_idx]);
			return 1;
		}
	}


///////////////////////////////////////////////////////////
	if (source != target && target == WP_CUSTOM_UTRA_B)
	{
		segments_adjust = 1;
	}

	if (source == WP_CUSTOM_UTRA_B && target == WP_CUSTOM_UTRA_B)
	{
		unsigned short w;
		unsigned short s = setup->Segments;
		if (0 == setup->Segments)
			setup->Segments = 20;

		if (key == KS_LEFT_BOTTOM)
		{
			//scroll_idx = 0;
			segments_adjust = !segments_adjust;
printf("(%s %d) WM_DBG(%d)\n", __FILE__, __LINE__, segments_adjust);
			return 1;
		}

		if (key == KS_LEFT_CENTER)
		{
			if (scroll_idx > 0)		scroll_idx--;
//printf("(%s %d) WM_DBG(%d %d)\n", __FILE__, __LINE__, scroll_idx, setup->Workload[scroll_idx]);
printf("(%s %d) WM_DBG(%d)\n", __FILE__, __LINE__, scroll_idx);
			return 1;
		}
		if (key == KS_RIGHT_CENTER)
		{
			//if (scroll_idx < 9)		scroll_idx++;
			if (scroll_idx < s -1)		scroll_idx++;
//printf("(%s %d) WM_DBG(%d %d)\n", __FILE__, __LINE__, scroll_idx, setup->Workload[scroll_idx]);
printf("(%s %d) WM_DBG(%d)\n", __FILE__, __LINE__, scroll_idx);
			return 1;
		}

	///////////////////
	///////////////////
		if (0 == segments_adjust)
		{
			///////////////
			if (key == KS_LEFT_TOP)
			{
				w = setup->Workload[scroll_idx];
				if (0 == w)			w = 2;
				else if (w < 30)		w++;
				setup->Workload[scroll_idx] = w;
	printf("(%s %d) WM_DBG(%d %d)\n", __FILE__, __LINE__, scroll_idx, setup->Workload[scroll_idx]);
				return 1;
			}
			if (key == KS_RIGHT_TOP)
			{
				w = setup->Workload[scroll_idx];
				if (0 == w)			w = 1;
				else if (w > 1)		w--;
				setup->Workload[scroll_idx] = w;

	printf("(%s %d) WM_DBG(%d %d)\n", __FILE__, __LINE__, scroll_idx, setup->Workload[scroll_idx]);
				return 1;
			}
		}
		else
		{
			///////////////
			if (key == KS_LEFT_TOP)
			{
				w = setup->Pace[scroll_idx];
				if (0 == w)			w = 2;
				else if (w < 20)		w++;
				setup->Pace[scroll_idx] = w;
	printf("(%s %d) WM_DBG(%d %d)\n", __FILE__, __LINE__, scroll_idx, setup->Pace[scroll_idx]);
				return 1;
			}
			if (key == KS_RIGHT_TOP)
			{
				w = setup->Pace[scroll_idx];
				if (0 == w)			w = 1;
				else if (w > 1)		w--;
				setup->Pace[scroll_idx] = w;

	printf("(%s %d) WM_DBG(%d %d)\n", __FILE__, __LINE__, scroll_idx, setup->Pace[scroll_idx]);
				return 1;
			}

		}
	}



///////////////////////////////////////////////////////////
	if (source == WP_DISTANCE_A && target == WP_DISTANCE_B)
	{
printf("(%s %d) WM_DBG(%d %d)\n", __FILE__, __LINE__, setup->Workout_distance, rt->workout_state);
		return 1;
	}

	if (source == WP_DISTANCE_B && target == WP_DISTANCE_B)
	{
		if (key == KS_LEFT_TOP)
		{
			setup->Workout_distance = 5000;
			setup->distance = 5000.00F;
printf("(%s %d) WM_DBG(%u)\n", __FILE__, __LINE__, setup->Workout_distance);
			return 1;
		}
		if (key == KS_LEFT_CENTER)
		{
			setup->Workout_distance = 10000;
			setup->distance = 10000.00F;
printf("(%s %d) WM_DBG(%u)\n", __FILE__, __LINE__, setup->Workout_distance);
			return 1;
		}
		if (key == KS_RIGHT_TOP)
		{
			setup->Workout_distance = 2 * 1609;
			setup->distance = (double)setup->Workout_distance;
printf("(%s %d) WM_DBG(%u)\n", __FILE__, __LINE__, setup->Workout_distance);
			return 1;
		}
		if (key == KS_RIGHT_CENTER)
		{
			setup->Workout_distance = 4 * 1609;
			setup->distance = (double)setup->Workout_distance;
printf("(%s %d) WM_DBG(%u)\n", __FILE__, __LINE__, setup->Workout_distance);
			return 1;
		}
	}


///////////////////////////////////////////////////////////
	if (
		(
			(source == WP_MANUAL_A && target == WP_MANUAL_A)
			|| (source == WP_C360_ARM_SCULPTOR && target == WP_C360_ARM_SCULPTOR)
			|| (source == WP_C360_LEG_SHAPER && target == WP_C360_LEG_SHAPER)
			|| (source == WP_C360_CUSTOM_A && target == WP_C360_CUSTOM_A)
			|| (source == WP_CALORIE_GOAL && target == WP_CALORIE_GOAL)
			|| (source == WP_GLUTE_BUSTER && target == WP_GLUTE_BUSTER)
			|| (source == WP_LEG_SHAPER_A && target == WP_LEG_SHAPER_A)
			|| (source == WP_WL_HRC_A && target == WP_WL_HRC_A)
			|| (source == WP_WL_A && target == WP_WL_A)
			|| (source == WP_TARGET_HRC_A && target == WP_TARGET_HRC_A)
			|| (source == WP_AEROBIC_HRC_A && target == WP_AEROBIC_HRC_A)
			|| (source == WP_INTERVAL_HRC_A && target == WP_INTERVAL_HRC_A)
			|| (source == WP_CARDIO_CHALLENGE && target == WP_CARDIO_CHALLENGE)
			|| (source == WP_DISTANCE_A && target == WP_DISTANCE_A)
			|| (source == WP_FITNESS_A && target == WP_FITNESS_A)
			|| (source == WP_WALK_INTERVALS_A && target == WP_WALK_INTERVALS_A)
			|| (source == WP_PACE_INTERVALS_A && target == WP_PACE_INTERVALS_A)
			|| (source == WP_PACE_RAMP_A && target == WP_PACE_RAMP_A)
			|| (source == WP_RANDOM_HILLS && target == WP_RANDOM_HILLS)
			|| (source == WP_ROLLING_HILLS && target == WP_ROLLING_HILLS)
			|| (source == WP_SINGLE_HILL && target == WP_SINGLE_HILL)
			|| (source == WP_HILL_INTVALS && target == WP_HILL_INTVALS)
			|| (source == WP_CUSTOM_UTRA_A && target == WP_CUSTOM_UTRA_A)
		) 
		&& key == KS_LEFT_TOP
	)
	{
		age_focused = 0; weight_focused = 0; workload_focused = 0; distance_focused = 0; time_focused = 0; calorie_focused = 0; gender_focused = 0; pace_focused = 0; segments_focused = 0; hr_focused = 0; scroll_idx = 0;
		age_focused = 1;

		if (target == WP_WL_HRC_A || target == WP_WL_A)	update->Target_heart_rate = setup->Target_heart_rate = (220 - setup->Age)  * 65 / 100;
		if (target == WP_AEROBIC_HRC_A)					update->Target_heart_rate = setup->Target_heart_rate = (220 - setup->Age)  * 80 / 100;
		if (target == WP_FITNESS_A)
		{
			if (0 == setup->Gender)			update->Target_heart_rate = setup->Target_heart_rate = (220 - setup->Age)  * 85 / 100;
			else								update->Target_heart_rate = setup->Target_heart_rate = (210 - setup->Age)  * 85 / 100;
		}
		if (target == WP_TARGET_HRC_A || target == WP_INTERVAL_HRC_A)
		{
			if (setup->Target_heart_rate >= 220 - setup->Age)
				update->Target_heart_rate = setup->Target_heart_rate = (220 - setup->Age) * 99 / 100;
			setup->Work_heart_rate = 100 * setup->Target_heart_rate / (220 - setup->Age);
		}
		printf("(%s %d) WM_DBG(age_focused %d %d %d)\n", __FILE__, __LINE__, setup->Age, setup->Target_heart_rate, setup->Work_heart_rate);
		return 1;
	}
	if (
		(
			(source == WP_MANUAL_A && target == WP_MANUAL_A)
			|| (source == WP_C360_ARM_SCULPTOR && target == WP_C360_ARM_SCULPTOR)
			|| (source == WP_C360_LEG_SHAPER && target == WP_C360_LEG_SHAPER)
			|| (source == WP_C360_CUSTOM_A && target == WP_C360_CUSTOM_A)
			|| (source == WP_CALORIE_GOAL && target == WP_CALORIE_GOAL)
			|| (source == WP_GLUTE_BUSTER && target == WP_GLUTE_BUSTER)
			|| (source == WP_LEG_SHAPER_A && target == WP_LEG_SHAPER_A)
			|| (source == WP_WL_A && target == WP_WL_A)
			|| (source == WP_WL_HRC_A && target == WP_WL_HRC_A)
			|| (source == WP_TARGET_HRC_A && target == WP_TARGET_HRC_A)
			|| (source == WP_AEROBIC_HRC_A && target == WP_AEROBIC_HRC_A)
			|| (source == WP_INTERVAL_HRC_A && target == WP_INTERVAL_HRC_A)
			|| (source == WP_CARDIO_CHALLENGE && target == WP_CARDIO_CHALLENGE)
			|| (source == WP_DISTANCE_A && target == WP_DISTANCE_A)
			|| (source == WP_FITNESS_A && target == WP_FITNESS_A)
			|| (source == WP_WALK_INTERVALS_A && target == WP_WALK_INTERVALS_A)
			|| (source == WP_PACE_INTERVALS_A && target == WP_PACE_INTERVALS_A)
			|| (source == WP_PACE_RAMP_A && target == WP_PACE_RAMP_A)
			|| (source == WP_RANDOM_HILLS && target == WP_RANDOM_HILLS)
			|| (source == WP_ROLLING_HILLS && target == WP_ROLLING_HILLS)
			|| (source == WP_SINGLE_HILL && target == WP_SINGLE_HILL)
			|| (source == WP_HILL_INTVALS && target == WP_HILL_INTVALS)
			|| (source == WP_CUSTOM_UTRA_A && target == WP_CUSTOM_UTRA_A)
		) 
		&& key == KS_LEFT_CENTER
	)
	{
		age_focused = 0; weight_focused = 0; workload_focused = 0; distance_focused = 0; time_focused = 0; calorie_focused = 0; gender_focused = 0; pace_focused = 0; segments_focused = 0; hr_focused = 0; scroll_idx = 0;
		weight_focused = 1;
printf("(%s %d) WM_DBG(weight_focused)\n", __FILE__, __LINE__);
		return 1;
	}
	if (
		(
			//(source == WP_MANUAL_B && target == WP_MANUAL_B)
			(source == WP_C360_ARM_SCULPTOR && target == WP_C360_ARM_SCULPTOR)
			|| (source == WP_C360_LEG_SHAPER && target == WP_C360_LEG_SHAPER)
			|| (source == WP_C360_CUSTOM_A && target == WP_C360_CUSTOM_A)
			|| (source == WP_CALORIE_GOAL && target == WP_CALORIE_GOAL)
			|| (source == WP_GLUTE_BUSTER && target == WP_GLUTE_BUSTER)
			|| (source == WP_LEG_SHAPER_A && target == WP_LEG_SHAPER_A)
			|| (source == WP_WL_A && target == WP_WL_A)
			|| (source == WP_WL_HRC_A&& target == WP_WL_HRC_A)
			|| (source == WP_TARGET_HRC_A && target == WP_TARGET_HRC_A)
			|| (source == WP_AEROBIC_HRC_A && target == WP_AEROBIC_HRC_A)
			|| (source == WP_INTERVAL_HRC_A && target == WP_INTERVAL_HRC_A)
			|| (source == WP_CARDIO_CHALLENGE && target == WP_CARDIO_CHALLENGE)
			//|| (source == WP_DISTANCE_A && target == WP_DISTANCE_A)
			//|| (source == WP_FITNESS_A && target == WP_FITNESS_A)
			|| (source == WP_WALK_INTERVALS_A && target == WP_WALK_INTERVALS_A)
			|| (source == WP_PACE_INTERVALS_A && target == WP_PACE_INTERVALS_A)
			|| (source == WP_PACE_RAMP_A && target == WP_PACE_RAMP_A)
			|| (source == WP_RANDOM_HILLS && target == WP_RANDOM_HILLS)
			|| (source == WP_ROLLING_HILLS && target == WP_ROLLING_HILLS)
			|| (source == WP_SINGLE_HILL && target == WP_SINGLE_HILL)
			|| (source == WP_HILL_INTVALS && target == WP_HILL_INTVALS)
			|| (source == WP_CUSTOM_UTRA_A && target == WP_CUSTOM_UTRA_A)
		) 
		&& key == KS_RIGHT_TOP
	)
	{
		age_focused = 0; weight_focused = 0; workload_focused = 0; distance_focused = 0; time_focused = 0; calorie_focused = 0; gender_focused = 0; pace_focused = 0; segments_focused = 0; hr_focused = 0; scroll_idx = 0;
		time_focused = 1;
printf("(%s %d) WM_DBG(time_focused)\n", __FILE__, __LINE__);
		return 1;
	}
	if (
		(
			(source == WP_C360_ARM_SCULPTOR && target == WP_C360_ARM_SCULPTOR)
			|| (source == WP_C360_LEG_SHAPER && target == WP_C360_LEG_SHAPER)
			|| (source == WP_C360_CUSTOM_A && target == WP_C360_CUSTOM_A)
			|| (source == WP_GLUTE_BUSTER && target == WP_GLUTE_BUSTER)
			|| (source == WP_LEG_SHAPER_A && target == WP_LEG_SHAPER_A)
			|| (source == WP_CARDIO_CHALLENGE && target == WP_CARDIO_CHALLENGE)
			|| (source == WP_DISTANCE_A && target == WP_DISTANCE_A)
			|| (source == WP_WALK_INTERVALS_A && target == WP_WALK_INTERVALS_A)
			|| (source == WP_PACE_INTERVALS_A && target == WP_PACE_INTERVALS_A)
			|| (source == WP_PACE_RAMP_A && target == WP_PACE_RAMP_A)
			|| (source == WP_RANDOM_HILLS && target == WP_RANDOM_HILLS)
			|| (source == WP_ROLLING_HILLS && target == WP_ROLLING_HILLS)
			|| (source == WP_SINGLE_HILL && target == WP_SINGLE_HILL)
			|| (source == WP_HILL_INTVALS && target == WP_HILL_INTVALS)
		) 
		&& key == KS_RIGHT_CENTER
	)
	{
		age_focused = 0; weight_focused = 0; workload_focused = 0; distance_focused = 0; time_focused = 0; calorie_focused = 0; gender_focused = 0; pace_focused = 0; segments_focused = 0; hr_focused = 0; scroll_idx = 0;
		workload_focused = 1;
printf("(%s %d) WM_DBG(workload_focused)\n", __FILE__, __LINE__);
		return 1;
	}

	if (source == WP_MANUAL_A && target == WP_MANUAL_A && key == KS_RIGHT_TOP)
	{
		age_focused = 0; weight_focused = 0; workload_focused = 0; distance_focused = 0; time_focused = 0; calorie_focused = 0; gender_focused = 0; pace_focused = 0; segments_focused = 0; hr_focused = 0; scroll_idx = 0;
		workload_focused = 1;
printf("(%s %d) WM_DBG(workload_focused)\n", __FILE__, __LINE__);
		return 1;
	}
	if (source == WP_CALORIE_GOAL && target == WP_CALORIE_GOAL && key == KS_RIGHT_CENTER)
	{
		age_focused = 0; weight_focused = 0; workload_focused = 0; distance_focused = 0; time_focused = 0; calorie_focused = 0; gender_focused = 0; pace_focused = 0; segments_focused = 0; hr_focused = 0; scroll_idx = 0;
		calorie_focused = 1;
printf("(%s %d) WM_DBG(calorie_focused)\n", __FILE__, __LINE__);
		return 1;
	}
	if (
		(
			(source == WP_LEG_SHAPER_B && target == WP_LEG_SHAPER_B)
			|| (source == WP_WALK_INTERVALS_B && target == WP_WALK_INTERVALS_B)
			|| (source == WP_PACE_INTERVALS_B && target == WP_PACE_INTERVALS_B)
			|| (source == WP_PACE_RAMP_B && target == WP_PACE_RAMP_B)
		)
		&& key == KS_LEFT_TOP
		)
	{
		age_focused = 0; weight_focused = 0; workload_focused = 0; distance_focused = 0; time_focused = 0; calorie_focused = 0; gender_focused = 0; pace_focused = 0; segments_focused = 0; hr_focused = 0; scroll_idx = 0;
		pace_focused = 1;
printf("(%s %d) WM_DBG(pace_focused)\n", __FILE__, __LINE__);
		return 1;
	}
	if (source == WP_FITNESS_A && target == WP_FITNESS_A && key == KS_RIGHT_TOP)
	{
		age_focused = 0; weight_focused = 0; workload_focused = 0; distance_focused = 0; time_focused = 0; calorie_focused = 0; gender_focused = 0; pace_focused = 0; segments_focused = 0; hr_focused = 0; scroll_idx = 0;
		gender_focused = 1;
printf("(%s %d) WM_DBG(gender_focused)\n", __FILE__, __LINE__);
		return 1;
	}
	if (source == WP_CUSTOM_UTRA_A && target == WP_CUSTOM_UTRA_A && key == KS_RIGHT_CENTER)
	{
		age_focused = 0; weight_focused = 0; workload_focused = 0; distance_focused = 0; time_focused = 0; calorie_focused = 0; gender_focused = 0; pace_focused = 0; segments_focused = 0; hr_focused = 0; scroll_idx = 0;
		segments_focused = 1;
printf("(%s %d) WM_DBG(segments_focused)\n", __FILE__, __LINE__);
		return 1;
	}

///////////////////////////////////////////////////////////
	if (1 <= calorie_focused  && (key != KS_LEFT_BOTTOM && key != KS_RIGHT_BOTTOM)
		&& (source == WP_CALORIE_GOAL && target == WP_CALORIE_GOAL)
	)
	{
		unsigned short calorie_hi_limt = 0;
		unsigned short calorie_lo_limt = 0;

		if (1 == product_type)
			calorie_hi_limt = (setup->Workout_time_1000 * 1000 + setup->Workout_time) / 60 * 30, calorie_lo_limt = 3;
		else
			calorie_hi_limt = (setup->Workout_time_1000 * 1000 + setup->Workout_time) / 60 * 44, calorie_lo_limt = 30;
		if (calorie_hi_limt > 999)
			calorie_hi_limt = 999;

		if(key == KS_DELETE)
		{
			setup->Calorie_goal /= 10;
			if (calorie_focused >= 4)														calorie_focused = 3;
			else if (calorie_focused == 3)													calorie_focused = 2;
			else if (calorie_focused == 2)													calorie_focused = 1;
			else																			calorie_focused = 1;
		}
		if(key == KS_WORKLOAD_UP)
		{
			if (setup->Calorie_goal >= calorie_hi_limt)											setup->Calorie_goal = calorie_hi_limt;
			else if (setup->Calorie_goal < calorie_hi_limt && setup->Calorie_goal >= calorie_lo_limt)	setup->Calorie_goal++;
			else																			setup->Calorie_goal = calorie_lo_limt;
			calorie_focused = 1;
		}
		if(key == KS_WORKLOAD_DOWN)
		{
			if (setup->Calorie_goal <= calorie_lo_limt)											setup->Calorie_goal = calorie_lo_limt;
			else if (setup->Calorie_goal <= calorie_hi_limt && setup->Calorie_goal > calorie_lo_limt)	setup->Calorie_goal--;
			else																			setup->Calorie_goal = calorie_hi_limt;
			calorie_focused = 1;
		}
		if(key == KS_NUM0 || key == KS_NUM1 || key == KS_NUM2 
			|| key == KS_NUM3 || key == KS_NUM4 || key == KS_NUM5 
				|| key == KS_NUM6 || key == KS_NUM7 || key == KS_NUM8 || key == KS_NUM9)
		{
			if(1 == calorie_focused)
			{
				calorie_focused = 2;
				focus_key[0] = (key - KS_NUM0);
				setup->Calorie_goal = focus_key[0];
				if(0 == setup->Calorie_goal)
					calorie_focused = 1;
			}
			else if(2 == calorie_focused)
			{
				calorie_focused = 3;
				focus_key[1] = (key - KS_NUM0);
				setup->Calorie_goal = 10 * focus_key[0] + focus_key[1];
				if(0 == setup->Calorie_goal)
					calorie_focused = 1;
			}
			else if(3 == calorie_focused)
			{
				calorie_focused = 4;
				focus_key[2] = (key - KS_NUM0);
				setup->Calorie_goal = 100 * focus_key[0] + 10 * focus_key[1] + focus_key[2];
				if (setup->Calorie_goal > calorie_hi_limt)
					setup->Calorie_goal = calorie_hi_limt;
				if(setup->Calorie_goal > 999 || setup->Calorie_goal < calorie_lo_limt)
					calorie_focused = 1;
			}
		}
		if (setup->Calorie_goal > calorie_hi_limt || setup->Calorie_goal < calorie_lo_limt)			focus_ok = 0;
		else																				focus_ok = 1;

/////////////////////
		if (focus_ok)
		{
			//static unsigned short watt_table[] = { 39, 41,  44,  48,  51,  55,  59,  65,  71,  78,  88,  96,  106, 116,  133,  150,  170,  180, 204, 228, 252, 265, 273, 278, 280, 280, 280, 280, 280, 280, 280 };
			unsigned short Watt_workload = 0;
			unsigned short minutes = 0;

			minutes = (1000 * setup->Workout_time_1000 + setup->Workout_time) / 60;

			if (1 == product_type)
			{
				Watt_workload = setup->Calorie_goal / minutes * 60 / 3 * 10 * 7 / 100 - setup->Weight * 7 / 100 * 35 / 10;
				setup->Workload_level = tables->Get_70rpm_Level_ByWatt(Watt_workload);
			}
			else
			{
				Watt_workload = ((setup->Calorie_goal / minutes) * 60 - 120) / 9;
				setup->Workload_level = tables->Get_60rpm_Level_ByWatt(Watt_workload);
			}

/*
			Watt_workload = ((setup->Calorie_goal / minutes) * 60 - 120) / 9;
			for (i = 0; i < (int)(sizeof(watt_table) / sizeof(unsigned short)) - 1; i++)
			{
				if (Watt_workload < watt_table[i + 1] && Watt_workload >= watt_table[i])
					break;
			}
			if((int)(sizeof(watt_table) / sizeof(unsigned short)) - 1 == i)
				setup->Workload_level = 1;
			else
			{
				setup->Workload_level = i + 1;
				if (setup->Workload_level > 30)
					setup->Workload_level = 30;
			}
*/

			setup->Segments = (setup->Workout_time_1000 * 1 + setup->Workout_time) / (60 * minutes);
			if (setup->Segments > 30)				setup->Segments = 30;
			if (setup->Segments == 0)				setup->Segments = 20;
			for (i = 0; i < 30; i++)
			{
				if (i < setup->Segments)			setup->Segments_time[i] = 60;
				else								setup->Segments_time[i] = 0;
			}
		}

printf("(%s %d) WM_DBG(%d)\n", __FILE__, __LINE__, setup->Calorie_goal);
		return 1;
	}

///////////////////////////////////////////////////////////
	if (1 <= pace_focused && (key != KS_LEFT_BOTTOM && key != KS_RIGHT_BOTTOM) &&
		(
			(source == WP_LEG_SHAPER_B && target == WP_LEG_SHAPER_B)
			|| (source == WP_WALK_INTERVALS_B && target == WP_WALK_INTERVALS_B)
			|| (source == WP_PACE_INTERVALS_B && target == WP_PACE_INTERVALS_B)
			|| (source == WP_PACE_RAMP_B && target == WP_PACE_RAMP_B)
		)
	)
	{
		if(key == KS_DELETE)
		{
			setup->Pace_level /= 10;
			if (pace_focused >= 4)										pace_focused = 3;
			else if (pace_focused == 3)									pace_focused = 2;
			else if (pace_focused == 2)									pace_focused = 1;
			else															pace_focused = 1;
		}
		if(key == KS_WORKLOAD_UP)
		{
			if(setup->Pace_level >= 20)										setup->Pace_level = 20;
			else if(setup->Pace_level >= 1 && setup->Pace_level < 20)			setup->Pace_level++;
			else															setup->Pace_level = 1;
			pace_focused = 1;
		}
		if(key == KS_WORKLOAD_DOWN)
		{
			if(setup->Pace_level <= 1)										setup->Pace_level = 1;
			else if(setup->Pace_level > 1 && setup->Pace_level <= 20)			setup->Pace_level--;
			else															setup->Pace_level = 20;
			pace_focused = 1;
		}
		if(key == KS_NUM0 || key == KS_NUM1 || key == KS_NUM2 
			|| key == KS_NUM3 || key == KS_NUM4 || key == KS_NUM5 
				|| key == KS_NUM6 || key == KS_NUM7 || key == KS_NUM8 || key == KS_NUM9)
		{
			if(1 == pace_focused)
			{
				pace_focused = 2;
				focus_key[0] = (key - KS_NUM0);
				setup->Pace_level = focus_key[0];
				if(0 == setup->Pace_level)
					pace_focused = 1;
			}
			else if(2 == pace_focused)
			{
				pace_focused = 3;
				focus_key[1] = (key - KS_NUM0);
				setup->Pace_level = 10 * focus_key[0] + focus_key[1];
				if(setup->Pace_level > 20 || setup->Pace_level < 1)
					pace_focused = 1;
			}
		}
		if (setup->Pace_level > 20 || setup->Pace_level < 1)				focus_ok = 0;
		else															focus_ok = 1;
printf("(%s %d) WM_DBG(%d), pace_focused=%d, focus_key[]=(%d,%d)\n", __FILE__, __LINE__, setup->Pace_level, pace_focused, focus_key[0], focus_key[1]);
		return 1;
	}

	if (1 <= gender_focused && key != KS_RIGHT_BOTTOM && (source == WP_FITNESS_A && target == WP_FITNESS_A))
	{
		if(key == KS_WORKLOAD_UP || key == KS_WORKLOAD_DOWN)
		{
			if(0 == setup->Gender)				setup->Gender = 1;
			else									setup->Gender = 0;

			if (0 == setup->Gender)				update->Target_heart_rate = setup->Target_heart_rate = (220 - setup->Age)  * 85 / 100;
			else									update->Target_heart_rate = setup->Target_heart_rate = (210 - setup->Age)  * 85 / 100;
		}
		focus_ok = 1;
printf("(%s %d) WM_DBG(%d)\n", __FILE__, __LINE__, setup->Gender);
		return 1;
	}

	if (1 <= segments_focused  && key != KS_RIGHT_BOTTOM
		&& (source == WP_CUSTOM_UTRA_A && target == WP_CUSTOM_UTRA_A)
	)
	{
		if(key == KS_DELETE)
		{
			if (setup->Segments >= 10)	
			{
				setup->Segments /= 10;
				if (segments_focused >= 4)									segments_focused = 3;
				else if (segments_focused == 3)								segments_focused = 2;
				else if (segments_focused == 2)								segments_focused = 1;
				else															segments_focused = 1;
			}
			else
			{
				setup->Segments = 2;
				segments_focused = 1;
			}
		}
		if(key == KS_WORKLOAD_UP)
		{
// jason note
// 抓時間, 不能讓 seg_time < 1分鐘
			unsigned short min_segment_time = 20;

			if (setup->Segments > 0)
				min_segment_time = (setup->Workout_time_1000 * 1000 + setup->Workout_time) / setup->Segments;
			if (min_segment_time > 60)
			{
				if(setup->Segments >= 30)
					setup->Segments = 30;
				else if(setup->Segments >= 1 && setup->Segments < 30)
					setup->Segments++;
				else	
					setup->Segments = 1;

			}
			else
			{
				setup->Segments = (setup->Workout_time_1000 * 1000 + setup->Workout_time) / 60;
			}
			segments_focused = 1;
		}
		if(key == KS_WORKLOAD_DOWN)
		{
			if(setup->Segments <= 2)									setup->Segments = 2;
			else if(setup->Segments > 2 && setup->Segments <= 30)		setup->Segments--;
			else														setup->Segments = 30;
			segments_focused = 1;
		}
		if(key == KS_NUM0 || key == KS_NUM1 || key == KS_NUM2 
			|| key == KS_NUM3 || key == KS_NUM4 || key == KS_NUM5 
				|| key == KS_NUM6 || key == KS_NUM7 || key == KS_NUM8 || key == KS_NUM9)
		{
			if(1 == segments_focused)
			{
				segments_focused = 2;
				focus_key[0] = (key - KS_NUM0);
				setup->Segments = focus_key[0];
				if(0 == setup->Segments)
					segments_focused = 1;
			}
			else if(2 == segments_focused)
			{
				segments_focused = 3;
				focus_key[1] = (key - KS_NUM0);
				setup->Segments = 10 * focus_key[0] + focus_key[1];
				if(setup->Segments > 30 || setup->Segments < 1)
					segments_focused = 1;
			}
		}
		if (setup->Segments > 30 || setup->Segments < 1)				focus_ok = 0;
		else	
		{
			for (i = 0; i < 30; i++)
			{
				if (i < setup->Segments)
				{
					if (0 == setup->Segments_time[i])
						setup->Segments_time[i] = 60;
				}
				else
				{
					setup->Segments_time[i] = 0;
				}
			}
			focus_ok = 1;
		}
printf("(%s %d) WM_DBG(%d)\n", __FILE__, __LINE__, setup->Segments);
		return 1;
	}


///////////////////////////////////////////////////////////
	if (1 <= workload_focused && (key != KS_LEFT_BOTTOM && key != KS_RIGHT_BOTTOM) &&
		(
			(source == WP_GLUTE_BUSTER && target == WP_GLUTE_BUSTER)
			|| (source == WP_ROLLING_HILLS && target == WP_ROLLING_HILLS)
			|| (source == WP_ROLLING_HILLS && target == WP_ROLLING_HILLS)
			|| (source == WP_HILL_INTVALS && target == WP_HILL_INTVALS)
			|| (source == WP_SINGLE_HILL && target == WP_SINGLE_HILL)
			|| (source == WP_RANDOM_HILLS && target == WP_RANDOM_HILLS)
			|| (source == WP_CARDIO_CHALLENGE && target == WP_CARDIO_CHALLENGE)
		)
	)
	{
		if(key == KS_DELETE)
		{
			setup->Workload_level /= 10;
			if (workload_focused >= 4)									workload_focused = 3;
			else if (workload_focused == 3)								workload_focused = 2;
			else if (workload_focused == 2)								workload_focused = 1;
			else															workload_focused = 1;
		}
		if(key == KS_WORKLOAD_UP)
		{
			if(setup->Workload_level >= 20)									setup->Workload_level = 20;
			else if(setup->Workload_level >= 1 && setup->Workload_level < 20)	setup->Workload_level++;
			else															setup->Workload_level = 1;
			workload_focused = 1;
		}
		if(key == KS_WORKLOAD_DOWN)
		{
			if(setup->Workload_level <= 1)									setup->Workload_level = 1;
			else if(setup->Workload_level > 1 && setup->Workload_level <= 20)	setup->Workload_level--;
			else															setup->Workload_level = 20;
			workload_focused = 1;
		}
		if(key == KS_NUM0 || key == KS_NUM1 || key == KS_NUM2 
			|| key == KS_NUM3 || key == KS_NUM4 || key == KS_NUM5 
				|| key == KS_NUM6 || key == KS_NUM7 || key == KS_NUM8 || key == KS_NUM9)
		{
			if(1 == workload_focused)
			{
				workload_focused = 2;
				focus_key[0] = (key - KS_NUM0);
				setup->Workload_level = focus_key[0];
				if(0 == setup->Workload_level)
					workload_focused = 1;
			}
			else if(2 == workload_focused)
			{
				workload_focused = 3;
				focus_key[1] = (key - KS_NUM0);
				setup->Workload_level = 10 * focus_key[0] + focus_key[1];
				if(setup->Workload_level > 20 || setup->Workload_level < 1)
					workload_focused = 1;
			}
		}
		if (setup->Workload_level > 20 || setup->Workload_level < 1)			focus_ok = 0; 
		else																focus_ok = 1;
		printf("(%s %d) WM_DBG(%d)\n", __FILE__, __LINE__, setup->Workload_level);
		return 1;
	}


///////////////////////////////////////////////////////////
	if (1 <= age_focused && (key != KS_LEFT_BOTTOM && key != KS_RIGHT_BOTTOM) 
		&& (
			(source == WP_WL_HRC_A && target == WP_WL_HRC_A)
			|| (source == WP_WL_A && target == WP_WL_A)
			|| (source == WP_AEROBIC_HRC_A && target == WP_AEROBIC_HRC_A)
			|| (source == WP_FITNESS_A && target == WP_FITNESS_A)

			|| (source == WP_TARGET_HRC_A && target == WP_TARGET_HRC_A)
			|| (source == WP_INTERVAL_HRC_A && target == WP_INTERVAL_HRC_A)
		)
	)
	{
		if(key == KS_DELETE)
		{
			setup->Age /= 10;
			if (age_focused >= 4)											age_focused = 3;
			else if (age_focused == 3)									age_focused = 2;
			else if (age_focused == 2)									age_focused = 1;
			else															age_focused = 1;
		}
		if(key == KS_WORKLOAD_UP)
		{
			if(setup->Age >= 99)											setup->Age = 99;
			else if(setup->Age >= 10 && setup->Age < 99)					setup->Age++;
			else															setup->Age= 10;
			age_focused = 1;
		}
		if(key == KS_WORKLOAD_DOWN)
		{
			if(setup->Age <= 10)											setup->Age = 10;
			else if(setup->Age > 10 && setup->Age <= 99)					setup->Age--;
			else															setup->Age = 99;
			age_focused = 1;
		}
		if(key == KS_NUM0 || key == KS_NUM1 || key == KS_NUM2 
			|| key == KS_NUM3 || key == KS_NUM4 || key == KS_NUM5 
				|| key == KS_NUM6 || key == KS_NUM7 || key == KS_NUM8 || key == KS_NUM9)
		{
			if(1 == age_focused)
			{
				age_focused = 2;
				focus_key[0] = (key - KS_NUM0);
				setup->Age = focus_key[0];
				if(0 == setup->Age)
					age_focused = 1;
			}
			else if(2 == age_focused)
			{
				age_focused = 3;
				focus_key[1] = (key - KS_NUM0);
				setup->Age = 10 * focus_key[0] + focus_key[1];
				if(setup->Age > 99 || setup->Age < 10)
					age_focused = 1;
			}
		}
		if (setup->Age > 99 || setup->Age < 10)								focus_ok = 0;
		else
		{
			if (source == WP_TARGET_HRC_A || source == WP_INTERVAL_HRC_A)
			{
				if (setup->Target_heart_rate >= 220 - setup->Age)
					update->Target_heart_rate = setup->Target_heart_rate = (220 - setup->Age) * 99 / 100;
				setup->Work_heart_rate = 100 * setup->Target_heart_rate / (220 - setup->Age);
			}
			else if (target == WP_WL_HRC_A || target == WP_WL_A)
			{
				update->Target_heart_rate = setup->Target_heart_rate = (220 - setup->Age)  * 65 / 100;
			}
			else if (target == WP_AEROBIC_HRC_A)
			{
				update->Target_heart_rate = setup->Target_heart_rate = (220 - setup->Age)  * 80 / 100;
			}
			else if (target == WP_FITNESS_A)
			{
				update->Target_heart_rate = setup->Target_heart_rate = (220 - setup->Age)  * 85 / 100;
			}
			focus_ok = 1;
		}
		printf("(%s %d) WM_DBG(%d %d %d)\n", __FILE__, __LINE__, setup->Age, setup->Target_heart_rate, setup->Work_heart_rate);
		return 1;
	}


	if (1 <= age_focused && (key != KS_LEFT_BOTTOM && key != KS_RIGHT_BOTTOM) 
		&& (
			(source == WP_MANUAL_A && target == WP_MANUAL_A)
			|| (source == WP_C360_ARM_SCULPTOR && target == WP_C360_ARM_SCULPTOR)
			|| (source == WP_C360_LEG_SHAPER && target == WP_C360_LEG_SHAPER)
			|| (source == WP_C360_CUSTOM_A && target == WP_C360_CUSTOM_A)
			|| (source == WP_CALORIE_GOAL && target == WP_CALORIE_GOAL)
			|| (source == WP_GLUTE_BUSTER && target == WP_GLUTE_BUSTER)
			|| (source == WP_LEG_SHAPER_A && target == WP_LEG_SHAPER_A)
			|| (source == WP_CARDIO_CHALLENGE && target == WP_CARDIO_CHALLENGE)
			|| (source == WP_DISTANCE_A && target == WP_DISTANCE_A)
			|| (source == WP_WALK_INTERVALS_A && target == WP_WALK_INTERVALS_A)
			|| (source == WP_PACE_INTERVALS_A && target == WP_PACE_INTERVALS_A)
			|| (source == WP_PACE_RAMP_A && target == WP_PACE_RAMP_A)
			|| (source == WP_ROLLING_HILLS && target == WP_ROLLING_HILLS)
			|| (source == WP_HILL_INTVALS && target == WP_HILL_INTVALS)
			|| (source == WP_SINGLE_HILL && target == WP_SINGLE_HILL)
			|| (source == WP_RANDOM_HILLS && target == WP_RANDOM_HILLS)
			|| (source == WP_CUSTOM_UTRA_A && target == WP_CUSTOM_UTRA_A)
		)
	)
	{
		if(key == KS_DELETE)
		{
			setup->Age /= 10;
			if (age_focused >= 4)											age_focused = 3;
			else if (age_focused == 3)									age_focused = 2;
			else if (age_focused == 2)									age_focused = 1;
			else															age_focused = 1;
		}
		if(key == KS_WORKLOAD_UP)
		{
			if(setup->Age >= 99)											setup->Age = 99;
			else if(setup->Age >= 10 && setup->Age < 99)					setup->Age++;
			else															setup->Age= 10;
			age_focused = 1;
		}
		if(key == KS_WORKLOAD_DOWN)
		{
			if(setup->Age <= 10)											setup->Age = 10;
			else if(setup->Age > 10 && setup->Age <= 99)					setup->Age--;
			else															setup->Age = 99;
			age_focused = 1;
		}
		if(key == KS_NUM0 || key == KS_NUM1 || key == KS_NUM2 
			|| key == KS_NUM3 || key == KS_NUM4 || key == KS_NUM5 
				|| key == KS_NUM6 || key == KS_NUM7 || key == KS_NUM8 || key == KS_NUM9)
		{
			if(1 == age_focused)
			{
				age_focused = 2;
				focus_key[0] = (key - KS_NUM0);
				setup->Age = focus_key[0];
				if(0 == setup->Age)
					age_focused = 1;
			}
			else if(2 == age_focused)
			{
				age_focused = 3;
				focus_key[1] = (key - KS_NUM0);
				setup->Age = 10 * focus_key[0] + focus_key[1];
				if(setup->Age > 99 || setup->Age < 10)
					age_focused = 1;
			}
		}
		if (setup->Age > 99 || setup->Age < 10)			focus_ok = 0;
		else											focus_ok = 1;
		printf("(%s %d) WM_DBG(%d)\n", __FILE__, __LINE__, setup->Age);
		return 1;
	}

///////////////////////////////////////////////////////////
	if (1 <= weight_focused && (key != KS_LEFT_BOTTOM && key != KS_RIGHT_BOTTOM) &&
		(
			(source == WP_MANUAL_A && target == WP_MANUAL_A)
			|| (source == WP_C360_ARM_SCULPTOR && target == WP_C360_ARM_SCULPTOR)
			|| (source == WP_C360_LEG_SHAPER && target == WP_C360_LEG_SHAPER)
			|| (source == WP_C360_CUSTOM_A && target == WP_C360_CUSTOM_A)
			|| (source == WP_CALORIE_GOAL && target == WP_CALORIE_GOAL)
			|| (source == WP_GLUTE_BUSTER && target == WP_GLUTE_BUSTER)
			|| (source == WP_LEG_SHAPER_A && target == WP_LEG_SHAPER_A)
			|| (source == WP_WL_HRC_A && target == WP_WL_HRC_A)
			|| (source == WP_WL_A && target == WP_WL_A)
			|| (source == WP_TARGET_HRC_A && target == WP_TARGET_HRC_A)
			|| (source == WP_AEROBIC_HRC_A && target == WP_AEROBIC_HRC_A)
			|| (source == WP_INTERVAL_HRC_A && target == WP_INTERVAL_HRC_A)
			|| (source == WP_CARDIO_CHALLENGE && target == WP_CARDIO_CHALLENGE)
			|| (source == WP_DISTANCE_A && target == WP_DISTANCE_A)
			|| (source == WP_FITNESS_A && target == WP_FITNESS_A)
			|| (source == WP_WALK_INTERVALS_A && target == WP_WALK_INTERVALS_A)
			|| (source == WP_PACE_INTERVALS_A && target == WP_PACE_INTERVALS_A)
			|| (source == WP_PACE_RAMP_A && target == WP_PACE_RAMP_A)
			|| (source == WP_ROLLING_HILLS && target == WP_ROLLING_HILLS)
			|| (source == WP_HILL_INTVALS && target == WP_HILL_INTVALS)
			|| (source == WP_SINGLE_HILL && target == WP_SINGLE_HILL)
			|| (source == WP_RANDOM_HILLS && target == WP_RANDOM_HILLS)
			|| (source == WP_CUSTOM_UTRA_A && target == WP_CUSTOM_UTRA_A)
		)
	)
	{
		double p = 150.00F;
		int m;

		if(key == KS_DELETE)
		{
			if(4 == weight_focused)
			{
				weight_focused = 3;
				focus_key[2] = 0;
				m = 10 * focus_key[0] + focus_key[1];
				setup->weight = (double)m / (double)2.2F;
				setup->Weight = (unsigned short)round(setup->weight);
				if(0 == m)
					weight_focused = 1;
			}
			else if(3 == weight_focused)
			{
				weight_focused = 2;
				focus_key[1] = 0;
				m = focus_key[0];
				setup->weight = (double)m / (double)2.2F;
				setup->Weight = (unsigned short)round(setup->weight);
				if(0 == m)
					weight_focused = 1;
			}
			else //if(1 == weight_focused)
			{
				weight_focused = 1;
				focus_key[0] = 0;
				m = 0;
				setup->weight = (double)m / (double)2.2F;
				setup->Weight = (unsigned short)round(setup->weight);
			}
		}
		if(key == KS_WORKLOAD_UP)
		{
			p = setup->weight * (double)2.2F;
			if (p >= weight_max)											p = weight_max;
			else if (p < weight_max && p >= 40)								printf("\n++"), p += 1;
			else															p = 40;
			setup->weight = (double)p / (double)2.2F;
			setup->Weight = (unsigned short)round(setup->weight);
			weight_focused = 1;
		}
		if(key == KS_WORKLOAD_DOWN)
		{
			p = setup->weight * (double)2.2F;
			if ((int)p <= 40)
				p = 40;
			else if (p <= weight_max && p > 40)								p -= 1;
			else															p = weight_max;
			setup->weight = (double)p / (double)2.2F;
			setup->Weight = (unsigned short)round(setup->weight);
			weight_focused = 1;
		}
		if(key == KS_NUM0 || key == KS_NUM1 || key == KS_NUM2 
			|| key == KS_NUM3 || key == KS_NUM4 || key == KS_NUM5 
				|| key == KS_NUM6 || key == KS_NUM7 || key == KS_NUM8 || key == KS_NUM9)
		{
			if(1 == weight_focused)
			{
				weight_focused = 2;
				focus_key[0] = (key - KS_NUM0);
				m = focus_key[0];
				setup->weight = (double)m / (double)2.2F;
				setup->Weight = (unsigned short)round(setup->weight);
				if(0 == m)
					weight_focused = 1;
			}
			else if(2 == weight_focused)
			{
				weight_focused = 3;
				focus_key[1] = (key - KS_NUM0);
				m = 10 * focus_key[0] + focus_key[1];
				setup->weight = (double)m / (double)2.2F;
				setup->Weight = (unsigned short)round(setup->weight);
				if(0 == m)
					weight_focused = 1;
			}
			else if(3 == weight_focused)
			{
				weight_focused = 4;
				focus_key[2] = (key - KS_NUM0);
				m = 100 * focus_key[0] + 10 * focus_key[1] + focus_key[2];
				setup->weight = (double)m / (double)2.2F;
				setup->Weight = (unsigned short)round(setup->weight);
				if (m < 40 || m > weight_max)
					weight_focused = 1;
			}
		}
		p = setup->weight * (double)2.2F;
		if ((int)p < 40 || (int)p > weight_max)			focus_ok = 0;
		else							focus_ok = 1;
printf("(%s %d) WM_DBG(%d %f)\n", __FILE__, __LINE__, setup->Weight, p);
		return 1;
	}

	if (1 <= workload_focused && key != KS_RIGHT_BOTTOM && 
		(
			(source == WP_MANUAL_A && target == WP_MANUAL_A)
			|| (source == WP_DISTANCE_A && target == WP_DISTANCE_A)
			|| (source == WP_WALK_INTERVALS_A && target == WP_WALK_INTERVALS_A)
			|| (source == WP_PACE_RAMP_A && target == WP_PACE_RAMP_A)
			|| (source == WP_PACE_INTERVALS_A && target == WP_PACE_INTERVALS_A)
			|| (source == WP_LEG_SHAPER_A && target == WP_LEG_SHAPER_A)
			|| (source == WP_C360_ARM_SCULPTOR && target == WP_C360_ARM_SCULPTOR)
			|| (source == WP_C360_LEG_SHAPER && target == WP_C360_LEG_SHAPER)
			|| (source == WP_C360_CUSTOM_A && target == WP_C360_CUSTOM_A)
		)
	)
	{
		if(key == KS_DELETE)
		{
			setup->Workload_level /= 10;
			if (workload_focused >= 4)									workload_focused = 3;
			else if (workload_focused == 3)								workload_focused = 2;
			else if (workload_focused == 2)								workload_focused = 1;
			else															workload_focused = 1;
		}
		if(key == KS_WORKLOAD_UP)
		{
			if(setup->Workload_level >= 30)									setup->Workload_level = 30;
			else if(setup->Workload_level >= 1 && setup->Workload_level < 30)	setup->Workload_level++;
			else															setup->Workload_level = 1;
			workload_focused = 1;
		}
		if(key == KS_WORKLOAD_DOWN)
		{
			if(setup->Workload_level <= 1)									setup->Workload_level = 1;
			else if(setup->Workload_level > 1 && setup->Workload_level <= 30)	setup->Workload_level--;
			else															setup->Workload_level = 30;
			workload_focused = 1;
		}
		if(key == KS_NUM0 || key == KS_NUM1 || key == KS_NUM2 
			|| key == KS_NUM3 || key == KS_NUM4 || key == KS_NUM5 
				|| key == KS_NUM6 || key == KS_NUM7 || key == KS_NUM8 || key == KS_NUM9)
		{
			if(1 == workload_focused)
			{
				workload_focused = 2;
				focus_key[0] = (key - KS_NUM0);
				setup->Workload_level = focus_key[0];
				if(0 == setup->Workload_level)
					workload_focused = 1;
			}
			else if(2 == workload_focused)
			{
				workload_focused = 3;
				focus_key[1] = (key - KS_NUM0);
				setup->Workload_level = 10 * focus_key[0] + focus_key[1];
				if(setup->Workload_level > 30 || setup->Workload_level < 1)
					workload_focused = 1;
			}
		}
		if (setup->Workload_level > 30 || setup->Workload_level < 1)		focus_ok = 0;
		else															focus_ok = 1;

printf("(%s %d) WM_DBG(%d)\n", __FILE__, __LINE__, setup->Workload_level);
		return 1;
	}

///////////////////////////////////////////////////////////
	if (source == WP_MANUAL_B && target == WP_MANUAL_B && key == KS_LEFT_TOP)
	{
		age_focused = 0; weight_focused = 0; workload_focused = 0; distance_focused = 0; time_focused = 0; calorie_focused = 0; gender_focused = 0; pace_focused = 0; segments_focused = 0; hr_focused = 0; scroll_idx = 0;
		time_focused = 1;
printf("(%s %d) WM_DBG(time_focused)\n", __FILE__, __LINE__);
		return 1;
	}
	if (source == WP_MANUAL_B && target == WP_MANUAL_B && key == KS_RIGHT_TOP)
	{
		age_focused = 0; weight_focused = 0; workload_focused = 0; distance_focused = 0; time_focused = 0; calorie_focused = 0; gender_focused = 0; pace_focused = 0; segments_focused = 0; hr_focused = 0; scroll_idx = 0;
		distance_focused = 1;
		manual_distance_flag = 1;
printf("(%s %d) WM_DBG(distance_focused %d %f)\n", __FILE__, __LINE__, setup->Workout_distance, setup->distance);
		return 1;
	}

///////////////////////////////////////////////////////////
	if (1 <= distance_focused && (key != KS_LEFT_BOTTOM && key != KS_RIGHT_BOTTOM)
		&& (source == WP_MANUAL_B && target == WP_MANUAL_B)
	)
	{
		double p;
		unsigned short m;

		if(key == KS_DELETE)
		{
			setup->Workout_distance /= 10;
			setup->distance = (double)setup->Workout_distance;
			if (distance_focused >= 4)									distance_focused = 3;
			else if (distance_focused == 3)								distance_focused = 2;
			else if (distance_focused == 2)								distance_focused = 1;
			else															distance_focused = 1;
		}
		if(key == KS_WORKLOAD_UP)
		{
			p = setup->distance / (double)1000.00F / (double)1.609344F * (double)10.00F;
			if (p >= 500)													p = 500;
			else if (p < 500 && p >= 1)										p = p + 1;
			else															p = 1;
			setup->distance = p * (double)1.609344F * (double)1000.00F / (double)10.00F;
			setup->Workout_distance = (unsigned int)round(setup->distance);
			distance_focused = 1;
		}
		if(key == KS_WORKLOAD_DOWN)
		{
			p = setup->distance / (double)1000.00F / (double)1.609344F * (double)10.00F;
			if (p <= 1)													p = 1;
			else if (p <= 500 && p > 1)										p = p - 1;
			else															p = 500;
			setup->distance = p * (double)1.609344F * (double)1000.00F / (double)10.00F;
			setup->Workout_distance = (unsigned int)round(setup->distance);
			distance_focused = 1;
		}
		if(key == KS_NUM0 || key == KS_NUM1 || key == KS_NUM2 
			|| key == KS_NUM3 || key == KS_NUM4 || key == KS_NUM5 
				|| key == KS_NUM6 || key == KS_NUM7 || key == KS_NUM8 || key == KS_NUM9)
		{
			if(1 == distance_focused)
			{
				distance_focused = 2;
				focus_key[0] = (key - KS_NUM0);
				m = focus_key[0];
				setup->distance = (double)m * (double)1.609344F * (double)1000.00F / (double)10.00F;
				setup->Workout_distance = (unsigned int)round(setup->distance);
				if(0 == m)
					distance_focused = 1;
			}
			else if(2 == distance_focused)
			{
				distance_focused = 3;
				focus_key[1] = (key - KS_NUM0);
				m = 10 * focus_key[0] + focus_key[1];
				setup->distance = (double)m * (double)1.609344F * (double)1000.00F / (double)10.00F;
				setup->Workout_distance = (unsigned int)round(setup->distance);
				if(0 == m)
					distance_focused = 1;
			}
			else if(3 == distance_focused)
			{
				distance_focused = 4;
				focus_key[2] = (key - KS_NUM0);
				m = 100 * focus_key[0] + 10 * focus_key[1] + focus_key[2];
				setup->distance = (double)m * (double)1.609344F * (double)1000.00F / (double)10.00F;
				setup->Workout_distance = (unsigned int)round(setup->distance);
				if (m < 1 || m > 500)
					distance_focused = 1;
			}
		}
		p = setup->distance / (double)1000.00F / (double)1.609344F * (double)10.00F;
		if (p < 1 || p > 500)			focus_ok = 0;
		else							focus_ok = 1;
printf("(%s %d) WM_DBG(%u %f)\n", __FILE__, __LINE__, setup->Workout_distance, p);
		return 1;
	}

///////////////////////////////////////////////////////////
	if (1 <= time_focused && (key != KS_LEFT_BOTTOM && key != KS_RIGHT_BOTTOM) && 
		(
			(source == WP_MANUAL_B && target == WP_MANUAL_B)
			|| (source == WP_C360_ARM_SCULPTOR && target == WP_C360_ARM_SCULPTOR)
			|| (source == WP_C360_LEG_SHAPER && target == WP_C360_LEG_SHAPER)
			|| (source == WP_C360_CUSTOM_A && target == WP_C360_CUSTOM_A)
			|| (source == WP_CALORIE_GOAL && target == WP_CALORIE_GOAL)
			|| (source == WP_GLUTE_BUSTER && target == WP_GLUTE_BUSTER)
			|| (source == WP_LEG_SHAPER_A && target == WP_LEG_SHAPER_A)
			|| (source == WP_WL_HRC_A && target == WP_WL_HRC_A)
			|| (source == WP_WL_A && target == WP_WL_A)
			|| (source == WP_TARGET_HRC_A && target == WP_TARGET_HRC_A)
			|| (source == WP_AEROBIC_HRC_A && target == WP_AEROBIC_HRC_A)
			|| (source == WP_INTERVAL_HRC_A && target == WP_INTERVAL_HRC_A)
			|| (source == WP_CARDIO_CHALLENGE && target == WP_CARDIO_CHALLENGE)
			|| (source == WP_WALK_INTERVALS_A && target == WP_WALK_INTERVALS_A)
			|| (source == WP_PACE_INTERVALS_A && target == WP_PACE_INTERVALS_A)
			|| (source == WP_PACE_RAMP_A && target == WP_PACE_RAMP_A)
			|| (source == WP_ROLLING_HILLS && target == WP_ROLLING_HILLS)
			|| (source == WP_HILL_INTVALS && target == WP_HILL_INTVALS)
			|| (source == WP_SINGLE_HILL && target == WP_SINGLE_HILL)
			|| (source == WP_RANDOM_HILLS && target == WP_RANDOM_HILLS)
			|| (source == WP_CUSTOM_UTRA_A && target == WP_CUSTOM_UTRA_A)
		)
	)
	{
		unsigned short t = 0;

		if(key == KS_DELETE)
		{
			t = (setup->Workout_time_1000 * 1000 + setup->Workout_time) / 60;
			t /= 10;
			setup->Workout_time_1000 = (t * 60) / 1000;
			setup->Workout_time = (t * 60) % 1000;
			if (time_focused >= 4)									time_focused = 3;
			else if (time_focused == 3)								time_focused = 2;
			else if (time_focused == 2)								time_focused = 1;
			else														time_focused = 1;
		}
		if(key == KS_WORKLOAD_UP)
		{
printf("(%s %d) ++)\n", __FILE__, __LINE__);

			t = (setup->Workout_time_1000 * 1000 + setup->Workout_time) / 60;
			if(t >= 99)						t = 99;
			else if(t < 99 && t >= 5)			t++;
			else								t = 5;
			setup->Workout_time_1000 = (t * 60) / 1000;
			setup->Workout_time = (t * 60) % 1000;
			time_focused = 1;
		}
		if(key == KS_WORKLOAD_DOWN)
		{
printf("(%s %d) --\n", __FILE__, __LINE__);

			t = (setup->Workout_time_1000 * 1000 + setup->Workout_time) / 60;
			if(t <= 5)						t = 5;
			else if(t <= 99 && t > 5)			t--;
			else								t = 99;
			setup->Workout_time_1000 = (t * 60) / 1000;
			setup->Workout_time = (t * 60) % 1000;
			time_focused = 1;
		}
		if(key == KS_NUM0 || key == KS_NUM1 || key == KS_NUM2 
			|| key == KS_NUM3 || key == KS_NUM4 || key == KS_NUM5 
				|| key == KS_NUM6 || key == KS_NUM7 || key == KS_NUM8 || key == KS_NUM9)
		{
			if(1 == time_focused)
			{
				time_focused = 2;
				t = focus_key[0] = (key - KS_NUM0);
				setup->Workout_time_1000 = (t * 60) / 1000;
				setup->Workout_time = (t * 60) % 1000;
				if(0 == focus_key[0])
					time_focused = 1;
			}
			else if(2 == time_focused)
			{
				time_focused = 3;
				focus_key[1] = (key - KS_NUM0);
				t = 10 * focus_key[0] + focus_key[1];
				setup->Workout_time_1000 = (t * 60) / 1000;
				setup->Workout_time = (t * 60) % 1000;
				if(t > 99 || t < 5)
					time_focused = 1;
			}
		}

		t = (setup->Workout_time_1000 * 1000 + setup->Workout_time) / 60;
		if(t > 99 || t < 5)			focus_ok = 0;
		else						focus_ok = 1;

////////////////
		//if (focus_ok && target == WP_PACE_INTERVALS_A)
		//{
		//}
		if (focus_ok && (target == WP_SINGLE_HILL || target == WP_GLUTE_BUSTER || target == WP_RANDOM_HILLS || target == WP_PACE_RAMP_A))
		{
			setup->Segments = 20;
			for (i = 0; i < setup->Segments; i++)
				setup->Segments_time[i] = (setup->Workout_time_1000 * 1000 + setup->Workout_time) / 20;
		}
		if (focus_ok && target == WP_MANUAL_B)
		{
			manual_distance_flag = 0;
			//setup->Workout_distance = 159;
			//setup->distance = 159.00F;
			//setup->Segments = (setup->Workout_time_1000 * 1 + setup->Workout_time) / 60;
		}
		if (focus_ok && target == WP_CALORIE_GOAL)
		{
			static unsigned short watt_table[] = { 39, 41,  44,  48,  51,  55,  59,  65,  71,  78,  88,  96,  106, 116,  133,  150,  170,  180, 204, 228, 252, 265, 273, 278, 280, 280, 280, 280, 280, 280, 280 };
			unsigned short Watt_workload = 0;
			unsigned short minutes = 0;

			minutes = (1000 * setup->Workout_time_1000 + setup->Workout_time) / 60;
			Watt_workload = ((setup->Calorie_goal / minutes) * 60 - 120) / 9;

			for (i = 0; i < (int)(sizeof(watt_table) / sizeof(unsigned short)) - 1; i++)
			{
				if (Watt_workload < watt_table[i + 1] && Watt_workload >= watt_table[i])
					break;
			}
			if((int)(sizeof(watt_table) / sizeof(unsigned short)) - 1 == i)
				setup->Workload_level = 1;
			else
			{
				setup->Workload_level = i + 1;
				if (setup->Workload_level > 30)
					setup->Workload_level = 30;
			}
			setup->Segments = (setup->Workout_time_1000 * 1 + setup->Workout_time) / 60;
			if (setup->Segments > 30)				setup->Segments = 30;
			if (setup->Segments == 0)				setup->Segments = 20;
			for (i = 0; i < 30; i++)
			{
				if (i < setup->Segments)			setup->Segments_time[i] = 60;
				else								setup->Segments_time[i] = 0;
			}
		}

printf("(%s %d) WM_DBG(%d %d)\n", __FILE__, __LINE__, 1000 * setup->Workout_time_1000 + setup->Workout_time, t);
		return 1;
	}


///////////////////////////////////////////////////////////
	// 固定 BPM
	if (
		(source != target && target == WP_TARGET_HRC_B)			// no fix, fix bpm
		|| (source != target && target == WP_INTERVAL_HRC_B)
	)
	{
		age_focused = 0; weight_focused = 0; workload_focused = 0; distance_focused = 0; time_focused = 0; calorie_focused = 0; gender_focused = 0; pace_focused = 0; segments_focused = 0; hr_focused = 0; scroll_idx = 0;
		hr_focused = 1;
		setup->Work_heart_rate = 100 * setup->Target_heart_rate / (220 - setup->Age);
		if (100 == setup->Work_heart_rate)
			setup->Work_heart_rate = 99;
printf("(%s %d) WM_DBG(%d %d)\n", __FILE__, __LINE__, setup->Target_heart_rate, setup->Work_heart_rate);
		return 1;
	}

	// 固定 %
	if (
		(source != target && target == WP_WL_HRC_B)					// fixed percent
		|| (source != target && target == WP_WL_B)
		|| (source != target && target == WP_AEROBIC_HRC_B)
		|| (source != target && target == WP_FITNESS_B)
	)
	{

		age_focused = 0; weight_focused = 0; workload_focused = 0; distance_focused = 0; time_focused = 0; calorie_focused = 0; gender_focused = 0; pace_focused = 0; segments_focused = 0; hr_focused = 0; scroll_idx = 0;
		hr_focused = 1;
		if (target == WP_WL_HRC_B || target == WP_WL_B)	update->Target_heart_rate = setup->Target_heart_rate = (220 - setup->Age)  * 65 / 100;
		if (target == WP_AEROBIC_HRC_B)					update->Target_heart_rate = setup->Target_heart_rate = (220 - setup->Age)  * 80 / 100;
		if (target == WP_FITNESS_B)						update->Target_heart_rate = setup->Target_heart_rate = (220 - setup->Age)  * 85 / 100;
printf("(%s %d) WM_DBG(%d %d)\n", __FILE__, __LINE__, setup->Target_heart_rate, setup->Work_heart_rate);
		return 1;
	}

	// 固定 BPM
	if (
		(
			(source == WP_TARGET_HRC_B && target == WP_TARGET_HRC_B)
			|| (source == WP_INTERVAL_HRC_B && target == WP_INTERVAL_HRC_B)
		)
		&& key == KS_LEFT_TOP
	)
	{
		age_focused = 0; weight_focused = 0; workload_focused = 0; distance_focused = 0; time_focused = 0; calorie_focused = 0; gender_focused = 0; pace_focused = 0; segments_focused = 0; hr_focused = 0; scroll_idx = 0;
		hr_focused = 1;
		setup->Work_heart_rate = 100 * setup->Target_heart_rate / (220 - setup->Age);
		if (100 == setup->Work_heart_rate)
			setup->Work_heart_rate = 99;
printf("(%s %d) WM_DBG()\n", __FILE__, __LINE__);
		return 1;
	}

	// 固定 %
	if (
		(
			(source == WP_WL_HRC_B && target == WP_WL_HRC_B)
			|| (source == WP_WL_B && target == WP_WL_B)
			|| (source == WP_AEROBIC_HRC_B && target == WP_AEROBIC_HRC_B)
			|| (source == WP_FITNESS_B && target == WP_FITNESS_B)
		)
		&& key == KS_LEFT_TOP
	)
	{
		age_focused = 0; weight_focused = 0; workload_focused = 0; distance_focused = 0; time_focused = 0; calorie_focused = 0; gender_focused = 0; pace_focused = 0; segments_focused = 0; hr_focused = 0; scroll_idx = 0;
		hr_focused = 1;
		if (target == WP_WL_HRC_B || target == WP_WL_B)	update->Target_heart_rate = setup->Target_heart_rate = (220 - setup->Age)  * 65 / 100;
		if (target == WP_AEROBIC_HRC_B)					update->Target_heart_rate = setup->Target_heart_rate = (220 - setup->Age)  * 80 / 100;
		if (target == WP_FITNESS_B)						update->Target_heart_rate = setup->Target_heart_rate = (220 - setup->Age)  * 85 / 100;
printf("(%s %d) WM_DBG()\n", __FILE__, __LINE__);
		return 1;
	}

	if (1 <= hr_focused && 
		(
			(source == WP_TARGET_HRC_B && target == WP_TARGET_HRC_B)
			|| (source == WP_INTERVAL_HRC_B && target == WP_INTERVAL_HRC_B)
		)
	)
	{
		unsigned short top = (220 - setup->Age - 1);//(220 - setup->Age);//freeman

		if(key == KS_DELETE)
		{
			update->Target_heart_rate = setup->Target_heart_rate /= 10;
			if (hr_focused >= 4)									hr_focused = 3;
			else if (hr_focused == 3)								hr_focused = 2;
			else if (hr_focused == 2)								hr_focused = 1;
			else													hr_focused = 1;
		}
		if(key == KS_WORKLOAD_UP)
		{
			if (top > 200)
				top = 200;

			if(setup->Target_heart_rate >= top)
				update->Target_heart_rate = setup->Target_heart_rate = top;
			else if(setup->Target_heart_rate >= 70 && setup->Target_heart_rate < top)
				update->Target_heart_rate = setup->Target_heart_rate++;
			else	
				update->Target_heart_rate = setup->Target_heart_rate = 70;

			age_focused = 0; weight_focused = 0; workload_focused = 0; distance_focused = 0; time_focused = 0; calorie_focused = 0; gender_focused = 0; pace_focused = 0; segments_focused = 0; hr_focused = 0; scroll_idx = 0;
			hr_focused = 1;
		}
		if(key == KS_WORKLOAD_DOWN)
		{
			if (top > 200)
				top = 200;

			if(setup->Target_heart_rate <= 70)
				update->Target_heart_rate = setup->Target_heart_rate = 70;
			else if(setup->Target_heart_rate > 70 && setup->Target_heart_rate <= top)
				update->Target_heart_rate = setup->Target_heart_rate--;
			else	
				update->Target_heart_rate = setup->Target_heart_rate = top;

			age_focused = 0; weight_focused = 0; workload_focused = 0; distance_focused = 0; time_focused = 0; calorie_focused = 0; gender_focused = 0; pace_focused = 0; segments_focused = 0; hr_focused = 0; scroll_idx = 0;
			hr_focused = 1;
		}
		if(key == KS_NUM0 || key == KS_NUM1 || key == KS_NUM2 
			|| key == KS_NUM3 || key == KS_NUM4 || key == KS_NUM5 
				|| key == KS_NUM6 || key == KS_NUM7 || key == KS_NUM8 || key == KS_NUM9)
		{
			if(1 == hr_focused)
			{
				hr_focused = 2;
				focus_key[0] = (key - KS_NUM0);
				update->Target_heart_rate = setup->Target_heart_rate = focus_key[0];
				if(0 == setup->Target_heart_rate)
					hr_focused = 1;
			}
			else if(2 == hr_focused)
			{
				hr_focused = 3;
				focus_key[1] = (key - KS_NUM0);
				update->Target_heart_rate = setup->Target_heart_rate = 10 * focus_key[0] + focus_key[1];
				//if(setup->Target_heart_rate > 200 || setup->Target_heart_rate < 70)
				if(0 == setup->Target_heart_rate)
					hr_focused = 1;
			}
			else if(3 == hr_focused)
			{
				hr_focused = 4;
				focus_key[2] = (key - KS_NUM0);
				update->Target_heart_rate = setup->Target_heart_rate = 100 * focus_key[0] + 10 * focus_key[1] + focus_key[2];
				if(setup->Target_heart_rate > top || setup->Target_heart_rate < 70)
					hr_focused = 1;
			}
		}
		if (setup->Target_heart_rate > top || setup->Target_heart_rate < 70)		focus_ok = 0;
		else																focus_ok = 1;
		if(setup->Age >= 220)
			setup->Age = 35;
		setup->Work_heart_rate = 100 * setup->Target_heart_rate / (220 - setup->Age);
printf("(%s %d) WM_DBG(%d)\n", __FILE__, __LINE__, setup->Target_heart_rate);//freeman note
		return 1;
	}


///////////////////////////////////////////////////////////
printf("(%s %d) WM_DBG(NULL)\n", __FILE__, __LINE__);
		return 1;
	return 0;
}


/*
void CFpcApp::InitWorkoutPhase02_Glute(unsigned char segment_time_min)
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
		segment_time_min = 1;
	rt->total_segment = rt->total_workout_time /(segment_time_min * 60);
	rt->workLoad_TableUsed = rt->total_segment;

	if(rt->workLoad_TableUsed > MAX_SEGMENTS)
		rt->workLoad_TableUsed = MAX_SEGMENTS;

	if(setup->Workload_level == 255)
		setup->Workload_level=1;
	rt->workLoad.current_load_level = setup->Workload_level;

	rt->GUI_Bar_window_index = 0;	
	rt->GUI_Bar_window_Left = 0;
	rt->GUI_Bar_window_Right = rt->GUI_Bar_window_Left + GUI_window_size-1;

	if(setup->Workload_level == 0){
		setup->Workload_level=1;
		update->Workload_level=1;
	}	
	//workload table
	rt->WLClass20_LoadTable = NULL;
	for(i=0; i < rt->workLoad_TableUsed ; i++){
		rt->workLoad_Table[i] = setup->Workload_level;
		rt->segmentTime_Table[i] = segment_time_min*60;
		rt->workPace_Table[i] = 0;
	}
	rt->segmentTime_Table[i] = 120;
	rt->workLoad_Table[i] = 1;
	rt->workPace_Table[i] = 0;
	Update_GUI_bar();
}

*/

//InitWorkoutPhase02_pcc(unsigned char segment_count, struct WLClass20_Load *WLClass20_LoadTable, struct WLClass20_Pace *WLClass20_PaceTable)

void CFpcApp::InitWorkoutPhase02_Leg(unsigned char segment_count)//, struct WLClass20_Load *WLClass20_LoadTable, struct WLClass20_Pace *WLClass20_PaceTable)
{
	unsigned char i;

	rt->segment_index = 0;
	exception->cmd.pause = 0; 
	rt->no_heart_rate_detect = 1000;
	rt->tick_1sec_per100ms_reload = 10;
	rt->tick_1sec_per100ms = rt->tick_1sec_per100ms_reload;
	rt->tick_30sec_reload = FP_TICKS_PER_SECOND * 30;
	rt->tick_30sec = rt->tick_30sec_reload;

	if(setup->Segments == 255)
		setup->Segments = segment_count;
	if (0 == setup->Segments)
		setup->Segments = 1;

	if(setup->Workload_level == 255)
		setup->Workload_level = 1;

	if(setup->Pace_level == 255 || setup->Pace_level == 0)
		setup->Pace_level = 1;
	update->Pace_level = rt->Target_Pace_Level = setup->Pace_level;
	exception->cmd.pace_up = 1;

	rt->workLoad.current_load_level = setup->Workload_level;

	rt->workLoad_TableUsed = setup->Segments;
	rt->total_segment = rt->workLoad_TableUsed;	
	rt->segment_time = rt->total_workout_time / rt->workLoad_TableUsed;


	//rt->pace_adj_mode = INDEXED_TARGET_PACE_ADJ_WLClass20;
	//rt->watt_calc_mod = CALC_BY_INDEXED_LOAD_LEVEL;
	//rt->load_adj_mode = INDEXED_TARGET_LOAD_ADJ_WLClass20;

	for(i = 0; i < rt->workLoad_TableUsed;i++)
	{
		rt->segmentTime_Table[i] = rt->segment_time;
		rt->workLoad_Table[i] = rt->workLoad_Table_cruise[i] = setup->Workload_level;
		rt->workPace_Table[i] = legShapePaceTable[setup->Pace_level - 1].work_pace[i];
	}


	//UpdateIndexedPaceTable_WLCLASS20
	rt->pace_adj_mode = INDEXED_TARGET_PACE_ADJ_WLClass20;
	rt->WLClass20_PaceTable = legShapePaceTable;

	rt->segmentTime_Table[i] = 60;
	rt->workLoad_Table[i] = 1;
	rt->workPace_Table[i] = 0;

	rt->GUI_Bar_window_index = 0;	
	rt->GUI_Bar_window_Left = 0;
	rt->GUI_Bar_window_Right = rt->GUI_Bar_window_Left + GUI_window_size - 1;

#if 0
	Init_GUI_bar_BiPace(struct BiPace * paceTable)
#endif

	Update_GUI_bar();
}


void CFpcApp::Random_Hills(void)
{
	int i;

	InitWorkoutPhase01(20);
	srand (time(NULL));
	for (i = 0; i < 20; i++)		randomWattTable[0].work_load[i] = 1 + (rand() % 8);
	for (i = 0; i < 20; i++)		randomWattTable[1].work_load[i] = randomWattTable[0].work_load[i] + 1;
	for (i = 0; i < 20; i++)		randomWattTable[2].work_load[i] = randomWattTable[0].work_load[i] + 2;
	for (i = 0; i < 20; i++)		randomWattTable[3].work_load[i] = randomWattTable[0].work_load[i] + 3;
	for (i = 0; i < 20; i++)		randomWattTable[4].work_load[i] = randomWattTable[0].work_load[i] + 4;
	for (i = 0; i < 20; i++)		randomWattTable[5].work_load[i] = randomWattTable[0].work_load[i] + 5;
	for (i = 0; i < 20; i++)		randomWattTable[6].work_load[i] = randomWattTable[0].work_load[i] + 6;
	for (i = 0; i < 20; i++)		randomWattTable[7].work_load[i] = randomWattTable[0].work_load[i] + 7;
	for (i = 0; i < 20; i++)		randomWattTable[8].work_load[i] = randomWattTable[0].work_load[i] + 8;
	for (i = 0; i < 20; i++)		randomWattTable[9].work_load[i] = randomWattTable[0].work_load[i] + 9;
	for (i = 0; i < 20; i++)		randomWattTable[10].work_load[i] = randomWattTable[0].work_load[i] + 10;
	for (i = 0; i < 20; i++)		randomWattTable[11].work_load[i] = randomWattTable[0].work_load[i] + 11;
	for (i = 0; i < 20; i++)		randomWattTable[12].work_load[i] = randomWattTable[0].work_load[i] + 12;
	for (i = 0; i < 20; i++)		randomWattTable[13].work_load[i] = randomWattTable[0].work_load[i] + 13;
	for (i = 0; i < 20; i++)		randomWattTable[14].work_load[i] = randomWattTable[0].work_load[i] + 14;
	for (i = 0; i < 20; i++)		randomWattTable[15].work_load[i] = randomWattTable[0].work_load[i] + 15;
	for (i = 0; i < 20; i++)		randomWattTable[16].work_load[i] = randomWattTable[0].work_load[i] + 16;
	for (i = 0; i < 20; i++)		randomWattTable[17].work_load[i] = randomWattTable[0].work_load[i] + 17;
	for (i = 0; i < 20; i++)		randomWattTable[18].work_load[i] = randomWattTable[0].work_load[i] + 18;
	for (i = 0; i < 20; i++)		randomWattTable[19].work_load[i] = randomWattTable[0].work_load[i] + 19;

	InitWorkoutPhase02_pobh(setup->Segments,  randomWattTable);//(20 segment, no timing table, indexed workload table)


	if (0 == update->Segment_time)
		update->Segment_time = 60;

	rt->exception_result = EXCEPTION_CONTINUE;	
	//while(rt->total_workout_time_tick > 0 && rt->exception_result == EXCEPTION_CONTINUE)
	while(rt->exception_result == EXCEPTION_CONTINUE)
	{
		//for each segment
		update->Segment_time = rt->segmentTime_Table[rt->segment_index];

		rt->segment_time_tick = FP_TICKS_PER_SECOND * update->Segment_time;

		if(exception->cmd.hr_cruise == 0)
		{
			rt->workLoad.current_load_level = rt->workLoad_Table[rt->segment_index];

			//can only be one of both
			//update->Workload_level = rt->workLoad.current_load_level;
			//update->Workload_level = setup->Workload_level;
			AdjustMachineWorkLoad(INDEXED_TARGET_LOAD_ADJ_WLClass20, 0);
		}
		
		while(rt->segment_time_tick > 0 && rt->exception_result == EXCEPTION_CONTINUE)
		{
			//get realtime heartrate
			//rt->current_heart_rate = lcb->hr->DisplayHeartRate;
			rt->current_heart_rate = update->Heart_rate;

			//handling the Exceptions		
			rt->exception_result = ExceptionHandler(PERFORMANCE_ONE_BIG_HILL);

			//end of handling the Exceptions
			if(rt->exception_result == EXCEPTION_COOLDOWN)
				break;
			if(rt->exception_result == EXCEPTION_BREAK)
				break;
			//end of handling the Exceptions
			
			if(exception->cmd.pause == 0)
			{
				/////20130409/////////
				//Workout Time Update
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

				/////////////////////
				//DataScreen Time / Bar Update
				//timer elapsed 1 sec after 100 tick count;
				if(rt->tick_1sec_per100ms >0)
					rt->tick_1sec_per100ms--;	
				if(rt->tick_1sec_per100ms == 0)
				{
					rt->tick_1sec_per100ms = rt->tick_1sec_per100ms_reload;
					Update_Workout_Time_Elapsed();
					Update_Workout_Time_Remaining();
					Update_GUI_bar();
				}
				/////20130409/////////
				if(exception->cmd.hr_cruise == 1)
					HR_Cruise();
			}

			WaitForProgramTick();
		}
	}

	EndofProgram(PERFORMANCE_ONE_BIG_HILL);	
}


void CFpcApp::Weight_Loss_Leg(void)
{

#if 1
	InitWorkoutPhase01(20);
	InitWorkoutPhase02_Leg(20);
#endif

	if (0 == update->Segment_time)		update->Segment_time = 60;
	rt->exception_result = EXCEPTION_CONTINUE;	

	//while(rt->total_workout_time_tick > 0 && rt->exception_result == EXCEPTION_CONTINUE)
	while(rt->exception_result == EXCEPTION_CONTINUE)
	{
		//for each segment
		update->Segment_time = rt->segmentTime_Table[rt->segment_index];

		rt->segment_time_tick = FP_TICKS_PER_SECOND * update->Segment_time;

		if(exception->cmd.hr_cruise == 0)
		{
			rt->watt_calc_mod = CALC_BY_INDEXED_LOAD_LEVEL;

			rt->workLoad.current_load_level = rt->workLoad_Table[rt->segment_index];

			//can only be one of both
			//update->Workload_level = rt->workLoad.current_load_level;
			//update->Workload_level = setup->Workload_level;
			AdjustMachineWorkLoad(INDEXED_TARGET_PACE_ADJ_WLClass20, 0);
		}
		
		while(rt->segment_time_tick > 0 && rt->exception_result == EXCEPTION_CONTINUE)
		{
			//get realtime heartrate
			//rt->current_heart_rate = lcb->hr->DisplayHeartRate;
			rt->current_heart_rate = update->Heart_rate;

			//handling the Exceptions		
			rt->exception_result = ExceptionHandler(PERFORMANCE_ONE_BIG_HILL);

			//end of handling the Exceptions
			if(rt->exception_result == EXCEPTION_COOLDOWN)
				break;
			if(rt->exception_result == EXCEPTION_BREAK)
				break;
			//end of handling the Exceptions
			
			if(exception->cmd.pause == 0)
			{
				/////20130409/////////
				//Workout Time Update
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

				/////////////////////
				//DataScreen Time / Bar Update
				//timer elapsed 1 sec after 100 tick count;
				if(rt->tick_1sec_per100ms >0)
					rt->tick_1sec_per100ms--;	
				if(rt->tick_1sec_per100ms == 0)
				{
					rt->tick_1sec_per100ms = rt->tick_1sec_per100ms_reload;
					Update_Workout_Time_Elapsed();
					Update_Workout_Time_Remaining();

					UpdateCoolDownPace(WEIGHT_LOSS_LEG_SHAPTER);

					Update_GUI_bar();
				}
				/////20130409/////////
				if(exception->cmd.hr_cruise == 1)
				{
					rt->watt_calc_mod = CALC_BY_CURRENT_LOAD_LEVEL;
					HR_Cruise();
				}
			}

			WaitForProgramTick();
		}
	}

	EndofProgram(WEIGHT_LOSS_LEG_SHAPTER);
}


void CFpcApp::Weight_Loss_Glute(void)
{
	InitWorkoutPhase01(20);
	InitWorkoutPhase02_pobh(setup->Segments,  gluteWattTable);//(20 segment, no timing table, indexed workload table)

	if (0 == update->Segment_time)		update->Segment_time = 60;

	rt->exception_result = EXCEPTION_CONTINUE;	

	//while(rt->total_workout_time_tick > 0 && rt->exception_result == EXCEPTION_CONTINUE)
	while(rt->exception_result == EXCEPTION_CONTINUE)
	{
		//for each segment
		update->Segment_time = rt->segmentTime_Table[rt->segment_index];

		rt->segment_time_tick = FP_TICKS_PER_SECOND * update->Segment_time;

		if(exception->cmd.hr_cruise == 0)
		{
			rt->watt_calc_mod = CALC_BY_INDEXED_LOAD_LEVEL;

			rt->workLoad.current_load_level = rt->workLoad_Table[rt->segment_index];

			//can only be one of both
			//update->Workload_level = rt->workLoad.current_load_level;
			//update->Workload_level = setup->Workload_level;
			AdjustMachineWorkLoad(INDEXED_TARGET_LOAD_ADJ_WLClass20, 0);
		}
		
		while(rt->segment_time_tick > 0 && rt->exception_result == EXCEPTION_CONTINUE)
		{
			//get realtime heartrate
			//rt->current_heart_rate = lcb->hr->DisplayHeartRate;
			rt->current_heart_rate = update->Heart_rate;

			//handling the Exceptions
			rt->exception_result = ExceptionHandler(PERFORMANCE_ONE_BIG_HILL);

			//end of handling the Exceptions
			if(rt->exception_result == EXCEPTION_COOLDOWN)
				break;
			if(rt->exception_result == EXCEPTION_BREAK)
				break;
			//end of handling the Exceptions
			
			if(exception->cmd.pause == 0)
			{
				/////20130409/////////
				//Workout Time Update
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

				/////////////////////
				//DataScreen Time / Bar Update
				//timer elapsed 1 sec after 100 tick count;
				if(rt->tick_1sec_per100ms >0)
					rt->tick_1sec_per100ms--;	
				if(rt->tick_1sec_per100ms == 0)
				{
					rt->tick_1sec_per100ms = 
						rt->tick_1sec_per100ms_reload;
					Update_Workout_Time_Elapsed();
					Update_Workout_Time_Remaining();
					Update_GUI_bar();
				}
				/////20130409/////////
				if(exception->cmd.hr_cruise == 1)
				{
					rt->watt_calc_mod = CALC_BY_CURRENT_LOAD_LEVEL;
					HR_Cruise();
				}
			}

			WaitForProgramTick();
		}
	}

	EndofProgram(PERFORMANCE_ONE_BIG_HILL);	
}
