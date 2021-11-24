
// Pool.cpp: implementation of the CPool class.
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

#include "Pool.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#ifdef WIN32
#define new DEBUG_NEW
#endif
#endif


//////////////////////////////////////////////////////////////////////
// static
//////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////
// CPool Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPool::CPool(INT nCli, INT nBuffers, INT nSize)
{
	start = end = 0;
	X = cliNum = nCli;
	Y = bufNum = nBuffers;
	Z = bufSize = nSize;
	Y++;
}

CPool::~CPool()
{
	if(start)	free(start), start = end = 0;
}

unsigned char *CPool::Malloc(void)
{
	if (start)	return start;
	start = (unsigned char *)malloc((X + 1) * (Y + 1) * (Z + 1));
	end = start + (X * Y * Z);
	memset(start, 0, X * Y * Z);
	return start;
}

INT CPool::GetEntryNum(unsigned char *p)
{
	INT i = 0;
	INT j = 0;
	INT l = 0;

	if(p < start)		return -1;
	if(p >= end)		return -1;
	l = (p - start);
	i = 0, j = 0;
	while(1)
	{
		j += (Y*Z);
		if (j > l)		break;
		i++;
	}
	return i;
}

INT CPool::GetEntryId(unsigned char *p)
{
	INT i = 0;
	INT j = 0;
	INT l = 0;
	INT ID = 0;

	if(p < start)				return -1;
	if(p >= end)				return -1;
	l = (p - start);
	i = 0, j = 0;
	while(1)
	{
		j += (Y*Z);
		if (j > l)		break;
		i++;
	}
	memcpy(&ID, start + (i * Y * Z), sizeof(ID));
	return ID;
}

unsigned char *CPool::SetEntryId(unsigned char *p, INT id)
{
	INT i = 0;
	INT j = 0;
	INT l = 0;

	if(p < start)				return 0;
	if(p >= end)				return 0;
	l = (p - start);
	i = 0, j = 0;
	while(1)
	{
		j += (Y*Z);
		if (j > l)		break;
		i++;
	}
	memcpy(start + (i * Y * Z), &id, sizeof(id));
	return (start + (i * Y * Z) + Z);
}

unsigned char *CPool::NewEntry(void)
{
	INT i;
	INT ID = -1;
	unsigned char *p = 0;
	INT f = 0;

	for (i = 0; i < X; i++)
	{
		p = start + (i * Y * Z);
		memcpy(&ID, p, sizeof(ID));
		if (0 == ID)
		{
			f = 1;
			break;
		}
	}
	if (0 == f)				return 0;
	ID = -1, memcpy((void *)(start + (i * Y * Z)), (void *)&ID, sizeof(ID));
	return (start + (i * Y * Z) + Z);
}

unsigned char *CPool::GetEntry(unsigned char *p)
{
	INT i = 0;
	INT j = 0;
	INT l = 0;

	if(p < start)				return 0;
	if(p >= end)				return 0;
	l = (p - start);
	i = 0, j = 0;
	while(1)
	{
		j += (Y*Z);
		if (j > l)		break;
		i++;
	}
	return (start + (i * Y * Z) + Z);
}

INT CPool::ReleaseEntry(unsigned char *p)
{
	unsigned char *e;

	if(p < start)				return 0;
	if(p >= end)				return 0;
	e = GetEntry(p);
	if (0 == e)				return 0;
	memset((e - Z), 0, Y * Z);
	return 1;
}

INT CPool::GetBuffNum(unsigned char *p)
{
	unsigned char *e;
	INT i = 0;
	INT l = 0;
	INT j = 0;

	if(p < start)				return -1;
	if(p >= end)				return -1;
	e = GetEntry(p);
	if (0 == e)				return -1;
	if (p < e)					return 0;
	l = (p - e);
	i = 0, j = 0;
	while(1)
	{
		j += Z;
		if (j > l)				break;
		i++;
	}
	return i;
};

unsigned char *CPool::GetBuff(unsigned char *p)
{
	unsigned char *e;
	INT i = 0;

	if(p < start)				return 0;
	if(p >= end)				return 0;
	e = GetEntry(p);
	if(0 == e)				return 0;
	//if (p < e)					return e;
	if (p < e)					return 0;
	i = GetBuffNum(p);
	if (-1 == i)				return 0;
	printf("(%s %d) i=%d\n", __FILE__, __LINE__, i);
	return (e + (i * Z));
};

unsigned char *CPool::EmptyBuff(unsigned char *p)
{
	unsigned char *e, *b;
	INT i = -1;

	if(p < start)				return 0;
	if(p >= end)				return 0;
	e = GetEntry(p);
	if (0 == e)				return 0;
	i = GetBuffNum(p);
	if (-1 == i)				return 0;
	b = (e + Z * i);
	memset(b, 0, Z);
	return b;
};

unsigned char *CPool::NextBuff(unsigned char *p)
{
	unsigned char *e, *b;
	INT i = -1;

	if(p < start)				return 0;
	if(p >= end)				return 0;
	e = GetEntry(p);
	if (0 == e)
	{
		printf("(%s %d) 0==GetEntry()\n", __FILE__, __LINE__);
		return 0;
	}
	i = GetBuffNum(p);
	if (-1 == i)
	{
		printf("(%s %d) -1==GetBuffNum()\n", __FILE__, __LINE__);
		return 0;
	}
	i++;
	i %= (Y - 1);
	b = (e + Z * i);
	memset(b, 0, sizeof(INT));
	return b;
};

INT CPool::GetBuffLen(unsigned char *p)
{
	unsigned char *b = 0;
	INT len = 0;

	if(p < start)				return -1;
	if(p >= end)				return -1;
	b = GetBuff(p);
	if (0 == b)				return -1;
	memcpy(&len, (void *)b, sizeof(len));
	return len;
};

unsigned char *CPool::Memcpy(unsigned char *p, unsigned char *s, INT len)
{
	INT offs = 0;
	INT left = 0;
	unsigned char *b;

	if(p < start)				return 0;
	if(p >= end)				return 0;
	if (len > Z)				return 0;
	b = GetBuff(p);
	if (0 == b)				return 0;

	memcpy(&offs, b, sizeof(offs));
	offs %= Z;
	left = Z - offs;
	if (left < len)				return 0;
	memcpy((b + offs), (void *)s, len);
	offs += len;
	memcpy(b, (void *)&offs, sizeof(offs));
	return (b + offs);
}

unsigned char *CPool::FindEntry(INT id)
{
	INT i;
	unsigned char *p = 0;
	INT ID = 0;

	if (id <= 0)	return 0;
	for (i = 0; i < X; i++)
	{
		p = start + (i * Y * Z);
		memcpy(&ID, p, sizeof(ID));
		if (0 == ID)		continue;
		if(ID == id)		break;
	}
	return p;
}


