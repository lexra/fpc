
#if !defined(AFX_UTILS_H__285CFF59_8261_4DE7_84F3_43E209FEF60C__INCLUDED_)
#define AFX_UTILS_H__285CFF59_8261_4DE7_84F3_43E209FEF60C__INCLUDED_

//#include <stdlib.h>
//#include <string.h>


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000



#ifndef INFINITE
 #define INFINITE				0xFFFFFFFF
#endif

#ifndef ETIMEDOUT
#define WAIT_TIMEOUT			ETIMEDOUT
#else
#define WAIT_TIMEOUT			0x00000102L
#endif

#define WAIT_FAILED			0xFFFFFFFFL
#define WAIT_OBJECT_0		0x00000000L
#define WAIT_ABANDONED	0x00000080L



//////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////

struct event_t
{
	int id;
	int type;
	char name[256];

	int manual_reset;
	pthread_cond_t event;
};



//////////////////////////////////////////////////////////////////////
// extern
//////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif

HANDLE CreateEvent(void *pAttr, int bManualReset, int bInitialState, const char *szName);
int CloseEvent(HANDLE hObject);
int SetEvent(HANDLE hEvent);
int ResetEvent(HANDLE hEvent);
int PulseEvent(HANDLE hEvent);
unsigned int WaitForSingleObject(HANDLE hEvent, unsigned int dwMilliseconds);
unsigned int WaitForMultipleObjects(unsigned int nCount, const HANDLE *lpHandles, int bWaitAll, unsigned int dwMilliseconds);

#ifdef __cplusplus
}
#endif

#endif // !defined(AFX_UTILS_H__285CFF59_8261_4DE7_84F3_43E209FEF60C__INCLUDED_)


