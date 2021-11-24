// StdAfx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__3F41C39C_B21A_4F9F_BD57_0061B53E9B86__INCLUDED_)
#define AFX_STDAFX_H__3F41C39C_B21A_4F9F_BD57_0061B53E9B86__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#ifdef WIN32
 #define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

 #include <afxwin.h>         // MFC core and standard components
 #include <afxext.h>         // MFC extensions
 #include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls

 #ifndef _AFX_NO_AFXCMN_SUPPORT
  #include <afxcmn.h>			// MFC support for Windows Common Controls
 #endif // _AFX_NO_AFXCMN_SUPPORT

#endif // WIN32


//{{AFX_INSERT_LOCATION}}
#ifndef NULL
 #define NULL 0
#endif
#ifndef BOOL
 #define BOOL int
#endif
#ifndef INT
 #define INT int
#endif
#ifndef UINT
 #define UINT unsigned int
#endif
#ifndef DWORD
 #define DWORD unsigned long
#endif
#ifndef LONG
#define LONG long
#endif
#ifndef WORD
 #define WORD unsigned short
#endif
#ifndef BYTE
 #define BYTE unsigned char
#endif
#ifndef TRUE
 #define TRUE 1
 #define FALSE 0
#endif
#ifndef WPARAM
 #define WPARAM unsigned int
#endif
#ifndef LPARAM
 #define LPARAM unsigned int
#endif
#ifndef LRESULT
 #define LRESULT unsigned int
#endif
#ifndef INFINITE
 #define INFINITE 0XFFFFFFFF  // Infinite timeout
#endif
#ifndef SIGNED
 #define SIGNED int
#endif
#ifndef UCHAR
 #define UCHAR unsigned char
#endif
#ifndef LONG
 #define LONG int
#endif
#ifndef INT64
 #define INT64 signed long long
#endif
#ifndef UINT64
 #define UINT64 unsigned long long
#endif
#ifndef FLOAT
 #define FLOAT float
#endif


typedef void *HANDLE;
typedef unsigned char U8_T;
typedef unsigned char U8_B_T;
typedef unsigned char U8_S_T;
typedef unsigned short U16_T;
typedef unsigned long U32_T;
typedef unsigned long U32_IP_T;
typedef char I8_T;
typedef short I16_T;
typedef long I32_T;


#define STATUS_WAIT_0						((DWORD)0X00000000L)
#define STATUS_ABANDONED_WAIT_0				((DWORD)0X00000080L)
#define STATUS_USER_APC						((DWORD)0X000000C0L)
#define STATUS_TIMEOUT						((DWORD)0X00000102L)
#define STATUS_PENDING						((DWORD)0X00000103L)

#define STATUS_SEGMENT_NOTIFICATION			((DWORD)0X40000005L)
#define STATUS_GUARD_PAGE_VIOLATION			((DWORD)0X80000001L)
#define STATUS_DATATYPE_MISALIGNMENT		((DWORD)0X80000002L)
#define STATUS_BREAKPOINT					((DWORD)0X80000003L)
#define STATUS_SINGLE_STEP					((DWORD)0X80000004L)
#define STATUS_ACCESS_VIOLATION				((DWORD)0XC0000005L)
#define STATUS_IN_PAGE_ERROR				((DWORD)0XC0000006L)
#define STATUS_INVALID_HANDLE				((DWORD)0XC0000008L)
#define STATUS_NO_MEMORY					((DWORD)0XC0000017L)
#define STATUS_ILLEGAL_INSTRUCTION			((DWORD)0XC000001DL)
#define STATUS_NONCONTINUABLE_EXCEPTION		((DWORD)0XC0000025L)
#define STATUS_INVALID_DISPOSITION			((DWORD)0XC0000026L)
#define STATUS_ARRAY_BOUNDS_EXCEEDED		((DWORD)0XC000008CL)
#define STATUS_FLOAT_DENORMAL_OPERAND		((DWORD)0XC000008DL)
#define STATUS_FLOAT_DIVIDE_BY_ZERO			((DWORD)0XC000008EL)
#define STATUS_FLOAT_INEXACT_RESULT			((DWORD)0XC000008FL)
#define STATUS_FLOAT_INVALID_OPERATION		((DWORD)0XC0000090L)
#define STATUS_FLOAT_OVERFLOW				((DWORD)0XC0000091L)
#define STATUS_FLOAT_STACK_CHECK			((DWORD)0XC0000092L)
#define STATUS_FLOAT_UNDERFLOW				((DWORD)0XC0000093L)
#define STATUS_INTEGER_DIVIDE_BY_ZERO		((DWORD)0XC0000094L)
#define STATUS_INTEGER_OVERFLOW				((DWORD)0XC0000095L)
#define STATUS_PRIVILEGED_INSTRUCTION		((DWORD)0XC0000096L)
#define STATUS_STACK_OVERFLOW				((DWORD)0XC00000FDL)
#define STATUS_CONTROL_C_EXIT				((DWORD)0XC000013AL)
#define STATUS_FLOAT_MULTIPLE_FAULTS		((DWORD)0XC00002B4L)
#define STATUS_FLOAT_MULTIPLE_TRAPS			((DWORD)0XC00002B5L)
#define STATUS_ILLEGAL_VLM_REFERENCE		((DWORD)0XC00002C0L)

#define ULONGLONGINT unsigned long long int


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

#define BIT0															(1<<0)
#define BIT1															(1<<1)
#define BIT2															(1<<2)
#define BIT3															(1<<3)
#define BIT4															(1<<4)
#define BIT5															(1<<5)
#define BIT6															(1<<6)
#define BIT7															(1<<7)


#define DBG_BASIC													(1 << 0)
#define DBG_APP														(1 << 1)

#define DBG_UART													(1 << 2)
#define DBG_JACK													(1 << 3)

#define DBG_MSG														(1 << 4)
#define DBG_FIFO													(1 << 5)
#define DBG_SOCKET													(1 << 6)
#define DBG_EVENT													(1 << 7)
#define DBG_MISC													(1 << 8)
#define DBG_MALLOC													(1 << 9)
#define DBG_MUTEX													(1 << 9)
#define DBG_SIGNAL													(1 << 10)
#define DBG_I2C														(1 << 11)
#define DBG_IOCTL													(1 << 12)
#define DBG_SPI														(1 << 13)
#define DBG_THREAD													(1 << 14)
#define DBG_INIT														(1 << 15)



#define DBG_ALL														0XFFFFFFFFFFFFFFFFLLU
#define DBG_DEFAULT												(DBG_UART | DBG_JACK | DBG_IOCTL | DBG_MALLOC | DBG_BASIC | DBG_SOCKET | DBG_THREAD | DBG_I2C)


#define dbg_printf(level, format, args...) \
	do { \
		if (_debug & level) \
			printf(format, ##args); \
	} while (0)

extern unsigned long long int _debug;



//////////////////////////////////////////////////////////////////////
// MSG structure definition
//////////////////////////////////////////////////////////////////////
#define M_INIT_INSTANCE					0X00000001
#define M_EXIT_INSTANCE					0X00000002
#define M_TIMER								0X00000003
#define M_SHELL								0X00000004
#define M_UART_DATA_RCV					0X00000005
#define M_SERVICE_DATA_RCV					0X00000006
#define M_SIG_ALRM							0X00000007
#define M_FIFO								0X00000008
#define M_CONN_DATA_RCV					0X00000009
#define M_JACK_DATA_RCV					0X0000000A
#define M_UI_INCOMING						0X0000000B
#define M_WAIT_FPC_END_OF_PROGRAM		0X0000000C
#define M_KEY_SCAN_RELEASED				0X0000000D
#define M_KEY_SCAN_PRESSED				0X0000000E
#define M_TICK								0X0000000F
#define M_LEFT_THUMB						0X00000010
#define M_RIGHT_THUMB						0X00000011
#define M_POST_INIT_INSTANCE				0X00000012



#endif // !defined(AFX_STDAFX_H__3F41C39C_B21A_4F9F_BD57_0061B53E9B86__INCLUDED_)

