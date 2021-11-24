// FpcApp.h : 
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FPCAPP_H__62B243F7_35C4_485A_A548_14BA74273546__INCLUDED_)
#define AFX_FPCAPP_H__62B243F7_35C4_485A_A548_14BA74273546__INCLUDED_


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000



#define PWM1_TIMER_ID							101
#define KEY_SCAN_TIMER_ID						102
#define ADC_SCAN_TIMER_ID						103
#define FPC_WORKOUT_IN_NORMAL_ID				104
#define FPC_PROCESS_KEY_EV_ID					105
#define FPC_RETURN_WORKOUT_HOME_ID			106
#define FPC_RETURN_DEFAULT_DS_ID				107
#define FPC_POPUP_WEIGHT_ID					108
#define FPC_DISMISS_WEIGHT_ID					109
#define FPC_ADJUST_RESIST_MOTOR_ID			110
#define FPC_PROBE_CURRENT_AD_ID				111
#define FPC_CLEAR_HRC_ENTERING_ID				112
#define RPM_ZERO_TIMER_ID						113
#define BUZZ_ON_TIMER_ID						114
#define BUZZ_OFF_TIMER_ID						115
#define CHK_BIKE_MOTO_ERR_TIMER_ID			116

#define IPOD_SIMPLE_MODE_TIMER_ID				117
#define IPOD_NEXT_TIMER_ID						118
#define IPOD_PREV_TIMER_ID						119
#define IPOD_SKIP_DEC_TIMER_ID					120
#define IPOD_SKIP_INC_TIMER_ID					121
#define IPOD_PREV_FLAG_TIMER_ID				122
#define IPOD_SELECT_TIMER_ID					123
#define IPOD_BUTTONRELEASE_TIMER1_ID			124
#define IPOD_BUTTONRELEASE_TIMER2_ID			125
#define UDISK_DETECT_TIMER_ID					126
#define NEXT_TRACK_TIMER_ID					127
#define PREV_TRACK_TIMER_ID					128
#define DELAY_NEXT_TRACK_TIMER_ID				129
#define DELAY_PREV_TRACK_TIMER_ID				130
#define SD55_SND_CHK_TIMER_ID					131
#define SD55_RCV_CHK_TIMER_ID					132
#define BL_OFF_TIMER_ID							133
#define IPOD_DETECT_TIMER_ID					134
#define IPOD_MUTE_TIMER_ID						135
#define IPOD_PERIOD_MUTE_TIMER_ID				136
#define IPOD_VOL_UP_TIMER_ID					137
#define IPOD_VOL_DOWN_TIMER_ID				138
#define IPOD_VOL_TIMER_ID						139

#define FPC_SCHEDULER_1MS_ID					301
#define FPC_SCHEDULER_10MS_ID					302
#define FPC_SCHEDULER_70MS_ID					303
#define FPC_SCHEDULER_100MS_ID				304
#define FPC_SCHEDULER_200MS_ID				305
#define FPC_SCHEDULER_250MS_ID				306
#define FPC_SCHEDULER_1000MS_ID				307


typedef struct work_mode_state_t {
	unsigned char type;
	unsigned int target;
	unsigned int source;
	unsigned char key;
	unsigned char shift;
} work_mode_state_t;


//////////////////////////////////////////////////////////////////////
// class
//////////////////////////////////////////////////////////////////////
void MoveMCU_Buzzer_Status(unsigned char Status);
void MoveMCU_Key_ScanToUpdate_Key_Scan(unsigned char key, unsigned char status);
void MoveMCU_ErrorCode_To_Fpc(unsigned char state);
void MoveMCU_Servo_Motor_Bike_Status_To_FPC(unsigned char state);
void MoveMCU_Get_Mcu_Version_To_FPC(char Version[]);

int  getProduct_type();
class CFpcApp : public CSd55App//, public CFpcData
{
public:
	CFpcApp();
	virtual ~CFpcApp();


///////////
public:
	pthread_t tHr;
	pthread_t tKs;
	pthread_t tUi;
	pthread_t tFpc;
	pthread_t tTick;
	HANDLE hFpcTick;
	HANDLE hFpcStart;
	int gpio;
	int pwm1;
	int i2c0;
	int adc;

	int ipod;
	int bv;


///////////
public:


	

	
	virtual void WaitForProgramTick(void);
	virtual void WorkoutData_initialize(void);
	virtual unsigned char ProgramEngineStart(void);
	virtual void Default_CoolDown(void);

	virtual void TellFpcStart(void);
	virtual void TellFpcWait(void);
	virtual void FpcWaitForStart(void);
	virtual void WakeupFpc(void);

	virtual void Default_CoolDown_pace_process(unsigned char for_program);
	virtual void EndofProgram(unsigned char for_program);
	virtual void All_Preset_Distance(void);
	//virtual void Check_SafePin(void);
	virtual void Manual_Quick_Start(void);
	virtual void Manual_Workout_time(void);
	virtual void Manual_Workout(void);
	virtual void Manual_Workout_distance(void);
	virtual void WorkoutLoopType_WLClass3(unsigned char for_program);
	virtual void Cardio360_Demo(void);
	virtual void Cardio360_Quick_Start(void);
	virtual void Cardio360_Video(void);
	virtual void Cardio360_Arm_Sculptor(void);
	virtual void WorkoutLoopType_WLClass3_V1(unsigned char for_program);
	virtual void Cardio360_Leg_Shaper(void);
	virtual void Cardio360_Customized(void);
	virtual void WorkoutLoopType1(unsigned char for_program, unsigned char load_adj, unsigned char watt_calc_mod);
	virtual void Weight_Loss_Calorie_Goal(void) ;
	virtual void Weight_Loss_Rolling_Hills(void);
	virtual void WorkoutLoopType_WLClass20(unsigned char for_program);
	virtual void Weight_Loss_Walk_and_Run(void);
	virtual void Hrc_Target(void);
	virtual void WorkoutLoopType_HRC(unsigned char for_program, unsigned char load_adj, unsigned char watt_calc_mod);
	virtual void Hrc_Weight_Loss(void);
	virtual void Hrc_Aerobic(void);
	virtual void Hrc_Distance(void);
	virtual void WorkoutLoopType_HRC_distance(unsigned char for_program, unsigned char load_adj, unsigned char watt_calc_mod);
	virtual void Hrc_Intervals(void);
	virtual void Performance_Cardio_Challenge(void);
	virtual void Performance_Fitness_Test(void);
	virtual void Performance_Pace_Ramp(void);
	virtual void Performance_Pace_Intervals(void);
	virtual void Performance_One_Big_Hill(void);
	virtual void Performance_Hill_Intervals(void);
	virtual void Custom_Pace(void);
	virtual void Custom_Hills(void);
	virtual void Custom_Ultra(void);
	virtual void Custom_HRC_Intervals(void);
	virtual unsigned char PackUpdateDataAndSend(struct M2M_Message *theMsg, int fd);
	virtual unsigned char PackSummaryDataAndSend(struct M2M_Message *theMsg, int fd);
	virtual void ReplyTime(struct M2M_Message *theMsg, int fd);
	virtual unsigned char ExceptionHandler(unsigned char for_program);
	virtual void AdjustMachineWorkLoad(unsigned char adj_type, unsigned char specify_level);
	virtual void HR_Cruise(void);
	virtual void Calculate(void);
	virtual void HR_Cruise_distance(void);
	virtual void Set_Stride_Motor_Position(unsigned char position);
	virtual void Set_WorkLoad_Motor_Position(unsigned char position);
	//virtual void CC_Init(void);
	virtual void HRC_conditioning(void);
	virtual void FadOutCruiseGUI(void);
	virtual void HRC_conditioning_hrci(void);


///////////////////////////////////////////////////////
	virtual void Random_Hills(void);
	virtual void Weight_Loss_Glute(void);
	virtual void Weight_Loss_Leg(void);
	//virtual void InitWorkoutPhase02_Glute(unsigned char segment_time_min);
	virtual void InitWorkoutPhase02_Leg(unsigned char segment_count);//, struct WLClass20_Load *WLClass20_LoadTable, struct WLClass20_Pace *WLClass20_PaceTable);



///////////
	unsigned char UnPackMessageToBinary(struct M2M_Message *theMsg);
	unsigned char ChecksumCheck(struct M2M_Message *theMsg);


	unsigned char ProcessUserMessage(struct M2M_Message *theMsg, int fd);
	unsigned char ProcessUserMessageB(unsigned char *buff, int fd);



///////////
public:
	virtual unsigned char OnWorkModeChange(struct work_mode_state_t *state);
	virtual unsigned char OnDataScreenChange(struct work_mode_state_t *state);


/*
	//virtual LRESULT OnWorkPageChanged(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult);
	//OnWorkModeChanged
	//virtual LRESULT OnDataScreenChanged(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult);

	virtual LRESULT OnStrideUpButton(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult);
	virtual LRESULT OnStrideDownButton(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult);
	virtual LRESULT OnScrollUpButton(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult);
	virtual LRESULT OnScrollDownButton(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult);

	virtual LRESULT OnStartButton(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult);

	virtual LRESULT OnNumButton(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult);
	virtual LRESULT OnStopButton(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult);
	virtual LRESULT OnDeleteButton(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult);

	virtual LRESULT OnLeftUpButton(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult);
	virtual LRESULT OnLeftDownButton(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult);
	virtual LRESULT OnLeftCenterButton(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult);

	virtual LRESULT OnRightUpButton(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult);
	virtual LRESULT OnRightDownButton(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult);
	virtual LRESULT OnRightCenterButton(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult);
*/


///////////
public:
	afx_msg LRESULT OnUserMessage(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult);
	afx_msg LRESULT OnKeyScanPressed(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult);
	afx_msg LRESULT OnKeyScanReleased(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult);

///////////
protected:

	//{{AFX_DATA(CFpcApp)
	afx_msg LRESULT OnInitInstance(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult);
	afx_msg LRESULT OnExitInstance(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult);
	afx_msg LRESULT OnTimer(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult);
	//afx_msg LRESULT OnUserMessage(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult);
	afx_msg LRESULT OnWaitFpcEndOfProgram(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult);
	//afx_msg LRESULT OnKeyScanPressed(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult);
	//afx_msg LRESULT OnKeyScanReleased(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult);
	afx_msg LRESULT OnLeftThumb(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult);
	afx_msg LRESULT OnRightThumb(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult);
	//}}AFX_DATA

	DECLARE_MESSAGE_MAP()
};


#endif // !defined(AFX_FPCAPP_H__62B243F7_35C4_485A_A548_14BA74273546__INCLUDED_)


