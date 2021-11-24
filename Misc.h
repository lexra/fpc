
#if !defined(AFX_MISC_H__A3151524_6E3A_44C9_BA50_39E1A15FA74F__INCLUDED_)
#define AFX_MISC_H__A3151524_6E3A_44C9_BA50_39E1A15FA74F__INCLUDED_


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000



//////////////////////////////////////////////////////////////////////
// macro
//////////////////////////////////////////////////////////////////////

#define BUILD_UINT16(loByte, hiByte) \
          ((unsigned short)(((loByte) & 0x00FF) + (((hiByte) & 0x00FF) << 8)))

#define BUILD_UINT32(Byte0, Byte1, Byte2, Byte3) \
          ((unsigned int)((unsigned int)((Byte0) & 0x00FF) + ((unsigned int)((Byte1) & 0x00FF) << 8) \
			+ ((unsigned int)((Byte2) & 0x00FF) << 16) + ((unsigned int)((Byte3) & 0x00FF) << 24)))

#define HI_UINT16(a) (((a) >> 8) & 0xFF)
#define LO_UINT16(a) ((a) & 0xFF)

#define BUILD_UINT8(hiByte, loByte) \
          ((unsigned char)(((loByte) & 0x0F) + (((hiByte) & 0x0F) << 4)))

#define HI_UINT8(a) (((a) >> 4) & 0x0F)
#define LO_UINT8(a) ((a) & 0x0F)

#define BREAK_UINT32(var, ByteNum) \
          (unsigned char)((unsigned int)(((var) >>((ByteNum) * 8)) & 0x00FF))


#define hang(format, arg...)	{ printf("hang(): " format "\n" , ##arg); while(1); };



//////////////////////////////////////////////////////////////////////
// macro
//////////////////////////////////////////////////////////////////////

typedef struct tm SYSTEMTIME;


//////////////////////////////////////////////////////////////////////
// extern
//////////////////////////////////////////////////////////////////////

#if defined(__cplusplus) || defined(__CPLUSPLUS__)
extern "C" {
#endif // __cplusplus


int SetBaudRate(int fd, int baudrate);

void Average2(unsigned short av1,unsigned short t1,unsigned short av2,unsigned short t2, volatile unsigned short * target);

int CONN(const char *domain, int port);

int search_lf(const unsigned char *str, int len);
int search_msb_on(const unsigned char *str, int len);
int search_ctrld(const unsigned char *str, int len);
int i2c_read_data(int fd_i2c, int bus_adr, int sub_adr, unsigned char *data, int length);
int i2c_write_data(int fd_i2c, int bus_adr, int sub_adr, const unsigned char *data, int length);

void CleanupRegx(void *param);
void CleanupFd(void *param);
void CleanupLock(void *param);
void CleanupHandle(void *param);

char *FileExtension(const char *path);
char *FileMaster(const char *path);
char *FileName(const char *path);

void RestoreSigmask(void *param);

void TimespecAdd(const struct timespec* a, const struct timespec* b, struct timespec* out);

int IsLittleEndian(void);
char *StripCrLf(char *line);

int tstrnlen(const char *s, int maxlen);
void ReverseBytes(unsigned char *pData, unsigned char len);
void GetLocalTime(SYSTEMTIME *st);

unsigned int ModbusCrc16(unsigned char *buf, unsigned short len);
unsigned char CalcFcs(unsigned char *msg_ptr, unsigned char len);

int fsize(FILE *fp);


#if defined(__cplusplus) || defined(__CPLUSPLUS__)
}
#endif // __cplusplus


#endif // !defined(AFX_MISC_H__A3151524_6E3A_44C9_BA50_39E1A15FA74F__INCLUDED_)


