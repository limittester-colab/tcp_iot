#ifndef __IOPORTS_H
#define __IOPORTS_H
//#warning "IOPORTS defined"
#define			VUCHAR volatile unsigned char

//#define NUM_PORTS (OUTPORTF_OFFSET-OUTPORTA_OFFSET)+1
#define 		NUM_PORTS 3
//#define 		NUM_PORT_BITS (NUM_PORTS*8)-8	// PORTC & F only has 6 bits each
#define 		NUM_PORT_BITS  		NUM_DATA_RECS

#define			DELAYTIME 50

#define			COMMQ_BAD_CH		160u

void OutPortA(int onoff, int bit);
UCHAR InPortByteA(void);

void OutPortB(int onoff, int bit);
UCHAR InPortByteB(void);

void OutPortC(int onoff, int bit);
UCHAR InPortByteC(void);

#endif

