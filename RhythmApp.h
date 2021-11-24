
#if !defined(AFX_RHYTHMAPP_H__35647243F7_35C4_485A_A548_14BA74273540__INCLUDED_)
#define AFX_RHYTHMAPP_H__35647243F7_35C4_485A_A548_14BA74273540__INCLUDED_


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000



//////////////////////////////////////////////////////////////////////
// define
//////////////////////////////////////////////////////////////////////

#define RM6T3_POST_INIT_TIMER_ID				11
#define RM6T3_FR_TIMER_ID						21
#define RM6T3_RR_TIMER_ID						22
#define RM6T3_RRFR_TIMER_ID					23



//////////////////////////////////////////////////////////////////////
// class
//////////////////////////////////////////////////////////////////////

class CRhythmApp : public CApp
{
public:
	CRhythmApp();
	virtual ~CRhythmApp();

public:
	HANDLE hUart;

	int ttyS0;
	pthread_t tUart;
	pthread_t tUi;

public:
	int AttachUart(const char *path, int baudrate = 9600);

private:
	int TestUart(const char *path);

protected:
	//{{AFX_DATA(CSd55App)
	afx_msg LRESULT OnInitInstance(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult);
	afx_msg LRESULT OnExitInstance(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult);
	afx_msg LRESULT OnTimer(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult);
	afx_msg LRESULT OnUartDataRcv(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult);
	afx_msg LRESULT OnUserMessage(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult);
	//}}AFX_DATA

	DECLARE_MESSAGE_MAP()
};


#endif // !defined(AFX_RHYTHMAPP_H__35647243F7_35C4_485A_A548_14BA74273540__INCLUDED_)



