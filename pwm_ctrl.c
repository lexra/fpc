
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdarg.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <signal.h>
#include <termios.h>
#include <netinet/tcp.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>

#include <asm/ioctl.h>
#include <asm/errno.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#include <regex.h>



//////////////////////////////////////////////////////////////////////
// define
//////////////////////////////////////////////////////////////////////

#define GP_PWM_MAGIC					'P'
#define PWM_IOCTL_SET_ENABLE			_IOW(GP_PWM_MAGIC, 1, unsigned int)
#define PWM_IOCTL_SET_ATTRIBUTE		_IOW(GP_PWM_MAGIC, 2, gp_pwm_config_t)
#define PWM_IOCTL_GET_ATTRIBUTE		_IOR(GP_PWM_MAGIC, 2, gp_pwm_config_t*)


typedef struct gp_pwm_config_s {
	unsigned int freq;
	int duty;
	int pin_index;
} gp_pwm_config_t;



//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void PrintUsage(void)
{
	char file[128];
	int i = 0;

	memset(file, 0, sizeof(file));
	strcpy(file, __FILE__);
	for (; i < 128; i++)
		if ('.' == file[i])	file[i] = 0;

	printf("Usage: %s -D [PWM-DEV] -F [FREQ] -U [DUTY] -T [TIMEOUT]\n", file);
	printf("Example: %s -D /dev/pwm2 -F 200 -U 10 -T 6\n", file), exit(1);
}

///////////////////////////////
// 0.372~2.77

// pwm_ctrl -F 200 -D 10


int main(int argc, char *argv[])
{
	int i;
	int t;
	int o;
	time_t timeout = 6;
	int pwm = -1;
	struct gp_pwm_config_s pwmc;
	struct gp_pwm_config_s attr;
	struct stat lbuf;
	char devName[256];
	int len = 0;
	//char tmp[256];
	//regex_t preg;
	//int cflags = REG_EXTENDED;
	//int err;
	//size_t nmatch = 12;
	//regmatch_t pmatch[12];
	//char pattern[256];


	setbuf(stdout, 0);
	if (argc < 2)											PrintUsage();
	strcpy(devName, "/dev/pwm2");
	memset(&pwmc, 0, sizeof(pwmc));
	pwmc.freq = 200;
	pwmc.duty = 10;
	pwmc.pin_index = 2;

	//memset(&preg, 0, sizeof(regex_t));
	//strcpy(pattern, "^/dev/pwm([0-2]{1})$");

	while ((o = getopt(argc, argv, "F:f:D:d:U:u:T:t:")) != -1)
	{
		switch (o)
		{
		default:
			break;

		case '?':
			PrintUsage();
			break;

		case 'd':
		case 'D':
			if (strlen(optarg) == 0)							printf("(%s %d) DBG\n", __FILE__, __LINE__), PrintUsage();
			if(0 > stat(optarg, &lbuf))						printf("(%s %d) DBG(%s)\n", __FILE__, __LINE__, optarg), PrintUsage();
			strcpy(devName, optarg);
			/*if(0 != (err = regcomp(&preg, pattern, cflags)))					printf("REGXCOMP ERROR !\n"), exit(1);
			if (0 != (err = regexec(&preg, devName, nmatch, pmatch, 0)))		printf("REGEXEC ERROR !\n"), regfree(&preg), exit(1);
			memset(tmp, 0, sizeof(tmp));
			strncpy(tmp, devName+ pmatch[1].rm_so, 1);
			pwmc.pin_index = atoi(tmp);
			regfree(&preg);*/
			break;

		case 'f':
		case 'F':
			len = strlen(optarg);
			for (i = 0; i < len; i++)
			{
				if ('0' != optarg[i] && '1' != optarg[i] && '2' != optarg[i] && '3' != optarg[i] && '4' != optarg[i] && '5' != optarg[i] && '6' != optarg[i] && '7' != optarg[i] && '8' != optarg[i] && '9' != optarg[i])
				{
					printf("(%s %d) DBG\n", __FILE__, __LINE__), PrintUsage();
				}
			}
			t = atoi(optarg);
			if (t <= 0)									printf("(%s %d) DBG\n", __FILE__, __LINE__), PrintUsage();
			pwmc.freq = (unsigned int)t;
			break;

		case 'u':
		case 'U':
			len = strlen(optarg);
			for (i = 0; i < len; i++)
			{
				if ('0' != optarg[i] && '1' != optarg[i] && '2' != optarg[i] && '3' != optarg[i] && '4' != optarg[i] && '5' != optarg[i] && '6' != optarg[i] && '7' != optarg[i] && '8' != optarg[i] && '9' != optarg[i])
				{
					PrintUsage();
				}
			}
			t = atoi(optarg);
			if (t <= 0)									printf("(%s %d) DBG\n", __FILE__, __LINE__), PrintUsage();
			pwmc.duty = t;
			break;

		case 't':
		case 'T':
			len = strlen(optarg);
			for (i = 0; i < len; i++)
			{
				if ('0' != optarg[i] && '1' != optarg[i] && '2' != optarg[i] && '3' != optarg[i] && '4' != optarg[i] && '5' != optarg[i] && '6' != optarg[i] && '7' != optarg[i] && '8' != optarg[i] && '9' != optarg[i])
				{
					PrintUsage();
				}
			}
			t = atoi(optarg);
			if (t <= 0)									printf("(%s %d) DBG\n", __FILE__, __LINE__), PrintUsage();
			timeout = t;
			timeout &= 0XFF;
			break;
		}
	}


	pwm = open(devName, O_RDWR);
	if (-1 == pwm)
	{
		printf("(%s %d) OPEN(%s) FAIL, EXIT \n", __FILE__, __LINE__, devName);
		return 1;
	}
	if (-1 == ioctl(pwm, PWM_IOCTL_GET_ATTRIBUTE, &attr))
		printf("PWM_IOCTL_GET_ATTRIBUTE, IOCTL() FAIL !!\n"), close(pwm), exit(1);
	pwmc.pin_index = attr.pin_index;
	printf("INIT ATTRIB, PIN=%d FREQ=%u DUTY=%d\n", attr.pin_index, attr.freq, attr.duty);

	if (-1 == ioctl(pwm, PWM_IOCTL_SET_ATTRIBUTE, &pwmc))
		printf("PWM_IOCTL_GET_ATTRIBUTE, IOCTL() FAIL !!\n"), close(pwm), exit(1);
	printf("SET ATTRIB, PIN=%d FREQ=%u DUTY=%d\n", pwmc.pin_index, pwmc.freq, pwmc.duty);

	if (-1 == ioctl(pwm, PWM_IOCTL_SET_ENABLE, 1))
		printf("PWM_IOCTL_SET_ENABLE(1) FAIL\n"), close(pwm), exit(1);
	sleep(timeout);

	if (-1 == ioctl(pwm, PWM_IOCTL_SET_ENABLE, 0))
		printf("PWM_IOCTL_SET_ENABLE(1) FAIL\n"), close(pwm), exit(1);
	close(pwm);

	printf("PWM_IOCTL DOWN\n");
	return 0;
}


