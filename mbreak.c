
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


//#include "Event.h"
//#include "Misc.h"


//////////////////////////////////////////////////////////////////////
// define
//////////////////////////////////////////////////////////////////////

#define BIT0															(1<<0)
#define BIT1															(1<<1)
#define BIT2															(1<<2)
#define BIT3															(1<<3)
#define BIT4															(1<<4)
#define BIT5															(1<<5)
#define BIT6															(1<<6)
#define BIT7															(1<<7)

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

int i2c_write_data(int fd_i2c, int bus_adr, int sub_adr, const unsigned char *data, int length)
{
	int res;
	char buf[256];
	int i;

	if (0 == data || 0 >= fd_i2c)
		return -1;

	res = ioctl(fd_i2c, I2C_SLAVE_FORCE, bus_adr);
	if(res < 0)
	{
		return -1;
	}
	buf[0] = sub_adr;
	for(i = 1; i <= length; i++)
	{
		buf[i] = data[i -1];
	}

	if(write(fd_i2c, buf, length + 1) != (length + 1))
	{
		return -1;
	}

	return length;
}

int i2c_read_data(int fd_i2c, int bus_adr, int sub_adr, unsigned char *data, int length)
{
	int res;

	if (0 == data || 0 >= fd_i2c)
		return -1;

	res = ioctl(fd_i2c, I2C_SLAVE_FORCE, bus_adr);
	if(res < 0)
	{
		return -1;
	}

	if(write(fd_i2c, &sub_adr, 1) != 1)
	{
		return -1;
	}

	return read(fd_i2c, data, length);
}


int ReadAdc(int fd, unsigned char *value)
{
	int i;

	int probe = 0;
	unsigned char X = 0;
	unsigned char Y = 0;
	static unsigned char O = 0;

	if (-1 == fd)		return 0;
	for (i = 0; i < 256; i++)
	{
		if (-1 == i2c_write_data(fd, PCA_9555_SLAVE_2, PCA_9555_OUTPUT_1, &X, 1))
		{
			printf("(%s %d) i2c_write_data FAIL \n", __FILE__, __LINE__);
			return 0;
		}
		usleep(1000);
		if (-1 == i2c_read_data(fd, PCA_9555_SLAVE_2, PCA_9555_INPUT_0, &Y, 1))
		{
			printf("(%s %d) i2c_read_data FAIL \n", __FILE__, __LINE__);
			return 0;
		}
		usleep(1000);

		if ((Y & BIT7) == BIT7)
		{
			probe = 1;
			break;
		}

		O = X;
		if (X >= 255)				X = 0;
		else						X++;
	}

	if (0 == probe)		return 0;
	*value = O;
	return 1;
}

void PrintUsage(void)
{
	char file[128];
	int i = 0;

	memset(file, 0, sizeof(file));
	strcpy(file, __FILE__);
	for (; i < 128; i++)
		if ('.' == file[i])	file[i] = 0;

	printf("Usage: %s -L [BRAKE-LEVEL] -T [TIMEOUT]\n", file);
	printf("Example: %s -L 6 -T 6\n", file), exit(1);
}

///////////////////////////////
// 0.372~2.77

// i2cw -A 0X21 -R 0X03 -D 0X00 

int main(int argc, char *argv[])
{
	int adc1 = -1;
	int t;
	time_t timeout = 6;
	int i2c0 = -1;
	int level = 6;
	//static unsigned char adc = 0;
	static unsigned int adc = 0;
	time_t now = 0;
	int D = 0;

	int res;
	int o;
	int len = 0;
	static unsigned char v;
	int i;
	int ok = 0;

	setbuf(stdout, 0);
	if (argc < 2)											PrintUsage();

	while ((o = getopt(argc, argv, "L:l:T:t:")) != -1)
	{
		switch (o)
		{
		default:
			break;
		case '?':
			PrintUsage();
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
			if (t <= 0)									PrintUsage();
			timeout = (time_t)t;
			timeout &= 0XFF;
			break;

		case 'l':
		case 'L':
			len = strlen(optarg);
			for (i = 0; i < len; i++)
			{
				if ('0' != optarg[i] && '1' != optarg[i] && '2' != optarg[i] && '3' != optarg[i] && '4' != optarg[i] && '5' != optarg[i] && '6' != optarg[i] && '7' != optarg[i] && '8' != optarg[i] && '9' != optarg[i])
				{
					PrintUsage();
				}
			}
			level = atoi(optarg);
			if (level <= 0)									PrintUsage();
			level &= 0XFF;
			break;
		}
	}

	adc1 = open("/dev/adc", O_RDWR);
	if (-1 == adc1)										printf("(%s %d) open('/dev/adc') Fail \n", __FILE__, __LINE__), close(adc1), exit(1);

	i2c0 = open("/dev/i2c-0", O_RDWR);
	if (-1 == i2c0)
	{
		printf("(%s %d) OPEN(/dev/i2c-0) FAIL, EXIT \n", __FILE__, __LINE__), close(adc1);
		return 1;
	}

	// CHR_Signal_GP, Polar_HR, SPU
	res = i2c_read_data(i2c0, PCA_9555_SLAVE_1, PCA_9555_CONFIG_1, &v, 1);
	if (-1 == res)
	{
		printf("(%s %d) i2c() FAIL, EXIT \n", __FILE__, __LINE__), close(i2c0), close(adc1);
		return 1;
	}
	v |= BIT4; v |= BIT5; v |= BIT6; // IN
	v &= ~BIT0; v &= ~BIT1; v &= ~BIT2; v &= ~BIT3; // OUT
	res = i2c_write_data(i2c0, PCA_9555_SLAVE_1, PCA_9555_CONFIG_1, &v, 1);
	if (-1 == res)
	{
		printf("(%s %d) i2c() FAIL, EXIT \n", __FILE__, __LINE__), close(i2c0), close(adc1);
		return 1;
	}


///////////////////////////////////////////////////////////
	res = i2c_read_data(i2c0, PCA_9555_SLAVE_1, PCA_9555_CONFIG_0, &v, 1);
	if (-1 == res)
	{
		printf("(%s %d) i2c() FAIL, EXIT \n", __FILE__, __LINE__), close(i2c0), close(adc1);
		return 1;
	}
	v |= BIT0; v |= BIT1; v |= BIT4;
	v &= ~BIT2; v &= ~BIT3; v &= ~BIT5; v &= ~BIT6; v &= ~BIT7;
	res = i2c_write_data(i2c0, PCA_9555_SLAVE_1, PCA_9555_CONFIG_0, &v, 1);
	if (-1 == res)
	{
		printf("(%s %d) i2c() FAIL, EXIT \n", __FILE__, __LINE__), close(i2c0), close(adc1);
		return 1;
	}

// BUZZ
	res = i2c_read_data(i2c0, PCA_9555_SLAVE_1, PCA_9555_INPUT_0, &v, 1);
	if (-1 == res)
	{
		printf("(%s %d) i2c() FAIL, EXIT \n", __FILE__, __LINE__), close(i2c0), close(adc1);
		return 1;
	}
	v &= ~(BIT6); v &= ~BIT7;
	res = i2c_write_data(i2c0, PCA_9555_SLAVE_1, PCA_9555_OUTPUT_0, &v, 1);
	if (-1 == res)
	{
		printf("(%s %d) i2c() FAIL, EXIT \n", __FILE__, __LINE__), close(i2c0), close(adc1);
		return 1;
	}


	// output
	v = 0X00; res = i2c_write_data(i2c0, PCA_9555_SLAVE_2, PCA_9555_CONFIG_1, &v, 1);
	if (-1 == res)
	{
		printf("(%s %d) i2c_write_data() FAIL, EXIT \n", __FILE__, __LINE__), close(i2c0), close(adc1);
		return 1;
	}
	// input
	v = 0XFF; res = i2c_write_data(i2c0, PCA_9555_SLAVE_2, PCA_9555_CONFIG_0, &v, 1);
	if (-1 == res)
	{
		printf("(%s %d) i2c_write_data() FAIL, EXIT \n", __FILE__, __LINE__), close(i2c0), close(adc1);
		return 1;
	}

	time(&now);
	timeout += now;
	for(;;)
	{
		int gpio;

		if (-1 == i2c_read_data(i2c0, PCA_9555_SLAVE_1, PCA_9555_INPUT_0, &v, 1))
		{
			printf("\n(%s %d) i2c_write_data() FAIL, EXIT \n", __FILE__, __LINE__);
			goto BAITOUT;
		}
		v |= BIT2; v |= BIT3;
		if (-1 == i2c_write_data(i2c0, PCA_9555_SLAVE_1, PCA_9555_OUTPUT_0, &v, 1))
		{
			printf("\n(%s %d) i2c_write_data() FAIL, EXIT \n", __FILE__, __LINE__);
			goto BAITOUT;
		}


		time(&now);
		if (now > timeout)
		{
			printf("\n(%s %d) TIMEOUT, EXIT \n", __FILE__, __LINE__);
			goto BAITOUT;
		}

		gpio = open("/dev/gpio", O_RDWR);
		if(-1 != gpio)
		{
			gpio_content_t ctx;
			ctx.pin_index = MK_GPIO_INDEX(0, 0, 9, 20);
			ctx.value = 0;
			ctx.value = GPIO_PULL_FLOATING; ctx.debounce = 0;
			ioctl(gpio, GPIO_IOCTL_SET_INPUT, &ctx);
			close(gpio);
		}

		if (-1 == ioctl(adc1, IOCTL_GP_ADC_START, 1))		close(i2c0), close(adc1), exit(1);
		if (-1 == read(adc1, &adc, sizeof(int)))				close(i2c0), close(adc1), exit(1);
		if (-1 == ioctl(adc1, IOCTL_GP_ADC_STOP, 1))			close(i2c0), close(adc1), exit(1);
		adc >>= 2;
/*
		if(0 == ReadAdc(i2c0, &adc))
		{
			printf("\n(%s %d) ReadAdc() FAIL, EXIT \n", __FILE__, __LINE__);
			//goto BAITOUT;
			//adc = 255;
			printf("L=%d A=XXX\n", level);
			//break;
		}
		else
		{
			printf("L=%d A=%d\n", level, adc);
		}
*/
		D = (int)level - (int)adc;
		if (D <= 1 && D >= -1)
		{
			ok = 1;
			break;
		}
		else
		{
			if (D >= 0)		{ v &= ~BIT2; v |= BIT3; }
			else				{ v &= ~BIT3; v |= BIT2; }

			if (-1 == i2c_write_data(i2c0, PCA_9555_SLAVE_1, PCA_9555_OUTPUT_0, &v, 1))
			{
				printf("\n(%s %d) i2c_write_data() FAIL, EXIT \n", __FILE__, __LINE__);
				goto BAITOUT;
			}
			usleep(20 * 1000);
		}
	}

	if (ok)
		printf("\nMORTOR BREAK, LEVEL=%d ADC=%d\n", level, adc);

BAITOUT:
	v |= BIT2; v |= BIT3;
	i2c_write_data(i2c0, PCA_9555_SLAVE_1, PCA_9555_OUTPUT_0, &v, 1);

	close(i2c0);
	close(adc1);
	return 0;
}

