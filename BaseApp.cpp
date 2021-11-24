// BaseApp.cpp : Defines the class behaviors for the application.
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
#include <sys/resource.h>


#include "list.h"
#include "Event.h"
#include "Misc.h"
#include "BaseApp.h"



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



//////////////////////////////////////////////////////////////////////
// static routine
//////////////////////////////////////////////////////////////////////

unsigned long long int _debug = DBG_DEFAULT;
//unsigned long long int _debug = DBG_ALL;


static CApp *pApp = 0;

static pthread_mutex_t xM = PTHREAD_MUTEX_INITIALIZER;
static struct MSG mQ;

static pthread_mutex_t xT = PTHREAD_MUTEX_INITIALIZER;
static struct TIMER tQ;


static Timer_t to[MAX_TIMER];


void *SigwaitThread(void *param)
{
	sigset_t nset;
	siginfo_t info;
	struct timespec ts;
	int res;
	int last_state;
	int last_type;

	ts.tv_sec = 12, ts.tv_nsec = 1000000;
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


		//if(SIGINT == res || SIGTERM == res || SIGHUP == res || SIGKILL)	{ dbg_printf(DBG_SIGNAL, "(%s %d) SIGTERM..., \n", __FILE__, __LINE__); exit(1); AfxGetApp()->PostMessage(M_EXIT_INSTANCE, 0, 0); break; }
		if(SIGINT == res || SIGTERM == res || SIGHUP == res || SIGKILL)	{ dbg_printf(DBG_SIGNAL, "(%s %d) SIGTERM..., \n", __FILE__, __LINE__); AfxGetApp()->PostMessage(M_EXIT_INSTANCE, 0, 0); continue; }
		if(SIGALRM == res)											{ dbg_printf(DBG_SIGNAL, "(%s %d) SIGALRM \n", __FILE__, __LINE__); AfxGetApp()->PostMessage(M_SIG_ALRM, 0, 0); continue; }
		//if(SIGUSR1 == res)											printf("counter=%d\n", *(UINT *)info.si_ptr); free(info.si_ptr);


		pthread_testcancel();
		pthread_setcanceltype(last_type, NULL);
		pthread_setcancelstate(last_state, NULL);
	}

	dbg_printf(DBG_THREAD, "(%s %d) SigwaitThread() EXIT\n", __FILE__, __LINE__);
	return 0;
}

static void PollTimer(void)
{
	static int task = 0;
	int j;
	int i;
	unsigned int now;
	UINT id = 0;
	TIMER_FUNC cb;
	struct timespec request;
	int N = sizeof(to) / sizeof(Timer_t);

	for (i = 0; i < N; i++)
	{
		j = (task + i) % N;
		if(to[j].enable == 0 || to[j].timeout == 0)
			continue;
		memset(&request, 0, sizeof(request));
		if (0 != clock_gettime(CLOCK_REALTIME, &request))
		{
			printf("(%s %d) clock_gettime() fail\n", __FILE__, __LINE__);
			continue;
		}
		now = request.tv_sec * 1000 + request.tv_nsec / 1000000;
		if(now >=  to[j].timeout)
		{
			id = to[j].id; cb = to[j].cb;
			memset(&to[j], 0, sizeof(Timer_t));
			if (cb)
			{
///////////////////////////////////////////////////////////
				to[j].processing = 1;

// call callback func
				cb(id);

///////////////////////////////////////////////////////////
				to[j].processing = 0;
			}
			else	
			{
				AfxGetApp()->PostMessage(M_TIMER, (WPARAM)id, (LPARAM)cb);
			}

///////////////////////////////////////////////////////////
			//continue;
			break;
		}
	}
	task++;
	task %= N;
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
		usleep(499*10);
	}

///////////////////////
	pthread_cleanup_pop(0);


///////////////////////
	pthread_cleanup_pop(0);

dbg_printf(DBG_THREAD, "(%s %d) TimerThread EXIT\n", __FILE__, __LINE__);
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
	memset(to, 0, sizeof(to));
	hQuit = 0;
	hMsg = 0;

	tMq = 0;
	tTm = 0;
	tSw = 0;

	INIT_LIST_HEAD(&mQ.list);
	if (!pApp)														pApp = this;

	hMsg = CreateEvent(0, AUTO_RESET, 0, "MSG");
	hQuit = CreateEvent(0, MANUAL_RESET, 0, "QUIT");

	PostMessage(M_INIT_INSTANCE, 0, 0);
}

CApp::~CApp()
{
	if (hQuit)															CloseEvent(hQuit);
	if (hMsg)															CloseEvent(hMsg);

	dbg_printf(DBG_BASIC, "(%s %d) PROGRAM EXIT GRACEFULLY \n", __FILE__, __LINE__);
}

MSG *CApp::GetMsgQueue()
{
	return (&mQ);
}

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
	int last_state;
	int last_type;
	int quit = 0;

	pthread_attr_t rr_attr;
	int rr_prio;
	struct sched_param rr_param;


///////////////////////////////////////////////////////////
	pthread_attr_init(&rr_attr);
	pthread_attr_setinheritsched(&rr_attr, PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setscope(&rr_attr, PTHREAD_SCOPE_SYSTEM);
	pthread_attr_setschedpolicy(&rr_attr, SCHED_FIFO);
	rr_prio = sched_get_priority_max(SCHED_FIFO);
	rr_param.sched_priority = rr_prio;
	pthread_attr_setschedparam(&rr_attr, &rr_param);


	sigemptyset(&nset);
	sigaddset(&nset, SIGHUP), sigaddset(&nset, SIGINT), sigaddset(&nset, SIGTERM), sigaddset(&nset, SIGUSR1), sigaddset(&nset, SIGALRM);
	pthread_sigmask(SIG_BLOCK, &nset, &oset);
	pthread_cleanup_push(RestoreSigmask, (void *)&oset);

	if (!hQuit)	return 0;
	if (!hMsg)	return 0;
	if (!tTm)
	{
		if(0 != pthread_create(&tTm, NULL, TimerThread, this))		{ dbg_printf(DBG_THREAD, "TimerThread() NG\n"); return 0; }
		dbg_printf(DBG_THREAD, "(%s %d) TimerThread() OK\n", __FILE__, __LINE__); 
	}
	if(!tSw)
	{
		if (0 != pthread_create(&tSw, NULL, SigwaitThread, this))			{ dbg_printf(DBG_THREAD, "SigwaitThread() NG\n"); return 0; }
		dbg_printf(DBG_THREAD, "(%s %d) SigwaitThread() OK\n", __FILE__, __LINE__);
	}


	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &last_state);
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &last_type);


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
			AFX_PMSG fArray[256 * 1];
			MSG * mArray[256 * 1];

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
								for(x = 0; x < 256 * 1; x++)
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

			for(x = 0; x < 256 * 1; x++)
			{
				if (0 == fArray[x])										{ continue; }	

///////////////////////////////////////////////////////////
				if (mArray[x]->type == M_TIMER)
					to[(UINT)mArray[x]->wparam].processing = 1;

// call message func
				(this->*fArray[x])(mArray[x]->type, mArray[x]->wparam, mArray[x]->lparam, &mArray[x]->result);

///////////////////////////////////////////////////////////
				if (mArray[x]->type == M_TIMER)
					to[(UINT)mArray[x]->wparam].processing = 0;

				delete mArray[x];
			}
			if (quit)													{ SetEvent(hQuit); }
		}
	}

////////////
	pthread_cleanup_pop(1);
	pthread_cleanup_pop(1);


////////////
	pthread_setcanceltype(last_type, NULL);
	pthread_setcancelstate(last_state, NULL);

	return 0;
}

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
	volatile unsigned int et = 0;
	struct timespec request;

	if (nId >= MAX_TIMER)
		return 0;

again:
	memset(&request, 0, sizeof(request));
	if (0 != clock_gettime(CLOCK_REALTIME, &request))
	{
		sched_yield();
		goto again;
	}
	et = (request.tv_sec * 1000) + (request.tv_nsec / 1000000) + nElapse;
	to[nId].cb = cb;
	to[nId].timeout = et;

//////////////////////////////////////////////////////////////////////
	to[nId].id = nId;
	to[nId].enable = 1;
	//to[nId].processing = 0;
	return (nId + 1);
}

BOOL CApp::KillTimer(UINT nId)
{
	if (nId >= MAX_TIMER)
		return 0;

	if (!pthread_equal(tTm, pthread_self()))
	{
		while(to[nId].processing)
			sched_yield();
	}
	to[nId].enable = 0;
	to[nId].timeout = 0;
	return (nId + 1);
}


//////////////////////////////////////////////////////////////////////
// message map & signal map
//////////////////////////////////////////////////////////////////////

BEGIN_BASE_MESSAGE_MAP(CApp)
	//{{AFX_MSG_MAP(CApp)
	ON_MESSAGE(M_INIT_INSTANCE, &CApp::OnInitInstance)
	ON_MESSAGE(M_EXIT_INSTANCE, &CApp::OnExitInstance)
	ON_MESSAGE(M_TIMER, &CApp::OnTimer)
	//}}AFX_MSG_MAP
END_BASE_MESSAGE_MAP()


//////////////////////////////////////////////////////////////////////
// message handlers
//////////////////////////////////////////////////////////////////////

LRESULT CApp::OnInitInstance(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
	//from -20 (highest priority) to 19 (lowest).
	//setpriority(PRIO_PROCESS, 0, -20);

	return 1;
}

LRESULT CApp::OnExitInstance(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
	dbg_printf(DBG_BASIC, "CApp::OnExitInstance()\n");

///////////////////////////////////////////////////////////
	for (int i = 0; i < MAX_TIMER; i++)	
		KillTimer(i);

	if (tSw)
	{
		pthread_cancel(tSw);
		dbg_printf(DBG_THREAD, "(%s %d) TRYING WAIT() tSw...,   ", __FILE__, __LINE__);
		pthread_join(tSw, NULL);
		tSw = 0;
		dbg_printf(DBG_THREAD, "(%s %d) WAIT() tSw OK\n", __FILE__, __LINE__);
		tSw = 0;
	}
	if (tTm)
	{
		pthread_cancel(tTm);
		dbg_printf(DBG_THREAD, "(%s %d) TRYING WAIT() tTm...,   ", __FILE__, __LINE__);
		pthread_join(tTm, NULL);
		tTm = 0;
		dbg_printf(DBG_THREAD, "(%s %d) WAIT() tTm OK\n", __FILE__, __LINE__);
		tTm = 0;
	}

	return 1;
}

LRESULT CApp::OnTimer(DWORD dwType, WPARAM wParam, LPARAM lParam, LRESULT *pResult)
{
	return 1;
}


