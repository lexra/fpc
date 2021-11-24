
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

#include "list.h"


#if !defined(MSG_NOSIGNAL)
 #define MSG_NOSIGNAL									0
#endif // MSG_NOSIGNAL



//////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////

#define PT0_SOF_0							0
#define PT0_SOF_1							1
#define PT0_LEN_H							2
#define PT0_LEN_L							3
#define PT0_RESERVE							4
#define PT0_D_RQ_RP							5
#define PT0_D_ID_H							6
#define PT0_D_ID_L							7
#define PT0_DATA_START						8

#define RM6T3_CTRL_CODE_REQ				0xf6
#define RM6T3_CTRL_CODE_ANS				0xf1
#define RM6T3_CTRL_CODE_ACK				0xf2
#define RM6T3_CTRL_CODE_NAK				0xf3
#define RM6T3_CTRL_CODE_END				0xf4
#define RM6T3_CTRL_CODE_SPL				0xf7
#define RM6T3_CTRL_CODE_SPH				0x0f


//////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////

#define BREAK_UINT32(var, ByteNum) \
          (unsigned char)((unsigned int)(((var) >>((ByteNum) * 8)) & 0x00FF))

#define BUILD_UINT16(loByte, hiByte) \
          ((unsigned short)(((loByte) & 0x00FF) + (((hiByte) & 0x00FF) << 8)))

#define BUILD_UINT32(Byte0, Byte1, Byte2, Byte3) \
          ((unsigned int)((unsigned int)((Byte0) & 0x00FF) + ((unsigned int)((Byte1) & 0x00FF) << 8) \
			+ ((unsigned int)((Byte2) & 0x00FF) << 16) + ((unsigned int)((Byte3) & 0x00FF) << 24)))

#define HI_UINT16(a) (((a) >> 8) & 0xFF)
#define LO_UINT16(a) ((a) & 0xFF)


//////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////

#define RM6T3_CONST_RATIO					2.0959443923478260869565217391306F

// 前滾輪+跑帶厚度後的滾輪直徑 (mm)
//static float rm6t3_wheel_diameter = 80.00F;
static float rm6t3_wheel_diameter = (90.00F+3.00F*2.00F);

// 齒輪比, 齒輪1 帶動齒輪2, 齒輪比=齒輪2/齒輪1
static float rm6t3_gear_ratio = (115.00F/40.00F);

// 齒輪比愈大, 跑帶速度愈小
// 齒輪比愈大, 每km/h, 變頻器設定輸出頻率愈大
// 前滾輪+跑帶厚度後的滾輪直徑愈大, 變頻器設定輸出頻率愈小

//單位 km/hr 所需輸出之變頻器頻率, 5.55HZ
static float rm6t3_freq_unit = RM6T3_CONST_RATIO * rm6t3_gear_ratio * 1000000.00F / (60.00F * 3.1416F * rm6t3_wheel_diameter) / 60.00F;


//////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////

static int speed = 51;
static int freq = (int)(rm6t3_freq_unit * 100.00F * (float)speed / 10.00F);



//////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////

int CONNECT(char *domain, int port)
{
	int sock_fd;
	struct hostent *site;
	struct sockaddr_in me;
	int v;
 
	site = gethostbyname(domain);
	if (0 == site)
		printf("(%s %d) GETHOSTBYNAME() FAIL !\n", __FILE__, __LINE__), exit(1);
	if (0 >= site->h_length)
		printf("(%s %d) 0 >= site->h_length \n", __FILE__, __LINE__), exit(1);

	if (0 > (sock_fd = socket(AF_INET, SOCK_STREAM, 0)))
		printf("(%s %d) SOCKET() FAIL !\n", __FILE__, __LINE__), exit(1);

	v = 1;
	if (-1 == setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, (const void *)&v, sizeof(v)))
		printf("(%s %d) SETSOCKOPT() FAIL !\n", __FILE__, __LINE__), exit(1);

	if (0 == memset(&me, 0, sizeof(struct sockaddr_in)))
		printf("(%s %d) MEMSET() FAIL !\n", __FILE__, __LINE__), exit(1);

	memcpy(&me.sin_addr, site->h_addr_list[0], site->h_length);
	me.sin_family = AF_INET;
	me.sin_port = htons(port);

	return (0 > connect(sock_fd, (struct sockaddr *)&me, sizeof(struct sockaddr))) ? -1 : sock_fd;
}


//////////////////////////////////////////////////////////////////////
// routines
//////////////////////////////////////////////////////////////////////

static unsigned int calccrc(unsigned char crcbuf, unsigned int crc)
{
	unsigned char i = 0;

	crc ^= crcbuf;
	for(i = 0; i < 8; i++)
	{
		unsigned char chk;

		chk = crc & 1;
		crc >>= 1;
		crc &= 0X7FFF;
		if (1 == chk)
			crc ^= 0XA001;
		crc &= 0XFFFF;
	}

	return crc;
}

unsigned int ModbusCrc16(unsigned char *buf, unsigned short len)
{
	unsigned char hi = 0, lo = 0;
	unsigned int i = 0;
	unsigned int crc = 0xFFFF;

	for (i = 0; i < len; i++)
	{
		crc = calccrc(*buf, crc);
		buf++;
	}
	hi = crc & 0XFF;
	lo = crc >> 8;
	crc = (hi << 8) | lo;

	return crc;
}

static int pack_ui_frame(unsigned short did, unsigned char ack, unsigned short len, const unsigned char *in, unsigned char *out)
{
	unsigned short i;
	unsigned short crc = 0;

	if (0 == out)
		return 0;
	for (i = 0; i < len + 10; i++)
		*(out + i) = 0;
	out[PT0_SOF_0] = 0XFE; 
	out[PT0_SOF_1] = 0XFE;
	out[PT0_LEN_H] = HI_UINT16(len);
	out[PT0_LEN_L] = LO_UINT16(len);
	out[PT0_RESERVE] = 0;
	out[PT0_D_RQ_RP] = ack;
	out[PT0_D_ID_H] = HI_UINT16(did), out[PT0_D_ID_L] = LO_UINT16(did);
	for (i = 0; i < len; i++)
		*(out + PT0_DATA_START + i) = *(in + i);
	crc = ModbusCrc16(out, len + 8);
	out[8 + len] = HI_UINT16(crc), out[9 + len] = LO_UINT16(crc);
	return (len + 10);
}

static int JkSpeedSet(int fd, unsigned int value)
{
	int i = 0;
	unsigned char buffer[64];
	int len = -1;
	unsigned char data[] = {0,0,0,0};

	if (-1 == fd)			return -1;
	data[0] = BREAK_UINT32(value, 3);
	data[1] = BREAK_UINT32(value, 2);
	data[2] = BREAK_UINT32(value, 1);
	data[3] = BREAK_UINT32(value, 0);
	len = pack_ui_frame(0X009B, 0, 4, (unsigned char *)&data, buffer);
	if(len)				write(fd, (const void*)buffer, len);

	for (i = 0; i < len; i++)
	{
		if (i == len - 1)	printf("%02X\n", buffer[i]);
		else				printf("%02X-", buffer[i]);
	}
	printf(">>SpeedSet (0X009B): [FREQ]=%u (0.01HZ), [SPEED]=%.02f (KM/H)\n", freq, (float)speed / 10.00F);
	return len;
}

void PrintUsage(void)
{
	char file[128];
	int i = 0;

	memset(file, 0, sizeof(file));
	strcpy(file, __FILE__);
	for (; i <128; i++)
		if ('.' == file[i])	file[i] = 0;

	printf("Usage: %s -H [host] -P [connect port] -V [SPEED,0.1KM/H,0~300]\n", file);
	printf("Example: %s -H localhost -P 34000 -V 32\n", file), exit(1);
}

int main(int argc, char *argv[])
{
	time_t first;
	time_t now;
	unsigned char *p = 0;
	unsigned int val = 0;
	int t = 2;
	int sd = -1;
	int v;
	int left = 0;
	unsigned short cmd;

	char host[128];
	int port = 34000;

	int len;

	int res;
	int state = 0;

	fd_set rset;
	struct timeval tv;
	int c;
	unsigned char ch;
	unsigned char rb[4096];


//////////////////////////////////////////////////////////////////////
	setbuf(stdout, 0);
	strcpy(host, "localhost");
	if (argc < 3)
		PrintUsage();

//////////////////////////////////////////////////////////////////////
	while ((c = getopt(argc, argv, "H:h:p:P:v:V:")) != -1)
	{
		switch (c)
		{
		default:
			break;
		case '?':
			PrintUsage();
			break;
		case 'h':
		case 'H':
			strcpy(host, optarg);
			break;
		case 'P':
		case 'p':
			port = atoi(optarg);
			break;

		case 'v':
		case 'V':
			speed = atoi(optarg);
			if (speed > 360)
				speed = 360;
			freq = (int)(rm6t3_freq_unit * 100.00F * (float)speed / 10.00F);
			break;
		}
	}


//////////////////////////////////////////////////////////////////////
	if (0 > (sd = CONNECT(host, port)))
		printf("TCP CONNECT TO (%s %d) fail !\n", host, port), exit(1);
	v = 1024 * 1024;
	if (0 > setsockopt(sd, SOL_SOCKET, SO_SNDBUF, (const void *)&v, sizeof(v)))
		printf("(%s %d) SO_SNDBUF fail !\n", __FILE__, __LINE__), close(sd), exit(1);
	if (0 > setsockopt(sd, SOL_SOCKET, SO_RCVBUF, (const void *)&v, sizeof(v)))
		printf("(%s %d) SO_RCVBUF fail !\n", __FILE__, __LINE__), close(sd), exit(1);
	if (0 > (v = fcntl(sd, F_GETFL, 0)))
		printf("(%s %d) FCNTL() fail, F_GETFL, EXIT \n", __FILE__, __LINE__), close(sd), exit(1);
	if (0 > fcntl(sd, F_SETFL, v | O_NONBLOCK))
		printf("(%s %d) FCNTL() F_SETFL fail, O_NONBLOCK, EXIT \n", __FILE__, __LINE__), close(sd), exit(1);
	JkSpeedSet(sd, speed);

//////////////////////////////////////////////////////////////////////
	time(&first);
	state = 0;
	while(1)
	{
		time(&now);
		if ((int)(now - first) > t)
		{
			printf("\nTIMEOUT (0X%04X) !\n\n", 0X009B);
			close(sd), exit(1);
		}

		FD_ZERO(&rset);
		FD_SET(sd, &rset);
		memset(&tv, 0, sizeof(tv)), tv.tv_sec = 1;
		res = select(sd + 1, &rset, 0, 0, &tv);
		if (res == 0)			continue;
		if (res < 0)			printf("(%s %d) SELECT() fail !\n", __FILE__, __LINE__), close(sd), exit(1);

		if (0 == state)
		{
			len = read(sd, &ch, 1);
			if (len <= 0)							printf("(%s %d) READ() FAIL !\n", __FILE__, __LINE__), close(sd), exit(1);
			if (0XFE == ch)
			{
				state = 1;
				rb[0] = ch;
				continue;
			}
			state = 0;
			printf("x");
			continue;
		}
		if (1 == state)
		{
			len = read(sd, &ch, 1);
			if (len <= 0)							printf("(%s %d) READ() FAIL !\n", __FILE__, __LINE__), close(sd), exit(1);
			if (0XFE == ch)
			{
				state = 2;
				rb[1] = ch;
				continue;
			}
			state = 0;
			printf("x");
			continue;
		}
		if (2 == state)
		{
			len = read(sd, &ch, 1);
			if (len <= 0)							printf("(%s %d) READ() FAIL !\n", __FILE__, __LINE__), close(sd), exit(1);
			state = 3;
			rb[2] = ch;
			continue;
		}
		if (3 == state)
		{
			len = read(sd, &ch, 1);
			if (len <= 0)							printf("(%s %d) READ() FAIL !\n", __FILE__, __LINE__), close(sd), exit(1);
			state = 4;
			rb[3] = ch;
			left = 6 + BUILD_UINT16(rb[3], rb[2]);
			if (left > 64)
			{
				state = 0;
				printf("x");
				continue;
			}
			p = &rb[4];
			continue;
		}
		if (4 == state)
		{
			unsigned int chk = 0;
			unsigned int crc = 0;

			len = read(sd, (char *)p, left);
			if (len <= 0)							printf("(%s %d) READ() FAIL !\n", __FILE__, __LINE__), close(sd), exit(1);
			p += len;
			left -= len;
			if (left > 0)							continue;
			state = 0;
			len = BUILD_UINT16(rb[3], rb[2]);
			chk = BUILD_UINT16(rb[len + 9], rb[len + 8]);
			crc = ModbusCrc16(rb, len + 8);
			if (chk != crc)
			{
				printf("\n");
				printf("(%s %d) CRC16 ERROR !\n", __FILE__, __LINE__);
				continue;
			}
		}

//////////////////////////////////////////////////////////////////////
		if (0 == rb[5])
		{
			printf("x");
			continue;
		}
		len = BUILD_UINT16(rb[PT0_LEN_L], rb[PT0_LEN_H]);
		cmd = BUILD_UINT16(rb[PT0_D_ID_L], rb[PT0_D_ID_H]);
		if (cmd == 0X009B)
		{
			val = BUILD_UINT32(rb[PT0_DATA_START + 3], rb[PT0_DATA_START + 2], rb[PT0_DATA_START + 1], rb[PT0_DATA_START]);
			//printf("<<FreqRsp (0X0091): [FREQ]=%u(0.01HZ), [SPEED]=%.02f(KM/H)\n", freq, (float)val / (rm6t3_freq_unit * 100.00F));
			printf("<<FreqRsp (0X009B): [FREQ]=%u(0.01HZ), [SPEED]=%.02f(KM/H)\n", freq, (float)val / 10.00F);
			break;
		}
		printf(".");
	}

	printf("\n");
	close(sd);
	return 0;
}



