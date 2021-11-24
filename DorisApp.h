// DorisApp.h: interface for the CDorisApp class.
//
//////////////////////////////////////////////////////////////////////


#if !defined(AFX_DORISAPP_H__A1DB9750_6E3A_44C9_BA50_39E1A15FA74F__INCLUDED_)
#define AFX_DORISAPP_H__A1DB9750_6E3A_44C9_BA50_39E1A15FA74F__INCLUDED_


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CDorisApp : public CApp
{
public:
	CDorisApp();
	virtual ~CDorisApp();

public:
	int AttachUart(const char *path, int baudrate = 9600);
	int DetachUart(const char *path);

private:
	int TestUart(const char *path);

public:
	int ttyS0;
	int ttyS1;
	int ttyS2;
	int ttyUSB0;

	pthread_t tUart;
	HANDLE hUart;
	pthread_t tSvc;

protected:
	//{{AFX_DATA(CDorisApp)
	afx_msg LRESULT OnInitInstance(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult);
	afx_msg LRESULT OnExitInstance(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult);
	afx_msg LRESULT OnTimer(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult);
	afx_msg LRESULT OnShell(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult);
	afx_msg LRESULT OnUartDataRcv(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult);
	afx_msg LRESULT OnServiceDataRcv(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult);
	afx_msg LRESULT OnAlrm(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult);
	//}}AFX_DATA

	DECLARE_MESSAGE_MAP()
};

#endif // !defined(AFX_DORISAPP_H__A1DB9750_6E3A_44C9_BA50_39E1A15FA74F__INCLUDED_)


