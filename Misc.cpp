// Misc.cpp : Defines the class behaviors for the application.
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
#include <time.h>
#include <dirent.h>
#include <sys/mount.h>
#include <pthread.h>
#include <signal.h>
#include <pthread.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <termios.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#include "Misc.h"
#include "Event.h"


//////////////////////////////////////////////////////////////////////
// 
//////////////////////////////////////////////////////////////////////

int SetBaudRate(int fd, int baudrate)
{
	struct termios term;

	if (-1 == fd)
		return 0;
	if (baudrate != 9600 && baudrate != 19200 && baudrate != 38400 && baudrate != 57600 && baudrate != 115200 && baudrate != 4800)
		baudrate = 9600;
	if (!isatty(fd))
		return 0;
	memset(&term, 0, sizeof(term));
	term.c_cflag |= B9600, 	term.c_cflag |= CLOCAL, term.c_cflag |= CREAD, term.c_cflag &= ~PARENB, term.c_cflag &= ~CSTOPB;
	term.c_cflag &= ~CSIZE, term.c_cflag |= CS8, term.c_iflag = IGNPAR, term.c_cc[VMIN] = 1, term.c_cc[VTIME] = 0;
	if (115200 == baudrate)											cfsetispeed(&term, B115200), cfsetospeed(&term, B115200);
	else if (57600 == baudrate)										cfsetispeed(&term, B57600), cfsetospeed(&term, B57600);
	else if (38400 == baudrate)										cfsetispeed(&term, B38400), cfsetospeed(&term, B38400);
	else if (19200 == baudrate)										cfsetispeed(&term, B19200), cfsetospeed(&term, B19200);
	else if (9600 == baudrate)											cfsetispeed(&term, B9600), cfsetospeed(&term, B9600);
	else if (4800 == baudrate)											cfsetispeed(&term, B4800), cfsetospeed(&term, B4800);
	else																cfsetispeed(&term, B9600), cfsetospeed(&term, B9600);
	tcsetattr(fd, TCSANOW, &term); tcflush(fd, TCIFLUSH);
	return 1;
}

int CONN(const char *domain, int port)
{
	int sock_fd;
	struct hostent *site;
	struct sockaddr_in me;
	int v;
	int res;
 
	site = gethostbyname(domain);
	if (0 == site)														{ dbg_printf(DBG_SOCKET, "(%s %d) GETHOSTBYNAME() fail !\n", __FILE__, __LINE__); return -1; }
	if (0 >= site->h_length)											{ dbg_printf(DBG_SOCKET, "(%s %d) 0 >= site->h_length \n", __FILE__, __LINE__); return -1; }
	if (0 > (sock_fd = socket(AF_INET, SOCK_STREAM, 0)))					{ dbg_printf(DBG_SOCKET, "(%s %d) SOCKET() fail !\n", __FILE__, __LINE__); return -1; }

	v = 1;
	if (-1 == setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, (const void *)&v, sizeof(v)))	
		{ dbg_printf(DBG_SOCKET, "(%s %d) SETSOCKOPT() fail !\n", __FILE__, __LINE__); return -1; }
	memset(&me, 0, sizeof(struct sockaddr_in));
	memcpy(&me.sin_addr, site->h_addr_list[0], site->h_length);
	me.sin_family = AF_INET;
	me.sin_port = htons(port);
	res = connect(sock_fd, (struct sockaddr *)&me, sizeof(struct sockaddr));
	if (res < 0)
	{
		close(sock_fd);
		return res;
	}
	return sock_fd;
}

int search_lf(const unsigned char *str, int len)
{
	int i;

	for (i = 0; i < len; i++)
	{
		if(str[i] > 0X0A)
		{
			return i;
		}
	}
	return -1;
}

int search_msb_on(const unsigned char *str, int len)
{
	int i;

	for (i = 0; i < len; i++)
	{
		if(str[i] > 127)
		{
			return i;
		}
	}
	return -1;
}

int search_ctrld(const unsigned char *str, int len)
{
	int i;

	for (i = 1; i < len; i++)
	{
		if(str[i] == 0XEC)
		{
			if (str[i -1] == 0XFF)
				return (i - 1);

		}
	}
	return -1;
}


#ifdef XXXX
static int set_i2c_register(int file,
                            unsigned char addr,
                            unsigned char reg,
                            unsigned char value) {

    unsigned char outbuf[2];
    struct i2c_rdwr_ioctl_data packets;
    struct i2c_msg messages[1];

    messages[0].addr  = addr;
    messages[0].flags = 0;
    messages[0].len   = sizeof(outbuf);
    messages[0].buf   = outbuf;

    /* The first byte indicates which register we'll write */
    outbuf[0] = reg;

    /* 
     * The second byte indicates the value to write.  Note that for many
     * devices, we can write multiple, sequential registers at once by
     * simply making outbuf bigger.
     */
    outbuf[1] = value;

    /* Transfer the i2c packets to the kernel and verify it worked */
    packets.msgs  = messages;
    packets.nmsgs = 1;
    if(ioctl(file, I2C_RDWR, &packets) < 0) {
        perror("Unable to send data");
        return 1;
    }

    return 0;
}


static int get_i2c_register(int file,
                            unsigned char addr,
                            unsigned char reg,
                            unsigned char *val) {
    unsigned char inbuf, outbuf;
    struct i2c_rdwr_ioctl_data packets;
    struct i2c_msg messages[2];

    /*
     * In order to read a register, we first do a "dummy write" by writing
     * 0 bytes to the register we want to read from.  This is similar to
     * the packet in set_i2c_register, except it's 1 byte rather than 2.
     */
    outbuf = reg;
    messages[0].addr  = addr;
    messages[0].flags = 0;
    messages[0].len   = sizeof(outbuf);
    messages[0].buf   = &outbuf;

    /* The data will get returned in this structure */
    messages[1].addr  = addr;
    messages[1].flags = I2C_M_RD/* | I2C_M_NOSTART*/;
    messages[1].len   = sizeof(inbuf);
    messages[1].buf   = &inbuf;

    /* Send the request to the kernel and get the result back */
    packets.msgs      = messages;
    packets.nmsgs     = 2;
    if(ioctl(file, I2C_RDWR, &packets) < 0) {
        perror("Unable to send data");
        return 1;
    }
    *val = inbuf;

    return 0;
}
#endif


#if 1
int i2c_write_data(int fd_i2c, int bus_adr, int sub_adr, const unsigned char *data, int length)
{
	int res = -1;
	unsigned char buf[512];
	struct i2c_msg msg;
	struct i2c_rdwr_ioctl_data queue;

	msg.addr = bus_adr;
	msg.flags = 0;
	msg.len  = (length + 1);
	msg.buf = buf;
	buf[0] = (unsigned char)sub_adr;
	if (data && length > 0)
		memcpy(&buf[1], data, length);

	queue.msgs = &msg;
	queue.nmsgs = 1;

	res = ioctl(fd_i2c, I2C_RDWR, &queue);
	if (-1 == res)
	{
		dbg_printf(DBG_I2C, "I2C (0X%02X) ERR 0\n", bus_adr);
		usleep(11);
	}
	return res;
}

int i2c_read_data(int fd_i2c, int bus_adr, int sub_adr, unsigned char *data, int length)
{
	int res = -1;
	unsigned char buf[512];
	struct i2c_msg  msg[2];
	struct i2c_rdwr_ioctl_data queue;

	buf[0] = (unsigned char)sub_adr;
	msg[0].addr  = bus_adr;
	msg[0].flags = 0;
	msg[0].len   = 1;
	msg[0].buf   = &buf[0];

	msg[1].addr  = bus_adr;
	msg[1].flags = I2C_M_RD /*| I2C_M_NOSTART*/;
	msg[1].len   = length;
	msg[1].buf   = &buf[1];

	queue.msgs = msg;
	queue.nmsgs = 2;

	res = ioctl(fd_i2c, I2C_RDWR, &queue);
	if (-1 == res)
	{
		dbg_printf(DBG_I2C, "I2C (0X%02X) ERR 1\n", bus_adr);
		usleep(11);
		return -1;
	}
	if (data && length > 0)
		memcpy(data, &buf[1], length);
	return res;
}
#else
int i2c_write_data(int fd_i2c, int bus_adr, int sub_adr, const unsigned char *data, int length)
{
	int res = -1;
	unsigned char buf[256];

	res = ioctl(fd_i2c, I2C_SLAVE_FORCE, bus_adr);
	if(res < 0)
	{
		dbg_printf(DBG_I2C, "I2C SET SLAVE (0X%02X) ERR\n", sub_adr);
		return -1;
	}
	buf[0] = (unsigned char)sub_adr;
	if (data && length > 0)
		memcpy(&buf[1], data, length);
	if(write(fd_i2c, buf, length + 1) != (length + 1))
	{
		dbg_printf(DBG_I2C, "I2C: write data error %x\n", sub_adr);
		return -1;
	}
	return length;
}

int i2c_read_data(int fd_i2c, int bus_adr, int sub_adr, unsigned char *data, int length)
{
	int res = -1;

	res = ioctl(fd_i2c, I2C_SLAVE_FORCE, bus_adr);
	if(res < 0)
	{
		dbg_printf(DBG_I2C, "I2C SET SLAVE (0X%02X) ERR\n", bus_adr);
		return -1;
	}
	if(write(fd_i2c, &sub_adr, 1) != 1)
	{
		dbg_printf(DBG_I2C, "I2C WRITE DATA ERR \n");
		return -1;
	}
	res = read(fd_i2c, data, length);
	return res;
}
#endif


void CleanupHandle(void *param)
{
	CloseEvent((HANDLE)param);
}

void CleanupRegx(void *param)
{
	regfree((regex_t *)param);
}

void CleanupFd(void *param)
{
	close(*(int *)param);
}

void CleanupLock(void *param)
{
	//int last_type;

	//pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &last_type);

	pthread_mutex_unlock((pthread_mutex_t *)param);

	//pthread_testcancel();
	//pthread_setcanceltype(last_type, NULL);
}

void RestoreSigmask(void *param)
{
	pthread_sigmask(SIG_SETMASK, (sigset_t *)param, NULL);
}

void TimespecAdd(const struct timespec* a, const struct timespec* b, struct timespec* out)
{
	time_t sec = a->tv_sec + b->tv_sec;
	long nsec = a->tv_nsec + b->tv_nsec;

	sec += nsec / 1000000000L;
	nsec = nsec % 1000000000L;
	out->tv_sec = sec;
	out->tv_nsec = nsec;
}

int IsLittleEndian(void)
{
	union _dword
	{
		unsigned int all;
		struct _bytes
		{
			unsigned char byte0;
			unsigned char pad[3]; 
		}bytes;

	}dw;

	dw.all = 0x87654321;
	return (0x21 == dw.bytes.byte0);
}

char *StripCrLf(char *line)
{
	int len;

	if(line == 0)
		return 0;

	len = strlen(line);
	if(len > 1000)
		return line;
	if(len <= 0)
		return line;

	if (line[len - 1] == '\n' || line[len - 1] == '\r')
		line[len - 1] = 0;
	len = strlen(line);
	if (len > 0)
		if (line[len - 1] == '\n' || line[len - 1] == '\r')
			line[len - 1] = 0;
	return line;
}

char *FileExtension(const char *path)
{
	int i, j, len;
	char szTmp[1024];
	static char ext[256];
	char *name;

	if(!path)
		return 0;

	name = FileName(path);
	if(!name)
		return 0;

	strcpy(szTmp, name);

	j = -1;
	len = strlen(szTmp);
	if(len < 0 || len >= 255)
		return 0;

	for (i = 0; i < len; i++)
	{
		if(szTmp[i] == '.')
			j = i;
	}

	if (j == -1 || j == 0)
	{
		ext[0] = 0;
		return ext;
	}

	strcpy(ext, &szTmp[j + 1]);
	return ext;
}

char *FileMaster(const char *path)
{
	int i, j, len;
	char szTmp[1024];
	static char master[256];
	char *name;

	if(!path)
		return 0;

	name = FileName(path);
	if(!name)
		return 0;

	strcpy(szTmp, name);

	j = -1;
	len = strlen(szTmp);
	if(len < 0 || len >= 255)
		return 0;

	for (i = 0; i < len; i++)
	{
		if(szTmp[i] == '.')
			j = i;
	}

	if (j != -1 && j != 0)
	{
		szTmp[j] = 0;
	}
	strcpy(master, szTmp);

	return master;
}

char *FileName(const char *path)
{
	int i, j, len;
	char szTmp[1024];
	static char name[256];

	if(!path)
		return 0;

	strcpy(szTmp, path);
	len = strlen(szTmp);
	if(len <= 0 || len >= 255)
		return 0;

	if(szTmp[len - 1] == '/')
	{
		szTmp[len - 1] = 0;
		len = strlen(szTmp);
	}

	j = -1;
	for (i = 0; i < len; i++)
	{
		if(szTmp[i] == '/')
			j = i;
	}

	strcpy(name, &szTmp[j + 1]);
	return name;
}

int tstrnlen(const char *s, int maxlen)
{
	int i = 0;

	if (maxlen <= 0)
		return 0;

	for (; i < maxlen; )
	{
		if(s[i] == 0)
			break;

		if((unsigned char)(s[i]) > 0x80)
		{
			if(i + 2 > maxlen)
				break;
			i += 2;
		}
		else
			i++;
	}

	return i;
}

void ReverseBytes(unsigned char *pData, unsigned char len)
{
	unsigned char i, j, temp;

	for (i = 0, j = len - 1; len > 1; len -= 2)
	{
		temp = pData[i];
		pData[i++] = pData[j];
		pData[j--] = temp;
	}
}

void GetLocalTime(SYSTEMTIME *st)
{
	time_t now;
	struct tm *pst = NULL;

	time(&now);
	pst = localtime(&now);
	memcpy(st, pst, sizeof(SYSTEMTIME));
	//st->tm_year += 1900;
//	GetLocalTime(&st);
//	printf("[%04d-%02d-%02d %02d:%02d:%02d %010u] ", st.tm_year + 1900, st.tm_mon + 1, st.tm_mday, st.tm_hour, st.tm_min, st.tm_sec, (unsigned int)mktime(&st));
}

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

unsigned char CalcFcs(unsigned char *msg_ptr, unsigned char len)
{
	unsigned char x;
	unsigned char xorResult;

	xorResult = 0;

	for (x = 0; x < len; x++, msg_ptr++)
		xorResult = xorResult ^ (*msg_ptr);

	return xorResult;
}

#if 0
void PresetModbusRegister(unsigned char slaveAddr, unsigned char fnCode, unsigned short reg, unsigned short data)
{
	unsigned char *query = 0;

	query = osal_mem_alloc(10);
	if (!query)	return;
	osal_memset(query, 0, 10);
	query[0] = slaveAddr;
	query[1] = fnCode;
	query[2] = (unsigned char)(reg >> 8);
	query[3] = (unsigned char)(reg & 0XFF);
	query[4] = (unsigned char)(data >> 8);
	query[5] = (unsigned char)(data & 0XFF);
	query[6] = (unsigned char)(crc16(query, 6) >> 8);
	query[7] = (unsigned char)(crc16(query, 6) & 0XFF);
	HalUARTWrite(ZTOOL_PORT, query, 8);
	osal_mem_free(query);
}

void QueryModbusRegister(unsigned char slaveAddr, unsigned char fnCode, unsigned short reg, unsigned short num)
{
	unsigned char *query;

	query = osal_mem_alloc(10);
	if (!query)						return;
	osal_memset(query, 0, 10);
	query[0] = slaveAddr;
	query[1] = fnCode;
	query[2] = (unsigned char)(reg >> 8);
	query[3] = (unsigned char)(reg & 0XFF);
	query[4] = (unsigned char)(num >> 8);
	query[5] = (unsigned char)(num & 0XFF);
	query[6] = (unsigned char)(crc16(query, 6) >> 8);
	query[7] = (unsigned char)(crc16(query, 6) & 0XFF);
	HalUARTWrite(ZTOOL_PORT, query, 8);
	osal_mem_free(query);
}
#endif

void Average2(unsigned short av1,unsigned short t1,unsigned short av2,unsigned short t2, volatile unsigned short * target)
{
	double a,b;
	a = (double)(av2*t2-av1*t1);
	b = (double)(t2-t1);
	(*target) = (unsigned short)(a/b);
}


int fsize(FILE *fp)
{
    int prev=ftell(fp);
    fseek(fp, 0L, SEEK_END);
    int sz=ftell(fp);
    fseek(fp,prev,SEEK_SET); //go back to where we were
    return sz;
}



