#include<unistd.h>
#include<sys/types.h>
#include<sys/mman.h>
#include<stdio.h>
#include<fcntl.h>
#include<assert.h>
#include<time.h>
#include<stdlib.h>
#include <sys/time.h>
#include "mytypes.h"
#include "ioports.h"

/**********************************************************************************************************/
struct timeval tv;

static double curtime(void)
{
	gettimeofday (&tv, NULL);
	return tv.tv_sec + tv.tv_usec / 1000000.0;
}
/**********************************************************************************************************/
static void mydelay(unsigned long i)
{
	unsigned long j;
	struct timespec sleepTime;
	struct timespec rettime;
	sleepTime.tv_sec = 0;
	sleepTime.tv_nsec = 948540;
	for(j = 0;j < i;j++)
	{
		nanosleep(&sleepTime, &rettime);

	}
}
/**********************************************************************************************************/
void OutPortA(int onoff, int bit)
{
	//printf("OutPortA: %d %d\n",onoff,bit);
	UCHAR val;
	UINT val2 = 0x10300;
	nbuslock();
	val = winpeek8(val2);

	if(onoff == 1)
		winpoke8(val2, val | (1 << bit));
	else
		winpoke8(val2, val & ~(1 << bit));

	nbusunlock();
}
/**********************************************************************************************************/
void OutPortB(int onoff, int bit)
{
	//printf("OutPortB: %d %d\n",onoff,bit);
	UCHAR val;
	UINT val2 = 0x10301;
	nbuslock();
	val = winpeek8(val2);
	if(onoff == 1)
		winpoke8(val2, val | (1 << bit));
	else
		winpoke8(val2, val & ~(1 << bit));
	nbusunlock();
}
/**********************************************************************************************************/
void OutPortC(int onoff, int bit)
{
	//printf("OutPortC: %d %d\n",onoff,bit);
	UCHAR val;
	UINT val2 = 0x10302;
	nbuslock();
	val = winpeek8(val2);
	if(onoff == 1)
		winpoke8(val2, val | (1 << bit));
	else
		winpoke8(val2, val & ~(1 << bit));
	nbusunlock();
}
/***********************************************************************************************************/
UCHAR InPortByteA(void)
{
	UCHAR val;
	UINT val2 = 0x10304;
	nbuslock();
	val = winpeek8(val2);
	nbusunlock();
	return val;
}
/***********************************************************************************************************/
UCHAR InPortByteB(void)
{
	UCHAR val;
	UINT val2 = 0x10305;
	nbuslock();
	val = winpeek8(val2);
	nbusunlock();
	return val;
}
/***********************************************************************************************************/
UCHAR InPortByteC(void)
{
	UCHAR val;
	UINT val2 = 0x10306;
	nbuslock();
	val = winpeek8(val2);
	nbusunlock();
	return val;
}
/***********************************************************************************************************/
/*	used for monitor_input_task
UCHAR InPortByte(int bank)
{
	UCHAR state 
*/
