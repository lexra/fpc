// FpcTb.h : 
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FPCTB_H__62B243F7_35C4_485A_A548_14BA74273542__INCLUDED_)
#define AFX_FPCTB_H__62B243F7_35C4_485A_A548_14BA74273542__INCLUDED_


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


//////////////////////////////////////////////////////////////////////
// struct
//////////////////////////////////////////////////////////////////////

#define FPC_LEVEL_TABLE_LEN 30
#define RPM_TABLE_LEN	12

// jason note
#define RPM60_COL	5

#define AS_TABLE_COUNT	12


struct AutoStrideTable{
	unsigned short rpm;
	unsigned short stride;
};
struct RpmLevelWatTable{
	unsigned short Watt[RPM_TABLE_LEN];
};


//////////////////////////////////////////////////////////////////////
// class
//////////////////////////////////////////////////////////////////////

class CFpcTb
{
public:
	CFpcTb();
	virtual ~CFpcTb();

public:
	struct AutoStrideTable *AutoSride_Table;
	struct RpmLevelWatTable *Rpm_Level_Watt_Table;
	//struct RpmLevelWatTable *pRpm_Level_Watt_Table_Bike;

//////////////////////////////////////////////////////////////////////
	unsigned char Get_AS_stride(unsigned char i);
	unsigned short Get_Watt(unsigned char index, unsigned char rpm);
	unsigned char Get_AS_rpm(unsigned char i);


//////////////////////////////////////////////////////////////////////
	unsigned char Get_70rpm_Level_ByWatt(unsigned short watt);
	unsigned char Get_60rpm_Level_ByWatt(unsigned short watt);
	unsigned char Get_30rpm_Level_ByWatt(unsigned short watt);
	unsigned char Get_40rpm_Level_ByWatt(unsigned short watt);
	unsigned char Get_50rpm_Level_ByWatt(unsigned short watt);

//////////////////////////////////////////////////////////////////////
	unsigned char Get_Level_ByRpmWatt(unsigned short rpm, unsigned short watt);
	unsigned short Get_Watt_ByRpmLevel(unsigned short rpm, unsigned char level);


//////////////////////////////////////////////////////////////////////
	unsigned short Get_60rpm_Watt_ByLevel(unsigned char level);
	unsigned short Get_50rpm_Watt_ByLevel(unsigned char level);
	unsigned short Get_40rpm_Watt_ByLevel(unsigned char level);
	unsigned short Get_30rpm_Watt_ByLevel(unsigned char level);
	unsigned short Get_70rpm_Watt_ByLevel(unsigned char level);
	unsigned short Get_80rpm_Watt_ByLevel(unsigned char level);
	unsigned short Get_90rpm_Watt_ByLevel(unsigned char level);
};


#endif // !defined(AFX_FPCTB_H__62B243F7_35C4_485A_A548_14BA74273542__INCLUDED_)

