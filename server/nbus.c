/* Copyright 2012, Unpublished Work of Technologic Systems                 
 * All Rights Reserved.                                                    
 *                                                                         
 * THIS WORK IS AN UNPUBLISHED WORK AND CONTAINS CONFIDENTIAL,             
 * PROPRIETARY AND TRADE SECRET INFORMATION OF TECHNOLOGIC SYSTEMS.        
 * ACCESS TO THIS WORK IS RESTRICTED TO (I) TECHNOLOGIC SYSTEMS            
 * EMPLOYEES WHO HAVE A NEED TO KNOW TO PERFORM TASKS WITHIN THE SCOPE     
 * OF THEIR ASSIGNMENTS AND (II) ENTITIES OTHER THAN TECHNOLOGIC           
 * SYSTEMS WHO HAVE ENTERED INTO APPROPRIATE LICENSE AGREEMENTS.  NO       
 * PART OF THIS WORK MAY BE USED, PRACTICED, PERFORMED, COPIED,            
 * DISTRIBUTED, REVISED, MODIFIED, TRANSLATED, ABRIDGED, CONDENSED,        
 * EXPANDED, COLLECTED, COMPILED, LINKED, RECAST, TRANSFORMED, ADAPTED     
 * IN ANY FORM OR BY ANY MEANS, MANUAL, MECHANICAL, CHEMICAL,              
 * ELECTRICAL, ELECTRONIC, OPTICAL, BIOLOGICAL, OR OTHERWISE WITHOUT       
 * THE PRIOR WRITTEN PERMISSION AND CONSENT OF TECHNOLOGIC SYSTEMS.        
 * ANY USE OR EXPLOITATION OF THIS WORK WITHOUT THE PRIOR WRITTEN          
 * CONSENT OF TECHNOLOGIC SYSTEMS COULD SUBJECT THE PERPETRATOR TO         
 * CRIMINAL AND CIVIL LIABILITY.                                           
 */                                                                        
/* To compile nbus in to your application,
 * use the appropriate cross compiler and run the command:
 *                                                                       
 *  gcc -fno-tree-cselim -Wall -O -mcpu=arm9 -o <app> <app.c>  nbus.c
 *                                                                       
 * On uclibc based initrd's, the following additional gcc options are    
 * necessary: -Wl,--rpath,/slib -Wl,-dynamic-linker,/slib/ld-uClibc.so.0 
 */
                                                                           
/* Version history:                                                        
 * 
 *   9/2015:  Added winpeek/winpoke -- Michael D. Peters                                   
 *                                                                         
 */     

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/futex.h>
#include <sched.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/sem.h>
#include <stdio.h>
#include <stdlib.h>                                                                   
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <unistd.h>


#include "nbus.h"

#ifndef TEMP_FAILURE_RETRY
# define TEMP_FAILURE_RETRY(expression) \
  (__extension__                                                              \
    ({ long int __result;                                                     \
       do __result = (long int) (expression);                                 \
       while (__result == -1L && errno == EINTR);                             \
       __result; }))
#endif

static int nbuslocked = 0;
static volatile unsigned long *mxsgpioregs;

void nbus_poke16(unsigned char adr, unsigned short dat)
{
	int did_lock = 0, x;
	
	if(!nbuslocked) {
		nbuslock();
		did_lock++;
	}
	mxsgpioregs[0x708/4] = (1 << 16) | (1 << 25) | (0xff);	
	mxsgpioregs[0x704/4] = (1 << 24) | (1 << 26) | adr ;
	mxsgpioregs[0x704/4] = (1 << 25);

	for(x = 1; x >= 0; x--) {	
		mxsgpioregs[0x708/4] = (1 << 26) | (1 << 25) | (0xff);
		mxsgpioregs[0x704/4] = (unsigned char)(dat >> (x*8));		
		mxsgpioregs[0x704/4] = (1 << 25);
	}	
	mxsgpioregs[0x704/4] = (1 << 16);

	while(mxsgpioregs[0x900/4] & (1 << 21)) {	
		mxsgpioregs[0x708/4] = (1 << 16);
		mxsgpioregs[0x704/4] = (1 << 16);
	}

	if(did_lock) nbusunlock();
}

unsigned short nbus_peek16(unsigned char adr)
{
	int did_lock = 0, x;
	unsigned short ret;

	if(!nbuslocked) {
		nbuslock();
		did_lock++;
	}
	
	mxsgpioregs[0x708/4] = (1 << 16) | (1 << 25) | (1 << 24) | (0xff);
	mxsgpioregs[0x704/4] = ((1 << 26) | adr);
	mxsgpioregs[0x704/4] = (1 << 25);
	mxsgpioregs[0xb08/4] = 0xff;

	do {
		ret = 0;
		for(x = 1; x >= 0; x--) {
			mxsgpioregs[0x708/4] = (1 << 26) | (1 << 25) | (0xff);
			mxsgpioregs[0x704/4] = (1 << 25);
			ret |= ((mxsgpioregs[0x900/4] & 0xff) << (x*8));
		}
		mxsgpioregs[0x704/4] = (1 << 16);
		mxsgpioregs[0x708/4] = (1 << 16);
	} while(mxsgpioregs[0x900/4] & (1 << 21));

	mxsgpioregs[0x704/4] = (1 << 16);
	mxsgpioregs[0xb04/4] = 0xff;

	if(did_lock) nbusunlock();

	return ret;
}


void winpoke16(unsigned int adr, unsigned short dat) {

    int did_lock = 0;

    if(!nbuslocked) {
        nbuslock();
        did_lock++;
    }
        nbus_poke16(MW_ADR, adr >> 11);
        nbus_poke16(MW_CONF, (adr & 0x7ff) | (0x10 << 11));
        nbus_poke16(MW_DAT1, dat);
    if(did_lock) nbusunlock();

}
unsigned short winpeek16(unsigned int adr) {
    int did_lock = 0;
   unsigned short retVal;

    if(!nbuslocked) {
        nbuslock();
        did_lock++;
    }
    nbus_poke16(MW_ADR, adr >> 11);
    nbus_poke16(MW_CONF, (adr & 0x7ff) | (0x10 << 11));
    retVal = nbus_peek16(MW_DAT1);
    if(did_lock) nbusunlock();
    return retVal;
}

void winpoke16a(unsigned int adr, unsigned short dat) {

    int did_lock = 0;

    if(!nbuslocked) {
        nbuslock();
        did_lock++;
    }
        nbus_poke16(MW_ADR, adr >> 11);
        nbus_poke16(MW_CONF, (adr & 0x7ff) | (0x12 << 11));
        nbus_poke16(MW_DAT1, dat);
    if(did_lock) nbusunlock();

}
unsigned short winpeek16a(unsigned int adr) {
    int did_lock = 0;
   unsigned short retVal;

    if(!nbuslocked) {
        nbuslock();
        did_lock++;
    }
    nbus_poke16(MW_ADR, adr >> 11);
    nbus_poke16(MW_CONF, (adr & 0x7ff) | (0x12 << 11));
    retVal = nbus_peek16(MW_DAT1);
    if(did_lock) nbusunlock();
    return retVal;
}

void winpoke32(unsigned int adr, unsigned int dat) {

    int did_lock = 0;

    if(!nbuslocked) {
        nbuslock();
        did_lock++;
    }

    nbus_poke16(MW_ADR, adr >> 11);
    nbus_poke16(MW_CONF, (adr & 0x7ff) | (0x0d << 11));
    nbus_poke16(MW_DAT2, dat & 0xffff);
    nbus_poke16(MW_DAT2, dat >> 16);
    if(did_lock) nbusunlock();
}

unsigned int winpeek32(unsigned int adr) {
    int did_lock = 0;
    unsigned int ret;
    if(!nbuslocked) {
        nbuslock();
        did_lock++;
    }
    nbus_poke16(MW_ADR, adr >> 11);
    nbus_poke16(MW_CONF, (adr & 0x7ff) | (0x0d << 11));
    ret = nbus_peek16(MW_DAT2);
    ret |= (nbus_peek16(MW_DAT2) << 16);
    if(did_lock) nbusunlock();
    return ret;
}

void winpoke8(unsigned int adr, unsigned char dat) {
    int did_lock = 0;
    if(!nbuslocked) {
        nbuslock();
        did_lock++;
    }
    nbus_poke16(MW_ADR, adr >> 11);
    nbus_poke16(MW_CONF, (adr & 0x7ff) | (0x18 << 11));
    nbus_poke16(MW_DAT1, dat);
    if(did_lock) nbusunlock();
}

unsigned char winpeek8(unsigned int adr) {
    int did_lock = 0;
    if(!nbuslocked) {
        nbuslock();
        did_lock++;
    }
    nbus_poke16(MW_ADR, adr >> 11);
    nbus_poke16(MW_CONF, (adr & 0x7ff) | (0x18 << 11));
    if(did_lock) nbusunlock();
    return nbus_peek16(MW_DAT1) & 0xff;
}

void winpoke8a(unsigned int adr, unsigned char dat) {
    int did_lock = 0;
    if(!nbuslocked) {
        nbuslock();
        did_lock++;
    }
    nbus_poke16(MW_ADR, adr >> 11);
    nbus_poke16(MW_CONF, (adr & 0x7ff) | (0x1a << 11));
    nbus_poke16(MW_DAT1, dat);
    if(did_lock) nbusunlock();
}

unsigned char winpeek8a(unsigned int adr) {
    int did_lock = 0;
    if(!nbuslocked) {
        nbuslock();
        did_lock++;
    }
    nbus_poke16(MW_ADR, adr >> 11);
    nbus_poke16(MW_CONF, (adr & 0x7ff) | (0x1a << 11));
    if(did_lock) nbusunlock();
    return nbus_peek16(MW_DAT1) & 0xff;
}

static int semid = -1;
void nbuslock(void)
{
	int devmem = 0;
	int r;
	struct sembuf sop;
	static int inited = 0;

	if(nbuslocked) return;

	if (semid == -1) {
		key_t semkey;
		semkey = 0x75000000;
		semid = semget(semkey, 1, IPC_CREAT|IPC_EXCL|0777);
		if (semid != -1) {
			sop.sem_num = 0;
			sop.sem_op = 1;
			sop.sem_flg = 0;
			r = semop(semid, &sop, 1);
			assert (r != -1);
		} else semid = semget(semkey, 1, 0777);
		assert (semid != -1);
	}
	sop.sem_num = 0;
	sop.sem_op = -1;
	sop.sem_flg = SEM_UNDO;


	/*Wrapper added to retry in case of EINTR*/
	r = TEMP_FAILURE_RETRY(semop(semid, &sop, 1));
	assert(r == 0);

	nbuslocked = 1;
	
	if(!inited) {
	
		devmem = open("/dev/mem", O_RDWR|O_SYNC);
		mxsgpioregs = mmap(0, getpagesize(), PROT_READ|PROT_WRITE, MAP_SHARED,
		  devmem, 0x80018000);
		  
		if (mxsgpioregs ==  MAP_FAILED) {		
		   fprintf(stderr, "%s %d, mmap() failed\n", __func__, __LINE__);
		} else {
		/* Set all NAND pins to GPIO */
		mxsgpioregs[0x104/4] = 0x0000ffff; //Set pinmux to GPIO
		mxsgpioregs[0x114/4] = 0x003f0c03; //Set pinmux to GPIO
		mxsgpioregs[0x704/4] = 0x070100ff; //Set to logic high
		mxsgpioregs[0xb00/4] = 0x070100ff; //Set to output
		inited = 1;
		}
	}
}
/*********************************************************************************/
void nbusunlock(void) 
{
	struct sembuf sop = { 0, 1, SEM_UNDO};
	int r;
	if(nbuslocked) {
		r = semop(semid, &sop, 1);
		assert(r == 0);
		nbuslocked = 0;
	}
}
/*********************************************************************************/
void nbuspreempt(void)
{
	int r;
	r = semctl(semid, 0, GETNCNT);
	assert(r != -1);
	if(r) {
		nbusunlock();
		sched_yield();
		nbuslock();
	}
}
/*********************************************************************************/
void mydelay(unsigned long i)
{
	unsigned long j;
	struct timespec sleepTime;
	struct timespec rettime;
	sleepTime.tv_sec = 0;
	sleepTime.tv_nsec = 948540;
	for(j = 0;j < i;j++)
	{
		nanosleep(&sleepTime, &rettime);
//		printf(".");
	}
}
/*********************************************************************************/
int get_pin(int pin)		// see dio.c
{
	unsigned short reg;
	if(pin >= 0 && pin <= 69) 
	{
		reg = nbus_peek16(0x8+(6*(pin/16)));
		return ((reg >> (pin%16)) & 1);
	}
	return 0;
}
/*********************************************************************************/
void set_dir(int pin, int dir)
{
	if(dir > 0)
		nbus_poke16(0x28, pin | 0x80);
	else nbus_poke16(0x28, pin & ~0x80);
	mydelay(1);
}
/*********************************************************************************/
void set_pin(int pin, int val)
{
	if(val > 0)
		nbus_poke16(0x26, pin | 0x80);
	else nbus_poke16(0x26, pin & ~0x80);
	mydelay(1);
}
