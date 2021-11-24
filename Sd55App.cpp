// Sd55App.cpp: implementation of the CSd55App class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <strings.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <regex.h>
#include <sys/ioctl.h> 
#include <sys/ipc.h>
#include <signal.h>
#include <sys/msg.h>
#include <pthread.h>
#include <assert.h>
//#include <sys/inotify.h>
#include <termios.h>
#include <netinet/in.h>
#include <inttypes.h>
#include <stddef.h>
#include <assert.h>
#include <sys/socket.h>


#include <asm/ioctl.h>
#include <asm/errno.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#include "list.h"
#include "Event.h"
#include "Misc.h"
#include "BaseApp.h"
#include "FpcTb.h"
#include "Sd55Data.h"
#include "FpcData.h"
#include "Sd55App.h"
#include "FpcApp.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#ifdef WIN32
#define new DEBUG_NEW
#endif//WIN32
#endif // _DEBUG



//////////////////////////////////////////////////////////////////////
// define
//////////////////////////////////////////////////////////////////////


#if 0
// /proc/tty/driver/usbserial
/*
0: module:pl2303 name:"pl2303" vendor:067b product:2303 num_ports:1 port:1 path:usb-spmp-1.2
1: module:pl2303 name:"pl2303" vendor:067b product:2303 num_ports:1 port:1 path:usb-spmp-1.3
2: module:pl2303 name:"pl2303" vendor:067b product:2303 num_ports:1 port:1 path:usb-spmp-1.4
*/
> 0XF5-0XC3-0X00-0X0F-0X2E
> 0XF5-0XD2-0X42-0XEC
> 0XF5-0XD2-0X07-0X27
> 0XF5-0XD2-0X08-0X26
#endif // 0


int sd55_err_state_ack = 0;

int sd55_ok = 0;
int nCmd = 0;

// F5 + LEN + 07 + SUM
// F5 + LEN + 05 + xx + SUM

//chuck modify 

int mProduct_type = 0;
int mRpmZeroCount = 0;

//============

void SendCmdB(int fd,unsigned int len, unsigned int cmd,unsigned int data)
{

	printf("Command = 0X%02X   data = %d \n",cmd,data);

	
	int x;
    unsigned char UartCommand[128] = {0xFE, 0xFE};

	//if(cmd != 0x20) return ;
	
	UartCommand[2] = HI_UINT16(len);
	UartCommand[3] = LO_UINT16(len);
	UartCommand[4] = 0;
	UartCommand[5] = 0;
	UartCommand[6] = HI_UINT16(cmd);
	UartCommand[7] = LO_UINT16(cmd);
	if(len == 2)
	{
	UartCommand[8] = HI_UINT16(data);
	UartCommand[9] = LO_UINT16(data);
	}
	else if(len == 1)
	{
	UartCommand[8] = LO_UINT16(data);
	}
	
    x =      ModbusCrc16(UartCommand,8+len);
	UartCommand[8+len] =	 HI_UINT16(x);
	UartCommand[9+len] =   LO_UINT16(x);


/*	printf(">\n");
	for (x = 0; x < 10+len; x++)
	{
		printf("0X%02X-", UartCommand[x]);
	}
	printf("\n");
*/


		
	write(fd, UartCommand, 10+len);


}





int SendCmd(int fd, int cmd, int len, const unsigned char value[])
{
	unsigned char buff[1024];
	int cs;
	int i;
	int l;

	if (len < 0)	return -1;
	if (len > 0)
	{
		if (0 == value)				return -1;
		for(i = 0; i < len; i++)			buff[i + 3] = value[i];
	}
	l = len + 2;
	buff[0] = SD55_FRAME_HEADER;
	buff[1] = l | (0XF0 - ( l << 4 ));

	buff[2] = cmd;

	cs = buff[1]; cs += buff[2];
	for(i = 0; i < len; i++)
		cs += buff[3 + i];
	buff[len + 3] = (unsigned char)(0 - cs);
	nCmd = cmd;
	l = write(fd, buff, len + 4);

#if 0
	do { printf("> : "); } while(0);
	for (i = 0; i < len + 4; i++)
	{
		if (i != len + 4 -1)
			do { printf("0X%02X-", buff[i]); } while(0);
		else
			do { printf("0X%02X", buff[i]); } while(0);
	}
	do { printf("\n"); } while(0);
#endif //0

	return l;
}



//////////////////////////////////////////////////////////////////////
// routines
//////////////////////////////////////////////////////////////////////

static pthread_mutex_t xUatrt = PTHREAD_MUTEX_INITIALIZER;
static struct incoming_t uartQ;


void *UartThread(void *param)
{
	CSd55App *app = (CSd55App *)param;
	HANDLE hUart = app->hUart;
	int res = 0;
	struct list_head *P, *Q;
	struct incoming_t *N;
	int fd;
	int maxfd;
	fd_set rset;
	int C;
	int len;
	sigset_t nset, oset;
	unsigned char buff[1024 * 4];

	unsigned int ret;
	unsigned char *p;
	struct timeval tv;
	int last_type;

	sigemptyset(&nset);
	sigaddset(&nset, SIGHUP), sigaddset(&nset, SIGINT), sigaddset(&nset, SIGTERM), sigaddset(&nset, SIGUSR1), sigaddset(&nset, SIGALRM);
	pthread_sigmask(SIG_BLOCK, &nset, &oset);
	pthread_cleanup_push(RestoreSigmask, (void *)&oset);

WAIT:
	hUart = app->hUart;
	if(0 == hUart)														{ dbg_printf(DBG_EVENT, "(%s %d) 0 == hUart\n", __FILE__, __LINE__); goto BAITOUT; }
	ret = WaitForSingleObject(hUart, INFINITE);
	if(WAIT_FAILED == ret)											{ dbg_printf(DBG_EVENT, "(%s %d) WaitForSingleObject() WAIT_FAILED\n", __FILE__, __LINE__); goto BAITOUT; }
	if(WAIT_TIMEOUT == ret)											{ dbg_printf(DBG_EVENT, "(%s %d) WaitForSingleObject() WAIT_TIMEOUT\n", __FILE__, __LINE__); goto WAIT; }

	for(;;)
	{
		pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &last_type);
		pthread_mutex_lock(&xUatrt);
		pthread_cleanup_push(CleanupLock, (void *)&xUatrt);
		pthread_setcanceltype(last_type, NULL);
		maxfd = 0; C = 0;
		FD_ZERO(&rset);
		list_for_each_safe(P, Q, &uartQ.list)
		{
			 C++; N = list_entry(P, struct incoming_t, list); assert(0 != N); fd = N->fd; FD_SET(fd, &rset);
			if (maxfd < fd)											maxfd = fd;
		}
		pthread_cleanup_pop(1);
		if (0 == C)
		{
			dbg_printf(DBG_UART, "(%s %d) ResetEvent(hUart) goto WAIT; \n", __FILE__, __LINE__);
			ResetEvent(hUart);
			goto WAIT;
		}

		tv.tv_sec = 1; tv.tv_usec = 1000;
		if (0 > (res = select(maxfd + 1, &rset, 0, 0, &tv)))
		{
			dbg_printf(DBG_UART, "(%s %d) SELECT() FAIL\n", __FILE__, __LINE__); 
			AfxGetApp()->PostMessage(M_EXIT_INSTANCE, 0, 0);
		}
		else if (0 == res)												{ continue; }
		else
		{
			pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &last_type);
			pthread_mutex_lock(&xUatrt);
			pthread_cleanup_push(CleanupLock, (void *)&xUatrt);
			pthread_setcanceltype(last_type, NULL);

			list_for_each_safe(P, Q, &uartQ.list)
			{
				N = list_entry(P, struct incoming_t, list); assert(0 != N); fd = N->fd;
				if (FD_ISSET(fd, &rset))
				{
					memset(buff, 0, sizeof(buff));
					len = read(fd, buff, sizeof(buff));
					if (len < 0)
					{
						if (errno == EAGAIN)							{ dbg_printf(DBG_UART, "(%s %d) EAGAIN\n", __FILE__, __LINE__); continue; }
						if (errno == EWOULDBLOCK)					{ dbg_printf(DBG_UART, "(%s %d) EWOULDBLOCK\n", __FILE__, __LINE__); continue; }
						if (errno == EBADF)							{ dbg_printf(DBG_UART, "(%s %d) EBADF\n", __FILE__, __LINE__); close(fd); list_del(P); free(N); continue; }
						dbg_printf(DBG_UART, "(%s %d) len < 0, errno=%d\n", __FILE__, __LINE__, errno); AfxGetApp()->PostMessage(M_EXIT_INSTANCE, 0, 0); close(fd); list_del(P); free(N); continue;
					}
					if (len == 0)										{ dbg_printf(DBG_UART, "(%s %d) len== 0\n", __FILE__, __LINE__); close(fd); list_del(P); free(N); continue; }
//printf(".");
					p = (unsigned char *)malloc(64 + len + 32);
					if (p)
					{
						CSd55App *app = 0;

						memset(p, 0, 64 + len);
						memcpy(p, &len, sizeof(int));
						strcpy((char *)p + sizeof(int), (char *)N->name);
						memcpy(p + 64, buff, len);

#if 0
						app->PostMessage(M_UART_DATA_RCV, (WPARAM)p, N->fd);
#else
						app = (CSd55App *)AfxGetApp();
						app->OnUartDataRcv(M_UART_DATA_RCV, (WPARAM)p, N->fd, 0);
#endif

					}
					else
					{ dbg_printf(DBG_MALLOC, "(%s %d) MALLOC() FAIL\n", __FILE__, __LINE__); }
				}
			}

			pthread_cleanup_pop(1);


		}
	}


BAITOUT:
	pthread_cleanup_pop(1);

dbg_printf(DBG_THREAD, "(%s %d) UartThread() EXIT\n", __FILE__, __LINE__); 
	return 0;
}



//////////////////////////////////////////////////////////////////////
// class member
//////////////////////////////////////////////////////////////////////

int CSd55App::AttachUart(const char *path, int baudrate)
{
	struct list_head *P, *Q;
	struct incoming_t *N;
	struct termios term;
	int v = 0, f = 0, fd = -1;
	int last_type;

	if (!TestUart(path))												{ dbg_printf(DBG_UART, "(%s %d) TestUart(%s) FAIL\n", __FILE__, __LINE__, path); return 0;}
	if (baudrate != 9600 && baudrate != 19200 && baudrate != 38400 && baudrate != 57600 && baudrate != 115200 && baudrate != 4800)
		baudrate = 9600;
	if (0 > (fd = open(path, O_RDWR | O_NOCTTY | O_NDELAY)))			{ dbg_printf(DBG_UART, "(%s %d) OPEN() FAIL\n", __FILE__, __LINE__); return 0; }
	if (!isatty(fd))														{ dbg_printf(DBG_UART, "(%s %d) !ISATTY() fd=%d\n", __FILE__, __LINE__, fd); close(fd); return 0; }

	memset(&term, 0, sizeof(term));
	term.c_cflag |= B9600, 	term.c_cflag |= CLOCAL, term.c_cflag |= CREAD, term.c_cflag &= ~PARENB, term.c_cflag &= ~CSTOPB;
	term.c_cflag &= ~CSIZE, term.c_cflag |= CS8, term.c_iflag = IGNPAR, term.c_cc[VMIN] = 1, term.c_cc[VTIME] = 0;
	if (115200 == baudrate)											cfsetispeed(&term, B115200), cfsetospeed(&term, B115200);
	else if (57600 == baudrate)										cfsetispeed(&term, B57600), cfsetospeed(&term, B57600);
	else if (38400 == baudrate)										cfsetispeed(&term, B38400), cfsetospeed(&term, B38400);
	else if (19200 == baudrate)										cfsetispeed(&term, B19200), cfsetospeed(&term, B19200);
	else if (9600 == baudrate)											cfsetispeed(&term, B9600), cfsetospeed(&term, B9600);
	else if (4800 == baudrate)											cfsetispeed(&term, B4800), cfsetospeed(&term, B4800);
	else																cfsetispeed(&term, B9600), cfsetospeed(&term, B9600);
	tcsetattr(fd, TCSANOW, &term); tcflush(fd, TCIFLUSH);
	v = fcntl(fd, F_GETFL, 0); fcntl(fd, F_SETFL, v | O_NONBLOCK);

	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &last_type);
	pthread_mutex_lock(&xUatrt);
	pthread_cleanup_push(CleanupLock, (void *)&xUatrt);
	pthread_setcanceltype(last_type, NULL);
	list_for_each_safe(P, Q, &uartQ.list)
	{
		N = list_entry(P, struct incoming_t, list); assert(0 != N);
		if (0 == strcmp(path, N->name))								{ f = 1; break; }
	}
	pthread_cleanup_pop(1);

	if (1 == f)														{ dbg_printf(DBG_UART, "(%s %d) 1 == f, CLOSE() fd=%d\n", __FILE__, __LINE__, fd); close(fd); return 0; }

	if (0 == (N = (struct incoming_t *)malloc(sizeof(struct incoming_t))))		{ dbg_printf(DBG_MALLOC, "(%s %d) MALLOC() FAIL\n", __FILE__, __LINE__); close(fd); return 0; }
	memset(N, 0, sizeof(struct incoming_t));
	N->fd = fd, strcpy(N->name, path);

	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &last_type);
	pthread_mutex_lock(&xUatrt);
	pthread_cleanup_push(CleanupLock, (void *)&xUatrt);
	pthread_setcanceltype(last_type, NULL);
	list_add_tail(&N->list, &uartQ.list);
	pthread_cleanup_pop(1);

	if (hUart)															SetEvent(hUart);
	return fd;
}

int CSd55App::TestUart(const char *path)
{
	struct stat lbuf;
	char pattern[128];
	int cflags = REG_EXTENDED;
	regex_t regx;
	int nmatch = 10;
	regmatch_t pmatch[10];

	if (0 == path)													return 0;
	memset(&regx, 0, sizeof(regex_t));
	strcpy(pattern, "^/dev/(tty[UMS]{1}[A-Z]{0,}[0-9]{1})$");
	if (0 != regcomp(&regx, pattern, cflags))								return 0;
	if (0 != regexec(&regx, path, nmatch, pmatch, 0))						{ dbg_printf(DBG_UART, "(%s %d) REGEXEC() FAIL\n", __FILE__, __LINE__); regfree(&regx); return 0; }
	regfree(&regx);

	if(0 > stat(path, &lbuf))											return 0;
	if(!S_ISCHR(lbuf.st_mode))											return 0;
	if(0 != access(path, R_OK))
	{
		printf("PLEASE RUN as ROOT\n");
		return 0;
	}
	return 1;
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSd55App::CSd55App() :CApp(), CFpcData()
{
	hUart = CreateEvent(0, 1, 0, "UART"), assert(0 != hUart);
	INIT_LIST_HEAD(&uartQ.list);

	tUart = 0;
	ttyUSB0 = -1;
}

CSd55App::~CSd55App()
{
	if (hUart)															CloseEvent(hUart);
}


//////////////////////////////////////////////////////////////////////
// message map & signal map
//////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CSd55App, CApp)
	//{{AFX_MSG_MAP(CSd55App)
	ON_MESSAGE(M_INIT_INSTANCE, &CSd55App::OnInitInstance)
	ON_MESSAGE(M_EXIT_INSTANCE, &CSd55App::OnExitInstance)
	ON_MESSAGE(M_UART_DATA_RCV, &CSd55App::OnUartDataRcv)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



//////////////////////////////////////////////////////////////////////
// member
//////////////////////////////////////////////////////////////////////







void CSd55App::SetCmd(unsigned char len, unsigned char cmd, ...)
{
	va_list vl;
	int i;
	unsigned char checksum;
	int cmdChar;

	if(len > Tx0_Buffer_Size) return;

	cc->txBuffer[0] = SD55_FRAME_HEADER;// Header
	cc->txBuffer[1] = len | (0xF0 - ( len << 4 ) );	// Data length
	cc->txBuffer[2] = cmd;	// Command

	va_start(vl, cmd);
	checksum = cc->txBuffer[1] + cc->txBuffer[2];

	for(i=3; i<1 + len; i++)
	{
		cmdChar = va_arg(vl, int);//to void warning, by Simon. 2012/11/14
		cc->txBuffer[i] = (unsigned char)cmdChar;// Data
		checksum += (unsigned char)cmdChar;
	}
	va_end(vl);

	cc->txBuffer[i++] = 0 - checksum;// Checksum
	cc->txLen = i;

	SerialUART_out();
}

void CSd55App::SerialUART_out(void)
{
	unsigned char i;
	for(i=0;i<Rx0_Buffer_Size;i++)cc->rxBuffer[i]=0;//by Simon.
	for(i=0;i<cc->txLen;i++)
	{
		if (-1 != ttyUSB0)
		{
			if (write(ttyUSB0, (const void*)&cc->txBuffer[i], 1) <= 0) {}
		}
	}
	data->sent_times++;
}

unsigned char CSd55App::SerialUART_in(unsigned char *buffer, unsigned char len)	//This CC is for SD55 communication control
{
	unsigned char i,ucChar;

	for(i=0;i<len;i++){
		//有資料		
		// 讀取數据
		ucChar = buffer[i];
		// 數据包頭
		if(cc->rxCount == 0)
		{
	 		if(ucChar != SD55_FRAME_HEADER)
				// 無效的數据包
				cc->rxCount = 0;
			// BUFFER滿,不接收數据
			else if(cc->rxCount < sizeof(cc->rxBuffer))
				cc->rxBuffer[cc->rxCount++] = ucChar;
		}else
			// 數据包長度			
			if(cc->rxCount == 1)
			{
				if((ucChar & 0x0F) + ((ucChar >> 4) & 0x0F) != 0x0F)
					// 包長度有誤
					cc->rxCount = 0;
				else
						// BUFFER滿,不接收數据
					if(cc->rxCount < sizeof(cc->rxBuffer))
						cc->rxBuffer[cc->rxCount++] = ucChar;
			}else
				// BUFFER滿,不接收數据
				if(cc->rxCount < sizeof(cc->rxBuffer))		
					cc->rxBuffer[cc->rxCount ++] = ucChar;
	}

	return i;
}


//////////////////////////////////////////////////////////////////////
// message handlers
//////////////////////////////////////////////////////////////////////

LRESULT CSd55App::OnInitInstance(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
	int res;

	CApp::OnInitInstance(dwType, wParam, lParam, pResult);

	if (!tUart)
	{
		res = pthread_create(&tUart, NULL, UartThread, this);
		dbg_printf(DBG_THREAD, "UartThread() OK\n");
	}
	usleep(1000 * 150);
	res = AttachUart("/dev/ttyS0", 19200);
	if (res)
	{
		dbg_printf(DBG_UART, "(%s, %d) /dev/ttyS0 Attach OK\n", __FILE__, __LINE__); ttyUSB0 = res;
		SetTimer(SOC_TO_MCU_SAY_HELLO, 8001, 0);
	}

	mProduct_type = getProduct_type();
	return 1;
}

LRESULT CSd55App::OnExitInstance(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
	struct list_head *P, *Q;
	struct incoming_t *N = 0;

	if (tUart)
	{
		pthread_cancel(tUart);
		dbg_printf(DBG_THREAD, "(%s %d) TRYING WAIT() tUart...,   ", __FILE__, __LINE__);
		pthread_join(tUart, NULL);
		dbg_printf(DBG_THREAD, "(%s %d) WAIT() tUart OK\n", __FILE__, __LINE__);
	}
	CApp::OnExitInstance(dwType, wParam, lParam, pResult);

	list_for_each_safe(P, Q, &uartQ.list)
	{
		N = list_entry(P, struct incoming_t, list); assert(0 != N); close(N->fd); list_del(P); free(N);
	}

	return 1;
}

void OnUartCharTest(int infd, unsigned char ch)
{
		CSd55App *app = (CSd55App *)AfxGetApp();
		static unsigned char mbuff[1024 * 2];
		static unsigned char *mp = mbuff;
		static unsigned char mLEN = 0;

		static unsigned char DESCRIPTOR;

		unsigned char *p;



		
		//printf("%d \n ",(mp - mbuff));
		if (0 == (mp - mbuff))
		{
			if (ch != FRAME_HEADER)
			{
				printf("Potocel Error 1\n");
				mLEN = 0; 
				mp = mbuff;
				return;
			}
			//printf("0X%02X-\n",ch);
			*mp++ = ch;
			return;
		}
		if (1 == (mp -mbuff))
		{
		if (ch != FRAME_HEADER)
		{
			printf("Potocel Error 2\n");
			mLEN = 0; mp = mbuff;
			return;
		}
		//printf("0X%02X-\n",ch);
		*mp++ = ch;
		return;
		}
		if (2 == (mp - mbuff))
		{
			//printf("0X%02X-\n",ch);
			*mp++ = ch;
			mLEN = ch ;
			return;
		}
		if (3 == (mp - mbuff))
		{
			//printf("0X%02X-\n",ch);
			*mp++ = ch;
			mLEN = BUILD_UINT16(ch,mLEN);
			return;
		}
		if (4 == (mp - mbuff))
		{
			//printf("0X%02X-\n",ch);
			*mp++ = ch;
			return;
		}
		if (5 == (mp - mbuff))
		{

			if (ch == 0)
			{
				printf("Is a REQ");
				mLEN = 0; mp = mbuff;
				return;
			}
			//printf("0X%02X-\n",ch);
			*mp++ = ch;
			return;
		}
		if (6 == (mp - mbuff))
		{
			//printf("0X%02X-\n",ch);
			*mp++ = ch;
			DESCRIPTOR = ch ;
			return;
		}
		if (7 == (mp - mbuff))
		{
			//printf("0X%02X-\n",ch);
			*mp++ = ch;
			DESCRIPTOR = BUILD_UINT16(ch,DESCRIPTOR);
			return;
		}

		if ((mp - mbuff) < (mLEN + 10))
		{
			//printf("0X%02X-\n",ch);
			*mp++ = ch;

			if((mp - mbuff) >=(mLEN + 10))
			{
				int x;
				p = (unsigned char *)malloc(mLEN+8);

				memcpy(p, mbuff, mLEN+8);
				
				
				x= ModbusCrc16(p,mLEN+8);
				//printf("0X%02X-0X%02X\n",mbuff[8],HI_UINT16(x));
				//printf("0X%02X-0X%02X\n",mbuff[9],LO_UINT16(x));
				if(mbuff[mLEN+8] ==HI_UINT16(x) && mbuff[mLEN+9] == LO_UINT16(x) )
				//if(1)
				{
					//printf("CRC16 Ok\n DESCRIPTOR =%d",DESCRIPTOR);


					switch(DESCRIPTOR)
					{

						default:
						break;

						case COMMAND_HELLO_RSP :
					//	printf("COMMAND_HELLO_RSP \n");
						break;
						case COMMAND_KEYSCAN_RSP :
					//	printf("COMMAND_KEYSCAN_RSP \n");
						MoveMCU_Key_ScanToUpdate_Key_Scan(mbuff[8],mbuff[9]);
						break;
						case COMMAND_POLAR_REP :
						
						app->hr->HeartRate_TEL =mbuff[8];
					//	printf("COMMAND_POLAR_RSP   %d\n",app->hr->HeartRate_TEL);
						break;
						case COMMAND_CHR_RSP :
						
						app->hr->HeartRate_HGP = mbuff[8];
					//	printf("COMMAND_CHR_RSP      %d\n",app->hr->HeartRate_HGP);
						break;
						case COMMAND_ADC_RSP :
					//	printf("COMMAND_ADC_RSP \n");
						break;
						case COMMAND_MOTO_PLUS_SET_RSP :
					//	printf("COMMAND_MOTO_PLUS_SET_RSP \n");
						break;
						case COMMAND_MOTO_MINUS_SET_RSP :
					//	printf("COMMAND_MOTO_MINUS_SET_RSP \n");
						break;
						case COMMAND_RPM_RSP :
						if(mProduct_type == 1)
						{
							if(mbuff[8] == 0)
							{
							 mRpmZeroCount++;
							}
							else
							{
							 mRpmZeroCount =0;
							}

							if(mRpmZeroCount <3 && mbuff[8] ==0)
							{
							 printf("COMMAND_RPM_RSP     %d\n",mbuff[8]);
							 break;
							}
							if(mLEN == 1)
							{
								if (mbuff[8] > 200)
									app->data->Rpm = 200;
								else
									app->data->Rpm = mbuff[8];
							}
							else if(mLEN == 2)
							{
								if (BUILD_UINT16(mbuff[9], mbuff[8]) > 200)
									app->data->Rpm = 200;
								else
									app->data->Rpm = BUILD_UINT16(mbuff[9], mbuff[8]);
							}
						//	printf("COMMAND_RPM_RSP     %d\n",app->data->Rpm);
						}
						break;
						case COMMAND_BREAK_PWM_SET_RSP :
					//	printf("COMMAND_BREAK_PWM_SET_RSP \n");
						break;
						case COMMAND_BUZZ_SET_RSP :
					//	printf("COMMAND_BUZZ_SET_RSP \n");
						break;
						case COMMAND_BACKLIGHT_ONOFF_SET_RSP :
					//	printf("COMMAND_BACKLIGHT_ONOFF_SET_RSP \n");
						break;
						case COMMAND_BACKLIGHT_PWM_SET_RSP :
					//	printf("COMMAND_BACKLIGHT_PWM_SET_RSP \n");
						break;
						case COMMAND_SERVO_MOTOR_BIKE_WORKLOAD_SET_RSP:
					//	printf("COMMAND_SERVO_MOTOR_BIKE_WORKLOAD_SET_RSP  \n");
						break;
						case COMMAND_SD55_RPM_RSP :

						if(mProduct_type == 0)
						{
							if (mbuff[8] > 200)
								app->data->Rpm = 200;
							else
								app->data->Rpm = mbuff[8];

						//	printf("COMMAND_SD55_RPM_RSP  %d \n",mbuff[8]);

						}
						
						break;

						case COMMAND_SERVO_MOTOR_BIKE_STATUS_RSP :
						if(mProduct_type == 1)	
						{
						MoveMCU_Servo_Motor_Bike_Status_To_FPC(mbuff[8]);	
						printf("COMMAND_SERVO_MOTOR_BIKE_STATUS_RSP  %d\n",mbuff[8]);	
						}
						break;	
						
						case COMMAND_SD55_ZERO_RSP :
					//	printf("COMMAND_SD55_ZERO_RSP \n");
						break;
						case COMMAND_SD55_STATUS_PSP :
						// [JayLee] test for get status data

						
						if(mProduct_type == 0)	
						{
						sd55_err_state_ack = 1;
						//app2->data->State = mbuff[9];
						MoveMCU_ErrorCode_To_Fpc(mbuff[9]);
						}
						//app2->data->State = mbuff[9];
					//	printf("COMMAND_SD55_STATUS_PSP  LEN = %d  data= %d\n" ,mLEN,mbuff[9] );
						break;
						case COMMAND_SD55_WORKLOAD_SET_RSP :
					//	printf("COMMAND_SD55_WORKLOAD_SET_RSP \n");
						break;
						case COMMAND_SD55_STRIDE_SET_RSP  :
					//	printf("COMMAND_SD55_STRIDE_SET_RSP  \n");
						break;
						case COMMAND_GET_MCU_VERSION  :
						
						MoveMCU_Get_Mcu_Version_To_FPC((char *)&mbuff[8]);
						break;

					}
					

					
				}
				else
				{
					printf("CRC16 Bad\n");
				}
				
				

				free(p);
				mLEN = 0; mp = mbuff;
				
			}
			
			

			return;
		}
		


		
	
		

}
void OnTtyUsbChar(int infd, unsigned char ch)
{
	CSd55App *app = (CSd55App *)AfxGetApp();

	static unsigned char buff[1024 * 2];
	static unsigned char *p = buff;
	static unsigned char LEN = 0;
	int cs = 0;
	int i;
	static unsigned char CMD;


	//F5 + D2 + 5A + SUM
	if (0 == (p - buff))
	{
		if (ch != SD55_FRAME_HEADER)
		{
			LEN = 0; p = buff;
			return;
		}
		CMD = nCmd;
		*p++ = ch;
		return;
	}
	if (1 == (p - buff))
	{
		*p++ = ch;
		LEN = LO_UINT8(ch);
		return;
	}

	if ((p - buff) < (LEN + 1))
	{
		*p++ = ch;
		return;
	}

///////////////////////////////////
	*p++ = ch;
	cs = 0;
	for (i = 0; i < LEN + 1; i++)
		cs += buff[i + 1];
	cs &= 0XFF;

	if (0 == cs)
	{
		sd55_ok = 1;

		switch (CMD)
		{
		default:
			break;

		case SERIAL_CMD_INIT:
			if(buff[2] != SD55_FRAME_YES)
			{
			}
			break;
		case SERIAL_RESET_CONTORLER:
			if(buff[2] != SD55_FRAME_YES){}
			break;

		case SERIAL_CMD_WR_RESIST_POS:		// 0x01
		case SERIAL_CMD_WR_STRIDE_POS:		// 0x03
		case SERIAL_CMD_WR_INCLINE_POS:	// 0x05
		case SERIAL_CMD_MOTOR_CTRL:		// 0x09
		case SERIAL_CMD_INIT_STEP:			// 0x0A
			if(buff[2] != SD55_FRAME_YES){}
			break;

		// 0x08 --> 讀狀態
		case SERIAL_CMD_RD_STATE:
			sd55_err_state_ack = 1;
			//app->data->State = buff[2];
			app->data->State = buff[3];
#if 0
Bit 0: 阻力??故障
Bit 1: 跨步?? 1 故障
Bit 2: 跨步?? 2 故障
Bit 3: 高度??故障
#endif

			break;

		// 0x06 --> 讀高度馬達位置
		case SERIAL_CMD_RD_INCLINE_POS:
			break;

		// 0x02 --> 讀阻力馬達位置
		case SERIAL_CMD_RD_RESIST_POS:
			app->data->ResistancePosition = buff[2];
			break;

		// 0x04 --> 讀跨步馬達位置
		case SERIAL_CMD_RD_STRIDE_POS:
			// [JayLee 2014-11-14] for auto stride
//printf("[JayLee <<%s>>(%d)] data=(%d)", __FILE__, __LINE__, buff[2]);
			app->data->StridePosition = buff[2];
			break;

		// 0x07 --> 讀 SPU INTERVAL
		case SERIAL_CMD_RD_SPU_INTERVAL:
/////////////////////////////////
			i = (int)(BUILD_UINT32(buff[5], buff[4], buff[3], buff[2]));
			if (i < 1000000000)		app->data->Drive_SPU_Interval = (unsigned long)i, app->cc->state.New_SPU_Pulse = 1;
			else					app->cc->state.New_SPU_Pulse = 0, app->data->Drive_SPU_Interval = 1000000000;
			//if(app->data->Drive_SPU_Interval)
			//	app->cc->state.New_SPU_Pulse = 1;
			break;

		// 0x0B --> 讀取步數
		case SERIAL_CMD_RD_STEP:
			i = BUILD_UINT32(buff[5], buff[4], buff[3], buff[2]);
			if (i < 32768)				app->data->Step = i;
			else						app->data->Step = 0;
			break;
		}
	}

	LEN = 0; p = buff;
	return;
}



LRESULT CSd55App::OnUartDataRcv(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
	int len;
	char path[128];
	unsigned char *buff = (unsigned char *)wParam + 64;

	memcpy(&len, (void *)wParam, sizeof(int));
	memset(path, 0, sizeof(path)), strcpy(path, ((char *)wParam) + sizeof(int));


	if (0 == wParam)
	{
		return 1;
	}
	pthread_cleanup_push(free, (void *)wParam);


	if (0 == strcmp(path, "/dev/ttyS0"))
	{

		for (int i = 0; i < len; i++)
		{


		OnUartCharTest((int)lParam, buff[i]);
		//OnTtyUsbChar((int)lParam, buff[i]);
		}
		//SerialUART_in(buff, len);
		goto baiout;
	}

baiout:
	pthread_cleanup_pop(1);
	return 1;
}

