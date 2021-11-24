// Pool.h: interface for the CPool class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_POOL_H__C387DA27_CEA1_40E4_BB6A_C0D9D9895823__INCLUDED_)
#define AFX_POOL_H__C387DA27_CEA1_40E4_BB6A_C0D9D9895823__INCLUDED_


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000



class CPool 
{
public:
	CPool(INT nCli = 16,  INT nBuffers = 8, INT nSize = 256);
	virtual ~CPool();

public:
	unsigned char *Malloc(void);
	INT GetEntryNum(unsigned char *p);
	INT GetEntryId(unsigned char *p);
	unsigned char *SetEntryId(unsigned char *p, INT id);
	unsigned char *NewEntry(void);
	INT ReleaseEntry(unsigned char *p);
	unsigned char *GetEntry(unsigned char *p);
	INT GetBuffNum(unsigned char *p);
	unsigned char *GetBuff(unsigned char *p);
	unsigned char *EmptyBuff(unsigned char *p);
	unsigned char *NextBuff(unsigned char *p);
	INT GetBuffLen(unsigned char *p);
	unsigned char *Memcpy(unsigned char *p, unsigned char *s, INT len);
	unsigned char *FindEntry(INT id);
	INT GetTotalEntries(void) { return cliNum; };

private:
	unsigned char *start;
	unsigned char *end;
	INT cliNum;
	INT bufNum;
	INT bufSize;

private:
	INT X, Y, Z;
};


#endif // !defined(AFX_POOL_H__C387DA27_CEA1_40E4_BB6A_C0D9D9895823__INCLUDED_)


