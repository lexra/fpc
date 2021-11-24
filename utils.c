
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
#include <time.h>
#include <dirent.h>
#include <sys/mount.h>
#include <pthread.h>

#include "Event.h"


//////////////////////////////////////////////////////////////////////
// define
//////////////////////////////////////////////////////////////////////

#define TIMEDWAIT_GRANULARITY		10000000


//////////////////////////////////////////////////////////////////////
// extern
//////////////////////////////////////////////////////////////////////

//extern char *strncpy(char *dest, const char *src, size_t n);


//////////////////////////////////////////////////////////////////////
// static
//////////////////////////////////////////////////////////////////////

static pthread_mutex_t si = PTHREAD_MUTEX_INITIALIZER;
static HANDLE pso[64] =
{
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};
static unsigned long long int cs = 0;
static pthread_mutex_t cm = PTHREAD_MUTEX_INITIALIZER;

static void CleanupLock(void *pParam)
{
	pthread_mutex_unlock((pthread_mutex_t *)pParam);
}


//////////////////////////////////////////////////////////////////////
// routines
//////////////////////////////////////////////////////////////////////

HANDLE CreateEvent(void *pAttr, int bManualReset, int bInitialState, const char *szName)
{
	int i;
	struct event_t *p = 0;
	unsigned long long int mask = 0;
	int last_type;

	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &last_type);
	pthread_mutex_lock(&si);
	pthread_cleanup_push(CleanupLock, (void *)&si);
	pthread_setcanceltype(last_type, NULL);
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

	if(p == 0)
		return 0;

	strcpy(p->name, szName);
	p->type = 1;
	p->manual_reset = bManualReset;
	pthread_cond_init(&p->event, NULL);

	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &last_type);
	pthread_mutex_lock(&cm);
	pthread_cleanup_push(CleanupLock, (void *)&cm);
	pthread_setcanceltype(last_type, NULL);
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
	int last_type;

	if(hObject == 0)
		return 0;

	p = (struct event_t *)hObject;
	i = p->id;
	mask = 1 << i;

	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &last_type);
	pthread_mutex_lock(&cm);
	pthread_cleanup_push(CleanupLock, (void *)&cm);
	pthread_setcanceltype(last_type, NULL);
	cs &= ~mask;
	pthread_cleanup_pop(1);

	pthread_mutex_lock(&si);
	pthread_cleanup_push(CleanupLock, (void *)&si);
	pso[i] = 0;
	pthread_cleanup_pop(1);

	free(hObject);

	return 0;
}

int SetEvent(HANDLE hEvent)
{
	struct event_t *p;
	unsigned long long int mask;
	int i;
	int res;
	int last_type;

	if(hEvent == 0)
		return 0;

	p = (struct event_t *)hEvent;
	i = p->id;
	mask = 1 << i;

	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &last_type);
	pthread_mutex_lock(&cm);
	pthread_cleanup_push(CleanupLock, (void *)&cm);
	pthread_setcanceltype(last_type, NULL);
	cs |= mask;
	res = pthread_cond_signal(&p->event);
	pthread_cleanup_pop(1);

	return (res == 0) ? 1 : 0;
}

int ResetEvent(HANDLE hEvent)
{
	struct event_t *p;
	unsigned long long int mask;
	int i;
	int last_type;

	if(hEvent == 0)
		return 0;

	p = (struct event_t *)hEvent;
	i = p->id;
	mask = 1 << i;

	// �O�@ pthread_cleanup_push
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &last_type);
	pthread_mutex_lock(&cm);
	pthread_cleanup_push(CleanupLock, (void *)&cm);
	pthread_setcanceltype(last_type, NULL);
	cs &= ~mask;
	pthread_cleanup_pop(1);

	return 1;
}

int PulseEvent(HANDLE hEvent)
{
	struct event_t *p;
	unsigned long long int mask;
	int i;
	int res;
	int last_type;

	if(hEvent == 0)
		return 0;

	p = (struct event_t *)hEvent;
	i = p->id;
	mask = 1 << i;

	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &last_type);
	pthread_mutex_lock(&cm);
	pthread_cleanup_push(CleanupLock, (void *)&cm);
	pthread_setcanceltype(last_type, NULL);
	cs |= mask;
	res = pthread_cond_broadcast(&p->event);
	pthread_cleanup_pop(1);

	return (res == 0) ? 1 : 0;
}

unsigned int WaitForSingleObject(HANDLE hEvent, unsigned int dwMilliseconds)
{
	struct event_t *p;
	unsigned long long int mask;
	int i;
	int res = EINVAL;
	struct timespec timeout;
	int last_type;

	if(hEvent == 0)
		return WAIT_FAILED;

	p = (struct event_t *)hEvent;
	i = p->id;
	mask = 1 << i;

	if (0 != clock_gettime(CLOCK_REALTIME, &timeout))		printf("(%s %d) ERROR ON clock_gettime()\n", __FILE__, __LINE__);
	if (dwMilliseconds != INFINITE)
	{
		timeout.tv_sec += (dwMilliseconds / 1000);
		timeout.tv_nsec += (dwMilliseconds % 1000000);
	}

	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &last_type); // �j�� PTHREAD_CANCEL_DEFERRED cancellation type
	pthread_mutex_lock(&cm);
	pthread_cleanup_push(CleanupLock, (void *)&cm);
	pthread_setcanceltype(last_type, NULL);						// �٭��Ӫ� cancellation type
	while((cs & mask) != mask)
	{
		pthread_testcancel();

		if (dwMilliseconds == INFINITE)
			res = pthread_cond_wait(&p->event, &cm);
		else
			res = pthread_cond_timedwait(&p->event, &cm, &timeout);
		if (res != 0)
		{
			//printf("(%s %d) pthread_cond_wait/pthread_cond_timedwait return\n", __FILE__, __LINE__);
			break;
		}
	}

	// Auto Reset
	if(!p->manual_reset)
		cs &= ~mask;
	pthread_cleanup_pop(1);

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
	int j = 0;
	int res = EINVAL;
	HANDLE handle = 0;
	struct event_t *p;
	unsigned long long int mask = 0;

	struct timespec to;
	struct timespec ts;
	long long int expired = 0, now;
	int last_type;

	assert(nCount > 0);
	assert(nCount <= 64);
	assert(lpHandles != 0);

	if (0 != clock_gettime(CLOCK_REALTIME, &to))		printf("(%s %d) ERROR ON clock_gettime()\n", __FILE__, __LINE__);
	if(dwMilliseconds != INFINITE)
	{
		expired = (long long int)to.tv_sec;
		expired *= 1000;
		expired += (long long int)(to.tv_nsec / 1000000);
		expired += (long long int)dwMilliseconds;
	}

	///////////////////////////////////////////////////////////
	// wait for any one of the events
	if (!bWaitAll)
	{
		pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &last_type);
		pthread_mutex_lock(&cm);
		pthread_cleanup_push(CleanupLock, (void *)&cm);
		pthread_setcanceltype(last_type, NULL);
		i = 0;
		do
		{
			pthread_testcancel();

			handle = lpHandles[i];
			p = (struct event_t *)handle;
			mask = (1 << p->id);
			if ((mask & cs) == mask)
				break;

			if (0 != clock_gettime(CLOCK_REALTIME, &ts))		printf("(%s %d) ERROR ON clock_gettime()\n", __FILE__, __LINE__);
			if(dwMilliseconds != INFINITE)
			{
				now = (long long int)ts.tv_sec;
				now *= 1000;
				now += (long long int)(ts.tv_nsec / 1000000);
				if(now >= expired)
				{
					r = WAIT_TIMEOUT;
					goto EXIT0;
				}
			}

			ts.tv_nsec += TIMEDWAIT_GRANULARITY;
			res = pthread_cond_timedwait(&p->event, &cm, &ts);
			if(res == 0)
			{
				//printf("(%s %d) pthread_cond_timedwait return\n", __FILE__, __LINE__);
				break;
			}
			if(res == ETIMEDOUT)
			{
				//printf("(%s %d) ETIMEDOUT\n", __FILE__, __LINE__);
			}
			if(res == EINVAL)
			{
				printf("(%s %d) pthread_cond_timedwait return EINVAL\n", __FILE__, __LINE__);
				r = WAIT_FAILED;
				goto EXIT0;
			}

			i++;
			i %= (int)nCount;
		} while(1);

		// Auto Reset
		if(!p->manual_reset)
			cs &= ~mask;

		r = (unsigned int)(WAIT_OBJECT_0 + i);
EXIT0:
		pthread_cleanup_pop(1);
		return r;
	}

	///////////////////////////////////////////////////////////
	// wait for all of the events
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &last_type);
	pthread_mutex_lock(&cm);
	pthread_cleanup_push(CleanupLock, (void *)&cm);
	pthread_setcanceltype(last_type, NULL);
	i = 0;
	do
	{
		pthread_testcancel();

		for (j = 0; j < (int)nCount; j++)
		{
			handle = lpHandles[j];
			p = (struct event_t *)handle;
			mask |= (1 << p->id);
		}

		if ((mask & cs) == mask)
			break;

		handle = lpHandles[i];
		p = (struct event_t *)handle;

		if (0 != clock_gettime(CLOCK_REALTIME, &ts))		printf("(%s %d) ERROR ON clock_gettime()\n", __FILE__, __LINE__);
		if(dwMilliseconds != INFINITE)
		{
			now = (long long int)ts.tv_sec;
			now *= 1000;
			now += (long long int)(ts.tv_nsec / 1000000);
			if(now >= expired)
			{
				r = WAIT_TIMEOUT;
				goto EXIT1;
			}
		}

		ts.tv_nsec += TIMEDWAIT_GRANULARITY;
		res = pthread_cond_timedwait(&p->event, &cm, &ts);
		if(res == EINVAL)
		{
			printf("pthread_cond_timedwait return EINVAL\n");
			r = WAIT_FAILED;
			goto EXIT1;
		}

		i++;
		i %= (int)nCount;
	} while(1);

	// Auto Reset
	for (j = 0; j < (int)nCount; j++)
	{
		handle = lpHandles[j];
		p = (struct event_t *)handle;
		if(!p->manual_reset)
			cs &= ~mask;
	}

	r = (unsigned int)(WAIT_OBJECT_0 + i);
EXIT1:
	pthread_cleanup_pop(1);
	return r;
}

