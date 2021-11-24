// Sd55App.h : 
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SD55APP_H__62B243F7_35C4_485A_A548_14BA74273540__INCLUDED_)
#define AFX_SD55APP_H__62B243F7_35C4_485A_A548_14BA74273540__INCLUDED_


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000



//////////////////////////////////////////////////////////////////////
// routine
//////////////////////////////////////////////////////////////////////

int SendCmd(int fd, int cmd, int len, const unsigned char *value);
void SendCmdB(int fd, unsigned int len, unsigned int cmd,unsigned int data);



//////////////////////////////////////////////////////////////////////
// class
//////////////////////////////////////////////////////////////////////

class CSd55App : public CApp, public CFpcData
{
public:
	CSd55App();
	virtual ~CSd55App();

public:
	HANDLE hUart;

	int ttyUSB0;
	pthread_t tUart;

public:
	virtual unsigned char SerialUART_in(unsigned char *buffer, unsigned char len);
	virtual void SerialUART_out(void);


	virtual void SetCmd(unsigned char len, unsigned char cmd, ...);
	

public:
	int AttachUart(const char *path, int baudrate = 9600);

private:
	int TestUart(const char *path);


public:
	afx_msg LRESULT OnUartDataRcv(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult);

protected:
	//{{AFX_DATA(CSd55App)
	afx_msg LRESULT OnInitInstance(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult);
	afx_msg LRESULT OnExitInstance(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult);
	//afx_msg LRESULT OnUartDataRcv(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult);
	//}}AFX_DATA

	DECLARE_MESSAGE_MAP()
};


#endif // !defined(AFX_SD55APP_H__62B243F7_35C4_485A_A548_14BA74273540__INCLUDED_)


