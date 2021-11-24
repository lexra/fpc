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
#include <sys/socket.h>


#include <asm/ioctl.h>
#include <asm/errno.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#include "list.h"
#include "Event.h"
#include "Misc.h"
#include "FpcTb.h"


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


struct RpmLevelWatTable pRpm_Level_Watt_Table_Bike[]=
{
//RPM
//   30	40	50	60	70	80	90	100	110	120	// level
	{{2,	 7,	15,	 25,	 36,	 53,	 66,	 82,	 97,	109, 127, 138}},	//   1
	{{3,	 8,	 16,	 28,	 40,	 59,	 74,	 91,	108,	122, 141, 153}},	//   2
	{{3,	 9,	 18,	 31,	 44,	 66,	 82,	 101,119,135, 155, 169}},	//   3
	{{3,	 10,	 20,	 34,	 48,	 72,	 90,	111,	130,	148, 169, 185}},	//   4
	{{3,	 10,	 21,	 37,	 52,	 78,	 98,	120,	141,	161, 183, 201}},	//   5
	{{3,	 12,	 24,	 41,	 59,	 89,	112,	137,	160,	183, 208, 228}},	//   6
	{{4,	 13,	 27,	 46,	 66,	 101,125,	153,	179,	204, 233, 256}},	//   7
	{{4,	 14,	 30,	 51,	 73,	112,	138,	169,	197, 225, 258, 283}},	//   8
	{{4,	 16,	 32,	 56,	 80,	123,	152,	185,	216,	247, 283, 311}},	//   9
	{{5,	 17,	 35,	 60,	 88,	134,	165,	201,	235,	268,308, 338}},	//  10
	{{5,	 19,	 39,	 67,	98,	149,	184,	225,	262,	298, 343, 376}},	//  11
	{{6,	 21,	 43,	 74,	108,	165,	203,	249,	290,	329, 378, 415}},	//  12
	{{6,	 23,	 47,	81,	118,	180,	222,	272,	317,	360, 413, 453}},	//  13
	{{7,	 25,	 52,	88,	128,	196,	240,	296,	344,	390, 448, 491}},	//  14
	{{7,	 27,	 56,	95,	138,	211,	259,	319,	372,	421, 483, 529}},	//  15
	{{8,	 29,	61,	105,	152,	228,	282,	348,	405,	458, 525, 575}},	//  16
	{{9,	 32,	67,	114,	166,	245,	306,	376,	438,	496, 566, 622}},	//  17
	{{10, 35,	73,	124,	180,	262,	329,	404,	471,	533, 608, 668}},	//  18
	{{10, 37,	78,	133,	194,	279,	352,	433,	504,	570, 649, 714}},	//  19
	{{11,40,	84,	143,	208,	296,	375,	461,	537,	608,690, 761}},	//  20
	{{11,41,	86,	146,	211,	299,	379,	464,	539,	611, 694, 764}},	//  21
	{{12,43,	88,	149,	214,	301,	382,	467,	541,	614, 697, 767}},	//  22
	{{12,44,	91,	152,	217,	304,	385,	470,	544,	617, 700, 770}},	//  23
	{{12,45,	93,	155,	220,	307,	388,	473,	546,	619, 703, 774}},	//  24
	{{13,46,	95,	159,	223,	309,	391,	476,	548,	622, 706, 777}},	//  25

	{{13,46,	95,	159,	223,	309,	391,	476,	548,	622, 706, 777}},	//  25
	{{13,46,	95,	159,	223,	309,	391,	476,	548,	622, 706, 777}},	//  25
	{{13,46,	95,	159,	223,	309,	391,	476,	548,	622, 706, 777}},	//  25
	{{13,46,	95,	159,	223,	309,	391,	476,	548,	622, 706, 777}},	//  25
	{{13,46,	95,	159,	223,	309,	391,	476,	548,	622, 706, 777}},	//  25

	{{13,46,	95,	159,	223,	309,	391,	476,	548,	622, 706, 777}},	//  25
	{{13,46,	95,	159,	223,	309,	391,	476,	548,	622, 706, 777}},	//  25
	{{13,46,	95,	159,	223,	309,	391,	476,	548,	622, 706, 777}},	//  25
	{{13,46,	95,	159,	223,	309,	391,	476,	548,	622, 706, 777}},	//  25
	{{13,46,	95,	159,	223,	309,	391,	476,	548,	622, 706, 777}},	//  25
	{{13,46,	95,	159,	223,	309,	391,	476,	548,	622, 706, 777}},	//  25
	{{13,46,	95,	159,	223,	309,	391,	476,	548,	622, 706, 777}},	//  25
	{{13,46,	95,	159,	223,	309,	391,	476,	548,	622, 706, 777}},	//  25
	{{13,46,	95,	159,	223,	309,	391,	476,	548,	622, 706, 777}},	//  25
	{{13,46,	95,	159,	223,	309,	391,	476,	548,	622, 706, 777}},	//  25
};


struct RpmLevelWatTable pRpm_Level_Watt_Table[]=
{
//RPM 30   40	   50   	 60	        70	        80	        90	       100	      110	       120	// level
	{{13, 13, 13,	 20,	 29,	 40,	 53,	 62,	 75,	 86,	 99,	109}},	//   1
	{{13, 13, 13,	 21,	 30,	 42,	 54,	 64,	 76,	 87,	100,	111}},	//   2
	{{14, 14, 14,	 22,	 32,	 45,	 56,	 69,	 82,	 93,	105,	117}},	//   3
	{{15, 15, 15,	 24,	 35,	 48,	 62,	 74,	 87,	 99,	116,	128}},	//   4
	{{16, 16, 16,	 25,	 37,	 52,	 65,	 78,	 91,	105,	125,	141}},	//   5
	{{17, 17, 17,	 27,	 40,	 56,	 69,	 85,	102,	117,	134,	151}},	//   6
	{{18, 18, 18,	 29,	 43,	 60,	 76,	 92,	113,	123,	141,	159}},	//   7
	{{19, 19, 19,	 31,	 47,	 66,	 83,	100,	125,	133,	157,	174}},	//   8
	{{21, 21, 21,	 34,	 50,	 72,	 90,	110,	134,	152,	171,	192}},	//   9
	{{23, 23, 23,	 37,	 55,	 79,	 99,	121,	145,	163,	188,	207}},	//  10
	{{26, 26, 26,	 42,	 62,	 88,	111,	135,	163,	187,	212,	235}},	//  11
	{{28, 28, 28,	 47,	 68,	 96,	122,	148,	177,	201,	229,	256}},	//  12
	{{31, 31, 31,	 51,	 76,	107,	136,	164,	195,	223,	257,	283}},	//  13
	{{35, 35, 35,	 56,	 84,	116,	149,	181,	222,	251,	281,	310}},	//  14
	{{38, 38, 38,	 63,	 94,	133,	167,	202,	245,	279,	315,	348}},	//  15
	{{44, 44, 44,	 73,	108,	151,	193,	234,	282,	316,	364,	403}},	//  16
	{{50, 50, 50,	 82,	122,	171,	218,	266,	317,	357,	409,	451}},	//  17
	{{54, 54, 54,	 88,	138,	181,	233,	281,	335,	379,	434,	475}},	//  18
	{{61, 61, 61,	 94,	155,	205,	264,	319,	373,	421,	483,	531}},	//  19
	{{68, 68, 68,	111,	175,	229,	295,	354,	415,	467,	533,	582}},	//  20
	{{75, 75, 75,	124,	194,	252,	326,	391,	460,	519,	576,	636}},	//  21
	{{78, 78, 78,	133,	206,	266,	347,	418,	494,	553,	613,	678}},	//  22
	{{83, 83, 83,	138,	213,	274,	358,	429,	507,	572,	631,	704}},	//  23
	{{84, 84, 84,	141,	217,	278,	362,	431,	509,	573,	641,	714}},	//  24
	{{85, 85, 85,	145,	221,	280,	364,	434,	514,	581,	645,	719}},	//  25

	{{85, 85, 85,	145,	221,	280,	364,	434,	514,	581,	645,	719}},	//  25
	{{85, 85, 85,	145,	221,	280,	364,	434,	514,	581,	645,	719}},	//  25
	{{85, 85, 85,	145,	221,	280,	364,	434,	514,	581,	645,	719}},	//  25
	{{85, 85, 85,	145,	221,	280,	364,	434,	514,	581,	645,	719}},	//  25
	{{85, 85, 85,	145,	221,	280,	364,	434,	514,	581,	645,	719}},	//  25

	{{85, 85, 85,	145,	221,	280,	364,	434,	514,	581,	645,	719}},	//  25
	{{85, 85, 85,	145,	221,	280,	364,	434,	514,	581,	645,	719}},	//  25
	{{85, 85, 85,	145,	221,	280,	364,	434,	514,	581,	645,	719}},	//  25
	{{85, 85, 85,	145,	221,	280,	364,	434,	514,	581,	645,	719}},	//  25
	{{85, 85, 85,	145,	221,	280,	364,	434,	514,	581,	645,	719}},	//  25
	{{85, 85, 85,	145,	221,	280,	364,	434,	514,	581,	645,	719}},	//  25
	{{85, 85, 85,	145,	221,	280,	364,	434,	514,	581,	645,	719}},	//  25
	{{85, 85, 85,	145,	221,	280,	364,	434,	514,	581,	645,	719}},	//  25
	{{85, 85, 85,	145,	221,	280,	364,	434,	514,	581,	645,	719}},	//  25
	{{85, 85, 85,	145,	221,	280,	364,	434,	514,	581,	645,	719}},	//  25
};


/*
91以上    27
85-90   26
80-84   25
75-79   24
70-74   23
65-69   22
60-64   21
55-59   20
50-54   19
45-49   18
40-44   17
40以下    16

*/


struct AutoStrideTable pAuto_Stride_Table[]=
{

#if 1
	{39,	32},
	{44,	34},
	{49,	36},
	{54,	38},
	{59,	40},
	{64,	42},
	{69,	44},
	{74,	46},
	{79,	48},
	{84,	50},
	{90,	52},
	{91,	54}
#else
	{39,	16},
	{44,	17},
	{49,	18},
	{54,	19},
	{59,	20},
	{64,	21},
	{69,	22},
	{74,	23},
	{79,	24},
	{84,	25},
	{90,	26},
	{91,	27},
#endif

};


//////////////////////////////////////////////////////////////////////
// routines
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

extern int product_type;




CFpcTb::CFpcTb()
{
	if (0 == product_type)
		Rpm_Level_Watt_Table = pRpm_Level_Watt_Table;
	else //if (1 == product_type)
		Rpm_Level_Watt_Table = pRpm_Level_Watt_Table_Bike;

	AutoSride_Table = pAuto_Stride_Table;
}

CFpcTb::~CFpcTb()
{
}


//////////////////////////////////////////////////////////////////////
// member
//////////////////////////////////////////////////////////////////////

unsigned short CFpcTb::Get_30rpm_Watt_ByLevel(unsigned char level)
{
	if (0 == level)		level = 1;
	if (30 < level)		level = 30;
	return pRpm_Level_Watt_Table[level - 1].Watt[RPM60_COL - 3];
}

unsigned short CFpcTb::Get_40rpm_Watt_ByLevel(unsigned char level)
{
	if (0 == level)		level = 1;
	if (30 < level)		level = 30;
	return pRpm_Level_Watt_Table[level - 1].Watt[RPM60_COL - 2];
}

unsigned short CFpcTb::Get_50rpm_Watt_ByLevel(unsigned char level)
{
	if (0 == level)		level = 1;
	if (30 < level)		level = 30;
	return pRpm_Level_Watt_Table[level - 1].Watt[RPM60_COL - 1];
}

unsigned short CFpcTb::Get_60rpm_Watt_ByLevel(unsigned char level)
{
	if (0 == level)		level = 1;
	if (30 < level)		level = 30;

	if (0 == product_type)
		return pRpm_Level_Watt_Table[level - 1].Watt[RPM60_COL];

	return pRpm_Level_Watt_Table_Bike[level - 1].Watt[5];
}

unsigned short CFpcTb::Get_70rpm_Watt_ByLevel(unsigned char level)
{
	if (0 == level)		level = 1;
	if (30 < level)		level = 30;

	if (0 == product_type)
		return pRpm_Level_Watt_Table[level - 1].Watt[RPM60_COL + 1];

	return pRpm_Level_Watt_Table_Bike[level - 1].Watt[6];
}

unsigned short CFpcTb::Get_80rpm_Watt_ByLevel(unsigned char level)
{
	if (0 == level)		level = 1;
	if (30 < level)		level = 30;
	return pRpm_Level_Watt_Table[level - 1].Watt[RPM60_COL + 2];
}

unsigned short CFpcTb::Get_90rpm_Watt_ByLevel(unsigned char level)
{
	if (0 == level)		level = 1;
	if (30 < level)		level = 30;
	return pRpm_Level_Watt_Table[level - 1].Watt[RPM60_COL + 3];
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
// 代謝當量(Metabolic Equivalent，METs)

unsigned short CFpcTb::Get_Watt_ByRpmLevel(unsigned short rpm, unsigned char level)
{
	unsigned idx = 3;

	if (0 == level)		level = 1;
	if (30 < level)		level = 30;

	if (rpm < 30)						idx = 0;
	else if (rpm >= 30 && rpm < 35)	idx = 0;
	else if (rpm >= 35 && rpm < 45)	idx = 1;
	else if (rpm >= 45 && rpm < 55)	idx = 2;
	else if (rpm >= 55 && rpm < 65)	idx = 3;
	else if (rpm >= 65 && rpm < 75)	idx = 4;
	else if (rpm >= 75 && rpm < 85)	idx = 5;
	else if (rpm >= 85 && rpm < 95)	idx = 6;
	else if (rpm >= 95 && rpm < 105)	idx = 7;
	else if (rpm >= 105 && rpm < 115)	idx = 8;
	else if (rpm >= 115 && rpm < 125)	idx = 9;
	else								idx = 9;

	return pRpm_Level_Watt_Table[level - 1].Watt[idx];
}


unsigned char CFpcTb::Get_Level_ByRpmWatt(unsigned short rpm, unsigned short watt)
{
	int i = 0;
	unsigned idx = 3;

	if (rpm < 30)						idx = 0;
	else if (rpm >= 30 && rpm < 35)	idx = 0;
	else if (rpm >= 35 && rpm < 45)	idx = 1;
	else if (rpm >= 45 && rpm < 55)	idx = 2;
	else if (rpm >= 55 && rpm < 65)	idx = 3;
	else if (rpm >= 65 && rpm < 75)	idx = 4;
	else if (rpm >= 75 && rpm < 85)	idx = 5;
	else if (rpm >= 85 && rpm < 95)	idx = 6;
	else if (rpm >= 95 && rpm < 105)	idx = 7;
	else if (rpm >= 105 && rpm < 115)	idx = 8;
	else if (rpm >= 115 && rpm < 125)	idx = 9;
	else								idx = 9;

	for(i = 0; i < FPC_LEVEL_TABLE_LEN; i++)
	{
		if(watt < pRpm_Level_Watt_Table[i].Watt[idx])
		{
			if(i < 1)
			{
				printf("(%s %d) L=%d W=%d\n", __FILE__, __LINE__, i,  pRpm_Level_Watt_Table[0].Watt[idx]);
				return 1;
			}
			printf("(%s %d) L=%d W=%d\n", __FILE__, __LINE__, i,  pRpm_Level_Watt_Table[i].Watt[idx]);
			return i;
		}	
	}
	printf("(%s %d) L=%d W=%d\n", __FILE__, __LINE__, i,  pRpm_Level_Watt_Table[29].Watt[idx]);
	return 30;
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
unsigned char CFpcTb::Get_30rpm_Level_ByWatt(unsigned short watt)
{
	int i = 0;

	for(i = 0; i < FPC_LEVEL_TABLE_LEN; i++)
	{
		if(watt < pRpm_Level_Watt_Table[i].Watt[RPM60_COL - 3])
		{
			if(i < 1)
			{
				printf("(%s %d) L=%d W=%d\n", __FILE__, __LINE__, i,  pRpm_Level_Watt_Table[0].Watt[RPM60_COL - 3]);
				return 1;
			}
			printf("(%s %d) L=%d W=%d\n", __FILE__, __LINE__, i,  pRpm_Level_Watt_Table[i].Watt[RPM60_COL - 3]);
			return i;
		}	
	}
	printf("(%s %d) L=%d W=%d\n", __FILE__, __LINE__, i,  pRpm_Level_Watt_Table[29].Watt[RPM60_COL - 3]);
	return 30;
}

unsigned char CFpcTb::Get_40rpm_Level_ByWatt(unsigned short watt)
{
	int i = 0;

	for(i = 0; i < FPC_LEVEL_TABLE_LEN; i++)
	{
		if(watt < pRpm_Level_Watt_Table[i].Watt[RPM60_COL - 2])
		{
			if(i < 1)
			{
				printf("(%s %d) L=%d W=%d\n", __FILE__, __LINE__, i,  pRpm_Level_Watt_Table[0].Watt[RPM60_COL - 2]);
				return 1;
			}
			printf("(%s %d) L=%d W=%d\n", __FILE__, __LINE__, i,  pRpm_Level_Watt_Table[i].Watt[RPM60_COL - 2]);
			return i;
		}	
	}
	printf("(%s %d) L=%d W=%d\n", __FILE__, __LINE__, i,  pRpm_Level_Watt_Table[29].Watt[RPM60_COL - 2]);
	return 30;
}

unsigned char CFpcTb::Get_50rpm_Level_ByWatt(unsigned short watt)
{
	int i = 0;

	for(i = 0; i < FPC_LEVEL_TABLE_LEN; i++)
	{
		if(watt < pRpm_Level_Watt_Table[i].Watt[RPM60_COL - 1])
		{
			if(i < 1)
			{
				printf("(%s %d) L=%d W=%d\n", __FILE__, __LINE__, i,  pRpm_Level_Watt_Table[0].Watt[RPM60_COL - 1]);
				return 1;
			}
			printf("(%s %d) L=%d W=%d\n", __FILE__, __LINE__, i,  pRpm_Level_Watt_Table[i].Watt[RPM60_COL - 1]);
			return i;
		}	
	}
	printf("(%s %d) L=%d W=%d\n", __FILE__, __LINE__, i,  pRpm_Level_Watt_Table[29].Watt[RPM60_COL - 1]);
	return 30;
}

unsigned char CFpcTb::Get_60rpm_Level_ByWatt(unsigned short watt)
{
	int i = 0;

	if (0 == product_type)
	{
		for(i = 0; i < FPC_LEVEL_TABLE_LEN; i++)
		{
			if(watt < pRpm_Level_Watt_Table[i].Watt[RPM60_COL])
			{
				if(i < 1)
				{
					return 1;
				}
				return i;
			}	
		}
		return 30;
	}

	for(i = 0; i < FPC_LEVEL_TABLE_LEN; i++)
	{
		if(watt < pRpm_Level_Watt_Table_Bike[i].Watt[5])
		{
			if(i < 1)
			{
				return 1;
			}
			return i;
		}	
	}
	return 30;
}

unsigned char CFpcTb::Get_70rpm_Level_ByWatt(unsigned short watt)
{
	int i = 0;

	if (0 == product_type)
	{
		for(i = 0; i < FPC_LEVEL_TABLE_LEN; i++)
		{
			if(watt < pRpm_Level_Watt_Table[i].Watt[RPM60_COL])
			{
				if(i < 1)
				{
					return 1;
				}
				return i;
			}	
		}
		return 30;
	}

	for(i = 0; i < FPC_LEVEL_TABLE_LEN; i++)
	{
		if(watt < pRpm_Level_Watt_Table_Bike[i].Watt[6])
		{
			if(i < 1)
			{
				return 1;
			}
			return i;
		}	
	}
	return 30;
}


///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

unsigned char CFpcTb::Get_AS_rpm(unsigned char i)
{
	return pAuto_Stride_Table[i].rpm;
}

unsigned char CFpcTb::Get_AS_stride(unsigned char i)
{
	return pAuto_Stride_Table[i].stride;
}

unsigned short CFpcTb::Get_Watt(unsigned char index, unsigned char rpm)
{
	if (0 == index)
		index = 1;
	if (0 == product_type)
		return pRpm_Level_Watt_Table[index - 1].Watt[rpm];
	return pRpm_Level_Watt_Table_Bike[index - 1].Watt[rpm];
}

