
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

#define GPIO_IOCTL_ID           'G'
#define GPIO_IOCTL_SET_VALUE    _IOW(GPIO_IOCTL_ID, 0, int)
#define GPIO_IOCTL_GET_VALUE    _IOR(GPIO_IOCTL_ID, 1, int)
#define GPIO_IOCTL_SET_INPUT	_IOW(GPIO_IOCTL_ID, 2, int)
#define GPIO_IOCTL_SET_PROPERTY _IOW(GPIO_IOCTL_ID, 3, int)
#define GPIO_IOCTL_SET_INT      _IOW(GPIO_IOCTL_ID, 4, int)

#define GPIO_FUNC_ORG           0
#define GPIO_FUNC_GPIO          1
#define GPIO_DIR_OUTPUT         0
#define GPIO_DIR_INPUT          1
#define GPIO_PULL_LOW           0
#define GPIO_PULL_HIGH          1
#define GPIO_PULL_FLOATING      2
#define GPIO_NO_PULL			3
#define GPIO_IRQ_DISABLE        0
#define GPIO_IRQ_ENABLE         1

#define GP_ADC_MAGIC	'G'
#define IOCTL_GP_ADC_START		_IOW(GP_ADC_MAGIC,0x01,unsigned long)	/*!< start ad conversion*/
#define IOCTL_GP_ADC_STOP		_IO(GP_ADC_MAGIC,0x02)	/*!<stop ad conversion*/

#define MK_GPIO_INDEX(ch, func, gid, pin) \
			(((ch) << 24) | ((func) << 16) | ((gid) << 8) | (pin))

typedef struct gpio_content_s {
	unsigned int pin_index;     /*!< @brief gpio pin index */
	unsigned int value;         /*!< @brief gpio value */
	unsigned int debounce;      /*!< @brief gpio debounce value */
} gpio_content_t;



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

	printf("Usage: %s -D [ADC-DEV] -C [CHANNEL]\n", file);
	printf("Example: %s -D /dev/adc -C 2\n", file), exit(1);
}

///////////////////////////////
//

int main(int argc, char *argv[])
{
	int channel = 2;
	int adc = 0;
	int fd = -1;
	struct stat lbuf;
	char devName[256];
	int len = 0;
	int o;
	int gpio;


	setbuf(stdout, 0);
	strcpy(devName, "/dev/adc");
	if (argc < 2)											PrintUsage();

	while ((o = getopt(argc, argv, "D:d:C:c:")) != -1)
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
			break;

		case 'c':
		case 'C':
			len = strlen(optarg);
			if (len != 1)									printf("(%s %d) DBG\n", __FILE__, __LINE__), PrintUsage();
			if (optarg[0] < '0')								printf("(%s %d) DBG\n", __FILE__, __LINE__), PrintUsage();
			if (optarg[0] > '6')								printf("(%s %d) DBG\n", __FILE__, __LINE__), PrintUsage();
			channel = atoi(optarg);
			if (channel < 0 || channel > 6)					printf("(%s %d) DBG\n", __FILE__, __LINE__), PrintUsage();
			break;
		}
	}


	if (1 == channel)
	{
		gpio = open("/dev/gpio", O_RDWR);
		if (-1 != gpio)
		{
			volatile gpio_content_t ctx;

			printf("SETTING GPIO0[21] INPUT, PULL FLOAT, ...\n");
			ctx.pin_index = MK_GPIO_INDEX(0, 0, 9, 20);
			ctx.debounce = 0;
			ctx.value = 0;
			//if (-1 == ioctl(gpio, GPIO_IOCTL_SET_VALUE, &ctx))	printf("(%s %d) GPIO_IOCTL_SET_VALUE(0) FAIL\n", __FILE__, __LINE__);
			ctx.value = GPIO_PULL_FLOATING;
			if (-1 == ioctl(gpio, GPIO_IOCTL_SET_INPUT, &ctx))
				printf("GPIO_IOCTL_SET_INPUT, IOCTL() FAIL !!\n");
			close(gpio);
		}
	}

	if (2 == channel)
	{
		gpio = open("/dev/gpio", O_RDWR);
		if (-1 != gpio)
		{
			volatile gpio_content_t ctx;

			printf("SETTING GPIO0[21] INPUT, PULL LOW, ...\n");
			ctx.pin_index = MK_GPIO_INDEX(0, 0, 10, 21);
			ctx.debounce = 0;
			ctx.value = 0;
			if (-1 == ioctl(gpio, GPIO_IOCTL_SET_VALUE, &ctx))	
				printf("(%s %d) GPIO_IOCTL_SET_VALUE(0) FAIL\n", __FILE__, __LINE__);
			//ioctl(gpio, GPIO_DIR_OUTPUT, &ctx);
			//sleep(10);

			ctx.value = GPIO_PULL_LOW;
			if (-1 == ioctl(gpio, GPIO_IOCTL_SET_INPUT, &ctx))
				printf("GPIO_IOCTL_SET_INPUT, IOCTL() FAIL !!\n");
			close(gpio);
		}
	}

	fd = open(devName, O_RDWR);
	if (-1 == fd)
	{
		printf("(%s %d) OPEN(%s) FAIL, EXIT \n", __FILE__, __LINE__, devName);
		return 1;
	}
	if (-1 == ioctl(fd, IOCTL_GP_ADC_START, channel))
		printf("IOCTL_GP_ADC_START, IOCTL() FAIL !!\n"), close(fd), exit(1);
	printf("IOCTL_GP_ADC_START(%d)\n", channel);

	if (-1 == read(fd, &adc, sizeof(adc)))
		printf("READ_ADC(%d) FAIL !!\n", channel), close(fd), exit(1);
	if (-1 == ioctl(fd, IOCTL_GP_ADC_STOP, channel))
		printf("IOCTL_GP_ADC_STOP, IOCTL() FAIL !!\n"), close(fd), exit(1);
	printf("IOCTL_GP_ADC_STOP()\n");


	printf("ADC(%d)=%d\n", channel, adc);
	close(fd);
	return 0;
}



