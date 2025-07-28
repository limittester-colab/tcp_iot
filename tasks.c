#if 1
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <assert.h>
#include <time.h>
#include <sys/time.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h> 
#include <string.h>
#include <sched.h>
#include <pthread.h>
#define closesocket close
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <dirent.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "cmd_types.h"
#include "mytypes.h"
#include "ioports.h"
//#include "serial_io.h"
#include "queue/ollist_threads_rw.h"
//#include "queue/cllist_threads_rw.h"
//#include "queue/dllist_threads_rw.h"
//#include "queue/sllist_threads_rw.h"
#include "tasks.h"
#include "nbus.h"
//#include "nbus/dio_ds1620.h"
//#include "nbus/dio_mcp3002.h"
//#include "cs_client/dconfig_file.h"
#define TOGGLE_OTP otp->onoff = (otp->onoff == 1?0:1)

pthread_mutex_t     io_mem_lock=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t     serial_write_lock=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t     serial_read_lock=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t     serial_write_lock2=PTHREAD_MUTEX_INITIALIZER;
//pthread_mutex_t     serial_read_lock2=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t     msg_queue_lock=PTHREAD_MUTEX_INITIALIZER;
int total_count;

static UCHAR check_inputs(int index, int test);
UCHAR inportstatus[NUM_DATA_RECS];

static CMD_STRUCT cmd_array[];
extern REAL_BANKS real_banks[40];

enum client_list	// if adding to this list, change MAX_CLIENTS above 
{
	_158,			// WINDOWS-11A (runs on Windows machine)
	_154,			// Cabin
	_147,			// Testbench
	_243,			// aux_client (runs on a PC linux box)
	_151,			// extra TS-4600 card
	_SERVER			// Garage (146)
}CLIENT_LIST2;

ollist_t oll;
/*
cllist_t cll;
dllist_t dll;
sllist_t sll;
int ds_index;
int ds_reset;
extern int cs_index;
extern CLIENT_TABLE client_table[];
PARAM_STRUCT ps;

extern UCHAR start_seq[];
extern UCHAR mcp_data[];
extern void init_MCP3002();

static UCHAR read_serial_buffer[SERIAL_BUFF_SIZE];
static UCHAR write_serial_buffer[SERIAL_BUFF_SIZE];
static int no_serial_buff;
char password[PASSWORD_SIZE];

static int serial_rec;
static int mask2int(UCHAR mask);
extern int shutdown_all;
static int raw_data_array[RAW_DATA_ARRAY_SIZE];
static int raw_data_ptr;
static void dsSleep(int interval);
int convertFi(int raw_data);
int max_ips;
IP ip[20];
extern UCHAR start_seq[];
extern UCHAR mcp_data[];

static COUNTDOWN count_down[COUNTDOWN_SIZE];
int curr_countdown_size;
*/

#define ON 1
#define OFF 0
enum input_types
{
	STARTER_INPUT,				// 0 	STARTER & COOLINGFAN don't go to the tray
	COOLINGFAN_INPUT,			// 1 	first card starts @ 280h
	BRAKE_INPUT,				// 2
	SEAT_SWITCH,				// 3
	DOOR_SWITCH,				// 4
	WIPER_HOME,					// 5  	red wire from the wiper motor
	WIPER1_INPUT,				// 6  	switch to turn on slow wipers
	WIPER2_INPUT,				// 7 	switch to turn on fast wipers
	WIPER_OFF_INPUT,			// 8
	BACKUP_INPUT,				// 9
	HEADLAMP_INPUT,				// 10 	this starts the DB-15 under the dash
	RUNNING_LIGHTS_INPUT,		// 11 	HEADLAMP_INPUT -> BRIGHTS_INPUT comes
	MOMENTARY_INPUT,			// 12 	from the AM turn signal switch
	LEFTBLINKER_INPUT,			// 13 	20 is the start of the 2nd card @ 300h
	RIGHTBLINKER_INPUT,			// 14
	BRIGHTS_INPUT,				// 15
	ESTOP_INPUT = 27,			// 27	emergency stop
	ROCKER1_INPUT,				// 28
	ROCKER2_INPUT,				// 29
	ROCKER3_INPUT,				// 30
	ROCKER4_INPUT,				// 31
	ROCKER5_INPUT,				// 32
}INPUT_TYPES;

// * 25 not working only because wire from DB-15 not going to cable on dip on TS-7800

enum output_types
{
#ifdef SERVER_146
	DESK_LIGHTa,
	EAST_LIGHTa,			// bank 0
	NORTHWEST_LIGHTa,
	SOUTHEAST_LIGHTa,
	MIDDLE_LIGHTa,
	WEST_LIGHTa,
	NORTHEAST_LIGHTa,
	SOUTHWEST_LIGHTa,
	WATER_PUMPa,
	WATER_VALVE1a,
	WATER_VALVE2a,
	WATER_VALVE3a,
	WATER_HEATERa
#endif 
#ifdef CL_151
#warning "CL_151 defined"
	COOP1_LIGHTa,
	COOP1_HEATERa,
	COOP2_LIGHTa,
	COOP2_HEATERa,
	OUTDOOR_LIGHT1a,
	OUTDOOR_LIGHT2a,
	UNUSED150_1a,
	UNUSED150_2a,
	UNUSED150_3a,
	UNUSED150_4a,
	UNUSED150_5a,
	UNUSED150_6a,
	UNUSED150_7a,
	UNUSED150_8a,
	UNUSED150_9a,
	UNUSED150_10a
#endif 
#ifdef CL_147	
#warning "CL_147 defined"
	CHICK_LIGHTa,
	CHICK_HEATERa,
	BENCH_12V_1a,
	BENCH_12V_2a,
	BLANKa,
	BENCH_5V_1a,
	BENCH_5V_2a,
	BENCH_3V3_1a,
	BENCH_3V3_2a,
	BENCH_LIGHT1a,
	BENCH_LIGHT2a,
	BATTERY_HEATERa,
	TEST_OUTPUT3,		// these all have the wires
	TEST_OUTPUT4,		// from io card to relay 
	TEST_OUTPUT5,		// but that's it
	TEST_OUTPUT6,
	TEST_OUTPUT7
#endif 
#ifdef CL_154
#warning "CL_154 defined"
	TEST_OUTPUT1,
	TEST_OUTPUT2,
	TEST_OUTPUT3,
	TEST_OUTPUT4,
	TEST_OUTPUT5,
	TEST_OUTPUT6,
	TEST_OUTPUT7,
	TEST_OUTPUT8,
	TEST_OUTPUT9,
	TEST_OUTPUT10,
	CABIN1a,
	CABIN2a,
	CABIN3a,
	CABIN4a,
	CABIN5a,
	CABIN6a,
	CABIN7a,
	CABIN8a,
	TEST_OUTPUT18,
	TEST_OUTPUT19
#endif 
}OUTPUT_TYPES;

int switch_status[10];

/****************************************************************************************************/
static double curtime(void)
{
	struct timeval tv;
	if(gettimeofday (&tv, NULL) == 0)
		return tv.tv_sec + tv.tv_usec / 1000000.0;
	else return 0.0;
}
/*
 clock_t start = clock();
   //... do work here
   clock_t end = clock();
   double time_elapsed_in_seconds = (end - start)/(double)CLOCKS_PER_SEC;
   return 0;
*/
/****************************************************************************************************/
int uSleep(time_t sec, long nanosec)
{
/* Setup timespec */
	struct timespec req;
	req.tv_sec = sec;
	req.tv_nsec = nanosec;

/* Loop until we've slept long enough */
	do
	{
/* Store remainder back on top of the original required time */
		if( 0 != nanosleep( &req, &req ) )
		{
/* If any error other than a signal interrupt occurs, return an error */
			if(errno != EINTR)
			{
				return -1;
			}
		}
		else
		{
/* nanosleep succeeded, so exit the loop */
			break;
		}
	} while ( req.tv_sec > 0 || req.tv_nsec > 0 );

	return 0;									  /* Return success */
}

/*********************************************************************/
static int mask2int(UCHAR mask)
{
	int i = 0;
	do
	{
		mask >>= 1;
		i++;
	}while(mask);
	return i - 1;
}
/*********************************************************************/
// change the outputs according to the type
// type 0 is normal, 1 is reverse momentary-contact, 2 is time delayed
// type 1 makes a momentary-contact switch act like a toggle switch
// (a button press and release changes state where type 0 state 
// toggles with the switch)
static void set_output(O_DATA *otp, int onoff)
{
	O_DATA **otpp = &otp;
	UCHAR buff[1];
	char tempx[20];

	//printf("test\r\n");
#ifndef USE_CARDS
	printf("not using cards\n");
	return;
#endif
	switch(otp->type)
	{
		case 0:
/*
			if(otp->polarity == 1)
				otp->onoff = (onoff == 1?0:1);
			else
				otp->onoff = onoff;
*/
			otp->onoff = onoff;
			change_output(otp->port,otp->onoff);
			ollist_insert_data(otp->port,&oll,otp);
			//printf("type 0 port: %d onoff: %d\r\n", otp->port, otp->onoff);
			break;
		case 1:
			if(otp->reset == 0)
			{
//				printf("type 1 port: %d onoff: %d\r\n", otp->port, otp->onoff);
				otp->reset = 1;
//				if(otp->polarity == 0)
				TOGGLE_OTP;
				change_output(otp->port,otp->onoff);
				ollist_insert_data(otp->port,&oll,otp);
//				printf("type 1 port: %d onoff: %d reset: %d pol: %d\r\n\r\n", otp->port,
//										otp->onoff, otp->reset, otp->polarity);
			}
			else if(otp->reset == 1)
			{
//				if(otp->polarity == 1)
//					TOGGLE_OTP;
				otp->reset = 0;
//				printf("type 1 port: %d onoff: %d reset: %d pol: %d\r\n", otp->port,
//							otp->onoff, otp->reset, otp->polarity);
			}
			break;
		case 2:
		case 3:
//			printf("type %d port: %d onoff: %d reset: %d\r\n",otp->type, otp->port, otp->onoff, otp->reset);
			if(otp->reset == 0)
			{
				otp->reset = 1;
				otp->time_left = otp->time_delay;
//							otp->onoff = onoff;
//				TOGGLE_OTP;
				otp->onoff = 1;
				change_output(otp->port,1);
				ollist_insert_data(otp->port,&oll,otp);
			}
			break;
		case 4:		// no type 4 anymore
			if(otp->reset == 0)
			{
				otp->reset = 1;
//				if(otp->polarity == 0)
//				TOGGLE_OTP;
				otp->onoff = 0;
				change_output(otp->port,otp->onoff);
				ollist_insert_data(otp->port,&oll,otp);
			}
			else if(otp->reset == 1)
			{
//				if(otp->polarity == 1)
//					TOGGLE_OTP;
				otp->reset = 3;
//				printf("type 4 port: %d onoff: %d reset: %d \r\n\r\n", otp->port,
//										otp->onoff, otp->reset);
			}
			break;
		default:
			break;
	}
/*
	if(otp->port == LHEADLAMP || otp->port == RHEADLAMP)
	{
		if(otp->onoff == 1)		// this sets the 'auto mode' for the lights to on 
		{
			lights_on = 3;
		}else
		{
			lights_on = 0;
		}
	}
*/
}
/*********************************************************************/
// if an input switch is changed, update the record for that switch
// and if an output applies to that input, change the output
// and record the event in llist - each input can be assigned any output
UCHAR monitor_input_task(int test)
{
//	I_DATA *itp;
//	I_DATA **itpp = &itp;
	O_DATA *otp;
	O_DATA **otpp = &otp;

	int status = -1;
	int bank, index;
	UCHAR result,result2, mask, onoff;
	int i, rc, flag;
	int input_port;
	char tempx[20];

//	TODO: what if more than 1 button is pushed in same bank or diff bank at same time?
#ifndef USE_CARDS
	printf("not using cards\n");

	while(TRUE)
	{
		uSleep(1,0);
		if(close_program)
			return 0;
	}
#endif

#ifdef SKIP_MONITOR_INPUTS
	printf("Skip monitor inputs\n");

	while(TRUE)
	{
		uSleep(1,0);
		if(close_program)
			return 0;
	}
#endif

	pthread_mutex_lock( &io_mem_lock);

	inportstatus[0] =  ~InPortByteA();
	inportstatus[1] =  ~InPortByteB();
	inportstatus[2] =  ~InPortByteC();
/*
	inportstatus[3] =  ~InPortByteD();
	inportstatus[4] =  ~InPortByteE();
	inportstatus[5] =  ~InPortByteF();
*/
	pthread_mutex_unlock( &io_mem_lock);

//	printf("monitor\r\n");

	while(TRUE)
	{
		for(bank = 0;bank < NUM_PORTS;bank++)
		{
			usleep(_500MS);
			usleep(_500MS);
			pthread_mutex_lock( &io_mem_lock);
			switch(bank)
			{
				case 0:
				result= InPortByteA();
				break;
				case 1:
				result= InPortByteB();
				break;
				case 2:
				result= InPortByteC();
				break;
			}
			pthread_mutex_unlock( &io_mem_lock);
			result = ~result;

			if(result != inportstatus[bank])  
			{
				mask = result ^ inportstatus[bank];
//				printf("\nresult:  %02x\n",result);
//				printf("%02x %d\n",inportstatus[bank],bank);
/*
				if(mask > 0x80)
				{
					myprintf1("bad mask\0");
					//printf("bad mask 1 %02x\r\n",mask);
					continue;
				}
*/
				index = mask2int(mask);

				if((mask & result) == mask)
				{
					onoff = ON;
				}
				else
				{
					onoff = OFF;
				}
				// since each card only has 20 ports then the 1st 2 port (A & B on 
				// 1st card; D & E on 2nd) are 8-bit and the 3rd is only 4-bits, 
				// so we have to translate the inportstatus array, representing 
				// 3 byts of each 2 (3x8x2 = 48) to one of the 40 actual bits as index
				for(i = 0;i < 20;i++)
				{
					if(real_banks[i].bank == bank && real_banks[i].index == index)
					{
						index = real_banks[i].i;
					}
				}
				printf("index: %d ",index);
				if(onoff == 1)printf("ON\n");
				else printf("OFF\n");
				// go threw the list of valid input ports and find the output port 
				// assigned to it - if input_type != 0 then call the function  
				// according to the input_type as one of the commands in cmd_array
#if 0
				for(i = 0;i < max_ips;i++)
				{
//					printf("%d %d %d\r\n",ip[i].port,ip[i].input,index);
					if(ip[i].input == index)
					{
						if(ip[i].function == 0)
						{
							ollist_find_data(ip[i].port,&otp,&oll);
							set_output(otp, onoff);
							printf("%d %d\n", index, otp->port);
//							myprintf1(tempx);
						}else
						{
							if(onoff == ON)
							{
								add_msg_queue(ip[i].function,0);
//								sprintf(tempx,"-%d %d", ip[i].input, ip[i].function);
	//							myprintf1(tempx);
	//							printf("msg queue: %d %d\r\n",ip[i].input, ip[i].function);
							}
						}
					}
				}
#endif
				inportstatus[bank] = result;
				//printf("leave 1: %02x\r\n\r\n",inportstatus[bank]);
			}
		}
		uSleep(0,TIME_DELAY/4);
		if(close_program)
		{
				//printf("done mon input tasks\r\n"); 
//				myprintf1("done mon input");
				//printString2("done mon");
			return 0;
		}
	}
	return 1;
}

/*********************************************************************/
// this is used so that inputs can be changed programatically by software
// as if an input button or switch were changed
int change_input(int index, int onoff)
{
	int bank;
	UCHAR mask = 1;
	UCHAR state = 0;

#ifndef USE_CARDS
	printf("not using cards\n");
	return;
#endif

	bank = real_banks[index].bank;
	index = real_banks[index].index;

	mask <<= index;
/*
	if(onoff)
	{
		fake_inportstatus2[bank] |= mask;
	}
	else
	{
		fake_inportstatus2[bank] &= ~mask;
	}
*/
}
/*********************************************************************/
// pass in the index into the total list of outputs
// since each card only has 20 outputs, the last 4 bits of PORT C & F are ignored
// index 0->19 = PORTA(0:7)->PORTC(0:4)
// index 24->39 = PORTD(0:7)->PORTF(0:4)
int change_output(int index, int onoff)
{
	int bank;
	int index2;
	
#ifndef USE_CARDS
	printf("not using cards\n");
	return 0;
#endif

	//printf("change output: %d\n",index);
//	pthread_mutex_lock( &io_mem_lock);

	bank = real_banks[index].bank;
	index2 = real_banks[index].index;
	// for this application, there's only 1 card and the 2nd address
	// doesn't work, so bank 0 is the 1st 8 bits and bank 2 is the 
	// last 4 - 280 & 282 (281 doesn't work)
//printf("%d %d %d\n",onoff, index, bank);

	printf("bank: %d index: %d index2 %d\r\n",bank,index, index2);
#ifndef SERVER_146
	switch(bank)
	{
		case 0:
			OutPortA(onoff, index2);			  // 0-7
			break;
		case 1:
			OutPortB(onoff, index2);			  // 0-7
			break;
		case 2:
			OutPortC(onoff, index2);			  // 0-3
			break;
		default:
			break;
	}
#endif

//	pthread_mutex_unlock(&io_mem_lock);
	//printf("change output: %d %d\r\n",index,onoff);

//	sprintf(tempx,"%d %d %d", bank, index, onoff);
//	myprintf1(tempx);

	return index;
}
#endif
/*********************************************************************/
UCHAR basic_controls_task(int *test)
{
	int i,j;
	UCHAR onoff;
	O_DATA *otp;
	O_DATA **otpp = &otp;
	int rc;
	int index;
	size_t isize;
	size_t osize;
	char errmsg[50];
	UCHAR cmd;
	char tempx[SERIAL_BUFF_SIZE];
	struct msgqbuf msg;
	int msgtype = 1;

//	memset(msg_queue,0,sizeof(msg_queue));
	msg.mtype = msgtype;
	memset(switch_status,0,sizeof(switch_status));

/*
	for(i = 0;i < 10;i++)	// test the 2nd relay module
	{
		change_output(TEST_OUTPUT10+i,1);
		uSleep(1,0);
		change_output(TEST_OUTPUT10+i,0);
		uSleep(1,0);
	}
*/

	while(TRUE)
	{
		if (msgrcv(basic_controls_qid, (void *) &msg, sizeof(msg.mtext), msgtype,
//		MSG_NOERROR | IPC_NOWAIT) == -1) 
		MSG_NOERROR) == -1) 
		{
			if (errno != ENOMSG) 
			{
				perror("msgrcv");
				printf("msgrcv error\n");
				exit(EXIT_FAILURE);
			}
		}
		cmd = msg.mtext[0];
		onoff = msg.mtext[1];
		
		printf("basic controls: ");
		print_cmd(cmd);
		//printf("%d\n",onoff);
		//usleep(_5MS);

		switch(cmd)
		{
#ifdef SERVER_146
			case DESK_LIGHT:
				change_output(DESK_LIGHTa,(int)onoff);
				usleep(_500MS);
				break;

			case EAST_LIGHT:	//  relay is wired nc while all others are no 
				if(onoff == 0)
					onoff = 1;
				else onoff = 0;
				change_output(EAST_LIGHTa,(int)onoff);
				usleep(_500MS);
				break;

			case NORTHWEST_LIGHT:
				change_output(NORTHWEST_LIGHTa,(int)onoff);
				usleep(_500MS);
				break;

			case SOUTHEAST_LIGHT:
				change_output(SOUTHEAST_LIGHTa,(int)onoff);
				usleep(_500MS);
				break;

			case MIDDLE_LIGHT:
				change_output(MIDDLE_LIGHTa,(int)onoff);
				usleep(_500MS);
				break;

			case WEST_LIGHT:
				change_output(WEST_LIGHTa,(int)onoff);
				usleep(_500MS);
				break;
				
			case NORTHEAST_LIGHT:
				change_output(NORTHEAST_LIGHTa,(int)onoff);
				usleep(_500MS);
				break;

			case SOUTHWEST_LIGHT:
				change_output(SOUTHWEST_LIGHTa,(int)onoff);
				usleep(_500MS);
				break;

			case WATER_HEATER:
				change_output(WATER_HEATERa,onoff);
				usleep(_500MS);
				break;

			case WATER_PUMP:
				change_output(WATER_PUMPa,onoff);
				usleep(_500MS);
				break;

			case WATER_VALVE1:
				change_output(WATER_VALVE1a,onoff);
				usleep(_500MS);
				break;

			case WATER_VALVE2:
				change_output(WATER_VALVE2a,onoff);
				usleep(_500MS);
				break;

			case WATER_VALVE3:
				change_output(WATER_VALVE3a,onoff);
				usleep(_500MS);
				break;
#endif
#ifdef CL_151
			case  COOP1_LIGHT:
				change_output(COOP1_LIGHTa,onoff);
				usleep(_500MS);
				break;

			case COOP1_HEATER:
				change_output(COOP1_HEATERa,onoff);
				usleep(_500MS);
				break;

			case COOP2_LIGHT:
				change_output(COOP2_LIGHTa,onoff);
				usleep(_500MS);
				break;

			case COOP2_HEATER:
				change_output(COOP2_HEATERa,onoff);
				usleep(_500MS);
				break;

			case OUTDOOR_LIGHT1:
				change_output(OUTDOOR_LIGHT1a,onoff);
				usleep(_500MS);
				break;

			case OUTDOOR_LIGHT2:
				change_output(OUTDOOR_LIGHT2a,onoff);
				usleep(_500MS);
				break;

			case UNUSED150_1:
				change_output(UNUSED150_1a,onoff);
				usleep(_500MS);
				break;

			case UNUSED150_2:
				change_output(UNUSED150_2a,onoff);
				usleep(_500MS);
				break;

			case UNUSED150_3:
				change_output(UNUSED150_3a,onoff);
				usleep(_500MS);
				break;

			case UNUSED150_4:
				change_output(UNUSED150_4a,onoff);
				usleep(_500MS);
				break;

			case UNUSED150_5:
				change_output(UNUSED150_5a,onoff);
				usleep(_500MS);
				break;

			case UNUSED150_6:
				change_output(UNUSED150_6a,onoff);
				usleep(_500MS);
				break;

			case UNUSED150_7:
				change_output(UNUSED150_7a,onoff);
				usleep(_500MS);
				break;

			case UNUSED150_8:
				change_output(UNUSED150_8a,onoff);
				usleep(_500MS);
				break;

			case UNUSED150_9:
				change_output(UNUSED150_9a,onoff);
				usleep(_500MS);
				break;
			case UNUSED150_10:
				change_output(UNUSED150_10a,onoff);
				usleep(_500MS);
				break;
#endif 
#ifdef CL_147
			case  CHICK_LIGHT:
				change_output(CHICK_LIGHTa,onoff);
				usleep(_500MS);
				break;

			case  CHICK_HEATER:
				change_output(CHICK_HEATERa,onoff);
				usleep(_500MS);
				break;

			case  BENCH_12V_1:
				change_output(BENCH_12V_1a,onoff);
				usleep(_500MS);
				break;

			case  BENCH_12V_2:
				change_output(BENCH_12V_2a,onoff);
				usleep(_500MS);
				break;

			case  BENCH_5V_1:
				change_output(BENCH_5V_1a,onoff);
				usleep(_500MS);
				break;

			case  BENCH_5V_2:
				change_output(BENCH_5V_2a,onoff);
				usleep(_500MS);
				break;

			case  BENCH_3V3_1:
				change_output(BENCH_3V3_1a,onoff);
				usleep(_500MS);
				break;

			case  BENCH_3V3_2:
				change_output(BENCH_3V3_2a,onoff);
				usleep(_500MS);
				break;

			case  BENCH_LIGHT1:
				change_output(BENCH_LIGHT1a,onoff);
				usleep(_500MS);
				break;

			case  BENCH_LIGHT2:
				change_output(BENCH_LIGHT2a,onoff);
				usleep(_500MS);
				break;

			case  BATTERY_HEATER:
				change_output(BATTERY_HEATERa,onoff);
				usleep(_500MS);
				break;
#endif 
#ifdef CL_154 
			case  CABIN_SOUTH:
				index = change_output(CABIN1a,onoff);
				usleep(_500MS);
				break;

			case  CABIN_FILECB:
				index = change_output(CABIN2a,onoff);	// currently these are just bare wires 
				usleep(_500MS);
				break;

			case  CABIN3:
				index = change_output(CABIN3a,onoff);
				usleep(_500MS);
				break;

			case  CABIN_KITCHEN:
				index = change_output(CABIN4a,onoff);
				usleep(_500MS);
				break;

			case  CABIN_DOOR:
				index = change_output(CABIN5a,onoff);
				usleep(_500MS);
				break;

			case  CABIN_EAST:
				index = change_output(CABIN6a,onoff);
				usleep(_500MS);
				break;

			case  CABIN7:
				index = change_output(CABIN7a,onoff);
				usleep(_500MS);
				break;

			case  CABIN8:
				index = change_output(CABIN8a,onoff);
				usleep(_500MS);
				break;
#endif
			case EXIT_TO_SHELL:
				snprintf(tempx, strlen(tempx), "exit to shell");
//				send_msg(strlen((char*)tempx)*2,(UCHAR*)tempx,REBOOT_IOBOX, _SERVER);
				uSleep(0,TIME_DELAY/16);
				//printf("tasks: exit to shell\n");
				close_program = 1;
//				reboot_on_exit = 1;
				break;

			case REBOOT_IOBOX:
				snprintf(tempx, strlen(tempx), "reboot iobox");
//				send_msg(strlen((char*)tempx)*2,(UCHAR*)tempx,REBOOT_IOBOX, _SERVER);
				uSleep(0,TIME_DELAY/16);
				//printf("tasks: reboot iobox\n");
				close_program = 1;
//				reboot_on_exit = 2;
				break;

			case SHUTDOWN_IOBOX:
				snprintf(tempx, strlen(tempx), "shutdown iobox");
//				send_msg(strlen((char*)tempx)*2,(UCHAR*)tempx,SHUTDOWN_IOBOX,_SERVER);
				uSleep(0,TIME_DELAY/16);
				//printf("tasks: shutdown iobox\n");
				close_program = 1;
//				reboot_on_exit = 3;
				break;

			case SHELL_AND_RENAME:
				snprintf(tempx, strlen(tempx), "shell and rename");
//				send_msg(strlen((char*)tempx)*2,(UCHAR*)tempx,SHUTDOWN_IOBOX,_SERVER);
				uSleep(0,TIME_DELAY/16);
				//printf("tasks: shell and rename\n");
				close_program = 1;
//				reboot_on_exit = 6;
				break;

			default:
				break;
		}	// end of switch

		if(close_program == 1)
		{
			//printf("stopping basic_controls_task\n");
			return 0;
		}

	}
	return 1;
}
/*********************************************************************/
/*
int avg_raw_data(int sample_size)
{
	int i;
	int temp_data = 0;
	int first_sample_index;
	D_DATA *dtp = (D_DATA *)malloc(sizeof(D_DATA));
	D_DATA **dtpp = &dtp;
	
	int actual_size = dllist_get_size(&dll);
	//printf("actual size: %d %d\n",actual_size,ds_index);
	if(sample_size > actual_size)
		sample_size = actual_size - 1;
	first_sample_index =  ds_index - sample_size;
	
	//printf("%d %d %d\n",first_sample_index, sample_size,ds_index);
	for(i = first_sample_index;i < ds_index-1;i++)
	{
		//printf("%d: \n",i);
		dllist_find_data(i, dtpp, &dll);
		//printf("%d:%d\n",temp_data,dtp->value);
		temp_data += dtp->value;
	}
	temp_data /= sample_size;
	free(dtp);
	return temp_data;
}
*/
/*********************************************************************/
float convertF(int raw_data)
{
	float T_F, T_celcius;
	int ret;
	if ((raw_data & 0x100) != 0)
	{
		raw_data = - (((~raw_data)+1) & 0xff); /* take 2's comp */
	}
	T_celcius = raw_data * 0.5;
	T_F = (T_celcius * 1.8) + 32;
	ret = (int)T_F;
	return ret;	// returns 257 -> -67
}
/*********************************************************************/
int convertFi(int raw_data)
{
	float fval,C,F;
	int ival;
	fval = (float)raw_data;

	if(fval >= 0.0 && fval <= 250.0)
	{
		C = fval/2.0;
		//tval = val + 109;

	}
	else if(fval >= 403 && fval <= 511)
	{
		C = (fval - 512.0)/2.0;
		//tval = val - 403;
	}
	F = C*9.0;
	F /= 5.0;
	F += 32.0;
	ival = (int)F;
	return ival;
}
