
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
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

#include <asm/errno.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>




/////////////////////////////////////////////////////////////////////////////////////////
//

static int fd_i2c = -1;

static int i2c_read_data(int slave_adr, int sub_adr, unsigned char *data, int length)
{
	int res;

	if (-1 == fd_i2c)
		return -1;

	if (0 == data)
		return -1;

	res = ioctl(fd_i2c, I2C_SLAVE_FORCE, slave_adr);
	if(res < 0)
	{
		printf("I2C SET SLAVE (0X%02X) ERR\n", slave_adr);
		return -1;
	}

	if(write(fd_i2c, &sub_adr, 1) != 1)
	{
		printf("I2C WRITE DATA ERR \n");
		return -1;
	}

	return read(fd_i2c, data, length);
}

static void PrintUsage(void)
{
	char filename[128];
	int i = 0;

	memset(filename, 0, sizeof(filename));
	strcpy(filename, __FILE__);
	for (; i <128; i++)
		if ('.' == filename[i])	filename[i] = 0;

	printf("Usage: %s -D [I2C-DEV] -S [BUS ADDR]\n", filename);
	printf("Example: %s -D /dev/i2c-0 -S 0X5D\n", filename), exit(1);
}

int main(int argc, char *argv[])
{
	int res;
	int slave = 0X5D;
	char dev[64];
	struct stat lbuf;
	int i = 0;
	unsigned char reg;
	int c;

	strcpy(dev, "/dev/i2c-0");
	setbuf(stdout, 0);
	if (argc < 5)									printf("TOTAL PARAMETER COUNT ERROR \n\n"), PrintUsage();

	while ((c = getopt(argc, argv, "s:S:d:D:")) != -1)
	{
		switch (c)
		{
		default:
			break;
		case '?':
			PrintUsage();
			break;
		case 'D':
		case 'd':
			strcpy(dev, optarg);
			break;
		case 'S':
		case 's':
			if (strlen(optarg) > 4)					printf("PARAM [SLAVE ADDR] ERROR \n\n"), PrintUsage();
			if ('0' == optarg[0] && ('X' == optarg[1] || 'x' == optarg[1]))
			{
				if ('X' == optarg[1])				res = sscanf(optarg, "%X", &slave);
				else								res = sscanf(optarg, "%x", &slave);
				if (-1 == res)						printf("PARAM [SLAVE ADDR] ERROR \n\n"), PrintUsage();
			}
			else									slave = atoi(optarg);
			slave &= 0XFF;
			break;
		}
	}

	if(0 > stat(dev, &lbuf))							printf("PARAM [I2C-DEV] ERROR, FILE NOT FOUND \n\n"), PrintUsage();
	if(!S_ISCHR(lbuf.st_mode))						printf("PARAM [I2C-DEV] ERROR, NOT CHARACTOR DEV \n\n"), PrintUsage();

	fd_i2c = open(dev, O_RDWR);
	if (fd_i2c < 0)								printf("OPEN [I2C-DEV] FAIL \n\n"), PrintUsage();
	i = 0, res = ioctl(fd_i2c, I2C_TENBIT, i);

	printf("I2C BUS ADDR 0X%02X\n", slave);
	printf("\n");

	for (i = 0; i < 256; i++)
	{
		res = i2c_read_data(slave, i, &reg, 1);
		usleep(10000);
		if (0 > res)								exit(1);
		printf("REG-0X%02X=0X%02X\n", i, reg);
	}


	printf("\n");

	close(fd_i2c);
	return 0;
}


