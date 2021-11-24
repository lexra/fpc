// App.cpp : Defines the class behaviors for the application.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"

#include <stdlib.h>
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
#include <netinet/in.h>
#include <inttypes.h>
#include <stddef.h>
#include <assert.h>
#include <sys/socket.h>

#include "list.h"
#include "Event.h"
#include "Misc.h"
#include "App.h"



#ifdef _DEBUG
#ifdef WIN32
#define new DEBUG_NEW
#endif
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



//////////////////////////////////////////////////////////////////////
// define
//////////////////////////////////////////////////////////////////////

#define BEGIN_BASE_MESSAGE_MAP(theClass) \
const AFX_MSGMAP *theClass::GetMessageMap() const \
	{ return &theClass::messageMap; } \
const AFX_MSGMAP theClass::messageMap = \
	{0, &theClass::_messageEntries[0]}; \
const AFX_MSGMAP_ENTRY theClass::_messageEntries[] = \
{

#define END_BASE_MESSAGE_MAP() \
	{0, 0} \
};


#define SOCKET_BUFFER_SIZE											(512 * 1024)
#define SHELL_LISTEN_PORT											9100



//////////////////////////////////////////////////////////////////////
// static routine
//////////////////////////////////////////////////////////////////////

unsigned long long int _debug = DBG_DEFAULT;


static CApp *pApp = 0;

static pthread_mutex_t xM = PTHREAD_MUTEX_INITIALIZER;
static struct MSG mQ;

static pthread_mutex_t xT = PTHREAD_MUTEX_INITIALIZER;
static struct TIMER tQ;


static Timer_t to[64] = 
{
	{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, 
	{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, 
	{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, 
	{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, 
	{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, 
	{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, 
	{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, 
	{0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, 
};


void *SigwaitThread(void *param)
{
	sigset_t nset;
	siginfo_t info;
	struct timespec ts;
	int res;
	int last_state;
	int last_type;

	ts.tv_sec = 1, ts.tv_nsec = 1000000;
	sigemptyset(&nset);
	sigaddset(&nset, SIGHUP), sigaddset(&nset, SIGINT), sigaddset(&nset, SIGTERM), sigaddset(&nset, SIGUSR1), sigaddset(&nset, SIGALRM);
	sigaddset(&nset, SIGKILL);

	for(;;)
	{
		if (-1 == (res = sigtimedwait(&nset, &info, &ts)))
		{
			if (errno == EAGAIN)										{ dbg_printf(DBG_SIGNAL, "(%s %d) SIGTIMEDWAIT() RETURN, EAGAIN \n", __FILE__, __LINE__); continue; }
			dbg_printf(DBG_SIGNAL, "(%s %d) SIGTIMEDWAIT() fail, errno=%d, EXIT() \n", __FILE__, __LINE__, errno); exit(1);
			break;
		}

		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &last_state);
		pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &last_type);



		if(SIGINT == res || SIGTERM == res || SIGHUP == res || SIGKILL)	{ dbg_printf(DBG_SIGNAL, "(%s %d) SIGTERM..., \n", __FILE__, __LINE__); AfxGetApp()->PostMessage(M_EXIT_INSTANCE, 0, 0); break; }
		if(SIGALRM == res)											{ dbg_printf(DBG_SIGNAL, "(%s %d) SIGALRM \n", __FILE__, __LINE__); AfxGetApp()->PostMessage(M_SIG_ALRM, 0, 0); continue; }
		//if(SIGUSR1 == res)											printf("counter=%d\n", *(UINT *)info.si_ptr); free(info.si_ptr);


		pthread_testcancel();
		pthread_setcanceltype(last_type, NULL);
		pthread_setcancelstate(last_state, NULL);
	}

	dbg_printf(DBG_THREAD, "(%s %d) SigwaitThread() EXIT\n", __FILE__, __LINE__);
	return 0;
}


void *ShellThread(void *param)
{
	int maxfd = 0;
	int lsd = -1;
	int v;
	struct sockaddr_in servaddr;
	fd_set rset;
	int res;
	int len;
	sigset_t nset, oset;

	int sd = -1;
	unsigned char buff[4096];
	unsigned char *p;

	if (-1 == (lsd = socket(AF_INET, SOCK_STREAM, 0)))
	{
		printf("(%s %d) SOCKET() FAIL\n", __FILE__, __LINE__); AfxGetApp()->PostMessage(M_EXIT_INSTANCE, 0, 0);
		return 0;
	}
	v = 1, setsockopt(lsd, SOL_SOCKET, SO_REUSEADDR, (const void *)&v, sizeof(v));
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET, servaddr.sin_addr.s_addr = htonl(INADDR_ANY); servaddr.sin_port = htons(SHELL_LISTEN_PORT);
	if (0 > bind(lsd, (struct sockaddr *)&servaddr, sizeof(struct sockaddr_in)))
	{
		printf("(%s %d) BIND() fail\n", __FILE__, __LINE__); AfxGetApp()->PostMessage(M_EXIT_INSTANCE, 0, 0);
		return 0;
	}
	if (0 > listen(lsd, 12))
	{
		printf("(%s %d) LISTEN() fail\n", __FILE__, __LINE__); AfxGetApp()->PostMessage(M_EXIT_INSTANCE, 0, 0);
		return 0;
	}
	v = fcntl(lsd, F_GETFL, 0); fcntl(lsd, F_SETFL, v | O_NONBLOCK);
	v = SOCKET_BUFFER_SIZE, setsockopt(lsd, SOL_SOCKET, SO_SNDBUF, (const void *)&v, sizeof(v)); setsockopt(lsd, SOL_SOCKET, SO_RCVBUF, (const void *)&v, sizeof(v));
	pthread_cleanup_push(CleanupFd, (void *)&lsd);

	sigemptyset(&nset);
	sigaddset(&nset, SIGHUP), sigaddset(&nset, SIGINT), sigaddset(&nset, SIGTERM), sigaddset(&nset, SIGUSR1), sigaddset(&nset, SIGALRM);
	pthread_sigmask(SIG_BLOCK, &nset, &oset);
	pthread_cleanup_push(RestoreSigmask, (void *)&oset);

#if 0
	v = fcntl(STDIN_FILENO, F_GETFL, 0); fcntl(STDIN_FILENO, F_SETFL, v | O_NONBLOCK);
	fcntl (STDOUT_FILENO, F_SETFL, fcntl(STDOUT_FILENO, F_GETFL, 0) | O_NONBLOCK);
#endif


	for (;;)
	{
		int clilen, connfd;
		struct sockaddr_in cliaddr;

		maxfd = 0; FD_ZERO(&rset);

#if 0
		FD_SET(STDIN_FILENO, &rset);
		if (maxfd < STDIN_FILENO)										maxfd = STDIN_FILENO;
#endif


		FD_SET(lsd, &rset);
		if (maxfd < lsd)												maxfd = lsd;

		if (-1 != sd)
		{
			FD_SET(sd, &rset);
			if (maxfd < sd)											maxfd = sd;
		}

		if (0 > (res = select(maxfd + 1, &rset, 0, 0, 0)))
		{
			dbg_printf(DBG_SOCKET, "(%s %d) SELECT() FAIL\n", __FILE__, __LINE__); AfxGetApp()->PostMessage(M_EXIT_INSTANCE, 0, 0); break;
		}
		if (0 == res)													{ dbg_printf(DBG_SOCKET, "(%s %d) SELECT() TIMEOUT\n", __FILE__, __LINE__); continue; }


		if (FD_ISSET(STDIN_FILENO, &rset))
		{
			memset(buff, 0, sizeof(buff));
			if (0 > (len = read(STDIN_FILENO, buff, sizeof(buff))))
			{
				if (errno == EAGAIN)									{ dbg_printf(DBG_SOCKET, "(%s %d) EAGAIN\n", __FILE__, __LINE__); continue; }
				if (errno == EWOULDBLOCK)							{ dbg_printf(DBG_SOCKET, "(%s %d) EWOULDBLOCK\n", __FILE__, __LINE__); continue; }
				dbg_printf(DBG_SOCKET, "(%s %d) READ() FAIL, errno=%d\n", __FILE__, __LINE__, errno); AfxGetApp()->PostMessage(M_EXIT_INSTANCE, 0, 0); close(sd); sd = -1; break;
			}
			if (0 == len)												{ dbg_printf(DBG_SOCKET, "(%s %d) READ() EOF\n", __FILE__, __LINE__); close(sd); sd = -1; continue; }
			if (0X0A != buff[len -1])									continue;
			StripCrLf((char *)buff);
			v = len + 64;
			p = (unsigned char *)malloc(len + 64);
			if (p)
			{
				memset(p, 0, len + 64);
				memcpy(p, &v, sizeof(int));
				memcpy(p + 64, buff, len);
				AfxGetApp()->PostMessage(M_SHELL, (WPARAM)p, (LPARAM)STDIN_FILENO);
			}
			else { dbg_printf(DBG_MALLOC, "(%s %d) MALLOC() FAIL\n", __FILE__, __LINE__); }
		}


		if (FD_ISSET(lsd, &rset))
		{
			clilen = sizeof(cliaddr);
			connfd = accept(lsd, (struct sockaddr *)&cliaddr, (socklen_t *)&clilen);
			if (-1 != connfd)
			{
				v = fcntl(connfd, F_GETFL, 0); fcntl(connfd, F_SETFL, v | O_NONBLOCK);
				v = SOCKET_BUFFER_SIZE, setsockopt(connfd, SOL_SOCKET, SO_SNDBUF, (const void *)&v, sizeof(v)); setsockopt(connfd, SOL_SOCKET, SO_RCVBUF, (const void *)&v, sizeof(v));
				close(sd); sd = connfd;
				continue;
			}
			else { dbg_printf(DBG_SOCKET, "(%s %d) ACCEPT() FAIL\n", __FILE__, __LINE__); }
		}

		if (-1 == sd)													{ continue; }
		if (!FD_ISSET(sd, &rset))										{ continue; }

		memset(buff, 0, sizeof(buff));
		if (0 > (len = read(sd, buff, sizeof(buff))))
		{
			if (errno == EAGAIN)										{ dbg_printf(DBG_SOCKET, "(%s %d) EAGAIN\n", __FILE__, __LINE__); continue;}
			if (errno == EWOULDBLOCK)								{ dbg_printf(DBG_SOCKET, "(%s %d) EWOULDBLOCK\n", __FILE__, __LINE__); continue;}
			if (errno == ECONNRESET || errno == EBADF)					{ dbg_printf(DBG_SOCKET, "(%s %d) ECONNRESET\n", __FILE__, __LINE__); close(sd); sd = -1; continue; }
			printf("(%s %d) READ() FAIL\n", __FILE__, __LINE__); AfxGetApp()->PostMessage(M_EXIT_INSTANCE, 0, 0); close(sd); sd = -1; break;
		}
		if (0 == len)													{ dbg_printf(DBG_SOCKET, "(%s %d) READ() EOF\n", __FILE__, __LINE__); close(sd); sd = -1; continue; }
		if (0X0A != buff[len -1])										continue;
		StripCrLf((char *)buff);
		v = len + 64;
		p = (unsigned char *)malloc(len + 64);
		if (p)
		{
			memset(p, 0, len + 64);
			memcpy(p, &v, sizeof(int));
			memcpy(p + 64, buff, len);
			AfxGetApp()->PostMessage(M_SHELL, (WPARAM)p, (LPARAM)sd);
		}
		else { dbg_printf(DBG_SOCKET, "(%s %d) ACCEPT() FAIL\n", __FILE__, __LINE__); }
	} // for(;;)

	pthread_cleanup_pop(1);
	pthread_cleanup_pop(1);

dbg_printf(DBG_THREAD, "(%s %d) ShellThread() EXIT\n", __FILE__, __LINE__);
	return 0;
}

static void PollTimer(void)
{
	int i;
	struct timeb tb;
	unsigned int now;
	UINT id = 0;
	TIMER_FUNC cb;

	for (i = 0; i < (int)(sizeof(to) / sizeof(Timer_t)); i++)
	{
		if(to[i].enable == 0)												{ continue; }
		if(to[i].timeout != 0)
		{
			ftime(&tb);
			now = tb.time * 1000 + tb.millitm;
			if(now >= to[i].timeout)
			{
				id = to[i].id; cb = to[i].cb;

				to[i].timeout = 0;
				to[i].id = 0;
				to[i].cb = 0;
				to[i].enable = 0;
				AfxGetApp()->PostMessage(M_TIMER, (WPARAM)id, (LPARAM)cb);
			}
		}
	}
}

static void CleanupMsg(void *param)
{
	struct list_head *p, *q;
	MSG *m;
	int last_type;

	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &last_type);
	pthread_mutex_lock(&xM);
	pthread_cleanup_push(CleanupLock, (void *)&xM);
	pthread_setcanceltype(last_type, NULL);
	list_for_each_safe(p, q, &mQ.list)
	{
		m = list_entry(p, MSG, list); assert(0 != m); list_del(p); free(m);
	}
	pthread_cleanup_pop(1);
}

static void CleanupTimer(void *param)
{
	struct list_head *P, *Q;
	TIMER *N;
	int last_type;

	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &last_type);
	pthread_mutex_lock(&xT);
	pthread_cleanup_push(CleanupLock, (void *)&xT);
	pthread_setcanceltype(last_type, NULL);
	list_for_each_safe(P, Q, &tQ.list)
	{
		N = list_entry(P, TIMER, list); assert(0 != N); list_del(P); free(N);
	}
	pthread_cleanup_pop(1);
}

void *TimerThread(void *param)
{
	sigset_t nset, oset;

#if 0
	//unsigned int res = 0;
	//CApp *app = (CApp *)param;
	//HANDLE hQuit = app->hQuit;

	//pthread_detach(pthread_self());
#endif


///////////////////////
	sigemptyset(&nset);
	sigaddset(&nset, SIGHUP), sigaddset(&nset, SIGINT), sigaddset(&nset, SIGTERM), sigaddset(&nset, SIGUSR1), sigaddset(&nset, SIGALRM);
	pthread_sigmask(SIG_BLOCK, &nset, &oset);
	pthread_cleanup_push(RestoreSigmask, (void *)&oset);


///////////////////////
	INIT_LIST_HEAD(&tQ.list);
	pthread_cleanup_push(CleanupTimer, 0);
	for(;;)
	{
		PollTimer();
		usleep(5000);
	}
///////////////////////
	pthread_cleanup_pop(0);


///////////////////////
	pthread_cleanup_pop(0);

dbg_printf(DBG_THREAD, "(%s %d) TimerThread EXIT\n", __FILE__, __LINE__);
	//pthread_exit(&res);
	return 0;
}


//////////////////////////////////////////////////////////////////////
// extern routine
//////////////////////////////////////////////////////////////////////

CApp *AfxGetApp()
{
	return pApp;
}


//////////////////////////////////////////////////////////////////////
// CApp
//////////////////////////////////////////////////////////////////////

CApp::CApp()
{
	hQuit = 0;
	hMsg = 0;

	tMq = 0;
	tTm = 0;
	tSw = 0;
	tShell = 0;

	INIT_LIST_HEAD(&mQ.list);
	if (!pApp)		pApp = this;

	hMsg = CreateEvent(0, AUTO_RESET, 0, "MSG");
	hQuit = CreateEvent(0, MANUAL_RESET, 0, "QUIT");

	PostMessage(M_INIT_INSTANCE, 0, 0);
}

CApp::~CApp()
{
	sleep(1);
	if (hQuit)															CloseEvent(hQuit);
	if (hMsg)															CloseEvent(hMsg);

	dbg_printf(DBG_BASIC, "(%s %d) PROGRAM EXIT GRACEFULLY \n", __FILE__, __LINE__);
}

MSG *CApp::GetMsgQueue()
{
	return (&mQ);
}

#if 1
int CApp::Run()
{
	struct list_head *P, *Q;
	MSG *N;
	unsigned int res;
	HANDLE h[] = {0, 0, 0};
	const AFX_MSGMAP *pMap;
	DWORD nMessage;
	AFX_PMSG pFnMsg;
	const AFX_MSGMAP_ENTRY *pEntries;
	sigset_t nset, oset;
	int last_type;
	int quit = 0;

	sigemptyset(&nset);
	sigaddset(&nset, SIGHUP), sigaddset(&nset, SIGINT), sigaddset(&nset, SIGTERM), sigaddset(&nset, SIGUSR1), sigaddset(&nset, SIGALRM);
	pthread_sigmask(SIG_BLOCK, &nset, &oset);
	pthread_cleanup_push(RestoreSigmask, (void *)&oset);

	if (!hQuit)	return 0;
	if (!hMsg)	return 0;
	if(0 != pthread_create(&tTm, NULL, TimerThread, this))					{ dbg_printf(DBG_THREAD, "TimerThread() NG\n"); return 0;}
	dbg_printf(DBG_THREAD, "(%s %d) TimerThread() OK\n", __FILE__, __LINE__);

	if (0 != pthread_create(&tSw, NULL, SigwaitThread, this))				{ dbg_printf(DBG_THREAD, "SigwaitThread() NG\n"); return 0;}
	dbg_printf(DBG_THREAD, "(%s %d) SigwaitThread() OK\n", __FILE__, __LINE__);

	tMq = pthread_self();

	//pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &last_state);
	//pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &last_type);

////////////
	pthread_cleanup_push(CleanupMsg, 0);

	h[0] = hQuit, h[1] = hMsg;
	for(;;)
	{
		res = WaitForMultipleObjects(2, h, WAIT_FOR_ANY_ONE_EVENTS, INFINITE);
		if (WAIT_FAILED == res) { usleep(10); dbg_printf(DBG_BASIC, "(%s %d) WaitForMultipleObjects() >  WAIT_FAILED \n", __FILE__, __LINE__); continue; }
		if (WAIT_OBJECT_0 + 0 == res) { dbg_printf(DBG_EVENT, "(%s %d) WaitForMultipleObjects() >  WAIT_OBJECT_0 \n", __FILE__, __LINE__); break; }
		if (WAIT_OBJECT_0 + 1 == res)
		{
			int x = 0;
			AFX_PMSG fArray[128];
			MSG * mArray[128];

			memset(fArray, 0, sizeof(fArray));
			memset(mArray, 0, sizeof(mArray));

			dbg_printf(DBG_EVENT, "(%s %d) WaitForMultipleObjects() >  WAIT_OBJECT_1 \n", __FILE__, __LINE__);

			pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &last_type);
			pthread_mutex_lock(&xM);
			pthread_cleanup_push(CleanupLock, (void *)&xM);
			pthread_setcanceltype(last_type, NULL);
			list_for_each_safe(P, Q, &mQ.list)
			{
				N= list_entry(P, MSG, list); assert(0 != N);
				pMap = GetMessageMap();
				while (pMap)
				{
					pEntries = pMap->lpEntries;
					nMessage = pEntries->nMessage;
					pFnMsg = pEntries->pfn;
					while(nMessage != 0)
					{
						if (N->type == nMessage)
						{
///////////////////////////////////////////////////////////
							if (0 == N->result && 0 != pFnMsg)
							{
								for(x = 0; x < 128; x++)
								{
									if (0 != fArray[x])					continue;
									fArray[x] = pFnMsg;
									mArray[x] = N;
///////////////////////////////////////////////////////////
									N->result = 1;
									break;
								}
							}
							if (N->type == M_EXIT_INSTANCE)			{ quit = 1; dbg_printf(DBG_MSG, "(%s %d) M_EXIT_INSTANCE\n", __FILE__, __LINE__);  }
						}
						pEntries++;
						nMessage = pEntries->nMessage;
						pFnMsg = pEntries->pfn;
					}
					pMap = pMap->pBaseMap;
				}
				list_del(P); //delete N;
			}
			pthread_cleanup_pop(1);

			for(x = 0; x < 128; x++)
			{
				if (0 == fArray[x])										{ continue; }
				(this->*fArray[x])(mArray[x]->type, mArray[x]->wparam, mArray[x]->lparam, &mArray[x]->result);
				delete mArray[x];
			}
			if (quit)													{ SetEvent(hQuit); }
		}
	}

////////////
	pthread_cleanup_pop(1);
	pthread_cleanup_pop(1);


////////////
	//pthread_setcanceltype(last_type, NULL);
	//pthread_setcancelstate(last_state, NULL);

	return 0;
}
#else
int CApp::Run()
{
	struct list_head *P, *Q;
	MSG *N;
	unsigned int res;
	HANDLE h[] = {0, 0, 0};
	const AFX_MSGMAP *pMap;
	DWORD nMessage;
	AFX_PMSG pFnMsg;
	const AFX_MSGMAP_ENTRY *pEntries;
	sigset_t nset, oset;
	int last_type;
	int quit = 0;

	sigemptyset(&nset);
	sigaddset(&nset, SIGHUP), sigaddset(&nset, SIGINT), sigaddset(&nset, SIGTERM), sigaddset(&nset, SIGUSR1), sigaddset(&nset, SIGALRM);
	pthread_sigmask(SIG_BLOCK, &nset, &oset);
	pthread_cleanup_push(RestoreSigmask, (void *)&oset);

	if (!hQuit)	return 0;
	if (!hMsg)	return 0;
	if(0 != pthread_create(&tTm, NULL, TimerThread, this))					{ dbg_printf(DBG_THREAD, "TimerThread() NG\n"); return 0;}
	dbg_printf(DBG_THREAD, "(%s %d) TimerThread() OK\n", __FILE__, __LINE__);

	if (0 != pthread_create(&tSw, NULL, SigwaitThread, this))				{ dbg_printf(DBG_THREAD, "SigwaitThread() NG\n"); return 0;}
	dbg_printf(DBG_THREAD, "(%s %d) SigwaitThread() OK\n", __FILE__, __LINE__);

	if (0 != pthread_create(&tShell, NULL, ShellThread, this))				{ dbg_printf(DBG_THREAD, "ShellThread() NG\n"); return 0;}
	dbg_printf(DBG_THREAD, "(%s %d) ShellThread() OK\n", __FILE__, __LINE__);

	tMq = pthread_self();

	//pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &last_state);
	//pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &last_type);

////////////
	pthread_cleanup_push(CleanupMsg, 0);

	h[0] = hQuit, h[1] = hMsg;
	for(;;)
	{
		res = WaitForMultipleObjects(2, h, WAIT_FOR_ANY_ONE_EVENTS, INFINITE);
		if (WAIT_FAILED == res) { usleep(10); dbg_printf(DBG_BASIC, "(%s %d) WaitForMultipleObjects() >  WAIT_FAILED \n", __FILE__, __LINE__); continue; }
		if (WAIT_OBJECT_0 + 0 == res) { dbg_printf(DBG_EVENT, "(%s %d) WaitForMultipleObjects() >  WAIT_OBJECT_0 \n", __FILE__, __LINE__); break; }
		if (WAIT_OBJECT_0 + 1 == res)
		{
			dbg_printf(DBG_EVENT, "(%s %d) WaitForMultipleObjects() >  WAIT_OBJECT_1 \n", __FILE__, __LINE__);

			pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &last_type);
			pthread_mutex_lock(&xM);
			pthread_cleanup_push(CleanupLock, (void *)&xM);
			pthread_setcanceltype(last_type, NULL);
			list_for_each_safe(P, Q, &mQ.list)
			{
				N= list_entry(P, MSG, list); assert(0 != N);
				pMap = GetMessageMap();
				while (pMap)
				{
					pEntries = pMap->lpEntries;
					nMessage = pEntries->nMessage;
					pFnMsg = pEntries->pfn;
					while(nMessage != 0)
					{
						if (N->type == nMessage)
						{
							if (0 == N->result && 0 != pFnMsg)
							{
								(this->*pFnMsg)(N->type, N->wparam, N->lparam, &N->result);
							}
							if (N->type == M_EXIT_INSTANCE)			{ quit = 1; dbg_printf(DBG_MSG, "(%s %d) M_EXIT_INSTANCE\n", __FILE__, __LINE__);  }
						}
						pEntries++;
						nMessage = pEntries->nMessage;
						pFnMsg = pEntries->pfn;
					}
					pMap = pMap->pBaseMap;
				}
				list_del(P); delete N;
			}
			pthread_cleanup_pop(1);

			if (quit)													{ SetEvent(hQuit); }
		}
		else { dbg_printf(DBG_BASIC, "(%s %d) WaitForMultipleObjects() >  WAIT_OBJECT_2+ \n", __FILE__, __LINE__); usleep(10); }
	}

////////////
	pthread_cleanup_pop(1);
	pthread_cleanup_pop(1);


////////////
	//pthread_setcanceltype(last_type, NULL);
	//pthread_setcancelstate(last_state, NULL);

	return 0;
}
#endif

BOOL CApp::PostMessage(struct MSG *pmsg)
{
	struct MSG *msg;
	int last_type;

	if (!hQuit || !hMsg || !pmsg)	return 0;
	if (!(msg = new MSG()))		return 0;
	memcpy(msg, pmsg, sizeof(struct MSG));

	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &last_type);
	pthread_mutex_lock(&xM);
	pthread_cleanup_push(CleanupLock, (void *)&xM);
	pthread_setcanceltype(last_type, NULL);
	list_add_tail(&msg->list, &mQ.list);
	pthread_cleanup_pop(1);

	return SetEvent(hMsg);
}

BOOL CApp::PostMessage(DWORD type, WPARAM wparam, LPARAM lparam)
{
	struct MSG *msg;
	int last_type;

	if (!hQuit || !hMsg)			return 0;
	if (!(msg = new MSG()))		return 0;
	msg->type = type, msg->wparam = (WPARAM)wparam, msg->lparam = (LPARAM)lparam;

	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &last_type);
	pthread_mutex_lock(&xM);
	pthread_cleanup_push(CleanupLock, (void *)&xM);
	pthread_setcanceltype(last_type, NULL);
	list_add_tail(&msg->list, &mQ.list);
	pthread_cleanup_pop(1);

	return SetEvent(hMsg);
}

UINT CApp::SetTimer(UINT nId, UINT nElapse, TIMER_FUNC cb)
{
	int f = 0, i = 0;
	struct timeb tb;
	unsigned int et;

	if (0 == nId)							return 0;
	ftime(&tb);
	et = tb.time * 1000 + tb.millitm + nElapse;
	for (i = 0; i < 64; i++)
	{
		if (to[i].id == nId)
		{
			f = 1;
			break;
		}
	}
	if (0 == f)
	{
		for (i = 0; i < 64; i++)
		{
			if (to[i].id == 0)				break;
		}
	}
	if (i >= 64)							return 0;

	to[i].id = nId;
	to[i].cb = cb;
	to[i].enable = 1;
	to[i].timeout = et;

	return (i + 1);
}

BOOL CApp::KillTimer(UINT nId)
{
	int i = 0;

	if (0 == nId)							return 0;
	for (i = 0; i < 64; i++)
	{
		if (to[i].id == nId)
		{
			break;
		}
	}
	if (i == 64)							return 0;
	to[i].id = 0;
	to[i].cb = 0;
	to[i].enable = 0;
	to[i].timeout = 0;

	return i;
}


//////////////////////////////////////////////////////////////////////
// message map & signal map
//////////////////////////////////////////////////////////////////////

BEGIN_BASE_MESSAGE_MAP(CApp)
	//{{AFX_MSG_MAP(CApp)
	ON_MESSAGE(M_INIT_INSTANCE, &CApp::OnInitInstance)
	ON_MESSAGE(M_EXIT_INSTANCE, &CApp::OnExitInstance)
	ON_MESSAGE(M_TIMER, &CApp::OnTimer)
	ON_MESSAGE(M_SHELL, &CApp::OnShell)
	//}}AFX_MSG_MAP
END_BASE_MESSAGE_MAP()


//////////////////////////////////////////////////////////////////////
// message handlers
//////////////////////////////////////////////////////////////////////

LRESULT CApp::OnInitInstance(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
	//dbg_printf(DBG_BASIC, "CApp::OnInitInstance()\n");
	return (*pResult = 1);
}

LRESULT CApp::OnExitInstance(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
	dbg_printf(DBG_BASIC, "CApp::OnExitInstance()\n");

	if (tShell)															{ pthread_cancel(tShell); dbg_printf(DBG_THREAD, "(%s %d) TRYING WAIT() tShell...,   ", __FILE__, __LINE__); pthread_join(tShell, NULL); dbg_printf(DBG_THREAD,"(%s %d) WAIT() tShell OK\n", __FILE__, __LINE__); }
	if (tSw)															{ pthread_cancel(tSw); dbg_printf(DBG_THREAD, "(%s %d) TRYING WAIT() tSw...,   ", __FILE__, __LINE__); pthread_join(tSw, NULL); dbg_printf(DBG_THREAD, "(%s %d) WAIT() tSw OK\n", __FILE__, __LINE__); }
	if (tTm)															{ pthread_cancel(tTm); dbg_printf(DBG_THREAD, "(%s %d) TRYING WAIT() tTm...,   ", __FILE__, __LINE__); pthread_join(tTm, NULL); dbg_printf(DBG_THREAD, "(%s %d) WAIT() tTm OK\n", __FILE__, __LINE__); }

	return (*pResult = 1);
}

LRESULT CApp::OnTimer(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
	return (*pResult = 1);
}

LRESULT CApp::OnShell(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
	if (0 == wParam)
		return (*pResult = 1);

	pthread_cleanup_push(free, (void *)wParam);



	pthread_cleanup_pop(1);
	return (*pResult = 1);
}

