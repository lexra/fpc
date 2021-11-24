// Event.cpp : Defines the class behaviors for the application.
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
#include <regex.h>
#include <assert.h>
#include <dirent.h>
#include <sys/mount.h>
#include <pthread.h>

#include "Misc.h"
#include "Event.h"


//////////////////////////////////////////////////////////////////////
// define
//////////////////////////////////////////////////////////////////////

#define TIMEDWAIT_GRANULARITY		10000000L
//#define TIMEDWAIT_GRANULARITY		1000000L


//////////////////////////////////////////////////////////////////////
// extern
//////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////
// static
//////////////////////////////////////////////////////////////////////

static pthread_mutex_t si = PTHREAD_MUTEX_INITIALIZER;
static HANDLE pso[] =
{
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};
unsigned long long int cs = 0;
static pthread_mutex_t cm = PTHREAD_MUTEX_INITIALIZER;



//////////////////////////////////////////////////////////////////////
// routines
//////////////////////////////////////////////////////////////////////

HANDLE CreateEvent(void *pAttr, int bManualReset, int bInitialState, const char *szName)
{
	int i;
	struct event_t *p = 0;
	unsigned long long int mask = 0;

#ifdef PCANCEL
	int last_type;

	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &last_type);
#endif // PCANCEL

	pthread_mutex_lock(&si);
	pthread_cleanup_push(CleanupLock, (void *)&si);
	for (i = 0; i < 64; i++)
	{
		if (pso[i] == 0)
		{
			pso[i] = malloc(sizeof(struct event_t));
			p = (struct event_t *)pso[i];
			assert(p);
			memset(p, 0, sizeof(struct event_t));
			p->id = i;
			mask = 1 << i;
			break;
		}
	}
	pthread_cleanup_pop(1);

#ifdef PCANCEL
	pthread_testcancel();
	pthread_setcanceltype(last_type, NULL);
#endif // PCANCEL

	if(p == 0)										return 0;
	strcpy(p->name, szName);
	p->type = 1;
	p->manual_reset = bManualReset;
	pthread_cond_init(&p->event, NULL);

	pthread_mutex_lock(&cm);
	pthread_cleanup_push(CleanupLock, (void *)&cm);
	if (bInitialState)
		cs |= mask;
	else
		cs &= ~mask;
	pthread_cleanup_pop(1);

	return (void *)p;
}

int CloseEvent(HANDLE hObject)
{
	struct event_t *p;
	unsigned long long int mask;
	int i;

#ifdef PCANCEL
	int last_type;
#endif

	if(hObject == 0)									return 0;
	p = (struct event_t *)hObject;
	i = p->id;
	mask = 1 << i;

#ifdef PCANCEL
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &last_type);
#endif

	pthread_mutex_lock(&cm);
	pthread_cleanup_push(CleanupLock, (void *)&cm);

	cs &= ~mask;
	pthread_cleanup_pop(1);

#ifdef PCANCEL
	pthread_testcancel();
	pthread_setcanceltype(last_type, NULL);

	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &last_type);
#endif

	pthread_mutex_lock(&si);
	pthread_cleanup_push(CleanupLock, (void *)&si);
	pso[i] = 0;
	pthread_cleanup_pop(1);

#ifdef PCANCEL
	pthread_testcancel();
	pthread_setcanceltype(last_type, NULL);
#endif

	free(hObject);

	return 0;
}

int SetEvent(HANDLE hEvent)
{
	struct event_t *p;
	unsigned long long int mask;
	int i;
	int res;

#ifdef PCANCEL
	int last_type;
#endif

	if(hEvent == 0)									return 0;
	p = (struct event_t *)hEvent;
	i = p->id;
	mask = 1 << i;

#ifdef PCANCEL
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &last_type);
#endif

	pthread_mutex_lock(&cm);
	pthread_cleanup_push(CleanupLock, (void *)&cm);
	cs |= mask;
	res = pthread_cond_signal(&p->event);
	pthread_cleanup_pop(1);

#ifdef PCANCEL
	pthread_testcancel();
	pthread_setcanceltype(last_type, NULL);
#endif

	return (res == 0) ? 1 : 0;
}

int ResetEvent(HANDLE hEvent)
{
	struct event_t *p;
	unsigned long long int mask;
	int i;

#ifdef PCANCEL
	int last_type;
#endif

	if(hEvent == 0)									return 0;
	p = (struct event_t *)hEvent;
	i = p->id;
	mask = 1 << i;

#ifdef PCANCEL
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &last_type);
#endif

	pthread_mutex_lock(&cm);
	pthread_cleanup_push(CleanupLock, (void *)&cm);

	cs &= ~mask;
	pthread_cleanup_pop(1);

#ifdef PCANCEL
	pthread_testcancel();
	pthread_setcanceltype(last_type, NULL);
#endif

	return 1;
}

int PulseEvent(HANDLE hEvent)
{
	struct event_t *p;
	unsigned long long int mask;
	int i;
	int res;

#ifdef PCANCEL
	int last_type;
#endif

	if(hEvent == 0)									return 0;
	p = (struct event_t *)hEvent;
	i = p->id;
	mask = 1 << i;

#ifdef PCANCEL
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &last_type);
#endif

	pthread_mutex_lock(&cm);
	pthread_cleanup_push(CleanupLock, (void *)&cm);
	cs |= mask;
	res = pthread_cond_broadcast(&p->event);
	pthread_cleanup_pop(1);

#ifdef PCANCEL
	pthread_testcancel();
	pthread_setcanceltype(last_type, NULL);
#endif

	return (res == 0) ? 1 : 0;
}

unsigned int WaitForSingleObject(HANDLE hEvent, unsigned int dwMilliseconds)
{
	struct event_t *p;
	unsigned long long int mask;
	int i;
	int res = EINVAL;
	struct timespec timeout;
	struct timespec dt, at;

#ifdef PCANCEL
	int last_type;
#endif

	if(hEvent == 0)									return WAIT_FAILED;
	p = (struct event_t *)hEvent;
	i = p->id;
	mask = 1 << i;

	if (dwMilliseconds != INFINITE)
	{
		if (0 != clock_gettime(CLOCK_REALTIME, &timeout))	dbg_printf(DBG_EVENT, "(%s %d) ERROR ON clock_gettime()\n", __FILE__, __LINE__);
		memset(&dt, 0, sizeof(dt));
		dt.tv_sec += (dwMilliseconds / 1000);
		dt.tv_nsec += ((dwMilliseconds % 1000) * 1000000);
		TimespecAdd(&timeout, &dt, &at);
	}

#ifdef PCANCEL
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &last_type);
#endif

	pthread_mutex_lock(&cm);
	pthread_cleanup_push(CleanupLock, (void *)&cm);
	while((cs & mask) != mask)
	{
		if (dwMilliseconds == INFINITE)				res = pthread_cond_wait(&p->event, &cm);
		else											res = pthread_cond_timedwait(&p->event, &cm, &at);
		if (res != 0)
		{
			dbg_printf(DBG_EVENT, "(%s %d) pthread_cond_wait/pthread_cond_timedwait return\n", __FILE__, __LINE__);
			break;
		}
	}

	// Auto Reset
	if(!p->manual_reset)
		cs &= ~mask;
	pthread_cleanup_pop(1);

#ifdef PCANCEL
	pthread_testcancel();
	pthread_setcanceltype(last_type, NULL);
#endif

	if (res == 0)
		return WAIT_OBJECT_0;
	else if (res == ETIMEDOUT)
		return WAIT_TIMEOUT;
	else
		return WAIT_FAILED;
}

unsigned int WaitForMultipleObjects(unsigned int nCount, const HANDLE *lpHandles, int bWaitAll, unsigned int dwMilliseconds)
{
	unsigned int r;
	int i;
	int res = EINVAL;
	HANDLE handle = 0;
	struct event_t *p;
	unsigned long long int mask = 0;

#ifdef PCANCEL
	int last_type;
#endif

	struct timespec ts;
	struct timespec dt, at;
	unsigned long long int id = 0;

	assert(nCount > 0);
	assert(nCount <= 64);
	assert(lpHandles != 0);

	if (0 != clock_gettime(CLOCK_REALTIME, &ts))			dbg_printf(DBG_EVENT, "(%s %d) ERROR ON clock_gettime()\n", __FILE__, __LINE__);
	if(dwMilliseconds != INFINITE)
	{
		memset(&dt, 0, sizeof(dt));
		dt.tv_sec = dwMilliseconds / 1000;
		dt.tv_nsec = (dwMilliseconds % 1000) * 1000000;
		TimespecAdd(&ts, &dt, &at);
	}

#ifdef PCANCEL
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &last_type);
#endif

	pthread_mutex_lock(&cm);
	pthread_cleanup_push(CleanupLock, (void *)&cm);

	mask = 0;
	for (i = 0; i < (int)nCount; i++)
	{
		handle = lpHandles[i];
		p = (struct event_t *)handle;
		mask |= (1 << p->id);
	}

///////////////////////////////////////////////////////////
// wait for any one of the events
	if (!bWaitAll)
	{
		while (!(mask & cs))
		{
			if (dwMilliseconds == INFINITE)			res = pthread_cond_wait(&p->event, &cm);
			else										res = pthread_cond_timedwait(&p->event, &cm, &at);
			if(res == 0)
			{
				dbg_printf(DBG_EVENT, "(%s %d) pthread_cond_timedwait return\n", __FILE__, __LINE__);
				break;
			}
			if(res == ETIMEDOUT)
			{
				dbg_printf(DBG_EVENT, "(%s %d) ETIMEDOUT\n", __FILE__, __LINE__);
				r = WAIT_TIMEOUT;
				goto EXIT;
			}
			if(res == EINVAL)
			{
				dbg_printf(DBG_EVENT, "(%s %d) pthread_cond_timedwait return EINVAL\n", __FILE__, __LINE__);
				r = WAIT_FAILED;
				goto EXIT;
			}
		}

		for (i = 0; i < (int)nCount; i++)
		{
			handle = lpHandles[i];
			p = (struct event_t *)handle;
			id = p->id;
			//if ((cs & (unsigned long long int)(1 << id)) == (unsigned long long int)(1 << id))
			if (cs & (1 << id))
			{
				if(!p->manual_reset)					cs &= ~(1 << id);
				break;
			}
		}

		r = WAIT_OBJECT_0 + (unsigned int)i;
		goto EXIT;
	}

///////////////////////////////////////////////////////////
// wait for all of the events
	while ((mask & cs) != cs)
	{
		if (dwMilliseconds == INFINITE)				res = pthread_cond_wait(&p->event, &cm);
		else											res = pthread_cond_timedwait(&p->event, &cm, &at);
		if(res == 0)
		{
			dbg_printf(DBG_EVENT, "(%s %d) pthread_cond_timedwait return\n", __FILE__, __LINE__);
			break;
		}
		if(res == ETIMEDOUT)
		{
			dbg_printf(DBG_EVENT, "(%s %d) ETIMEDOUT\n", __FILE__, __LINE__);
			r = WAIT_TIMEOUT;
			goto EXIT;
		}
		if(res == EINVAL)
		{
			dbg_printf(DBG_EVENT, "(%s %d) pthread_cond_timedwait return EINVAL\n", __FILE__, __LINE__);
			r = WAIT_FAILED;
			goto EXIT;
		}
	}

	for (i = 0; i < (int)nCount; i++)
	{
		handle = lpHandles[i];
		p = (struct event_t *)handle;
		id = p->id;
		//if ((cs & (unsigned long long int)(1 << id)) == (unsigned long long int)(1 << id))
		if (cs & (1 << id))
		{
			if(!p->manual_reset)						cs &= ~(1 << id);
		}
	}

	r = WAIT_OBJECT_0;
	goto EXIT;


///////////////////////////////////////////////////////////
// RETURN
EXIT:
	pthread_cleanup_pop(1);


#ifdef PCANCEL
	pthread_testcancel();
	pthread_setcanceltype(last_type, NULL);
#endif

	return r;
}

