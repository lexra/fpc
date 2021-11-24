
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


// #mbreak -L 6

void PrintUsage(void)
{
	char file[128];
	int i = 0;

	memset(file, 0, sizeof(file));
	strcpy(file, __FILE__);
	for (; i < 128; i++)
		if ('.' == file[i])	file[i] = 0;

	printf("Usage: %s -D [TTY-SERIAL-DEV] -B [BAUD] -H [HEX-CODE] -T [TIMEOUT]\n", file);
	printf("Example: %s -D /dev/ttyUSB0 -B 4800 -H F5-C3-00-0F-2E -T 6\n", file);
	printf("Example: %s -D /dev/ttyUSB0 -B 4800 -H F5-D2-42-EC -T 6\n", file);
	printf("Example: %s -D /dev/ttyUSB0 -B 4800 -H F5-D2-07-27 -T 6\n", file);
	printf("Example: %s -D /dev/ttyUSB0 -B 4800 -H F5-D2-08-26 -T 6\n", file), exit(1);
}

int main(int argc, char *argv[])
{
	int ttyfd = -1;
	int res;
	unsigned char hex[1024];
	unsigned char rcv[1024];
	int baud = 4800;
	char ttyS[256];
	char tmp[256];
	int o;
	struct stat lbuf;
	int len = 0, l, v;
	int i;
	char c;
	struct termios term;
	int left = 0;
	unsigned char *p = hex;
	fd_set rset;
	struct timeval tv;
	int X;
	int L = 0;


	//setbuf(stdout, 0);
	memset(&tv, 0, sizeof(tv));
	tv.tv_sec = 6; tv.tv_usec = 1000;
	memset(tmp, 0, sizeof(tmp));
	strcpy(ttyS, "/dev/ttyUSB0");
	if (argc < 4)											printf("(%s %d) DBG\n", __FILE__, __LINE__), PrintUsage();

	while ((o = getopt(argc, argv, "d:D:B:b:H:h:T:t:")) != -1)
	{
		switch (o)
		{
		default:
			break;
		case '?':
			printf("(%s %d) DBG\n", __FILE__, __LINE__), PrintUsage();
			break;

		case 't':
		case 'T':
			len = strlen(optarg);
			if (len == 0)									printf("(%s %d) DBG\n", __FILE__, __LINE__), PrintUsage();
			for (i = 0; i < len; i++)
			{
				c = optarg[i];
				if (c != '0' && c != '1' && c != '2' && c != '3' && c != '4' && c != '5' && c != '6' && c != '7' && c != '8' && c != '9')
					PrintUsage();
			}
			tv.tv_sec = atoi(optarg);
			break;


		case 'd':
		case 'D':
			if (strlen(optarg) == 0)							printf("(%s %d) DBG\n", __FILE__, __LINE__), PrintUsage();
			if(0 > stat(optarg, &lbuf))						printf("(%s %d) DBG\n", __FILE__, __LINE__), PrintUsage();
			if(!S_ISCHR(lbuf.st_mode))						printf("(%s %d) DBG\n", __FILE__, __LINE__), PrintUsage();
			strcpy(ttyS, optarg);
			break;

		case 'B':
		case 'b':
			len = strlen(optarg);
			if (len < 4)									printf("(%s %d) DBG\n", __FILE__, __LINE__), PrintUsage();
			if ('0' == optarg[0])							printf("(%s %d) DBG optarg[0]=0X%02X\n", __FILE__, __LINE__, optarg[0]), PrintUsage();
			for (i = 0; i < len; i++)
			{
				c = optarg[i];
				if (c < '0' || c > '9')						printf("(%s %d) DBG\n", __FILE__, __LINE__), PrintUsage();
			}
			baud = atoi(optarg);
			if (
				baud != 230400
				&& baud != 115200
				&& baud != 57600
				&& baud != 38400
				&& baud != 19200
				&& baud != 9600
				&& baud != 4800
				&& baud != 2400
				&& baud != 1800
				&& baud != 1200
			)											printf("(%s %d) DBG\n", __FILE__, __LINE__), PrintUsage();
			break;
		case 'h':
		case 'H':
			L = len = strlen(optarg);
			if (len < 2)									printf("(%s %d) DBG\n", __FILE__, __LINE__), PrintUsage();
			if (2 != (len % 3))
			strcpy(tmp, optarg);
			for (i = 0; i < len; i++)
			{
				c = optarg[i];
				tmp[i] = c;
				if (2 == (i % 3))
				{
					if (c != '-' )							printf("(%s %d) DBG\n", __FILE__, __LINE__), PrintUsage();
					tmp[i] = 0;
				}
				else
				{
					if (
						c != '0' && c != '1' && c != '2' && c != '3' && c != '4' && c != '5' && c != '6' && c != '7' && c != '8' && c != '9'
						&& c != 'a' && c != 'b' && c != 'c' && c != 'd' && c != 'e' && c != 'f'
						&& c != 'A' && c != 'B' && c != 'C' && c != 'D' && c != 'E' && c != 'F'
					)
					{
						printf("(%s %d) DBG\n", __FILE__, __LINE__), PrintUsage();
					}
				}
				//if (i == len -1)				printf("%02X\n", tmp[i]);
				//else							printf("%02X-", tmp[i]);
			}

			len /= 3;
			len++;
			for (i = 0; i < len; i++)
			{
				// res = sscanf(optarg, "%X", &txopts);
				res = sscanf(tmp+3*i, "%X", &X);
				if (-1 == res)								printf("(%s %d) DBG i=%d len=%d S=%s ARG=%s\n", __FILE__, __LINE__, i, len, tmp+3*i, optarg), PrintUsage();
				hex[i] = (unsigned char)(X & 0XFF);
			}
			break;
		}
	}


	if (0 > (ttyfd = open(ttyS, O_RDWR | O_NOCTTY | O_NDELAY)))
		printf("(%s %d) SERIAL DEVICE( \"/dev/tty[M|S|USB][0-9]\") OPEN() fail, please run as root and retry later, EXIT \n", __FILE__, __LINE__), exit(1);
	if (!isatty(ttyfd))
		printf("(%s %d) DEVICE( \"%s\"), ttyfd=%d, not ISATTY(), EXIT \n", __FILE__, __LINE__, ttyS, ttyfd), close(ttyfd), exit(1);

	memset(&term, 0, sizeof(struct termios));
	term.c_cflag |= B4800, term.c_cflag |= CLOCAL, term.c_cflag |= CREAD, term.c_cflag &= ~PARENB, term.c_cflag &= ~CSTOPB, term.c_cflag &= ~CSIZE, term.c_cflag |= CS8, term.c_iflag = IGNPAR, term.c_cc[VMIN] = 1, term.c_cc[VTIME] = 0;
	cfsetispeed(&term, B4800), cfsetospeed(&term, B4800);
	if (1200 == baud)	cfsetispeed(&term, B1200), cfsetospeed(&term, B1200);
	if (1800 == baud)	cfsetispeed(&term, B1800), cfsetospeed(&term, B1800);
	if (2400 == baud)	cfsetispeed(&term, B2400), cfsetospeed(&term, B2400);
	if (4800 == baud)	cfsetispeed(&term, B4800), cfsetospeed(&term, B4800);
	if (9600 == baud)	cfsetispeed(&term, B9600), cfsetospeed(&term, B9600);
	if (19200 == baud)	cfsetispeed(&term, B19200), cfsetospeed(&term, B19200);
	if (38400 == baud)	cfsetispeed(&term, B38400), cfsetospeed(&term, B38400);
	if (57600 == baud)	cfsetispeed(&term, B57600), cfsetospeed(&term, B57600);
	if (115200 == baud)	cfsetispeed(&term, B115200), cfsetospeed(&term, B115200);
	if (230400 == baud)	cfsetispeed(&term, B230400), cfsetospeed(&term, B230400);
	tcsetattr(ttyfd, TCSANOW, &term);
	if (0 > (v = fcntl(ttyfd, F_GETFL, 0)))						printf("(%s %d) FCNTL() F_GETFL(%d,%s) fail, EXIT \n", __FILE__, __LINE__, ttyfd, ttyS), close(ttyfd), exit(1);
	if (0 > fcntl(ttyfd, F_SETFL, v | O_NONBLOCK))				printf("(%s %d) FCNTL() F_SETFL,O_NONBLOCK(%d,%s) fail, EXIT \n", __FILE__, __LINE__, ttyfd, ttyS), close(ttyfd), exit(1);
	tcflush(ttyfd, TCIFLUSH);


	L = (L / 3) + 1;
	left = L;
	p = hex;
	while (left > 0)
	{
		l = write(ttyfd, p, left);
		if (0 > l)											printf("TTYFD WRITE FAIL, %s \n", ttyS), close(ttyfd), exit(1);
		if (0 == l)										break;
		p += l, left -= l;
	}

	printf("> ");
	for (i = 0; i < L; i++)
	{
		if (i != L - 1)										{ do { printf("%02X-", hex[i]); }while(0); }
		else												{ do { printf("%02X", hex[i]); }while(0); }
	}
	printf("\n");

	FD_ZERO(&rset);
	FD_SET(ttyfd, &rset);
	res = select(ttyfd + 1, &rset, 0, 0, &tv);
	if (res < 0)											printf("(%s %d) SELECT() FAIL !\n", __FILE__, __LINE__), close(ttyfd), exit(1);
	if (res == 0)											{ printf("(%s %d) SELECT() TIMEOUT !\n", __FILE__, __LINE__), close(ttyfd), exit(1); }
	if ((len = read(ttyfd, rcv, sizeof(rcv))) < 0)				printf("(%s %d) READ() FAIL !\n", __FILE__, __LINE__), close(ttyfd), exit(1);
	printf("< ");
	for (i = 0; i < len; i++)
	{
		if (i != len - 1)									do { printf("%02X-", rcv[i]); }while(0);
		else												do { printf("%02X", rcv[i]); }while(0);
	}
	printf("\n");

	close(ttyfd);
	return 0;
}




