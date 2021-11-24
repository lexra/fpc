
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

#include <sys/time.h>
#include <sys/timeb.h>

 #include <assert.h>

#include "list.h"
#include "b64.h"


/////////////////////////////////////////////////////////////////////////////////////////
//

#define MS 	1000

#define BUILD_UINT16(loByte, hiByte) \
          ((unsigned short)(((loByte) & 0x00FF) + (((hiByte) & 0x00FF) << 8)))

#define BUILD_UINT32(Byte0, Byte1, Byte2, Byte3) \
          ((unsigned int)((unsigned int)((Byte0) & 0x00FF) + ((unsigned int)((Byte1) & 0x00FF) << 8) \
			+ ((unsigned int)((Byte2) & 0x00FF) << 16) + ((unsigned int)((Byte3) & 0x00FF) << 24)))

#define HI_UINT16(a) (((a) >> 8) & 0xFF)
#define LO_UINT16(a) ((a) & 0xFF)


#define WS_NEED_REQ						1
#define WS_NEED_RSP						2
#define WS_NEED_RSP_PENDING				3
#define WS_NEED_OP							4
#define WS_NEED_OP_PENDING				5
#define WS_NEED_LEN_0						6
#define WS_NEED_LEN_0_PENDING			7
#define WS_NEED_LEN_1						8
#define WS_NEED_LEN_1_PENDING			9
#define WS_NEED_LEN_2						10
#define WS_NEED_LEN_2_PENDING			11
#define WS_NEED_MASK						12
#define WS_NEED_MASK_PENDING				13
#define WS_NEED_PAYLOAD					14
#define WS_NEED_PAYLOAD_PENDING			15



/////////////////////////////////////////////////////////////////////////////////////////
//

static const char *wskey =
  "jhw783HGj2gh5j210d";
static const char *wsreq =
  "GET /chat HTTP/1.1\r\n"
  "Host: %s:%d\r\n"
  "Upgrade: websocket\r\n"
  "Connection: Upgrade\r\n"
  "Sec-WebSocket-Key: %s\r\n"
  "Sec-WebSocket-Version: 13\r\n\r\n";

static const char *wsrsp = 
	"HTTP/1.1 101 Switching Protocols\r\n"
	"Upgrade: WebSocket\r\n"
	"Connection: Upgrade\r\n"
	"Sec-WebSocket-Accept: lwoAexbelHJFigL+fqC2NdoLFFg=\r\n\r\n";


typedef struct tm SYSTEMTIME;

void GetLocalTime(SYSTEMTIME *st)
{
	struct tm *pst = NULL;
	time_t t = time(NULL);
	pst = localtime(&t);
	memcpy(st, pst, sizeof(SYSTEMTIME));
	st->tm_year += 1900;
}

static int CONNECT(char *domain, int port)
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


static void PrintUsage(void)
{
	char file[128];
	int i = 0;

	memset(file, 0, sizeof(file));
	strcpy(file, __FILE__);
	for (i = 0; i <128; i++)
		if ('.' == file[i])	file[i] = 0;

	printf("Usage: %s -h[host] -p[connect port]\n", file);
	printf("Example: %s -h192.168.3.30 -p34000\n", file), exit(1);
}

static int set_nonblocking(int fd, int nonblocking)
{
	int v;

	v = fcntl(fd, F_GETFL, 0);
	if (-1 == v)
		return -1;
	if (nonblocking)
		return fcntl(fd, F_SETFL, v & ~O_NONBLOCK);
	return fcntl(fd, F_SETFL, v | O_NONBLOCK);
}

static int parse_argument(int argc, char **argv, char *h, int *p)
{
	int c;

	if (0 == p || 0 == h)	return 0;
	while ((c = getopt(argc, argv, "H:h:p:P:")) != -1)
	{
		switch (c)
		{
		default:
			break;
		case '?':
			PrintUsage();
			return 1;
		case 'H':
		case 'h':
			strcpy(h, optarg);
			break;
		case 'P':
		case 'p':
			*p = atoi(optarg);
			break;
		}
	}
	return 0;
}


/*
0                   1                   2                   3
 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-------+-+-------------+-------------------------------+
|F|R|R|R| opcode|M| Payload len |    Extended payload length    |
|I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
|N|V|V|V|       |S|             |   (if payload len==126/127)   |
| |1|2|3|       |K|             |                               |
+-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
|     Extended payload length continued, if payload len == 127  |
+ - - - - - - - - - - - - - - - +-------------------------------+
|                               |Masking-key, if MASK set to 1  |
+-------------------------------+-------------------------------+
| Masking-key (continued)       |          Payload Data         |
+-------------------------------- - - - - - - - - - - - - - - - +
:                     Payload Data continued ...                :
+ - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
|                     Payload Data continued ...                |
+---------------------------------------------------------------+
*/

#define RX_SIZE		(1024 * 512)

//unsigned char rx[1024 * 1024 * 4];
static unsigned char *rx = 0;


int main(int argc, char *argv[])
{
	int sd = -1;
	char host[128];
	int port = 4321;
	int so_size = 1024 * 1024;
	char filename[128];
	int i;

	unsigned char *pr = rx;

	fd_set rset, wset;
	struct timeval tv;
	int res;
	int maxfd;

	ssize_t len;
	int state = WS_NEED_REQ;

	static int fin = 0;

	memset(&tv, 0, sizeof(tv));
	//tv.tv_sec = 1;
	tv.tv_usec = (1000*MS);

	memset(filename, 0, sizeof(filename));
	strcpy(filename, __FILE__);
	for (i = 0; i < (int)sizeof(filename); i++)
	{
		if ('.' == filename[i])
			filename[i] = 0;
	}

	setbuf(stdout, 0);
	strcpy(host, "localhost");
	if (argc < 3)
		PrintUsage();
	parse_argument(argc, argv, host, &port);

	if (0 > (sd = CONNECT(host, port)))
		printf("CONNECT (%s %d) fail !\n", host, port), exit(2);

	setsockopt(sd, SOL_SOCKET, SO_SNDBUF, (const void *)&so_size, sizeof(so_size));
	setsockopt(sd, SOL_SOCKET, SO_RCVBUF, (const void *)&so_size, sizeof(so_size));
	set_nonblocking(sd, 1);

	rx = (unsigned char *)malloc(RX_SIZE);
	for(;;)
	{

		maxfd = 0;
		FD_ZERO(&rset);
		FD_ZERO(&wset);
		FD_SET(sd, &rset);
		if (WS_NEED_REQ == state)
			FD_SET(sd, &wset);
		if (maxfd < sd)
			maxfd = sd;
		res = select(maxfd + 1, &rset, &wset, 0, &tv);
		if (res < 0)
		{
			printf ("SELECT FAIL\n");
			state = 0;
			break;
		}
		if (0 == res)
		{
			continue;
		}

		if (FD_ISSET(sd, &rset))
		{
			static unsigned char *payload = 0;
			static size_t left = 0;
			static size_t L = 0;
			static unsigned char M = 0;

			if (WS_NEED_RSP == state || WS_NEED_RSP_PENDING == state)
			{
				if (WS_NEED_RSP == state)
					left = 129, memset(pr = rx, 0, RX_SIZE), L = 0, state = WS_NEED_RSP_PENDING;
				len = read(sd, pr, left);
				if (0 >= len)
				{
					printf("DBG(%d) \n", __LINE__);
					break;
				}
				pr += len;
				left -= len;
				if (0 == left)
				{
					if (0 != memcmp(rx, wsrsp, 129))
						break;
					state = WS_NEED_OP;
				}
			}
			else if (WS_NEED_OP == state)
			{
				L = 0;
				memset(pr = rx, 0, RX_SIZE);
				len = read(sd, pr, 1);
				if (0 >= len)
				{
					printf("DBG(%d) \n", __LINE__);
					break;
				}

				if (0X82 == rx[0])
				{
					if (fin)
					{
						fin = 0;
						//printf(".");
					}
					state = WS_NEED_LEN_0;
					pr++;
				}
				else
				{
					if (fin)	fin = 0, printf("x");
					state = WS_NEED_OP;
				}
			}
			else if (WS_NEED_LEN_0 == state)
			{
				len = read(sd, pr, 1);
				if (0 >= len)
				{
					printf("DBG(%d) \n", __LINE__);
					break;
				}
				M = (rx[1] & 0X80) ? 1 : 0;
				if (M)
				{
					state = WS_NEED_OP;
					printf("s\n"); fin = 0;
				}
				else
				{
					if ((rx[1] & 0X7F) <= 125)
					{
						L = (rx[1] & 0X7F);
						if (M)	state = WS_NEED_MASK;
						else 	state = WS_NEED_PAYLOAD;
					}
					//else if ((rx[1] & 0X7F) == 126)
					else if (rx[1] == 0X7E)
					{
						state = WS_NEED_LEN_1;
					}
					//else if ((rx[1] & 0X7F) == 127)
					else if (rx[1] == 0X7F)
					{
						state = WS_NEED_LEN_2;
					}
					else
					{
						printf("\n????????xxx?????????????????????????????????????????");
						state = WS_NEED_OP;
					}
					pr++;
				}
			}
			else if (WS_NEED_LEN_1 == state || WS_NEED_LEN_1_PENDING == state)
			{
				if (WS_NEED_LEN_1 == state)
					left = 2, state = WS_NEED_LEN_1_PENDING;
				len = read(sd, pr, left);
				if (0 >= len)
				{
					printf("DBG(%d) \n", __LINE__);
					break;
				}
				pr += len;
				left -= len;
				if (0 == left)
				{
					L = (size_t)(BUILD_UINT16(rx[3], rx[2]));
					if (L)
					{
						if (M)	state = WS_NEED_MASK;
						else 	state = WS_NEED_PAYLOAD;
					}
					else
					{
						printf("\n?????????????????????????????????????????????????");
						state = WS_NEED_OP;
					}
				}
			}
			else if (WS_NEED_LEN_2 == state || WS_NEED_LEN_2_PENDING == state)
			{
				if (WS_NEED_LEN_2 == state)
					left = 8, state = WS_NEED_LEN_2_PENDING;
				len = read(sd, pr, left);
				if (0 >= len)
				{
					printf("DBG(%d) \n", __LINE__);
					break;
				}
				pr += len;
				left -= len;
				if (0 == left)
				{
					L = (size_t)(BUILD_UINT32(rx[9], rx[8], rx[7], rx[6]));
					if (L)
					{
						if (M)	state = WS_NEED_MASK;
						else 	state = WS_NEED_PAYLOAD;
					}
					else
					{
						printf("\n?????????????????????????????????????????????????");
						state = WS_NEED_OP;
					}
				}
			}
			else if (WS_NEED_MASK == state || WS_NEED_MASK_PENDING == state)
			{
				if (WS_NEED_MASK == state)
					left = 4, state = WS_NEED_MASK_PENDING;
				len = read(sd, pr, left);
				if (0 >= len)
				{
					printf("DBG(%d) \n", __LINE__);
					break;
				}
				pr += len;
				left -= len;
				if (0 == left)
				{
					state = WS_NEED_PAYLOAD;
				}
			}
			//else //if (WS_NEED_PAYLOAD == state)
			else if (WS_NEED_PAYLOAD == state || WS_NEED_PAYLOAD_PENDING == state)
			{
				if (WS_NEED_PAYLOAD == state)
					payload = pr, left = L, state = WS_NEED_PAYLOAD_PENDING;
				len = read(sd, pr, left);
				if (0 >= len)
				{
					printf("DBG(%d) \n", __LINE__);
					break;
				}
				pr += len;
				left -= len;
				if (0 == left)
				{
					/*if (0X42 == payload[0] && 0X00 == payload[1])
					{
						printf("A=%u\n", L);
					}
					else if (0X41 == payload[0] && 0X00 == payload[1] && 0X00 == payload[2] && 0X00 == payload[3] && 0X00 == payload[4] && 0X01 == payload[5])
					{
						printf("V=%u\n", L);
					}
					else if (0X43 == payload[0] && 0X62 == payload[1] && 0X01 == payload[2])
					{
						printf("F\n");
					}
					else
					{
						printf("payload[0]=0X%02X\n", payload[0]);
					}*/


					//printf("L=%d,%d M=%d\n", L, (int)(pr-rx), M);
					fin = 1;
					state = WS_NEED_OP;
				}
			}
		}
		if (FD_ISSET(sd, &wset))
		{
			char key[25];
			char buf[25];
			int j;

			// Encode client key
			for(j = 0; j < 6; ++j)
				encodeblock((unsigned char *)(wskey + j * 3),(unsigned char *)(key + j * 4),3);
			key[24] = '\0';
			sprintf(buf, wsreq, host, port, key);
			j = strlen(buf);
			len = write(sd, buf, j);
			if (0 > len)
			{
				printf ("CONNECT RESET BY PEERS\n");
				break;
			}
			//printf ("+\n");
			state = WS_NEED_RSP;
		}
	}

	state = WS_NEED_REQ;
	close(sd);
	if (rx)	free(rx);

	printf ("EXIT0\n");
	return 0;
}

#if 0
bool WebSocketSendVideo(Socket *S, char *data, int length)
{
  int I, J;
  char header[10];
  header[0] = 0x82;

  length = length + 2;
  char *video = data+2;
  video[0] = 0x41;
  video[1] = 0;

  if (length <= 125)
  {
    header[1] = length;
    I = S->Send(header, 2);
    if(I<=0)
    {
      return false;
    }
  }
  else if (length < 65536)
  {
    header[1] = 0x7E;		// 126
    header[2] = (length >> 8) & 0xFF;
    header[3] = length & 0xFF;
    I = S->Send(header, 4);
    if(I<=0)
    {
      return false;
    }
  }
  else
  {
    header[1] = 0x7F;
    header[2] = 0;
    header[3] = 0;
    header[4] = 0;
    header[5] = 0;
    header[6] = (length >> 24) & 0xFF;
    header[7] = (length >> 16) & 0xFF;
    header[8] = (length >> 8) & 0xFF;
    header[9] = length & 0xFF;
    I = S->Send(header, 10);
    if(I<=0)
    {
      return false;
    }
  }

  char separator[6];
  separator[0] = video[0];
  separator[1] = video[1];
  separator[2] = 0x00;
  separator[3] = 0x00;
  separator[4] = 0x00;
  separator[5] = 0x01;
  I = S->Send(separator,6);
  if(I<=0)
  {
      return false;
  }

   // Send video frame
  for(J=2;J<length-4;J+=I)
  {
      I = S->Send(video+J,length-J-4);
      if(I<=0)
      {
        return false;
      }
  }

  return true;
}

bool WebSocketSendAudio(Socket *S, char *data, int length)
{
  int I, J;
  char header[10];
  header[0] = 0x82;
  length = length + 2;

  char *audioData = data;

  if (length <= 125)
  {
    header[1] = length;
    I = S->Send(header, 2);
    if(I<=0)
    {
      return false;
    }
  }
  else if (length < 65536)
  {
    header[1] = 0x7E;
    header[2] = (length >> 8) & 0xFF;
    header[3] = length & 0xFF;
    I = S->Send(header, 4);
    if(I<=0)
    {
      return false;
    }
  }
  else
  {
    header[1] = 0x7F;
    header[2] = 0;
    header[3] = 0;
    header[4] = 0;
    header[5] = 0;
    header[6] = (length >> 24) & 0xFF;
    header[7] = (length >> 16) & 0xFF;
    header[8] = (length >> 8) & 0xFF;
    header[9] = length & 0xFF;
    I = S->Send(header, 10);
    if(I<=0)
    {
      return false;
    }
  }

  char audio[2];// = data+2;
  audio[0] = 0x42;
  audio[1] = 0;
  I = S->Send(audio, 2);
  if(I<=0)
  {
    return false;
  }
   // Send video frame
  for(J=0;J<length-2;J+=I)
  {
      I = S->Send(audioData+J,length-2-J);
      if(I<=0)
      {
          return false;
      }
  }

  return true;
}

bool WebSocketSendHeader(Socket *S, char *data, int length)
{
  int I;
  char header[2];
  header[0] = 0x82;
  header[1] = length+4;
  length = length + 4;
  I = S->Send(header, 2);
  if(I<=0)
  {
      return false;
  }

  char *video = data;
  video[0] = 0x41;
  video[1] = 0;

  char separator[6];
  separator[0] = video[0];
  separator[1] = video[1];
  separator[2] = 0x00;
  separator[3] = 0x00;
  separator[4] = 0x00;
  separator[5] = 0x01;
  I = S->Send(separator,6);
  if(I<=0)
  {
      return false;
  }

  I = S->Send(video+2,length-6);
  if(I<=0)
  {
      return false;
  }

  return true;
}

#endif


