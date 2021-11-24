// DorisApp.cpp: implementation of the CDorisApp class.
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
#include "App.h"
#include "DorisApp.h"


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

#define SERIAL_BUFFER_COUNT										8
#define SERIAL_NAME_SIZE											60
#define SERIAL_BUFFER_SIZE											128
#define INT_LEN														sizeof(int)

#define SOCKET_BUFFER_SIZE											(512 * 1024)
#define SERVICE_BUFFER_COUNT										8
#define SERVICE_BUFFER_SIZE										(1024 * 4)
#define SERVICE_LISTEN_PORT										34000



//////////////////////////////////////////////////////////////////////
// routines
//////////////////////////////////////////////////////////////////////

static int cfprintf(int fd, const char *fmt, ...)
{
	int len = 0;
	char buff[256];
	va_list ap;

	if (STDIN_FILENO == fd)											fd = STDOUT_FILENO;
	if (STDOUT_FILENO == fd)
	{
		va_start(ap, fmt);

#if 0
		vfprintf(stdout, fmt, ap);
#else
		printf(fmt, ap);
#endif // 0

		va_end(ap);
		return 1;
	}

	va_start(ap, fmt);
	len = sprintf(buff, fmt, ap);
	va_end(ap);
	if (len < 0)		return len;
	if (len >= 0)		len = write(fd, buff, len + 1);
	return len;
}


//static pthread_mutex_t xSvc= PTHREAD_MUTEX_INITIALIZER;
static struct incoming_t svcCliQ;


static void CleanupIncoming(void *param)
{
	struct incoming_t *pQ = (struct incoming_t *)param;
	struct list_head *P, *Q;
	struct incoming_t *N;

	list_for_each_safe(P, Q, &pQ->list)
	{
		N = list_entry(P, struct incoming_t, list); list_del(P); free(N);
	}
}

void *SvcThread(void *param)
{
	struct list_head *P, *Q;
	struct incoming_t *N;

	int maxfd = 0;
	int lsd = -1, fd, res, v = 0, len;
	struct sockaddr_in servaddr;
	fd_set rset;
	sigset_t nset, oset;

	unsigned char buff[1024 * 4];
	unsigned char *p;

	if (-1 == (lsd = socket(AF_INET, SOCK_STREAM, 0)))					{ dbg_printf(DBG_SOCKET, "(%s %d) SOCKET() FAIL\n", __FILE__, __LINE__); AfxGetApp()->PostMessage(M_EXIT_INSTANCE, 0, 0); return 0; }
	v = 1, setsockopt(lsd, SOL_SOCKET, SO_REUSEADDR, (const void *)&v, sizeof(v));
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET, servaddr.sin_addr.s_addr = htonl(INADDR_ANY); servaddr.sin_port = htons(SERVICE_LISTEN_PORT);
	if (0 > bind(lsd, (struct sockaddr *)&servaddr, sizeof(struct sockaddr_in)))	{ dbg_printf(DBG_SOCKET, "(%s %d) BIND() fail\n", __FILE__, __LINE__); AfxGetApp()->PostMessage(M_EXIT_INSTANCE, 0, 0); return 0; }
	if (0 > listen(lsd, 12))												{ dbg_printf(DBG_SOCKET, "(%s %d) LISTEN() fail\n", __FILE__, __LINE__); AfxGetApp()->PostMessage(M_EXIT_INSTANCE, 0, 0); return 0; }
	v = fcntl(lsd, F_GETFL, 0); fcntl(lsd, F_SETFL, v | O_NONBLOCK);
	v = SOCKET_BUFFER_SIZE, setsockopt(lsd, SOL_SOCKET, SO_SNDBUF, (const void *)&v, sizeof(v)); setsockopt(lsd, SOL_SOCKET, SO_RCVBUF, (const void *)&v, sizeof(v));
	pthread_cleanup_push(CleanupFd, (void *)&lsd);

	sigemptyset(&nset);
	sigaddset(&nset, SIGHUP); sigaddset(&nset, SIGINT); sigaddset(&nset, SIGTERM); sigaddset(&nset, SIGUSR1); sigaddset(&nset, SIGALRM);
	pthread_sigmask(SIG_BLOCK, &nset, &oset);
	pthread_cleanup_push(RestoreSigmask, (void *)&oset);

	pthread_cleanup_push(CleanupIncoming, (void *)&svcCliQ);

	for (;;)
	{
		int clilen, connfd;
		struct sockaddr_in cliaddr;

		maxfd = 0; FD_ZERO(&rset); FD_SET(lsd, &rset);
		if (maxfd < lsd)												maxfd = lsd;
		list_for_each_safe(P, Q, &svcCliQ.list)
		{
			N = list_entry(P, struct incoming_t, list); assert(0 != N); fd = N->fd; FD_SET(fd, &rset);
			if (maxfd < fd)											maxfd = fd;
		}

		if (0 > (res = select(maxfd + 1, &rset, 0, 0, 0)))					{ dbg_printf(DBG_SOCKET, "(%s %d) SELECT() FAIL\n", __FILE__, __LINE__); AfxGetApp()->PostMessage(M_EXIT_INSTANCE, 0, 0); break; }
		if (0 == res)													{ dbg_printf(DBG_SOCKET, "(%s %d) SELECT() TIMEOUT\n", __FILE__, __LINE__); continue; }

		if (FD_ISSET(lsd, &rset))
		{
			clilen = sizeof(cliaddr);
			connfd = accept(lsd, (struct sockaddr *)&cliaddr, (socklen_t *)&clilen);
			if (-1 != connfd)
			{
				v = fcntl(connfd, F_GETFL, 0); fcntl(connfd, F_SETFL, v | O_NONBLOCK);
				v = SOCKET_BUFFER_SIZE, setsockopt(connfd, SOL_SOCKET, SO_SNDBUF, (const void *)&v, sizeof(v)); setsockopt(connfd, SOL_SOCKET, SO_RCVBUF, (const void *)&v, sizeof(v));
				if (0 != (N = (struct incoming_t*)malloc(sizeof(struct incoming_t))))
				{
					memset(N, 0, sizeof(struct incoming_t)); N->fd = connfd, memcpy(&N->cliaddr, &cliaddr, clilen);
					list_add_tail(&N->list, &svcCliQ.list);
				}
				else													{ dbg_printf(DBG_SOCKET, "(%s %d) malloc() FAIL\n", __FILE__, __LINE__); close(connfd); }
			}
			else { }
		}
		list_for_each_safe(P, Q, &svcCliQ.list)
		{
			N = list_entry(P, struct incoming_t, list); assert(0 != N);
			fd = N->fd;
			if (!FD_ISSET(fd, &rset))									{ continue;}

			memset(buff, 0, sizeof(buff));
			if (0 > (len = read(fd, buff, sizeof(buff))))
			{
				if (errno == EAGAIN)									{ dbg_printf(DBG_SOCKET, "(%s %d) EAGAIN\n", __FILE__, __LINE__); continue; }
				if (errno == EWOULDBLOCK)							{ dbg_printf(DBG_SOCKET, "(%s %d) EWOULDBLOCK\n", __FILE__, __LINE__); continue; }
				if (errno == ECONNRESET || errno == EBADF)				{ dbg_printf(DBG_SOCKET, "(%s %d) ECONNRESET\n", __FILE__, __LINE__); close(fd); list_del(P); free(N); continue; }
				dbg_printf(DBG_SOCKET, "(%s %d) READ() FAIL, errno=%d\n", __FILE__, __LINE__, errno); AfxGetApp()->PostMessage(M_EXIT_INSTANCE, 0, 0); close(fd); list_del(P); free(N); break;
			}
			if (0 == len)												{ dbg_printf(DBG_SOCKET, "(%s %d) READ() EOF\n", __FILE__, __LINE__); close(fd); list_del(P); free(N); continue; }
			v = len + 64;
			p = (unsigned char *)malloc(len + 64);
			if(p)
			{
				memcpy(p, &v, sizeof(int));
				memcpy(p, buff + 64, len);
				AfxGetApp()->PostMessage(M_JACK_DATA_RCV, (WPARAM)p, (LPARAM)fd);
			}
			else { dbg_printf(DBG_MALLOC, "(%s %d) MALLOC() FAIL\n", __FILE__, __LINE__); }
		}
	}

	pthread_cleanup_pop(1);
	pthread_cleanup_pop(1);
	pthread_cleanup_pop(1);

dbg_printf(DBG_THREAD, "(%s %d) SvcThread() EXIT\n", __FILE__, __LINE__); 
	return 0;
}


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

pthread_mutex_t xUatrt = PTHREAD_MUTEX_INITIALIZER;
struct incoming_t uartQ;


void *UartThread(void *param)
{
	CDorisApp *app = (CDorisApp *)param;
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
//printf("UART READ RETURN\n");
					p = (unsigned char *)malloc(64 + len + 32);
					if (p)
					{
						memset(p, 0, 64 + len);
						memcpy(p, &len, sizeof(int));
						strcpy((char *)p + sizeof(int), (char *)N->name);
						memcpy(p + 64, buff, len);
						app->PostMessage(M_UART_DATA_RCV, (WPARAM)p, N->fd);
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
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDorisApp::CDorisApp() :CApp()
{
	hUart = CreateEvent(0, 1, 0, "UART"); assert(0 != hUart);
	INIT_LIST_HEAD(&svcCliQ.list);
	INIT_LIST_HEAD(&uartQ.list);

	tUart = 0; 
	tSvc = 0;
	ttyS0 = -1;
	ttyS1 = -1;
	ttyS2 = -1;
	ttyUSB0 = -1;
}

CDorisApp::~CDorisApp()
{
	if (hUart)															CloseEvent(hUart);
}

int CDorisApp::DetachUart(const char *path)
{
	struct list_head *P, *Q;
	struct incoming_t *N;
	int f = 0;
	int last_type;

	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &last_type);
	pthread_mutex_lock(&xUatrt);
	pthread_cleanup_push(CleanupLock, (void *)&xUatrt);
	pthread_setcanceltype(last_type, NULL);
	list_for_each_safe(P, Q, &uartQ.list)
	{
		N = list_entry(P, struct incoming_t, list); assert(0 != N);
		if (0 == strcmp(path, N->name))
		{
			f = 1;
			list_del(P); free(N);
			break;
		}
	}
	pthread_cleanup_pop(1);

	return f;
}

int CDorisApp::AttachUart(const char *path, int baudrate)
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

int CDorisApp::TestUart(const char *path)
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
// message map & signal map
//////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CDorisApp, CApp)
	//{{AFX_MSG_MAP(CDorisApp)
	ON_MESSAGE(M_INIT_INSTANCE, &CDorisApp::OnInitInstance)
	ON_MESSAGE(M_EXIT_INSTANCE, &CDorisApp::OnExitInstance)
	ON_MESSAGE(M_TIMER, &CDorisApp::OnTimer)
	ON_MESSAGE(M_SHELL, &CDorisApp::OnShell)
	ON_MESSAGE(M_SERVICE_DATA_RCV, &CDorisApp::OnServiceDataRcv)
	ON_MESSAGE(M_SIG_ALRM, &CDorisApp::OnAlrm)
	ON_MESSAGE(M_UART_DATA_RCV, &CDorisApp::OnUartDataRcv)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



//////////////////////////////////////////////////////////////////////
// message handlers
//////////////////////////////////////////////////////////////////////

LRESULT CDorisApp::OnAlrm(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
	return (*pResult = 1);
}

LRESULT CDorisApp::OnInitInstance(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
	int res = 0;

	CApp::OnInitInstance(dwType, wParam, lParam, pResult);

	res = pthread_create(&tUart, NULL, UartThread, this); assert(0 == res);
	dbg_printf(DBG_THREAD, "UartThread() OK\n");

	res = pthread_create(&tSvc, NULL, SvcThread, this); assert(0 == res);
	dbg_printf(DBG_THREAD, "SvcThread() OK\n");


	res = AttachUart("/dev/ttyUSB0", 38400);
	if (res)															{ dbg_printf(DBG_UART, "(%s, %d) /dev/ttyUSB0 Attach OK\n", __FILE__, __LINE__); ttyUSB0 = res; }


#if 0
	res = AttachUart("/dev/ttyS2", 38400);
	if (res)															{ ttyS2 = res; }
#endif


#if 0
	res = AttachUart("/dev/ttyS0", 4800);
	if (res)															{ dbg_printf(DBG_UART, "/dev/ttyS0 Attach OK\n"); ttyS0 = res;}
#endif


#if 0
	res = AttachUart("/dev/ttyS1", 38400);
	if (res)															{ ttyS1 = res; }
#endif


	return (*pResult = 1);
}

LRESULT CDorisApp::OnExitInstance(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
	struct list_head *P, *Q;
	struct incoming_t *N = 0;

	dbg_printf(DBG_BASIC, "CDorisApp::OnExitInstance()\n");

	if (tUart)															{ pthread_cancel(tUart); dbg_printf(DBG_THREAD, "(%s %d) TRYING WAIT() tUart...,   ", __FILE__, __LINE__); pthread_join(tUart, NULL); dbg_printf(DBG_THREAD, "(%s %d) WAIT() tUart OK\n", __FILE__, __LINE__); }
	//if (ttyS0 != -1)													{ dbg_printf(DBG_UART, "(%s %d) ttyS0 CLOSE(%d) \n", __FILE__, __LINE__, ttyS0); close(ttyS0); ttyS0 = -1;}
	//if (ttyS1 != -1)													{ dbg_printf(DBG_UART, "(%s %d) ttyS1 CLOSE(%d) \n", __FILE__, __LINE__, ttyS1); close(ttyS1); ttyS1 = -1;}
	//if (ttyS2 != -1)													{ dbg_printf(DBG_UART, "(%s %d) ttyS2 CLOSE(%d) \n", __FILE__, __LINE__, ttyS2); close(ttyS2); ttyS2 = -1;}
	//if (ttyUSB0 != -1)													{ dbg_printf(DBG_UART, "(%s %d) ttyUSB0 CLOSE(%d) \n", __FILE__, __LINE__, ttyUSB0); close(ttyUSB0); ttyUSB0 = -1;}
	list_for_each_safe(P, Q, &uartQ.list)
	{
		N = list_entry(P, struct incoming_t, list); assert(0 != N); close(N->fd); list_del(P); free(N);
	}

	if (tSvc)															{ pthread_cancel(tSvc); dbg_printf(DBG_THREAD, "(%s %d) TRYING WAIT() tSvc...,   ", __FILE__, __LINE__); pthread_join(tSvc, NULL); dbg_printf(DBG_THREAD, "(%s %d) WAIT() tSvc OK\n", __FILE__, __LINE__); }

	CApp::OnExitInstance(dwType, wParam, lParam, pResult);

	return (*pResult = 1);
}

LRESULT CDorisApp::OnTimer(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
	CApp::OnTimer(dwType, wParam, lParam, pResult);

	return (*pResult = 1);
}

LRESULT CDorisApp::OnShell(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
	char *incoming = ((char *)wParam + 64);
	int fd = (int)lParam;
	union sigval sv;
	FILE *pipe = 0;
	char reply[1024];
	int IL = 0;
	//char cmd[32];

	if (0 == wParam)
		return (*pResult = 1);

	pthread_cleanup_push(free, (void *)wParam);
	memcpy(&IL, ((char *)wParam), sizeof(int));


	if (0 == strncasecmp("QUIT", incoming, 4))
	{
		memset(&sv, 0, sizeof(sv));
		if (-1 == sigqueue(getpid(), SIGINT, sv))							cfprintf(fd, "(%s %d) sigqueue() fail\n", __FILE__, __LINE__);
		goto baiout;
	}

/*
	if (0 == strncasecmp("DEBUG ", incoming, 6))
	{
		unsigned long long int ll;

		if (2 != sscanf(incoming, "%s %llu", cmd, &ll))
		{
			cfprintf(fd, "...command error !!\n\ndoris#");
			goto baiout;
		}
		_debug = ll;
		cfprintf(fd, "_debug=%llu", _debug);
		cfprintf(fd, "doris#");
		goto baiout;
	}
*/


#if 0
	if(65 == IL && 0 == incoming[0])
	{
		cfprintf(fd, "doris#");
		goto baiout;
	}
	if(0 == incoming[0])
	{
		cfprintf(fd, "doris#");
		goto baiout;
	}
#endif


	pipe = popen(incoming, "r");
	if(pipe == 0)														{ dbg_printf(DBG_BASIC, "(%s %d) 0 == pipe\n", __FILE__, __LINE__); goto baiout; }
	while((fgets(reply, sizeof(reply), pipe)) != 0)							cfprintf(fd, reply);
	pclose(pipe);

	cfprintf(fd, "doris#");
	goto baiout;



/*
	char incoming[1024];
	int result = *pResult = 1;
	int fd = -1;
	union sigval sv;
	char reply[1024];
	int IL = 0;
	char cmd[32];
	char param[128];
	UINT id = 0, elapse = 0;

	memcpy(&IL, ((char *)wParam), sizeof(int));
	if (0 != IL)														memcpy(incoming, (void *)((char *)wParam + sizeof(int)), IL);
	else																memset(incoming, 0, sizeof(incoming));

	fd = (int)lParam;
	if (fd == STDIN_FILENO)											fd = STDOUT_FILENO;


///////////////////////
	if (0 == strncasecmp("QUIT", incoming, 4))
	{
		memset(&sv, 0, sizeof(sv));
		if (-1 == sigqueue(getpid(), SIGINT, sv))							cfprintf(fd, "(%s %d) sigqueue() fail\n", __FILE__, __LINE__);
		goto QUIT;
	}

	if (0 == strncasecmp("SETTIMER ", incoming, 9))
	{
		if (3 != sscanf(incoming, "%s %u %u", cmd, &id, &elapse))
		{
			cfprintf(fd, "example: SETTIMER 3 3000\ndoris#");
			goto QUIT;
		}
		if (0 >= id || 0 >= elapse)
		{
			cfprintf(fd, "example: SETTIMER 5 1000\ndoris#");
			goto QUIT;
		}
		AfxGetApp()->SetTimer(id, elapse);
		cfprintf(fd, "SETTIMER OK. \ndoris#");
		goto QUIT;
	}

	if (0 == strncasecmp("ALARM ", incoming, 6))
	{
		if (2 != sscanf(incoming, "%s %u", cmd, &elapse))
		{
			cfprintf(fd, "example: ALARM 10\ndoris#");
			goto QUIT;
		}
		alarm(elapse);
		if (elapse)													cfprintf(fd, "ALARM %u seconds OK. \ndoris#", elapse);
		else															cfprintf(fd, "ALARM CANCELLED. \ndoris#");
		goto QUIT;
	}

	if (0 == strncasecmp("BASH ", incoming, 5))
	{
		FILE *pipe = 0;

#if 0
		pipe = popen(strcat (incoming + 5, " 2>&1"), "r");
#else
		pipe = popen(incoming + 5, "rw");
#endif // 0

		if(pipe == 0)													goto QUIT;
		while((fgets(reply, sizeof(reply), pipe)) != 0)						cfprintf(fd, reply);
		pclose(pipe);

		cfprintf(fd, "\ndoris#");
		goto QUIT;
	}

	if (0 == strncasecmp("ATTACH ", incoming, 7))
	{
		int baud = 9600;

		if (3 != sscanf(incoming, "%s %s %d", cmd, param, &baud))
		{
			cfprintf(fd, "example: ATTACH /dev/tty/USB0 9600\ndoris#");
			goto QUIT;
		}
		if (0 >= AttachUart(param, baud))
			cfprintf(fd, "ATTACH FAIL. \ndoris#");
		else
			cfprintf(fd, "ATTACH OK.   \ndoris#");
		goto QUIT;
	}

	if (0 == strncasecmp("SIGUSR1", incoming, 7))
	{
		static UINT counter = 0;

		sv.sival_ptr = (void *)malloc(sizeof(UINT));
		memcpy(sv.sival_ptr, &counter, sizeof(UINT));
		counter++;
		if (-1 == sigqueue(getpid(), SIGUSR1, sv))			printf("(%s %d) sigqueue fail\n", __FILE__, __LINE__), exit(1);

		cfprintf(fd, "CDorisApp::OnShell() SIGUSR1\nDORIS$\n");
		goto QUIT;
	}

	if (IL == 5)
	{
		cfprintf(fd, "doris#");
		goto QUIT;
	}


	cfprintf(fd, "...command error !!\n\ndoris#");


QUIT:

	return result;
*/


baiout:

	pthread_cleanup_pop(1);
	return (*pResult = 1);
}

LRESULT CDorisApp::OnServiceDataRcv(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
	if (0 == wParam)
		return (*pResult = 1);

	pthread_cleanup_push(free, (void *)wParam);



	pthread_cleanup_pop(1);
	return (*pResult = 1);
}

LRESULT CDorisApp::OnUartDataRcv(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
	if (0 == wParam)		return (*pResult = 1);

	pthread_cleanup_push(free, (void *)wParam);



	pthread_cleanup_pop(1);
	return (*pResult = 1);
}

