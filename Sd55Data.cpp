// Sd55Data.cpp: implementation of the CSd55Data class.
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
#include <sys/socket.h>


#include <asm/ioctl.h>
#include <asm/errno.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#include <math.h>


#include "list.h"
#include "Event.h"
#include "Misc.h"
#include "FpcTb.h"
#include "Sd55Data.h"


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



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//extern int pca955_i2c;// = -1;
int SPU_PIN = 0;


// HeartRate
HeartRate::HeartRate()
{
	DisplayHeartRate = 0;
	this->i2c_fd = -1;
	//Init_HeartRate_Data();
}

HeartRate::~HeartRate()
{
}

unsigned char HeartRate::PCA9555_hardware_read(void)
{
	unsigned char v = 0;

	if (-1 == i2c_fd)		{ printf("-1==i2c_fd\n"); return 255;}
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

// Polar_HR
unsigned char HeartRate::HeartRate_TEL_hardware_read(void)
{
	return TEL_PIN;
}

// CHR_Signal_GP
unsigned char HeartRate::HeartRate_HGP_hardware_read(void)
{
	return HGP_PIN;
}

void HeartRate::HeartRate_Process(void)
{
	unsigned char i = 0;
	unsigned char max_index = 0;                      //緩衝區中最大值的序號
	unsigned char min_index = 0;                      //緩衝區中最小值的序號
	unsigned char temp = 0;
	unsigned short hr = 0, deltaHr = 0;

	hr = 0;

	//檢查采集到的無線心率，
	if(state.HR_SAMPLING_VALID_FLAG_TEL == 1){
		if(Times_Valid_TEL < MAX_VALID_TIMES){
			Times_Valid_TEL++;
			if(Times_Lose_TEL)
			{
				//if (Times_Lose_TEL >= 2)
				//	Times_Lose_TEL -= 2;
				//else
					Times_Lose_TEL--;
				//Times_Lose_TEL = 0;
			}
		}
		//memcpy(&HR_Buffer_TEL[0], &HR_Buffer_TEL[1], 3);
		for(i=0;i<3;i++){
			HR_Buffer_TEL[i] = HR_Buffer_TEL[i+1];
		}
		HR_Buffer_TEL[3] = HR_Sampling_TEL;
		//state &= ~(1 << HR_SAMPLING_VALID_FLAG_TEL);
		state.HR_SAMPLING_VALID_FLAG_TEL = 0;
	}

	//檢查采集到的手握心率
	//if(state & (1 << HR_SAMPLING_VALID_FLAG_HGP)){
	if(state.HR_SAMPLING_VALID_FLAG_HGP == 1){
		if(Times_Valid_HGP < MAX_VALID_TIMES){
			Times_Valid_HGP++;
			if(Times_Lose_HGP)
			{
				//if (Times_Lose_HGP >= 2)
				//	Times_Lose_HGP -= 2;
				//else
					Times_Lose_HGP--;
				//Times_Lose_HGP = 0;
			};
		};
		//memcpy(HR_Buffer_HGP, &HR_Buffer_HGP[1],3);
		for(i=0;i<3;i++){
			HR_Buffer_HGP[i] = HR_Buffer_HGP[i+1];
		}

		HR_Buffer_HGP[3] = HR_Sampling_HGP;
		//state &= ~(1 << HR_SAMPLING_VALID_FLAG_HGP); 
		state.HR_SAMPLING_VALID_FLAG_HGP = 0;
	}
  
	//每到1.6秒計算一次心率
	//if((unsigned char)(Tick_100ms - Time_100ms) >= (MAX_CYCLE/100)){
	Count_ticks_per_100ms --;
	if(Count_ticks_per_100ms ==0){
		//Count_ticks_per_100ms = Count_ticks_reload;
		Count_ticks_per_100ms = 16;
		//無線心率
		if(Times_Valid_TEL >= MAX_VALID_TIMES){//計算當前無線心率
			hr = 0;
			max_index = 0;
			min_index = 0;
			temp = HR_Buffer_TEL[0];                                      //為比較是否所有采樣一致准備
			//state |= (1 << ALL_BUFFER_SAME_FLAG_TEL);        //先認為所有Buffer內容相同
			state.ALL_BUFFER_SAME_FLAG_TEL = 1;
				
			//濾波算法為四個采樣中去掉最大最小值后取平均值
			for (i = 0; i < 4; i++){
				hr += HR_Buffer_TEL[i];
				if(HR_Buffer_TEL[i] > HR_Buffer_TEL[max_index]){
					max_index = i;
				};
				if(HR_Buffer_TEL[i] < HR_Buffer_TEL[min_index]){
					min_index = i;
				};
				if(temp != HR_Buffer_TEL[i]){
					//state  &= ~(1 << ALL_BUFFER_SAME_FLAG_TEL);   //檢查是否所有BUFFER完全相同
					state.ALL_BUFFER_SAME_FLAG_TEL = 0;
				};
			};


#if 1
			hr -= (HR_Buffer_TEL[max_index] + HR_Buffer_TEL[min_index]);
			hr /= 2;
#else
			hr -= HR_Buffer_TEL[min_index];
			hr /= 3;
#endif

			//if(state & (1 << ALL_BUFFER_SAME_FLAG_TEL)){
			if(state.ALL_BUFFER_SAME_FLAG_TEL == 1){
				HeartRate_TEL = hr;
			}else{
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
		}else
			if(Times_Lose_TEL >= MAX_LOSE_TIMES){
				for (i=0;i<4;i++){
					HR_Buffer_TEL[i]=0;
				}
				Lose_Temp_TEL++;
				if(Lose_Temp_TEL > MAX_LOSE_TIMES)
				{
					Lose_Temp_TEL = 0;
					HeartRate_TEL = 0;
				}
			};

		if(Times_Lose_TEL < MAX_LOSE_TIMES){
			Times_Lose_TEL++;
		};
		//手握心率
		if(Times_Valid_HGP >= MAX_VALID_TIMES){	//計算當前手握心率
			hr = 0;
			max_index = 0;
			min_index = 0;
			temp = HR_Buffer_HGP[0];//為比較是否所有采樣一致准備
			//state |= (1 << ALL_BUFFER_SAME_FLAG_HGP);        //先認為所有Buffer內容相同
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
					//state  &= ~(1 << ALL_BUFFER_SAME_FLAG_HGP);   //檢查是否所有BUFFER完全相同
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
			//if(abs(hr - HR_Buffer_HGP[min_index]) > 10){	//by Simon
			if(deltaHr > 10){						//by Simon
				if(HR_Buffer_HGP[min_index] > 80)
					hr = HR_Buffer_HGP[min_index];
				else
					hr = HR_Buffer_HGP[max_index];
			} 

			//if(state & (1 << ALL_BUFFER_SAME_FLAG_HGP)){
			if(state.ALL_BUFFER_SAME_FLAG_HGP == 1){
				HeartRate_HGP = hr;
			}else
				if(HeartRate_HGP > hr){		//by Simon
					deltaHr = HeartRate_HGP - hr;
				}else{
					deltaHr = hr - HeartRate_HGP;
				}
				//if(abs(hr - HeartRate_HGP) <= 2){	//by Simon 
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
				if(Lose_Temp_HGP> MAX_LOSE_TIMES)
				{
					Lose_Temp_HGP = 0;
					HeartRate_HGP=0;
				}
			};
		if(Times_Lose_HGP < MAX_LOSE_TIMES){
			Times_Lose_HGP++;
		};

		//手握作假
		// max_index = 0;
		// min_index = 0;
		/* 
		HR_Buffer_temp[Heart_Rate_count] = hr;
		Heart_Rate_count++;
		if(Heart_Rate_count > 4){
			Heart_Rate_count = 0;
			for (i = 0; i < 4; i++){
				hr+=HR_Buffer_temp[i];
				//      if(HR_Buffer_temp[i] < HR_Buffer_temp[min_index])
				//        min_index = i;
			}
			hr/=4;
		}
		*/
		if(DisplayHeartRate > hr)
		{			
		 	deltaHr = DisplayHeartRate - hr;	//by Simon
		}else{
			deltaHr = hr - DisplayHeartRate;	//by Simon
		}
	
		//if(abs(DisplayHeartRate-hr) > 6 ){
		if(deltaHr > 6 )
		{					//by Simon
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
		}
		else
		{
			if(HeartRate_HGP)
			{
				hr = HeartRate_HGP;
				state.hr_ready = 1;//by Simon@2013/0401
				state.TEL_in_use = 0;	//by Simon@2013/0401
			}
			else
			{
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
				// if(hr < Heart_Rate)
				//   Heart_Rate=Heart_Rate-5;
				//  if(hr > Heart_Rate)
			      if(hr < 100 && hr > 60){//(DisplayHeartRate < 80 && hr < 80)
					DisplayHeartRate=hr;
				}else{
					if(hr > 40)
					 DisplayHeartRate=DisplayHeartRate+5;
			      }
			}else
				if(DisplayHeartRate && Heart_Rate_up_Flag < 3 && Heart_Rate_up_Flag != 0){
					//   if(hr < Heart_Rate)
					if(hr < 100 && hr > 60){//(DisplayHeartRate < 80 && hr < 80)
						DisplayHeartRate=hr;
					}else{
						if(hr > 40)
							DisplayHeartRate=DisplayHeartRate-5;
					}
					//   if(hr > Heart_Rate)
					//     Heart_Rate=Heart_Rate+5;
				}else{ 
					DisplayHeartRate = hr;
					Heart_Rate_down_Flag = 0;
					Heart_Rate_up_Flag = 0;
				};
   
		if(Heart_Rate_up_Flag >= 3)
			Heart_Rate_up_Flag = 0;

		if(Heart_Rate_down_Flag >= 3)
		Heart_Rate_down_Flag = 0;
      
		/*
		if(Heart_Rate){
			Heart_Rate = hr;           //暫時這樣，再改
		}else
			Heart_Rate = hr; 
		*/
		/*
		if(!(System_Dat.hr_mode)){
			if(Heart_Rate_TEL){
				Heart_Rate = Heart_Rate_TEL;
			}else{
				Heart_Rate = Heart_Rate_HGP;
			};
		}else{
			if(Heart_Rate_HGP){
				Heart_Rate = Heart_Rate_HGP;
			}else{
				Heart_Rate = Heart_Rate_TEL;
			};
		}
		*/
		//無線优先
		if(HeartRate_TEL){
			DisplayHeartRate = HeartRate_TEL;
		}


		//檢查輸出范圍
		if(DisplayHeartRate > MAX_HEARTRATE)
		{
printf("(%s %d) MAX_HEARTRATE\n", __FILE__, __LINE__);
			DisplayHeartRate = MAX_HEARTRATE;
		}
		else
		{
			if(DisplayHeartRate < MIN_HEARTRATE)
			{
printf("(%s %d) MIN_HEARTRATE\n", __FILE__, __LINE__);
				DisplayHeartRate = 0;
			};
		}
//printf("(%s %d) DisplayHeartRate=%d\n", __FILE__, __LINE__, DisplayHeartRate);


		//thomas平均心率計算
		/*if(Heart_Rate != 0){
			Word_Temp += (word)Heart_Rate;
			Ave_Heart_Rate = (byte) (Word_Temp/2);
			Word_Temp = Ave_Heart_Rate;
		}else{
			Ave_Heart_Rate = Ave_Heart_Rate;
		}*/

		//計算末得到心率次數
		/*
		if(Times_Lose_HGP < MAX_LOSE_TIMES){
			Times_Lose_HGP++;
		};
		if(Times_Lose_HGP >= MAX_LOSE_TIMES){
			Heart_Rate_HGP = 0;
		}*/    
		Times_Valid_TEL=MAX_VALID_TIMES-1;
		Times_Valid_HGP=MAX_VALID_TIMES-1;
			
		//by Simon@2013/03/27	
 		//DisplayHeartRate = 76;//fake hr
// 		state.hr_ready = 1;
// 		state.TEL_in_use = 0;
	}
// //<<<<<<
// #if USER_DEBUG
// 	DisplayHeartRate = 120;
// #endif
// //>>>>>>
} 



void HeartRate::HeartRate_Sampling_TEL(void)	//this function will be ticked from the FPC Shceduler every 1ms.
{
	Sampling_Timer_TEL++;                                 //采樣定時器+1ms
	//采集心率的PIN腳狀態
	Pin_Buffer_TEL <<= 1;
	//if((SAMPLING_PIN & (1 << HR_TEL)) == (VALID_PULSE_TEL << HR_TEL))
	
	//無線心率輸入 硬體 部份
	//hardware read pin state	//by Simon
	if(HeartRate_TEL_hardware_read()){
		Pin_Buffer_TEL++;
	}
	
	//1KHZ的軟件濾波
	if((Pin_Buffer_TEL & BIT0) == (Pin_Buffer_TEL & BIT2)){
		if((Pin_Buffer_TEL & BIT0) != (Pin_Buffer_TEL & BIT1)){
			if(Pin_Buffer_TEL & BIT0){
				Pin_Buffer_TEL |= BIT1;
			}else{
				Pin_Buffer_TEL &= ~BIT1;
			}
		}
	}
	//得到穩定的有效脈寬
	if((Pin_Buffer_TEL >= 0xFC) || ((Pin_Buffer_TEL & 0x3F) == 0x3F)){
		Pulse_Width_TEL++;
	}
	//檢測有效的脈衝沿
	if(HR_Pause_Delay>0){
		HR_Pause_Delay--;
		if(0x3f == Pin_Buffer_TEL){
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
	}else{
		if(0x3f == Pin_Buffer_TEL){
			if(Sampling_Timer_TEL < MIN_CYCLE){
				//state |= (1 << CODING_FLAG_TEL);//在短時間內檢到多于一個脈衝沿時認為是CODING格式
				state.CODING_FLAG_TEL = 1;        		//在短時間內檢到多于一個脈衝沿時認為是CODING格式
			}else{
				if(Sampling_Timer_TEL <= MAX_CYCLE){			//采樣周期符合要求
					//if(state & (1 << CODING_FLAG_TEL)){
					if(state.CODING_FLAG_TEL == 1){
						if((Pulse_Width_TEL >= MIN_PULSE_WIDTH_TEL_CODING) && (Pulse_Width_TEL <= MAX_PULSE_WIDTH_TEL_CODING)){                                                 //CODING時寬度符合要求，則采集有效
							if (Sampling_Timer_TEL > 0)
							{
								HR_Sampling_TEL = 60000 / Sampling_Timer_TEL;
								//state |=  1 << HR_SAMPLING_VALID_FLAG_TEL;
								state.HR_SAMPLING_VALID_FLAG_TEL = 1;
							}
						}
					}else{
						if((Pulse_Width_TEL >= MIN_PULSE_WIDTH_TEL_UNCODING) && (Pulse_Width_TEL <= MAX_PULSE_WIDTH_TEL_UNCODING)){                                                 //UNCODING時寬度符合要求，則采集有效
							if(Sampling_Timer_TEL > 0)
							{
								HR_Sampling_TEL = 60000 / Sampling_Timer_TEL;
								//state |=  1 << HR_SAMPLING_VALID_FLAG_TEL;
								state.HR_SAMPLING_VALID_FLAG_TEL = 1;
							}
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
	//if((SAMPLING_PIN & (1 << HR_HGP)) == (VALID_PULSE_HGP << HR_HGP))
  
	// 有線手握心率輸入 硬體 部份
	//hardware read pin state	//by Simon
	if(HeartRate_HGP_hardware_read())
	{	//by Simon.
		Pin_Buffer_HGP++;
	}
	//1KHZ的軟件濾波
	if((Pin_Buffer_HGP & BIT0) == (Pin_Buffer_HGP & BIT2)){
		if((Pin_Buffer_HGP & BIT0) != (Pin_Buffer_HGP & BIT1)){
			if(Pin_Buffer_HGP & BIT0){
				Pin_Buffer_HGP |= BIT1;
			}else{
				Pin_Buffer_HGP &= ~BIT1;
			}
		}
	}
	//得到穩定的有效脈寬
	if((Pin_Buffer_HGP >= 0xFC) || ((Pin_Buffer_HGP & 0x3F) == 0x3F)){
		Pulse_Width_HGP++;
	}
	//檢測有效的脈衝沿
	if(0x3f == Pin_Buffer_HGP){
		if((Sampling_Timer_HGP >= MIN_CYCLE) && (Sampling_Timer_HGP <= MAX_CYCLE)){
			//采樣周期符合要求
			if((Pulse_Width_HGP >= MIN_PULSE_WIDTH_HGP) && (Pulse_Width_HGP <= MAX_PULSE_WIDTH_HGP)){
				//寬度符合要求，則采集有效
				if (Sampling_Timer_TEL > 0)
				{
					HR_Sampling_HGP = 60000 / Sampling_Timer_HGP;
					//state |=  1 << HR_SAMPLING_VALID_FLAG_HGP;
					state.HR_SAMPLING_VALID_FLAG_HGP = 1;
				}
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
	unsigned char i = 0;

	this->i2c_fd = fd;

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
	
	50,55,60,65,70,75,80,85,90,95,100,105,110,115,120,125,130,135,140,145,150,155,160,165,170,175,180,185,190,195,200

  /*(unsigned char)round( MIN_AS + (0.00F * DELTA_AS) ), 
	(unsigned char)round( MIN_AS + (1.00F * DELTA_AS) ), 
	(unsigned char)round( MIN_AS + (2.00F * DELTA_AS) ), 
	(unsigned char)round( MIN_AS + (3.00F * DELTA_AS) ), 
	(unsigned char)round( MIN_AS + (4.00F * DELTA_AS) ), 
	(unsigned char)round( MIN_AS + (5.00F * DELTA_AS) ), 
	(unsigned char)round( MIN_AS + (6.00F * DELTA_AS) ), 
	(unsigned char)round( MIN_AS + (7.00F * DELTA_AS) ), 
	(unsigned char)round( MIN_AS + (8.00F * DELTA_AS) ), 
	(unsigned char)round( MIN_AS + (9.00F * DELTA_AS) ), 
	(unsigned char)round( MIN_AS + (10.00F * DELTA_AS) ), 
	(unsigned char)round( MIN_AS + (11.00F * DELTA_AS) ), 
	(unsigned char)round( MIN_AS + (12.00F * DELTA_AS) ), 
	(unsigned char)round( MIN_AS + (13.00F * DELTA_AS) ), 
	(unsigned char)round( MIN_AS + (14.00F * DELTA_AS) ), 
	(unsigned char)round( MIN_AS + (15.00F * DELTA_AS) ), 
	(unsigned char)round( MIN_AS + (16.00F * DELTA_AS) ), 
	(unsigned char)round( MIN_AS + (17.00F * DELTA_AS) ), 
	(unsigned char)round( MIN_AS + (18.00F * DELTA_AS) ), 
	(unsigned char)round( MIN_AS + (19.00F * DELTA_AS) ), 
	(unsigned char)round( MIN_AS + (20.00F * DELTA_AS) ), 
	(unsigned char)round( MIN_AS + (21.00F * DELTA_AS) ), 
	(unsigned char)round( MIN_AS + (22.00F * DELTA_AS) ), 
	(unsigned char)round( MIN_AS + (23.00F * DELTA_AS) ), 
	(unsigned char)round( MIN_AS + (24.00F * DELTA_AS) ), 
	(unsigned char)round( MIN_AS + (25.00F * DELTA_AS) ), 
	(unsigned char)round( MIN_AS + (26.00F * DELTA_AS) ), 
	(unsigned char)round( MIN_AS + (27.00F * DELTA_AS) ), 
	(unsigned char)round( MIN_AS + (28.00F * DELTA_AS) ), 
	(unsigned char)round( MIN_AS + (29.00F * DELTA_AS) ), 

	*/
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

/*
// 阻力馬達段位
unsigned char pAsResist_Level_Table[] =	
{
	24,	26,	30,	37,	44,	50,	56,	62,	68,	74, 
	83,	89,	96,	104,	112,	121,	126,	132,	139,	145,
	151,	157,	163,	168,	174,	180,	185,	190,	196,	201
};
*/

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


SD55_Tables::SD55_Tables()
{
	SD55_Resist_Level_Table = pSD55_Resist_Level_Table;
	Stride_Len_BitMap_Table = pStride_Len_BitMap_Table;
}

unsigned char SD55_Tables::Get_ADC_Target(unsigned char level)
{
	if (level == 0)	return 0;
	return SD55_Resist_Level_Table[level - 1];
}

unsigned char SD55_Tables::Get_Stride_Target(unsigned char level)
{
	if (level == 0)	return 0;
	return Stride_Len_BitMap_Table[level - 1];
}

SD55_data::SD55_data()
{
	StrideLength = MIN_STRIDE_LENGTH;
	ResistanceLevel  = MIN_RESISTANCE_LEVEL;
	Drive_SPU_Interval = 0;
}

Communication_control::Communication_control()
{
	state.txInDemand = 0;
	state.txInUse = 0;
	state.ComError = 0;
	state.txNeedResend = 0;
	txRepeat = 0;
	txRollStep = 0;
	Cmd	 = 0;
	comInterval = 500;
	ErrorNo = 0;

	state.New_SPU_Pulse = 0;
	//data->Drive_SPU_Interval = 0;

	txLen = 0;
	txCount = 0;
	rxCount = 0;

	memset(rxBuffer, 0, sizeof(rxBuffer));
	memset(txBuffer, 0, sizeof(txBuffer));
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSd55Data::CSd55Data() : CFpcTb(), HeartRate(), SD55_Tables(), SD55_data(), Communication_control()
{
	data = (struct SD55_data *)this;
	cc = (struct Communication_control *)this;
	hr = (class HeartRate *)this;
	Tables = (class SD55_Tables *)this;
}

CSd55Data::~CSd55Data()
{
}

void CSd55Data::SetSendCmd(unsigned int flag)
{
	cc->Cmd |= flag;
}

unsigned char CSd55Data::Checksum(void)
{
	unsigned char i,checksum;

	checksum = 0;
	for(i=1; i<cc->rxCount; i++){
		checksum += cc->rxBuffer[i];
	}
	checksum &= 0xFF;
	if(checksum != 0)return 0;
	return 1;
}

void CSd55Data::ResetSendCmd(unsigned int flag)
{
	cc->Cmd &= ~flag;
}

unsigned int CSd55Data::CheckSendCmd(unsigned int flag)
{
	return (cc->Cmd & flag);
}

/*
void CSd55Data::Set_Stride_Motor_Position(unsigned char position)
{
	if((position < MIN_STRIDE_LENGTH) || (position > MAX_STRIDE_LENGTH))return;


//printf("position=%d\n", position);
//////////////////
// jason note
	data->StrideLength = position;

	SetSendCmd(SET_STRIDE_POS);
}
*/

unsigned char CSd55Data::Get_Stride_Motor_Position(void)
{
	return data->StrideLength;
}

/*
void CSd55Data::Set_WorkLoad_Motor_Position(unsigned char position)
{
	if((position < MIN_RESISTANCE_LEVEL) || (position > MAX_RESISTANCE_LEVEL))return;
	data->ResistanceLevel = position;
	SetSendCmd(SET_RESIST_POS);
}
*/

unsigned short CSd55Data::Get_RPM(void)
{
	unsigned short temp_rpm = 0;

	if (data->Rpm < 253)
		temp_rpm = data->Rpm;

	return temp_rpm;
	//return (unsigned short)data->Rpm;	//*1.7);	//(word)((float)inBios.Rpm/10.5);
}

unsigned char CSd55Data::Get_RunRPM(void)
{
	unsigned short temp_rpm = 0;

	if (data->Rpm < 253)
		temp_rpm = data->Rpm;

	if(temp_rpm == 0)
	{
// jason note
		return 0;
	}

	if(temp_rpm > MAX_RPM)			// 120
		temp_rpm = MAX_RPM;
	if(temp_rpm < MIN_RPM)			// 30
		temp_rpm = MIN_RPM;
	return (unsigned char)temp_rpm;
}

void CSd55Data::DecInterval(void)
{
	if(!cc->state.txInUse)
	{
		if(cc->comInterval > 0)	cc->comInterval--;
	}
}

void CSd55Data::Init_SPU(void)
{
	unsigned char i;
	for(i=0; i<3; i++)
		data->Rpm_Buffer[i] = 0;
	data->Buffer_Index = 0;
	data->Rpm = 0;
	data->Clear_Rpm_Delay = 0;
	//Clear_Rpm_Delay = Tick_10ms;
}


