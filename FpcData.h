// FpcData.h : 
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FPCDATA_H__62B243F7_35C4_485A_A548_14BA74273541__INCLUDED_)
#define AFX_FPCDATA_H__62B243F7_35C4_485A_A548_14BA74273541__INCLUDED_


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


//////////////////////////////////////////////////////////////////////
// define
//////////////////////////////////////////////////////////////////////

#define SEGMENTS		10


#define FPC_FIRE_START											1
#define FPC_FIRE_STOP											2
#define FPC_FIRE_PAUSE											3
#define FPC_FIRE_RESUME										4
#define FPC_FIRE_COOL_DOWN									5
#define FPC_FIRE_SUMMARY_READY								6
#define FPC_FIRE_READ_SETUP									7


//////////////////////////////////////////////////////////////////////

#define WP_HOME												1

#define WP_CARDIO_360_DEMO									(WP_HOME * 10 + 4)
#define WP_CARDIO_360_DEMO_RUN								(WP_CARDIO_360_DEMO * 10 + 0)
#define WP_CARDIO_360_DEMO_SUMMARY							(WP_CARDIO_360_DEMO * 10 + 1)

#define WP_MANUAL_QUICK_START								(WP_HOME * 10 + 0)
#define WP_MANUAL_QUICK_START_RUN							(WP_MANUAL_QUICK_START * 10 + 0)
#define WP_MANUAL_QUICK_START_SUMMARY						(WP_MANUAL_QUICK_START * 10 + 1)

#define WP_CARDIO_360_QUICK_START							(WP_HOME * 10 + 1)
#define WP_CARDIO_360_QUICK_START_RUN						(WP_CARDIO_360_QUICK_START * 10 + 0)
#define WP_CARDIO_360_QUICK_START_SUMMARY					(WP_CARDIO_360_QUICK_START * 10 + 1)

#define WP_LANG_SELECT											(WP_HOME * 10 + 2)

#define WP_WORKOUT_FINDER										(WP_HOME * 10 + 3)

#define WP_C360													(WP_WORKOUT_FINDER * 10 + 0)
#define WP_WL													(WP_WORKOUT_FINDER * 10 + 1)
#define WP_HRC													(WP_WORKOUT_FINDER * 10 + 2)
#define WP_PERFORMANCE										(WP_WORKOUT_FINDER * 10 + 3)
#define WP_CUSTOM												(WP_WORKOUT_FINDER * 10 + 4)
#define WP_MANUAL_A											(WP_WORKOUT_FINDER * 10 + 5)
#define WP_MANUAL_B											(WP_WORKOUT_FINDER * 10 + 6)
#define WP_MANUAL_RUN											(WP_MANUAL_A * 10 + 0)
#define WP_MANUAL_SUMMARY									(WP_MANUAL_A * 10 + 1)
#define WP_MANUAL_SAVE										(WP_MANUAL_A * 10 + 2)


#define WP_C360_QUICK_START									(WP_C360 * 10 + 0)
#define WP_C360_QUICK_START_RUN								(WP_C360_QUICK_START * 10 + 0)
#define WP_C360_QUICK_START_SUMMARY							(WP_C360_QUICK_START * 10 + 1)

#define WP_C360_ARM_SCULPTOR									(WP_C360 * 10 + 1)
#define WP_C360_ARM_SCULPTOR_RUN								(WP_C360_ARM_SCULPTOR * 10 + 0)
#define WP_C360_ARM_SCULPTOR_SUMMARY						(WP_C360_ARM_SCULPTOR * 10 + 1)
#define WP_C360_ARM_SCULPTOR_SAVE							(WP_C360_ARM_SCULPTOR * 10 + 2)


#define WP_C360_LEG_SHAPER									(WP_C360 * 10 + 2)
#define WP_C360_LEG_SHAPER_RUN								(WP_C360_LEG_SHAPER * 10 + 0)
#define WP_C360_LEG_SHAPER_SUMMARY							(WP_C360_LEG_SHAPER * 10 + 1)
#define WP_C360_LEG_SHAPER_SAVE								(WP_C360_LEG_SHAPER * 10 + 2)


#define WP_C360_CUSTOM_A										(WP_C360 * 10 + 3)
#define WP_C360_CUSTOM_B										(WP_C360 * 10 + 4)
#define WP_C360_CUSTOM_RUN									(WP_C360_CUSTOM_A * 10 + 0)
#define WP_C360_CUSTOM_SUMMARY								(WP_C360_CUSTOM_A * 10 + 1)
#define WP_C360_CUSTOM_SAVE									(WP_C360_CUSTOM_A * 10 + 2)


#define WP_CALORIE_GOAL										(WP_WL * 10 + 0)
#define WP_CALORIE_GOAL_RUN									(WP_CALORIE_GOAL * 10 + 0)
#define WP_CALORIE_GOAL_SUMMARY								(WP_CALORIE_GOAL * 10 + 1)
#define WP_CALORIE_GOAL_SAVE									(WP_CALORIE_GOAL * 10 + 2)

#define WP_GLUTE_BUSTER										(WP_WL * 10 + 1)
#define WP_GLUTE_BUSTER_RUN									(WP_GLUTE_BUSTER * 10 + 0)
#define WP_GLUTE_BUSTER_SUMMARY								(WP_GLUTE_BUSTER * 10 + 1)
#define WP_GLUTE_BUSTER_SAVE									(WP_GLUTE_BUSTER * 10 + 2)

#define WP_LEG_SHAPER_A										(WP_WL * 10 + 2)
#define WP_LEG_SHAPER_B										(WP_WL * 10 + 4)
#define WP_LEG_SHAPER_RUN										(WP_LEG_SHAPER_A * 10 + 0)
#define WP_LEG_SHAPER_SUMMARY								(WP_LEG_SHAPER_A * 10 + 1)
#define WP_LEG_SHAPER_SAVE									(WP_LEG_SHAPER_A * 10 + 2)

#define WP_WL_A												(WP_WL * 10 + 3)
#define WP_WL_B													(WP_WL * 10 + 5)
#define WP_WL_RUN												(WP_WL_A * 10 + 0)
#define WP_WL_SUMMARY											(WP_WL_A * 10 + 1)
#define WP_WL_SAVE												(WP_WL_A * 10 + 2)


#define WP_TARGET_HRC_A										(WP_HRC * 10 + 0)
#define WP_TARGET_HRC_B										(WP_HRC * 10 + 4)
#define WP_TARGET_HRC_RUN										(WP_TARGET_HRC_A * 10 + 0)
#define WP_TARGET_HRC_SUMMARY								(WP_TARGET_HRC_A * 10 + 1)
#define WP_TARGET_HRC_SAVE									(WP_TARGET_HRC_A * 10 + 2)

#define WP_WL_HRC_A											(WP_HRC * 10 + 1)
#define WP_WL_HRC_B											(WP_HRC * 10 + 5)
#define WP_WL_HRC_RUN											(WP_WL_HRC_A * 10 + 0)
#define WP_WL_HRC_SUMMARY									(WP_WL_HRC_A * 10 + 1)
#define WP_WL_HRC_SAVE										(WP_WL_HRC_A * 10 + 2)

#define WP_AEROBIC_HRC_A										(WP_HRC * 10 + 2)
#define WP_AEROBIC_HRC_B										(WP_HRC * 10 + 6)
#define WP_AEROBIC_HRC_RUN									(WP_AEROBIC_HRC_A * 10 + 0)
#define WP_AEROBIC_HRC_SUMMARY								(WP_AEROBIC_HRC_A * 10 + 1)
#define WP_AEROBIC_HRC_SAVE									(WP_AEROBIC_HRC_A * 10 + 2)

#define WP_INTERVAL_HRC_A										(WP_HRC * 10 + 3)
#define WP_INTERVAL_HRC_B										(WP_HRC * 10 + 7)
#define WP_INTERVAL_HRC_RUN									(WP_INTERVAL_HRC_A * 10 + 0)
#define WP_INTERVAL_HRC_SUMMARY								(WP_INTERVAL_HRC_A * 10 + 1)
#define WP_INTERVAL_HRC_SAVE									(WP_INTERVAL_HRC_A * 10 + 2)


#define WP_CARDIO_CHALLENGE									(WP_PERFORMANCE * 10 + 0)
#define WP_CARDIO_CHALLENGE_RUN								(WP_CARDIO_CHALLENGE * 10 + 0)
#define WP_CARDIO_CHALLENGE_SUMMARY							(WP_CARDIO_CHALLENGE * 10 + 1)
#define WP_CARDIO_CHALLENGE_SAVE								(WP_CARDIO_CHALLENGE * 10 + 2)


#define WP_PACE													(WP_PERFORMANCE * 10 + 1)
#define WP_WALK_INTERVALS_A									(WP_PACE * 10 + 0)
#define WP_WALK_INTERVALS_B									(WP_PACE * 10 + 3)
#define WP_WALK_INTERVALS_RUN								(WP_WALK_INTERVALS_A * 10 + 0)
#define WP_WALK_INTERVALS_SUMMARY							(WP_WALK_INTERVALS_A * 10 + 1)
#define WP_WALK_INTERVALS_SAVE								(WP_WALK_INTERVALS_A * 10 + 2)

#define WP_PACE_INTERVALS_A									(WP_PACE * 10 + 1)
#define WP_PACE_INTERVALS_B									(WP_PACE * 10 + 4)
#define WP_PACE_INTERVALS_RUN									(WP_PACE_INTERVALS_A * 10 + 0)
#define WP_PACE_INTERVALS_SUMMARY							(WP_PACE_INTERVALS_A * 10 + 1)
#define WP_PACE_INTERVALS_SAVE								(WP_PACE_INTERVALS_A * 10 + 2)

#define WP_PACE_RAMP_A										(WP_PACE * 10 + 2)
#define WP_PACE_RAMP_B										(WP_PACE * 10 + 5)
#define WP_PACE_RAMP_RUN										(WP_PACE_RAMP_A * 10 + 0)
#define WP_PACE_RAMP_SUMMARY									(WP_PACE_RAMP_A * 10 + 1)
#define WP_PACE_RAMP_SAVE										(WP_PACE_RAMP_A * 10 + 2)


#define WP_HILLS												(WP_PERFORMANCE * 10 + 2)
#define WP_ROLLING_HILLS										(WP_HILLS * 10 + 0)
#define WP_ROLLING_HILLS_RUN									(WP_ROLLING_HILLS * 10 + 0)
#define WP_ROLLING_HILLS_SUMMARY								(WP_ROLLING_HILLS * 10 + 1)
#define WP_ROLLING_HILLS_SAVE									(WP_ROLLING_HILLS * 10 + 2)

#define WP_HILL_INTVALS										(WP_HILLS * 10 + 1)
#define WP_HILL_INTVALS_RUN									(WP_HILL_INTVALS * 10 + 0)
#define WP_HILL_INTVALS_SUMMARY								(WP_HILL_INTVALS * 10 + 1)
#define WP_HILL_INTVALS_SAVE									(WP_HILL_INTVALS * 10 + 2)

#define WP_SINGLE_HILL											(WP_HILLS * 10 + 2)
#define WP_SINGLE_HILL_RUN										(WP_SINGLE_HILL * 10 + 0)
#define WP_SINGLE_HILL_SUMMARY								(WP_SINGLE_HILL * 10 + 1)
#define WP_SINGLE_HILL_SAVE									(WP_SINGLE_HILL * 10 + 2)

#define WP_RANDOM_HILLS										(WP_HILLS * 10 + 3)
#define WP_RANDOM_HILLS_RUN									(WP_RANDOM_HILLS * 10 + 0)
#define WP_RANDOM_HILLS_SUMMARY								(WP_RANDOM_HILLS * 10 + 1)
#define WP_RANDOM_HILLS_SAVE									(WP_RANDOM_HILLS * 10 + 2)


#define WP_DISTANCE_A											(WP_PERFORMANCE * 10 + 3)
#define WP_DISTANCE_B											(WP_PERFORMANCE * 10 + 5)
#define WP_DISTANCE_RUN										(WP_DISTANCE_A * 10 + 0)
#define WP_DISTANCE_SUMMARY									(WP_DISTANCE_A * 10 + 1)
#define WP_DISTANCE_SAVE										(WP_DISTANCE_A * 10 + 2)

#define WP_FITNESS_A											(WP_PERFORMANCE * 10 + 4)
#define WP_FITNESS_B											(WP_PERFORMANCE * 10 + 6)
#define WP_FITNESS_RUN											(WP_FITNESS_A * 10 + 0)
#define WP_FITNESS_SUMMARY									(WP_FITNESS_A * 10 + 1)
#define WP_FITNESS_SAVE										(WP_FITNESS_A * 10 + 2)


#define WP_READ_SAVE_0											(WP_CUSTOM * 10 + 0)
#define WP_READ_SAVE_1											(WP_CUSTOM * 10 + 1)
#define WP_READ_SAVE_2											(WP_CUSTOM * 10 + 2)
#define WP_READ_SAVE_3											(WP_CUSTOM * 10 + 3)

#define WP_CUSTOM_UTRA_A										(WP_CUSTOM * 10 + 4)
#define WP_CUSTOM_UTRA_B										(WP_CUSTOM * 10 + 5)
#define WP_CUSTOM_UTRA_C										(WP_CUSTOM * 10 + 6)
#define WP_CUSTOM_UTRA_RUN									(WP_CUSTOM_UTRA_A * 10 + 0)
#define WP_CUSTOM_UTRA_SUMMARY								(WP_CUSTOM_UTRA_A * 10 + 1)
#define WP_CUSTOM_UTRA_SAVE									(WP_CUSTOM_UTRA_A * 10 + 2)







//////////////////////////////////////////////////////////////////////

#define WM_WORKOUT_SUMMARY									601
#define WM_HOME_SCREEN										101
#define WM_DATA_SCREEN										201
#define WM_LANG_SCREEN										102
#define WM_WORKOUT_FINDER									100


// HOME
#define WM_CARDIO_360											202
#define WM_WEIGHT_LOSS										203
#define WM_HRC_WORKOUTS										204
#define WM_PERFORMANCE										205
#define WM_CUSTOM_WORKOUTS									206
#define WM_MANUAL_WORKOUT									207

// CARDIO 360
#define WM_C360_QUICK_START									301
#define WM_C360_ARM_SCULPTOR									302
#define WM_C360_LEG_SHAPER									303
#define WM_C360_CUSTOM										304

// WEIGHT LOSS
#define WM_CALORIE_GOAL										305
#define WM_GLUTE_BUSTER										306
#define WM_LEG_SHAPER											307
#define WM_WEIGHT_LOSS_HRC									308

// HRC WORKOUTS
#define WM_TARGET_HRC											309
#define WM_WEIGHT_LOSS_HRC_1									310
#define WM_AEROBIC_HRC											311
#define WM_INTERVALS_HRC										312


#define WM_TARGET_HRC_1										331
#define WM_AEROBIC_HRC_1										332
#define WM_INTERVALS_HRC_1									333
#define WM_DISTANCE_WORKOUTS_1								334
#define WM_FITNESS_TEST_1										335

#define WM_WALK_RUN_INTERVALS_1								441
#define WM_PACE_INTERVALS_1									442
#define WM_PACE_RAMP_1										443




// PERFORMANCE
#define WM_CARDIO_CHALLENGE									313
#define WM_PACE_WORKOUTS										314
#define WM_HILL_WORKOUTS										315
#define WM_DISTANCE_WORKOUTS									316
#define WM_FITNESS_TEST										317

// CUSTOM WORKOUTS
#define WM_CUSTOM_ULTRA_WORKOUT								330
#define WM_SAVED_WORKOUTS									318

#define WM_MANUAL_WORKOUT_B									321
#define WM_MANUAL_WORKOUT_SETUP								322


#define WM_MANUAL_QUICK_START								401

#define WM_C360_ARM_SCULPTOR_SETUP							402
#define WM_C360_LEG_SHAPER_SETUP								403
#define WM_C360_CUSTOM_SETUP									404
#define WM_CALORIE_GOAL_SETUP								405
#define WM_GLUTE_BUSTER_SETUP								406
#define WM_LEG_SHAPER_SETUP									407
#define WM_WL_HRC_SETUP										408
#define WM_TARGET_HRC_SETUP									409
#define WM_AEROBIC_HRC_SETUP									411
#define WM_INTERVALS_HRC_SETUP								412
#define WM_CARDIO_CHAL_SETUP									413


#define WM_WALK_RUN_INTERVALS								414
#define WM_PACE_INTERVALS										415
#define WM_PACE_RAMP											416

#define WM_ROLLING_HILLS										417
#define WM_HILL_INTERVALS										418
#define WM_SINGLE_HILL											419
#define WM_RANDOM_HILLS										422

#define WM_DISTANCE_SETUP										420
#define WM_FIT_TEST_SETUP										421

#define WM_CUST_ULTRA_SETUP_A								430
//#define WM_APPROPRIATE_WORKOUT_SETUP						431

#define WM_HRC_CHART											421

#define WM_WALK_RUN_SETUP									514
#define WM_PACE_INTERVALS_SETUP								515
#define WM_PACE_RAMP_SETUP									516

#define WM_ROLLING_HILLS_SETUP								517
#define WM_HILL_INTERVALS_SETUP								518
#define WM_SINGLE_HILL_SETUP									519
#define WM_RANDOM_HILLS_SETUP								522

#define WM_CUST_ULTRA_SETUP_B								530



//////////////////////////////////////////////////////////////////////
// struct
//////////////////////////////////////////////////////////////////////

#define CHECKSUM_OK		0
#define CHECKSUM_FAIL	1

#define MAGF 1000

#define DRIVE_NONE_ERR		0xFF
#define ERR_STRIDE1_LIMIT_LOW_SW	0x0001
#define ERR_STRIDE1_LIMIT_HIGH_SW	0x0002
#define ERR_STRIDE2_LIMIT_LOW_SW	0x0004
#define ERR_STRIDE2_LIMIT_HIGH_SW	0x0008
#define ERR_RESIST_MOTOR_ERR	0x0100
#define ERR_STRIDE_MOTOR1_ERR	0x0200
#define ERR_STRIDE_MOTOR2_ERR	0x0400
#define ERR_INCLINE_MOTOR_ERR	0x0800

#define INIT_FLAG       0x80
#define M2M_USART_MASKTIME		40	//ms

#define FP_TICKS_PER_SECOND	10
#define HR_MARGIN 		10



// chuck modify 


#define COMMAND_HELLO_RSP 0x0000
#define COMMAND_KEYSCAN_RSP 0x0010
#define COMMAND_POLAR_REP 0x0011
#define COMMAND_CHR_RSP 0x0012
#define COMMAND_ADC_RSP 0x0013
#define COMMAND_MOTO_PLUS_SET_RSP 0x0014
#define COMMAND_MOTO_MINUS_SET_RSP 0x0015
#define COMMAND_RPM_RSP 0x0016
#define COMMAND_BREAK_PWM_SET_RSP 0x0017
#define COMMAND_BUZZ_SET_RSP 0x0018
#define COMMAND_BACKLIGHT_ONOFF_SET_RSP 0x0019
#define COMMAND_BACKLIGHT_PWM_SET_RSP 0x001A
#define COMMAND_SERVO_MOTOR_BIKE_WORKLOAD_SET_RSP 0x001B
#define COMMAND_SERVO_MOTOR_BIKE_STATUS_RSP 0x001C

#define COMMAND_SD55_RPM_RSP 0x0020
#define COMMAND_SD55_ZERO_RSP 0x0021
#define COMMAND_SD55_STATUS_PSP 0x0022
#define COMMAND_SD55_WORKLOAD_SET_RSP 0x0023
#define COMMAND_SD55_STRIDE_SET_RSP 0x0024

#define COMMAND_GET_MCU_VERSION 0x0030




#if 1 // TS210
#define GUI_window_size	12
#else
#define GUI_window_size	10	//gui windows size
#endif



//#define HOLD_STILL_INDEX	4
#define HOLD_STILL_INDEX	5

#define	MAX_SEGMENTS	1501
//#define	MAX_SEGMENTS	255

enum fitness_programs{
	ALLPRESET_DISTANCE		=0,	//Distance
	ALLPRESET_ARM_SCULPTOR,	//Arm Sculptor

	CARDIO360_QUICK_START	=10,	//Quick Start
	CARDIO360_VIDEO,		//Demo
	CARDIO360_ARM_SCULPTOR,	//Cardio 360 Arm Sculptor
	CARDIO360_LEG_SHAPER,	//Leg base
	CARDIO360_CUSTOMIZED,	//Customized
	CARDIO360_DEMO,		//Demo
	
	CUSTOM_HILLS			=20,		//Hills_111810
	CUSTOM_HRC_INTERVALS,	//HRC Intervals_11192010
	CUSTOM_ONE_BIG_HILL,		//Custom_HRC_Intervals();
	CUSTOM_PACE,			//Pace_111810
	CUSTOM_ULTRA,			//Ultra_11182010

	HRC_AEROBIC			=30,	//Aerobic
	HRC_DISTANCE,			//Distance
	HRC_INTERVALS,		//Intervals
	HRC_TARGET,			//Target
	HRC_WEIGHT_LOSS,		//Weight loss	
	
	MANUAL_MANUAL			=40,	//Manual
	MANUAL_QUICK_START,		//Quick Start

	PERFORMANCE_CARDIO_CHALLENGE=50,	//Cardio Challenge
	PERFORMANCE_FITNESS_TEST,		//Fitness test  
	PERFORMANCE_HILL_INTERVALS,		//Hill Intervals
	PERFORMANCE_ONE_BIG_HILL,		//One Big Hill
	PERFORMANCE_PACE_INTERVAL,		//Pace Intervals
	PERFORMANCE_PACE_RAMP,		//Pace Ramp

	WEIGHT_LOSS_CALORIE_GOAL	=60,	//Calorie Goal
	WEIGHT_LOSS_ROLLING_HILLS,		//Rolling hills
	WEIGHT_LOSS_WALK_AND_RUN,	//Walk & Run Intervals
	WEIGHT_LOSS_LEG_SHAPTER, 

	DEFAULT_COOL_DOWN
};

enum fitness_program_sets{
	FPC_All_Presets = 0,
	FPC_Cardio360,	
	FPC_Custom,
	FPC_HRC,
	FPC_Performance,
	FPC_Weight_Loss,
};

enum fitness_program_state{
	IN_NORMAL = 0,
	IN_RUNNING,
	READY_TO_COOL_DOWN,	
	READY_TO_FINISH,
	READY_TO_SUMMARY,

	IN_WARM_UP = 90,
	IN_ERROR = 99,
};

enum process_result{
	EXCEPTION_BREAK = 1,
	EXCEPTION_CONTINUE,
	EXCEPTION_COOLDOWN,
	EXCEPTION_OVERHR_BREAK,
	EXCEPTION_STAGE_ADVANCE,
	EXCEPTION_NO_HR
};

enum segments{
	REST_SEGMENT = 0,
	WORK_SEGMENT,
	REST_PACE,
	WORK_PACE,
	WALK_PACE,
	RUN_PACE,
	WARM_UP_MODE,	
	PROGRESS_MODE,
	OVER_HR_MODE,
	MANUAL_MODE,
	RESUME_MODE,
	HR_LOAD_TEST_MODE,
	PAUSE_MODE,	
	STOP_MODE,	
	BREAK_MODE,	
	FINISH_MODE,	
};

enum adj_types{
//Load ADJ	
	SEGMENT_LOAD_ADJ = 0,
	SEGMENT_INDEX_LOAD_ADJ,

	INDEXED_TARGET_LOAD_ADJ_WLClass2,
	INDEXED_TARGET_LOAD_ADJ_WLClass3,
	INDEXED_TARGET_LOAD_ADJ_WLClass20,

//Pace ADJ
	INDEXED_TARGET_PACE_ADJ_WLClass2,
	INDEXED_TARGET_PACE_ADJ_WLClass20,
};

enum watt_calc_mod_num{
	CALC_BY_CURRENT_LOAD_LEVEL  = 0,
	CALC_BY_INDEXED_LOAD_LEVEL = 1,
};

typedef enum
{
	// 0x00, 耴箂 (筿况穦–Ωpowen onの笲笆祘Α砆硈尿既氨琘琿丁, 癸SD55祇皑笷竚耴箂 
	SERIAL_CMD_INIT = 0,
	// 0x01 --> 砞皑笷竚
	SERIAL_CMD_WR_RESIST_POS,
	// 0x02 --> 弄皑笷竚
	SERIAL_CMD_RD_RESIST_POS,
	// 0x03 --> 砞阁˙皑笷竚
	SERIAL_CMD_WR_STRIDE_POS,
	// 0x04 --> 弄阁˙皑笷竚
	SERIAL_CMD_RD_STRIDE_POS,
	// 0x05 --> 砞蔼皑笷竚
	SERIAL_CMD_WR_INCLINE_POS,
	// 0x06 --> 弄蔼皑笷竚
	SERIAL_CMD_RD_INCLINE_POS,
	// 0x07 --> 弄 SPU INTERVAL
	SERIAL_CMD_RD_SPU_INTERVAL,
	// 0x08 --> 弄篈
	SERIAL_CMD_RD_STATE,
	// 0x09 --> 皑笷钡北
	SERIAL_CMD_MOTOR_CTRL,
	// 0x0A --> 耴箂˙计
	SERIAL_CMD_INIT_STEP,
	// 0x0B --> 弄˙计
	SERIAL_CMD_RD_STEP,
	// 0x0C --> 砞﹚STRIDE程蔼琿计癬﹍
	SERIAL_STRIDE_FIRST_POS,

	SERIAL_CMD_RESERVED1,		// 0x0D
	SERIAL_CMD_RESERVED2,		// 0x0E
	SERIAL_CMD_RESERVED3,		// 0x0F
	SERIAL_CMD_RESERVED4,		// 0x10

	// 0x11 --> 硁ン蝋SD55
	SERIAL_RESET_CONTORLER
}SD55_CMD;

enum RPM_TYPE
{
	RPM_ON,
	RPM_OFF
};

enum    MODE_WORK                       //┮Τ家Α篈审
{
 	MODE_IDLE   =  INIT_FLAG,
	MODE_INPUT      ,               //计誹块
 	MODE_RUN        ,
 	MODE_PAUSE      ,
	MODE_COOLDOWN   ,
	MODE_ENDING     ,
	MODE_TEST       ,
	MODE_SETUP      ,
	MODE_SLEEP      ,              
	MODE_SERVICE    , 
	MODE_INPUTID	,
	MODE_CONNECT	,              
	MODE_SCM_SLEEP  , 		//祇筿审璶ΤSLEEP
	MODE_HW_TEST    , 		//钡筿方蝴臔浪琩
	MODE_BV,			///nick add 20100322 BV_Board_Support
 	MODE_END
};

struct Control_System{	//from CS800 SystemData
	unsigned char Mode_Work;
	unsigned char unit_mode;//そ璣
	unsigned char rpm_mode;//rpm秨闽1-->闽超, 0-->秨币
 	unsigned char	PowrOnPauseTime;
 	unsigned short beep_time_ms;
};

struct WLClass20_Load{
	unsigned short work_load[20];
};

struct WLClass20_Pace{
	unsigned short work_pace[20];
};

//Cardio_360_Quick_Start section
struct WLClass3{
	unsigned char work_load[3];//Low;//class 0
};

struct WLClass2_load{
	unsigned char work_load[2];//Low;//class 0
};

struct BiPace{
	unsigned char pace0;
	unsigned char pace1;
};

///////////////////////////////////////////////////////////
struct ExceptionCmd{
	unsigned long start			:1;	//1
	unsigned long stop			:1;	//2
	unsigned long pause			:1;	//3
	unsigned long resume			:1;	//4
	unsigned long finish			:1;	//5
	unsigned long cool_down		:1;	//6
	unsigned long forward_segment	:1;	//7
	unsigned long backward_segment	:1;	//8

	unsigned long pace_up		:1;	//1
	unsigned long pace_down		:1;	//2
	unsigned long level_up		:1;	//3
	unsigned long level_down		:1;	//4
	unsigned long work_load_up		:1;	//5
	unsigned long work_load_down	:1;	//6
	unsigned long stride_up		:1;	//7
	unsigned long strid_down		:1;	//8

	unsigned long work_hr_reached	:1;	//1
	unsigned long rest_hr_reached	:1;	//2
	unsigned long hr_exceeded		:1;	//3
	unsigned long hr_cruise		:1;	//4	
	unsigned long auto_stride		:1;	//5
	unsigned long reserved16		:1;	//6
	unsigned long reserved17		:1;	//7
	unsigned long reserved18		:1;	//8

	unsigned long reserved01		:1;	//1
	unsigned long reserved02		:1;	//2
	unsigned long auto_populate_pace	:1;	//3
	unsigned long auto_populate_load	:1;	//4
	unsigned long start_distance_set	:1;	//5
	unsigned long distance_started	:1;	//6
	unsigned long segment_shift		:1;	//7
	unsigned long fpc_shutdown		:1;	//8
};

struct	WorkOutSummaryState{
	unsigned char has_warmup			:1;	//1
	unsigned char has_test_load			:1;	//2
	unsigned char has_workout			:1;	//3
	unsigned char has_cooldown			:1;	//4
	unsigned char did_warmup			:1;	//1
	unsigned char did_test_load			:1;	//2
	unsigned char did_workout			:1;	//3
	unsigned char did_cooldown			:1;	//4
};


struct Exception{
	struct ExceptionCmd cmd;
	unsigned char segment_adj;
	unsigned char pace_adj;
	unsigned char level_adj;
	unsigned char load_adj;
	unsigned char stride_adj;
};

struct WorkLoad{
	unsigned char current_workout_level;
	unsigned char current_load_level;
	unsigned char reached_work_hr_level;
	unsigned char reached_rest_hr_level;
};

struct	Audio{
	unsigned char audio_source;
	unsigned char audio_system_volume;
	unsigned char audio_iPod_volume;
};

struct Cardio360_exercise{
	unsigned char sport_mode;
	unsigned char customized_time;
	unsigned char default_segment_time;
	unsigned char work_load_class;
	unsigned char reserved;	
};

struct WorkOutSummaryData{	//for run time var
	float Calories_burned;
	float Distance_metric;
	float Distance_imperial;

	unsigned short Time_elapsed;
	unsigned short Time_elapsed_1000;

	unsigned short Max_HeartRate;
	unsigned short Average_Heart_Rate;
	unsigned long Average_Heart_Rate_base;
	unsigned short Max_Rpm;
	unsigned short Average_Rpm;
	unsigned long Average_Rpm_base;
};

struct RunTimeVar	//run time variable
{
	unsigned char start_calc;
	unsigned char	tick_1sec_per100ms,tick_1sec_per100ms_reload;
	unsigned char	tick_10sec,tick_10sec_reload;
	unsigned short tick_30sec,tick_30sec_reload;
	unsigned short timer_tick,timer_tick_reload;
	unsigned char workout_mode;
	unsigned char exercise_done;
	
	//added by simon@2013/05/07
  	unsigned short elapsed_time_1000;//(计)	
 	unsigned short elapsed_time;//(κ计)

  	unsigned short workout_time_1000;//x
 	unsigned short workout_time;//x

 	unsigned short total_workout_time;//x
	unsigned short total_workout_time_tick;	//x
	unsigned short segment_time;
	unsigned short segment_time_tick;
	
	unsigned char random_index[10];
	
	//for Performance fitness test
	unsigned char stage_extended;
	unsigned short Target_Watts;

	//for user change
	unsigned short Auto_Stride;
	unsigned short Target_Stride;
	unsigned short Target_Workload_level;
	unsigned short Target_Pace_Level;
	unsigned short indexed_Target_Workload;
	unsigned char adjusted_Target_Workload;

	
	unsigned short no_heart_rate_detect;
	unsigned char stage_index;
	
	unsigned short deltaHR;
	unsigned char pace;
	unsigned char level;	
	unsigned char stride;
 	unsigned char watt_calc_mod;
	unsigned char watt_calc_mod_org;
	unsigned char exception_result;
	unsigned char workout_stage;

	unsigned short segment_index;
	unsigned short workout_state;

	unsigned short Max_Rpm;
	unsigned short Average_Rpm;
	unsigned short Average_Rpm_base;
	unsigned long Average_Rpm_ACC;
	
	unsigned short Max_HeartRate;
	unsigned short Average_Heart_Rate;
	unsigned short Average_Heart_Rate_base;
	unsigned long Average_Heart_Rate_ACC;
	
	unsigned long waste_rpm_elapsed_time;
	unsigned long waste_hr_elapsed_time;

	float start_Distance_metric;
	float Distance_metric;
	float Distance_imperial;
	float target_workout_distance;	
	float RemainDistance_metric;
	float RemainDistance_imperial;
	float Calories;
	float Calories_HR;	
	float Mets_metric;
	float Mets_imperial;
	
	unsigned short Watts;
	unsigned short currnet_work_load_level;
	unsigned short current_heart_rate;	
	unsigned short target_cruise_heart_rate;	
	unsigned short TargetCalorie_Watts;
	
	//correct
	struct WorkLoad workLoad;
	unsigned short workLoad_TableUsed;

	unsigned char load_adj_mode;
	unsigned char pace_adj_mode;

	unsigned char *workLoad_Table_cruise;	// Added for Cruise control @20130426
	unsigned short base_cruise_watt;
	unsigned short base_hrc_watt;
	unsigned char before_cruise_level;
	
	unsigned char before_cruise_setup_level;
	unsigned char before_cruise_current_level;
	unsigned char before_cruise_update_level;
	unsigned char before_cruise_Target_heart_rate;
	
	//table pointer
	unsigned char *workLoad_Table;
	unsigned char *workWatt_Table;
	unsigned char *workPace_Table;
	unsigned char *segmentTime_Table;
	struct WLClass20_Load * WLClass20_LoadTable;
	struct WLClass20_Pace * WLClass20_PaceTable;
	struct WLClass2_load  * WLClass2_LoadTable;
	struct Cardio360_exercise * C360_Table;
	struct BiPace * BiPaceTable;
	
//GUI window control
	unsigned short GUI_Bar_window_index;
	unsigned short GUI_Bar_window_Left;
	unsigned short GUI_Bar_window_Right;
	unsigned short total_segment;
//addd 2013/01/25

	//used in Custom_Hills.cpp & ustom_Pace.cpp
	unsigned char customized_hill_LevelTable[30];
	unsigned short warmup_Time_elapsed;

	struct WorkOutSummaryData warmup;
	struct WorkOutSummaryData workout;
	struct WorkOutSummaryData cooldown;
	struct WorkOutSummaryData total;
	struct WorkOutSummaryState summary_state;
	
//for IPDOD control	
	unsigned char ipod_key;
	unsigned char ipod_in_duck;
	struct	Audio audio;
	
};

struct WorkoutSummary{
	unsigned short Vo2;
	
	unsigned short Warmup_avg_heart_rate;	
	unsigned short Warmup_max_heart_rate;	
	unsigned short Warmup_average_pace;	
	unsigned short Warmup_max_pace;	
	
	unsigned short Workout_avg_heart_rate;	
	unsigned short Workout_max_heart_rate;	
	unsigned short Workout_average_pace;	
	unsigned short Workout_max_pace;	
	
	unsigned short Cooldown_avg_heart_rate;	
	unsigned short Cooldown_max_heart_rate;	
	unsigned short Cooldown_average_pace;	
	unsigned short Cooldown_max_pace;	

	unsigned short Total_avg_heart_rate;	
	unsigned short Total_max_heart_rate;	
	unsigned short Total_average_pace;	
	unsigned short Total_max_pace;	
	
	unsigned short Warmup_time_elapsed_1000;//(计)	
	unsigned short Warmup_time_elapsed;//(κ计)	
	unsigned short Warmup_distance_km_i;//(俱计)	
	unsigned short Warmup_distance_km_f;//(计)	
	unsigned short Warmup_calories_burned_1000;	//(计)		
	unsigned short Warmup_calories_burned;//(κ计)
	
	unsigned short Workout_time_elapsed_1000;//(计)	
	unsigned short Workout_time_elapsed;//(κ计)	
	unsigned short Workout_distance_km_i;//(俱计)	
	unsigned short Workout_distance_km_f;//(计)	
	unsigned short Workout_calories_burned_1000;	//(计)	
	unsigned short Workout_calories_burned;//(κ计)	
	
	unsigned short Cooldown_time_elapsed_1000;	//(计)	
	unsigned short Cooldown_time_elapsed;//(κ计)	
	unsigned short Cooldown_distance_km_i;//(俱计)	
	unsigned short Cooldown_distance_km_f;//(计)	
	unsigned short Cooldown_calories_burned_1000;	//(计)	
	unsigned short Cooldown_calories_burned;//(κ计)	

	unsigned short Total_time_elapsed_1000;	//(计)	
	unsigned short Total_time_elapsed;//(κ计)	
	unsigned short Total_distance_km_i;//(俱计)	
	unsigned short Total_distance_km_f;//(计)	
	unsigned short Total_calories_burned_1000;	//(计)	
	unsigned short Total_calories_burned;//(κ计)	
};

struct WorkoutSummaryB
{
	unsigned short data[31];
};


#if 0//def TS210

struct SetupData
{
	unsigned short Work_mode;	//x
	unsigned short Age;	
	unsigned short Segments;	
	unsigned short Pace_level;	
	unsigned short Workload_level;	
	unsigned short Target_heart_rate;	
	unsigned short Work_heart_rate;	
	unsigned short Rest_heart_rate;	
	unsigned short Workout_level;	
	unsigned short Gender;	 
	unsigned short Weight;	
	unsigned short Workout_time_1000;	//()	
	unsigned short Workout_time;	//(κ)	
	unsigned short Calorie_goal;	
	unsigned short Workout_distance;	
	unsigned short Segments_time[10];	//0~9	 
	unsigned char Workload[30];		//0~29
	unsigned char Pace[30];		//0~29
	unsigned char checksum;		//(355)
};

#else


struct SetupData
{
	unsigned int Work_mode;
	unsigned short Age;	
	unsigned short Segments;
	unsigned short Pace_level;	
	unsigned short Workload_level;	
	unsigned short Target_heart_rate;	
	unsigned short Work_heart_rate;	
	unsigned short Rest_heart_rate;	
	unsigned short Workout_level;	
	unsigned short Gender;	 
	unsigned short Weight;	
	//()	
	unsigned short Workout_time_1000;
	//(κ)	
	unsigned short Workout_time;
	unsigned short Calorie_goal;	


	unsigned int Workout_distance;



	//0~9	 
	unsigned short Segments_time[64];
	//0~29
	unsigned char Workload[64];
	unsigned char Pace[64];//0~29
	unsigned char checksum;//(355)


	double distance;
	double weight;
};
#endif // TS210



struct SetupDataB
{
	unsigned short data[25];
	unsigned char data2[61];
};



#if 0//def TS210

struct RealtimeUpdateData{
	//25 unsigned short
	unsigned short Workload_level;	
	unsigned short Heart_rate;	
	unsigned short Target_heart_rate;	
	unsigned short Pace_level;	
	unsigned short Pace_RPM;
	unsigned short Sports_mode;	
	unsigned short Stride_length;	
	unsigned short Time_elapsed_1000;		//(计)	
	unsigned short Time_elapsed;		//(κ计)
	unsigned short Time_remaining_1000;	//(计)
	unsigned short Time_remaining;		//(κ计)	
	
	unsigned short Distance_km_i;		//(km)(タ俱计)	
	unsigned short Distance_km_f;		//(km)(计)	
	
	unsigned short Distance_mi_i;		//(mi)(タ俱计)	
	unsigned short Distance_mi_f;		//(mi)(计)	
	
	unsigned short Distance_remaining_km_i;	//(km)(タ俱计)	
	unsigned short Distance_remaining_km_f;	//(km)(计)	
	
	unsigned short Distance_remaining_mi_i;	//(mi)(タ俱计)	
	unsigned short Distance_remaining_mi_f;	//(mi)(计)	
	
	unsigned short Calories_burned_1000cal;	//(计)	
	unsigned short Calories_burned;		//(κ计)	
	unsigned short Calories_per_hour_1000cal;	//added @2013/01/24 unit:cal
	unsigned short Calories_per_hour;		
	unsigned short Watts;	
	unsigned short Mets;
	unsigned short Segment_time_1000;//x	//added @2013/01/24 unit:second
	unsigned short Segment_time;//x
	//22 unsigned short
	unsigned char workload_index;
	unsigned char Workload_bar[10];	 	//0~9
	unsigned char pace_index;
	unsigned char Pace_bar[10];			//0~9
	unsigned char checksum;
};

#else

struct RealtimeUpdateData{
	//25 unsigned short
	unsigned short Workload_level;	
	unsigned short Heart_rate;	
	unsigned short Target_heart_rate;	
	unsigned short Pace_level;	
	unsigned short Pace_RPM;
	unsigned short Sports_mode;
	unsigned short Stride_length;	

	//(计)	
	unsigned short Time_elapsed_1000;
	//(κ计)
	unsigned short Time_elapsed;
	//(计)
	unsigned short Time_remaining_1000;
	//(κ计)	
	unsigned short Time_remaining;

	//(km)(タ俱计)	
	unsigned short Distance_km_i;
	//(km)(计)
	unsigned short Distance_km_f;

	//(mi)(タ俱计)	
	unsigned short Distance_mi_i;
	//(mi)(计)	
	unsigned short Distance_mi_f;
	
	unsigned short Distance_remaining_km_i;	//(km)(タ俱计)	
	unsigned short Distance_remaining_km_f;	//(km)(计)	
	
	unsigned short Distance_remaining_mi_i;	//(mi)(タ俱计)	
	unsigned short Distance_remaining_mi_f;	//(mi)(计)	
	
	unsigned short Calories_burned_1000cal;	//(计)	
	unsigned short Calories_burned;//(κ计)	
	unsigned short Calories_per_hour_1000cal;	//added @2013/01/24 unit:cal
	unsigned short Calories_per_hour;
	unsigned short Watts;
	unsigned short Mets;

	//x	//added @2013/01/24 unit:second
	unsigned short Segment_time_1000;
	unsigned short Segment_time;

	//22 unsigned short
	unsigned char workload_index;
	//unsigned char Workload_bar[10];	 	//0~9
	unsigned char Workload_bar[32];	 	//0~9

	unsigned char pace_index;
	//unsigned char Pace_bar[10];//0~9
	unsigned char Pace_bar[32];//0~9

	unsigned char checksum;
};

#endif // TS210


struct RealtimeUpdateDataB
{
	unsigned short data[27];
	unsigned char data1[22];
};

enum 
{
	ENGLISH_SYS,	//璣
	METRIC   	//そへ
};



///////////////////////////////////////////////////////////
struct M2M_bHeader{
	unsigned int head;	//command ID
	unsigned int function;//command ID
	unsigned int len;	//the length of exten data
	unsigned char data[1];
};

struct M2M_aHeader{
	unsigned char head[4];	//command ID
	unsigned char function[4];//command ID
	unsigned char len[4];	//the length of exten data
	unsigned char data[1];
};

//message type definition
#define M2M_ASCII_MESSAGE	0
#define M2M_BINARY_MESSAGE	1
#define MAX_MESSAGE_LENGTH	300	

struct MESSAGE_STATE{
	unsigned char none_error	:1;
	unsigned char	header_error	:1;
	unsigned char	len_error	:1;
	unsigned char checksum_error:1;

	unsigned char	ascii_type	:1;
	unsigned char	reserve2	:1;
	unsigned char	reserve3	:1;
	unsigned char	reserve4	:1;
};

struct M2M_Message{
	unsigned char *binary_message;
	unsigned char *work_message;
	unsigned short total_wait_len;
	unsigned short max_wait_time;	//in ms
	unsigned short total_wait_time;	//in ms
	unsigned short delay_respond_time;	//in ms
	unsigned short item_one_data;	//in ms
	unsigned short item_two_data;	//in ms
	unsigned char check_sum;

	struct MESSAGE_STATE state;
	struct M2M_bHeader header;
};

//change name by Simon.
enum protocol_result{	
	M2M_Succeed = 0,		//error result
	M2M_NoRespond,
	M2M_WaiteData,
	M2M_Timeout,
	M2M_Back,
	M2M_CheckSum,
	M2M_Length,
	M2M_ProtocolError,
	M2M_Retry,
};

struct M2M_Status{
	unsigned char error_no;	//protocol_result
	unsigned long error_count;
	unsigned long in_count;
	unsigned long out_count;
};

extern char reply0[];// = "555,000,000,000"; 
extern char reply1[];// = "555,001,000,000"; 
extern char reply3[];// = "555,003,000,000"; 
extern char reply4[];// = "555,004,000,000"; 
extern char reply5[];// = "555,005,000,000"; 
extern char reply6[];// = "555,006,000,000"; 
extern char reply7[];// = "555,007,000,000"; 
extern char reply8[];// = "555,008,000,000"; 
extern char reply9[];// = "555,009,000,000"; 
extern char reply10[];// = "555,010,000,000";
extern char reply11[];// = "555,011,000,000";
extern char reply12[];// = "555,012,000,000";
extern char reply13[];// = "555,013,000,000";
extern char reply15[];// = "555,015,000,000";
extern char reply16[];// = "555,016,032,"; //"xxx,xxx,xxx,xxx,xxx,xxx,xxx,ccc"
extern char reply17[];// = "555,017,000,000";
extern char reply18[];// = "555,018,000,000";
extern char reply19_no[];// = "555,019,004,000,000";
extern char reply19_yes[];// = "555,019,004,001,001";
extern char reply20[];// = "555,020,000,000";
extern char reply99[];// = "555,020,000,000";
extern char reply101[];




//////////////////////////////////////////////////////////////////////
// class
//////////////////////////////////////////////////////////////////////

class CFpcData : public CSd55Data, public Control_System, public RunTimeVar, public SetupData, public Exception, public RealtimeUpdateData, public WorkoutSummary
{
public:
	CFpcData();
	virtual ~CFpcData();


public:
	virtual unsigned char Get_Drive_ErrMessage(void);
	virtual void InitialSummary(void);
	virtual void InititalCalculation(void);
	virtual void Process_SPU(void);
	virtual void Process_Motor_Events(void);
	virtual void CollectWorkOutSummaryData(void);
	virtual void CollectCooldownSummaryData(void);
	virtual void CollectWarmUpSummaryData(void);
	virtual void CalculateCoolDownSummaryWithWorkout(void);
	virtual void CalculateCoolDownSummaryWithWarmup(void);
	virtual void CalculateWorkoutSummaryWithWarmup(void);
	virtual void CalculateSummaryData(void);
	virtual void Update_GUI_bar(void);
	virtual void Update_GUI_bar_Fitness(void);
	virtual void Init_GUI_bar_BiPace(struct BiPace *paceTable);
	virtual void Init_GUI_bar(unsigned short segment_count, unsigned short *work_loadTable);
	virtual void InitWorkoutPhase01(unsigned short default_min);

	virtual void InitWorkoutPhase01_distance(unsigned short default_min);
 	virtual void InitWorkoutPhase02_hrcd(unsigned char segment_time_min);

	virtual void Update_Workout_Time_Elapsed(void);
	virtual void Update_Workout_Time_Remaining(void);
	virtual void UpdateIndexedPaceTable_WLCLASS20(void);
	virtual void UpdateCoolDownPace(unsigned char for_program);
	virtual void UpdateIndexedPaceTable(void);
	virtual float Watts_Calc(void);
	virtual unsigned short Watts_Calc_cruise(void);
	virtual void DataCollection(void);
	//virtual void AdjustMachineWorkLoad(unsigned char adj_type, unsigned char specify_level);
	virtual void InitWorkoutPhase01_mw(void);
	virtual void InitWorkoutPhase02_mw(unsigned short segment_time_min);

	virtual void GetIndexedWorkload_WLClass2_populate(void);
	virtual void GetIndexedWorkload_WLClass20_Pace(void);
	virtual void GetIndexedWorkload_WLClass20_single(void);

	virtual unsigned char GetWorkPace_of_ppi(unsigned char level);
	virtual unsigned char GetWorkPaceLevel_of_ppi(unsigned char rpm);
	virtual void GetIndexedWorkload_C360(void);
	virtual void GetIndexedWorkload_WLClass20_populate(void);
	virtual void InitWorkoutPhase02_apd(unsigned short segment_time_min);
	virtual void Update_GUI_bar_distance(void);
	virtual unsigned char GetHiger10WattWorkLoad_cruise(void);
	virtual unsigned char GetLower10WattWorkLoad_cruise(void);
	virtual void InitWorkoutPhase02_mqs(unsigned short segment_time_min);

	virtual void InitWorkoutPhase01_sec(unsigned short default_sec);
	virtual void InitWorkoutPhase02_C360(unsigned short segment_count, unsigned char for_program, struct Cardio360_exercise * c360_Table);
	virtual void InitWorkoutPhase02_C360V(unsigned short segment_count);
	virtual void InitWorkoutPhase02_C360_V1(unsigned short segment_count, unsigned char for_program, struct Cardio360_exercise * c360_Table);
	virtual void InitWorkoutPhase02_wlcg(unsigned short segment_time_min);
	virtual void InitWorkoutPhase02_wlrh(unsigned short segment_count, struct WLClass20_Load *WLClass20_LoadTable);
	virtual void InitWorkoutPhase02_segTime_pace(unsigned char segment_time_min, struct BiPace *paceTable);
	virtual void InitWorkoutPhase02_1pre_segTime(unsigned short segment_time_min);

	virtual void PopulateWorkLoad(void);
	virtual unsigned char GetHiger10WattWorkLoad(void);
	virtual unsigned char GetLower10WattWorkLoad(void);
	virtual void InitWorkoutPhase02_harci(unsigned char sets);
	virtual unsigned char GetRestSegmentLoadLevel_by_level(unsigned char work_level);
	virtual void PopulateWorkLoad_Intervals_hrci3(void);
	virtual void InitWorkoutPhase02_pcc(unsigned short segment_count, struct WLClass20_Load *WLClass20_LoadTable, struct WLClass20_Pace *WLClass20_PaceTable);
	virtual void InitWorkoutPhase02_pft(unsigned short segment_time_min);
	virtual float GetNewTargeWatt_stage_hr(unsigned char stage, unsigned char last_stage_hr);
	virtual void InitWorkoutPhase02_ppr(unsigned short segment_count);
	virtual void InitWorkoutPhase02_pobh(unsigned short segment_count, struct WLClass20_Load *WLClass20_LoadTable);
	virtual void InitWorkoutPhase02_hi(unsigned short segment_time_min, struct WLClass2_load *WLClass2_LoadTable);
	virtual void InitWorkoutPhase02_cp(void);
	virtual void InitWorkoutPhase02_ch(void);
	virtual void InitWorkoutPhase02_cu(void);
	virtual void PopulateBothSegmentLoad(void);
	virtual void InitWorkoutPhase02_1pre_segSegment_chrci(void);
	virtual void PopulateRestSegmentLoad(void);
	virtual void PopulateWorkSegmentLoad(void);
	virtual void FadInCruiseGUI(void);


public:
	class CSd55Data *lcb;
	class CFpcTb *tables;

	//Inter communication with GUI
	struct Control_System *sys;
	struct RunTimeVar *rt;
	struct Exception *exception;
	struct SetupData *setup;
	struct RealtimeUpdateData *update;
	struct WorkoutSummary *summary;



/*
public:
	void All_Preset_Distance(void);

*/



};


#endif // !defined(AFX_FPCDATA_H__62B243F7_35C4_485A_A548_14BA74273541__INCLUDED_)

