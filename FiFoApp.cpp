// FifoApp.cpp: implementation of the CFifoApp class.
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
#include <assert.h>
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
#include <netinet/in.h>
#include <netdb.h>

#include <asm/ioctl.h>
#include <asm/errno.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#include <stdbool.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>


#include "list.h"
#include "Event.h"
#include "tree.hh"
#include "Misc.h"

#include "App.h"
#include "DorisApp.h"
#include "FiFoApp.h"


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

#define FILE_MODE													(S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)
#define DORIS_FIFO_PATH											"/mnt/asec/"
#define DORIS_FIFO													"/mnt/asec/doris.fifo"
#define CONN_HOST													"127.0.0.1"
#define CONN_PORT													9366
#define SOCKET_BUFFER_SIZE											(512 * 1024)
#define I2C0_BUS_ADDR												0X5D
#define I2C1_BUS_ADDR												0X5D

#define JACK_SERVICE_LISTEN_PORT									8366

#define CMD_ACK_BIT													14
#define CMD_UART													1
#define CMD_I2C														2
#define CMD_SPI														3
#define CMD_USB_UART												4
#define CMD_GPIO													5
#define CMD_FPC														32
#define CMD_GUI														33
#define CMD_IP														63




//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

static pthread_mutex_t xJ = PTHREAD_MUTEX_INITIALIZER;
static struct incoming_t jackCliQ;


static void CleanupIncoming(void *param)
{
	struct incoming_t *pQ = (struct incoming_t *)param;
	struct list_head *P, *Q;
	struct incoming_t *N;

	list_for_each_safe(P, Q, &pQ->list)
	{
		N = list_entry(P, struct incoming_t, list); assert(0 != N); list_del(P); free(N);
	}
}

static int pack(unsigned char ch, unsigned char cl, int len, unsigned char *data)
{
	unsigned char p[1024 * 3];

	*(p + 0) = 0X05;
	*(p + 1) = 0X05;
	*(p + 2) = ch;
	*(p + 3) = cl;
	*(p + 4) = HI_UINT16(len);
	*(p + 5) = LO_UINT16(len);
	memcpy(p + 6, (void *)data, len);
	*(p + 6 + len) = 0X50;
	*(p + 7 + len) = 0X50;

	memcpy(data, p, len + 8);
	return (len + 8);
}

void *JackSvcThread(void *param)
{
	struct list_head *P, *Q;
	struct incoming_t *N;
	int fd;
	//CFifoApp *app = (CFifoApp *)AfxGetApp();
	int maxfd = 0;
	int lsd = -1, res, v = 0;
	struct sockaddr_in servaddr;
	fd_set rset;
	sigset_t nset, oset;

	unsigned short LEN = 0;
	int len = 0;
	unsigned char *frame;

	unsigned char *p;
	int left = 1, offs = 0;
	int last_type;

	if (-1 == (lsd = socket(AF_INET, SOCK_STREAM, 0)))					{ dbg_printf(DBG_SOCKET, "(%s %d) SOCKET() FAIL\n", __FILE__, __LINE__); AfxGetApp()->PostMessage(M_EXIT_INSTANCE, 0, 0); return 0; }
	v = 1, setsockopt(lsd, SOL_SOCKET, SO_REUSEADDR, (const void *)&v, sizeof(v));
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET, servaddr.sin_addr.s_addr = htonl(INADDR_ANY), servaddr.sin_port = htons(JACK_SERVICE_LISTEN_PORT);
	if (0 > bind(lsd, (struct sockaddr *)&servaddr, sizeof(struct sockaddr_in)))	{ dbg_printf(DBG_SOCKET, "(%s %d) BIND() fail\n", __FILE__, __LINE__); AfxGetApp()->PostMessage(M_EXIT_INSTANCE, 0, 0); return 0; }
	if (0 > listen(lsd, 12))												{ dbg_printf(DBG_SOCKET, "(%s %d) LISTEN() fail\n", __FILE__, __LINE__); AfxGetApp()->PostMessage(M_EXIT_INSTANCE, 0, 0); return 0; }
	v = fcntl(lsd, F_GETFL, 0); fcntl(lsd, F_SETFL, v | O_NONBLOCK);
	v = SOCKET_BUFFER_SIZE; setsockopt(lsd, SOL_SOCKET, SO_SNDBUF, (const void *)&v, sizeof(v)); setsockopt(lsd, SOL_SOCKET, SO_RCVBUF, (const void *)&v, sizeof(v));

	pthread_cleanup_push(CleanupFd, (void *)&lsd);

	sigemptyset(&nset);
	sigaddset(&nset, SIGHUP), sigaddset(&nset, SIGINT), sigaddset(&nset, SIGTERM), sigaddset(&nset, SIGUSR1), sigaddset(&nset, SIGALRM);
	pthread_sigmask(SIG_BLOCK, &nset, &oset);
	pthread_cleanup_push(RestoreSigmask, (void *)&oset);

	pthread_cleanup_push(CleanupIncoming, (void *)&jackCliQ);

	for (;;)
	{
		int clilen, connfd;
		struct sockaddr_in cliaddr;

		maxfd = 0; FD_ZERO(&rset); FD_SET(lsd, &rset);
		if (maxfd < lsd)												maxfd = lsd;

		pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &last_type);
		pthread_mutex_lock(&xJ);
		pthread_cleanup_push(CleanupLock, (void *)&xJ);
		pthread_setcanceltype(last_type, NULL);

		list_for_each_safe(P, Q, &jackCliQ.list)
		{
			N = list_entry(P, struct incoming_t, list); assert(0 != N); fd = N->fd; FD_SET(fd, &rset);
			if (maxfd < fd)											maxfd = fd;
		}
		pthread_cleanup_pop(1);

		if (0 > (res = select(maxfd + 1, &rset, 0, 0, 0)))					{ dbg_printf(DBG_SOCKET, "(%s %d) SELECT() FAIL\n", __FILE__, __LINE__); AfxGetApp()->PostMessage(M_EXIT_INSTANCE, 0, 0); break; }
		if (0 == res)													{ dbg_printf(DBG_SOCKET, "(%s %d) SELECT() TIMEOUT\n", __FILE__, __LINE__); continue; }

		if (FD_ISSET(lsd, &rset))
		{
			clilen = sizeof(cliaddr);
			connfd = accept(lsd, (struct sockaddr *)&cliaddr, (socklen_t *)&clilen);
			if (-1 != connfd)
			{
				v = fcntl(connfd, F_GETFL, 0), fcntl(connfd, F_SETFL, v | O_NONBLOCK);
				v = SOCKET_BUFFER_SIZE, setsockopt(connfd, SOL_SOCKET, SO_SNDBUF, (const void *)&v, sizeof(v)), setsockopt(connfd, SOL_SOCKET, SO_RCVBUF, (const void *)&v, sizeof(v));
				if (0 != (N = (struct incoming_t *)malloc(sizeof(struct incoming_t) + 1024)))
				{
					dbg_printf(DBG_SOCKET, "(%s %d) ACCEPT()\n", __FILE__, __LINE__);

					memset(N, 0, sizeof(struct incoming_t)); N->fd = connfd, memcpy(&N->cliaddr, &cliaddr, clilen); N->left = 1; N->offs = 0;


					pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &last_type);
					pthread_mutex_lock(&xJ);
					pthread_cleanup_push(CleanupLock, (void *)&xJ);
					pthread_setcanceltype(last_type, NULL);
					list_add_tail(&N->list, &jackCliQ.list);
					pthread_cleanup_pop(1);
				}
				else													{ dbg_printf(DBG_MALLOC, "(%s %d) MALLOC() FAIL\n", __FILE__, __LINE__); close(connfd); }
			}
			else { dbg_printf(DBG_MALLOC, "(%s %d) ACCEPT() FAIL\n", __FILE__, __LINE__); }
			continue;
		}


		pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &last_type);
		pthread_mutex_lock(&xJ);
		pthread_cleanup_push(CleanupLock, (void *)&xJ);
		pthread_setcanceltype(last_type, NULL);

		list_for_each_safe(P, Q, &jackCliQ.list)
		{
			N = list_entry(P, struct incoming_t, list); assert(0 != N);
			fd = N->fd;
			if (!FD_ISSET(fd, &rset))									{ continue; }

			left = N->left, offs = N->offs;
			p = N->msg + offs;
			if (0 == offs || 1 == offs || 2 == offs || 3 == offs)				left = 1;
			if (4 == offs || 5 == offs)									left = 1;
			if (6 == offs)
			{
				LEN = BUILD_UINT16(N->msg[5], N->msg[4]);
				if (LEN > 1000)
				{
					dbg_printf(DBG_SOCKET, "(%s %d) PROTOCOL MISMACH\n", __FILE__, __LINE__);
					N->offs = 0; N->left = 1;
					//continue;
				}
				left = N->left = (LEN + 2);
			}

			if (0 > (len = read(fd, p, left)))
			{
				if (errno == EAGAIN)									{ dbg_printf(DBG_SOCKET, "(%s %d) READ() EAGAIN\n", __FILE__, __LINE__); continue;}
				if (errno == EWOULDBLOCK)							{ dbg_printf(DBG_SOCKET, "(%s %d) READ() EWOULDBLOCK\n", __FILE__, __LINE__); continue;}
				if (errno == ECONNRESET || errno == EBADF)				{ dbg_printf(DBG_SOCKET, "(%s %d) READ() ECONNRESET\n", __FILE__, __LINE__); close(fd); list_del(P); free(N); continue; }

				dbg_printf(DBG_SOCKET, "(%s %d) READ() FAIL, errno=%d\n", __FILE__, __LINE__, errno); AfxGetApp()->PostMessage(M_EXIT_INSTANCE, 0, 0); close(fd); list_del(P); free(N); break;
			}
			if (0 == len)												{ dbg_printf(DBG_SOCKET, "(%s %d) READ() EOF\n", __FILE__, __LINE__); close(fd); list_del(P); free(N); continue; }

			if (0 == offs && 0X05 != N->msg[0])							{ dbg_printf(DBG_SOCKET, "(%s %d) PROTOCOL MISMACH\n", __FILE__, __LINE__); N->offs = 0; N->left = 1; continue; }
			if (1 == offs && 0X05 != N->msg[1])							{ dbg_printf(DBG_SOCKET, "(%s %d) PROTOCOL MISMACH\n", __FILE__, __LINE__); N->offs = 0; N->left = 1; continue; }

			offs += len, N->offs = offs;
			left -= len, N->left = left;
			if (offs >= 8 && left <= 0)
			{
				LEN = BUILD_UINT16(N->msg[5], N->msg[4]);
				if (0X50 == N->msg[LEN + 6] && 0X50 == N->msg[LEN + 7])
				{
					v = LEN + 8;// + 64;
					frame = (unsigned char *)malloc(LEN + 8 + 64);
					if (frame)
					{
						memcpy(frame, &v, sizeof(int));
						memcpy(frame + 64, N->msg, LEN + 8);
						AfxGetApp()->PostMessage(M_JACK_DATA_RCV, (WPARAM)frame, (LPARAM)fd);
					}
					else
					{
						dbg_printf(DBG_MALLOC, "(%s %d) MALLOC() FAIL\n", __FILE__, __LINE__);
					}
				}
				N->offs = 0; N->left = 1;
			}
			else
			{
				//printf("(%s %d) offs=%d left=%d\n", __FILE__, __LINE__, offs, left);
			}

		}
		pthread_cleanup_pop(1);

	}
	pthread_cleanup_pop(1);
	pthread_cleanup_pop(1);
	pthread_cleanup_pop(1);

dbg_printf(DBG_THREAD, "(%s %d) JackSvcThread() EXIT\n", __FILE__, __LINE__); 
	return 0;
}

void *ConnThread(void *param)
{
	int connfd = -1;
	unsigned char buff[1024];
	int n;
	unsigned char *p;
	fd_set rset;
	int v;
	int res;


RECONN:
	connfd = CONN("127.0.0.1", CONN_PORT);
	if (connfd < 0)													{ sleep(6); goto RECONN; }
	v = fcntl(connfd, F_GETFL, 0), fcntl(connfd, F_SETFL, v | O_NONBLOCK);
	v = SOCKET_BUFFER_SIZE, setsockopt(connfd, SOL_SOCKET, SO_SNDBUF, (const void *)&v, sizeof(v)), setsockopt(connfd, SOL_SOCKET, SO_RCVBUF, (const void *)&v, sizeof(v));

READ:
	FD_ZERO(&rset); FD_SET(connfd, &rset);
	if (0 > (res = select(connfd + 1, &rset, 0, 0, 0)))						{ close(connfd); sleep(6); goto RECONN; }
	if (0 == res)														{ dbg_printf(DBG_SOCKET, "(%s %d) SELECT() TIMEOUT\n", __FILE__, __LINE__); goto READ; }
	if (!FD_ISSET(connfd, &rset))										{ dbg_printf(DBG_SOCKET, "(%s %d) !FD_ISSET(connfd)\n", __FILE__, __LINE__); goto READ; }

	memset(buff, 0, sizeof(buff));
	n = read(connfd, buff, sizeof(buff));
	if (n < 0 && EAGAIN == errno)										{ dbg_printf(DBG_SOCKET, "(%s %d) EAGAIN\n", __FILE__, __LINE__); goto READ; }
	if (n < 0 && EWOULDBLOCK == errno)								{ dbg_printf(DBG_SOCKET, "(%s %d) EWOULDBLOCK\n", __FILE__, __LINE__); goto READ; }
	if (n <= 0)
	{
		if (0 == n)													dbg_printf(DBG_SOCKET, "(%s %d) READ() EOF\n", __FILE__, __LINE__);
		else															dbg_printf(DBG_SOCKET, "(%s %d) READ() FAIL\n", __FILE__, __LINE__);
		close(connfd); sleep(6);
		goto RECONN;
	}
	p = (unsigned char *)malloc(n + 64);
	if (p)
	{
		memcpy(p, (const void *)&n, sizeof(int));
		memcpy(p, buff + 64, n);
		AfxGetApp()->PostMessage(M_CONN_DATA_RCV, (WPARAM)p, (LPARAM)connfd);
	}
	else { dbg_printf(DBG_MALLOC, "(%s %d) MALLOC() FAIL\n", __FILE__, __LINE__); }

	goto READ;

////////////////////////


dbg_printf(DBG_THREAD, "(%s %d) ConnThread() EXIT\n", __FILE__, __LINE__); 
	return 0;
}


void *FIFOThread(void *param)
{
	CFifoApp *app;

	fd_set rset;
	int fifo;
	unsigned char buff[1024 * 4];
	unsigned char *frame;
	int len;
	unsigned short LEN = 0;
	//int last_type;
	int v = 1;
	int res;
	sigset_t nset, oset;


	app = (CFifoApp *)AfxGetApp();
	fifo = app->fifo;

	sigemptyset(&nset);
	sigaddset(&nset, SIGHUP), sigaddset(&nset, SIGINT), sigaddset(&nset, SIGTERM), sigaddset(&nset, SIGUSR1), sigaddset(&nset, SIGALRM);
	pthread_sigmask(SIG_BLOCK, &nset, &oset);
	pthread_cleanup_push(RestoreSigmask, (void *)&oset);

	//pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &last_type);
	//pthread_setcanceltype(last_type, NULL);


#ifdef F_SETPIPE_SZ
	fcntl(fifo, F_SETPIPE_SZ, 32 * 1024);
#endif // F_SETPIPE_SZ


	v = fcntl(fifo, F_GETFL, 0), fcntl(fifo, F_SETFL, v | O_NONBLOCK);

	for (;;)
	{
		//pthread_setcanceltype(last_type, NULL);

		fifo = app->fifo;

		FD_ZERO(&rset);
		FD_SET(fifo, &rset);
		if (0 > (res = select(fifo + 1, &rset, 0, 0, 0)))						{ dbg_printf(DBG_SOCKET, "(%s %d) SELECT() FAIL \n", __FILE__, __LINE__); AfxGetApp()->PostMessage(M_EXIT_INSTANCE, 0, 0); break; }
		if (0 == res)													{ dbg_printf(DBG_SOCKET, "(%s %d) SELECT() TIMEOUT\n", __FILE__, __LINE__); continue; }
		if (!FD_ISSET(fifo, &rset))										{ dbg_printf(DBG_SOCKET, "(%s %d) !FD_ISSET(FIFO)\n", __FILE__, __LINE__); continue; }

		memset(buff, 0, sizeof(buff));
		len = read(fifo, buff, sizeof(buff));
		if (len < 0 && EAGAIN == errno)									{ dbg_printf(DBG_SOCKET, "(%s %d) EAGAIN\n", __FILE__, __LINE__); continue; }
		if (len < 0 && EWOULDBLOCK == errno)							{ dbg_printf(DBG_SOCKET, "(%s %d) EWOULDBLOCK\n", __FILE__, __LINE__); continue; }
		if (len <= 0)
		{
			if(0 == len)												dbg_printf(DBG_SOCKET, "(%s %d) READ() EOF\n", __FILE__, __LINE__); 
			else														dbg_printf(DBG_SOCKET, "(%s %d) READ() FAIL, errno=%d\n", __FILE__, __LINE__, errno);
			AfxGetApp()->PostMessage(M_EXIT_INSTANCE, 0, 0); break;
		}

		//pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &last_type);

		LEN = BUILD_UINT16(buff[5], buff[4]);
		if (len != LEN + 8)												{ dbg_printf(DBG_JACK, "(%s %d) PROTOCOL MISMATCH\n", __FILE__, __LINE__); continue; }
		if (0X05 != buff[0] || 0X05 != buff[1] || 0X50 != buff[LEN + 6] || 0X50 != buff[LEN + 7])
			{ dbg_printf(DBG_BASIC, "(%s %d) PROTOCOL MISMATCH\n", __FILE__, __LINE__); continue; }

		v = 64+ 8 + LEN;
		frame = (unsigned char *)malloc(64+ 8 + LEN + 128);//, assert(0 != frame);
		if(frame)
		{
			memcpy(frame, &v, sizeof(v));
			memcpy(frame + 64, buff, 8 + LEN);
			AfxGetApp()->PostMessage(M_FIFO, (WPARAM)frame, (LPARAM)fifo);
		}
		else { dbg_printf(DBG_MALLOC, "(%s %d) MALLOC() FAIL\n", __FILE__, __LINE__); }

	}

	pthread_cleanup_pop(1);

dbg_printf(DBG_THREAD, "(%s %d) FIFOThread() EXIT\n", __FILE__, __LINE__); 
	return 0;
}


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

CFifoApp::CFifoApp() : CDorisApp()
{
	INIT_LIST_HEAD(&jackCliQ.list);
	fifo = -1;
	tFifo = 0;
	tConn = 0;
	tJack = 0;
	i2c0 = -1;
	i2c1 = -1;
	spi0 = -1;
}

CFifoApp::~CFifoApp()
{
}


//////////////////////////////////////////////////////////////////////
// message map & signal map
//////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CFifoApp, CDorisApp)
	//{{AFX_MSG_MAP(CFifoApp)
	ON_MESSAGE(M_INIT_INSTANCE, &CFifoApp::OnInitInstance)
	ON_MESSAGE(M_EXIT_INSTANCE, &CFifoApp::OnExitInstance)
	ON_MESSAGE(M_TIMER, &CFifoApp::OnTimer)
	ON_MESSAGE(M_FIFO, &CFifoApp::OnFIFO)
	ON_MESSAGE(M_FIFO, &CFifoApp::OnConnDataRcv)
	ON_MESSAGE(M_JACK_DATA_RCV, &CFifoApp::OnJackDataRcv)
	ON_MESSAGE(M_UART_DATA_RCV, &CFifoApp::OnUartDataRcv)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



//////////////////////////////////////////////////////////////////////
// message handlers
//////////////////////////////////////////////////////////////////////

LRESULT CFifoApp::OnInitInstance(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
	struct stat lbuf;
	int res;

	dbg_printf(DBG_INIT, "CFifoApp::OnInitInstance()\n");
	CDorisApp::OnInitInstance(dwType, wParam, lParam, pResult);

	i2c0 = open("/dev/i2c-0", O_RDWR);
	i2c1 = open("/dev/i2c-1", O_RDWR);
	spi0 = open("/dev/spg", O_RDWR);

	if(0 > stat(DORIS_FIFO_PATH, &lbuf))
	{
		if (0 != mkdir(DORIS_FIFO_PATH, 0666))							{ dbg_printf(DBG_BASIC, "(%s %d) MKDIR() != 0\n", __FILE__, __LINE__); AfxGetApp()->PostMessage(M_EXIT_INSTANCE, 0, 0); return 0; }
	}
	else
	{
		if(!S_ISDIR(lbuf.st_mode))										{ dbg_printf(DBG_BASIC, "(%s %d) !S_ISDIR()\n", __FILE__, __LINE__); AfxGetApp()->PostMessage(M_EXIT_INSTANCE, 0, 0); return 0; }
	}

	if(0 > stat(DORIS_FIFO, &lbuf))
	{
		if (0 > mkfifo(DORIS_FIFO, FILE_MODE) && errno != EEXIST)			{ dbg_printf(DBG_BASIC, "MKFIFO /tmp/doris.fifo FAIL !!\n"); AfxGetApp()->PostMessage(M_EXIT_INSTANCE, 0, 0); return 0; }
	}
	else
	{
		if(!S_ISFIFO(lbuf.st_mode))										{ dbg_printf(DBG_BASIC, "(%s %d) !S_ISFIFO()\n", __FILE__, __LINE__); AfxGetApp()->PostMessage(M_EXIT_INSTANCE, 0, 0); return 0; }
	}
	fifo = open(DORIS_FIFO, O_RDWR | O_NDELAY, 0);
	if(fifo == -1)														{ dbg_printf(DBG_BASIC, "OPEN FIFO FAIL !!\n"); AfxGetApp()->PostMessage(M_EXIT_INSTANCE, 0, 0); return 0; }

	res = pthread_create(&tFifo, NULL, FIFOThread, this), assert(0 == res);
	dbg_printf(DBG_THREAD, "FIFOThread() OK\n");

	res = pthread_create(&tConn, NULL, ConnThread, this), assert(0 == res);
	dbg_printf(DBG_THREAD, "ConnThread() OK\n");

	res = pthread_create(&tJack, NULL, JackSvcThread, this), assert(0 == res);
	dbg_printf(DBG_THREAD, "JackSvcThread() OK\n");

	return (*pResult = 1);
}

LRESULT CFifoApp::OnExitInstance(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
	dbg_printf(DBG_BASIC, "CFifoApp::OnExitInstance()\n");

	if (tFifo)															{ pthread_cancel(tFifo); dbg_printf(DBG_THREAD, "(%s %d) TRYING WAIT() tFifo...,   ", __FILE__, __LINE__); pthread_join(tFifo, NULL); dbg_printf(DBG_THREAD, "(%s %d) WAIT() tFifo OK\n", __FILE__, __LINE__); }
	if (-1 != fifo)														close(fifo), fifo = -1, unlink(DORIS_FIFO);

	if (tConn)														{ pthread_cancel(tConn); dbg_printf(DBG_THREAD, "(%s %d) TRYING WAIT() tConn...,   ", __FILE__, __LINE__); pthread_join(tConn, NULL); dbg_printf(DBG_THREAD, "(%s %d) WAIT() tConn OK\n", __FILE__, __LINE__); }

	if (tJack)															{ pthread_cancel(tJack); dbg_printf(DBG_THREAD, "(%s %d) TRYING WAIT() tJack...,   ", __FILE__, __LINE__); pthread_join(tJack, NULL); dbg_printf(DBG_THREAD, "(%s %d) WAIT() tJack OK\n", __FILE__, __LINE__);}

	if (i2c0 != -1)													close(i2c0), i2c0 = -1;
	if (i2c1 != -1)													close(i2c1), i2c1 = -1;
	if (spi0 != -1)													close(spi0), spi0 = -1;

	CDorisApp::OnExitInstance(dwType, wParam, lParam, pResult);

	return (*pResult = 1);
}

LRESULT CFifoApp::OnTimer(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
	unsigned char ack[] = {0X05, 0X05, 0, 0, 0, 0, 0X50, 0X50, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	int left, offs, l;
	struct list_head *P, *Q;
	struct incoming_t *N;
	int fd;
	int len;
	unsigned char *p;

	/*if (5400 == (UINT)wParam)
	{
		int res;

		ttyUSB0 = -1;
		DetachUart("/dev/ttyUSB0");
		res = AttachUart("/dev/ttyUSB0", 38400);
		if (res)															{ dbg_printf(DBG_UART, "(%s, %d) /dev/ttyUSB0 Attach OK\n", __FILE__, __LINE__); ttyUSB0 = res; }
		//else { SetTimer(5400, 1000, 0); }
		return (*pResult = 1);
	}*/

	if (201 == (UINT)wParam)
	{
		//int res;

		KillTimer((UINT)wParam);

/*
		res = DetachUart("/dev/ttyUSB0");
		if (0 != res)
		{
			res = AttachUart("/dev/ttyUSB0", 38400);
			if (res)
			{
				ttyUSB0 = res;
				dbg_printf(DBG_UART, "(%s, %d) AttchUart(\"/dev/ttyUSB0\") OK \n", __FILE__, __LINE__);
			}
			else	
			{
				ttyUSB0 = -1;
				dbg_printf(DBG_UART, "(%s, %d) AttchUart(\"/dev/ttyUSB0\") FAIL \n", __FILE__, __LINE__);
			}
		}
		else
		{
			dbg_printf(DBG_UART, "(%s, %d) DetachUart(\"/dev/ttyUSB0\") FAIL \n", __FILE__, __LINE__);
		}
*/

		len = pack(CMD_GUI | (1 << 6), 0, 0, ack);

		pthread_mutex_lock(&xJ);
		pthread_cleanup_push(CleanupLock, (void *)&xJ);
		list_for_each_safe(P, Q, &jackCliQ.list)
		{
			N = list_entry(P, struct incoming_t, list); assert(0 != N); fd = N->fd;
			p = ack, left = len, offs = 0;
			while(left > 0)
			{
				l = write(fd, (const void*)(p + offs), left);
				if (l <= 0)	break;
				left -= l, offs +=l;
			}
		}
		pthread_cleanup_pop(1);

		return (*pResult = 1);
	}

	return (*pResult = 1);
}

LRESULT CFifoApp::OnConnDataRcv(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
	if (0 == wParam)
		return (*pResult = 1);

	pthread_cleanup_push(free, (void *)wParam);



	pthread_cleanup_pop(1);
	return (*pResult = 1);
}

// OnFpcDataRcv
LRESULT CFifoApp::OnFIFO(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
	struct list_head *P, *Q;
	struct incoming_t *N;
	int fd;

	unsigned char *frame;
	unsigned short cmd;
	unsigned char cmdh, cmdl;
	unsigned short LEN;
	unsigned char ack[] = {0X05, 0X05, 0, 0, 0, 0, 0X50, 0X50};
	int res;

	unsigned char *p;
	int l;

	if (0 == wParam)
		return (*pResult = 1);

	pthread_cleanup_push(free, (void *)wParam);

	frame = ((unsigned char *)wParam) + 64;
	cmdh = frame[2], cmdl = frame[3]; 
	cmd = BUILD_UINT16(cmdh, cmdl);
	LEN = BUILD_UINT16(frame[5], frame[4]);

	if (cmdh == CMD_FPC)												goto baiout;

	if (cmd & (1 << CMD_ACK_BIT))
	{
		pthread_mutex_lock(&xJ);
		pthread_cleanup_push(CleanupLock, (void *)&xJ);
		list_for_each_safe(P, Q, &jackCliQ.list)
		{
			N = list_entry(P, struct incoming_t, list); assert(0 != N);; fd = N->fd;
			res = write(fd, (const void *)frame, (size_t)(LEN + 8));
		}
		pthread_cleanup_pop(1);
		goto baiout;
	}

	if (cmdh == CMD_GUI)
	{
		pthread_mutex_lock(&xJ);
		pthread_cleanup_push(CleanupLock, (void *)&xJ);
		list_for_each_safe(P, Q, &jackCliQ.list)
		{
			N = list_entry(P, struct incoming_t, list); assert(0 != N); fd = N->fd;
			res = write(fd, (const void *)frame, (size_t)(LEN + 8));
		}
		pthread_cleanup_pop(1);
		goto baiout;
	}

	if (cmdh == CMD_UART && cmdl == 2)
	{
		if (ttyS2 == -1)												goto baiout;
		res = write(ttyS2, (const void*)&frame[6], (size_t)LEN);
		if (0 >= res)													goto baiout;
		ack[2] = cmdh | (1 << 6);
		ack[3] = cmdl;
		res = write((int)lParam, (const void *)ack, sizeof(ack));
		if (0 >= res)													goto baiout;
		goto baiout;
	}

	if (cmdh == CMD_UART && cmdl == 1)
	{
		if (ttyS1 == -1)												goto baiout;
		res = write(ttyS1, (const void*)&frame[6], (size_t)LEN);
		if (0 >= res)													goto baiout;
		ack[2] = cmdh | (1 << 6);
		ack[3] = cmdl;
		res = write((int)lParam, (const void *)ack, sizeof(ack));
		if (0 >= res)													goto baiout;
		goto baiout;
	}

	if (cmdh == CMD_UART && cmdl == 0)
	{
		if (ttyS0 == -1)												goto baiout;
		res = write(ttyS0, (const void*)&frame[6], (size_t)LEN);
		if (0 >= res)													goto baiout;
		ack[2] = cmdh | (1 << 6);
		ack[3] = cmdl;
		res = write((int)lParam, (const void *)ack, sizeof(ack));
		if (0 >= res)													goto baiout;
		goto baiout;
	}

	if (cmdh == CMD_I2C && cmdl == 0)
	{
		if (-1 == i2c0)												goto baiout;

		if (0 == frame[6])
		{
			p = (unsigned char *)malloc(l = (12 + frame[9]));
			if (p)
			{
				memcpy(p, (void *)frame, LEN + 8);
				i2c_read_data(i2c0, frame[7], frame[8], &p[10], frame[9]);
				p[2] = cmdh | (1 << 6);
				p[l -2] = 0X50;
				p[l -1] = 0X50;
				res = write((int)lParam, (const void*)p, l);
				if (0 >= res)											{ dbg_printf(DBG_I2C, "(%s %d) WRITE() <= 0\n", __FILE__, __LINE__); }
				free(p);
			}
			else { dbg_printf(DBG_MALLOC, "(%s %d) MALLOC() FAIL\n", __FILE__, __LINE__); }
			goto baiout;
		}
		p = (unsigned char *)malloc(24);//, assert(0 != p);
		if(p)
		{
			i2c_write_data(i2c0, frame[7], frame[8], &frame[10], frame[9]);
			memcpy(p, (void *)frame, 10);
			p[2] = cmdh | (1 << 6);
			p[10] = 0X50;
			p[11] = 0X50;
			res = write((int)lParam, (const void*)p, 12);
			if (0 >= res)												{ dbg_printf(DBG_I2C, "(%s %d) WRITE() <= 0\n", __FILE__, __LINE__); }
			free(p);
		}
		else { dbg_printf(DBG_MALLOC, "(%s %d) MALLOC() FAIL\n", __FILE__, __LINE__); }
		goto baiout;
	}


	if (cmdh == CMD_SPI && cmdl == 0)
	{
		struct spi_ioc_transfer mesg[2];
		unsigned char spi_buff[300];
		int x;

		if (spi0 == -1)												{ dbg_printf(DBG_I2C, "(%s %d) spi0 == -1\n", __FILE__, __LINE__); goto baiout; }
		memset(spi_buff, 0, sizeof(spi_buff));
		memset(mesg, 0, sizeof(mesg));

		if (0 == frame[6])
		{
			mesg[0].tx_buf = (unsigned long)spi_buff;

/////////////////////////////////////////////////////
// OP_R

			// PASE 1
			// W, SLAVE, 0XBA= 0X5D * 2
			spi_buff[0] = frame[7] * 2;

			// PASE 2
			// W, REGISTER, 0X00
			spi_buff[1] = frame[8];
			mesg[0].len = 2;
			mesg[0].tx_buf = (unsigned long)spi_buff;
			res = ioctl(spi0, SPI_IOC_MESSAGE(1), mesg);
			if (0 != res)												{ dbg_printf(DBG_I2C, "OP_W, PASE 1/2 FAIL\n"); goto baiout; }

			// PASE 3
			// W, SLAVE, 0XBA= 0X5D * 2 + 1
			spi_buff[0] = frame[7] * 2 + 1;
			mesg[0].len = 1;
			mesg[0].tx_buf = (unsigned long)spi_buff;

			// PASE 4
			// R, DATA[0], DATA[1], ...
			mesg[1].len = frame[9];
			mesg[1].rx_buf = (unsigned long)spi_buff;
			res = ioctl(spi0, SPI_IOC_MESSAGE(2), mesg);
			if (0 != res)												{ dbg_printf(DBG_I2C, "OP_W, PASE 3/4 FAIL\n"); goto baiout; }

			// OUR ACK
			p = (unsigned char *)malloc(l = (12 + frame[9]));//, assert(0 != p);
			if(p)
			{
				memcpy(p, (void *)frame, LEN + 8);
				for (x = 0; x < frame[9]; x++)							spi_buff[x] = frame[10 + x];
				p[2] = cmdh | (1 << 6);
				p[l -2] = 0X50;
				p[l -1] = 0X50;
				res = write((int)lParam, (const void*)p, l);
				if (0 >= res)											{ dbg_printf(DBG_I2C, "(%s %d) WRITE() <= 0\n", __FILE__, __LINE__); }
				free(p);
			}
			else { dbg_printf(DBG_MALLOC, "(%s %d) MALLOC() FAIL\n", __FILE__, __LINE__); }
		}


/////////////////////////////////////////////////////
// OP_W

		// PASE 1
		// W, SLAVE, 0XBA= 0X5D * 2
		spi_buff[0] = frame[7] * 2;

		// PASE 2
		// W, REGISTER, 0X00
		spi_buff[1] = frame[8];

		// PASE 3
		// W, REGISTER, 0X00

		for (x = 0; x < frame[9]; x++)									spi_buff[2 + x] = frame[10 + x];
		mesg[0].len = frame[9] + 2;
		mesg[0].tx_buf = (unsigned long)spi_buff;
		res = ioctl(spi0, SPI_IOC_MESSAGE(1), mesg);
		if (0 != res)													{ dbg_printf(DBG_I2C, "OP_W, PASE 1/2/3 FAIL\n");goto baiout; }

		// ACK
		p = (unsigned char *)malloc(24);//, assert(0 != p);
		if (!p)														{ dbg_printf(DBG_MALLOC, "(%s %d) MALLOC() FAIL\n", __FILE__, __LINE__); goto baiout; }
		memcpy(p, (void *)frame, LEN + 8);
		memcpy(p, (void *)frame, 10);
		p[2] = cmdh | (1 << 6);
		p[10] = 0X50;
		p[11] = 0X50;
		res = write((int)lParam, (const void*)p, 12);
		if (0 >= res)													{ dbg_printf(DBG_I2C, "(%s %d) WRITE() <= 0\n", __FILE__, __LINE__); }
		free(p);
		goto baiout;
	}


baiout:
	pthread_cleanup_pop(1);
	return (*pResult = 1);
}

extern pthread_mutex_t xUatrt;
extern struct incoming_t uartQ;


LRESULT CFifoApp::OnJackDataRcv(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
	unsigned char *frame;
	unsigned short cmd;
	unsigned char cmdh, cmdl;
	unsigned short LEN;
	int res;
	unsigned char *p;
	int l;
	int left, offs;

	struct list_head *P, *Q;
	struct incoming_t *N;
	int fd;
	int last_type;


	if (0 == wParam)
		return (*pResult = 1);

	pthread_cleanup_push(free, (void *)wParam);

	frame = ((unsigned char *)wParam) + 64;
	cmdh = frame[2], cmdl = frame[3]; 
	cmd = BUILD_UINT16(cmdh, cmdl);
	LEN = BUILD_UINT16(frame[5], frame[4]);

	if (cmdh == CMD_GUI)												goto baiout;
	if (cmd & (1 << CMD_ACK_BIT))										goto baiout;

	if (cmdh == CMD_UART && cmdl == 2)
	{
		if (ttyS2 == -1)												{ dbg_printf(DBG_UART, "(%s %d) ttyS1 == -1\n", __FILE__, __LINE__); goto baiout;}
		res = write(ttyS2, (const void*)&frame[6], (size_t)LEN);
		if (0 >= res)													{ dbg_printf(DBG_UART, "(%s %d) WRITE() /dev/ttyS1 <= 0\n", __FILE__, __LINE__); goto baiout;}
		goto baiout;
	}
	if (cmdh == CMD_UART && cmdl == 1)
	{
		if (ttyS1 == -1)												{ dbg_printf(DBG_UART, "(%s %d) ttyS1 == -1\n", __FILE__, __LINE__); goto baiout;}
		res = write(ttyS1, (const void*)&frame[6], (size_t)LEN);
		if (0 >= res)													{ dbg_printf(DBG_UART, "(%s %d) WRITE() /dev/ttyS1 <= 0\n", __FILE__, __LINE__); goto baiout;}
		goto baiout;
	}
	if (cmdh == CMD_UART && cmdl == 0)
	{
		if (ttyS0 == -1)												{ dbg_printf(DBG_UART, "(%s %d) ttyS0 == -1\n", __FILE__, __LINE__); goto baiout;}
		res = write(ttyS0, (const void*)&frame[6], (size_t)LEN);
		if (0 >= res)													{ dbg_printf(DBG_UART, "(%s %d) WRITE() /dev/ttyS0 <= 0\n", __FILE__, __LINE__); goto baiout;}
		goto baiout;
	}

	if (cmdh == CMD_I2C && cmdl == 0)
	{
		if (-1 == i2c0)												{ dbg_printf(DBG_I2C, "(%s %d) -1 == i2c0\n", __FILE__, __LINE__); goto baiout; }

		if (0 == frame[6])
		{
			p = (unsigned char *)malloc(l = (12 + frame[9]));//, assert(0 != p);
			if(p)
			{
				memcpy(p, (void *)frame, LEN + 8);
				i2c_read_data(i2c0, frame[7], frame[8], &p[10], frame[9]);
				p[2] = cmdh | (1 << 6);
				p[l -2] = 0X50;
				p[l -1] = 0X50;
				res = write((int)lParam, (const void*)p, l);
				if (0 >= res)											{ dbg_printf(DBG_I2C, "(%s %d) WRITE() <= 0\n", __FILE__, __LINE__); }
				free(p);
			}
			else { 	dbg_printf(DBG_MALLOC, "(%s %d) MALLOC() FAIL\n", __FILE__, __LINE__); }
			goto baiout;
		}
		p = (unsigned char *)malloc(24);//, assert(0 != p);
		if(p)
		{
			i2c_write_data(i2c0, frame[7], frame[8], &frame[10], frame[9]);
			memcpy(p, (void *)frame, 10);
			p[2] = cmdh | (1 << 6);
			p[10] = 0X50;
			p[11] = 0X50;
			res = write((int)lParam, (const void*)p, 12);
			if (0 >= res)												{ dbg_printf(DBG_I2C, "(%s %d) WRITE() <= 0\n", __FILE__, __LINE__); }
			free(p);
		}
		else { dbg_printf(DBG_MALLOC, "(%s %d) MALLOC() FAIL\n", __FILE__, __LINE__); }
		goto baiout;
	}

	if (cmdh == CMD_FPC)
	{
		int fail = 0;

///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////

		dbg_printf(DBG_JACK, "(%s %d %d) OnJackDataRcv: ", __FILE__, __LINE__, ttyUSB0);
		for (int i = 0; i < LEN + 8; i++)									dbg_printf(DBG_JACK, "%c", frame[i]);
		dbg_printf(DBG_JACK, "\n");

		pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &last_type);
		pthread_mutex_lock(&xUatrt);
		pthread_cleanup_push(CleanupLock, (void *)&xUatrt);
		pthread_setcanceltype(last_type, NULL);
		list_for_each_safe(P, Q, &uartQ.list)
		{
			N = list_entry(P, struct incoming_t, list); assert(0 != N); fd = N->fd;
			if (0 == strcmp(N->name, "/dev/ttyUSB0"))
			{
				ttyUSB0 = fd;

				left = LEN; offs = 0;
				while(left > 0)
				{
					l = write(fd, (const void*)(frame + 6 + offs), left);
					if (l <= 0)
					{
						fail = 1;
						dbg_printf(DBG_UART, "(%s %d) WRITE() <= 0\n", __FILE__, __LINE__);
						close(fd); list_del(P); free(N);
						ttyUSB0 = -1;
						break;
					}
					left -= l, offs +=l;
				}
				SetTimer(201, 1000, 0);
			}
		}
		pthread_cleanup_pop(1);

		if (fail == 1)
		{
			res = AttachUart("/dev/ttyUSB0", 38400);
			if (res)															{ dbg_printf(DBG_UART, "(%s, %d) /dev/ttyUSB0 Attach OK\n", __FILE__, __LINE__); ttyUSB0 = res; }
		}

		goto baiout;
	}

	if (cmdh == CMD_GPIO)
	{
		goto baiout;
	}

	if (cmdh == CMD_USB_UART)
	{
		goto baiout;
	}

	if (cmdh == CMD_SPI)
	{


		goto baiout;
	}

baiout:
	pthread_cleanup_pop(1);
	return (*pResult = 1);
}

static void OnTtyUsbChar(int infd, unsigned char ch)
{
	static unsigned char buff[1024 * 2];
	static unsigned char *p = buff;
	int len = 0;
	int idx;
	int left, offs, l;
	struct list_head *P, *Q;
	struct incoming_t *N;
	int sd;


///////////////////////////////////////////////////////////
	idx = (int)(p - buff);
	if (ch > 127)														{ dbg_printf(DBG_UART, "(%s %d) d0 idx=%d ch=`%c`\n", __FILE__, __LINE__, idx, ch); p = buff; return; }

///////////////////////////////////////////////////////////
	if (0 == (p - buff))
	{
		len = 0;
		if (ch != '5')													{ dbg_printf(DBG_UART, "(%s %d) d1 idx=%d ch=`%c`\n", __FILE__, __LINE__, idx, ch); p = buff; return; }
		*p++ = ch;
		return;
	}
	if (1 == (p - buff))
	{
		if (ch != '5')													{ dbg_printf(DBG_UART, "(%s %d) d2 idx=%d ch=`%c\n", __FILE__, __LINE__, idx, ch); p = buff; return; }
		*p++ = ch;
		return;
	}
	if (2 == (p - buff))
	{
		if (ch != '5')													{ dbg_printf(DBG_UART, "(%s %d) d3 idx=%d ch=`%c`\n", __FILE__, __LINE__, idx, ch); p = buff; return; }
		*p++ = ch;
		return;
	}
	if(3 == ((p - buff) % 4)	)
	{
		if (ch != ',')													{ dbg_printf(DBG_UART, "(%s %d) d4 idx=%d ch=`%c`\n", __FILE__, __LINE__, idx, ch); p = buff; return; }
		*p++ = ch;
		return;
	}
	if (4 == (p - buff) || 5 == (p - buff) || 6 == (p - buff))
	{
		*p++ = ch;
		return;
	}
	if (8 == (p - buff) || 9 == (p - buff))
	{
		*p++ = ch;
		return;
	}
	if (12 == (p - buff) || 13 == (p - buff))
	{
		*p++ = ch;
		return;
	}
	if (10 == (p - buff))
	{
		*p++ = ch;
		if (buff[8] < 48 || buff[8] > 57)									{ dbg_printf(DBG_UART, "(%s %d) d5 idx=%d ch=`%c`\n", __FILE__, __LINE__, idx, ch); p = buff; return; }
		if (buff[9] < 48 || buff[9] > 57)									{ dbg_printf(DBG_UART, "(%s %d) d5 idx=%d ch=`%c`\n", __FILE__, __LINE__, idx, ch); p = buff; return; }
		if (buff[10] < 48 || buff[10] > 57)								{ dbg_printf(DBG_UART, "(%s %d) d5 idx=%d ch=`%c`\n", __FILE__, __LINE__, idx, ch); p = buff; return; }
		len = 100 * (buff[8] - 48) + 10 * (buff[9] - 48) + (buff[10] - 48);
		if (len > 1000)												{ dbg_printf(DBG_UART, "(%s %d) d11 idx=%d ch=`%c\n", __FILE__, __LINE__, idx, ch); p = buff; return; }
		if (0 != (len % 4))												{ dbg_printf(DBG_UART, "(%s %d) d12 idx=%d ch=`%c\n", __FILE__, __LINE__, idx, ch); p = buff; return; }
		return;
	}

	len = 100 * (buff[8] - 48) + 10 * (buff[9] - 48) + (buff[10] - 48);
	if (14 == (p - buff) && 0 != len)
	{
		*p++ = ch;
		return;
	}

	len = 100 * (buff[8] - 48) + 10 * (buff[9] - 48) + (buff[10] - 48);
	if ((len + 14) == (p - buff))
	{
		AfxGetApp()->KillTimer(301);
		*p++ = ch;

		dbg_printf(DBG_UART, "(%s %d %d) TTYUSB0: ", __FILE__, __LINE__, (int)(p-buff));
		for (int i = 0; i < (p - buff); i++)									{ dbg_printf(DBG_UART, "%c", buff[i]); }
		dbg_printf(DBG_UART, "\n");


///////////////////////////////////////////////////////////
		pack(CMD_GUI | (1 << 6), 0, len + 15, buff);

		pthread_mutex_lock(&xJ);
		pthread_cleanup_push(CleanupLock, (void *)&xJ);
		list_for_each_safe(P, Q, &jackCliQ.list)
		{
			N = list_entry(P, struct incoming_t, list); assert(0 != N); sd = N->fd;
			p = buff, left = len + 15 + 8, offs = 0;
			while(left > 0)
			{
				l = write(sd, (const void*)(p + offs), left);
				if (l <= 0)
				{
					dbg_printf(DBG_UART, "WRITE() FAIL !!\n");
					break;
				};
				left -= l, offs +=l;
			}
		}
		pthread_cleanup_pop(1);

		p = buff;
		return;
	}

	*p++ = ch;
	return;
}

static void OnTtyUsb(int infd, const char *path, unsigned char *data, int len)
{
	for (int i = 0; i < len; i++)											OnTtyUsbChar(infd, data[i]);
}

LRESULT CFifoApp::OnUartDataRcv(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
	int len;
	char path[128];

	if (0 == wParam)													return (*pResult = 1);
	pthread_cleanup_push(free, (void *)wParam);


	memcpy(&len, (void *)wParam, sizeof(int));
	memset(path, 0, sizeof(path)), strcpy(path, ((char *)wParam) + sizeof(int));

	//printf("OnUartDataRcv()\n");

#if 0
	if (0 == strcmp(path, "/dev/ttyS1"))
	{
		printf("OnTtyS1()\n");

		unsigned char *p;

		p = (unsigned char *)malloc(len + 48), assert(0 != p);
		pthread_cleanup_push(free, (void *)p);

		*(p + 0) = 0X05;
		*(p + 1) = 0X05;
		//*(p + 2) = CMD_UART | (1 << 6);
		*(p + 2) = CMD_GUI | (1 << 6);
		//*(p + 3) = 0X01;
		*(p + 3) = 0X00;
		*(p + 4) = HI_UINT16(len);
		*(p + 5) = LO_UINT16(len);
		memcpy(p + 6, (void *)((char *)wParam + 64), len);
		*(p + 6 + len) = 0X50;
		*(p + 7 + len) = 0X50;

		pthread_mutex_lock(&xJ);
		pthread_cleanup_push(CleanupLock, (void *)&xJ);
		list_for_each_safe(P, Q, &jackCliQ.list)
		{
			N = list_entry(P, struct incoming_t, list); assert(0 != N); fd = N->fd;
			res = write(fd, (const void *)p, (size_t)(len + 8));
			if (res <= 0) {}
		}
		pthread_cleanup_pop(1);

		pthread_cleanup_pop(1);
		goto baiout;
	}
#endif


	if (0 == strcmp(path, "/dev/ttyUSB0"))
	{
		OnTtyUsb((int)lParam, (char *)wParam + sizeof(int), (unsigned char *)wParam + 64, len);
		goto baiout;
	}

	if (0 == strcmp(path, "/dev/ttyS2"))
	{
		//OnTtyS2((unsigned char *)wParam + 64, len);
		goto baiout;
	}


baiout:
	pthread_cleanup_pop(1);
	return (*pResult = 1);
}



