
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
#include "RhythmApp.h"


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

#define SOCKET_BUFFER_SIZE					(512 * 1024)
#define SERVICE_BUFFER_COUNT				8
#define SERVICE_BUFFER_SIZE				(1024 * 4)
#define RM6T3_SERVICE_LISTEN_PORT			34000


#if 0
#define RM6T3_SERIAL_PORT					"/dev/ttyS0"
#else
#define RM6T3_SERIAL_PORT					"/dev/ttyUSB0"
#endif


#define PT0_SOF_0							0
#define PT0_SOF_1							1
#define PT0_LEN_H							2
#define PT0_LEN_L							3
#define PT0_RESERVE							4
#define PT0_D_RQ_RP							5
#define PT0_D_ID_H							6
#define PT0_D_ID_L							7
#define PT0_DATA_START						8

#define RM6T3_BR_BPS						9600

#define RM6T3_CTRL_CODE_REQ				0xf6
#define RM6T3_CTRL_CODE_ANS				0xf1
#define RM6T3_CTRL_CODE_ACK				0xf2
#define RM6T3_CTRL_CODE_NAK				0xf3
#define RM6T3_CTRL_CODE_END				0xf4
#define RM6T3_CTRL_CODE_SPL				0xf7
#define RM6T3_CTRL_CODE_SPH				0x0f
#define RM6T3_CTRL_CODE_SPB				0xf0

#define RM6T3_INS_CODE_WRITE_FLAG		0x80
#define RM6T3_INS_CODE_OPT_CMD			0x20
#define RM6T3_INS_CODE_F_CMD				0x10
#define RM6T3_INS_CODE_READ_SPU			0x1b
#define RM6T3_INS_CODE_INC_CMD			0x18
#define RM6T3_INS_CODE_INC_OPT			0x28
#define RM6T3_INS_CODE_TROUBLE_MSG		0x1a
#define RM6T3_INS_CODE_FOOTFALL_SPEED	0x30
#define RM6T3_INS_CODE_TOTAL_FOOTFALL	0x31

#define RM6T3_OPT_CMD_FR					7
#define RM6T3_OPT_CMD_RR					6
#define RM6T3_OPT_CMD_STOP				1
#define RM6T3_OPT_CMD_ESP					5
#define RM6T3_OPT_CMD_RST					3
#define RM6T3_OPT_CMD_RPM					0

#define RM6T3_INCL_OPT_CAL					7
#define RM6T3_INCL_OPT_UP					1
#define RM6T3_INCL_OPT_DOWN				1

#define RM6T3_PARAM_MAX_FREQ				10
#define RM6T3_PARAM_WTD					48
#define RM6T3_PARAM_INC_DIFF				54
#define RM6T3_PARAM_INC_DIR				59
#define RM6T3_PARAM_INC_SELECT			88
#define RM6T3_PARAM_VR_MAX				89
#define RM6T3_PARAM_VR_MIN				90

//#define RM6T3_CONST_RATIO					(1.00F / 2.67F)
//#define RM6T3_CONST_RATIO					(6.00F / (115.00F/40.00F))
#define RM6T3_CONST_RATIO					2.0959443923478260869565217391306F

#define NEED_SPL(x)							(x>=0xf0&&x<=0xf7)



//////////////////////////////////////////////////////////////////////
// routines
//////////////////////////////////////////////////////////////////////

// 前滾輪+跑帶厚度後的滾輪直徑 (mm)
//static float rm6t3_wheel_diameter = 80.00F;
static float rm6t3_wheel_diameter = (90.00F+3.00F*2.00F);

// 齒輪比, 齒輪1 帶動齒輪2, 齒輪比=齒輪2/齒輪1
static float rm6t3_gear_ratio = (115.00F/40.00F);

// 齒輪比愈大, 跑帶速度愈小
// 齒輪比愈大, 每km/h, 變頻器設定輸出頻率愈大
// 前滾輪+跑帶厚度後的滾輪直徑愈大, 變頻器設定輸出頻率愈小

//單位 km/hr 所需輸出之變頻器頻率, 5.55HZ
static float rm6t3_freq_unit = RM6T3_CONST_RATIO * rm6t3_gear_ratio * 1000000.00F / (60.00F * 3.1416F * rm6t3_wheel_diameter) / 60.00F;

// 飛輪 RPM 值 / 60, spu
static float rm6t3_freq = 0.00F;

#define RM6T3_VR_OFFSET					5
// VR 最大值
static unsigned short rm6t3_max_vr = 800;
static unsigned short rm6t3_min_vr = 70;


//////////////////////////////////////////////////////////////////////
// routines
//////////////////////////////////////////////////////////////////////

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

unsigned int crc_chk(unsigned char *data, unsigned char length)
{
	int i;
	unsigned int reg_crc = 0xffff;

	while(length--)
	{
		reg_crc ^= *data++;
		for(i = 0; i < 8; i++)
			if (reg_crc & 0x01)	reg_crc = (reg_crc>>1) ^ 0xa001;
			else				reg_crc = reg_crc>>1;
	}
	return reg_crc;
}


static pthread_mutex_t xUatrt = PTHREAD_MUTEX_INITIALIZER;
static struct incoming_t uartQ;

void *UartThread(void *param)
{
	CRhythmApp *app = (CRhythmApp *)param;
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
	if(0 == hUart)														{ goto BAITOUT; }
	ret = WaitForSingleObject(hUart, INFINITE);
	if(WAIT_FAILED == ret)											{ goto BAITOUT; }
	if(WAIT_TIMEOUT == ret)											{ goto WAIT; }

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
			ResetEvent(hUart);
			goto WAIT;
		}

		tv.tv_sec = 1; tv.tv_usec = 1000;
		if (0 > (res = select(maxfd + 1, &rset, 0, 0, &tv)))
		{
			AfxGetApp()->PostMessage(M_EXIT_INSTANCE, 0, 0);
		}
		else if (0 == res)												{ continue; }
		else
		{
			int x;
			struct incoming_t *X[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

			pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &last_type);
			pthread_mutex_lock(&xUatrt);
			pthread_cleanup_push(CleanupLock, (void *)&xUatrt);
			pthread_setcanceltype(last_type, NULL);
			list_for_each_safe(P, Q, &uartQ.list)
			{
				N = list_entry(P, struct incoming_t, list); assert(0 != N);// fd = N->fd;
				if (!FD_ISSET(fd, &rset))
					continue;
				memset(buff, 0, sizeof(buff));
				len = read(fd, buff, sizeof(buff));
				if (len < 0)
				{
					if (errno == EAGAIN)								{ continue; }
					if (errno == EWOULDBLOCK)						{ continue; }
					if (errno == EBADF)								{ close(fd); list_del(P); free(N); continue; }
					AfxGetApp()->PostMessage(M_EXIT_INSTANCE, 0, 0); close(fd); list_del(P); free(N); continue;
				}

				p = (unsigned char *)malloc(64 + len + 32);
				if (!p)			continue;
				memset(p, 0, 64 + len);
				memcpy(p, &len, sizeof(int));
				strcpy((char *)p + sizeof(int), (char *)N->name);
				memcpy(p + 64, buff, len);
				N->dyna = p;
				for (x = 0; x < 32; x++)
					if (0 == X[x])	break;
				if (32 == x)		continue;
				X[x] = N;
			} // list_for_each_safe
			pthread_cleanup_pop(1);

			for (x = 0; x < 32; x++)
			{
				if (0 == X[x])		continue;
				AfxGetApp()->PostMessage(M_UART_DATA_RCV, (WPARAM)X[x]->dyna, (LPARAM)X[x]->fd);
			}
		}
	}


BAITOUT:
	pthread_cleanup_pop(1);

	return 0;
}


static pthread_mutex_t xF = PTHREAD_MUTEX_INITIALIZER;
static struct incoming_t cliQ;

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

	if (-1 == (lsd = socket(AF_INET, SOCK_STREAM, 0)))
	{
		printf("DBG(%s %d)\n", __FUNCTION__, __LINE__);
		AfxGetApp()->PostMessage(M_EXIT_INSTANCE, 0, 0);
		return 0;
	}
	v = 1, setsockopt(lsd, SOL_SOCKET, SO_REUSEADDR, (const void *)&v, sizeof(v));
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET, servaddr.sin_addr.s_addr = htonl(INADDR_ANY), servaddr.sin_port = htons(RM6T3_SERVICE_LISTEN_PORT);
	if (0 > bind(lsd, (struct sockaddr *)&servaddr, sizeof(struct sockaddr_in)))
	{
		printf("DBG(%s %d)\n", __FUNCTION__, __LINE__);
		AfxGetApp()->PostMessage(M_EXIT_INSTANCE, 0, 0);
		return 0;
	}
	if (0 > listen(lsd, 12))
	{
		printf("DBG(%s %d)\n", __FUNCTION__, __LINE__);
		AfxGetApp()->PostMessage(M_EXIT_INSTANCE, 0, 0);
		return 0;
	}
	v = fcntl(lsd, F_GETFL, 0); fcntl(lsd, F_SETFL, v | O_NONBLOCK);
	v = SOCKET_BUFFER_SIZE; setsockopt(lsd, SOL_SOCKET, SO_SNDBUF, (const void *)&v, sizeof(v)); setsockopt(lsd, SOL_SOCKET, SO_RCVBUF, (const void *)&v, sizeof(v));

	pthread_cleanup_push(CleanupFd, (void *)&lsd);

	sigemptyset(&nset);
	sigaddset(&nset, SIGHUP), sigaddset(&nset, SIGINT), sigaddset(&nset, SIGTERM), sigaddset(&nset, SIGUSR1), sigaddset(&nset, SIGALRM);
	pthread_sigmask(SIG_BLOCK, &nset, &oset);
	pthread_cleanup_push(RestoreSigmask, (void *)&oset);

	pthread_cleanup_push(CleanupIncoming, (void *)&cliQ);

	for (;;)
	{
		int x;
		unsigned char *X[64] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
		unsigned int F[64] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

		int clilen, connfd;
		struct sockaddr_in cliaddr;

		maxfd = 0; FD_ZERO(&rset); FD_SET(lsd, &rset);
		if (maxfd < lsd)												maxfd = lsd;

		pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &last_type);
		pthread_mutex_lock(&xF);
		pthread_cleanup_push(CleanupLock, (void *)&xF);
		pthread_setcanceltype(last_type, NULL);
		list_for_each_safe(P, Q, &cliQ.list)
		{
			N = list_entry(P, struct incoming_t, list); assert(0 != N); fd = N->fd; FD_SET(fd, &rset);
			if (maxfd < fd)											maxfd = fd;
		}
		pthread_cleanup_pop(1);


		if (0 > (res = select(maxfd + 1, &rset, 0, 0, 0)))
		{
			printf("DBG(%s %d)\n", __FUNCTION__, __LINE__);
			AfxGetApp()->PostMessage(M_EXIT_INSTANCE, 0, 0);
			break;
		}
		if (0 == res)
		{
			printf("DBG(%s %d)\n", __FUNCTION__, __LINE__);
			continue;
		}

		if (FD_ISSET(lsd, &rset))
		{
			clilen = sizeof(cliaddr);
			connfd = accept(lsd, (struct sockaddr *)&cliaddr, (socklen_t *)&clilen);
			if (-1 != connfd)
			{
				v = fcntl(connfd, F_GETFL, 0), fcntl(connfd, F_SETFL, v | O_NONBLOCK);
				v = SOCKET_BUFFER_SIZE, setsockopt(connfd, SOL_SOCKET, SO_SNDBUF, (const void *)&v, sizeof(v)), setsockopt(connfd, SOL_SOCKET, SO_RCVBUF, (const void *)&v, sizeof(v));
				if (0 != (N = (struct incoming_t *)malloc(sizeof(struct incoming_t) + 1024 * 2)))
				{
					memset(N, 0, sizeof(struct incoming_t)); N->fd = connfd, memcpy(&N->cliaddr, &cliaddr, clilen); N->left = 1; N->offs = 0;

					pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &last_type);
					pthread_mutex_lock(&xF);
					pthread_cleanup_push(CleanupLock, (void *)&xF);
					pthread_setcanceltype(last_type, NULL);
					list_add_tail(&N->list, &cliQ.list);
					pthread_cleanup_pop(1);
				}
				else
				{
					printf("DBG(%s %d)\n", __FUNCTION__, __LINE__);
					close(connfd);
				}
			}
			else
			{
				printf("DBG(%s %d)\n", __FUNCTION__, __LINE__);
			}
			continue;
		}

		pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &last_type);
		pthread_mutex_lock(&xF);
		pthread_cleanup_push(CleanupLock, (void *)&xF);
		pthread_setcanceltype(last_type, NULL);
		list_for_each_safe(P, Q, &cliQ.list)
		{
			N = list_entry(P, struct incoming_t, list); assert(0 != N);
			fd = N->fd;
			if (!FD_ISSET(fd, &rset))									{ continue; }

			left = N->left, offs = N->offs;
			p = N->msg + offs;

			if (offs <= PT0_LEN_L)										left = 1;
			if (PT0_RESERVE == offs)									left = (6 + LEN);

			if (0 > (len = read(fd, p, left)))
			{
				if (errno == EAGAIN)									{ continue;}
				if (errno == EWOULDBLOCK)							{ continue;}
				if (errno == ECONNRESET || errno == EBADF)				{ close(fd); list_del(P); free(N); continue; }
				AfxGetApp()->PostMessage(M_EXIT_INSTANCE, 0, 0); close(fd); list_del(P); free(N); break;
			}
			else if (0 == len)											{ list_del(P); free(N); continue; }

			if (PT0_SOF_0 == offs && 0XFE != N->msg[PT0_SOF_0])	
			{
				N->offs = 0; N->left = 1;
				LEN = 0;
				continue;
			}
			if (PT0_SOF_1 == offs && 0XFE != N->msg[PT0_SOF_1])
			{
				N->offs = 0; N->left = 1;
				LEN = 0;
				continue;
			}
			if (PT0_LEN_L == offs)
			{
				LEN = BUILD_UINT16(N->msg[PT0_LEN_L], N->msg[PT0_LEN_H]);
				if (LEN > 128)	
				{
					N->offs = 0; N->left = 1;
					LEN = 0;
					continue;
				}
			}

			offs += len, N->offs = offs;
			left -= len, N->left = left;

			if (offs > PT0_RESERVE && left <= 0)
			{
				unsigned short crc;
				unsigned short chk;

				crc = (unsigned short)ModbusCrc16(&N->msg[PT0_SOF_0], LEN + 8);
				chk = BUILD_UINT16(N->msg[9 + LEN], N->msg[8 + LEN]);
				if (crc == chk)
				{
					v = LEN + 10;
					frame = (unsigned char *)malloc(64 + 32 + v);
					if (frame)
					{
						memcpy(frame, &v, sizeof(int));
						memcpy(frame + 64, N->msg, v);
						for (x = 0; x < 64; x++)
						{
							if (0 == X[x])
								break;
						}
						if (64 == x)	free(frame), frame = 0;
						if (frame)	X[x] = frame, F[x] = fd;
					}
				}
				else
				{
					printf("CRC NG(%s %d), offs=%d, LEN=%d\n", __FUNCTION__, __LINE__, offs, LEN);
				}
				N->offs = 0; N->left = 1;
				LEN = 0;
				continue;
			}
		}
		pthread_cleanup_pop(1);

		for (x = 0; x < 32; x++)
		{
			if (0 == X[x])		continue;
			AfxGetApp()->PostMessage(M_UI_INCOMING, (WPARAM)X[x], (LPARAM)F[x]);
		}
	}
	pthread_cleanup_pop(1);	//CleanupIncoming
	pthread_cleanup_pop(1);	//RestoreSigmask
	pthread_cleanup_pop(1);	//CleanupFd

	return 0;
}

int pack_ui_frame(unsigned short did, unsigned char ack, unsigned short len, const unsigned char *in, unsigned char *out)
{
	unsigned short i;
	unsigned short crc = 0;

	if (0 == out)
		return 0;

	for (i = 0; i < len + 10; i++)
		*(out + i) = 0;

	out[PT0_SOF_0] = 0XFE; 
	out[PT0_SOF_1] = 0XFE;
	out[PT0_LEN_H] = HI_UINT16(len);
	out[PT0_LEN_L] = LO_UINT16(len);
	out[PT0_RESERVE] = 0;
	out[PT0_D_RQ_RP] = ack;
	out[PT0_D_ID_H] = HI_UINT16(did), out[PT0_D_ID_L] = LO_UINT16(did);

	for (i = 0; i < len; i++)
		*(out + PT0_DATA_START + i) = *(in + i);

	crc = ModbusCrc16(out, len + 8);
	out[8 + len] = HI_UINT16(crc), out[9 + len] = LO_UINT16(crc);
	return (len + 10);
}

void OnRm6FreqSet(int fd, unsigned int value)
{
	unsigned short crc;
	unsigned char chk[6];
	unsigned char data[32];
	unsigned char ins;
	unsigned char dh = 0, dl = 0;
	unsigned char c;
	unsigned char *p = data;
	int len;
	int i;

//value *= 5;

	if (fd < 0)		return;
	dh = HI_UINT16((unsigned short)value);
	dl = LO_UINT16((unsigned short)value);
	*p++ = RM6T3_CTRL_CODE_REQ;
	ins = (RM6T3_INS_CODE_F_CMD | RM6T3_INS_CODE_WRITE_FLAG);
	if NEED_SPL(ins)				*p++ = RM6T3_CTRL_CODE_SPL, *p++ = ins - RM6T3_CTRL_CODE_SPB;
	else							*p++ = ins;
	if NEED_SPL(dh)				*p++ = RM6T3_CTRL_CODE_SPL, *p++ = dh - RM6T3_CTRL_CODE_SPB;
	else							*p++ = dh;
	if NEED_SPL(dl)				*p++ = RM6T3_CTRL_CODE_SPL, *p++ = dl - RM6T3_CTRL_CODE_SPB;
	else							*p++ = dl;
	chk[0] = ins, chk[1] = dh, chk[2] = dl;
	crc = crc_chk(chk, 3);
	c = HI_UINT16(crc);
	if NEED_SPL(c)				*p++ = RM6T3_CTRL_CODE_SPL, *p++ = c - RM6T3_CTRL_CODE_SPB;
	else							*p++ = c;
	c = LO_UINT16(crc);
	if NEED_SPL(c)				*p++ = RM6T3_CTRL_CODE_SPL, *p++ = c - RM6T3_CTRL_CODE_SPB;
	else							*p++ = c;
	*p++ = RM6T3_CTRL_CODE_END;
	len = int(p - data);
	write(fd, (const void*)data, len);

	if (0 != rm6t3_freq_unit)
		printf("OnRm6FreqSet (0X0091): [FREQ]=0X%04X %u(0.01HZ), [SPEED]=%.02f(KM/H)\n>>", value, value, (float)value / rm6t3_freq_unit / 100.00F);
	for (i = 0; i < len; i++)
	{
		if (i == len - 1)	printf("%02X\n", data[i]);
		else				printf("%02X-", data[i]);
	}
}

// FR, RR, ESP, STOP
void OnRm6ForwardRunSet(int fd, unsigned int value)
{
	unsigned short crc;
	unsigned char chk[6];
	unsigned char data[32];
	unsigned char ins;
	unsigned char dh = 0, c = 0;
	unsigned char *p = data;
	int len;

	if (fd < 0)									return;
	*p++ = RM6T3_CTRL_CODE_REQ;
	ins = (RM6T3_INS_CODE_OPT_CMD | RM6T3_INS_CODE_WRITE_FLAG);
	if NEED_SPL(ins)								*p++ = RM6T3_CTRL_CODE_SPL, *p++ = ins - RM6T3_CTRL_CODE_SPB;
	else											*p++ = ins;
	if (RM6T3_OPT_CMD_RPM == value)				return;
	else if (RM6T3_OPT_CMD_FR == value)			dh = 1 << RM6T3_OPT_CMD_FR;
	else if (RM6T3_OPT_CMD_RR == value)			dh = 1 << RM6T3_OPT_CMD_RR;
	else if (RM6T3_OPT_CMD_ESP == value)			dh = 1 << RM6T3_OPT_CMD_ESP;
	else if (RM6T3_OPT_CMD_RST == value)			dh = 1 << RM6T3_OPT_CMD_RST;
	else if (RM6T3_OPT_CMD_STOP == value)		{ dh = 0; }
	else											return;
	if NEED_SPL(dh)								*p++ = RM6T3_CTRL_CODE_SPL, *p++ = dh - RM6T3_CTRL_CODE_SPB;
	else											*p++ = dh;
	chk[0] = ins, chk[1] = dh;
	crc = crc_chk(chk, 2);
	c = HI_UINT16(crc);
	if NEED_SPL(c)								*p++ = RM6T3_CTRL_CODE_SPL, *p++ = c - RM6T3_CTRL_CODE_SPB;
	else											*p++ = c;
	c = LO_UINT16(crc);
	if NEED_SPL(c)								*p++ = RM6T3_CTRL_CODE_SPL, *p++ = c - RM6T3_CTRL_CODE_SPB;
	else											*p++ = c;
	*p++ = RM6T3_CTRL_CODE_END;
	len = int(p - data);
	write(fd, (const void*)data, len);
}

void OnRm6IncineSet(int fd, unsigned int value)
{
	unsigned short crc;
	unsigned char chk[6];
	unsigned char data[32];
	unsigned char ins;
	unsigned char dh = 0, dl = 0, c = 0;
	unsigned char *p = data;
	int len;

	if (fd < 0)		return;
	dh = HI_UINT16((unsigned short)value);
	dl = LO_UINT16((unsigned short)value);

	*p++ = RM6T3_CTRL_CODE_REQ;
	ins = (RM6T3_INS_CODE_INC_CMD | RM6T3_INS_CODE_WRITE_FLAG);
	if NEED_SPL(ins)				*p++ = RM6T3_CTRL_CODE_SPL, *p++ = ins - RM6T3_CTRL_CODE_SPB;
	else							*p++ = ins;
	if NEED_SPL(dh)				*p++ = RM6T3_CTRL_CODE_SPL, *p++ = dh - 0XF0;
	else							*p++ = dh;
	if NEED_SPL(dl)				*p++ = RM6T3_CTRL_CODE_SPL, *p++ = dl - 0XF0;
	else							*p++ = dl;

	chk[0] = ins, chk[1] = dh, chk[2] = dl;
	crc = crc_chk(chk, 3);
	c = HI_UINT16(crc);
	if NEED_SPL(c)				*p++ = RM6T3_CTRL_CODE_SPL, *p++ = c - 0XF0;
	else							*p++ = c;
	c = LO_UINT16(crc);
	if NEED_SPL(c)				*p++ = RM6T3_CTRL_CODE_SPL, *p++ = c - 0XF0;
	else							*p++ = c;
	*p++ = RM6T3_CTRL_CODE_END;
	len = int(p - data);
	write(fd, (const void*)data, len);

	printf("OnRm6IncineSet (0X0090/0X0093): [INC_CMD]=%u(AD)\n>>", value);
	for (int i = 0; i < len; i++)
	{
		if (i == len - 1)	printf("%02X\n", data[i]);
		else				printf("%02X-", data[i]);
	}
}

void OnRm6AutoCalibrationSet(int fd)
{
	unsigned short crc;
	unsigned char chk[6];
	unsigned char data[32];
	unsigned char ins;
	unsigned char *p = data;
	int len;
	unsigned char c;

	if (fd < 0)		return;
	*p++ = RM6T3_CTRL_CODE_REQ;
	ins = (RM6T3_INS_CODE_INC_OPT | RM6T3_INS_CODE_WRITE_FLAG);
	if NEED_SPL(ins)								*p++ = RM6T3_CTRL_CODE_SPL, *p++ = ins - RM6T3_CTRL_CODE_SPB;
	else											*p++ = ins;
	*p++ = (1 << RM6T3_INCL_OPT_CAL);
	chk[0] = ins, chk[1] = 0X80;
	crc = crc_chk(chk, 2);
	c = HI_UINT16(crc);
	if NEED_SPL(c)				*p++ = RM6T3_CTRL_CODE_SPL, *p++ = c - 0XF0;
	else							*p++ = c;
	c = LO_UINT16(crc);
	if NEED_SPL(c)				*p++ = RM6T3_CTRL_CODE_SPL, *p++ = c - 0XF0;
	else							*p++ = c;
	*p++ = RM6T3_CTRL_CODE_END;
	len = int(p - data);
	write(fd, (const void*)data, len);
}

void Rm6ParamGet(int fd, unsigned char param)
{
	unsigned short crc;
	unsigned char chk[6];
	unsigned char data[32];
	unsigned char *p = data;
	unsigned char c;
	unsigned char dh = 0, dm = 0, dl = 0;
	int len;

	if (fd < 0)		return;

	*p++ = RM6T3_CTRL_CODE_REQ;
	c = 0X7F;
	if NEED_SPL(c)				*p++ = RM6T3_CTRL_CODE_SPL, *p++ = c - RM6T3_CTRL_CODE_SPB;
	else							*p++ = c;

	c = param;
	if NEED_SPL(c)				*p++ = RM6T3_CTRL_CODE_SPL, *p++ = c - RM6T3_CTRL_CODE_SPB;
	else							*p++ = c;

	if NEED_SPL(dh)				*p++ = RM6T3_CTRL_CODE_SPL, *p++ = dh - RM6T3_CTRL_CODE_SPB;
	else							*p++ = dh;
	if NEED_SPL(dm)				*p++ = RM6T3_CTRL_CODE_SPL, *p++ = dm - RM6T3_CTRL_CODE_SPB;
	else							*p++ = dm;
	if NEED_SPL(dl)				*p++ = RM6T3_CTRL_CODE_SPL, *p++ = dl - RM6T3_CTRL_CODE_SPB;
	else							*p++ = dl;

	chk[0] = 0X7F, chk[1] = param, chk[2] = dh, chk[3] = dm, chk[4] = dl;
	crc = crc_chk(chk, 5);
	c = HI_UINT16(crc);
	if NEED_SPL(c)				*p++ = RM6T3_CTRL_CODE_SPL, *p++ = c - RM6T3_CTRL_CODE_SPB;
	else							*p++ = c;
	c = LO_UINT16(crc);
	if NEED_SPL(c)				*p++ = RM6T3_CTRL_CODE_SPL, *p++ = c - RM6T3_CTRL_CODE_SPB;
	else							*p++ = c;
	*p++ = RM6T3_CTRL_CODE_END;
	len = int(p - data);
	write(fd, (const void*)data, len);
}

void Rm6ParamSet(int fd, unsigned char param, unsigned int value)
{
	unsigned short crc;
	unsigned char chk[6];
	unsigned char data[32];
	unsigned char *p = data;
	unsigned char c;
	unsigned char dh = 0, dm = 0, dl = 0;
	int len;

	if (fd < 0)					return;

	if (RM6T3_PARAM_WTD == param)		printf("WDT=%d\n", value);
	if (RM6T3_PARAM_VR_MAX == param)	rm6t3_min_vr = value;
	if (RM6T3_PARAM_VR_MAX == param)
	{
		if (value > 3000)			value = 3000;
		rm6t3_max_vr = value;
	}

	*p++ = RM6T3_CTRL_CODE_REQ;
	c = 0XFF;
	if NEED_SPL(c)				*p++ = RM6T3_CTRL_CODE_SPL, *p++ = c - 0XF0;
	else							*p++ = c;

	c = param;
	if NEED_SPL(c)				*p++ = RM6T3_CTRL_CODE_SPL, *p++ = c - 0XF0;
	else							*p++ = c;

	dh = BREAK_UINT32(value, 2);
	dm = BREAK_UINT32(value, 1);
	dl = BREAK_UINT32(value, 0);

	if NEED_SPL(dh)				*p++ = RM6T3_CTRL_CODE_SPL, *p++ = dh - 0XF0;
	else							*p++ = dh;
	if NEED_SPL(dm)				*p++ = RM6T3_CTRL_CODE_SPL, *p++ = dm - 0XF0;
	else							*p++ = dm;
	if NEED_SPL(dl)				*p++ = RM6T3_CTRL_CODE_SPL, *p++ = dl- 0XF0;
	else							*p++ = dl;

	chk[0] = 0XFF, chk[1] = param, chk[2] = dh, chk[3] = dm, chk[4] = dl;
	crc = crc_chk(chk, 5);
	c = HI_UINT16(crc);
	if NEED_SPL(c)				*p++ = RM6T3_CTRL_CODE_SPL, *p++ = c - 0XF0;
	else							*p++ = c;
	c = LO_UINT16(crc);
	if NEED_SPL(c)				*p++ = RM6T3_CTRL_CODE_SPL, *p++ = c - 0XF0;
	else							*p++ = c;
	*p++ = RM6T3_CTRL_CODE_END;
	len = int(p - data);
	write(fd, (const void*)data, len);
}

/*
void OnRm6MaxFreqSet(int fd, unsigned int value)
{
	unsigned short crc;
	unsigned char chk[6];
	unsigned char data[32];
	unsigned char *p = data;
	unsigned char c;
	unsigned char dh = 0, dm = 0, dl = 0;
	int len;

	if (fd < 0)		return;
	if (value > 4000)	return;

	*p++ = RM6T3_CTRL_CODE_REQ;
	// *p++ = 0XFF;
	c = 0XFF;
	if NEED_SPL(c)		*p++ = RM6T3_CTRL_CODE_SPL, *p++ = c - 0XF0;
	else							*p++ = c;

	// 最大頻率設定
	*p++ = RM6T3_PARAM_MAX_FREQ;
	*p++ = dh;
	dm = HI_UINT16((unsigned short)value);
	if (HI_UINT8(dm) == 0X0F)	*p++ = RM6T3_CTRL_CODE_SPL, *p++ = dm - 0XF0;
	else							*p++ = dm;
	if (HI_UINT8(dl) == 0X0F)		*p++ = RM6T3_CTRL_CODE_SPL, *p++ = dl- 0XF0;
	else							*p++ = dl;

	chk[0] = 0XFF, chk[1] = RM6T3_PARAM_MAX_FREQ, chk[2] = dh, chk[3] = dm, chk[4] = dl;
	crc = crc_chk(chk, 5);
	c = HI_UINT16(crc);
	if NEED_SPL(c)		*p++ = RM6T3_CTRL_CODE_SPL, *p++ = c - 0XF0;
	else							*p++ = c;
	c = LO_UINT16(crc);
	if NEED_SPL(c)		*p++ = RM6T3_CTRL_CODE_SPL, *p++ = c - 0XF0;
	else							*p++ = c;
	*p++ = RM6T3_CTRL_CODE_END;
	len = int(p - data);
	write(fd, (const void*)data, len);
}

void OnRm6InclineDifSet(int fd, unsigned int value)
{
	unsigned short crc;
	unsigned char chk[6];
	unsigned char data[32];
	unsigned char c;
	unsigned char dh = 0, dm = 0, dl = 0;
	unsigned char *p = data;
	int len;

	if (fd < 0)		return;
	if (value > 255)	return;

	*p++ = RM6T3_CTRL_CODE_REQ;
	// *p++ = 0XFF;
	c = 0XFF;
	if NEED_SPL(c)		*p++ = RM6T3_CTRL_CODE_SPL, *p++ = c - 0XF0;
	else							*p++ = c;

	*p++ = RM6T3_PARAM_INC_DIFF;
	*p++ = dh;
	*p++ = dm;
	dl = (unsigned char)value;
	if (HI_UINT8(dl) == 0X0F)		*p++ = RM6T3_CTRL_CODE_SPL, *p++ = dl - 0XF0;
	else							*p++ = dl;

	chk[0] = 0XFF, chk[1] = RM6T3_PARAM_INC_DIFF, chk[2] = dh, chk[3] = dm, chk[4] = dl;
	crc = crc_chk(chk, 5);
	c = HI_UINT16(crc);
	if NEED_SPL(c)		*p++ = RM6T3_CTRL_CODE_SPL, *p++ = c - 0XF0;
	else							*p++ = c;
	c = LO_UINT16(crc);
	if NEED_SPL(c)		*p++ = RM6T3_CTRL_CODE_SPL, *p++ = c - 0XF0;
	else							*p++ = c;
	*p++ = RM6T3_CTRL_CODE_END;
	len = int(p - data);
	write(fd, (const void*)data, len);
}

void OnRm6WdtSet(int fd, unsigned int value)
{
	unsigned short crc;
	unsigned char chk[6];
	unsigned char data[32];
	unsigned char c;
	unsigned char dh = 0, dm = 0, dl = 0;
	unsigned char *p = data;
	int len;

	if (fd < 0)		return;
	if (value > 255)	return;

	*p++ = RM6T3_CTRL_CODE_REQ;
	// *p++ = 0XFF;
	c = 0XFF;
	if NEED_SPL(c)		*p++ = RM6T3_CTRL_CODE_SPL, *p++ = c - 0XF0;
	else							*p++ = c;

	// 通訊停止時間
	*p++ = RM6T3_PARAM_WTD;
	*p++ = dh;
	*p++ = dm;
	dl = (unsigned char)value;
	if (HI_UINT8(dl) == 0X0F)		*p++ = RM6T3_CTRL_CODE_SPL, *p++ = dl - 0XF0;
	else							*p++ = dl;

	chk[0] = 0XFF, chk[1] = RM6T3_PARAM_WTD, chk[2] = dh, chk[3] = dm, chk[4] = dl;
	crc = crc_chk(chk, 5);
	c = HI_UINT16(crc);
	if NEED_SPL(c)		*p++ = RM6T3_CTRL_CODE_SPL, *p++ = c - 0XF0;
	else							*p++ = c;
	c = LO_UINT16(crc);
	if NEED_SPL(c)		*p++ = RM6T3_CTRL_CODE_SPL, *p++ = c - 0XF0;
	else							*p++ = c;
	*p++ = RM6T3_CTRL_CODE_END;
	len = int(p - data);
	write(fd, (const void*)data, len);
}
*/

void OnRm6TotalFootFallGet(int fd)
{
	unsigned short crc;
	unsigned char chk[6];
	unsigned char data[32];
	unsigned char ins;
	unsigned char *p = data;
	int len;
	unsigned char c;

	if (fd < 0)		return;
	*p++ = RM6T3_CTRL_CODE_REQ;
	ins = RM6T3_INS_CODE_TOTAL_FOOTFALL;
	if NEED_SPL(ins)				*p++ = RM6T3_CTRL_CODE_SPL, *p++ = ins - RM6T3_CTRL_CODE_SPB;
	else							*p++ = ins;
	chk[0] = ins;
	crc = crc_chk(chk, 1);
	c = HI_UINT16(crc);
	if NEED_SPL(c)				*p++ = RM6T3_CTRL_CODE_SPL, *p++ = c - 0XF0;
	else							*p++ = c;
	c = LO_UINT16(crc);
	if NEED_SPL(c)				*p++ = RM6T3_CTRL_CODE_SPL, *p++ = c - 0XF0;
	else							*p++ = c;
	*p++ = RM6T3_CTRL_CODE_END;
	len = int(p - data);
	write(fd, (const void*)data, len);
}

void OnRm6FootFallSpeedGet(int fd)
{
	unsigned short crc;
	unsigned char chk[6];
	unsigned char data[32];
	unsigned char ins;
	unsigned char *p = data;
	int len;
	unsigned char c;

	if (fd < 0)		return;
	*p++ = RM6T3_CTRL_CODE_REQ;
	ins = RM6T3_INS_CODE_FOOTFALL_SPEED;
	if NEED_SPL(ins)				*p++ = RM6T3_CTRL_CODE_SPL, *p++ = ins - RM6T3_CTRL_CODE_SPB;
	else							*p++ = ins;
	chk[0] = ins;
	crc = crc_chk(chk, 1);
	c = HI_UINT16(crc);
	if NEED_SPL(c)				*p++ = RM6T3_CTRL_CODE_SPL, *p++ = c - 0XF0;
	else							*p++ = c;
	c = LO_UINT16(crc);
	if NEED_SPL(c)				*p++ = RM6T3_CTRL_CODE_SPL, *p++ = c - 0XF0;
	else							*p++ = c;
	*p++ = RM6T3_CTRL_CODE_END;
	len = int(p - data);
	write(fd, (const void*)data, len);
}

void OnRm6SpuGet(int fd)
{
	unsigned short crc;
	unsigned char chk[6];
	unsigned char data[32];
	unsigned char ins;
	unsigned char *p = data;
	int len;
	unsigned char c;

	if (fd < 0)		return;
	*p++ = RM6T3_CTRL_CODE_REQ;
	ins = RM6T3_INS_CODE_READ_SPU;
	if NEED_SPL(ins)				*p++ = RM6T3_CTRL_CODE_SPL, *p++ = ins - RM6T3_CTRL_CODE_SPB;
	else							*p++ = ins;
	chk[0] = ins;
	crc = crc_chk(chk, 1);
	c = HI_UINT16(crc);
	if NEED_SPL(c)				*p++ = RM6T3_CTRL_CODE_SPL, *p++ = c - 0XF0;
	else							*p++ = c;
	c = LO_UINT16(crc);
	if NEED_SPL(c)				*p++ = RM6T3_CTRL_CODE_SPL, *p++ = c - 0XF0;
	else							*p++ = c;
	*p++ = RM6T3_CTRL_CODE_END;
	len = int(p - data);
	write(fd, (const void*)data, len);
}

void OnRm6TroubleMsgGet(int fd)
{
	unsigned short crc;
	unsigned char chk[6];
	unsigned char data[32];
	unsigned char ins;
	unsigned char *p = data;
	int len;
	unsigned char c;

	if (fd < 0)		return;
	*p++ = RM6T3_CTRL_CODE_REQ;
	ins = RM6T3_INS_CODE_TROUBLE_MSG;
	if NEED_SPL(ins)				*p++ = RM6T3_CTRL_CODE_SPL, *p++ = ins - RM6T3_CTRL_CODE_SPB;
	else							*p++ = ins;
	chk[0] = ins;
	crc = crc_chk(chk, 1);
	c = HI_UINT16(crc);
	if NEED_SPL(c)				*p++ = RM6T3_CTRL_CODE_SPL, *p++ = c - 0XF0;
	else							*p++ = c;
	c = LO_UINT16(crc);
	if NEED_SPL(c)				*p++ = RM6T3_CTRL_CODE_SPL, *p++ = c - 0XF0;
	else							*p++ = c;
	*p++ = RM6T3_CTRL_CODE_END;
	len = int(p - data);
	write(fd, (const void*)data, len);
}

void OnRm6TotalFootFallRsp(int fd, unsigned int value)
{
	unsigned char buffer[64];
	int len = 0;
	unsigned char data[] = {0,0,0,0};

	if (-1 == fd)		return;
	data[0] = BREAK_UINT32(value, 3);
	data[1] = BREAK_UINT32(value, 2);
	data[2] = BREAK_UINT32(value, 1);
	data[3] = BREAK_UINT32(value, 0);
	len = pack_ui_frame(0X009A, 1, 4, (unsigned char *)&data, buffer);
	if(len)			write(fd, (const void*)buffer, len);
}

void OnRm6FootFallSpeedRsp(int fd, unsigned int value)
{
	unsigned char buffer[64];
	int len = 0;
	unsigned char data[] = {0,0,0,0};

	if (-1 == fd)		return;
	data[0] = BREAK_UINT32(value, 3);
	data[1] = BREAK_UINT32(value, 2);
	data[2] = BREAK_UINT32(value, 1);
	data[3] = BREAK_UINT32(value, 0);
	len = pack_ui_frame(0X0099, 1, 4, (unsigned char *)&data, buffer);
	if(len)			write(fd, (const void*)buffer, len);
}

void OnRm6SpuRsp(int fd, unsigned int value)
{
	unsigned char buffer[64];
	int len = 0;
	unsigned char data[] = {0,0,0,0};

	if (-1 == fd)		return;
	data[0] = BREAK_UINT32(value, 3);
	data[1] = BREAK_UINT32(value, 2);
	data[2] = BREAK_UINT32(value, 1);
	data[3] = BREAK_UINT32(value, 0);
	len = pack_ui_frame(0X0094, 1, 4, (unsigned char *)&data, buffer);
	if(len)			write(fd, (const void*)buffer, len);
}

void OnRm6TroubleMsgRsp(int fd, unsigned int value)
{
	unsigned char buffer[64];
	int len = 0;
	unsigned char data[] = {0,0,0,0};

	if (-1 == fd)		return;
	data[0] = BREAK_UINT32(value, 3);
	data[1] = BREAK_UINT32(value, 2);
	data[2] = BREAK_UINT32(value, 1);
	data[3] = BREAK_UINT32(value, 0);
	len = pack_ui_frame(0X0097, 1, 4, (unsigned char *)&data, buffer);
	if(len)			write(fd, (const void*)buffer, len);
}

void OnRm6StatusRsp(int fd, unsigned int value)
{
	unsigned char buffer[64];
	int len = 0;
	unsigned char data[] = {0,0,0,0};

	if (-1 == fd)		return;
	data[0] = BREAK_UINT32(value, 3);
	data[1] = BREAK_UINT32(value, 2);
	data[2] = BREAK_UINT32(value, 1);
	data[3] = BREAK_UINT32(value, 0);
	len = pack_ui_frame(0X0096, 1, 4, (unsigned char *)&data, buffer);
	if(len)			write(fd, (const void*)buffer, len);
}

void OnRm6ParamRead(unsigned char param, unsigned int value)
{
	int last_type = 0;
	struct list_head *P, *Q;
	struct incoming_t *N;
	int len = 0;

	unsigned char buffer[64];
	unsigned char data[] = {0,0,0,0};
	unsigned int val = 0;

	data[0] = param;
	data[1] = BREAK_UINT32(value, 2);
	data[2] = BREAK_UINT32(value, 1);
	data[3] = BREAK_UINT32(value, 0);

	if (RM6T3_PARAM_VR_MAX == param)
	{
		val = BUILD_UINT32(data[3], data[2], data[1], 0);
		if (val <= 3000)
			rm6t3_max_vr = val;
		printf("MAX-VR=%d\n", rm6t3_max_vr);
	}

	if (RM6T3_PARAM_VR_MIN == param)
	{
		val = BUILD_UINT32(data[3], data[2], data[1], 0);
		rm6t3_min_vr = val;
		printf("MIN-VR=%d\n", rm6t3_min_vr);
	}

	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &last_type);
	pthread_mutex_lock(&xF);
	pthread_cleanup_push(CleanupLock, (void *)&xF);
	pthread_setcanceltype(last_type, NULL);
	list_for_each_safe(P, Q, &cliQ.list)
	{
		N = list_entry(P, struct incoming_t, list);

		len = pack_ui_frame(0X009E, 1, 4, (unsigned char *)&data, buffer);
		if(len)	write(N->fd, (const void*)buffer, len);
	}
	pthread_cleanup_pop(1);
}



//////////////////////////////////////////////////////////////////////
// class member
//////////////////////////////////////////////////////////////////////

int CRhythmApp::AttachUart(const char *path, int baudrate)
{
	struct list_head *P, *Q;
	struct incoming_t *N;
	struct termios term;
	int v = 0, f = 0, fd = -1;
	int last_type;

	if (!TestUart(path))													{ return 0;}
	if (baudrate != 9600 && baudrate != 19200 && baudrate != 38400 && baudrate != 57600 && baudrate != 115200 && baudrate != 4800)
		baudrate = 9600;
	if (0 > (fd = open(path, O_RDWR | O_NOCTTY | O_NDELAY)))				{ return 0; }
	if (!isatty(fd))															{ close(fd); return 0; }

	memset(&term, 0, sizeof(term));
	term.c_cflag |= B9600, 	term.c_cflag |= CLOCAL, term.c_cflag |= CREAD, term.c_cflag &= ~PARENB, term.c_cflag &= ~CSTOPB;
	term.c_cflag &= ~CSIZE, term.c_cflag |= CS8, term.c_iflag = IGNPAR, term.c_cc[VMIN] = 1, term.c_cc[VTIME] = 0;
	if (115200 == baudrate)												cfsetispeed(&term, B115200), cfsetospeed(&term, B115200);
	else if (57600 == baudrate)											cfsetispeed(&term, B57600), cfsetospeed(&term, B57600);
	else if (38400 == baudrate)											cfsetispeed(&term, B38400), cfsetospeed(&term, B38400);
	else if (19200 == baudrate)											cfsetispeed(&term, B19200), cfsetospeed(&term, B19200);
	else if (9600 == baudrate)												cfsetispeed(&term, B9600), cfsetospeed(&term, B9600);
	else if (4800 == baudrate)												cfsetispeed(&term, B4800), cfsetospeed(&term, B4800);
	else																	cfsetispeed(&term, B9600), cfsetospeed(&term, B9600);
	tcsetattr(fd, TCSANOW, &term); tcflush(fd, TCIFLUSH);
	v = fcntl(fd, F_GETFL, 0); fcntl(fd, F_SETFL, v | O_NONBLOCK);

	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &last_type);
	pthread_mutex_lock(&xUatrt);
	pthread_cleanup_push(CleanupLock, (void *)&xUatrt);
	pthread_setcanceltype(last_type, NULL);
	list_for_each_safe(P, Q, &uartQ.list)
	{
		N = list_entry(P, struct incoming_t, list); assert(0 != N);
		if (0 == strcmp(path, N->name))									{ f = 1; break; }
	}
	pthread_cleanup_pop(1);

	if (1 == f)															{ close(fd); return 0; }

	if (0 == (N = (struct incoming_t *)malloc(sizeof(struct incoming_t) + 1024)))	{ close(fd); return 0; }
	memset(N, 0, sizeof(struct incoming_t));
	N->fd = fd, strcpy(N->name, path);

	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &last_type);
	pthread_mutex_lock(&xUatrt);
	pthread_cleanup_push(CleanupLock, (void *)&xUatrt);
	pthread_setcanceltype(last_type, NULL);
	list_add_tail(&N->list, &uartQ.list);
	pthread_cleanup_pop(1);

	if (hUart)																SetEvent(hUart);
	return fd;
}

int CRhythmApp::TestUart(const char *path)
{
	struct stat lbuf;
	char pattern[128];
	int cflags = REG_EXTENDED;
	regex_t regx;
	int nmatch = 10;
	regmatch_t pmatch[10];

	if (0 == path)														return 0;
	memset(&regx, 0, sizeof(regex_t));
	strcpy(pattern, "^/dev/(tty[UMS]{1}[A-Z]{0,}[0-9]{1})$");
	if (0 != regcomp(&regx, pattern, cflags))									return 0;
	if (0 != regexec(&regx, path, nmatch, pmatch, 0))							{ regfree(&regx); return 0; }
	regfree(&regx);
	if(0 > stat(path, &lbuf))												return 0;
	if(!S_ISCHR(lbuf.st_mode))												return 0;
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

CRhythmApp::CRhythmApp() :CApp()
{
	hUart = CreateEvent(0, 1, 0, "UART");
	INIT_LIST_HEAD(&uartQ.list);
	INIT_LIST_HEAD(&cliQ.list);

	tUi = 0; tUart = 0, ttyS0 = -1;
}

CRhythmApp::~CRhythmApp()
{
	if (hUart)																CloseEvent(hUart);
}


//////////////////////////////////////////////////////////////////////
// message map & signal map
//////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CRhythmApp, CApp)
	//{{AFX_MSG_MAP(CRhythm)
	ON_MESSAGE(M_INIT_INSTANCE, &CRhythmApp::OnInitInstance)
	ON_MESSAGE(M_EXIT_INSTANCE, &CRhythmApp::OnExitInstance)
	ON_MESSAGE(M_TIMER, &CRhythmApp::OnTimer)
	ON_MESSAGE(M_UART_DATA_RCV, &CRhythmApp::OnUartDataRcv)
	ON_MESSAGE(M_UI_INCOMING, &CRhythmApp::OnUserMessage)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



//////////////////////////////////////////////////////////////////////
// message handlers
//////////////////////////////////////////////////////////////////////


// 0XFE CMD0 CMD1 DATA0 DATA1 DATA2 DATA3 XORSUM
LRESULT CRhythmApp::OnUserMessage(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
	unsigned char *frame;
	unsigned short did;
	unsigned int val;
	unsigned int LEN;
	unsigned short crc;
	//int fd;

	if (0 == wParam)														return 1;
	pthread_cleanup_push(free, (void *)wParam);
	if (-1 == ttyS0)		goto baitout;

	frame = ((unsigned char *)wParam) + 64;
	did = BUILD_UINT16(frame[PT0_D_ID_L], frame[PT0_D_ID_H]);
	LEN = BUILD_UINT16(frame[PT0_LEN_L], frame[PT0_LEN_H]);

	// HELLO
	if (0X0000 == did)
	{
		frame[PT0_D_RQ_RP] = 1;
		crc = ModbusCrc16(frame, LEN + 8);
		frame[8 + LEN] = HI_UINT16(crc), frame[9 + LEN] = LO_UINT16(crc);
		write((int)lParam, (const void*)frame, LEN + 10);
		goto baitout;
	}

	// 滾輪速度, 以變頻器頻率計算
	if (0X0091 == did && LEN == 4)
	{
		val = BUILD_UINT32(frame[PT0_DATA_START + 3], frame[PT0_DATA_START + 2], frame[PT0_DATA_START + 1], frame[PT0_DATA_START]);
		if (val > 40000)
			goto baitout;
		rm6t3_freq = val;
		OnRm6FreqSet(ttyS0, val);
		if (val)	SetTimer(RM6T3_FR_TIMER_ID, 23, 0);
		else		SetTimer(RM6T3_RRFR_TIMER_ID, 23, 0);

		frame[PT0_D_RQ_RP] = 1;
		crc = ModbusCrc16(frame, LEN + 8);
		frame[8 + LEN] = HI_UINT16(crc), frame[9 + LEN] = LO_UINT16(crc);
		write((int)lParam, (const void*)frame, LEN + 10);
		goto baitout;
	}

	// 滾輪速度, 以 0.1 KM/HR 計算
	if (0X009B == did && LEN == 4)
	{
		val = BUILD_UINT32(frame[PT0_DATA_START + 3], frame[PT0_DATA_START + 2], frame[PT0_DATA_START + 1], frame[PT0_DATA_START]);

		// 單位 0.1KM/HR
		rm6t3_freq = (float)val / 10.00F * (rm6t3_freq_unit * 100.00F);
		OnRm6FreqSet(ttyS0, (unsigned int)rm6t3_freq);
		if (val)	SetTimer(RM6T3_FR_TIMER_ID, 23, 0);
		else		SetTimer(RM6T3_RRFR_TIMER_ID, 23, 0);

		frame[PT0_D_RQ_RP] = 1;
		crc = ModbusCrc16(frame, LEN + 8);
		frame[8 + LEN] = HI_UINT16(crc), frame[9 + LEN] = LO_UINT16(crc);
		write((int)lParam, (const void*)frame, LEN + 10);
	}

	// RM6T3_OPT_CMD_FR, RM6T3_OPT_CMD_RR, RM6T3_OPT_CMD_STOP, RM6T3_OPT_CMD_ESP, RM6T3_OPT_CMD_RST, RM6T3_OPT_CMD_RPM
	// 正轉, 反轉, 降速停止, 緊急停止, 異常重置
	if (0X0092 == did && LEN == 4)
	{
		val = BUILD_UINT32(frame[PT0_DATA_START + 3], frame[PT0_DATA_START + 2], frame[PT0_DATA_START + 1], frame[PT0_DATA_START]);
		if (val != RM6T3_OPT_CMD_FR && val != RM6T3_OPT_CMD_RR && val != RM6T3_OPT_CMD_RST &&  val != RM6T3_OPT_CMD_STOP && val != RM6T3_OPT_CMD_RPM)
			goto baitout;
		OnRm6ForwardRunSet(ttyS0, val);
		frame[PT0_D_RQ_RP] = 1;
		crc = ModbusCrc16(frame, LEN + 8);
		frame[8 + LEN] = HI_UINT16(crc), frame[9 + LEN] = LO_UINT16(crc);
		write((int)lParam, (const void*)frame, LEN + 10);
		goto baitout;
	}

	// 揚角設定 %
	if (0X0090 == did && LEN == 4)
	{
		// 百分比
		val = BUILD_UINT32(frame[PT0_DATA_START + 3], frame[PT0_DATA_START + 2], frame[PT0_DATA_START + 1], frame[PT0_DATA_START]);
		if (val > 1000)						goto baitout;
		if (rm6t3_max_vr <= rm6t3_min_vr)	goto baitout;
		OnRm6IncineSet(ttyS0, (rm6t3_min_vr + RM6T3_VR_OFFSET) + ((rm6t3_max_vr - RM6T3_VR_OFFSET) - (rm6t3_min_vr + RM6T3_VR_OFFSET)) * val / 1000);
		frame[PT0_D_RQ_RP] = 1;
		crc = ModbusCrc16(frame, LEN + 8);
		frame[8 + LEN] = HI_UINT16(crc), frame[9 + LEN] = LO_UINT16(crc);
		write((int)lParam, (const void*)frame, LEN + 10);
		goto baitout;
	}

	// 揚角設定
	if (0X0093 == did && LEN == 4)
	{
		// AD 值 70~900
		val = BUILD_UINT32(frame[PT0_DATA_START + 3], frame[PT0_DATA_START + 2], frame[PT0_DATA_START + 1], frame[PT0_DATA_START]);
		if (val > 3000)			goto baitout;
		OnRm6IncineSet(ttyS0, val);
		frame[PT0_D_RQ_RP] = 1;
		crc = ModbusCrc16(frame, LEN + 8);
		frame[8 + LEN] = HI_UINT16(crc), frame[9 + LEN] = LO_UINT16(crc);
		write((int)lParam, (const void*)frame, LEN + 10);
		goto baitout;
	}

	// 讀取 SPU
	if (0X0094 == did && LEN == 4)
	{
		OnRm6SpuGet(ttyS0);
		goto baitout;
	}

	if (0X0099 == did && LEN == 4)
	{
		OnRm6FootFallSpeedGet(ttyS0);
		goto baitout;
	}
	if (0X009A == did && LEN == 4)
	{
		OnRm6TotalFootFallGet(ttyS0);
		goto baitout;
	}

	// 通訊停止時間設定
	if (0X0095 == did && LEN == 4)
	{
		val = BUILD_UINT32(frame[PT0_DATA_START + 3], frame[PT0_DATA_START + 2], frame[PT0_DATA_START + 1], frame[PT0_DATA_START]);
		if (val > 255)		goto baitout;
		//OnRm6WdtSet(ttyS0, val);
		Rm6ParamSet(ttyS0, RM6T3_PARAM_WTD, val);

		frame[PT0_D_RQ_RP] = 1;
		crc = ModbusCrc16(frame, LEN + 8);
		frame[8 + LEN] = HI_UINT16(crc), frame[9 + LEN] = LO_UINT16(crc);
		write((int)lParam, (const void*)frame, LEN + 10);
		goto baitout;
	}

	// PARAM GET
	if (0X009E == did && LEN == 4)
	{
		Rm6ParamGet(ttyS0, frame[PT0_DATA_START]);
		goto baitout;
	}

	// PARAM SET
	if (0X009F == did && LEN == 4)
	{
		val = BUILD_UINT32(frame[PT0_DATA_START + 3], frame[PT0_DATA_START + 2], frame[PT0_DATA_START + 1], 0);
		Rm6ParamSet(ttyS0, frame[PT0_DATA_START], val);

		frame[PT0_D_RQ_RP] = 1;
		crc = ModbusCrc16(frame, LEN + 8);
		frame[8 + LEN] = HI_UINT16(crc), frame[9 + LEN] = LO_UINT16(crc);
		write((int)lParam, (const void*)frame, LEN + 10);
		goto baitout;
	}

	// TROUBLE MESSAGE
	if (0X0097 == did && LEN == 4)
	{
		OnRm6TroubleMsgGet(ttyS0);
		goto baitout;
	}

	// 自動校正
	if (0X0098 == did && LEN == 4)
	{
		OnRm6AutoCalibrationSet(ttyS0);
		frame[PT0_D_RQ_RP] = 1;
		crc = ModbusCrc16(frame, LEN + 8);
		frame[8 + LEN] = HI_UINT16(crc), frame[9 + LEN] = LO_UINT16(crc);
		write((int)lParam, (const void*)frame, LEN + 10);
		goto baitout;
	}

	// 前滾輪直徑
	if (0X009C == did && LEN == 4)
	{
		val = BUILD_UINT32(frame[PT0_DATA_START + 3], frame[PT0_DATA_START + 2], frame[PT0_DATA_START + 1], frame[PT0_DATA_START]);
		if (0 == val)	goto baitout;
		rm6t3_wheel_diameter = (float)val / 1.00;
		rm6t3_freq_unit = RM6T3_CONST_RATIO * rm6t3_gear_ratio * 1000000.00F / (60.00F * 3.1416F * rm6t3_wheel_diameter) / 60.00F;
		frame[PT0_D_RQ_RP] = 1;
		crc = ModbusCrc16(frame, LEN + 8);
		frame[8 + LEN] = HI_UINT16(crc), frame[9 + LEN] = LO_UINT16(crc);
		write((int)lParam, (const void*)frame, LEN + 10);
	}

	// 齒輪比
	if (0X009D == did && LEN == 4)
	{
		val = BUILD_UINT32(frame[PT0_DATA_START + 3], frame[PT0_DATA_START + 2], frame[PT0_DATA_START + 1], frame[PT0_DATA_START]);
		if (0 == val)	goto baitout;
		rm6t3_gear_ratio = (float)val / 100.00F;
		rm6t3_freq_unit = RM6T3_CONST_RATIO * rm6t3_gear_ratio * 1000000.00F / (60.00F * 3.1416F * rm6t3_wheel_diameter) / 60.00F;
		frame[PT0_D_RQ_RP] = 1;
		crc = ModbusCrc16(frame, LEN + 8);
		frame[8 + LEN] = HI_UINT16(crc), frame[9 + LEN] = LO_UINT16(crc);
		write((int)lParam, (const void*)frame, LEN + 10);
	}


baitout:
	pthread_cleanup_pop(1);
	return 1;
}


//F2-FF-30-20-28-54-F4-F2-FF-36-20-88-57-F4-
//F2-FF-0A-20-88-46-F4-F1-7F-59-00-03-84-20-01-85-F4-
//F1-7F-5A-00-00-46-20-61-60-F4-
//F2-90-20-68-6C-F4-F2-A0-20-68-78-F4-
static void OnTtyS0(const char *path, int infd, unsigned char ch)
{
	static unsigned char frame[1024];
	static unsigned char *p = frame;
	static int split = 0;
	int len = 0;
	unsigned int crc;
	unsigned int chk;

	unsigned int data = 0;
	struct list_head *P, *Q;
	struct incoming_t *N = 0;
	int last_type = 0;
	unsigned char rm_status = 0;

	if (p == frame)
	{
		if (ch == RM6T3_CTRL_CODE_ANS || ch == RM6T3_CTRL_CODE_NAK || ch == RM6T3_CTRL_CODE_ACK)
		{
			*p++ = ch;
			split = 0;
		}
	}
	// 6,7,8,9,10
	else if (
		ch == RM6T3_CTRL_CODE_END && 
		(
			(5 == len && RM6T3_CTRL_CODE_ACK == frame[0] && 0XFF != frame[1]) ||
			(6 == len && RM6T3_CTRL_CODE_ACK == frame[0] && 0XFF == frame[1]) ||
			(6 == len && RM6T3_CTRL_CODE_NAK == frame[0] && 0XFF != frame[1]) ||
			(7 == len && RM6T3_CTRL_CODE_ACK == frame[0] && 0XFF == frame[1]) ||
			(7 == len && RM6T3_CTRL_CODE_ANS == frame[0] && 0X7F != frame[1]) ||
			(8 == len && RM6T3_CTRL_CODE_ANS == frame[0] && 0X7F != frame[1]) ||
			(9 == len && RM6T3_CTRL_CODE_ANS == frame[0] && 0X7F == frame[1])
		)
	)
	{
		*p++ = ch;
		len = (int)(p - frame);
		if (len <= 4)
		{
			memset(p = frame, 0, sizeof(frame));
			return;
		}
		crc = BUILD_UINT16(frame[len - 2], frame[len - 3]);
		chk = crc_chk(&frame[1], len - 4);
		if (chk == crc)
		{
			static unsigned char init = 0;
			if (0 == init)	init = 1, printf("RM6T3 PROTOCOL CHECK CRC(OK).\n");

// #8. No error
// ACK FFH PAR Status
			if (RM6T3_CTRL_CODE_ACK == frame[0] && 0XFF == frame[1])
			{
			}

// #6. No error
// ACK INS Status
			else if (RM6T3_CTRL_CODE_ACK == frame[0] && 0XFF != frame[1])
			{
				if ((RM6T3_INS_CODE_F_CMD | RM6T3_INS_CODE_WRITE_FLAG) == frame[1])
				{
				}
			}

// #9. Error
// NAK FFH PAR Error Status
			else if (RM6T3_CTRL_CODE_NAK == frame[0] && 0XFF == frame[1])
			{
				//static unsigned char init_9 = 0;
				//if (0 == init_9)	init_9 = 1, printf("NACK(9)\n");
			}

// #7. Error
// NAK INS Error Status
			else if (RM6T3_CTRL_CODE_NAK == frame[0] && 0XFF != frame[1])
			{
				//static unsigned char init_7 = 0;
				//if (0 == init_7)	init_7 = 1, printf("NACK(7)\n");
			}

// #12. Parameter read
// ANS 7FH PAR DH DM DL Status
			else if (RM6T3_CTRL_CODE_ANS == frame[0] && 0X7F == frame[1] && 10 == len)
			{
				data = BUILD_UINT32(frame[5], frame[4], frame[3], 0);
				OnRm6ParamRead(frame[2], data);
			}

// #11. Data read
// ANS INS Data Status
			else if (RM6T3_CTRL_CODE_ANS == frame[0] && 7 == len)
			{
				//static unsigned char init_11 = 0;
				//if (0 == init_11)	init_11 = 1, printf("DATA8 READ(11)\n");

				if (RM6T3_INS_CODE_TROUBLE_MSG == frame[1])
				{
					data = frame[2];
					rm_status = frame[3];

					pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &last_type);
					pthread_mutex_lock(&xF);
					pthread_cleanup_push(CleanupLock, (void *)&xF);
					pthread_setcanceltype(last_type, NULL);
					list_for_each_safe(P, Q, &cliQ.list)
					{
						N = list_entry(P, struct incoming_t, list);
						OnRm6TroubleMsgRsp(N->fd, data);
						OnRm6StatusRsp(N->fd, rm_status);
					}
					pthread_cleanup_pop(1);
				}
			}

// #10. Data read
// ANS INS DH DL Status
			else if (RM6T3_CTRL_CODE_ANS == frame[0] && 8 == len)
			{
				//static unsigned char init_10 = 0;
				//if (0 == init_10)	init_10 = 1, printf("DATA16 READ(10)\n");

				if (RM6T3_INS_CODE_READ_SPU == frame[1] || RM6T3_INS_CODE_FOOTFALL_SPEED == frame[1] || RM6T3_INS_CODE_TOTAL_FOOTFALL == frame[1])
				{
					data = (int)BUILD_UINT16(frame[3], frame[2]);
					rm_status = frame[4];

					pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &last_type);
					pthread_mutex_lock(&xF);
					pthread_cleanup_push(CleanupLock, (void *)&xF);
					pthread_setcanceltype(last_type, NULL);
					list_for_each_safe(P, Q, &cliQ.list)
					{
						N = list_entry(P, struct incoming_t, list);
						//if (RM6T3_INS_CODE_READ_SPU == frame[1])			rm6t3_spu = (float)data, OnRm6SpuRsp(N->fd, data);
						if (RM6T3_INS_CODE_READ_SPU == frame[1])			OnRm6SpuRsp(N->fd, data);
						if (RM6T3_INS_CODE_FOOTFALL_SPEED == frame[1])	OnRm6FootFallSpeedRsp(N->fd, data);
						if (RM6T3_INS_CODE_TOTAL_FOOTFALL == frame[1])	OnRm6TotalFootFallRsp(N->fd, data);
						OnRm6StatusRsp(N->fd, rm_status);
					}
					pthread_cleanup_pop(1);
				}
				if (RM6T3_INS_CODE_F_CMD == frame[1] || (RM6T3_INS_CODE_F_CMD | 0X80) == frame[1])
				{
				}
			}
			else //if (RM6T3_CTRL_CODE_ANS == frame[0] && 10 == len && 0X7F == frame[1])			// PARAM READ, 12
			{
			}
		}
		else
		{
			printf("RX CRC NG\n");
		}
		memset(p = frame, 0, sizeof(frame));
	}
	else if (split)
	{
		ch |= RM6T3_CTRL_CODE_SPB;
		*p++ = ch;
		split = 0;
	}
	else if (ch == RM6T3_CTRL_CODE_SPL)
	{
		split = 1;
	}
	else if (p - frame > 24)
	{
		memset(p = frame, 0, sizeof(frame));
	}
	else
	{
		*p++ = ch;
	}
}


LRESULT CRhythmApp::OnUartDataRcv(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
	int len;
	char path[128];
	unsigned char *buff = (unsigned char *)wParam + 64;

	if (0 == wParam)		return 1;
	memcpy(&len, (void *)wParam, sizeof(int));
	memset(path, 0, sizeof(path)), strcpy(path, ((char *)wParam) + sizeof(int));


//////////////////////////////////////////////////////////////////////
	pthread_cleanup_push(free, (void *)wParam);

	if (0 == strcmp(path, RM6T3_SERIAL_PORT))
	{
		for (int i = 0; i < len; i++)
			OnTtyS0(path, (int)lParam, buff[i]);
		goto baiout;
	}


//////////////////////////////////////////////////////////////////////
baiout:
	pthread_cleanup_pop(1);
	return 1;
}


LRESULT CRhythmApp::OnInitInstance(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
	int res = 0;

	CApp::OnInitInstance(dwType, wParam, lParam, pResult);
	if (!tUart)	res = pthread_create(&tUart, NULL, UartThread, this), assert(0 == res);
	if (!tUi)		res = pthread_create(&tUi, NULL, UiThread, this), assert(0 == res);

	SetTimer(RM6T3_POST_INIT_TIMER_ID, 997, 0);

//rm6t3_freq_unit=0.991750
//5*rm6t3_freq_unit=4.958748
//printf ("rm6t3_freq_unit=%f\n", rm6t3_freq_unit);
//printf ("5*rm6t3_freq_unit=%f\n", rm6t3_freq_unit*5.00F);
	return 1;
}

LRESULT CRhythmApp::OnExitInstance(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
	struct list_head *P, *Q;
	struct incoming_t *N = 0;

	if (tUart)
	{
		pthread_cancel(tUart);
		pthread_join(tUart, NULL);
		tUart = 0;
	}
	if (tUi)
	{
		pthread_cancel(tUi);
		pthread_join(tUi, NULL);
		tUi = 0;
	}

	CApp::OnExitInstance(dwType, wParam, lParam, pResult);
	list_for_each_safe(P, Q, &uartQ.list)
	{
		N = list_entry(P, struct incoming_t, list); assert(0 != N); close(N->fd); list_del(P); free(N);
	}
	return 1;
}

LRESULT CRhythmApp::OnTimer(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
	UINT TimerId = (UINT)wParam;

	if (RM6T3_POST_INIT_TIMER_ID == TimerId)
	{
		int res;

		res = AttachUart(RM6T3_SERIAL_PORT, RM6T3_BR_BPS);
		if (res)
		{
			ttyS0 = res;

// 通訊停止時間關閉
			Rm6ParamSet(ttyS0, RM6T3_PARAM_WTD, 255);
			//OnRm6WdtSet(ttyS0, 255);
			SetTimer(RM6T3_POST_INIT_TIMER_ID + 1, 23, 0);
		}
		return 1;
	}

// Incline AD 最小變化量設定
	if (RM6T3_POST_INIT_TIMER_ID + 1 == TimerId)
	{
		if (-1 != ttyS0)
		{
			Rm6ParamSet(ttyS0, RM6T3_PARAM_INC_DIFF, 1);
			//OnRm6InclineDifSet(ttyS0, 1);
			SetTimer(RM6T3_POST_INIT_TIMER_ID + 2, 23, 0);
		}
		return 1;
	}

// 最大輸出頻率設定
	if (RM6T3_POST_INIT_TIMER_ID + 2 == TimerId)
	{
		if (-1 != ttyS0)
		{
			Rm6ParamSet(ttyS0, RM6T3_PARAM_MAX_FREQ, 4000);
			//OnRm6MaxFreqSet(ttyS0, 4000);
			SetTimer(RM6T3_POST_INIT_TIMER_ID + 3, 23, 0);
		}
		return 1;
	}

// 揚角馬達介面選擇, 3: VR 偵測角度range 手動設定控制; Incline（Pin8）動作時，AD 值變大； 反之，當Decline（Pin7）動作時，AD 值變小
	if (RM6T3_POST_INIT_TIMER_ID + 3 == TimerId)
	{
		if (-1 != ttyS0)
		{
			Rm6ParamSet(ttyS0, RM6T3_PARAM_INC_SELECT, 3);
			SetTimer(RM6T3_POST_INIT_TIMER_ID + 4, 23, 0);
		}
		return 1;
	}

// 讀取 VR 最大值
	if (RM6T3_POST_INIT_TIMER_ID + 4 == TimerId)
	{
		if (-1 != ttyS0)
		{
			//Rm6ParamGet(ttyS0, RM6T3_PARAM_VR_MAX);
			Rm6ParamSet(ttyS0, RM6T3_PARAM_VR_MAX, (unsigned int)rm6t3_max_vr);
			SetTimer(RM6T3_POST_INIT_TIMER_ID + 5, 23, 0);
		}
		return 1;
	}

// 讀取 VR 最小值
	if (RM6T3_POST_INIT_TIMER_ID + 5 == TimerId)
	{
		if (-1 != ttyS0)
		{
			//Rm6ParamGet(ttyS0, RM6T3_PARAM_VR_MIN);
			Rm6ParamSet(ttyS0, RM6T3_PARAM_VR_MIN, (unsigned int)rm6t3_min_vr);
			SetTimer(RM6T3_POST_INIT_TIMER_ID + 6, 23, 0);
		}
		return 1;
	}

//輸出頻率
	if (RM6T3_POST_INIT_TIMER_ID + 6 == TimerId)
	{
		if (rm6t3_max_vr <= 100 || rm6t3_max_vr <= rm6t3_min_vr)
		{
			rm6t3_max_vr = 800, rm6t3_min_vr = 70;
			printf("VR_MAX/VR_MIN(%d %d) ERROR, LET'S ASSUME VR_MAX=800 AND VR_MIN=70\n", rm6t3_max_vr, rm6t3_min_vr);
		}
		if (-1 != ttyS0)
		{
			OnRm6FreqSet(ttyS0, (unsigned int)rm6t3_freq);
			if (0 == (unsigned int)rm6t3_freq)	SetTimer(RM6T3_RRFR_TIMER_ID, 23, 0);
			else								SetTimer(RM6T3_FR_TIMER_ID, 23, 0);
		}
		return 1;
	}

// 正轉, FORWARD RUN
	if (RM6T3_FR_TIMER_ID == TimerId)
	{
		if (-1 != ttyS0)
		{
			OnRm6ForwardRunSet(ttyS0, RM6T3_OPT_CMD_FR);
			//printf("RM6T3_OPT_CMD_FR\n");
		}
		return 1;
	}

// 停止, STOP
	if (RM6T3_RRFR_TIMER_ID == TimerId)
	{
		if (-1 != ttyS0)
		{
			OnRm6ForwardRunSet(ttyS0, RM6T3_OPT_CMD_STOP);
			//printf("RM6T3_OPT_CMD_STOP\n");
		}
		return 1;
	}

	return 1;
}


