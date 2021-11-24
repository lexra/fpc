// App.h : 
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_APP_H__62B243F7_35C4_485A_A548_14BA7427E747__INCLUDED_)
#define AFX_APP_H__62B243F7_35C4_485A_A548_14BA7427E747__INCLUDED_


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


/////////////////////////////////////////////////////////////////////////////
// defines & macros
/////////////////////////////////////////////////////////////////////////////

struct AFX_MSGMAP_ENTRY;
struct AFX_MSGMAP;

#define DECLARE_MESSAGE_MAP() \
public: \
	static const AFX_MSGMAP_ENTRY _messageEntries[]; \
public: \
	static const AFX_MSGMAP messageMap; \
	virtual const AFX_MSGMAP *GetMessageMap() const;

#define afx_msg virtual


typedef void (*TIMER_FUNC)(UINT nId);


typedef struct Timer_t
{
	UINT id;
	unsigned int timeout;
	TIMER_FUNC cb;
	unsigned int enable;
} Timer_t;


typedef struct TIMER
{

#if defined(__cplusplus) || defined(__CPLUSPLUS__)
public:
	TIMER()
	{
		id = 0;
		timeout = 0;
		cb = 0;
	};
#endif //__cplusplus

	struct list_head list;
	UINT id;
	unsigned int timeout;
	TIMER_FUNC cb;
} TIMER;


typedef struct MSG
{

#if defined(__cplusplus) || defined(__CPLUSPLUS__)
public:
	MSG()
	{
		type = 0, wparam = 0, lparam = 0, result = 0;
		time(&stamp);
	};
#endif //__cplusplus

	struct list_head list;
	DWORD type;
	time_t stamp;
	WPARAM wparam;
	LPARAM lparam;
	LRESULT result;
} MSG, *PMSG;


struct incoming_t
{
	struct list_head list;
	struct sockaddr_in cliaddr;
	char name[60];
	int type;
	int fd;
	int len;
	int offs;
	int left;
	unsigned char msg[0];
};


/////////////////////////////////////////////////////////////////////////////
// CApp
/////////////////////////////////////////////////////////////////////////////

class CApp
{
public:
	CApp();
	virtual ~CApp();
	
public:
	int Run();
	MSG *GetMsgQueue();
	BOOL PostMessage(DWORD type, WPARAM wparam = 0, LPARAM lparam = 0);
	BOOL PostMessage(struct MSG *pmsg);
	UINT SetTimer(UINT nId, UINT nElapse = 1000, TIMER_FUNC cb = 0);
	BOOL KillTimer(UINT nId);

public:
	HANDLE hMsg;
	HANDLE hQuit;
	pthread_t tMq;
	pthread_t tTm;
	pthread_t tShell;
	pthread_t tSw;

protected:
	//{{AFX_DATA(CApp)
	afx_msg LRESULT OnInitInstance(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult);
	afx_msg LRESULT OnExitInstance(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult);
	afx_msg LRESULT OnTimer(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult);
	afx_msg LRESULT OnShell(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult);
	//}}AFX_DATA

	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////
// global routine
/////////////////////////////////////////////////////////////////////////////

CApp *AfxGetApp();


/////////////////////////////////////////////////////////////////////////////
// defines & macros
/////////////////////////////////////////////////////////////////////////////

typedef LRESULT (CApp::*AFX_PMSG)(DWORD, WPARAM, LPARAM, LRESULT *);
struct AFX_MSGMAP_ENTRY
{
	UINT nMessage;
	//UINT nSig;
	AFX_PMSG pfn;
};
struct AFX_MSGMAP
{
	const AFX_MSGMAP *pBaseMap;
	const AFX_MSGMAP_ENTRY *lpEntries;
};

#define BEGIN_MESSAGE_MAP(theClass, baseClass) \
	const AFX_MSGMAP *theClass::GetMessageMap() const \
		{ return &theClass::messageMap; } \
	const AFX_MSGMAP theClass::messageMap = \
		{&baseClass::messageMap, &theClass::_messageEntries[0]}; \
	const AFX_MSGMAP_ENTRY theClass::_messageEntries[] = \
		{

#define END_MESSAGE_MAP() \
			{0, 0} \
		};

#define ON_MESSAGE(id, memberFxn) \
			{id, static_cast<AFX_PMSG>(memberFxn)},


#endif // !defined(AFX_APP_H__62B243F7_35C4_485A_A548_14BA7427E747__INCLUDED_)


