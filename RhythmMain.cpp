// RhythmMain.cpp : Defines the class behaviors for the application.
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
#include <stddef.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>


#include <asm/ioctl.h>
#include <asm/errno.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#include "list.h"
#include "tree.hh"
#include "BaseApp.h"
#include "RhythmApp.h"



/////////////////////////////////////////////////////////////////////////////////////////
//

int main(int argc, char *argv[])
{
	CRhythmApp theApp;

	setbuf(stdout, 0), printf("Compile Date: [%s %s]\n", __DATE__, __TIME__);
	theApp.Run();

	return 0;
}

