// FiFoApp.h: interface for the CFiFoApp class.
//
//////////////////////////////////////////////////////////////////////


#if !defined(AFX_FIFO_APP_H__A1DB9750_6E3A_44C9_BA50_39E1A15FA74F__INCLUDED_)
#define AFX_FIFO_APP_H__A1DB9750_6E3A_44C9_BA50_39E1A15FA74F__INCLUDED_


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CFifoApp : public CDorisApp
{
public:
	CFifoApp();
	virtual ~CFifoApp();

public:
	int fifo;
	//int jack;
	int i2c0;
	int i2c1;
	int spi0;

	pthread_t tFifo;
	pthread_t tConn;
	pthread_t tJack;


protected:
	//{{AFX_DATA(CFifoApp)
	afx_msg LRESULT OnInitInstance(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult);
	afx_msg LRESULT OnExitInstance(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult);
	afx_msg LRESULT OnTimer(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult);
	afx_msg LRESULT OnFIFO(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult);
	afx_msg LRESULT OnConnDataRcv(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult);
	afx_msg LRESULT OnJackDataRcv(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult);
	afx_msg LRESULT OnUartDataRcv(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult);
	//}}AFX_DATA

	DECLARE_MESSAGE_MAP()
};

#endif // !defined(AFX_FIFO_APP_H__A1DB9750_6E3A_44C9_BA50_39E1A15FA74F__INCLUDED_)


