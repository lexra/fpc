// QsApp.h: interface for the CQsApp class.
//
//////////////////////////////////////////////////////////////////////


#if !defined(AFX_QSAPP_H__A1DB9950_6E3A_44C9_BA50_39E1A157A74F__INCLUDED_)
#define AFX_QSAPP_H__A1DB9950_6E3A_44C9_BA50_39E1A157A74F__INCLUDED_


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


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
	HR_LOAD_TEST_MODE,
	PAUSE_MODE,	
	STOP_MODE,	
	BREAK_MODE,	
	FINISH_MODE,	
};

enum watt_calc_mod_num{
	CALC_BY_CURRENT_LOAD_LEVEL  = 0,
	CALC_BY_INDEXED_LOAD_LEVEL = 1,
};

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

enum fitness_program_state{
	IN_NORMAL = 0,
	IN_RUNNING,
	READY_TO_COOL_DOWN,	
	READY_TO_FINISH,
	READY_TO_SUMMARY,

	IN_WARM_UP = 90,
	IN_ERROR = 99,
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

struct Cardio360_exercise{
	unsigned char sport_mode;
	unsigned char customized_time;
	unsigned char default_segment_time;
	unsigned char work_load_class;
	unsigned char reserved;	
};

struct WLClass20_Load{
	unsigned short work_load[20];
};

struct WLClass20_Pace{
	unsigned short work_pace[20];
};

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
	
	unsigned short Warmup_time_elapsed_1000;//(千位數)	
	unsigned short Warmup_time_elapsed;//(百位數以下)	
	unsigned short Warmup_distance_km_i;//(整數)	
	unsigned short Warmup_distance_km_f;//(小數)	
	unsigned short Warmup_calories_burned_1000;	//(千位數)		
	unsigned short Warmup_calories_burned;//(百位數以下)
	
	unsigned short Workout_time_elapsed_1000;//(千位數)	
	unsigned short Workout_time_elapsed;//(百位數以下)	
	unsigned short Workout_distance_km_i;//(整數)	
	unsigned short Workout_distance_km_f;//(小數)	
	unsigned short Workout_calories_burned_1000;	//(千位數)	
	unsigned short Workout_calories_burned;//(百位數以下)	
	
	unsigned short Cooldown_time_elapsed_1000;	//(千位數)	
	unsigned short Cooldown_time_elapsed;//(百位數以下)	
	unsigned short Cooldown_distance_km_i;//(整數)	
	unsigned short Cooldown_distance_km_f;//(小數)	
	unsigned short Cooldown_calories_burned_1000;	//(千位數)	
	unsigned short Cooldown_calories_burned;//(百位數以下)	

	unsigned short Total_time_elapsed_1000;	//(千位數)	
	unsigned short Total_time_elapsed;//(百位數以下)	
	unsigned short Total_distance_km_i;//(整數)	
	unsigned short Total_distance_km_f;//(小數)	
	unsigned short Total_calories_burned_1000;	//(千位數)	
	unsigned short Total_calories_burned;//(百位數以下)	
};

struct ExceptionCmd
{
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

struct Exception
{
	struct ExceptionCmd cmd;
	unsigned char segment_adj;
	unsigned char pace_adj;
	unsigned char level_adj;
	unsigned char load_adj;
	unsigned char stride_adj;
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

struct WorkOutSummaryState
{
	unsigned char has_warmup			:1;	//1
	unsigned char has_test_load			:1;	//2
	unsigned char has_workout			:1;	//3
	unsigned char has_cooldown			:1;	//4
	unsigned char did_warmup			:1;	//1
	unsigned char did_test_load			:1;	//2
	unsigned char did_workout			:1;	//3
	unsigned char did_cooldown			:1;	//4
};

struct WorkLoad
{
	unsigned char current_workout_level;
	unsigned char current_load_level;
	unsigned char reached_work_hr_level;
	unsigned char reached_rest_hr_level;
};

struct Audio{
	unsigned char audio_source;
	unsigned char audio_system_volume;
	unsigned char audio_iPod_volume;
};

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
	unsigned short Workout_time_1000;
	unsigned short Workout_time;
	unsigned short Calorie_goal;	
	unsigned short Workout_distance;
	unsigned short Segments_time[64];
	unsigned char Workload[64];
	unsigned char Pace[30];//0~29
	unsigned char checksum;//(355)
};

struct RealtimeUpdateData{
	unsigned short Workload_level;	
	unsigned short Heart_rate;	
	unsigned short Target_heart_rate;	
	unsigned short Pace_level;	
	unsigned short Pace_RPM;
	unsigned short Sports_mode;
	unsigned short Stride_length;	
	unsigned short Time_elapsed_1000;
	unsigned short Time_elapsed;
	unsigned short Time_remaining_1000;
	unsigned short Time_remaining;
	unsigned short Distance_km_i;
	unsigned short Distance_km_f;
	unsigned short Distance_mi_i;
	unsigned short Distance_mi_f;
	unsigned short Distance_remaining_km_i;	//(km)(正整數)	
	unsigned short Distance_remaining_km_f;	//(km)(小數)	
	unsigned short Distance_remaining_mi_i;	//(mi)(正整數)	
	unsigned short Distance_remaining_mi_f;	//(mi)(小數)	
	unsigned short Calories_burned_1000cal;	//(千位數)	
	unsigned short Calories_burned;//(百位數以下)	
	unsigned short Calories_per_hour_1000cal;	//added @2013/01/24 unit:cal
	unsigned short Calories_per_hour;
	unsigned short Watts;
	unsigned short Mets;
	unsigned short Segment_time_1000;
	unsigned short Segment_time;
	unsigned char workload_index;
	unsigned char Workload_bar[12];	 	//0~9
	unsigned char pace_index;
	unsigned char Pace_bar[12];//0~9
	unsigned char checksum;
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
  	unsigned short elapsed_time_1000;//(千位數)	
 	unsigned short elapsed_time;//(百位數以下)

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
	unsigned char GUI_Bar_window_index;
	unsigned char GUI_Bar_window_Left;
	unsigned char GUI_Bar_window_Right;
	unsigned char total_segment;
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

struct SD55_data
{
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
 	unsigned short Rpm_Buffer[3];
 	unsigned long Drive_SPU_Interval;

	unsigned char New_SPU_Pulse;
};



//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

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
#define MAX_VALID_TIMES    4
//連續檢到無效的心率次數
#define MAX_LOSE_TIMES     4

//心率顯示的最值，注意要在心率采樣周期的范圍內
#define MAX_HEARTRATE       220
#define MIN_HEARTRATE       40


struct HR_ControlState{
	unsigned char hr_ready				:1;//0
	unsigned char TEL_in_use				:1;//1
	unsigned char reserved2				:1;//2
	unsigned char ALL_BUFFER_SAME_FLAG_HGP		:1;//3       //當所有的心率Buffer相同時為true
	unsigned char ALL_BUFFER_SAME_FLAG_TEL		:1;//4       //當所有的心率Buffer相同時為true
	unsigned char CODING_FLAG_TEL			:1;//5       //無線心率的格式為CODED
	unsigned char HR_SAMPLING_VALID_FLAG_HGP		:1;//6       //采樣到的手握心率周期有效標志
	unsigned char HR_SAMPLING_VALID_FLAG_TEL		:1;//7       //采樣到的無線心率周期有效標志
};

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
	//系統:當前無線心率的值
	unsigned char HeartRate_TEL;

	//系統:當前手握心率的值
	unsigned char HeartRate_HGP;
	
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
	unsigned char PCA9555_hardware_read(void);
	void HeartRate_Process(void);
	void HeartRate_Sampling_TEL(void);	//this function will be ticked from the FPC Shceduler every 1ms.
	void HeartRate_Sampling_HGP(void);	//this function will be ticked from the FPC Shceduler every 1ms.
	void Init_HeartRate_Data(int fd);
	unsigned char HeartRate_TEL_hardware_read(void);
	unsigned char HeartRate_HGP_hardware_read(void);
};



//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

class CQsApp : public CApp
{
public:
	CQsApp();
	virtual ~CQsApp();

public:
	int AttachUart(const char *path, int baudrate = 9600);
	int TestUart(const char *path);


//////////////////////////////////////////////////////////////////////
	virtual void WorkoutData_initialize(void);
	virtual void Update_GUI_bar(void);
	virtual void InitWorkout(unsigned short default_min);
	virtual void InitSegment(unsigned char segment_time_min);

	virtual void Init_SPU(void);
	virtual void Process_SPU(void);
	virtual void Set_Stride_Motor_Position(unsigned char position);
	virtual void Set_WorkLoad_Motor_Position(unsigned char position);
	virtual void AdjustMachineWorkLoad(unsigned char adj_type, unsigned char specify_level);

	virtual void Update_Workout_Time_Elapsed(void);
	virtual void Update_Workout_Time_Remaining(void);
	virtual unsigned char GetWorkPace_of_ppi(unsigned char level);
	virtual unsigned char GetWorkPaceLevel_of_ppi(unsigned char rpm);
	virtual void CollectCooldownSummaryData(void);
	virtual void Default_CoolDown_pace_process(unsigned char for_program);
	virtual void Default_CoolDown(void);

	virtual unsigned char ExceptionHandler(unsigned char for_program);

	virtual void UpdateIndexedPaceTable(void);
	virtual void UpdateIndexedPaceTable_WLCLASS20(void);
	virtual void GetIndexedWorkload_C360(void);
	virtual void PopulateWorkLoad_Intervals_hrci3(void);
	virtual void GetIndexedWorkload_WLClass2_populate(void);
	virtual void GetIndexedWorkload_WLClass20_Pace(void);
	virtual void GetIndexedWorkload_WLClass20_populate(void);
	virtual void CollectWarmUpSummaryData(void);
	virtual void CollectWorkOutSummaryData(void);
	virtual unsigned char GetLower10WattWorkLoad_cruise(void);
	virtual unsigned char GetHiger10WattWorkLoad_cruise(void);
	virtual void HR_Cruise(void);
	virtual unsigned char GetRestSegmentLoadLevel_by_level(unsigned char work_level);
	virtual void DataCollection(void);
	virtual void Calculate(void);
	virtual unsigned short Get_RPM(void);
	virtual unsigned char Get_RunRPM(void);
	virtual float Watts_Calc(void);
	virtual unsigned short Watts_Calc_cruise(void);

	virtual unsigned char ProgramStart(void);
	virtual void EndofProgram(unsigned char for_program);




//////////////////////////////////////////////////////////////////////
	virtual unsigned char ProcessUserMessage(unsigned char *buff, int fd);

	void TellFpcWait(void);
	void TellFpcStart(void);
	void FpcWaitForStart(void);
	void WakeupFpc(void);
	void WaitForProgramTick(void);



//////////////////////////////////////////////////////////////////////
public:
	int ipod;
	int bv;

	int adc;
	int gpio;
	int i2c0;
	pthread_t tUi;
	int pwm1;

	HANDLE hUart;
	int ttyUSB0;

	pthread_t tHr;
	pthread_t tKs;
	pthread_t tUart;
	pthread_t tFpc;
	pthread_t tTick;

	HANDLE hFpcStart;
	HANDLE hFpcTick;


//////////////////////////////////////////////////////////////////////
	struct SetupData SETUP;
	struct SetupData *setup;

	struct RunTimeVar RT;
	struct RunTimeVar *rt;

	struct Exception EXCEPTION;
	struct Exception *exception;

	struct RealtimeUpdateData UPDTATE;
	struct RealtimeUpdateData *update;

	struct WorkoutSummary SUMMARY;
	struct WorkoutSummary *summary;

	struct SD55_data DATA;
	struct SD55_data *data;

	class HeartRate HR;
	class HeartRate *hr;

	class CFpcTb TABLES;
	class CFpcTb *tables;



public:
	afx_msg LRESULT OnUartDataRcv(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult);
	afx_msg LRESULT OnUserMessage(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult);
	afx_msg LRESULT OnKeyScanPressed(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult);
	afx_msg LRESULT OnKeyScanReleased(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult);

protected:
	//{{AFX_DATA(CQsApp)
	afx_msg LRESULT OnInitInstance(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult);
	afx_msg LRESULT OnExitInstance(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult);
	afx_msg LRESULT OnTimer(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult);
	//afx_msg LRESULT OnUserMessage(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult);
	//afx_msg LRESULT OnKeyScanPressed(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult);
	//afx_msg LRESULT OnKeyScanReleased(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult);
	//afx_msg LRESULT OnUartDataRcv(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult);
	afx_msg LRESULT OnLeftThumb(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult);
	afx_msg LRESULT OnRightThumb(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult);
	//}}AFX_DATA

	DECLARE_MESSAGE_MAP()
};

#endif // !defined(AFX_QSAPP_H__A1DB9950_6E3A_44C9_BA50_39E1A157A74F__INCLUDED_)

