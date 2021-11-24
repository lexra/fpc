// Sd55Data.h : 
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SD55DATA_H__62B243F7_35CX_485C_A548_14BA74273541__INCLUDED_)
#define AFX_SD55DATA_H__62B243F7_35CX_485C_A548_14BA74273541__INCLUDED_


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


//////////////////////////////////////////////////////////////////////
// struct
//////////////////////////////////////////////////////////////////////

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


#define SD55_CMD_ID											400
#define SD55_CMD_INIT_TIMEOUT_ID							401
#define SD55_CMD_RESET_CONTORLER_TIMEOUT_ID				402
#define SD55_CMD_CC_INIT_ID								403
#define SD55_CMD_DEFAULT_STRIDE_ID						404
#define SD55_CMD_RD_SPU_ID								405
#define SD55_CMD_SET_STRIDE_ID							406
#define SD55_CMD_SET_AUTO_STRIDE_ID						407

#define SOC_TO_MCU_SAY_HELLO						408



#define Mile_KM_ratio		1.60F	//not 1.609344

// //心率采樣的周期范圍
#define MIN_CYCLE           250                 //最小周期ms,得到最大心率=60000/250=240BPM
#define MAX_CYCLE           1600                //是大周期ms,得到最小心率=60000/1600=37.5BPM

// //無線心率的脈衝寬度范圍，因要兼容CODED格式的心率所以最大值要加大
// //Polar Heart Rate Pulse output
//15.6ms Pulse Width 1.32 sec ~~~ 488ms
#define MIN_PULSE_WIDTH_TEL_CODING   30
#define MAX_PULSE_WIDTH_TEL_CODING   100
#define MIN_PULSE_WIDTH_TEL_UNCODING 8
#define MAX_PULSE_WIDTH_TEL_UNCODING 37
//手握心率的脈衝寬度范圍
#define MIN_PULSE_WIDTH_HGP 20
#define MAX_PULSE_WIDTH_HGP 400

//連續檢到有效的心率次數
#define MAX_VALID_TIMES    1
//連續檢到無效的心率次數
#define MAX_LOSE_TIMES     12

//心率顯示的最值，注意要在心率采樣周期的范圍內
#define MAX_HEARTRATE       220
#define MIN_HEARTRATE       40

//#define BIT0	0x01
//#define BIT1	0x02
//#define BIT2	0x04

struct	HR_ControlState{
	unsigned char hr_ready				:1;//0
	unsigned char TEL_in_use				:1;//1
	unsigned char reserved2				:1;//2
	unsigned char ALL_BUFFER_SAME_FLAG_HGP		:1;//3       //當所有的心率Buffer相同時為true
	unsigned char ALL_BUFFER_SAME_FLAG_TEL		:1;//4       //當所有的心率Buffer相同時為true
	unsigned char CODING_FLAG_TEL			:1;//5       //無線心率的格式為CODED
	unsigned char HR_SAMPLING_VALID_FLAG_HGP		:1;//6       //采樣到的手握心率周期有效標志
	unsigned char HR_SAMPLING_VALID_FLAG_TEL		:1;//7       //采樣到的無線心率周期有效標志
};

struct HRC_Intervals_exercise{
	unsigned char work_done;
	unsigned char work_type;
	unsigned short time;
	unsigned char level;
};


///////////////////////
///////////////////////

#define MIN_AS		50.00F
#define MAX_AS		200.00F
#define DELTA_AS	((MAX_AS - MIN_AS) / 29.00F)


#define LEVEL_TABLE_LEN	30
#define STRIDE_TABLE_LEN	23
#define Rx0_Buffer_Size 64
#define Tx0_Buffer_Size 64

#define MAX_STRIDE_LENGTH				(27*2)		//JESSE 20120717			
#define MIN_STRIDE_LENGTH				(16*2)   	//JESSE 20120717

// jason note
#define DEFAULT_STRIDE_LENGTH			(21*2)  		//JESSE 20120717
//#define DEFAULT_STRIDE_LENGTH			18*2  		//JESSE 20120717


//阻力等級的范圍
#define MAX_RESISTANCE_LEVEL    30	//25 by simon@2013/01/04
#define MIN_RESISTANCE_LEVEL    1

#define RESET_ERR_STATE		0x0001	// (1<<0)
#define SET_RESIST_POS		0x0002	// (1<<1)
#define SET_STRIDE_POS		0x0004	// (1<<2)
#define SET_INCLINE_POS		0x0008	// (1<<3)
#define SET_MOTOR_CTRL		0x0010	// (1<<4)
#define SET_INIT_STEP		0x0020	// (1<<5)
#define SET_READ_STEP		0x0040	// (1<<6)
#define SET_STRIDE_FIRST_POS	0x0080	// (1<<7)
#define SET_RESET_CONTORLER		0x0100	// (1<<8)

#define SD55_FRAME_HEADER		0xF5
#define SD55_FRAME_YES		0x5A
#define SD55_FRAME_NOT		0xA5
#define SD55_FRAME_NOTHING		0x55

#define MAX_RPM   120
#define MIN_RPM   30






#define FRAME_HEADER  0xFE





typedef enum
{
	ERR_MSG_STRIDE1_LIMIT_LOW_SW,
	ERR_MSG_STRIDE1_LIMIT_HIGH_SW,
	ERR_MSG_STRIDE2_LIMIT_LOW_SW,
	ERR_MSG_STRIDE2_LIMIT_HIGH_SW,
	ERR_MSG_RESIST_MOTOR_ERR,
	ERR_MSG_STRIDE_MOTOR1_ERR,
	ERR_MSG_STRIDE_MOTOR2_ERR,
	ERR_MSG_INCLINE_MOTOR_ERR,
	ERR_MSG_SERIAL_ERROR
}SD55_ERROR;




///////////////////////
///////////////////////

struct UART_State
{
	unsigned char txNeedResend		:1;
	unsigned char ComError		:1;
	unsigned char txInDemand		:1;
	unsigned char txInUse		:1;
	unsigned char Spu_Hi_Flag		:1;
	unsigned char Spu_Hi_Lo_Flag	:1;
	unsigned char New_SPU_Pulse		:1;
	unsigned char Key_Changed		:1;
};

struct Communication_control
{

#if defined(__cplusplus) || defined(__CPLUSPLUS__)
public:
	Communication_control();
#endif

 	//unsigned long hUart;
 	unsigned long ulBitTime;
	unsigned long UARTEvent;
 	unsigned short Cmd;
 	unsigned short ErrorNo;
 	unsigned char txLen;
 	unsigned char rxCount,txCount;
 	unsigned char rxBuffer[Rx0_Buffer_Size];
 	unsigned char txBuffer[Tx0_Buffer_Size];
 	unsigned char txRepeat, txRollStep;
 	unsigned short comInterval;

	struct UART_State state;
};

struct SD55_data
{

#if defined(__cplusplus) || defined(__CPLUSPLUS__)
public:
	SD55_data();
#endif

 	unsigned short sent_times;
 	unsigned short received_times;
 	unsigned long Step;
 	unsigned char State;
 	unsigned char ResistanceLevel;
 	unsigned char StrideLength;
 	unsigned char ResistancePosition;
 	unsigned char StridePosition;
	//spu
 	unsigned char Clear_Rpm_Delay;
 	unsigned char	Buffer_Index;                  
 	unsigned short Rpm;
 	unsigned short Rpm_Buffer[101];
 	unsigned long Drive_SPU_Interval;
};



//////////////////////////////////////////////////////////////////////
// routine
//////////////////////////////////////////////////////////////////////

//int SendCmd(int fd, int cmd, int len, const unsigned char *value);


//////////////////////////////////////////////////////////////////////
// class
//////////////////////////////////////////////////////////////////////

class HeartRate
{
public:
	HeartRate();
	virtual ~HeartRate();

public:
	int i2c_fd;

	//Control Flag	
	struct	HR_ControlState state;	//心率采樣是否有效的狀態
	//display	
	unsigned char DisplayHeartRate;
	unsigned char TEL_PIN;
	unsigned char HGP_PIN;
	//Control
	unsigned char Heart_Rate_up_Flag;
	unsigned char Heart_Rate_down_Flag;
	unsigned char HR_Pause_Delay;
	//Wire	
	unsigned char HeartRate_TEL;	//系統:當前無線心率的值
	unsigned char HeartRate_HGP;	//系統:當前手握心率的值
	
	unsigned char HR_Sampling_TEL;	//無線心跳的采樣值，單位BPM
	unsigned char HR_Sampling_HGP;	//手握心跳的采樣值，單位BPM
	unsigned char HR_Buffer_TEL[16];	//用于計算無線心率的緩衝區
	unsigned char HR_Buffer_HGP[16];	//用于計算手握心率的緩衝區

	unsigned char Times_Valid_TEL;	//有效的次數
	unsigned char Times_Valid_HGP;	//有效的次數
	unsigned char Times_Lose_TEL;	//無效的次數
	unsigned char Times_Lose_HGP;	//無效的次數	

	unsigned char Lose_Temp_TEL;	//心率為零計數
	unsigned char Lose_Temp_HGP;	//心率為零計數

	unsigned char Count_ticks_per_100ms;	//本段代碼中用到的100ms單位的定時器
	unsigned char Count_ticks_reload;//本段代碼中用到的100ms單位的定時器 倒數量

	unsigned short Sampling_Timer_HGP;        //手握心率的采樣計時
	unsigned char Pin_Buffer_HGP;             //按位左移的PIN腳狀態緩衝
	unsigned char Pulse_Width_HGP;            //手握心率的有效脈寬

	//Wireless
	unsigned short Sampling_Timer_TEL;//無線心率的采樣計時
	unsigned char Pin_Buffer_TEL;//按位左移的PIN腳狀態緩衝
	unsigned char Pulse_Width_TEL;//無線心率的有效脈寬	

public:
	void HeartRate_Process(void);
	void HeartRate_Sampling_TEL(void);	//this function will be ticked from the FPC Shceduler every 1ms.
	void HeartRate_Sampling_HGP(void);	//this function will be ticked from the FPC Shceduler every 1ms.
	void Init_HeartRate_Data(int fd);
	unsigned char HeartRate_TEL_hardware_read(void);
	unsigned char HeartRate_HGP_hardware_read(void);
	unsigned char PCA9555_hardware_read(void);
};


//////////////////////////////////////////////////////////////////////
// class
//////////////////////////////////////////////////////////////////////

//int SendCmd(int fd, int cmd, int len, unsigned char value, ...);


class SD55_Tables
{
public:
	SD55_Tables();

	//properties
	unsigned char *SD55_Resist_Level_Table;
	unsigned char *Stride_Len_BitMap_Table;

	virtual unsigned char Get_ADC_Target(unsigned char level);
	virtual unsigned char Get_Stride_Target(unsigned char level);
};


//////////////////////////////////////////////////////////////////////
// class
//////////////////////////////////////////////////////////////////////

class CSd55Data :  public CFpcTb, public HeartRate, public SD55_Tables, public SD55_data, public Communication_control
{
public:
	CSd55Data();
	virtual ~CSd55Data();

public:
	struct SD55_data *data;
	struct Communication_control *cc;
	class HeartRate *hr;
	class SD55_Tables *Tables;

public:
	//virtual void CC_Init(void);
	virtual void SetSendCmd(unsigned int flag);
	virtual unsigned char Checksum(void);
	virtual void ResetSendCmd(unsigned int);
	virtual unsigned int CheckSendCmd(unsigned int flag);
	//virtual void Set_Stride_Motor_Position(unsigned char position);
	virtual unsigned char Get_Stride_Motor_Position(void);
	//virtual void Set_WorkLoad_Motor_Position(unsigned char position);
	virtual unsigned short Get_RPM(void);
	virtual unsigned char Get_RunRPM(void);
	virtual void DecInterval(void);
	virtual void Init_SPU(void);
};


#endif // !defined(AFX_SD55DATA_H__62B243F7_35CX_485C_A548_14BA74273541__INCLUDED_)

