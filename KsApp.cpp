// KsApp.cpp: implementation of the CKsApp class.
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
#include <sys/reboot.h>


#include <asm/ioctl.h>
#include <asm/errno.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#include "list.h"
#include "Event.h"
#include "Misc.h"
#include "BaseApp.h"
#include "KsApp.h"


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

#define GP_PWM_MAGIC		'P'
#define PWM_IOCTL_SET_ENABLE		_IOW(GP_PWM_MAGIC, 1, unsigned int)
#define PWM_IOCTL_SET_ATTRIBUTE		_IOW(GP_PWM_MAGIC, 2, gp_pwm_config_t)
#define PWM_IOCTL_GET_ATTRIBUTE		_IOR(GP_PWM_MAGIC, 2, gp_pwm_config_t*)


#define KEY_SCAN_TIMER_ID						102
#define PWM1_TIMER_ID							101
#define BUZZER_START_PWM_TIMER_ID			198
#define BUZZER_STOP_PWM_TIMER_ID				199


#define PCA_9555_SLAVE_0											0X24
#define PCA_9555_SLAVE_1											0X20
#define PCA_9555_SLAVE_2											0X21

#define PCA_9555_INPUT_0											0
#define PCA_9555_INPUT_1											1
#define PCA_9555_OUTPUT_0											2
#define PCA_9555_OUTPUT_1											3
#define PCA_9555_POLARITY_0										4
#define PCA_9555_POLARITY_1										5
#define PCA_9555_CONFIG_0											6
#define PCA_9555_CONFIG_1											7

#define KS_LEFT_TOP												1
#define KS_LEFT_CENTER											2
#define KS_LEFT_BOTTOM											3
#define KS_RIGHT_TOP											4
#define KS_RIGHT_CENTER										5
#define KS_RIGHT_BOTTOM										6
#define KS_START												9
#define KS_STOP													21
#define KS_WORKLOAD_UP										7
#define KS_WORKLOAD_DOWN										8
#define KS_STRIDE_UP											22
#define KS_STRIDE_DOWN											23
#define KS_NUM0													10
#define KS_NUM1													11
#define KS_NUM2													12
#define KS_NUM3													13
#define KS_NUM4													14
#define KS_NUM5													15
#define KS_NUM6													16
#define KS_NUM7													17
#define KS_NUM8													18
#define KS_NUM9													19
#define KS_DELETE												20



//////////////////////////////////////////////////////////////////////
// routines
//////////////////////////////////////////////////////////////////////

unsigned char key_scan[64];

static pthread_mutex_t xF = PTHREAD_MUTEX_INITIALIZER;
static struct incoming_t fpcCliQ;


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

void *UiThread(void *param)
{
	struct list_head *P, *Q;
	struct incoming_t *N;
	int fd;
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
	servaddr.sin_family = AF_INET, servaddr.sin_addr.s_addr = htonl(INADDR_ANY), servaddr.sin_port = htons(8366);
	if (0 > bind(lsd, (struct sockaddr *)&servaddr, sizeof(struct sockaddr_in)))	{ dbg_printf(DBG_SOCKET, "(%s %d) BIND() fail\n", __FILE__, __LINE__); AfxGetApp()->PostMessage(M_EXIT_INSTANCE, 0, 0); return 0; }
	if (0 > listen(lsd, 12))												{ dbg_printf(DBG_SOCKET, "(%s %d) LISTEN() fail\n", __FILE__, __LINE__); AfxGetApp()->PostMessage(M_EXIT_INSTANCE, 0, 0); return 0; }
	v = fcntl(lsd, F_GETFL, 0); fcntl(lsd, F_SETFL, v | O_NONBLOCK);
	v = (512 * 1024); setsockopt(lsd, SOL_SOCKET, SO_SNDBUF, (const void *)&v, sizeof(v)); setsockopt(lsd, SOL_SOCKET, SO_RCVBUF, (const void *)&v, sizeof(v));

	pthread_cleanup_push(CleanupFd, (void *)&lsd);

	sigemptyset(&nset);
	sigaddset(&nset, SIGHUP), sigaddset(&nset, SIGINT), sigaddset(&nset, SIGTERM), sigaddset(&nset, SIGUSR1), sigaddset(&nset, SIGALRM);
	pthread_sigmask(SIG_BLOCK, &nset, &oset);
	pthread_cleanup_push(RestoreSigmask, (void *)&oset);

	pthread_cleanup_push(CleanupIncoming, (void *)&fpcCliQ);

	for (;;)
	{
		int clilen, connfd;
		struct sockaddr_in cliaddr;

		maxfd = 0; FD_ZERO(&rset); FD_SET(lsd, &rset);
		if (maxfd < lsd)												maxfd = lsd;

		pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &last_type);
		pthread_mutex_lock(&xF);
		pthread_cleanup_push(CleanupLock, (void *)&xF);
		pthread_setcanceltype(last_type, NULL);

		list_for_each_safe(P, Q, &fpcCliQ.list)
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
				v = (512 * 1024), setsockopt(connfd, SOL_SOCKET, SO_SNDBUF, (const void *)&v, sizeof(v)), setsockopt(connfd, SOL_SOCKET, SO_RCVBUF, (const void *)&v, sizeof(v));
				if (0 != (N = (struct incoming_t *)malloc(sizeof(struct incoming_t) + 1024 * 4)))
				{
					dbg_printf(DBG_SOCKET, "(%s %d) ACCEPT()\n", __FILE__, __LINE__);

					memset(N, 0, sizeof(struct incoming_t)); N->fd = connfd, memcpy(&N->cliaddr, &cliaddr, clilen); N->left = 1; N->offs = 0;

					pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &last_type);
					pthread_mutex_lock(&xF);
					pthread_cleanup_push(CleanupLock, (void *)&xF);
					pthread_setcanceltype(last_type, NULL);
					list_add_tail(&N->list, &fpcCliQ.list);
					pthread_cleanup_pop(1);
				}
				else													{ dbg_printf(DBG_MALLOC, "(%s %d) MALLOC() FAIL\n", __FILE__, __LINE__); close(connfd); }
			}
			else { dbg_printf(DBG_MALLOC, "(%s %d) ACCEPT() FAIL\n", __FILE__, __LINE__); }
			continue;
		}


		pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &last_type);
		pthread_mutex_lock(&xF);
		pthread_cleanup_push(CleanupLock, (void *)&xF);
		pthread_setcanceltype(last_type, NULL);

		list_for_each_safe(P, Q, &fpcCliQ.list)
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
					continue;
				}
				else
				{
					left = N->left = (LEN + 2);
				}
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
					v = LEN + 8;
					frame = (unsigned char *)malloc(LEN + 8 + 64 + 120);
					if (frame)
					{
						memcpy(frame, &v, sizeof(int));
						memcpy(frame + 64, N->msg, LEN + 8);
						AfxGetApp()->PostMessage(M_UI_INCOMING, (WPARAM)frame, (LPARAM)fd);
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
			}

		}
		pthread_cleanup_pop(1);

	}
	pthread_cleanup_pop(1);
	pthread_cleanup_pop(1);
	pthread_cleanup_pop(1);

dbg_printf(DBG_THREAD, "(%s %d) UiThread() EXIT\n", __FILE__, __LINE__); 
	return 0;
}


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

int packOurProtocol(unsigned char ch, unsigned char cl, int len, const unsigned char *data, unsigned char *out)
{
	unsigned char p[1024 * 8];

	*(p + 0) = 0X05;
	*(p + 1) = 0X05;
	*(p + 2) = ch;
	*(p + 3) = cl;
	*(p + 4) = HI_UINT16(len);
	*(p + 5) = LO_UINT16(len);
	memcpy(p + 6, (void *)data, len);
	*(p + 6 + len) = 0X50;
	*(p + 7 + len) = 0X50;

	if (out)
		memcpy(out, (void *)p, len + 8);
	return (len + 8);
}


static unsigned char P1;


int PollKeyScan(unsigned char line)
{
	unsigned char P0;

	int longPress = 0;
	int keyNum = 0;
	time_t now = 0;
	CKsApp *app = (CKsApp *)AfxGetApp();

	static unsigned char O0 = 0;
	static time_t T0 = 0;
	static unsigned char O1 = 0;
	static time_t T1 = 0;
	static unsigned char O2 = 0;
	static time_t T2 = 0;
	static unsigned char O3 = 0;
	static time_t T3 = 0;
	static unsigned char O4 = 0;
	static time_t T4 = 0;
	static unsigned char O5 = 0;
	static time_t T5 = 0;
	static unsigned char O6 = 0;
	static time_t T6 = 0;


	if (line >= 8)		return -1;
	memset(&P0, 0XFF, sizeof(P0));
	P0 &= ~(1<<line);

	if (-1 == i2c_write_data(app->i2c0, PCA_9555_SLAVE_0, PCA_9555_OUTPUT_0, (unsigned char *)&P0, 1))
	{
		printf("(%s %d) i2c_write_data() FAIL\n", __FILE__, __LINE__);
		return -1;
	}
	if (-1 == i2c_read_data(app->i2c0, PCA_9555_SLAVE_0, PCA_9555_INPUT_1, &P1, 1))
	{
		printf("(%s %d) i2c_read_data() FAIL\n", __FILE__, __LINE__);
		return -1;
	}


///////////////////////////////////////////////////////////
	if (0 == line)
	{
		////////////
		if ((0XF9^BIT0) == P1 && 0XF9 == O0)
		{
			time(&T0);
			keyNum = 5;
			key_scan[keyNum] = 1;
			AfxGetApp()->PostMessage(M_KEY_SCAN_PRESSED, (WPARAM)keyNum, 0);
		}
		if ((0XF9^BIT0) == P1 && (0XF9^BIT0) == O0)
		//if ((0XF9^BIT0) == P1 && BIT0 != (O0&BIT0))
		{
			time(&now);
			if (now - T0 >= 3)
			{
				keyNum = 5;
				key_scan[keyNum] = 2;
			}
		}
		if (0XF9 == P1 && (0XF9^BIT0) == O0)
		{
			keyNum = 5;
			time(&now);
			if (now - T0 >= 3)		longPress = 1;
			AfxGetApp()->PostMessage(M_KEY_SCAN_RELEASED, (WPARAM)keyNum, (LPARAM)longPress);
			key_scan[keyNum] = 0;
		}

		////////////
		if ((0XF9^BIT3) == P1 && 0XF9 == O0)
		{
			time(&T0);
			keyNum = 22;
			key_scan[keyNum] = 1;
			AfxGetApp()->PostMessage(M_KEY_SCAN_PRESSED, (WPARAM)keyNum, 0);
		}
		if ((0XF9^BIT3) == P1 && (0XF9^BIT3) == O0)
		{
			time(&now);
			if (now - T0 >= 3)
			{
				keyNum = 22;
				key_scan[keyNum] = 2;
			}
		}
		if (0XF9 == P1 && (0XF9^BIT3) == O0)
		{
			keyNum = 22;
			time(&now);
			if (now - T0 >= 3)		longPress = 1;
			AfxGetApp()->PostMessage(M_KEY_SCAN_RELEASED, (WPARAM)keyNum, (LPARAM)longPress);
			key_scan[keyNum] = 0;
		}

		////////////
		if ((0XF9^BIT4) == P1 && 0XF9 == O0)
		{
			time(&T0);
			keyNum = 21;
			key_scan[keyNum] = 1;
			AfxGetApp()->PostMessage(M_KEY_SCAN_PRESSED, (WPARAM)keyNum, 0);
		}
		if ((0XF9^BIT4) == P1 && (0XF9^BIT4) == O0)
		{
			time(&now);
			if (now - T0 >= 3)
			{
				keyNum = 21;
				key_scan[keyNum] = 2;
			}
		}
		if (0XF9 == P1 && (0XF9^BIT4) == O0)
		{
			keyNum = 21;
			time(&now);
			if (now - T0 >= 3)		longPress = 1;
			AfxGetApp()->PostMessage(M_KEY_SCAN_RELEASED, (WPARAM)keyNum, (LPARAM)longPress);
			key_scan[keyNum] = 0;
		}


		////////////
		if ((0XF9^BIT5) == P1 && 0XF9 == O0)
		{
			time(&T0);
			keyNum = 12;
			key_scan[keyNum] = 1;
			AfxGetApp()->PostMessage(M_KEY_SCAN_PRESSED, (WPARAM)keyNum, 0);
		}
		if ((0XF9^BIT5) == P1 && (0XF9^BIT5) == O0)
		{
			time(&now);
			if (now - T0 >= 3)
			{
				keyNum = 12;
				key_scan[keyNum] = 2;
			}
		}
		if (0XF9 == P1 && (0XF9^BIT5) == O0)
		{
			keyNum = 12;
			time(&now);
			if (now - T0 >= 3)		longPress = 1;
			AfxGetApp()->PostMessage(M_KEY_SCAN_RELEASED, (WPARAM)keyNum, (LPARAM)longPress);
			key_scan[keyNum] = 0;
		}


		O0 = P1;
	}


///////////////////////////////////////////////////////////
	if (1 == line)
	{
		////////////
		if ((0XF9^BIT0) == P1 && 0XF9 == O1)
		{
			time(&T1);
			keyNum = 4;
			key_scan[keyNum] = 1;
			AfxGetApp()->PostMessage(M_KEY_SCAN_PRESSED, (WPARAM)keyNum, 0);
		}
		if ((0XF9^BIT0) == P1 && (0XF9^BIT0) == O1)
		{
			time(&now);
			if (now - T1 >= 3)
			{
				keyNum = 4;
				key_scan[keyNum] = 2;
			}
		}
		if (0XF9 == P1 && (0XF9^BIT0) == O1)
		{
			keyNum = 4;
			time(&now);
			if (now - T1 >= 3)		longPress = 1;
			AfxGetApp()->PostMessage(M_KEY_SCAN_RELEASED, (WPARAM)keyNum, (LPARAM)longPress);
			key_scan[keyNum] = 0;
		}

		////////////
		if ((0XF9^BIT3) == P1 && 0XF9 == O1)
		{
			time(&T1);
			keyNum = 23;
			key_scan[keyNum] = 1;
			AfxGetApp()->PostMessage(M_KEY_SCAN_PRESSED, (WPARAM)keyNum, 0);
		}
		if ((0XF9^BIT3) == P1 && (0XF9^BIT3) == O1)
		{
			time(&now);
			if (now - T1 >= 3)
			{
				keyNum = 23;
				key_scan[keyNum] = 2;
			}
		}
		if (0XF9 == P1 && (0XF9^BIT3) == O1)
		{
			keyNum = 23;
			time(&now);
			if (now - T1 >= 3)		longPress = 1;
			AfxGetApp()->PostMessage(M_KEY_SCAN_RELEASED, (WPARAM)keyNum, (LPARAM)longPress);
			key_scan[keyNum] = 0;
		}

		////////////
		if ((0XF9^BIT4) == P1 && 0XF9 == O1)
		{
			time(&T1);
			keyNum = 16;
			key_scan[keyNum] = 1;
			AfxGetApp()->PostMessage(M_KEY_SCAN_PRESSED, (WPARAM)keyNum, 0);
		}
		if ((0XF9^BIT4) == P1 && (0XF9^BIT4) == O1)
		{
			time(&now);
			if (now - T1 >= 3)
			{
				keyNum = 16;
				key_scan[keyNum] = 2;
			}
		}
		if (0XF9 == P1 && (0XF9^BIT4) == O1)
		{
			keyNum = 16;
			time(&now);
			if (now - T1 >= 3)		longPress = 1;
			AfxGetApp()->PostMessage(M_KEY_SCAN_RELEASED, (WPARAM)keyNum, (LPARAM)longPress);
			key_scan[keyNum] = 0;
		}

		////////////
		if ((0XF9^BIT5) == P1 && 0XF9 == O1)
		{
			time(&T1);
			keyNum = 13;
			key_scan[keyNum] = 1;
			AfxGetApp()->PostMessage(M_KEY_SCAN_PRESSED, (WPARAM)keyNum, 0);
		}
		if ((0XF9^BIT5) == P1 && (0XF9^BIT5) == O1)
		{
			time(&now);
			if (now - T1 >= 3)
			{
				keyNum = 13;
				key_scan[keyNum] = 2;
			}
		}
		if (0XF9 == P1 && (0XF9^BIT5) == O1)
		{
			keyNum = 13;
			time(&now);
			if (now - T1 >= 3)		longPress = 1;
			AfxGetApp()->PostMessage(M_KEY_SCAN_RELEASED, (WPARAM)keyNum, (LPARAM)longPress);
			key_scan[keyNum] = 0;
		}

		O1 = P1;
	}


///////////////////////////////////////////////////////////
	if (2 == line)
	{
		////////////
		if ((0XF9^BIT0) == P1 && 0XF9 == O2)
		{
			time(&T2);
			keyNum = 1;
			key_scan[keyNum] = 1;
			AfxGetApp()->PostMessage(M_KEY_SCAN_PRESSED, (WPARAM)keyNum, 0);
		}
		if ((0XF9^BIT0) == P1 && (0XF9^BIT0) == O2)
		{
			time(&now);
			if (now - T2 >= 3)
			{
				keyNum = 1;
				key_scan[keyNum] = 2;
			}
		}
		if (0XF9 == P1 && (0XF9^BIT0) == O2)
		{
			keyNum = 1;
			time(&now);
			if (now - T2 >= 3)		longPress = 1;
			AfxGetApp()->PostMessage(M_KEY_SCAN_RELEASED, (WPARAM)keyNum, (LPARAM)longPress);
			key_scan[keyNum] = 0;
		}

		////////////
		if ((0XF9^BIT3) == P1 && 0XF9 == O2)
		{
			time(&T2);
			keyNum = 20;
			key_scan[keyNum] = 1;
			AfxGetApp()->PostMessage(M_KEY_SCAN_PRESSED, (WPARAM)keyNum, 0);
		}
		if ((0XF9^BIT3) == P1 && (0XF9^BIT3) == O2)
		{
			time(&now);
			if (now - T2 >= 3)
			{
				keyNum = 20;
				key_scan[keyNum] = 2;
			}
		}
		if (0XF9 == P1 && (0XF9^BIT3) == O2)
		{
			keyNum = 20;
			time(&now);
			if (now - T2 >= 3)		longPress = 1;
			AfxGetApp()->PostMessage(M_KEY_SCAN_RELEASED, (WPARAM)keyNum, (LPARAM)longPress);
			key_scan[keyNum] = 0;
		}

		////////////
		if ((0XF9^BIT4) == P1 && 0XF9 == O2)
		{
			time(&T2);
			keyNum = 17;
			key_scan[keyNum] = 1;
			AfxGetApp()->PostMessage(M_KEY_SCAN_PRESSED, (WPARAM)keyNum, 0);
		}
		if ((0XF9^BIT4) == P1 && (0XF9^BIT4) == O2)
		{
			time(&now);
			if (now - T2 >= 3)
			{
				keyNum = 1;
				key_scan[keyNum] = 2;
			}
		}
		if (0XF9 == P1 && (0XF9^BIT4) == O2)
		{
			keyNum = 17;
			time(&now);
			if (now - T2 >= 3)		longPress = 1;
			AfxGetApp()->PostMessage(M_KEY_SCAN_RELEASED, (WPARAM)keyNum, (LPARAM)longPress);
			key_scan[keyNum] = 0;
		}

		////////////
		if ((0XF9^BIT5) == P1 && 0XF9 == O2)
		{
			time(&T2);
			keyNum = 14;
			key_scan[keyNum] = 1;
			AfxGetApp()->PostMessage(M_KEY_SCAN_PRESSED, (WPARAM)keyNum, 0);
		}
		if ((0XF9^BIT5) == P1 && (0XF9^BIT5) == O2)
		{
			time(&now);
			if (now - T2 >= 3)
			{
				keyNum = 1;
				key_scan[keyNum] = 2;
			}
		}
		if (0XF9 == P1 && (0XF9^BIT5) == O2)
		{
			keyNum = 14;
			time(&now);
			if (now - T2 >= 3)		longPress = 1;
			AfxGetApp()->PostMessage(M_KEY_SCAN_RELEASED, (WPARAM)keyNum, (LPARAM)longPress);
			key_scan[keyNum] = 0;
		}

		O2 = P1;
	}


///////////////////////////////////////////////////////////
	if (3 == line)
	{
		////////////
		if ((0XF9^BIT0) == P1 && 0XF9 == O3)
		{
			time(&T3);
			keyNum = 2;
			key_scan[keyNum] = 1;
			AfxGetApp()->PostMessage(M_KEY_SCAN_PRESSED, (WPARAM)keyNum, 0);
		}
		if ((0XF9^BIT0) == P1 && (0XF9^BIT0) == O3)
		{
			time(&now);
			if (now - T3 >= 3)
			{
				keyNum = 2;
				key_scan[keyNum] = 2;
			}
		}
		if (0XF9 == P1 && (0XF9^BIT0) == O3)
		{
			keyNum = 2;
			time(&now);
			if (now - T3 >= 3)		longPress = 1;
			AfxGetApp()->PostMessage(M_KEY_SCAN_RELEASED, (WPARAM)keyNum, (LPARAM)longPress);
			key_scan[keyNum] = 0;
		}

		////////////
		if ((0XF9^BIT3) == P1 && 0XF9 == O3)
		{
			time(&T3);
			keyNum = 19;
			key_scan[keyNum] = 1;
			AfxGetApp()->PostMessage(M_KEY_SCAN_PRESSED, (WPARAM)keyNum, 0);
		}
		if ((0XF9^BIT3) == P1 && (0XF9^BIT3) == O3)
		{
			time(&now);
			if (now - T3 >= 3)
			{
				keyNum = 19;
				key_scan[keyNum] = 2;
			}
		}
		if (0XF9 == P1 && (0XF9^BIT3) == O3)
		{
			keyNum = 19;
			time(&now);
			if (now - T3 >= 3)		longPress = 1;
			AfxGetApp()->PostMessage(M_KEY_SCAN_RELEASED, (WPARAM)keyNum, (LPARAM)longPress);
			key_scan[keyNum] = 0;
		}

		////////////
		if ((0XF9^BIT4) == P1 && 0XF9 == O3)
		{
			time(&T3);
			keyNum = 18;
			key_scan[keyNum] = 1;
			AfxGetApp()->PostMessage(M_KEY_SCAN_PRESSED, (WPARAM)keyNum, 0);
		}
		if ((0XF9^BIT4) == P1 && (0XF9^BIT4) == O3)
		{
			time(&now);
			if (now - T3 >= 3)
			{
				keyNum = 18;
				key_scan[keyNum] = 2;
			}
		}
		if (0XF9 == P1 && (0XF9^BIT4) == O3)
		{
			keyNum = 18;
			time(&now);
			if (now - T3 >= 3)		longPress = 1;
			AfxGetApp()->PostMessage(M_KEY_SCAN_RELEASED, (WPARAM)keyNum, (LPARAM)longPress);
			key_scan[keyNum] = 0;
		}

		////////////
		if ((0XF9^BIT5) == P1 && 0XF9 == O3)
		{
			time(&T3);
			keyNum = 15;
			key_scan[keyNum] = 1;
			AfxGetApp()->PostMessage(M_KEY_SCAN_PRESSED, (WPARAM)keyNum, 0);
		}
		if ((0XF9^BIT5) == P1 && (0XF9^BIT5) == O3)
		{
			time(&now);
			if (now - T3 >= 3)
			{
				keyNum = 15;
				key_scan[keyNum] = 2;
			}
		}
		if (0XF9 == P1 && (0XF9^BIT5) == O3)
		{
			keyNum = 15;
			time(&now);
			if (now - T3 >= 3)		longPress = 1;
			AfxGetApp()->PostMessage(M_KEY_SCAN_RELEASED, (WPARAM)keyNum, (LPARAM)longPress);
			key_scan[keyNum] = 0;
		}

		O3 = P1;
	}


///////////////////////////////////////////////////////////
	if (4 == line)
	{
		////////////
		if ((0XF9^BIT0) == P1 && 0XF9 == O4)
		{
			time(&T4);
			keyNum = 3;
			key_scan[keyNum] = 1;
			AfxGetApp()->PostMessage(M_KEY_SCAN_PRESSED, (WPARAM)keyNum, 0);
		}
		if ((0XF9^BIT0) == P1 && (0XF9^BIT0) == O4)
		{
			time(&now);
			if (now - T4 >= 3)
			{
				keyNum = 3;
				key_scan[keyNum] = 2;
			}
		}
		if (0XF9 == P1 && (0XF9^BIT0) == O4)
		{
			keyNum = 3;
			time(&now);
			if (now - T4 >= 3)		longPress = 1;
			AfxGetApp()->PostMessage(M_KEY_SCAN_RELEASED, (WPARAM)keyNum, (LPARAM)longPress);
			key_scan[keyNum] = 0;
		}

		////////////
		if ((0XF9^BIT3) == P1 && 0XF9 == O4)
		{
			time(&T4);
			keyNum = 8;
			key_scan[keyNum] = 1;
			AfxGetApp()->PostMessage(M_KEY_SCAN_PRESSED, (WPARAM)keyNum, 0);
		}
		if ((0XF9^BIT3) == P1 && (0XF9^BIT3) == O4)
		{
			time(&now);
			if (now - T4 >= 3)
			{
				keyNum = 8;
				key_scan[keyNum] = 2;
			}
		}
		if (0XF9 == P1 && (0XF9^BIT3) == O4)
		{
			keyNum = 8;
			time(&now);
			if (now - T4 >= 3)		longPress = 1;
			AfxGetApp()->PostMessage(M_KEY_SCAN_RELEASED, (WPARAM)keyNum, (LPARAM)longPress);
			key_scan[keyNum] = 0;
		}

		////////////
		if ((0XF9^BIT4) == P1 && 0XF9 == O4)
		{
			time(&T4);
			keyNum = 9;
			key_scan[keyNum] = 1;
			AfxGetApp()->PostMessage(M_KEY_SCAN_PRESSED, (WPARAM)keyNum, 0);
		}
		if ((0XF9^BIT4) == P1 && (0XF9^BIT4) == O4)
		{
			time(&now);
			if (now - T4 >= 3)
			{
				keyNum = 9;
				key_scan[keyNum] = 2;
			}
		}
		if (0XF9 == P1 && (0XF9^BIT4) == O4)
		{
			keyNum = 9;
			time(&now);
			if (now - T4 >= 3)		longPress = 1;
			AfxGetApp()->PostMessage(M_KEY_SCAN_RELEASED, (WPARAM)keyNum, (LPARAM)longPress);
			key_scan[keyNum] = 0;
		}


		////////////
		if ((0XF9^BIT5) == P1 && 0XF9 == O4)
		{
			time(&T4);
			keyNum = 11;
			key_scan[keyNum] = 1;
			AfxGetApp()->PostMessage(M_KEY_SCAN_PRESSED, (WPARAM)keyNum, 0);
		}
		if ((0XF9^BIT5) == P1 && (0XF9^BIT5) == O4)
		{
			time(&now);
			if (now - T4 >= 3)
			{
				keyNum = 11;
				key_scan[keyNum] = 2;
			}
		}
		if (0XF9 == P1 && (0XF9^BIT5) == O4)
		{
			keyNum = 11;
			time(&now);
			if (now - T4 >= 3)		longPress = 1;
			AfxGetApp()->PostMessage(M_KEY_SCAN_RELEASED, (WPARAM)keyNum, (LPARAM)longPress);
			key_scan[keyNum] = 0;
		}

		O4 = P1;
	}


///////////////////////////////////////////////////////////
	if (5 == line)
	{
		////////////
		if ((0XF9^BIT0) == P1 && 0XF9 == O5)
		{
			time(&T5);
			keyNum = 7;
			key_scan[keyNum] = 1;
			AfxGetApp()->PostMessage(M_KEY_SCAN_PRESSED, (WPARAM)keyNum, 0);
		}
		if ((0XF9^BIT0) == P1 && (0XF9^BIT0) == O5)
		{
			time(&now);
			if (now - T5 >= 3)
			{
				keyNum = 7;
				key_scan[keyNum] = 2;
			}
		}
		if (0XF9 == P1 && (0XF9^BIT0) == O5)
		{
			keyNum = 7;
			time(&now);
			if (now - T5 >= 3)		longPress = 1;
			AfxGetApp()->PostMessage(M_KEY_SCAN_RELEASED, (WPARAM)keyNum, (LPARAM)longPress);
			key_scan[keyNum] = 0;
		}

		////////////
		if ((0XF9^BIT5) == P1 && 0XF9 == O5)
		{
			time(&T5);
			keyNum = 10;
			key_scan[keyNum] = 1;
			AfxGetApp()->PostMessage(M_KEY_SCAN_PRESSED, (WPARAM)keyNum, 0);
		}
		if ((0XF9^BIT5) == P1 && (0XF9^BIT5) == O5)
		{
			time(&now);
			if (now - T5 >= 3)
			{
				keyNum = 10;
				key_scan[keyNum] = 2;
			}
		}
		if (0XF9 == P1 && (0XF9^BIT5) == O5)
		{
			keyNum = 10;
			time(&now);
			if (now - T5 >= 3)		longPress = 1;
			AfxGetApp()->PostMessage(M_KEY_SCAN_RELEASED, (WPARAM)keyNum, (LPARAM)longPress);
			key_scan[keyNum] = 0;
		}

		O5 = P1;
	}


///////////////////////////////////////////////////////////
	if (6 == line)
	{
		////////////
		if ((0XF9^BIT5) == P1 && 0XF9 == O6)
		{
			time(&T6);
			keyNum = 6;
			key_scan[keyNum] = 1;
			AfxGetApp()->PostMessage(M_KEY_SCAN_PRESSED, (WPARAM)keyNum, 0);
		}
		if ((0XF9^BIT5) == P1 && (0XF9^BIT5) == O6)
		{
			time(&now);
			if (now - T6 >= 3)
			{
				keyNum = 6;
				key_scan[keyNum] = 2;
			}
		}
		if (0XF9 == P1 && (0XF9^BIT5) == O6)
		{
			keyNum = 6;
			time(&now);
			if (now - T6 >= 3)		longPress = 1;
			AfxGetApp()->PostMessage(M_KEY_SCAN_RELEASED, (WPARAM)keyNum, (LPARAM)longPress);
			key_scan[keyNum] = 0;
		}

		O6 = P1;
	}

	return 0;
}



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CKsApp::CKsApp() :CApp()
{
	INIT_LIST_HEAD(&fpcCliQ.list);
	i2c0 = -1;
	tUi = 0;
	pwm1 = -1;
}

CKsApp::~CKsApp()
{

}




//////////////////////////////////////////////////////////////////////
// message map & signal map
//////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CKsApp, CApp)
	//{{AFX_MSG_MAP(CKsApp)
	ON_MESSAGE(M_INIT_INSTANCE, &CKsApp::OnInitInstance)
	ON_MESSAGE(M_EXIT_INSTANCE, &CKsApp::OnExitInstance)
	ON_MESSAGE(M_TIMER, &CKsApp::OnTimer)
	ON_MESSAGE(M_UI_INCOMING, &CKsApp::OnUserMessage)
	ON_MESSAGE(M_KEY_SCAN_PRESSED, &CKsApp::OnKeyScanPressed)
	ON_MESSAGE(M_KEY_SCAN_RELEASED, &CKsApp::OnKeyScanReleased)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



//////////////////////////////////////////////////////////////////////
// message handlers
//////////////////////////////////////////////////////////////////////

LRESULT CKsApp::OnInitInstance(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
	int res = 0;
	unsigned char v;
	int err;

	CApp::OnInitInstance(dwType, wParam, lParam, pResult);

	if (-1 == pwm1)		pwm1 = open("/dev/pwm1", O_RDWR);
	if (-1 == i2c0)		i2c0 = open("/dev/i2c-0", O_RDWR);
	if (-1 != i2c0)
	{
		v = 0X00;
		res = i2c_write_data(i2c0, PCA_9555_SLAVE_0, PCA_9555_CONFIG_0, &v, 1);
		if (-1 != res)
		{
			v = 0XFF; i2c_write_data(i2c0, PCA_9555_SLAVE_0, PCA_9555_CONFIG_1, &v, 1);
			i2c_read_data(i2c0, PCA_9555_SLAVE_1, PCA_9555_CONFIG_0, &v, 1);
			v |= BIT0; v |= BIT1; v |= BIT4;
			v &= ~BIT2; v &= ~BIT3; v &= ~BIT5; v &= ~BIT6; v &= ~BIT7;
			i2c_write_data(i2c0, PCA_9555_SLAVE_1, PCA_9555_CONFIG_0, &v, 1);

			i2c_read_data(i2c0, PCA_9555_SLAVE_1, PCA_9555_INPUT_0, &v, 1);
			v &= ~(BIT6); v &= ~BIT7;
			i2c_write_data(i2c0, PCA_9555_SLAVE_1, PCA_9555_OUTPUT_0, &v, 1);

			SetTimer(KEY_SCAN_TIMER_ID, 15, 0);
		}
	}

	err = pthread_create(&tUi, NULL, UiThread, this), assert(0 == err);
	dbg_printf(DBG_THREAD, "UiThread() OK\n");

	return 1;
}

LRESULT CKsApp::OnExitInstance(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
	KillTimer(KEY_SCAN_TIMER_ID);
	KillTimer(PWM1_TIMER_ID);
	KillTimer(BUZZER_START_PWM_TIMER_ID);
	KillTimer(BUZZER_STOP_PWM_TIMER_ID);
	if (-1 != i2c0)		close(i2c0), i2c0 = -1;
	if (-1 != pwm1)		close(pwm1), pwm1 = -1;
	if (tUi)
	{
		pthread_cancel(tUi);
		dbg_printf(DBG_THREAD, "(%s %d) TRYING WAIT() tUi...,   ", __FILE__, __LINE__);
		pthread_join(tUi, NULL);
		dbg_printf(DBG_THREAD, "(%s %d) WAIT() tUi OK\n", __FILE__, __LINE__);
		tUi = 0;
	}

	CApp::OnExitInstance(dwType, wParam, lParam, pResult);
	return 1;
}

LRESULT CKsApp::OnTimer(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
	UINT TimerId = (UINT)wParam;

	if (KEY_SCAN_TIMER_ID == TimerId)
	{
		static unsigned char I = 0;
		if (-1 == PollKeyScan(I++))		return 1;
		I %= 8;
		SetTimer(TimerId, 15, 0);
		return 1;
	}

	if (PWM1_TIMER_ID == TimerId)
	{
		if (-1 != pwm1)
			ioctl(pwm1, PWM_IOCTL_SET_ENABLE, 0);
		return 1;
	}

	return 1;
}

LRESULT CKsApp::OnUserMessage(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
	unsigned char *frame;

	if (0 == wParam)													return 1;
	pthread_cleanup_push(free, (void *)wParam);

	frame = ((unsigned char *)wParam) + 64;
	ProcessUserMessage((frame + 6), (int)lParam);

	pthread_cleanup_pop(1);
	return 1;
}

LRESULT CKsApp::OnKeyScanPressed(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{

	if (-1 != pwm1)
	{
		if (0 == ioctl(pwm1, PWM_IOCTL_SET_ENABLE, 1))
			SetTimer(PWM1_TIMER_ID, 100, 0);
	}

	if (-1 == i2c0)
		return 1;

	return 1;
}

LRESULT CKsApp::OnKeyScanReleased(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{

	if (wParam == KS_RIGHT_TOP)				printf("KS_RIGHT_TOP\n");
	if (wParam == KS_RIGHT_CENTER)			printf("KS_RIGHT_CENTER\n");
	if (wParam == KS_LEFT_TOP)				printf("KS_LEFT_TOP\n");
	if (wParam == KS_LEFT_CENTER)			printf("KS_LEFT_CENTER\n");
	if (wParam == KS_LEFT_BOTTOM)			printf("KS_LEFT_BOTTOM\n");
	if (wParam == KS_RIGHT_BOTTOM)			printf("KS_RIGHT_BOTTOM\n");
	if (wParam == KS_START)					printf("KS_START\n");
	if (wParam == KS_STOP)					printf("KS_STOP\n");
	if (wParam == KS_DELETE)					printf("KS_DELETE\n");
	if (wParam == KS_WORKLOAD_UP)			printf("KS_WORKLOAD_UP\n");
	if (wParam == KS_WORKLOAD_DOWN)		printf("KS_WORKLOAD_DOWN\n");
	if (wParam == KS_STRIDE_UP)				printf("KS_STRIDE_UP\n");
	if (wParam == KS_STRIDE_DOWN)			printf("KS_STRIDE_DOWN\n");
	if (wParam == KS_NUM0 || wParam == KS_NUM1 || wParam == KS_NUM2 || wParam == KS_NUM3 || wParam == KS_NUM4 || wParam == KS_NUM5 || wParam == KS_NUM6 || wParam == KS_NUM7 || wParam == KS_NUM8 || wParam == KS_NUM9)
	{
		printf("KS_NUM: %d\n", wParam - KS_NUM0);
	}

	if (wParam == KS_RIGHT_TOP)
	{
	}
	if (wParam == KS_RIGHT_CENTER)
	{
	}
	if (wParam == KS_RIGHT_BOTTOM)
	{
	}

	return 1;
}

unsigned char CKsApp::ProcessUserMessage(unsigned char *buff, int fd)
{
	unsigned char reply[1024];
	unsigned char out[1024 * 4];
	unsigned char H, F, L;

	H = buff[0];
	if(H & BIT7)
		return 0;
	F = buff[1];
	L = buff[2];
	H |= BIT7; buff[0] = H;


	if(0X79 == F)
	{
		sync(); sync(); sync();
		sync(); sync(); sync();

		reply[0] = H; reply[1] = F; L = 0, reply[2] = L;
		packOurProtocol(33 | (1 << 6), 0, L + 3, (unsigned char *)reply, out);
		if (write(fd, (const void *)out, L + 3 + 8) <= 0){}

		reboot(RB_AUTOBOOT);
		return 0;
	}

	if(0X00 == F && L == 5)
	{
		memcpy(reply, buff, L + 3);
		packOurProtocol(33 | (1 << 6), 0, L + 3, (unsigned char *)reply, out);
		if (write(fd, (const void *)out, L + 3 + 8) <= 0){}
		return 0;
	}

	if(0X95 == F)
	{
		reply[0] = H; reply[1] = F; L = 24, reply[2] = L;
		memcpy(&reply[3 + 0], &key_scan[1], 6);		// S1~6
		reply[3 + 6] = key_scan[9];				// S9
		reply[3 + 7] = key_scan[21];
		reply[3 + 8] = key_scan[7];
		reply[3 + 9] = key_scan[8];
		reply[3 + 10] = key_scan[22];
		reply[3 + 11] = key_scan[23];
		memcpy(&reply[3 + 12], &key_scan[10], 11);
		reply[3 + 23] = key_scan[24];

		if (0 != key_scan[24])
			key_scan[24] = 0;

		packOurProtocol(33 | (1 << 6), 0, L + 3, (unsigned char *)reply, out);
		if (write(fd, (const void *)out, L + 3 + 8) <= 0){}
		return 0;
	}

	return 0;
}



