// KsApp.h: interface for the CDorisApp class.
//
//////////////////////////////////////////////////////////////////////


#if !defined(AFX_KSAPP_H__A1DB9750_6E3A_44C9_BA50_39E1A157A74F__INCLUDED_)
#define AFX_KSAPP_H__A1DB9750_6E3A_44C9_BA50_39E1A157A74F__INCLUDED_


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CKsApp : public CApp
{
public:
	CKsApp();
	virtual ~CKsApp();

public:
	unsigned char ProcessUserMessage(unsigned char *buff, int fd);

public:
	int i2c0;
	pthread_t tUi;
	int pwm1;


protected:
	//{{AFX_DATA(CKsApp)
	afx_msg LRESULT OnInitInstance(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult);
	afx_msg LRESULT OnExitInstance(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult);
	afx_msg LRESULT OnTimer(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult);
	afx_msg LRESULT OnUserMessage(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult);
	afx_msg LRESULT OnKeyScanPressed(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult);
	afx_msg LRESULT OnKeyScanReleased(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult);
	//}}AFX_DATA

	DECLARE_MESSAGE_MAP()
};

#endif // !defined(AFX_KSAPP_H__A1DB9750_6E3A_44C9_BA50_39E1A157A74F__INCLUDED_)

