// FpcData.cpp: implementation of the CFpcData class.
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
//#include <sys/inotify.h>
#include <termios.h>
#include <netinet/in.h>
#include <inttypes.h>
#include <stddef.h>
#include <assert.h>
#include <math.h>
#include <sys/socket.h>


#include <asm/ioctl.h>
#include <asm/errno.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#include "list.h"
#include "Event.h"
#include "Misc.h"
#include "FpcTb.h"
#include "Sd55Data.h"
#include "FpcData.h"


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


//////////////////////////////////////////////////////////////////////
// routines
//////////////////////////////////////////////////////////////////////

extern int manual_distance_flag;
extern int product_type;


// H   , F,  , L,  , CS
// 555,099,000,000
// 555,100,

char reply0[] = "555,000,000,000"; 
char reply1[] = "555,001,000,000"; 
char reply3[] = "555,003,000,000"; 
char reply4[] = "555,004,000,000"; 
char reply5[] = "555,005,000,000"; 
char reply6[] = "555,006,000,000"; 
char reply7[] = "555,007,000,000"; 
char reply8[] = "555,008,000,000"; 
char reply9[] = "555,009,000,000"; 
char reply10[] = "555,010,000,000";
char reply11[] = "555,011,000,000";
char reply12[] = "555,012,000,000";
char reply13[] = "555,013,000,000";
char reply15[] = "555,015,000,000";
char reply16[] = "555,016,032,"; //"xxx,xxx,xxx,xxx,xxx,xxx,xxx,ccc"
char reply17[] = "555,017,000,000";
char reply18[] = "555,018,000,000";
char reply19_no[] = "555,019,004,000,000";
char reply19_yes[] = "555,019,004,001,001";
char reply20[] = "555,020,000,000";
char reply99[] = "555,099,000,000";
char reply101[] = "555,101,000,000"; 


#define HRC_INTERVAL_SEGMENT_COUNT 24

struct HRC_Intervals_exercise hi_Table[HRC_INTERVAL_SEGMENT_COUNT] =
{
	{0,WORK_SEGMENT,30,1},	// 0
	{0,WORK_SEGMENT,30,1},	// 1
	{0,REST_SEGMENT,30,1},	// 2
	{0,WORK_SEGMENT,30,1},	// 3
	{0,WORK_SEGMENT,30,1},	// 4
	{0,REST_SEGMENT,30,1},	//5
	{0,WORK_SEGMENT,30,1},	//6
	{0,WORK_SEGMENT,30,1},	//7
	{0,REST_SEGMENT,30,1},	//8
	{0,WORK_SEGMENT,30,1},	//9
	{0,WORK_SEGMENT,30,1},	//10
	{0,REST_SEGMENT,30,1},	//11
	{0,WORK_SEGMENT,30,1},	//12
	{0,WORK_SEGMENT,30,1},	//13
	{0,REST_SEGMENT,30,1},	//14
	{0,WORK_SEGMENT,30,1},	//15
	{0,WORK_SEGMENT,30,1},	//16
	{0,REST_SEGMENT,30,1},	//17
	{0,WORK_SEGMENT,30,1},	//18
	{0,WORK_SEGMENT,30,1},	//19
	{0,REST_SEGMENT,30,1},	//20
	{0,WORK_SEGMENT,30,1},	//21
	{0,WORK_SEGMENT,30,1},	//22
	{0,REST_SEGMENT,30,1}	//23
};



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFpcData::CFpcData() : CSd55Data(), Control_System(), RunTimeVar(), SetupData(), Exception(), RealtimeUpdateData(), WorkoutSummary()
{
	lcb = (class CSd55Data *)this;
	tables = (class CFpcTb *)this;

	sys = (struct Control_System *)this;
	rt = (struct RunTimeVar *)this;

	setup = (struct SetupData *)this;
	exception = (struct Exception *)this;
	update = (struct RealtimeUpdateData *)this;

	summary = (struct WorkoutSummary *)this;
}

CFpcData::~CFpcData()
{
}


//////////////////////////////////////////////////////////////////////
// member
//////////////////////////////////////////////////////////////////////

unsigned char CFpcData::Get_Drive_ErrMessage(void)
{
	unsigned short error_msg_temp = cc->ErrorNo;

	if((sys->rpm_mode == RPM_OFF) || (sys->PowrOnPauseTime == 0) || (sys->Mode_Work != MODE_RUN))	return DRIVE_NONE_ERR;

	//----------------------------------------------------------------------
	if(error_msg_temp == 0){
		if(!cc->state.ComError)
			return DRIVE_NONE_ERR;
		else
			return ERR_MSG_SERIAL_ERROR;
	}else{
		if((error_msg_temp & ERR_STRIDE1_LIMIT_LOW_SW) == ERR_STRIDE1_LIMIT_LOW_SW)
			return ERR_MSG_STRIDE1_LIMIT_LOW_SW;
		if((error_msg_temp & ERR_STRIDE1_LIMIT_HIGH_SW) == ERR_STRIDE1_LIMIT_HIGH_SW)
			return ERR_MSG_STRIDE1_LIMIT_HIGH_SW;
		if((error_msg_temp & ERR_STRIDE2_LIMIT_LOW_SW) == ERR_STRIDE2_LIMIT_LOW_SW)
			return ERR_MSG_STRIDE2_LIMIT_LOW_SW;
		if((error_msg_temp & ERR_STRIDE2_LIMIT_HIGH_SW) == ERR_STRIDE2_LIMIT_HIGH_SW)
			return ERR_MSG_STRIDE2_LIMIT_HIGH_SW;
		if((error_msg_temp & ERR_RESIST_MOTOR_ERR) == ERR_RESIST_MOTOR_ERR)
			return ERR_MSG_RESIST_MOTOR_ERR;
		if((error_msg_temp & ERR_STRIDE_MOTOR1_ERR) == ERR_STRIDE_MOTOR1_ERR)
			return ERR_MSG_STRIDE_MOTOR1_ERR;
		if((error_msg_temp & ERR_STRIDE_MOTOR2_ERR) == ERR_STRIDE_MOTOR2_ERR)
			return ERR_MSG_STRIDE_MOTOR2_ERR;
		if((error_msg_temp & ERR_INCLINE_MOTOR_ERR) == ERR_INCLINE_MOTOR_ERR)
			return ERR_MSG_INCLINE_MOTOR_ERR;
		return DRIVE_NONE_ERR;
	}
}

void CFpcData::InitialSummary(void)
{
	rt->summary_state.has_warmup = 0;
	rt->summary_state.has_test_load = 0;
	//rt->summary_state.has_workout = 1;
	//rt->summary_state.has_cooldown = 1;

	rt->summary_state.has_workout = 0;
	rt->summary_state.has_cooldown = 0;

	rt->summary_state.did_warmup = 0;
	rt->summary_state.did_test_load = 0;
	rt->summary_state.did_workout = 0;
	rt->summary_state.did_cooldown = 0;
}

void CFpcData::InititalCalculation(void)
{
	rt->Watts 			 = 0;
	rt->Calories			 = 0;
	rt->Calories			 = 0;
	rt->Mets_metric		 = 0;
	rt->Mets_imperial 		 = 0;
	rt->Distance_metric		 = 0;
	rt->Distance_imperial 	 = 0;
	rt->RemainDistance_metric 	 = 0;
	rt->RemainDistance_imperial  = 0;

	update->Watts		 = 0;
	update->Calories_per_hour	 = 0;
	update->Calories_burned_1000cal  = 0;
	update->Calories_burned 	 = 0;
	update->Mets 		 = 0;
	update->Distance_km_i 	 = 0;
	update->Distance_km_f 	 = 0;
	update->Distance_mi_i 	 = 0;
	update->Distance_mi_f 	 = 0;

	update->Distance_remaining_km_i  = 0;
	update->Distance_remaining_km_f = 0;
	update->Distance_remaining_mi_i = 0;
	update->Distance_remaining_mi_f = 0;

	lcb->hr->DisplayHeartRate = 0;

	rt->Max_Rpm 			 = 0;
}


void CFpcData::Process_SPU(void)
{
	// 存放Rpm的計算值   
	unsigned short rpm = 0, rpm_sum;
	unsigned char i;
	unsigned short lower = 0;


	// chuck modify
	
	update->Pace_RPM = data->Rpm;
	return;

	

	
	if (0 == product_type)
		lower = 5;

	//In stead of checking the cycle time by the flag,
	//this function is called every 200ms from FPC_Scheduler_Task from FastTask.c	//by Simon @2012/12/25
	//if(data.Read_SPUReading_Flag){
		//data.Read_SPUReading_Flag = 0;// 為下次采樣作准備   

		//this can be the realtime I/O state of the SPU pin 
		if(cc->state.New_SPU_Pulse == 1)
		{
			cc->state.New_SPU_Pulse = 0;
			// 每6個spu為1圈     Get_SPU()->每秒有多少個SPU    *1.17-->跟實際RPM不太合，可能是速比不對，或是打滑 
			if (data->Drive_SPU_Interval == 0)
				data->Drive_SPU_Interval = 1;
			rpm = (60000 / data->Drive_SPU_Interval);
			if(2 == product_type)
				rpm = rpm / 52;
			if(rpm > 0)
				data->Clear_Rpm_Delay = 0;
			if(rpm > lower)
			{
				//第一次采集到Rpm的值需要填充到3個buffer中去
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
					//保存當前采集的數据
					data->Rpm_Buffer[data->Buffer_Index] = rpm;
					data->Buffer_Index++;
				}
				if(data->Buffer_Index >= 3)
				{
					//一個采集周期完成則計算				
					rpm_sum = 0;
					for(i = 0; i < 3; i++)
						rpm_sum += data->Rpm_Buffer[i];
					data->Rpm = rpm_sum / 3;                                
					if(data->Rpm > 199)
						data->Rpm = 199;
					//data->Rpm = (unsigned short)(data->Rpm);
					data->Buffer_Index = 0;
				}
			}
			else
			{
				//data->Rpm = rpm;
				data->Rpm = (unsigned short)(rpm);//(data->Rpm);
				for(i = 0; i < 3; i++)
					data->Rpm_Buffer[i] = 0;
				data->Buffer_Index = 0;
			}
		}
		//else
		//{
		//	if(System_Dat.rpm_mode == RPM_OFF)
		//	{
		//		Rpm = 60;
		//		Clear_Rpm_Delay = 0;
		//		Buffer_Index = 0;
		//	}
		//} 
		//if(mach.mach.rpm_mode == RPM_OFF){
		if(sys->rpm_mode == RPM_OFF)
		{
			data->Rpm = 60;
			data->Clear_Rpm_Delay = 0;
			data->Buffer_Index = 0;
		}
	//}

	if(data->Clear_Rpm_Delay > 70)
	{
		if (0 == product_type)
			Init_SPU();
	}

	if (1 == product_type)
	{
		if (update->Pace_RPM >= data->Rpm)
		{
			if (update->Pace_RPM - data->Rpm > 0)
				update->Pace_RPM -= 1;
			else
				update->Pace_RPM = data->Rpm;
		}
		else
		{
			if (data->Rpm - update->Pace_RPM > 1)
				update->Pace_RPM += 1;
			else
				update->Pace_RPM = data->Rpm;
		}
	}
	else
	{
		update->Pace_RPM = data->Rpm;
	}
}


void CFpcData::Process_Motor_Events(void)
{
	unsigned char M2M_error;
	M2M_error = Get_Drive_ErrMessage();
	if(M2M_error != DRIVE_NONE_ERR){
		//Clear_MessageBuf();
		switch(M2M_error){
		case ERR_MSG_STRIDE1_LIMIT_LOW_SW:
			//Clear_MessageBuf();
			//Show_Message(CENTER, "STRIDE1 LIMIT LOW");
			break;
		case ERR_MSG_STRIDE1_LIMIT_HIGH_SW:
			//Clear_MessageBuf();
			//Show_Message(CENTER, "STRIDE1 LIMIT HI");
			break;
		case ERR_MSG_STRIDE2_LIMIT_LOW_SW:
			//Clear_MessageBuf();
			//Show_Message(CENTER, "STRIDE2 LIMIT LOW");
			break;
		case ERR_MSG_STRIDE2_LIMIT_HIGH_SW:
			//Clear_MessageBuf();
			//Show_Message(CENTER, "STRIDE2 LIMIT HI");
			break;
		case ERR_MSG_RESIST_MOTOR_ERR:
			//Clear_MessageBuf();
			//Show_Message(CENTER, "RESIST MOTOR ERR");
			break;
		case ERR_MSG_STRIDE_MOTOR1_ERR:
			//Clear_MessageBuf();
			//Show_Message(CENTER, "STRIDE MOTOR1 ERR");
			break;
		case ERR_MSG_STRIDE_MOTOR2_ERR:
			//Clear_MessageBuf();
			//Show_Message(CENTER, "STRIDE MOTOR2 ERR");
			break;
		case ERR_MSG_SERIAL_ERROR:
			//Clear_MessageBuf();
			//Show_Message(CENTER, "SERIAL ERR");
			break;
		case ERR_MSG_INCLINE_MOTOR_ERR:
			//Clear_MessageBuf();
			//Show_Message(CENTER, "INCLINE MOTOR ERR");
			break;
		default:
			break;
		}
	}

// 	cc.Sec_tick_100ms --;
// 	if(cc.Sec_tick_100ms == 0){
// 		cc.Sec_tick_100ms = 10;
// 		if(data.Initial_Time < 25){	//todo: by Simon. 2012/11/15
// 			//if(cc.Initial_Time == 0)Switch_Message("INITIALIZING","PLEASE WAIT");
// 			data.Initial_Time++;
// 			if(data.Initial_Time == 3){
// 				//Set_Stride_Motor_Position(mach.mach.Stride_Length);
// 				Set_Stride_Motor_Position(DEFAULT_STRIDE_LENGTH);	//by Simon @20130624
// 				Set_WorkLoad_Motor_Position(1);
// 			}
// 			//if(data.Initial_Time == 15){
// 			if(data.Initial_Time == 23){					//by Simon. for testing the longest return wait 2012/11/15
// 				//Set_Stride_Motor_Position(mach.mach.Stride_Length);
// 				Set_Stride_Motor_Position(DEFAULT_STRIDE_LENGTH);	//by Simon @20130624
// 				Set_WorkLoad_Motor_Position(1);
// 			}
// 		}
// 	}
}

void CFpcData::CollectWarmUpSummaryData(void)	// Every secods, Calculate the Calories, Mets, Distance, HeartRate called from FPC_Scheduler_Task() in Fast_task.c 
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

void CFpcData::CollectWorkOutSummaryData(void)	// Every secods, Calculate the Calories, Mets, Distance, HeartRate called from FPC_Scheduler_Task() in Fast_task.c 
{
	rt->workout.Time_elapsed = rt->elapsed_time;
	rt->workout.Time_elapsed_1000 = rt->elapsed_time_1000;
	
	rt->workout.Calories_burned = rt->Calories;
	rt->workout.Distance_metric = rt->Distance_metric;
	rt->workout.Distance_imperial = rt->Distance_imperial;
	
	rt->workout.Max_HeartRate	= rt->Max_HeartRate;
	rt->workout.Average_Heart_Rate = rt->Average_Heart_Rate;
	rt->workout.Average_Heart_Rate_base = rt->Average_Heart_Rate_base;
	
	rt->workout.Max_Rpm = rt->Max_Rpm;
	rt->workout.Average_Rpm = rt->Average_Rpm;
	rt->workout.Average_Rpm_base = rt->Average_Rpm_base;

	rt->summary_state.has_workout = 1;//GOT_WORKOUT_SUMMARY;

printf("(%s %d) workout Time_elapsed=%d\n", __FILE__, __LINE__, rt->workout.Time_elapsed + 1000 * rt->workout.Time_elapsed_1000);

}

void CFpcData::CollectCooldownSummaryData(void)	// Every secods, Calculate the Calories, Mets, Distance, HeartRate called from FPC_Scheduler_Task() in Fast_task.c 
{
//update->Time_remaining


// jason note
#if 0
	rt->cooldown.Time_elapsed = 120 - update->Time_remaining;
	rt->cooldown.Time_elapsed_1000 = 0;
#else
	rt->cooldown.Time_elapsed = rt->elapsed_time;
	rt->cooldown.Time_elapsed_1000 = rt->elapsed_time_1000;
#endif // 0

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


printf("(%s %d) CollectCooldownSummaryData(%d)\n", __FILE__, __LINE__, rt->cooldown.Time_elapsed + rt->cooldown.Time_elapsed_1000 * 1000);

}

void CFpcData::CalculateCoolDownSummaryWithWorkout(void)
{
	unsigned long temp_integer1,temp_integer2;

	temp_integer1 = rt->workout.Time_elapsed + rt->workout.Time_elapsed_1000 * 1000;
	temp_integer2 = rt->cooldown.Time_elapsed + rt->cooldown.Time_elapsed_1000 * 1000;


	rt->cooldown.Time_elapsed_1000 = (temp_integer2 - temp_integer1) / MAGF;
	rt->cooldown.Time_elapsed = (temp_integer2 - temp_integer1) - rt->cooldown.Time_elapsed_1000 * MAGF;

	rt->cooldown.Calories_burned = rt->cooldown.Calories_burned - rt->workout.Calories_burned;

	rt->cooldown.Distance_metric = rt->cooldown.Distance_metric - rt->workout.Distance_metric;
	rt->cooldown.Distance_imperial = rt->cooldown.Distance_imperial - rt->workout.Distance_imperial;


	Average2(
		rt->workout.Average_Heart_Rate,
		rt->workout.Average_Heart_Rate_base,
	
		rt->cooldown.Average_Heart_Rate,
		rt->cooldown.Average_Heart_Rate_base,
		
		&rt->cooldown.Average_Heart_Rate
	);
	Average2(
		rt->workout.Average_Rpm,
		rt->workout.Average_Rpm_base,

		rt->cooldown.Average_Rpm,
		rt->cooldown.Average_Rpm_base,
		
		&rt->cooldown.Average_Rpm
	);


printf("(%s %d) CalculateCoolDownSummaryWithWorkout(%lu %lu)\n", __FILE__, __LINE__, temp_integer1, temp_integer2);
}

void CFpcData::CalculateCoolDownSummaryWithWarmup(void)
{
	unsigned long temp_integer1,temp_integer2;
	temp_integer1 = rt->warmup.Time_elapsed_1000*1000 + rt->warmup.Time_elapsed;
	temp_integer2 = rt->cooldown.Time_elapsed + rt->cooldown.Time_elapsed_1000*1000;
	
	rt->cooldown.Time_elapsed_1000  = (temp_integer2-temp_integer1) / MAGF;
	rt->cooldown.Time_elapsed = (temp_integer2-temp_integer1) - rt->cooldown.Time_elapsed_1000 * MAGF;
	rt->cooldown.Calories_burned = rt->cooldown.Calories_burned - rt->warmup.Calories_burned;
	rt->cooldown.Distance_metric = rt->cooldown.Distance_metric - rt->warmup.Distance_metric;
	rt->cooldown.Distance_imperial = rt->cooldown.Distance_imperial - rt->warmup.Distance_imperial;
	Average2(
		rt->warmup.Average_Heart_Rate,
		rt->warmup.Average_Heart_Rate_base,
	
		rt->cooldown.Average_Heart_Rate,
		rt->cooldown.Average_Heart_Rate_base,
	
		&rt->cooldown.Average_Heart_Rate
	);
	
	Average2(
		rt->warmup.Average_Rpm,
		rt->warmup.Average_Rpm_base,
		
		rt->cooldown.Average_Rpm,
		rt->cooldown.Average_Rpm_base,
		
		&rt->cooldown.Average_Rpm
	);	
}

void CFpcData::CalculateWorkoutSummaryWithWarmup(void)
{
	unsigned long temp_integer1,temp_integer2;

	temp_integer1 = rt->warmup.Time_elapsed + rt->warmup.Time_elapsed_1000*1000;
	temp_integer2 = rt->workout.Time_elapsed + rt->workout.Time_elapsed_1000*1000;
	

	rt->workout.Time_elapsed_1000  = (temp_integer2-temp_integer1) / MAGF;
	rt->workout.Time_elapsed 	 = (temp_integer2-temp_integer1) - 
								rt->workout.Time_elapsed_1000 * MAGF;
	
	rt->workout.Calories_burned	
	 = rt->workout.Calories_burned - rt->warmup.Calories_burned;
	
	rt->workout.Distance_metric 	
	 = rt->workout.Distance_metric - rt->warmup.Distance_metric;
	
	rt->workout.Distance_imperial	
	 = rt->workout.Distance_imperial - rt->warmup.Distance_imperial;

	Average2(
		rt->warmup.Average_Heart_Rate,
		temp_integer1,
		rt->workout.Average_Heart_Rate,
		temp_integer2,
		&rt->workout.Average_Heart_Rate
	);
	
	Average2(
		rt->warmup.Average_Rpm,
		temp_integer1,
		rt->workout.Average_Rpm,
		temp_integer2,
		&rt->workout.Average_Rpm
	);
}

// elapsed_time
// Every secods, Calculate the Calories, Mets, Distance, HeartRate called from FPC_Scheduler_Task() in Fast_task.c 
void CFpcData::CalculateSummaryData(void)
{
	unsigned long temp_integer;

	if(rt->summary_state.has_cooldown == 1)
	{
//printf("(%s %d) CooldownSummaryData(%d)\n", __FILE__, __LINE__, rt->cooldown.Time_elapsed + rt->cooldown.Time_elapsed_1000 * 1000);
printf("(%s %d) CalculateSummaryData(0, %d, %d)\n",  __FILE__, __LINE__, rt->cooldown.Time_elapsed, rt->summary_state.has_workout);


		//assigne cooldown summary to total summary
		rt->total.Time_elapsed = rt->cooldown.Time_elapsed;
		rt->total.Time_elapsed_1000 = rt->cooldown.Time_elapsed_1000;
		
		rt->total.Calories_burned = rt->cooldown.Calories_burned;
		rt->total.Distance_metric = rt->cooldown.Distance_metric;
		rt->total.Distance_imperial = rt->cooldown.Distance_imperial;
		
		rt->total.Max_HeartRate = rt->cooldown.Max_HeartRate;
		rt->total.Average_Heart_Rate = rt->cooldown.Average_Heart_Rate;
		rt->total.Max_Rpm = rt->cooldown.Max_Rpm;
		rt->total.Average_Rpm = rt->cooldown.Average_Rpm;

		if(rt->summary_state.has_workout == 1)
		{
printf("(%s %d) rt->summary_state.has_workout == 1\n",  __FILE__, __LINE__);

			CalculateCoolDownSummaryWithWorkout();

printf("(%s %d) rt->cooldown.Time_elapsed=%d\n",  __FILE__, __LINE__, rt->cooldown.Time_elapsed);


			if(rt->summary_state.has_warmup == 1)
			{
				CalculateWorkoutSummaryWithWarmup();
			}
			else
			{
				//CalculateWorkoutSummaryWithoutWarmup();
			}
		}
		else
		{
			if(rt->summary_state.has_warmup == 1)
			{
				CalculateCoolDownSummaryWithWarmup();
			}
			else
			{
// jason note
				//CalculateCoolDownSummaryWithWorkout();
				//CalculateCoolDownSummaryWithWarmup();

				//CalculateCoolDownSummaryWithoutAny();
				//there is no warmup
			}
		}		
	}
	else
	{
		if(rt->summary_state.has_workout == 1)
		{
//printf("(%s %d) CalculateSummaryData(1)\n",  __FILE__, __LINE__);

			//set total to workout
			//assigne workout summary to total summary
			rt->total.Time_elapsed = rt->workout.Time_elapsed;
			rt->total.Time_elapsed_1000 = rt->workout.Time_elapsed_1000;

			rt->total.Calories_burned  = rt->workout.Calories_burned;
			rt->total.Distance_metric  = rt->workout.Distance_metric;
			rt->total.Distance_imperial  = rt->workout.Distance_imperial;
			
			rt->total.Max_HeartRate	 = rt->workout.Max_HeartRate;
			rt->total.Average_Heart_Rate = rt->workout.Average_Heart_Rate;
			rt->total.Max_Rpm		 = rt->workout.Max_Rpm;
			rt->total.Average_Rpm	 = rt->workout.Average_Rpm;

//printf("(%s %d) rt->summary_state.has_workout == 1 ",  __FILE__, __LINE__);
//printf("(%s %d) rt->total.Time_elapsed=%d rt->total.Time_elapsed_1000=%d\n",  __FILE__, __LINE__, rt->total.Time_elapsed, rt->total.Time_elapsed_1000);

			
			if(rt->summary_state.has_warmup == 1)
			{
				CalculateWorkoutSummaryWithWarmup();
			}
			else
			{
				//there is no warmup
			}
		}
		else
		{
//printf("(%s %d) rt->summary_state.has_workout == 0\n",  __FILE__, __LINE__);
//printf("(%s %d) CalculateSummaryData(2)\n",  __FILE__, __LINE__);

			//there is no workout			
			if(rt->summary_state.has_warmup == 1)
			{
				//set total to warmup
				//assigne warmup summary to total summary
				rt->total.Time_elapsed	 = rt->warmup.Time_elapsed;
				rt->total.Time_elapsed_1000 = rt->warmup.Time_elapsed_1000;
				
				rt->total.Calories_burned  = rt->warmup.Calories_burned;
				rt->total.Distance_metric  = rt->warmup.Distance_metric;
				rt->total.Distance_imperial  = rt->warmup.Distance_imperial;
				
				rt->total.Max_HeartRate	 = rt->warmup.Max_HeartRate;
				rt->total.Average_Heart_Rate = rt->warmup.Average_Heart_Rate;
				rt->total.Max_Rpm		 = rt->warmup.Max_Rpm;
				rt->total.Average_Rpm	 = rt->warmup.Average_Rpm;
				
				//CalculateWarmupSummary();
			}
			else
			{


				//there is no warmup
			}
		}		
		
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//Summary: warmup part
	if(rt->summary_state.has_warmup == 1)
	{
//printf("(%s %d) rt->summary_state.has_warmup == 1\n",  __FILE__, __LINE__);
//printf("(%s %d) CalculateSummaryData(3)\n",  __FILE__, __LINE__);

		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//Summary: Warm up part
		summary->Warmup_avg_heart_rate	 = rt->warmup.Average_Heart_Rate;
		summary->Warmup_max_heart_rate	 = rt->warmup.Max_HeartRate;	
		summary->Warmup_average_pace	 = rt->warmup.Average_Rpm;
		summary->Warmup_max_pace		 = rt->warmup.Max_Rpm;	

		//based on rt->workout_Time_elapsed
		summary->Warmup_time_elapsed_1000 = rt->warmup.Time_elapsed_1000;
		summary->Warmup_time_elapsed	 = rt->warmup.Time_elapsed;
		
		//based on rt->Workout_Distance_metri
		temp_integer = (unsigned long)(rt->warmup.Distance_metric*MAGF);
		summary->Warmup_distance_km_i	 = temp_integer/MAGF;
		summary->Warmup_distance_km_f	 = temp_integer - summary->Warmup_distance_km_i*MAGF;	
		
		//based on rt->Calories_burned
		temp_integer = (unsigned long)(rt->warmup.Calories_burned);
		summary->Warmup_calories_burned_1000 = temp_integer/1000;
		summary->Warmup_calories_burned	 = temp_integer - summary->Warmup_calories_burned_1000*1000;
	}
	else
	{
printf("(%s %d) CalculateSummaryData(4)\n",  __FILE__, __LINE__);

		summary->Warmup_avg_heart_rate = 0;
		summary->Warmup_max_heart_rate = 0;	
		summary->Warmup_average_pace = 0;
		summary->Warmup_max_pace = 0;	

		//based on rt->workout_Time_elapsed
		summary->Warmup_time_elapsed_1000 = 0;
		summary->Warmup_time_elapsed = 0;
		
		//based on rt->Workout_Distance_metri
		summary->Warmup_distance_km_i = 0;
		summary->Warmup_distance_km_f = 0;	
		
		//based on rt->Calories_burned
		summary->Warmup_calories_burned_1000 = 0;
		summary->Warmup_calories_burned = 0;
	}


		
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//Summary: Work out part
	if(rt->summary_state.has_workout == 1)
	{
		summary->Workout_avg_heart_rate = rt->workout.Average_Heart_Rate;
		summary->Workout_max_heart_rate = rt->workout.Max_HeartRate;	
		summary->Workout_average_pace = rt->workout.Average_Rpm;
		summary->Workout_max_pace = rt->workout.Max_Rpm;	

		//based on rt->workout_Time_elapsed
		summary->Workout_time_elapsed_1000 = rt->workout.Time_elapsed_1000;
		summary->Workout_time_elapsed = rt->workout.Time_elapsed;
	
		//based on rt->Workout_Distance_metri
		temp_integer = (unsigned long)(rt->workout.Distance_metric*MAGF);
		summary->Workout_distance_km_i = temp_integer/MAGF;
		summary->Workout_distance_km_f = temp_integer - summary->Workout_distance_km_i*MAGF;	
		
		//based on rt->Calories_burned
		temp_integer = (unsigned long)(rt->workout.Calories_burned);
		summary->Workout_calories_burned_1000 = temp_integer/1000;
		summary->Workout_calories_burned	 = temp_integer - summary->Workout_calories_burned_1000*1000;

printf("(%s %d) CalculateSummaryData(5), Workout_time_elapsed=%d\n",  __FILE__, __LINE__, Workout_time_elapsed + 1000 * Workout_time_elapsed_1000);
	}
	else
	{
//printf("(%s %d) rt->summary_state.has_workout == 0\n",  __FILE__, __LINE__);
//printf("(%s %d) CalculateSummaryData(6)\n",  __FILE__, __LINE__);

		summary->Workout_avg_heart_rate	 = 0;
		summary->Workout_max_heart_rate	 = 0;
		summary->Workout_average_pace	 = 0;
		summary->Workout_max_pace		 = 0;
		//based on rt->cooldwon_Time_elapsed
		summary->Workout_time_elapsed_1000 = 0;
		summary->Workout_time_elapsed	 = 0;
		//based on rt->cooldwon_Distance_metri 
		summary->Workout_distance_km_i	 = 0;
		summary->Workout_distance_km_f	 = 0;
		//based on rt->cooldown_Calories_burned
		summary->Workout_calories_burned_1000 = 0;
		summary->Workout_calories_burned	 = 0;
	}


	
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//Summary: Cool down part
	if(rt->summary_state.has_cooldown == 1)
	{
//printf("(%s %d) rt->summary_state.has_cooldown == 1\n",  __FILE__, __LINE__);
		summary->Cooldown_avg_heart_rate	 = rt->cooldown.Average_Heart_Rate;
		summary->Cooldown_max_heart_rate = rt->cooldown.Max_HeartRate;	
		summary->Cooldown_average_pace = rt->cooldown.Average_Rpm;
		summary->Cooldown_max_pace = rt->cooldown.Max_Rpm;	
		
		summary->Cooldown_time_elapsed_1000 = rt->cooldown.Time_elapsed_1000;
		summary->Cooldown_time_elapsed = rt->cooldown.Time_elapsed;

		//based on rt->cooldwon_Distance_metri 
		temp_integer = (unsigned long)(rt->cooldown.Distance_metric*MAGF);
		summary->Cooldown_distance_km_i  = temp_integer/MAGF;
		summary->Cooldown_distance_km_f  = temp_integer - summary->Cooldown_distance_km_i*MAGF;
		
		//based on rt->cooldown_Calories_burned
		temp_integer = (unsigned long)(rt->cooldown.Calories_burned);
		summary->Cooldown_calories_burned_1000 = temp_integer/MAGF;
		summary->Cooldown_calories_burned = temp_integer - summary->Cooldown_calories_burned_1000*MAGF;

printf("(%s %d) CalculateSummaryData(7), rt->cooldown.Time_elapsed=%d\n",  __FILE__, __LINE__, rt->cooldown.Time_elapsed + 1000 * rt->cooldown.Time_elapsed_1000);
	}
	else
	{
//printf("(%s %d) rt->summary_state.has_cooldown == 0\n",  __FILE__, __LINE__);
//printf("(%s %d) CalculateSummaryData(8)\n",  __FILE__, __LINE__);

		summary->Cooldown_avg_heart_rate	 = 0;
		summary->Cooldown_max_heart_rate	 = 0;
		summary->Cooldown_average_pace	 = 0;
		summary->Cooldown_max_pace	 = 0;
		//based on rt->cooldwon_Time_elapsed
		summary->Cooldown_time_elapsed_1000 = 0;
		summary->Cooldown_time_elapsed	 = 0;
		//based on rt->cooldwon_Distance_metri 
		summary->Cooldown_distance_km_i	 = 0;
		summary->Cooldown_distance_km_f	 = 0;
		//based on rt->cooldown_Calories_burned
		summary->Cooldown_calories_burned_1000 = 0;
		summary->Cooldown_calories_burned = 0;
	}



	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//Summary: Total part

/*
	ELAPSED TIME
	AVG HR
	MAX HR
	AVG PACE
	MAX PACE
	DISTANCE
	CALORIES
*/

	summary->Total_avg_heart_rate = rt->total.Average_Heart_Rate;
	summary->Total_max_heart_rate = rt->total.Max_HeartRate;	
	summary->Total_average_pace = rt->total.Average_Rpm;
	summary->Total_max_pace = rt->total.Max_Rpm;	

	//based on rt->cooldwon_Time_elapsed
	summary->Total_time_elapsed_1000 = rt->total.Time_elapsed_1000;
	summary->Total_time_elapsed = rt->total.Time_elapsed;

	//based on rt->cooldwon_Distance_metri 
	temp_integer = (unsigned long)(rt->total.Distance_metric*MAGF);
	summary->Total_distance_km_i = temp_integer/MAGF;
	summary->Total_distance_km_f = temp_integer - summary->Total_distance_km_i*MAGF;	

	//based on rt->cooldown_Calories_burned
	temp_integer = (unsigned long)rt->total.Calories_burned;
	summary->Total_calories_burned_1000 = temp_integer/MAGF;
	summary->Total_calories_burned = temp_integer - summary->Total_calories_burned_1000*MAGF;


	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	//pick the max pace
	if(summary->Total_max_pace < summary->Cooldown_max_pace)
		summary->Total_max_pace = summary->Cooldown_max_pace;
	if(summary->Total_max_pace < summary->Workout_max_pace)
		summary->Total_max_pace = summary->Workout_max_pace;
	if(summary->Total_max_pace < summary->Warmup_max_pace)
		summary->Total_max_pace = summary->Warmup_max_pace;

	//pick the max heartrate
	if(summary->Total_max_heart_rate < summary->Cooldown_max_heart_rate)
		summary->Total_max_heart_rate = summary->Cooldown_max_heart_rate;
	if(summary->Total_max_heart_rate < summary->Workout_max_heart_rate)
		summary->Total_max_heart_rate = summary->Workout_max_heart_rate;
	if(summary->Total_max_heart_rate < summary->Warmup_max_heart_rate)
		summary->Total_max_heart_rate = summary->Warmup_max_heart_rate;



//printf("(%s %d) Total_time_elapsed_1000=%d Total_time_elapsed=%d\n",  __FILE__, __LINE__, summary->Total_time_elapsed_1000, summary->Total_time_elapsed);




// 	if(rt->has_cooldown == 1){
// 		summary->Total_avg_heart_rate = (rt->workout.Average_Heart_Rate + rt->cooldown.Average_Heart_Rate)/2;
// 	}
	
// 	if(rt->summary_state.has_cooldown == 1){
// 		summary->Total_average_pace = (rt->workout.Average_Rpm + rt->cooldown.Average_Rpm)/2;
// 	}
	//rt->cooldown.Average_Rpm	 = rt->Average_Rpm;	
}	




#if 0
void CFpcData::Update_GUI_bar(void)
{
	unsigned char i,j;
	unsigned char total_bar_count;
	total_bar_count = rt->total_segment+1;
	
	if(exception->cmd.auto_populate_pace == 1)
	{
		for(i=rt->segment_index; i<rt->workLoad_TableUsed ; i++)
		{
			//pace table
			rt->workPace_Table[i] = update->Pace_RPM;
		}
		rt->workPace_Table[i] = update->Pace_RPM;
	}
	else
	{
		rt->workPace_Table[rt->segment_index] = update->Pace_RPM;
	}


	//if((rt->workLoad_TableUsed+1) >= GUI_window_size){
	if(total_bar_count >= GUI_window_size)
	{
		//all counting from 0
		rt->workPace_Table[rt->segment_index] = update->Pace_RPM;

		if(rt->segment_index > HOLD_STILL_INDEX)
		{
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
		//for(i=rt->GUI_Bar_window_Left,j=0;i <= rt->GUI_Bar_window_Right, j<GUI_window_size;i++,j++){
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
		//total_bar_count < GUI_window_size
		rt->workPace_Table[rt->segment_index] = update->Pace_RPM;
		rt->GUI_Bar_window_index = rt->segment_index;
		update->workload_index = rt->GUI_Bar_window_index;

		for(i=0;i<=rt->workLoad_TableUsed;i++)
		{
			if(exception->cmd.hr_cruise == 0){
				update->Workload_bar[i] = rt->workLoad_Table[i];
			}else{
				update->Workload_bar[i] = rt->workLoad_Table_cruise[i];
			}
			update->Pace_bar[i] = rt->workPace_Table[i];//_barwork_load[i];
		}
	}
	if(exception->cmd.hr_cruise == 1)
	{
		rt->workLoad_Table[rt->segment_index-1] = rt->workLoad_Table_cruise[rt->segment_index-1];
	}else{
		rt->workLoad_Table_cruise[rt->segment_index] = rt->workLoad_Table[rt->segment_index];
	}


	//for Cardio360
	//update->Workload_level = rt->workLoad_Table[rt->segment_index];
}
#else
void CFpcData::Update_GUI_bar(void)
{
	unsigned char i = 0, j = 0;
	unsigned char total_bar_count;
	total_bar_count = rt->total_segment + 1;

	if(exception->cmd.auto_populate_pace == 1)
	{
		for(i = rt->segment_index; i < rt->workLoad_TableUsed; i++)
			rt->workPace_Table[i] = update->Pace_RPM;
		rt->workPace_Table[i] = update->Pace_RPM;
	}
	else
		rt->workPace_Table[rt->segment_index] = update->Pace_RPM;


////////////////////////////////////////////////////
	if(total_bar_count >= GUI_window_size)
	{
		rt->workPace_Table[rt->segment_index] = update->Pace_RPM;

// jason
		if(rt->segment_index > HOLD_STILL_INDEX)
		{
			rt->GUI_Bar_window_Right = rt->segment_index + (GUI_window_size - HOLD_STILL_INDEX - 1);
			if(rt->GUI_Bar_window_Right >= total_bar_count)
			{
				rt->GUI_Bar_window_Right = total_bar_count;
				rt->GUI_Bar_window_Left = rt->GUI_Bar_window_Right - GUI_window_size;
				rt->GUI_Bar_window_index = rt->segment_index - rt->GUI_Bar_window_Left;
			}
			else
			{
				rt->GUI_Bar_window_Left = (rt->GUI_Bar_window_Right + 1) - GUI_window_size;
				rt->GUI_Bar_window_index = HOLD_STILL_INDEX;
			}
		}
		else
		{
			rt->GUI_Bar_window_index = rt->segment_index;
		}

		update->workload_index = rt->GUI_Bar_window_index;


		for(i = rt->GUI_Bar_window_Left, j = 0; j < GUI_window_size; i++, j++)
		{
			if(exception->cmd.hr_cruise == 0)
			{
				if (0 == rt->workLoad_Table[i])
					rt->workLoad_Table[i] = 1;
				update->Workload_bar[j] = rt->workLoad_Table[i];
			}
			else
			{
				if (0 == rt->workLoad_Table_cruise[i])
					rt->workLoad_Table_cruise[i] = 1;

				update->Workload_bar[j] = 	rt->workLoad_Table_cruise[i];
//printf("(%s %d) update->Workload_bar[%d]=%d, rt->workLoad_Table_cruise[%d]=%d\n", __FILE__, __LINE__, j, update->Workload_bar[j], i, rt->workLoad_Table_cruise[i]);

			}
			update->Pace_bar[j] = rt->workPace_Table[i];
		}
		if(j < GUI_window_size - 1)
		{
			update->Workload_bar[j] = 1;
			update->Pace_bar[j] =  0;
		}

		goto HrCruise;
	}




////////////////////////////////////////////////////
	rt->workPace_Table[rt->segment_index] = update->Pace_RPM;
	rt->GUI_Bar_window_index = rt->segment_index;
	update->workload_index = rt->GUI_Bar_window_index;

	for(i = 0; i <= rt->workLoad_TableUsed; i++)
	{
		if(exception->cmd.hr_cruise == 0)
		{
			if (0 == rt->workLoad_Table[i])
				rt->workLoad_Table[i] = 1;
			update->Workload_bar[i] = rt->workLoad_Table[i];
		}
		else
		{
			if (0 == rt->workLoad_Table_cruise[i])
				rt->workLoad_Table_cruise[i] = 1;

			//if (i >= rt->segment_index)
				update->Workload_bar[i] = rt->workLoad_Table_cruise[i];
		}
		update->Pace_bar[i] = rt->workPace_Table[i];
	}



///////////////////////////////////////////////////////////
HrCruise:
	if(exception->cmd.hr_cruise == 1)
	{
		if (0 == rt->workLoad_Table_cruise[rt->segment_index - 1])
			rt->workLoad_Table_cruise[rt->segment_index - 1] = 1;
		rt->workLoad_Table[rt->segment_index - 1] = rt->workLoad_Table_cruise[rt->segment_index - 1];
	}
	else
	{
		if (0 == rt->workLoad_Table[rt->segment_index])
			rt->workLoad_Table[rt->segment_index] = 1;
		rt->workLoad_Table_cruise[rt->segment_index] = rt->workLoad_Table[rt->segment_index];
	}

	//for Cardio360
	//update->Workload_level = rt->workLoad_Table[rt->segment_index];
}


void CFpcData::Update_GUI_bar_Fitness(void)//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
{
	unsigned char i,j;
	unsigned char total_bar_count;
	total_bar_count = rt->total_segment + 1;
	if(exception->cmd.auto_populate_pace == 1)
	{
		for(i = rt->segment_index; i < rt->workLoad_TableUsed; i++)
		{
			//pace table
			rt->workPace_Table[i] = update->Pace_RPM;
		}
		rt->workPace_Table[i] = update->Pace_RPM;
	}
	else
	{
		rt->workPace_Table[rt->segment_index] = update->Pace_RPM;
	}


	if(total_bar_count >= GUI_window_size/*12*/)
	{
		//all counting from 0
		rt->workPace_Table[rt->segment_index] = update->Pace_RPM;

// jason
		if(rt->segment_index > 13/*12*//*HOLD_STILL_INDEX*//*5*/)
		{
	
			rt->GUI_Bar_window_Right/*26*/ = rt->segment_index + (GUI_window_size/*12*/-HOLD_STILL_INDEX/*5*/ - 1);
			
			if(rt->GUI_Bar_window_Right >= total_bar_count/*21*/)
			{
				rt->GUI_Bar_window_Right = total_bar_count/*21*/;
				rt->GUI_Bar_window_Left = rt->GUI_Bar_window_Right - GUI_window_size/*12*/;				
				rt->GUI_Bar_window_index = rt->segment_index - rt->GUI_Bar_window_Left;
				
			}
			else
			{			
				rt->GUI_Bar_window_Left = (rt->GUI_Bar_window_Right/*20*/ +1) - GUI_window_size;
				rt->GUI_Bar_window_index = HOLD_STILL_INDEX;
				
			}//*/
		}
		else
		{
			rt->GUI_Bar_window_index = rt->segment_index;
		}
		
		update->workload_index = rt->GUI_Bar_window_index;
		//for(i=rt->GUI_Bar_window_Left,j=0;i <= rt->GUI_Bar_window_Right, j<GUI_window_size;i++,j++){
		for(i = rt->GUI_Bar_window_Left/*0*/, j = 0; j < GUI_window_size; i++, j++)
		{
			if(exception->cmd.hr_cruise == 0)
			{
				update->Workload_bar[j] = rt->workLoad_Table[i];//_barwork_load[i];
			}
			else
			{
				update->Workload_bar[j] = rt->workLoad_Table_cruise[i];
			}
			update->Pace_bar[j] = rt->workPace_Table[i];
		}
		if(j < GUI_window_size - 1)
		{
			update->Workload_bar[j] = 1;
			update->Pace_bar[j] =  0;
		}

		goto HrCruise;
	}

	rt->workPace_Table[rt->segment_index] = update->Pace_RPM;
	rt->GUI_Bar_window_index = rt->segment_index;
	update->workload_index = rt->GUI_Bar_window_index;

	for(i = 0; i <= rt->workLoad_TableUsed; i++)
	{
		if(exception->cmd.hr_cruise == 0)
		{
			update->Workload_bar[i] = rt->workLoad_Table[i];
		}
		else
		{
			update->Workload_bar[i] = rt->workLoad_Table_cruise[i];
		}

//printf("%d ", rt->workPace_Table[i]);
		update->Pace_bar[i] = rt->workPace_Table[i];//_barwork_load[i];
	}
//printf("\n");


HrCruise:
	if(exception->cmd.hr_cruise == 1)
	{
		rt->workLoad_Table[rt->segment_index - 1] = rt->workLoad_Table_cruise[rt->segment_index - 1];
	}
	else
	{
		rt->workLoad_Table_cruise[rt->segment_index] = rt->workLoad_Table[rt->segment_index];
	}

	//for Cardio360
	//update->Workload_level = rt->workLoad_Table[rt->segment_index];
}
#endif



void CFpcData::Init_GUI_bar_BiPace(struct BiPace *paceTable)
{
	unsigned char i,j;
	rt->GUI_Bar_window_index = 0;	
	rt->GUI_Bar_window_Left = 0;
	rt->GUI_Bar_window_Right = rt->GUI_Bar_window_Left + GUI_window_size-1;

	if(setup->Workload_level == 0)
	{
		setup->Workload_level = 1;
		update->Workload_level = 1;
	}

	//workload table
	for(i=0; i < rt->workLoad_TableUsed ; i++)
	{
		rt->workLoad_Table[i] = setup->Workload_level;
	}
	rt->workLoad_Table[i] = 1;

	//pace table
	j=0;
	for(i = 0; i < rt->workLoad_TableUsed; i++)
	{
		switch(j)
		{
		case 0:
			rt->workPace_Table[i] = paceTable[setup->Workload_level].pace0;
			j = 1;
			break;
		case 1:
			rt->workPace_Table[i] = paceTable[setup->Workload_level].pace1;
			j = 0;
			break;
		}
	}
	rt->BiPaceTable = paceTable;
	rt->pace_adj_mode = INDEXED_TARGET_PACE_ADJ_WLClass2;

	Update_GUI_bar();
}

void CFpcData::Init_GUI_bar(unsigned short segment_count, unsigned short *work_loadTable)
{
	unsigned char i;
	rt->GUI_Bar_window_index = 0;	
	rt->GUI_Bar_window_Left = 0;
	rt->GUI_Bar_window_Right = rt->GUI_Bar_window_Left + GUI_window_size-1;
	
	rt->total_segment = segment_count;	
	rt->workLoad_TableUsed = segment_count;
	
	if(setup->Workload_level == 0){
		setup->Workload_level=1;
		update->Workload_level=1;
	}	
	if(work_loadTable != NULL)
	{
		for(i=0; i < rt->workLoad_TableUsed ; i++){
			rt->workLoad_Table[i] = rt->workLoad_Table_cruise[i] = work_loadTable[i];
		}
		rt->workLoad_Table[i] = 1;
	}else{
		for(i=0; i < rt->workLoad_TableUsed ; i++){
			rt->workLoad_Table[i] = rt->workLoad_Table_cruise[i] = setup->Workload_level;
		}
		rt->workLoad_Table[i] = 1;
	}
	for(i=0; i < rt->workLoad_TableUsed ; i++){
		rt->workPace_Table[i]=0;
	}
	Update_GUI_bar();
}

void CFpcData::InitWorkoutPhase01(unsigned short default_min)
{
	//ASCII Type message set the zero value to be 255
	//release this casting if the message type is binary
	if(setup->Workout_time_1000 == 255)		setup->Workout_time_1000 = 0;
	if(setup->Workout_time == 255)				setup->Workout_time = 0;
	if(setup->Workout_time_1000 == 0)
	{
		if(setup->Workout_time == 0)
		{
			//use FPC default
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
			rt->total_workout_time = setup->Workout_time_1000 * 1000 + setup->Workout_time;
			update->Time_remaining  = setup->Workout_time;
			update->Time_remaining_1000 = setup->Workout_time_1000;
		}
	}
	else
	{

// JC NOTE
// total_workout_time_tick
/*
		rt->total_workout_time = 60 * default_min;
		update->Time_remaining_1000 = rt->total_workout_time / 1000;
		update->Time_remaining = rt->total_workout_time % 1000;

*/

		rt->total_workout_time = setup->Workout_time_1000 * 1000 + setup->Workout_time;
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

void CFpcData::InitWorkoutPhase01_distance(unsigned short default_min)
{
	/*switch(setup->Workout_distance)
	{
	case 50:
		rt->target_workout_distance = 5000;
		break;
	case 100:
		rt->target_workout_distance = 10000;
		break;
	case 32: // 2 miles	 1 Mile = 1.609344 Kilometers.
		rt->target_workout_distance = 2000.0*Mile_KM_ratio;
		break;
	case 64:// 4 miles	 1 Mile = 1.609344 Kilometers.
		rt->target_workout_distance = 4000.0*Mile_KM_ratio;
		break;
	}*/


/*
	update->Time_remaining = 940;
	update->Time_remaining_1000 = 5;
	
	update->Time_elapsed_1000 = 0;
	update->Time_elapsed = 0;	


	rt->total_workout_time = default_min * 60;
	rt->total_workout_time_tick = rt->total_workout_time * FP_TICKS_PER_SECOND;
	
	exception->cmd.pause = 0;
	exception->cmd.stop  = 0;
	exception->cmd.resume  = 0;
	exception->cmd.start  = 1;
	
 	rt->target_workout_distance = (float)setup->Workout_distance;
 	rt->target_workout_distance /= 10;


// jason note
	rt->Distance_metric = 0.00F;
	rt->target_workout_distance = (float)setup->Workout_distance / 1000.00F;
*/



	if(setup->Workout_time_1000 == 255)
		setup->Workout_time_1000 = 0;
	if(setup->Workout_time == 255)
		setup->Workout_time = 0;
	if(setup->Workout_distance == 255)
		setup->Workout_distance = 0;

	rt->total_workout_time = setup->Workout_time_1000 * 1000 + setup->Workout_time;

	//update->Time_remaining_1000 = 99 * 60;
	//update->Time_remaining = 0;
	update->Time_remaining_1000 = 0;
	update->Time_remaining = 0;

	//End with distance achivement
	rt->target_workout_distance = rt->total_workout_time;
	rt->target_workout_distance	 /= 100;

	rt->Distance_metric = 0.00F;
	rt->target_workout_distance = (float)setup->Workout_distance / 1000.00F;

	update->Time_elapsed_1000 = 0;
	update->Time_elapsed = 0;	

	exception->cmd.pause  = 0;
	exception->cmd.stop  = 0;
	exception->cmd.resume  = 0;
	exception->cmd.start  = 1;
	rt->total_workout_time_tick = rt->total_workout_time * FP_TICKS_PER_SECOND;
	rt->exception_result = EXCEPTION_CONTINUE;
}

void CFpcData::InitWorkoutPhase02_hrcd(unsigned char segment_time_min)
{
	unsigned char i;

	rt->segment_index = 0;
	exception->cmd.pause = 0; 
	rt->no_heart_rate_detect = 1000;
	rt->tick_1sec_per100ms_reload = 10;
	rt->tick_1sec_per100ms = rt->tick_1sec_per100ms_reload;
	rt->tick_30sec_reload = FP_TICKS_PER_SECOND * 30;
	rt->tick_30sec = rt->tick_30sec_reload;

	//FPC machine state
	//rt->workout_state = IN_NORMAL;

	if(0 == segment_time_min)
		segment_time_min = 1;
	rt->total_segment = 1 + rt->total_workout_time/(segment_time_min*60);
	rt->workLoad_TableUsed = rt->total_segment;

	if(rt->workLoad_TableUsed > MAX_SEGMENTS)
		rt->workLoad_TableUsed = MAX_SEGMENTS;
	
	if(setup->Workload_level == 255)
		setup->Workload_level = 1;
	rt->workLoad.current_load_level = setup->Workload_level;	
	
	rt->GUI_Bar_window_index = 0;	
	rt->GUI_Bar_window_Left = 0;
	rt->GUI_Bar_window_Right = rt->GUI_Bar_window_Left + GUI_window_size-1;

	if(setup->Workload_level == 0){
		setup->Workload_level=1;
		update->Workload_level=1;
	}	
	
	rt->watt_calc_mod = CALC_BY_CURRENT_LOAD_LEVEL;
	rt->load_adj_mode = SEGMENT_LOAD_ADJ;
	
	for(i=0;i < rt->workLoad_TableUsed;i++){
		if(i==0){
			rt->segmentTime_Table[i] = 60;
		}else{			
			rt->segmentTime_Table[i] = segment_time_min*60;
		}
		rt->workLoad_Table[i]  = rt->workLoad_Table_cruise[i] = setup->Workload_level;
		rt->workPace_Table[i] = 0;
	}
	rt->segmentTime_Table[i] = 120;
	rt->workLoad_Table[i] = 1;
	rt->workPace_Table[i] = 0;
	Update_GUI_bar();
}

void CFpcData::Update_Workout_Time_Elapsed(void)
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

void CFpcData::Update_Workout_Time_Remaining(void)
{	
	if(update->Segment_time > 1)
		update->Segment_time--;
	
	if(update->Time_remaining > 0)
		update->Time_remaining --;
	
	if(update->Time_remaining == 0)
	{
		if(update->Time_remaining_1000 > 0)
		{
			update->Time_remaining_1000--;
			update->Time_remaining = 1000;
		}
	}
}

void CFpcData::UpdateIndexedPaceTable_WLCLASS20(void)
{
	unsigned char i;

	for(i = rt->segment_index; i < rt->workLoad_TableUsed ; i++)
	{
		rt->workPace_Table[i] = rt->WLClass20_PaceTable[rt->Target_Pace_Level - 1].work_pace[i];
	}
}


// jason note
void CFpcData::UpdateCoolDownPace(unsigned char for_program)
{
	float fl;

	switch(for_program)
	{
	default:
	case CUSTOM_PACE:
	case CUSTOM_HILLS:
	case CUSTOM_ULTRA:
	case PERFORMANCE_PACE_RAMP:
	case PERFORMANCE_PACE_INTERVAL:
	case WEIGHT_LOSS_WALK_AND_RUN:
	case WEIGHT_LOSS_LEG_SHAPTER:
		fl = (float)rt->Average_Rpm;
		rt->workPace_Table[rt->total_segment] = (unsigned char)(fl * 0.4);
		//printf("(%s %d) UpdateCoolDownPace(%d %d %f)\n", __FILE__, __LINE__, rt->total_segment, rt->Average_Rpm, fl * 0.4);
		break;
	}
}

void CFpcData::UpdateIndexedPaceTable(void)
{
	unsigned char i;
	unsigned char j = 0;


	for(i = 0; i < rt->workLoad_TableUsed ; i++)
	{
		switch(j)
		{
		case 0:
			j = 1;
			if(i >= rt->segment_index)
			{
				rt->workPace_Table[i] = rt->BiPaceTable[rt->Target_Pace_Level - 1].pace0;
			}
			break;
		case 1:
			j = 0;
			if(i >= rt->segment_index)
			{
				rt->workPace_Table[i] = rt->BiPaceTable[rt->Target_Pace_Level - 1].pace1;
			}
			break;
		}
	}
}

#if 0
float CFpcData::Watts_Calc(void)
{
	float watt;
	unsigned char rpm = lcb->Get_RunRPM();
	watt = 0;
	if(rpm >= 30)
	{
		rpm = (rpm - 30)/10;
		switch(rt->watt_calc_mod){
		case CALC_BY_INDEXED_LOAD_LEVEL:
			//watt = ((float)rpm_level_watt_TS210_Table[rt->indexed_Target_Workload - 1].Watt[rpm]);
			//watt = ((float)Rpm_level_watt_TS210_Table[rt->workLoad_Table[rt->segment_index] - 1].Watt[rpm]);
			watt = tables->Get_Watt(rt->segment_index, rpm);
			break;
		case CALC_BY_CURRENT_LOAD_LEVEL:
			//watt = ((float)rpm_level_watt_TS210_Table[rt->workLoad.current_load_level - 1].Watt[rpm]);
			watt = tables->Get_Watt(rt->workLoad.current_load_level, rpm);
			break;
		}
	}
	return watt;
}
#else
float CFpcData::Watts_Calc(void)
{
	float watt = 0.00F;
	unsigned char rpm = lcb->Get_RunRPM();

	if (rpm == 253 || rpm == 254 || rpm == 255)
		rpm = 0;
	if(rpm < 30)
		return watt;
	rpm = (rpm - 30) / 10;

	switch(rt->watt_calc_mod)
	{
	case CALC_BY_INDEXED_LOAD_LEVEL:
		watt = tables->Get_Watt(rt->segment_index, rpm);
printf("(%s, %d) 1 watt=%f idx=%d rpm=%d\n", __FILE__, __LINE__, watt, rt->segment_index, rpm);
		break;

	case CALC_BY_CURRENT_LOAD_LEVEL:
		watt = tables->Get_Watt(rt->workLoad.current_load_level, rpm);
printf("(%s, %d) 0 watt=%f idx=%d rpm=%d\n", __FILE__, __LINE__, watt, rt->workLoad.current_load_level, rpm);
		break;
	}
	return watt;
}
#endif


unsigned short CFpcData::Watts_Calc_cruise(void)
{
	float watt = 0.00F;

	switch(rt->watt_calc_mod)
	{
	case CALC_BY_INDEXED_LOAD_LEVEL:
		watt = tables->Get_Watt(rt->workLoad_Table[rt->segment_index], RPM60_COL);
		break;
	
	case CALC_BY_CURRENT_LOAD_LEVEL :
		watt = tables->Get_Watt(rt->workLoad_Table[rt->workLoad.current_load_level], RPM60_COL);
		break;
	}
	return watt;
}




/*
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
					rt->workLoad_Table_cruise[i] = rt->workLoad.current_load_level;

				if (rt->watt_calc_mod == CALC_BY_CURRENT_LOAD_LEVEL)
					update->Workload_level = rt->workLoad.current_load_level;
				else
					update->Workload_level = rt->workLoad_Table[rt->segment_index];

				if(rt->base_cruise_watt == 0)
					rt->base_cruise_watt = tables->Get_60rpm_Watt_ByLevel(rt->workLoad_Table_cruise[rt->segment_index]);

				exception->cmd.hr_cruise = 1;
				printf("(%s %d) ENTER HRC CRUISE (%d)\n", __FILE__, __LINE__, update->Target_heart_rate);
			}
*/






void CFpcData::DataCollection(void)
{
	// unsigned short t1,t2;
	//heart rate data is copyied from thisHR structure.
 	if(exception->cmd.hr_cruise == 1)
 	{
 		//update->Target_heart_rate = rt->target_cruise_heart_rate;
 	}
	else
	{
 		//update->Target_heart_rate = setup->Target_heart_rate;
	}

	update->Heart_rate = lcb->hr->DisplayHeartRate;
	update->Stride_length = lcb->data->StrideLength * 5;

	if(setup->Pace_level == 0)
		setup->Pace_level =1;
	update->Pace_level = setup->Pace_level;	


	//t1 = rt->workout_time/10;
	//t2 = rt->elapsed_time/10;
	//update->Time_remaining_1000 = rt->workout_time_1000;
	//update->Time_remaining = rt->workout_time;
	//update->Time_elapsed_1000 = rt->elapsed_time_1000;
	//update->Time_elapsed = rt->elapsed_time;	
}

void CFpcData::InitWorkoutPhase01_mw(void)
{
	//ASCII Type message set the zero value to be 255
	//release this casting if the message type is binary
	if(setup->Workout_time_1000 == 255)
		setup->Workout_time_1000 = 0;
	if(setup->Workout_time == 255)
		setup->Workout_time = 0;
	if(setup->Workout_distance == 255)
		setup->Workout_distance = 0;

	rt->total_workout_time = setup->Workout_time_1000 * 1000 + setup->Workout_time;
	
	//if(setup->Workout_distance == 0)
	if(manual_distance_flag == 0)
	{
		//End with timeout
		update->Time_remaining_1000 = setup->Workout_time_1000;
		update->Time_remaining = setup->Workout_time;

		rt->target_workout_distance  = 0;
	}
	else
	{
		//update->Time_remaining_1000 = 99 * 60;
		//update->Time_remaining = 0;
		update->Time_remaining_1000 = 0;
		update->Time_remaining = 0;

		//End with distance achivement
		rt->target_workout_distance = rt->total_workout_time;
		rt->target_workout_distance	 /= 100;

		rt->Distance_metric = 0.00F;
		rt->target_workout_distance = (float)setup->Workout_distance / 1000.00F;
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

void CFpcData::InitWorkoutPhase02_mw(unsigned short segment_time_min)
{
	unsigned char i;

	rt->segment_index = 0;
	exception->cmd.pause = 0; 
	rt->no_heart_rate_detect = 1000;
	rt->tick_1sec_per100ms_reload = 10;
	rt->tick_1sec_per100ms = rt->tick_1sec_per100ms_reload;
	rt->tick_30sec_reload = FP_TICKS_PER_SECOND * 30;
	rt->tick_30sec = rt->tick_30sec_reload;

	//FPC machine state
	//rt->workout_state = IN_NORMAL;	

	//if(setup->Workout_distance == 0)
	if(manual_distance_flag == 0)
	{
		if (segment_time_min == 0)	segment_time_min = 1;
		rt->total_segment = rt->total_workout_time / (segment_time_min * 60);
	}
	else
	{
		rt->total_segment = GUI_window_size + 1;

		// rt->Distance_metric < rt->target_workout_distance
		//setup->Workout_distance

	}


	rt->workLoad_TableUsed = rt->total_segment;
	if(rt->total_segment > MAX_SEGMENTS)
		rt->total_segment  = MAX_SEGMENTS;
	rt->workLoad_TableUsed = rt->total_segment;

	if(setup->Workload_level == 255)
		setup->Workload_level=1;
	if(setup->Workload_level == 0)
		setup->Workload_level=1;
	rt->workLoad.current_load_level = setup->Workload_level;
	update->Workload_level = rt->workLoad.current_load_level;

	rt->GUI_Bar_window_index = 0;	
	rt->GUI_Bar_window_Left = 0;
	rt->GUI_Bar_window_Right = rt->GUI_Bar_window_Left + GUI_window_size-1;

	//workload table
	rt->WLClass20_LoadTable = NULL;
	for(i = 0; i < rt->workLoad_TableUsed ; i++)
	{
		//workload table
		rt->workLoad_Table[i] = rt->workLoad_Table_cruise[i] = setup->Workload_level;
		//pace table
		rt->workPace_Table[i] = 0;
		//segment time table
		rt->segmentTime_Table[i] = segment_time_min * 60;
	}
	rt->segmentTime_Table[i] = 120;
	rt->workLoad_Table[i] = 1;
	rt->workPace_Table[i] = 0;

	Update_GUI_bar();
}

void CFpcData::GetIndexedWorkload_WLClass2_populate(void)
{
	unsigned char i;

	//get the load and populate to all segments
	for(i=rt->segment_index;i < rt->workLoad_TableUsed;i++){
		switch(i % 2){
		case 0://rest
			rt->workLoad_Table[i] = 
				rt->WLClass2_LoadTable[rt->workLoad.current_load_level-1].work_load[0];
			break;
		case 1://work
			rt->workLoad_Table[i] = 
				rt->WLClass2_LoadTable[rt->workLoad.current_load_level-1].work_load[1];
			break;
		}	
	}
}

void CFpcData::GetIndexedWorkload_WLClass20_Pace(void)
{
	unsigned char i;

	for(i=rt->segment_index; i < rt->workLoad_TableUsed;i++)
	{
		rt->workLoad_Table[i]= rt->WLClass20_LoadTable[rt->workLoad.current_load_level-1].work_load[i];
		rt->workPace_Table[i]= rt->WLClass20_PaceTable[rt->workLoad.current_load_level-1].work_pace[i];
	}
}

void CFpcData::GetIndexedWorkload_WLClass20_single(void)
{
	rt->indexed_Target_Workload = rt->workLoad_Table[rt->segment_index];
}

struct BiPace ppiPaceTable[]= {
//Suggested PACE chart
//REST PACE MAX PACE	   Level
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

unsigned char CFpcData::GetWorkPace_of_ppi(unsigned char level)
{
	return ppiPaceTable[level-1].pace1;
}

unsigned char CFpcData::GetWorkPaceLevel_of_ppi(unsigned char rpm)
{
	unsigned char i;
	for(i=0;i<20;i++){
		if(ppiPaceTable[i].pace1 >= rpm){
			if(ppiPaceTable[i].pace1 == rpm)
				return (i+1);
			if(i<=1){
				return 1;
			}else{
				return(i-1);
			}
		}
	}
	return (20);
}

void CFpcData::GetIndexedWorkload_WLClass20_populate(void)
{
	unsigned char i;
	for(i=rt->segment_index;i<rt->workLoad_TableUsed;i++){
		rt->workLoad_Table[i]=
			rt->WLClass20_LoadTable[rt->workLoad.current_load_level-1].work_load[i];
//  		sprintf(debug,"[%03d]",rt->workLoad_Table[i]);
//  		DisplayString(50,60+i*11, debug);
	}	
}


#if 1
struct WLClass3 Cardio360_LoadTable[]=
{
	//低阻力 	中阻力	高阻力
	//class 
	//0	1,	2
	{{1,	2,	3}},//LV.1	       
	{{2,	3,	4}},//LV.2	       
	{{3,	4,	5}},//LV.3	       
	{{4,	5,	6}},//LV.4	       
	{{5,	6,	7}},//LV.5	       
	{{6,	7,	8}},//LV.6	       
	{{6,	8,	9}},//LV.7	       
	{{7,	8,	9}},//LV.8	       
	{{7,	9,	10}},//LV.9	       
	{{8,	9,	10}},//LV.10	
	{{8,	10,	11}},//LV.11	
	{{9,	10,	11}},//LV.12	
	{{9,	11,	12}},//LV.13	
	{{10,11,	12}},//LV.14	
	{{11,12,	13}},//LV.15	
	{{12,13,	14}},//LV.16	
	{{13,14,	15}},//LV.17	
	{{14,15,	16}},//LV.18	
	{{15,16,	17}},//LV.19	
	{{16,17,	18}},//LV.20	
	{{17,18,	19}},//LV.21	
	{{17,19,	20}},//LV.22	
	{{18,19,	20}},//LV.23	
	{{19,20,	21}},//LV.24	
	{{20,21,	22}},//LV.25	
	{{21,22,	23}},//LV.26	
	{{22,23,	24}},//LV.27	
	{{22,24,	25}},//LV.28	
	{{23,24,	25}},//LV.29	
	{{23,24,	25}},//LV.30	

	//{{23,	19,	21}},//LV.31	
};
#else
	//低阻力 	中阻力	高阻力
	//class 
	//0	1,	2
	{{1,	4,	7}},//LV.1	       
	{{1,	7,	8}},//LV.2	       
	{{1,	8,	9}},//LV.3	       
	{{1,	10,	11}},//LV.4	       
	{{1,	11,	12}},//LV.5	       
	{{4,	11,	12}},//LV.6	       
	{{4,	11,	12}},//LV.7	       
	{{4,	12,	12}},//LV.8	       
	{{4,	12,	13}},//LV.9	       
	{{4,	12,	14}},//LV.10	
	{{7,	12,	14}},//LV.11	
	{{7,	13,	14}},//LV.12	
	{{7,	13,	15}},//LV.13	
	{{7,	13,	15}},//LV.14	
	{{8,	13,	15}},//LV.15	
	{{8,	13,	16}},//LV.16	
	{{8,	14,	16}},//LV.17	
	{{8,	14,	17}},//LV.18	
	{{10,	14,	18}},//LV.19	
	{{10,	14,	18}},//LV.20	
	{{10,	14,	18}},//LV.21	
	{{10,	16,	18}},//LV.22	
	{{11,	16,	18}},//LV.23	
	{{11,	16,	19}},//LV.24	
	{{11,	16,	19}},//LV.25	
	{{11,	16,	19}},//LV.26	
	{{12,	16,	19}},//LV.27	
	{{12,	17,	20}},//LV.28	
	{{12,	18,	20}},//LV.29	
	{{14,	19,	21}},//LV.30	

	//{{14,	19,	21}},//LV.30	
#endif

void CFpcData::GetIndexedWorkload_C360(void)
{
	unsigned char i;

	for(i = rt->segment_index; i < rt->workLoad_TableUsed;i++)
	{
 		rt->workLoad_Table[i] = 
			Cardio360_LoadTable[rt->workLoad.current_load_level - 1].work_load[rt->C360_Table[rt->random_index[i]].work_load_class];
		update->Sports_mode =  rt->C360_Table[rt->random_index[rt->segment_index]].sport_mode;
	}

	rt->indexed_Target_Workload = rt->workLoad_Table[rt->segment_index];
}


void CFpcData::InitWorkoutPhase02_apd(unsigned short segment_time_min)
{

	unsigned char i;

	rt->segment_index = 0;
	exception->cmd.pause = 0; 
	rt->no_heart_rate_detect = 1000;
	rt->tick_1sec_per100ms_reload = 10;
	rt->tick_1sec_per100ms = rt->tick_1sec_per100ms_reload;
	rt->tick_30sec_reload = FP_TICKS_PER_SECOND * 30;
	rt->tick_30sec = rt->tick_30sec_reload;

	//FPC machine state
	//rt->workout_state = IN_NORMAL;

	rt->total_segment = GUI_window_size + 1;

	rt->workLoad_TableUsed = rt->total_segment;
	if(rt->total_segment > MAX_SEGMENTS)
		rt->total_segment  = MAX_SEGMENTS;
	rt->workLoad_TableUsed = rt->total_segment;

	if(setup->Workload_level == 255)
		setup->Workload_level=1;
	if(setup->Workload_level == 0)
		setup->Workload_level=1;
	rt->workLoad.current_load_level = setup->Workload_level;
	update->Workload_level = rt->workLoad.current_load_level;

	rt->GUI_Bar_window_index = 0;	
	rt->GUI_Bar_window_Left = 0;
	rt->GUI_Bar_window_Right = rt->GUI_Bar_window_Left + GUI_window_size - 1;

	//workload table
	rt->WLClass20_LoadTable = NULL;
	for(i = 0; i < rt->workLoad_TableUsed ; i++)
	{
		//workload table
		rt->workLoad_Table[i] = rt->workLoad_Table_cruise[i]= setup->Workload_level;
		//pace table
		rt->workPace_Table[i] = 0;
		//segment time table
		rt->segmentTime_Table[i] = segment_time_min * 60;
	}
	rt->segmentTime_Table[i] = 120;
	rt->workLoad_Table[i] = 1;
	rt->workPace_Table[i] = 0;

	Update_GUI_bar();

/*
	unsigned char i;
	rt->segment_index = 0;
	exception->cmd.pause = 0; 
	rt->no_heart_rate_detect = 1000;
	rt->tick_1sec_per100ms_reload = 10;
	rt->tick_1sec_per100ms = rt->tick_1sec_per100ms_reload;
	rt->tick_30sec_reload = FP_TICKS_PER_SECOND * 30;
	rt->tick_30sec = rt->tick_30sec_reload;

	if(0 == segment_time_min)
		segment_time_min = 1;

	rt->total_segment = rt->total_workout_time / (segment_time_min * 60);
	rt->workLoad_TableUsed = rt->total_segment;


	if(rt->workLoad_TableUsed > MAX_SEGMENTS)
		rt->workLoad_TableUsed = MAX_SEGMENTS;


	if(setup->Workload_level == 255)
		setup->Workload_level = 1;
	rt->workLoad.current_load_level = setup->Workload_level;	


	rt->GUI_Bar_window_index = 0;	
	rt->GUI_Bar_window_Left = 0;
	rt->GUI_Bar_window_Right = rt->GUI_Bar_window_Left + GUI_window_size - 1;

	if(setup->Workload_level == 0)
	{
		setup->Workload_level=1;
		update->Workload_level=1;
	}


	//workload table
	rt->WLClass20_LoadTable = NULL;
	for(i = 0; i < rt->workLoad_TableUsed ; i++)
	{
		rt->workLoad_Table[i] = setup->Workload_level;
		rt->segmentTime_Table[i] = segment_time_min*60;
		rt->workPace_Table[i] = 0;
	}


// ??
// Profile圖形和Manual相似,但是達到目標距離為停止點,因此不會有預設的cool down segment,現在的圖形在第11,12 segment是不正確的
	rt->segmentTime_Table[i] = 120;
	rt->workLoad_Table[i] = 1;
	rt->workPace_Table[i] = 0;

	Update_GUI_bar();

*/

}

void CFpcData::Update_GUI_bar_distance(void)
{
	unsigned char i;

	//all counting from 0
	//update current pace bar
	//if(rt->segment_time_tick==0){
	rt->workPace_Table[rt->segment_index]= update->Pace_RPM;
	if(exception->cmd.segment_shift == 1)
	{
		exception->cmd.segment_shift = 0;
		if(rt->segment_index > HOLD_STILL_INDEX)
		{
			rt->segment_index = HOLD_STILL_INDEX;
			//for(i=0; i<GUI_window_size ;i++){
			for(i=0; i<= rt->segment_index ;i++)
			{
				if(exception->cmd.hr_cruise == 0)
				{
					rt->workLoad_Table[i] = rt->workLoad_Table[i+1];
				}
				else
				{
// jason
					rt->workLoad_Table_cruise[i] = rt->workLoad_Table_cruise[i+1];
					//rt->workLoad_Table_cruise[i] = rt->workLoad_Table_cruise[i+1] = rt->workLoad.current_load_level;
				}
				rt->workPace_Table[i] = rt->workPace_Table[i+1];
			}
			//rt->workLoad_Table_cruise[i] = 1;
			//update->Workload_bar[i] = 1;
			//update->Pace_bar[i] = 0;
		}	
	}	

	if(exception->cmd.auto_populate_pace == 1)
	{
		for(i = rt->segment_index; i<rt->workLoad_TableUsed ; i++)
		{
			//pace table
			rt->workPace_Table[i] = update->Pace_RPM;
		}
		rt->workPace_Table[i] = update->Pace_RPM;
	}
	else
	{
		rt->workPace_Table[rt->segment_index] = update->Pace_RPM;
	}

	for(i = 0; i < GUI_window_size ; i++)
	{
		if(exception->cmd.hr_cruise == 0)
		{
			update->Workload_bar[i] = rt->workLoad_Table[i];
		}
		else
		{
// jason
			update->Workload_bar[i] = rt->workLoad_Table_cruise[i];
			//update->Workload_bar[i] = rt->workLoad_Table_cruise[i] = rt->workLoad.current_load_level;
		}
		update->Pace_bar[i] =  rt->workPace_Table[i];
	}	
	if(exception->cmd.hr_cruise == 1){
		rt->workLoad_Table[rt->segment_index-1] = 
				rt->workLoad_Table_cruise[rt->segment_index-1];
	}else{
		rt->workLoad_Table_cruise[rt->segment_index] =
				rt->workLoad_Table[rt->segment_index];
	}	
	rt->GUI_Bar_window_index = rt->segment_index;
	update->workload_index = rt->GUI_Bar_window_index;
	rt->workPace_Table[rt->segment_index] = update->Pace_RPM;
	update->Workload_level = rt->workLoad_Table[rt->segment_index];
}

unsigned char CFpcData::GetLower10WattWorkLoad_cruise(void)
{
	unsigned char targetLevel;
	unsigned char currentLevel;
	signed short deltaWatt;

	//currentLevel = rt->workLoad_Table[rt->segment_index];
	currentLevel = rt->workLoad_Table_cruise[rt->segment_index];
	targetLevel = currentLevel;
	deltaWatt = 0;
	if(targetLevel >1)
	{
		do
		{
			if(targetLevel == 1)
				break;
			targetLevel --;
			deltaWatt = rt->base_cruise_watt - tables->Get_60rpm_Watt_ByLevel(targetLevel);
			if(targetLevel == 1)
				break;
		}
		while(deltaWatt < 10);

		if(deltaWatt == 10)
		{
			rt->base_cruise_watt -= 10;
		}

		if(deltaWatt > 10)
		{
			if((currentLevel-1) > targetLevel)
				targetLevel++;
			rt->base_cruise_watt -= 10;
		}
		if(deltaWatt < 10)
		{
			if(currentLevel > targetLevel)
				rt->base_cruise_watt -= 10;
		}		
	}

	return targetLevel;
}

unsigned char CFpcData::GetHiger10WattWorkLoad_cruise(void)
{
	unsigned char targetLevel,currentLevel;
	signed short deltaWatt;
	//currentLevel = rt->workLoad_Table[rt->segment_index];
	currentLevel = rt->workLoad_Table_cruise[rt->segment_index];
	targetLevel = currentLevel;
	deltaWatt = 0;
	if(targetLevel <30)
	{
		do
		{
			if(targetLevel == 30)
				break;
			targetLevel ++;
			deltaWatt = tables->Get_60rpm_Watt_ByLevel(targetLevel) - rt->base_cruise_watt;
			if(targetLevel == 30)
				break;
		}while(deltaWatt < 10);
		if(deltaWatt == 10)
		{
			rt->base_cruise_watt += 10;
		}
		if(deltaWatt > 10){
			if((targetLevel-1) > currentLevel)
				targetLevel--;
			rt->base_cruise_watt += 10;
		}
		if(deltaWatt < 10)
		{
			if(targetLevel > currentLevel)rt->base_cruise_watt += 10;
		}		
	}
	return targetLevel;
}

void CFpcData::InitWorkoutPhase02_mqs(unsigned short segment_time_min)
{
	unsigned char i;

	rt->segment_index = 0;
	exception->cmd.pause = 0; 
	rt->no_heart_rate_detect = 1000;		// 10 sec
	rt->tick_1sec_per100ms_reload = 10;
	rt->tick_1sec_per100ms = rt->tick_1sec_per100ms_reload;
	rt->tick_30sec_reload = FP_TICKS_PER_SECOND * 30;

	rt->tick_30sec = rt->tick_30sec_reload;

	//FPC machine state
	//rt->workout_state = IN_NORMAL;

	if (0 == segment_time_min)
		segment_time_min = 1;

	rt->total_segment = rt->total_workout_time / (segment_time_min * 60);
	rt->workLoad_TableUsed = rt->total_segment;

	if(rt->workLoad_TableUsed > MAX_SEGMENTS)
		rt->workLoad_TableUsed  = MAX_SEGMENTS;


	for(i = 0; i < rt->workLoad_TableUsed; i++)
	{
		rt->segmentTime_Table[i] = segment_time_min * 60;
	}

//do {printf("(%s %d) DBG\n", __FILE__, __LINE__);} while(0);


	if(setup->Workload_level == 255)
		setup->Workload_level = 1;
	rt->workLoad.current_load_level = setup->Workload_level;
	
	//bar control
	rt->GUI_Bar_window_index = 0;	
	rt->GUI_Bar_window_Left = 0;
	rt->GUI_Bar_window_Right = rt->GUI_Bar_window_Left + GUI_window_size-1;

	if(setup->Workload_level == 0)
	{
		setup->Workload_level = 1;
		update->Workload_level = 1;
	}

//do {printf("(%s %d) DBG\n", __FILE__, __LINE__);} while(0);
	
	//workload table
	//rt->WLClass20_LoadTable = NULL;
	for(i = 0; i < rt->workLoad_TableUsed ; i++)
	{
		rt->workLoad_Table[i] = rt->workLoad_Table_cruise[i] = setup->Workload_level;
		rt->workPace_Table[i] = 0;
		rt->segmentTime_Table[i] = segment_time_min * 60;
	}

//do {printf("(%s %d) DBG\n", __FILE__, __LINE__);} while(0);


	rt->segmentTime_Table[i] = 60;
	rt->workLoad_Table[i] = 1;
	rt->workPace_Table[i] = 0;
	Update_GUI_bar();

//do {printf("(%s %d) DBG\n", __FILE__, __LINE__);} while(0);

}

void CFpcData::InitWorkoutPhase01_sec(unsigned short default_sec)
{
	//ASCII Type message set the zero value to be 255
	//release this casting if the message type is binary
	if(setup->Workout_time_1000 == 255)setup->Workout_time_1000 = 0;
	if(setup->Workout_time == 255)setup->Workout_time = 0;

	if(setup->Workout_time_1000 == 0){
		if(setup->Workout_time == 0){
			//use FPC default
			if(default_sec > 0){
				rt->total_workout_time = default_sec;
				update->Time_remaining_1000 = rt->total_workout_time/1000;
				update->Time_remaining = rt->total_workout_time%1000;
			}else{
				update->Time_remaining_1000 = 0;
				update->Time_remaining	 = 0;
			
			}
		}else{
			rt->total_workout_time =
				setup->Workout_time_1000*1000 + setup->Workout_time;
			update->Time_remaining 	 = setup->Workout_time;
			update->Time_remaining_1000 = setup->Workout_time_1000;
		}
	}else{
			rt->total_workout_time = setup->Workout_time_1000 * 1000 + setup->Workout_time;
			update->Time_remaining = setup->Workout_time;
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

void CFpcData::InitWorkoutPhase02_C360(unsigned short segment_count, unsigned char for_program, struct Cardio360_exercise *c360_Table)
{
	unsigned short to = 0;
	unsigned char i;
	unsigned char re_random = 0;
	unsigned char r;
	//unsigned char j;
	unsigned char none_zerotime_execise;

	rt->segment_index = 0;
	exception->cmd.pause = 0; 
	rt->no_heart_rate_detect = 1000;
	rt->tick_1sec_per100ms_reload = 10;
	rt->tick_1sec_per100ms = rt->tick_1sec_per100ms_reload;
	rt->tick_30sec_reload = FP_TICKS_PER_SECOND * 30;
	rt->tick_30sec = rt->tick_30sec_reload;

#if 0
	CFpcApp::WorkoutLoopType_WLClass3(unsigned char for_program)
#endif


	rt->total_segment = segment_count;
	rt->workLoad_TableUsed = rt->total_segment;

	none_zerotime_execise = 0;

	for(i = 0; i < 10; i++)
	{
		if(i < segment_count)
		{
			if(setup->Segments_time[i] == 255)
				setup->Segments_time[i] = 0;
			c360_Table[i].customized_time = setup->Segments_time[i];// ccc3_TimeTable[rt->random_index[i]].default_time;
			if(c360_Table[i].customized_time > 0)
				none_zerotime_execise++;
		}
		else
		{
			// c3qs_Table
			c360_Table[i].customized_time = 0;
		}
		rt->random_index[i] = i;
	}


#if 1
	//only for Quick Start
	rt->random_index[0] = 0;
	rt->random_index[9] = 9;
	
	if((for_program == CARDIO360_CUSTOMIZED) || (for_program == CARDIO360_QUICK_START)){
		for(i=1;i<9;i++)
		{
			do{
				r =(unsigned char)(rand()%8)+1;//range 1~8
				re_random = 0;
				/*for(j=1;j<i;j++)
				{
					if(rt->random_index[j] == r)
					{
						re_random = 1;
						break;
					}
				}*/
				if(r == 3)
				{
					printf("r =%d   i = %d \n",r,i);
					re_random = 1;	
				}
				else if(r == 2)
				{
					
					printf("r =%d   i = %d \n",r,i);

					if(i > 7 || rt->random_index[i-1] ==3)
					{
						re_random = 1;	
					}
					else
					{
						
						rt->random_index[i] = r;
						i++;
						r = 3;
					}
					printf("r =%d   i = %d \n",r,i);
						
						
				
				}
				else
				{
					if(rt->random_index[i-1] == r)
					{
					 re_random = 1;
					}
					if( i>7 && rt->random_index[1] == r)
					{
					 re_random = 1;
					}
				}
				
			  }while(re_random == 1);
			rt->random_index[i] = r;
		}
		for(int i=0;i<10;i++)
		{
			printf("rt->random_index[%d] = %d \n",i,rt->random_index[i]);
		}
	}	
#else
	//only for Quick Start
	rt->random_index[0] = 0;
	rt->random_index[9] = 9;
	if(for_program == CARDIO360_CUSTOMIZED || for_program == CARDIO360_QUICK_START)
	{
		i = 1;
RECHK:
		while(i <= 8)
		{
			srand((unsigned)time(NULL));
			r = (unsigned char)(rand() % 8) + 1;
			if (i != 8)
			{
				if (r == 2 || r == 3)
				{
					rt->random_index[i] = 2; i++;
					rt->random_index[i] = 3; i++;
					goto RECHK;
				}
				else
				{
					rt->random_index[i] = r; i++;
				}
			}
			else
			{
RERANDOM:
				if (r == 2 || r == 3)
				{
					srand((unsigned)time(NULL));
					r = (unsigned char)(rand() % 8) + 1;
					if (r == 2 || r == 3)
						goto RERANDOM;
				}
				else
				{
					rt->random_index[i] = r; i++;
				}
			}
		}
	}	
	rt->random_index[0] = 0;
	rt->random_index[9] = 9;
#endif


	for(i = 0; i < rt->workLoad_TableUsed; i++)
	{
		//segment time defined by GUI 
		rt->segmentTime_Table[i] = c360_Table[rt->random_index[i]].customized_time;
	}

	if(setup->Workload_level == 255)
		setup->Workload_level = 1;
	rt->workLoad.current_load_level = setup->Workload_level;
	
	rt->C360_Table = c360_Table;
	rt->watt_calc_mod = CALC_BY_INDEXED_LOAD_LEVEL;
	rt->load_adj_mode = INDEXED_TARGET_LOAD_ADJ_WLClass3;
	
	rt->GUI_Bar_window_index = 0;	
	rt->GUI_Bar_window_Left = 0;
	rt->GUI_Bar_window_Right = rt->GUI_Bar_window_Left + GUI_window_size - 1;

	for(i = 0; i < rt->workLoad_TableUsed; i++)
	{
		rt->workLoad_Table[i] = 
			Cardio360_LoadTable[rt->workLoad.current_load_level - 1].work_load[rt->C360_Table[rt->random_index[i]].work_load_class];

		rt->workPace_Table[i] = 0;
	}
	rt->workLoad_Table[i] = 1;
	rt->workPace_Table[i] = 0;	

	rt->segment_index = 0;
	Update_GUI_bar();	


	for(i = 1; i < rt->workLoad_TableUsed - 1; i++)
	{
		to += (unsigned short)rt->segmentTime_Table[i];
		//printf("(%s %d) rt->segmentTime_Table[%d]=%d\n", __FILE__, __LINE__, i, rt->segmentTime_Table[i]);
	}
	if (to == 0)
		rt->segmentTime_Table[1] = 15;
}

void CFpcData::InitWorkoutPhase02_C360V(unsigned short segment_count)
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

	if(0 == setup->Segments)
		setup->Segments = 1;
	rt->workLoad_TableUsed = setup->Segments;
	rt->total_segment = rt->workLoad_TableUsed;	

	if (rt->workLoad_TableUsed > 0)		rt->segment_time = rt->total_workout_time / rt->workLoad_TableUsed;
	else								rt->workLoad_TableUsed = 1;

	if(setup->Workload_level == 255)
		setup->Workload_level = 1;
	rt->workLoad.current_load_level = setup->Workload_level;

	rt->GUI_Bar_window_index = 0;
	rt->GUI_Bar_window_Left = 0;
	rt->GUI_Bar_window_Right = rt->GUI_Bar_window_Left + GUI_window_size - 1;

	if(setup->Workload_level == 0)
	{
		setup->Workload_level = 1;
		update->Workload_level = 1;
	}	

	rt->WLClass20_LoadTable = NULL;
	for(i = 0; i < rt->workLoad_TableUsed; i++)
	{
		//rt->workLoad_Table[i]  = setup->Workload_level;
		rt->workLoad_Table[i]  = rt->workLoad_Table_cruise[i] = setup->Workload_level;

		rt->segmentTime_Table[i] = rt->segment_time;
		rt->workPace_Table[i]	=0;
	}
	rt->segmentTime_Table[i] = 60;
	rt->workLoad_Table[i] = 1;
	rt->workPace_Table[i] = 0;
	Update_GUI_bar();

}

void CFpcData::InitWorkoutPhase02_C360_V1(unsigned short segment_count, unsigned char for_program, struct Cardio360_exercise *c360_Table)
{
	unsigned char i;
	unsigned char re_random = 0;
	unsigned char r;
	//unsigned char j;
	unsigned char none_zerotime_execise;

	rt->segment_index = 0;
	exception->cmd.pause = 0; 
	rt->no_heart_rate_detect = 1000;
	rt->tick_1sec_per100ms_reload = 10;
	rt->tick_1sec_per100ms = rt->tick_1sec_per100ms_reload;
	rt->tick_30sec_reload = FP_TICKS_PER_SECOND * 30;
	rt->tick_30sec = rt->tick_30sec_reload;

	//FPC machine state
	//rt->workout_state = IN_NORMAL;

	rt->workLoad_TableUsed = rt->total_segment = segment_count;


	none_zerotime_execise = 0;
// 	for(i = 0; i < rt->total_segment; i++)
//{
// 		if(setup->Segments_time[i] == 255)
//			setup->Segments_time[i]=0;
// 		c360_Table[i].customized_time = setup->Segments_time[i];// ccc3_TimeTable[rt->random_index[i]].default_time;
// 		if(c360_Table[i].customized_time > 0)none_zerotime_execise++;
// 		rt->random_index[i] = i;
// 	}
	
	for(i = 0; i < 10; i++)
	{
		if(i < segment_count)
		{
			if(setup->Segments_time[i] == 255)
				setup->Segments_time[i] = 0;

			// ccc3_TimeTable[rt->random_index[i]].default_time;
			c360_Table[i].customized_time = setup->Segments_time[i];

			if(c360_Table[i].customized_time > 0)
				none_zerotime_execise++;
		}else
		{
			c360_Table[i].customized_time = 0;
		}
		rt->random_index[i] = i;
	}


#if 1
	//only for Quick Start
	rt->random_index[0] = 0;
	rt->random_index[9] = 9;

	if((for_program == CARDIO360_CUSTOMIZED) || (for_program == CARDIO360_QUICK_START)){
		for(i=1;i<9;i++)
		{
			do{
				r =(unsigned char)(rand()%8)+1;//range 1~8
				re_random = 0;
				/*for(j=1;j<i;j++)
				{
					if(rt->random_index[j] == r)
					{
						re_random = 1;
						break;
					}
				}*/
				if(r == 3)
				{
					printf("r =%d   i = %d \n",r,i);
					re_random = 1;	
				}
				else if(r == 2)
				{
					
					printf("r =%d   i = %d \n",r,i);

					if(i > 7 || rt->random_index[i-1] ==3)
					{
						re_random = 1;	
					}
					else
					{
						
						rt->random_index[i] = r;
						i++;
						r = 3;
					}
					printf("r =%d   i = %d \n",r,i);
						
						
				
				}
				else
				{
					if(rt->random_index[i-1] == r)
					{
					 re_random = 1;
					}
					if( i>7 && rt->random_index[1] == r)
					{
					 re_random = 1;
					}
				}
				
			  }while(re_random == 1);
			rt->random_index[i] = r;
		}
		for(int i=0;i<10;i++)
		{
			printf("rt->random_index[%d] = %d \n",i,rt->random_index[i]);
		}
	}	
#else
	//only for Quick Start
	rt->random_index[0] = 0;
	rt->random_index[9] = 9;
	if(for_program == CARDIO360_CUSTOMIZED || for_program == CARDIO360_QUICK_START)
	{
		i = 1;
RECHK:
		while(i <= 8)
		{
			srand((unsigned)time(NULL));
			r = (unsigned char)(rand() % 8) + 1;
			if (i != 8)
			{
				if (r == 2 || r == 3)
				{
					rt->random_index[i] = 2; i++;
					rt->random_index[i] = 3; i++;
					goto RECHK;
				}
				else
				{
					rt->random_index[i] = r; i++;
				}
			}
			else
			{
RERANDOM:
				if (r == 2 || r == 3)
				{
					srand((unsigned)time(NULL));
					r = (unsigned char)(rand() % 8) + 1;
					if (r == 2 || r == 3)
						goto RERANDOM;
				}
				else
				{
					rt->random_index[i] = r; i++;
				}
			}
		}
	}	
	rt->random_index[0] = 0;
	rt->random_index[9] = 9;
#endif


	for(i = 0; i < rt->workLoad_TableUsed; i++)
	{
		rt->segmentTime_Table[i] = c360_Table[rt->random_index[i]].customized_time; //segment time defined by GUI 
	}


	if(setup->Workload_level == 255)
		setup->Workload_level = 1;
	rt->workLoad.current_load_level = setup->Workload_level;
	
	rt->C360_Table = c360_Table;
	rt->watt_calc_mod = CALC_BY_CURRENT_LOAD_LEVEL;
	rt->load_adj_mode = SEGMENT_LOAD_ADJ;
	
	rt->GUI_Bar_window_index = 0;	
	rt->GUI_Bar_window_Left = 0;
	rt->GUI_Bar_window_Right = rt->GUI_Bar_window_Left + GUI_window_size-1;
	
	for(i = 0; i < rt->workLoad_TableUsed; i++)
	{
		rt->workLoad_Table[i] = rt->workLoad.current_load_level;
		rt->workPace_Table[i] = 0;	
	}
	rt->workLoad_Table[i] = 1; // for cooldown
	rt->workPace_Table[i] = 0; // for cooldown
	rt->segment_index = 0;

	Update_GUI_bar();	
}

void CFpcData::InitWorkoutPhase02_wlcg(unsigned short segment_time_min)
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
	rt->total_segment = rt->total_workout_time / (segment_time_min * 60);
	rt->workLoad_TableUsed = rt->total_segment;

	if(rt->workLoad_TableUsed > MAX_SEGMENTS)
		rt->workLoad_TableUsed  = MAX_SEGMENTS;
	for(i = 0;i < rt->workLoad_TableUsed; i++)
	{
	}

	if(setup->Workload_level == 255)
		setup->Workload_level=1;
	rt->workLoad.current_load_level = setup->Workload_level;
	//Init_GUI_WLClass20(loadTable, rt->workLoad.current_load_level);

	rt->GUI_Bar_window_index = 0;	
	rt->GUI_Bar_window_Left = 0;
	rt->GUI_Bar_window_Right = rt->GUI_Bar_window_Left + GUI_window_size - 1;

	if(setup->Workload_level == 0)
	{
		setup->Workload_level = 1;
		update->Workload_level = 1;
	}	

	rt->WLClass20_LoadTable = NULL;

	for(i = 0; i < rt->workLoad_TableUsed; i++)
	{
		rt->workLoad_Table[i] = rt->workLoad_Table_cruise[i] = setup->Workload_level;
		rt->workPace_Table[i] = 0;
		rt->segmentTime_Table[i] = segment_time_min * 60;
	}
	rt->segmentTime_Table[i] = 60;
	rt->workLoad_Table[i] = 1;
	rt->workPace_Table[i] = 0;
	Update_GUI_bar();	
}

void CFpcData::InitWorkoutPhase02_wlrh(unsigned short segment_count, struct WLClass20_Load *WLClass20_LoadTable)
{
	unsigned char i;
	rt->segment_index = 0;
	exception->cmd.pause = 0; 
	rt->no_heart_rate_detect = 1000;
	rt->tick_1sec_per100ms_reload = 10;
	rt->tick_1sec_per100ms = rt->tick_1sec_per100ms_reload;
	rt->tick_30sec_reload = FP_TICKS_PER_SECOND * 30;
	rt->tick_30sec = rt->tick_30sec_reload;

	//FPC machine state
	//rt->workout_state = IN_NORMAL;
	
	rt->total_segment = segment_count;	
	rt->workLoad_TableUsed = rt->total_segment;

	rt->segment_time = 
		rt->total_workout_time/rt->workLoad_TableUsed;

	if(setup->Workload_level == 255)setup->Workload_level = 1;
	rt->workLoad.current_load_level=setup->Workload_level;
	
	//Init_GUI_WLClass20(loadTable, rt->workLoad.current_load_level);
	rt->GUI_Bar_window_index = 0;	
	rt->GUI_Bar_window_Left = 0;
	rt->GUI_Bar_window_Right = rt->GUI_Bar_window_Left + GUI_window_size-1;

	if(setup->Workload_level == 255)setup->Workload_level=1;
	rt->workLoad.current_load_level=setup->Workload_level;
	
	//workload table	
	rt->WLClass20_LoadTable = WLClass20_LoadTable;	
	rt->watt_calc_mod = CALC_BY_INDEXED_LOAD_LEVEL;
	rt->load_adj_mode = INDEXED_TARGET_LOAD_ADJ_WLClass20;
	for(i=0; i < rt->workLoad_TableUsed ; i++){
		//rt->workLoad_Table[i] = 
		//	GetLevelFromWatt(loadTable[setup->Workload_level-1].work_load[i]);
		//rt->workLoad_Table[i] = 
		//	Get60RPM_LevelByWatt(WLClass20_Table[setup->Workload_level-1].work_load[i]);
		//by simon@20130429
		rt->workLoad_Table[i] = 
			rt->WLClass20_LoadTable[setup->Workload_level-1].work_load[i];
		rt->workPace_Table[i] = 0;
		rt->segmentTime_Table[i] = rt->segment_time;
	}
	rt->workLoad_Table[i] = 1; //for Cool down segment
	rt->workPace_Table[i] = 0;
	rt->segment_index = 0;
	rt->segmentTime_Table[i] = 60;
	Update_GUI_bar();
}

void CFpcData::InitWorkoutPhase02_segTime_pace(unsigned char segment_time_min, struct BiPace *paceTable)
{
	unsigned char i;

	rt->segment_index = 0;
	exception->cmd.pause = 0;
	rt->no_heart_rate_detect = 1000;
	rt->tick_1sec_per100ms_reload = 10;
	rt->tick_1sec_per100ms = rt->tick_1sec_per100ms_reload;
	rt->tick_30sec_reload = FP_TICKS_PER_SECOND * 30;
	rt->tick_30sec = rt->tick_30sec_reload;

	//FPC machine state
	//rt->workout_state = IN_NORMAL;	

	if (0 == segment_time_min)
		segment_time_min = 1;

	rt->total_segment = rt->total_workout_time / (segment_time_min * 60);
	rt->workLoad_TableUsed = rt->total_segment;
	
	if(setup->Workload_level == 255)
		setup->Workload_level=1;
	rt->workLoad.current_load_level = setup->Workload_level;

	if(setup->Pace_level == 255 || setup->Pace_level == 0)
		setup->Pace_level = 1;
	update->Pace_level = rt->Target_Pace_Level = setup->Pace_level;
	exception->cmd.pace_up = 1;
	
	if(rt->workLoad_TableUsed > MAX_SEGMENTS)
		rt->workLoad_TableUsed  = MAX_SEGMENTS;

// JC NOTE

	for(i = 0; i < rt->workLoad_TableUsed; i++)
	{
		rt->workLoad_Table[i] = rt->workLoad_Table_cruise[i] = rt->workLoad.current_load_level;
		rt->segmentTime_Table[i] = segment_time_min * 60;
		if( (i % 2) ==0)
		{
			rt->workPace_Table[i] = paceTable[setup->Pace_level - 1].pace0;
		}
		else
		{
			rt->workPace_Table[i] = paceTable[setup->Pace_level - 1].pace1;
		}
	}

// JC NOTE
	rt->segmentTime_Table[i] = 60;


	Init_GUI_bar_BiPace(paceTable);
}

void CFpcData::InitWorkoutPhase02_1pre_segTime(unsigned short segment_time_min)
{
	unsigned char i;
	rt->segment_index = 0;
	exception->cmd.pause = 0; 
	rt->no_heart_rate_detect = 1000;
	rt->tick_1sec_per100ms_reload = 10;
	rt->tick_1sec_per100ms = rt->tick_1sec_per100ms_reload;
	rt->tick_30sec_reload = FP_TICKS_PER_SECOND * 30;
	rt->tick_30sec = rt->tick_30sec_reload;

	//FPC machine state
	//rt->workout_state = IN_NORMAL;

	if (0 == segment_time_min)
		segment_time_min = 1;
	rt->total_segment = 1 + rt->total_workout_time / (segment_time_min * 60);

	rt->workLoad_TableUsed = rt->total_segment;

	if(rt->workLoad_TableUsed > MAX_SEGMENTS)
		rt->workLoad_TableUsed = MAX_SEGMENTS;
	
	if(setup->Workload_level == 255)
		setup->Workload_level=1;
	rt->workLoad.current_load_level = setup->Workload_level;	
	
	rt->GUI_Bar_window_index = 0;	
	rt->GUI_Bar_window_Left = 0;
	rt->GUI_Bar_window_Right = rt->GUI_Bar_window_Left + GUI_window_size-1;

	if(setup->Workload_level == 0)
	{
		setup->Workload_level = 1;
		update->Workload_level = 1;
	}	

	for(i = 0; i < rt->workLoad_TableUsed; i++)
	{
		if(i == 0)
		{
			rt->segmentTime_Table[i] = 60;
		}
		else
		{			
			rt->segmentTime_Table[i] = segment_time_min*60;
		}
		rt->workLoad_Table[i] = rt->workLoad_Table_cruise[i] = setup->Workload_level;
		rt->workPace_Table[i] = 0;
	}
	rt->segmentTime_Table[i] = 120;
	rt->workLoad_Table[i] = 1;
	rt->workPace_Table[i] = 0;
	Update_GUI_bar();
}

unsigned char CFpcData::GetLower10WattWorkLoad(void)
{
	unsigned char targetLevel,currentLevel;
	signed short deltaWatt;	
	currentLevel = rt->workLoad_Table[rt->segment_index];
	//currentLevel = rt->workLoad_Table_cruise[rt->segment_index];
	if(rt->base_hrc_watt == 0)rt->base_hrc_watt = tables->Get_60rpm_Watt_ByLevel(currentLevel);
	targetLevel = currentLevel;
	deltaWatt = 0;
	if(targetLevel >1){
		do{
			if(targetLevel == 1)break;
			targetLevel --;
			deltaWatt = rt->base_hrc_watt - tables->Get_60rpm_Watt_ByLevel(targetLevel);
			if(targetLevel == 1)break;
		}while(deltaWatt < 10);
		if(deltaWatt == 10){
			rt->base_hrc_watt -= 10;
		}
		if(deltaWatt > 10){
			if((currentLevel-1) > targetLevel)targetLevel++;
			rt->base_hrc_watt -= 10;
		}
		if(deltaWatt < 10){
			if(currentLevel > targetLevel)rt->base_hrc_watt -= 10;
		}		
	}
	return targetLevel;
}

unsigned char CFpcData::GetHiger10WattWorkLoad(void)
{
	unsigned char targetLevel,currentLevel;
	signed short deltaWatt;
	currentLevel = rt->workLoad_Table[rt->segment_index];
	if(rt->base_hrc_watt == 0)
		rt->base_hrc_watt = tables->Get_60rpm_Watt_ByLevel(currentLevel);
	targetLevel = currentLevel;
	deltaWatt = 0;
	if(targetLevel <30)
	{
		do
		{
			if(targetLevel == 30)
				break;
			targetLevel ++;
			deltaWatt = tables->Get_60rpm_Watt_ByLevel(targetLevel)-rt->base_hrc_watt;
			if(targetLevel == 30)break;
		}
		while(deltaWatt < 10);
		if(deltaWatt == 10)
		{
			rt->base_hrc_watt += 10;
		}
		if(deltaWatt > 10)
		{
			if((targetLevel-1) > currentLevel)targetLevel--;
			rt->base_hrc_watt += 10;
		}
		if(deltaWatt < 10){
			if(targetLevel > currentLevel)rt->base_hrc_watt += 10;
		}		
	}
	return targetLevel;
}

void CFpcData::PopulateWorkLoad(void)
{
	unsigned char i;
	//rest_level = GetRestSegmentLoadLevel_by_level(work_level);	
	for(i=rt->segment_index; i < rt->workLoad_TableUsed; i++){
		rt->workLoad_Table[i] = rt->adjusted_Target_Workload;
	}
	rt->workLoad.current_load_level = rt->workLoad_Table[rt->segment_index];
	update->Workload_level = rt->workLoad.current_load_level;
}

void CFpcData::InitWorkoutPhase02_harci(unsigned char sets)
{
	unsigned char i;
	unsigned short unit_seg_time;
	unsigned short remain_time;

	rt->segment_index = 0;
	exception->cmd.pause = 0; 
	rt->no_heart_rate_detect = 1000;
	rt->tick_1sec_per100ms_reload = 10;
	rt->tick_1sec_per100ms = rt->tick_1sec_per100ms_reload;
	rt->tick_30sec_reload = FP_TICKS_PER_SECOND * 30;
	rt->tick_30sec = rt->tick_30sec_reload;

	//FPC machine state
	if (0 == sets)
		sets = 1;

	rt->total_segment = 1 + (sets * 3);
	rt->workLoad_TableUsed = rt->total_segment;

	//rt->workout_state = IN_NORMAL;
	
	if(setup->Workload_level == 255)
		setup->Workload_level = 1;
	rt->workLoad.current_load_level = setup->Workload_level;

#if 0
0         23   24
	  1368 1380

57 57 ... 57 12
57 57 ... 69 120
#endif // 0


	//todo:整除問題
	unit_seg_time = rt->total_workout_time / (3 * sets);
	//remain_time = rt->total_workout_time - (unit_seg_time * 3 * sets);
	remain_time = rt->total_workout_time % (unit_seg_time * 3 * sets);

	// 0~24, 25
	for(i = 0; i < rt->workLoad_TableUsed; i++)
	{
		if(i == 0)
		{
			rt->segmentTime_Table[i] = 60;
		}
		else
		{
			switch(hi_Table[i - 1].work_type)
			{
			case REST_SEGMENT:
				rt->segmentTime_Table[i] = unit_seg_time * 1;
				break;
			case WORK_SEGMENT:
				rt->segmentTime_Table[i] = unit_seg_time * 1;
				break;
			default:
				break;
			}
		}
		rt->workLoad_Table[i] = rt->workLoad.current_load_level;
		rt->workPace_Table[i] = 0;
	}	
	if(remain_time > 0)
		rt->segmentTime_Table[i - 1] += remain_time;
		
	rt->segmentTime_Table[i] = 120;	//for cool down
	rt->workLoad_Table[i] = 1;
	rt->workPace_Table[i] = 0;

	if(setup->Workload_level == 255)
		setup->Workload_level = 1;
	rt->workLoad.current_load_level = setup->Workload_level;

	rt->GUI_Bar_window_index = 0;	
	rt->GUI_Bar_window_Left = 0;
	rt->GUI_Bar_window_Right = rt->GUI_Bar_window_Left + GUI_window_size - 1;

	Update_GUI_bar();	
}

unsigned char CFpcData::GetRestSegmentLoadLevel_by_level(unsigned char work_level)
{
	float work_wattF;
	unsigned char j;
	unsigned short rest_watt;


printf("(%s %d) DBG\n", __FILE__, __LINE__);

	if(work_level >1)
	{
		work_wattF = tables->Get_60rpm_Watt_ByLevel(work_level);
	}else{
		work_wattF = tables->Get_60rpm_Watt_ByLevel(1);
	}
	rest_watt = (unsigned short)(work_wattF*0.65);
	for(j=work_level;j>0;j--){
		if(tables->Get_60rpm_Watt_ByLevel(j) <= rest_watt)
			break;
	}
	if(j==0)j=1;
	return j;
}

void CFpcData::PopulateWorkLoad_Intervals_hrci3(void)
{
	unsigned char i,work_level,rest_level;	

printf("(%s %d) PopulateWorkLoad_Intervals_hrci3()\n", __FILE__, __LINE__);

	work_level = rt->adjusted_Target_Workload;

	rest_level = GetRestSegmentLoadLevel_by_level(work_level);	
	for(i=rt->segment_index; i<rt->total_segment ; i++)
	{
		if(i==0)
		{
			rt->workLoad_Table[i] = work_level;
		}
		else
		{
			if(i >= rt->segment_index)
			{
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

	//update->Workload_level = rt->workLoad.current_load_level;
}

void CFpcData::InitWorkoutPhase02_pcc(unsigned short segment_count, struct WLClass20_Load *WLClass20_LoadTable, struct WLClass20_Pace *WLClass20_PaceTable)
{
	unsigned char i;
	rt->segment_index = 0;
	exception->cmd.pause = 0; 
	rt->no_heart_rate_detect = 1000;
	rt->tick_1sec_per100ms_reload = 10;
	rt->tick_1sec_per100ms = rt->tick_1sec_per100ms_reload;
	rt->tick_30sec_reload = FP_TICKS_PER_SECOND * 30;
	rt->tick_30sec = rt->tick_30sec_reload;

	//FPC machine state
	//rt->workout_state = IN_NORMAL;
	
	rt->total_segment = segment_count;	
	rt->workLoad_TableUsed = rt->total_segment;

	rt->segment_time = rt->total_workout_time / rt->workLoad_TableUsed;
	
	for(i=0; i < rt->workLoad_TableUsed + 1; i++)
	{
		rt->segmentTime_Table[i] = rt->segment_time;
	}

	if(setup->Workload_level == 255)setup->Workload_level = 1;
	rt->workLoad.current_load_level=setup->Workload_level;
	
	//Init_GUI_WLClass20(loadTable, rt->workLoad.current_load_level);
	rt->GUI_Bar_window_index = 0;	
	rt->GUI_Bar_window_Left = 0;
	rt->GUI_Bar_window_Right = rt->GUI_Bar_window_Left + GUI_window_size-1;

	if(setup->Workload_level == 255)
		setup->Workload_level = 1;
	rt->workLoad.current_load_level = setup->Workload_level;


	//workload table	
	rt->WLClass20_LoadTable = WLClass20_LoadTable;	
	rt->WLClass20_PaceTable = WLClass20_PaceTable;
	rt->pace_adj_mode = INDEXED_TARGET_PACE_ADJ_WLClass20;
	rt->watt_calc_mod = CALC_BY_INDEXED_LOAD_LEVEL;
	rt->load_adj_mode = INDEXED_TARGET_LOAD_ADJ_WLClass20;

	for(i = 0; i < rt->workLoad_TableUsed ; i++)
	{
		//rt->workLoad_Table[i] = GetLevelFromWatt(loadTable[setup->Workload_level-1].work_load[i]);
		//rt->workLoad_Table[i] = Get60RPM_LevelByWatt(WLClass20_Table[setup->Workload_level-1].work_load[i]);

		//by simon@20130429
		rt->workLoad_Table[i] = WLClass20_LoadTable[setup->Workload_level - 1].work_load[i];
		//pace table
		rt->workPace_Table[i] = WLClass20_PaceTable[setup->Workload_level - 1].work_pace[i];
	}
	rt->workLoad_Table[i] = 1; //for Cool down segment
	rt->workPace_Table[i] = 0;	

	rt->segment_index = 0;
	Update_GUI_bar();
}

void CFpcData::InitWorkoutPhase02_pft(unsigned short segment_time_min)
{
	unsigned char i = 0;
	float wt = 0;
	float new_target_watt = 0;
	rt->segment_index = 0;
	exception->cmd.pause = 0; 
	rt->no_heart_rate_detect = 1000;
	rt->tick_1sec_per100ms_reload = 10;
	rt->tick_1sec_per100ms = rt->tick_1sec_per100ms_reload;
	rt->tick_30sec_reload = FP_TICKS_PER_SECOND * 30;
	rt->tick_30sec = rt->tick_30sec_reload;

	if (0 == segment_time_min)
		segment_time_min = 1;
	rt->total_segment = rt->total_workout_time / (segment_time_min * 60);
	rt->workLoad_TableUsed = rt->total_segment;

	if(setup->Workload_level == 255)
		setup->Workload_level = 1;
	rt->workLoad.current_load_level = setup->Workload_level;
	update->Workload_level = rt->workLoad.current_load_level;

	rt->watt_calc_mod = CALC_BY_CURRENT_LOAD_LEVEL;
	rt->load_adj_mode = SEGMENT_LOAD_ADJ;

	new_target_watt = 0;
	if(rt->workLoad_TableUsed > MAX_SEGMENTS)
		rt->workLoad_TableUsed  = MAX_SEGMENTS;
	for(i = 0; i < rt->workLoad_TableUsed; i++)
	{
		wt = (float)setup->Weight * 2.2F;

		rt->segmentTime_Table[i] = segment_time_min * 60;
		switch(i / 3)
		{
		case 0:
			new_target_watt = 25.00F;
			break;
		case 1:
			new_target_watt = 125.00F * powf((float)(wt / 150.00F), (float)0.67F);
			break;
		case 2:
			new_target_watt = 150.00F * powf((float)(wt / 150.00F), (float)0.67F);
			break;
		case 3:
			new_target_watt = 175.00F * powf((float)(wt / 150.00F), (float)0.67F);
			break;
		}

		if (0 == product_type)
			rt->workLoad_Table[i] = rt->workLoad_Table_cruise[i] = tables->Get_60rpm_Level_ByWatt((unsigned short)new_target_watt);
		else
		{
			//rt->workLoad_Table[i] = rt->workLoad_Table_cruise[i] = tables->Get_70rpm_Level_ByWatt((unsigned short)new_target_watt);
			rt->workLoad_Table[i] = rt->workLoad_Table_cruise[i] = tables->Get_60rpm_Level_ByWatt((unsigned short)new_target_watt);
		}

		rt->workPace_Table[i] = 0;
	}


	rt->segmentTime_Table[i] = 120;
	rt->workLoad_Table[i]=1;
	rt->workPace_Table[i]=0;

	rt->GUI_Bar_window_index = 0;
	rt->GUI_Bar_window_Left = 0;
	rt->GUI_Bar_window_Right = rt->GUI_Bar_window_Left + GUI_window_size - 1;

	Update_GUI_bar();
}

float CFpcData::GetNewTargeWatt_stage_hr(unsigned char stage, unsigned char last_stage_hr)
{
	float wt = 0.00F;
	float new_target_watt = 0.00F;

	new_target_watt = 0.00F;
	wt = (float)setup->Weight;

	switch(stage)
	{
	default:
	case 0://warm up statge
		//rt->Target_Watts = 25;
		new_target_watt = (float)25.0F;
		break;

	case 1:
		if(last_stage_hr > 100)
			new_target_watt = (float)50.00F * powf((float)(wt / (float)150.00F), (float)0.67F);
		else if(last_stage_hr >  90 && last_stage_hr <= 100)
				new_target_watt = 75.00F * powf((float)(wt / (float)150.00F), (float)0.67F);
		else if(last_stage_hr > 80 && last_stage_hr <= 90)
			new_target_watt = 100.00F * powf((float)(wt / (float)150.00F), (float)0.67F);
		else //if(last_stage_hr < 80)
			new_target_watt = 125.00F * powf((float)(wt / (float)150.00F), (float)0.67F);
		break;

	case 2:
		if(last_stage_hr > 100)
			new_target_watt = 75.00F * powf((float)(wt/150.00F), (float)0.67F);
		else if(last_stage_hr >  90 && last_stage_hr <= 100)
			new_target_watt = 100.00F * powf((float)(wt / 150.00F), (float)0.67F);
		else if(last_stage_hr > 80 && last_stage_hr <= 90)
			new_target_watt = 125.00F * powf((float)(wt / 150.00F), (float)0.67F);
		else //if(last_stage_hr < 80)
			new_target_watt = 150.00F * powf((float)(wt / 150.00F), (float)0.67F);
		break;

	case 3:
		if(last_stage_hr > 100)
			new_target_watt = 100.00F * powf((float)(wt / 150.00F), (float)0.67F);
		else if(last_stage_hr >  90 && last_stage_hr <= 100)
			new_target_watt = 125.00F * powf((float)(wt / 150.00F), (float)0.67F);
		else if(last_stage_hr > 80 && last_stage_hr <= 90)
			new_target_watt = 150.00F * powf((float)(wt/150.00F), (float)0.67F);
		else //if(last_stage_hr < 80)
			new_target_watt = 175.00F * powf((float)(wt/150.00F), (float)0.67F);
		break;
	}
	return new_target_watt;
}

struct WLClass20_Load wpr_Table[]=
{
	{{50	,50	,54	,54	,58	,58	,62	,62	,65	,65	,65	,65	,62	,62	,58	,58	,54	,54	,50	,50 }},	//1
	{{50	,50	,55	,55	,59	,59	,63	,63	,68	,68	,68	,68	,63	,63	,59	,59	,55	,55	,50	,50 }},	//2
	{{52	,52	,55	,55	,60	,60	,65	,65	,70	,70	,70	,70	,65	,65	,60	,60	,55	,55	,52	,52 }},	//3
	{{52	,52	,56	,56	,61	,61	,66	,66	,73	,73	,73	,73	,66	,66	,61	,61	,56	,56	,52	,52 }},	//4
	{{52	,52	,56	,56	,62	,62	,67	,67	,75	,75	,75	,75	,67	,67	,62	,62	,56	,56	,52	,52 }},	//5
	{{54	,54	,58	,58	,63	,63	,69	,69	,78	,78	,78	,78	,69	,69	,63	,63	,58	,58	,54	,54 }},	//6
	{{56	,56	,61	,61	,66	,66	,72	,72	,80	,80	,80	,80	,72	,72	,66	,66	,61	,61	,56	,56 }},	//7
	{{58	,58	,64	,64	,69	,69	,75	,75	,83	,83	,83	,83	,75	,75	,69	,69	,64	,64	,58	,58 }},	//8
	{{58	,58	,65	,65	,70	,70	,76	,76	,85	,85	,85	,85	,76	,76	,70	,70	,65	,65	,58	,58 }},	//9
	{{60	,60	,65	,65	,72	,72	,80	,80	,88	,88	,88	,88	,80	,80	,72	,72	,65	,65	,60	,60 }},	//10
	{{60	,60	,66	,66	,73	,73	,82	,82	,90	,90	,90	,90	,82	,82	,73	,73	,66	,66	,60	,60 }},	//11
	{{62	,62	,68	,68	,75	,75	,84	,84	,93	,93	,93	,93	,84	,84	,75	,75	,68	,68	,62	,62 }},	//12
	{{64	,64	,70	,70	,78	,78	,85	,85	,95	,95	,95	,95	,85	,85	,78	,78	,70	,70	,64	,64 }},	//13
	{{64	,64	,74	,74	,82	,82	,90	,90	,98	,98	,98	,98	,90	,90	,82	,82	,74	,74	,64	,64 }},	//14
	{{66	,66	,74	,74	,84	,84	,92	,92	,100	,100	,100	,100	,92	,92	,84	,84	,74	,74	,66	,66 }},	//15
	{{66	,66	,75	,75	,84	,84	,94	,94	,103	,103	,103	,103	,94	,94	,84	,84	,75	,75	,66	,66 }},	//16
	{{68	,68	,78	,78	,85	,85	,95	,95	,105	,105	,105	,105	,95	,95	,85	,85	,78	,78	,68	,68 }},	//17
	{{68	,68	,78	,78	,88	,88	,98	,98	,108	,108	,108	,108	,98	,98	,88	,88	,78	,78	,68	,68 }},	//18
	{{70	,70	,80	,80	,90	,90	,100	,100	,110	,110	,110	,110	,100	,100	,90	,90	,80	,80	,70	,70 }},	//19
	{{70	,70	,80	,80	,90	,90	,100	,100	,115	,115	,115	,115	,100	,100	,90	,90	,80	,80	,70	,70 }} 	//20

};

struct WLClass20_Pace ppr_Table[]=
{
	//Pace Chart																		
	//(Suggested RPM)																	
	//Pace Ramp segment																			 	 	 	 	 	 
	//1	2	3	4	5	6	7	8	9	10	11	12	13	14	15	16	17	18	19	20
	{{50	,50	,54	,54	,58	,58	,62	,62	,65	,65	,65	,65	,62	,62	,58	,58	,54	,54	,50	,50 }},	//1
	{{50	,50	,55	,55	,59	,59	,63	,63	,68	,68	,68	,68	,63	,63	,59	,59	,55	,55	,50	,50 }},	//2
	{{52	,52	,55	,55	,60	,60	,65	,65	,70	,70	,70	,70	,65	,65	,60	,60	,55	,55	,52	,52 }},	//3
	{{52	,52	,56	,56	,61	,61	,66	,66	,73	,73	,73	,73	,66	,66	,61	,61	,56	,56	,52	,52 }},	//4
	{{52	,52	,56	,56	,62	,62	,67	,67	,75	,75	,75	,75	,67	,67	,62	,62	,56	,56	,52	,52 }},	//5
	{{54	,54	,58	,58	,63	,63	,69	,69	,78	,78	,78	,78	,69	,69	,63	,63	,58	,58	,54	,54 }},	//6
	{{56	,56	,61	,61	,66	,66	,72	,72	,80	,80	,80	,80	,72	,72	,66	,66	,61	,61	,56	,56 }},	//7
	{{58	,58	,64	,64	,69	,69	,75	,75	,83	,83	,83	,83	,75	,75	,69	,69	,64	,64	,58	,58 }},	//8
	{{58	,58	,65	,65	,70	,70	,76	,76	,85	,85	,85	,85	,76	,76	,70	,70	,65	,65	,58	,58 }},	//9
	{{60	,60	,65	,65	,72	,72	,80	,80	,88	,88	,88	,88	,80	,80	,72	,72	,65	,65	,60	,60 }},	//10
	{{60	,60	,66	,66	,73	,73	,82	,82	,90	,90	,90	,90	,82	,82	,73	,73	,66	,66	,60	,60 }},	//11
	{{62	,62	,68	,68	,75	,75	,84	,84	,93	,93	,93	,93	,84	,84	,75	,75	,68	,68	,62	,62 }},	//12
	{{64	,64	,70	,70	,78	,78	,85	,85	,95	,95	,95	,95	,85	,85	,78	,78	,70	,70	,64	,64 }},	//13
	{{64	,64	,74	,74	,82	,82	,90	,90	,98	,98	,98	,98	,90	,90	,82	,82	,74	,74	,64	,64 }},	//14
	{{66	,66	,74	,74	,84	,84	,92	,92	,100	,100	,100	,100	,92	,92	,84	,84	,74	,74	,66	,66 }},	//15
	{{66	,66	,75	,75	,84	,84	,94	,94	,103	,103	,103	,103	,94	,94	,84	,84	,75	,75	,66	,66 }},	//16
	{{68	,68	,78	,78	,85	,85	,95	,95	,105	,105	,105	,105	,95	,95	,85	,85	,78	,78	,68	,68 }},	//17
	{{68	,68	,78	,78	,88	,88	,98	,98	,108	,108	,108	,108	,98	,98	,88	,88	,78	,78	,68	,68 }},	//18
	{{70	,70	,80	,80	,90	,90	,100	,100	,110	,110	,110	,110	,100	,100	,90	,90	,80	,80	,70	,70 }},	//19
	{{70	,70	,80	,80	,90	,90	,100	,100	,115	,115	,115	,115	,100	,100	,90	,90	,80	,80	,70	,70 }} 	//20

};

void CFpcData::InitWorkoutPhase02_ppr(unsigned short segment_count)
{
	unsigned char i;

	rt->segment_index = 0;

	exception->cmd.pause = 0; 
	rt->no_heart_rate_detect = 1000;
	rt->tick_1sec_per100ms_reload = 10;
	rt->tick_1sec_per100ms = rt->tick_1sec_per100ms_reload;
	rt->tick_30sec_reload = FP_TICKS_PER_SECOND * 30;
	rt->tick_30sec = rt->tick_30sec_reload;

	//FPC machine state
	//rt->workout_state = IN_NORMAL;
	
	if(setup->Segments == 255)setup->Segments = segment_count;

	if(setup->Workload_level == 255)
		setup->Workload_level = 1;
	rt->workLoad.current_load_level = setup->Workload_level;


	if(setup->Pace_level == 255 || setup->Pace_level == 0)
		setup->Pace_level = 1;
	update->Pace_level = rt->Target_Pace_Level = setup->Pace_level;
	exception->cmd.pace_up = 1;



	rt->workLoad_TableUsed = setup->Segments;
	rt->total_segment = rt->workLoad_TableUsed;	
	rt->segment_time = rt->total_workout_time / rt->workLoad_TableUsed;
			
	for(i = 0;i < rt->workLoad_TableUsed;i++)
	{
		rt->segmentTime_Table[i] = rt->segment_time;
		//rt->workLoad_Table[i] = 1;
		//rt->workPace_Table[i] = ppr_Table[rt->workLoad.current_load_level-1].work_pace[i];
		rt->workPace_Table[i] = ppr_Table[setup->Pace_level - 1].work_pace[i];
	}

	rt->pace_adj_mode = INDEXED_TARGET_PACE_ADJ_WLClass20;
	rt->WLClass20_PaceTable = ppr_Table;
				
	rt->segmentTime_Table[i] = 60;
	rt->workLoad_Table[i] = 1;
	rt->workPace_Table[i] = 0;

	rt->GUI_Bar_window_index	= 0;	
	rt->GUI_Bar_window_Left	= 0;
	rt->GUI_Bar_window_Right	= rt->GUI_Bar_window_Left + GUI_window_size-1;

	Update_GUI_bar();
}

void CFpcData::InitWorkoutPhase02_pobh(unsigned short segment_count, struct WLClass20_Load *WLClass20_LoadTable)
{
	unsigned char i;
	unsigned short last_sement_time = 0;

	//all the initialize values
	rt->segment_index = 0;
	exception->cmd.pause = 0; 
	rt->no_heart_rate_detect = 1000;
	rt->tick_1sec_per100ms_reload = 10;
	rt->tick_1sec_per100ms = rt->tick_1sec_per100ms_reload;
	rt->tick_30sec_reload = FP_TICKS_PER_SECOND * 30;
	rt->tick_30sec = rt->tick_30sec_reload;
	//end of all the initialize values
	
	//FPC machine state first state
	//rt->workout_state = IN_NORMAL;	

	if (0 == segment_count)
		segment_count = 1;

	rt->total_segment = segment_count;	
	rt->workLoad_TableUsed = rt->total_segment;

	//for those time that can be remain for last segment
	last_sement_time = 0;
	
	rt->segment_time =  rt->total_workout_time / rt->workLoad_TableUsed;
		
	//rt->total_segment = rt->total_workout_time/(segment_time_min*60);
	//if there is remain segment time
	last_sement_time = rt->total_workout_time - rt->workLoad_TableUsed * rt->segment_time;
	//end of for those time that can be remain for last segment

	//work load
	if(setup->Workload_level == 255)
		setup->Workload_level = 1;
	rt->workLoad.current_load_level = setup->Workload_level;
	rt->WLClass20_LoadTable = WLClass20_LoadTable;
	
	//Init_GUI_WLClass20(loadTable, rt->workLoad.current_load_level);
	rt->GUI_Bar_window_index = 0;	
	rt->GUI_Bar_window_Left = 0;
	rt->GUI_Bar_window_Right = rt->GUI_Bar_window_Left + GUI_window_size - 1;

	if(setup->Workload_level == 255)
		setup->Workload_level = 1;
	rt->workLoad.current_load_level = setup->Workload_level;
	
	//workload table	
	rt->watt_calc_mod = CALC_BY_INDEXED_LOAD_LEVEL;
	rt->load_adj_mode = INDEXED_TARGET_LOAD_ADJ_WLClass20;

	for(i = 0; i < rt->workLoad_TableUsed ; i++)
	{
		//rt->workLoad_Table[i] = GetLevelFromWatt(loadTable[setup->Workload_level-1].work_load[i]);
		//rt->workLoad_Table[i] = Get60RPM_LevelByWatt(WLClass20_Table[setup->Workload_level-1].work_load[i]);
		//by simon@20130429
		rt->workLoad_Table[i] = rt->WLClass20_LoadTable[setup->Workload_level - 1].work_load[i];

// jason
		rt->workLoad_Table_cruise[i] = rt->WLClass20_LoadTable[setup->Workload_level - 1].work_load[i];

		rt->segmentTime_Table[i] = rt->segment_time;
		rt->workPace_Table[i]  = 0;
	}
	if(last_sement_time > 0)
	{
		rt->segmentTime_Table[i - 1] += last_sement_time;
	}

	rt->segmentTime_Table[i] = 120;	//for cool down segment
	rt->workLoad_Table[i] = 1;	//cool down load level = 1
	rt->workPace_Table[i] = 0;	//pace = 0

	Update_GUI_bar();
}

void CFpcData::InitWorkoutPhase02_hi(unsigned short segment_time_min, struct WLClass2_load *WLClass2_LoadTable)
{
	unsigned char i;
	unsigned short last_sement_time;
	
	//all the initialize values
	rt->segment_index = 0;
	exception->cmd.pause = 0; 
	rt->no_heart_rate_detect = 1000;
	rt->tick_1sec_per100ms_reload = 10;
	rt->tick_1sec_per100ms = rt->tick_1sec_per100ms_reload;
	rt->tick_30sec_reload = FP_TICKS_PER_SECOND * 30;
	rt->tick_30sec = rt->tick_30sec_reload;
	//end of all the initialize values
	
	//FPC machine state first state
	//rt->workout_state = IN_NORMAL;	
	
	//for those time that can be remain for last segment
	last_sement_time = 0;
	if (0 == segment_time_min)
		segment_time_min = 1;
	rt->total_segment = rt->total_workout_time/(segment_time_min*60);
	//if there is remain segment time
	last_sement_time = rt->total_workout_time - rt->total_segment * segment_time_min*60;
	if(last_sement_time >0)rt->total_segment++;
	rt->workLoad_TableUsed = rt->total_segment;
	//end of for those time that can be remain for last segment

	//work load
	if(setup->Workload_level == 255)setup->Workload_level=1;
	rt->workLoad.current_load_level = setup->Workload_level;
	rt->WLClass2_LoadTable = WLClass2_LoadTable;

	
	if(rt->workLoad_TableUsed > MAX_SEGMENTS)rt->workLoad_TableUsed  = MAX_SEGMENTS;
	for(i=0;i < rt->workLoad_TableUsed;i++){
		switch(i % 2){
		case 0://Rest Segment
			rt->workLoad_Table[i] = 
				rt->WLClass2_LoadTable[rt->workLoad.current_load_level-1].work_load[0];
			break;
		case 1://Work Segment
			rt->workLoad_Table[i] = 
				rt->WLClass2_LoadTable[rt->workLoad.current_load_level-1].work_load[1];
			break;
		}	
		rt->segmentTime_Table[i] = segment_time_min*60;
		rt->workPace_Table[i] = 0;
	}
	
	if(last_sement_time >0)
		rt->segmentTime_Table[i-1] = last_sement_time;
	
	rt->segmentTime_Table[i] = 120;	//fro cool down segment
	rt->workLoad_Table[i] = 1;	//cool down load level = 1
	rt->workPace_Table[i] = 0;	//pace = 0

	rt->GUI_Bar_window_index = 0;	
	rt->GUI_Bar_window_Left = 0;
	rt->GUI_Bar_window_Right = rt->GUI_Bar_window_Left + GUI_window_size-1;

	Update_GUI_bar();	
}

void CFpcData::InitWorkoutPhase02_cp(void)
{
	unsigned char i;
	unsigned short segment_time_sec;
	unsigned short remain_time;
	rt->segment_index = 0;
	exception->cmd.pause = 0; 
	rt->no_heart_rate_detect = 1000;
	rt->tick_1sec_per100ms_reload = 10;
	rt->tick_1sec_per100ms = rt->tick_1sec_per100ms_reload;
	rt->tick_30sec_reload = FP_TICKS_PER_SECOND * 30;
	rt->tick_30sec = rt->tick_30sec_reload;

	//FPC machine state
	//rt->workout_state = IN_NORMAL;	

	if (0 == setup->Segments)
		setup->Segments = 1;

	rt->total_segment = setup->Segments;
	rt->workLoad_TableUsed = rt->total_segment;

	if (rt->total_segment > 0)
		segment_time_sec = rt->total_workout_time / rt->total_segment;
	else
		segment_time_sec = 60;
	remain_time = rt->total_workout_time - segment_time_sec* rt->total_segment;
	
	if(rt->workLoad_TableUsed > MAX_SEGMENTS)rt->workLoad_TableUsed  = MAX_SEGMENTS;
	for(i=0;i < rt->workLoad_TableUsed;i++){
		rt->segmentTime_Table[i] = segment_time_sec;
		//workload table
		rt->workLoad_Table[i] = setup->Workload[i];
		rt->customized_hill_LevelTable[i] = setup->Workload[i];
		//pace table		
		rt->workPace_Table[i]	 = GetWorkPace_of_ppi(setup->Pace[i]);
		//rt->workPace_Table[i] = GetWorkPace_of_ppi(setup->Pace_level-1);
	}
	if(remain_time)
		rt->segmentTime_Table[i-1] += remain_time;
	
	rt->segmentTime_Table[i] = 120;
	rt->workLoad_Table[i] = 1;
	rt->workPace_Table[i] = 1;

	rt->workLoad.current_load_level=setup->Workload_level=rt->workLoad_Table[0];
	Update_GUI_bar();
}

void CFpcData::InitWorkoutPhase02_ch(void)
{
	unsigned char i;
	unsigned short segment_time_sec;
	unsigned short remain_time;
	rt->segment_index = 0;
	exception->cmd.pause = 0; 
	rt->no_heart_rate_detect = 1000;
	rt->tick_1sec_per100ms_reload = 10;
	rt->tick_1sec_per100ms = rt->tick_1sec_per100ms_reload;
	rt->tick_30sec_reload = FP_TICKS_PER_SECOND * 30;
	rt->tick_30sec = rt->tick_30sec_reload;

	//FPC machine state
	//rt->workout_state = IN_NORMAL;	

	if (0 == setup->Segments)
		setup->Segments = 1;

	rt->total_segment = setup->Segments;
	rt->workLoad_TableUsed = rt->total_segment;

	if (rt->total_segment > 0)
		segment_time_sec = rt->total_workout_time / rt->total_segment;
	else
		segment_time_sec = 60;

	remain_time = rt->total_workout_time - segment_time_sec* rt->total_segment;
	
	if(rt->workLoad_TableUsed > MAX_SEGMENTS)rt->workLoad_TableUsed  = MAX_SEGMENTS;
	for(i=0;i < rt->workLoad_TableUsed;i++){
		rt->segmentTime_Table[i] = segment_time_sec;
		//workload table
		rt->workLoad_Table[i] = setup->Workload[i];
		rt->customized_hill_LevelTable[i] = setup->Workload[i];
		//pace table		
		rt->workPace_Table[i]	 = GetWorkPace_of_ppi(setup->Pace[i]);
		//rt->workPace_Table[i] = GetWorkPace_of_ppi(setup->Pace_level-1);
	}
	if(remain_time)
		rt->segmentTime_Table[i-1] += remain_time;
	rt->segmentTime_Table[i] = 120;
	rt->workLoad_Table[i] = 1;
	rt->workPace_Table[i] = 0;

	rt->workLoad.current_load_level=setup->Workload_level=rt->workLoad_Table[0];
	Update_GUI_bar();
}

void CFpcData::InitWorkoutPhase02_cu(void)
{
	unsigned char i;
	unsigned short unit_segment_time;
	unsigned short remain_time;
	rt->segment_index = 0;
	exception->cmd.pause = 0; 
	rt->no_heart_rate_detect = 1000;
	rt->tick_1sec_per100ms_reload = 10;
	rt->tick_1sec_per100ms = rt->tick_1sec_per100ms_reload;
	rt->tick_30sec_reload = FP_TICKS_PER_SECOND * 30;
	rt->tick_30sec = rt->tick_30sec_reload;

	//FPC machine state
	//rt->workout_state = IN_NORMAL;	

	if(setup->Segments > MAX_SEGMENTS)
		setup->Segments = MAX_SEGMENTS;
	rt->total_segment = setup->Segments;
	rt->workLoad_TableUsed = rt->total_segment;
	unit_segment_time = rt->total_workout_time / rt->total_segment;
	remain_time = rt->total_workout_time - unit_segment_time* rt->total_segment;
	
	if(setup->Workload_level == 255)
		setup->Workload_level=1;

	for(i=0;i < rt->workLoad_TableUsed;i++){
		rt->segmentTime_Table[i] = unit_segment_time;
		rt->workLoad_Table[i] = setup->Workload[i];
		rt->workPace_Table[i] = GetWorkPace_of_ppi(setup->Pace[i]);
	}
	if(remain_time)
		rt->segmentTime_Table[i-1] += remain_time;
	
	rt->segmentTime_Table[i] = 60;
	rt->workLoad_Table[i] = 1;
	rt->workPace_Table[i] = 0;
	
	//rt->workLoad.current_load_level=setup->Workload_level;	
	//Init_GUI_WLClass20(loadTable, rt->workLoad.current_load_level);

	rt->GUI_Bar_window_index = 0;	
	rt->GUI_Bar_window_Left = 0;
	rt->GUI_Bar_window_Right = rt->GUI_Bar_window_Left + GUI_window_size-1;

	if(setup->Workload_level == 0)
	{
		setup->Workload_level=1;
		update->Workload_level=1;
	}	
	Update_GUI_bar();
}

void CFpcData::PopulateBothSegmentLoad(void)
{
	unsigned char i;
	for(i=rt->segment_index;i < rt->workLoad_TableUsed;i++){
		rt->workLoad_Table[i] = rt->workLoad.reached_work_hr_level;
		switch(i){
		case 0:
			rt->workLoad_Table[i] = rt->workLoad.reached_work_hr_level;
			break;
		case 1:
			rt->workLoad_Table[i] = rt->workLoad.reached_work_hr_level;
			break;
		case 2:
			rt->workLoad_Table[i] = rt->workLoad.reached_work_hr_level;
			break;
		default:
			switch(i %2){
			case 1://WORK_SEGMENT
				rt->workLoad_Table[i] = rt->workLoad.reached_work_hr_level;
				break;
			case 0://REST_SEGMENT:
				rt->workLoad_Table[i] = rt->workLoad.reached_work_hr_level;
				break;
			}
			break;
		}
	}
}

void CFpcData::InitWorkoutPhase02_1pre_segSegment_chrci(void)
{
	unsigned char i;
	rt->segment_index = 0;
	exception->cmd.pause = 0; 
	rt->no_heart_rate_detect = 1000;
	rt->tick_1sec_per100ms_reload = 10;
	rt->tick_1sec_per100ms = rt->tick_1sec_per100ms_reload;
	rt->tick_30sec_reload = FP_TICKS_PER_SECOND * 30;
	rt->tick_30sec = rt->tick_30sec_reload;
	rt->tick_10sec_reload = FP_TICKS_PER_SECOND * 10;
	rt->tick_10sec = rt->tick_10sec_reload;
	
	rt->total_segment = 1 + 2 + rt->total_workout_time /60;

	if(setup->Workload_level == 0){
		setup->Workload_level=1;
		update->Workload_level=1;
	}

	if(setup->Workload_level == 255)
		setup->Workload_level=1;
	rt->workLoad.current_load_level = setup->Workload_level;	
	
	if(rt->total_segment < 100)
	{		
		rt->workLoad_TableUsed = rt->total_segment;
		//rt->workout_state = IN_NORMAL;

		for(i=0;i < rt->workLoad_TableUsed;i++){
			rt->segmentTime_Table[i] = 60;
			rt->workLoad_Table[i] = rt->workLoad.current_load_level;
			rt->workPace_Table[i] = 0;
		}
		rt->segmentTime_Table[i] = 120;	//for cool down
		rt->workLoad_Table[i] = 1;
		rt->workPace_Table[i] = 0;
	}

	rt->GUI_Bar_window_index = 0;	
	rt->GUI_Bar_window_Left = 0;
	rt->GUI_Bar_window_Right = rt->GUI_Bar_window_Left + GUI_window_size-1;
	Update_GUI_bar();	
}

void CFpcData::PopulateRestSegmentLoad(void)
{
	unsigned char i;
	for(i=rt->segment_index;i < rt->workLoad_TableUsed;i++){
		switch(i){
		case 0:
			//rt->workLoad_Table[i] = rt->workLoad.reached_work_hr_level;
			break;
		case 1:
			//rt->workLoad_Table[i] = rt->workLoad.reached_work_hr_level;
			break;
		case 2:
			rt->workLoad_Table[i] = rt->workLoad.reached_rest_hr_level;
			break;
		default:
			switch(i%2){
			case 1://WORK_SEGMENT
				//rt->workLoad_Table[i] = rt->workLoad.reached_work_hr_level;
				break;
			case 0://REST_SEGMENT:
				rt->workLoad_Table[i] = rt->workLoad.reached_rest_hr_level;
				break;
			}
			break;
		}		
	}
}

void CFpcData::PopulateWorkSegmentLoad(void)
{
	unsigned char i;
	for(i=rt->segment_index;i < rt->workLoad_TableUsed;i++){
		switch(i){
		case 0:
			rt->workLoad_Table[i] = rt->workLoad.reached_work_hr_level;
			break;
		case 1:
			rt->workLoad_Table[i] = rt->workLoad.reached_work_hr_level;
			break;
		case 2:
			//rt->workLoad_Table[i] = rt->workLoad.reached_work_hr_level;
			break;
		default:
			switch(i %2){
			case 1://WORK_SEGMENT
				rt->workLoad_Table[i] = rt->workLoad.reached_work_hr_level;
				break;
			case 0://REST_SEGMENT:
				//rt->workLoad_Table[i] = rt->workLoad.reached_work_hr_level;
				break;
			}
			break;
		}		
	}
}

void CFpcData::FadInCruiseGUI(void)
{
	unsigned char i;
	
	exception->cmd.hr_cruise = 1;
	//rt->base_cruise_watt = Watts_Calc_cruise();

	//backup settings
	rt->watt_calc_mod_org	 = rt->watt_calc_mod;
	
	rt->before_cruise_setup_level = setup->Workload_level;
	rt->before_cruise_update_level = update->Workload_level;
	rt->before_cruise_current_level = rt->workLoad.current_load_level;
	rt->before_cruise_Target_heart_rate = setup->Target_heart_rate;
	
	//endof backup settings
	
	for(i=rt->segment_index; i < rt->workLoad_TableUsed ; i++){
		rt->workLoad_Table_cruise[i] = rt->workLoad_Table[rt->segment_index];
// 		switch(rt->watt_calc_mod){
// 		case CALC_BY_INDEXED_LOAD_LEVEL:
// 			rt->workLoad_Table_cruise[i] = rt->workLoad_Table[rt->segment_index];
// 			break;
// 		
// 		case CALC_BY_CURRENT_LOAD_LEVEL:
// 			rt->workLoad_Table_cruise[i] = rt->workLoad.current_load_level;
// 			break;
// 		}
	}
	rt->workLoad_Table_cruise[i] = 1;
	//use current normal 
	if(rt->base_cruise_watt == 0){
		rt->base_cruise_watt = 
			tables->Get_60rpm_Watt_ByLevel(rt->workLoad_Table_cruise[rt->segment_index]);	
	}
	//rt->base_cruise_watt	 = Watts_Calc_cruise();
	
	//new setting
	rt->workLoad.current_load_level = rt->workLoad_Table_cruise[rt->segment_index];
	rt->watt_calc_mod		 = CALC_BY_CURRENT_LOAD_LEVEL;
	
	setup->Target_heart_rate = rt->target_cruise_heart_rate;
	update->Workload_level = rt->workLoad.current_load_level;
	Update_GUI_bar();
}


