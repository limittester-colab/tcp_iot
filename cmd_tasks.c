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
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <dirent.h> 
#include <time.h>
#include "cmd_types.h"
#include "mytypes.h"
#include "ioports.h"
//#include "serial_io.h"
#include "queue/ollist_threads_rw.h"
//#include "queue/cllist_threads_rw.h"
//#include "queue/dllist_threads_rw.h"
//#include "queue/sllist_threads_rw.h"
#include "tasks.h"
//#include "raw_data.h"
#include "cs_client/config_file.h"

int trunning_days, trunning_hours, trunning_minutes, trunning_seconds;
int trunning_seconds_off;

static struct  sockaddr_in sad;  /* structure to hold server's address  */
#define TOGGLE_OTP otp->onoff = (otp->onoff == 1?0:1)

static CMD_STRUCT cmd_array[];
REAL_BANKS real_banks[40];

enum client_list	// if adding to this list, change MAX_CLIENTS above 
{
	_158,			// WINDOWS-11A (runs on Windows machine)
	_154,			// Cabin
	_147,			// Testbench
	_243,			// aux_client (runs on a PC linux box)
	_151,			// extra TS-4600 card
	_146,			// Garage 
	_SERVER			// 239 (Raspberry Pi)
}CLIENT_LIST;

//UCHAR msg_buf[SERIAL_BUFF_SIZE];

inline int pack4chars(char c1, char c2, char c3, char c4) {
    return ((int)(((unsigned char)c1) << 24)
            |  (int)(((unsigned char)c2) << 16)
            |  (int)(((unsigned char)c3) << 8)
            |  (int)((unsigned char)c4));
}
#endif

void print_cmd(UCHAR cmd)
{
	if(cmd >= NO_CMDS)
	{
		printf("unknown cmd: %d\n",cmd);
	}else printf("%d %s\n",cmd, cmd_array[cmd].cmd_str);
}
/*********************************************************************/
void add_msg_queue(UCHAR cmd, UCHAR onoff)
{
	struct msgqbuf msg;
	int msgtype = 1;
	msg.mtype = msgtype;
	msg.mtext[0] = cmd;
	msg.mtext[1] = onoff;
//	pthread_mutex_lock(&msg_queue_lock);

	if (msgsnd(basic_controls_qid, (void *) &msg, sizeof(msg.mtext), MSG_NOERROR) == -1) 
	{
		// keep getting "Invalid Argument" - cause I didn't set the mtype
		perror("msgsnd error");
		exit(EXIT_FAILURE);
	}
//	printf("add_msg_queue\n");
//	pthread_mutex_unlock(&msg_queue_lock);
//	printf("add: %d %x\r\n",msg_queue_ptr,cmd);
}
/*********************************************************************/
// task to get commands from the sock
UCHAR get_host_cmd_task(int *test)
{
	O_DATA *otp;
	O_DATA **otpp = &otp;
//	C_DATA *ctp;
//	C_DATA **ctpp = &ctp;
//	D_DATA *dtp;
//	D_DATA **dtpp = &dtp;
//	C_DATA *cttp;
	int rc = 0; 
	UCHAR cmd = 0x21;
	UCHAR onoff;
	char errmsg[50];
	char filename[15];
	char oFileName[20];
	int i;
	int j;
	int k;
	int min_idx;
	size_t csize;
	size_t osize;
	size_t dsize;
	size_t ssize;
	UCHAR tempx[SERIAL_BUFF_SIZE];
	char temp_time[5];
	char *pch, *pch2;
	time_t curtime2;
	time_t *pcurtime2 = &curtime2;
	off_t fsize;
	struct timeval mtv;
	struct tm t;
	struct tm tm;
	time_t T;
	struct tm *pt = &t;
	int msg_len;
	char version[15] = "sched v1.03\0";
	UINT utemp;
//	next_client = 0;
	char label[30];
	int index;
	UCHAR mask;
	UCHAR mask2;
	int sample_size = 10;
	struct msgqbuf msg;		// this has to be shared by send_sock_msg & get_host_cmd_task
	int msgtype = 1;
	float F,C, fval; 
	int ival;
	DIR *d;
	struct dirent *dir;
	struct stat st;

#ifdef SERVER_146
	printf("starting server\n");
	this_client_id = _SERVER;
#endif
#ifdef CLIENT_146
	printf("starting 146 (garage)\n");
	this_client_id = _146;
#endif
#ifdef CL_147
	printf("starting 147 (studio)\n");
	this_client_id = _147;
#endif 
#ifdef CL_154
	printf("starting 154 (cabin)\n");
	this_client_id = _154;
#endif 
#ifdef CL_151
	printf("starting 151 (outdoors)\n");
	this_client_id = _151;
#endif 
#if 1
	// since each card only has 20 ports then the 1st 2 port access bytes
	// are 8-bit and the 3rd is only 4-bits, so we have to translate the
	// inportstatus array, representing 3 byts of each 2 (3x8x2 = 48) to
	// one of the 40 actual bits as index
	// 2/15/23 - working with 4600 w/ just one card - 

	// the check_inputs & change_outputs functions
	// use the array to adjust from index to bank
	// since there are only 4 bits in banks 3 & 5
	//printf("starting...\n");

	real_banks[0].i = 0;
	real_banks[0].bank = 0;
	real_banks[0].index = 0;

	real_banks[1].i = 1;
	real_banks[1].bank = 0;
	real_banks[1].index = 1;

	real_banks[2].i = 2;
	real_banks[2].bank = 0;
	real_banks[2].index = 2;

	real_banks[3].i = 3;
	real_banks[3].bank = 0;
	real_banks[3].index = 3;

	real_banks[4].i = 4;
	real_banks[4].bank = 0;
	real_banks[4].index = 4;

	real_banks[5].i = 5;
	real_banks[5].bank = 0;
	real_banks[5].index = 5;

	real_banks[6].i = 6;
	real_banks[6].bank = 0;
	real_banks[6].index = 6;

	real_banks[7].i = 7;
	real_banks[7].bank = 0;
	real_banks[7].index = 7;

	real_banks[8].i = 8;
	real_banks[8].bank = 1;
	real_banks[8].index = 0;

	real_banks[9].i = 9;
	real_banks[9].bank = 1;
	real_banks[9].index = 1;

	real_banks[10].i = 10;
	real_banks[10].bank = 1;
	real_banks[10].index = 2;

	real_banks[11].i = 11;
	real_banks[11].bank = 1;
	real_banks[11].index = 3;

	real_banks[12].i = 12;
	real_banks[12].bank = 1;
	real_banks[12].index = 4;

	real_banks[13].i = 13;
	real_banks[13].bank = 1;
	real_banks[13].index = 5;

	real_banks[14].i = 14;
	real_banks[14].bank = 1;
	real_banks[14].index = 6;

	real_banks[15].i = 15;
	real_banks[15].bank = 1;
	real_banks[15].index = 7;

	real_banks[16].i = 16;
	real_banks[16].bank = 2;
	real_banks[16].index = 0;

	real_banks[17].i = 17;
	real_banks[17].bank = 2;
	real_banks[17].index = 1;

	real_banks[18].i = 18;
	real_banks[18].bank = 2;
	real_banks[18].index = 2;

	real_banks[19].i = 19;
	real_banks[19].bank = 2;
	real_banks[19].index = 3;

/*
	for(i = 0;i < 20;i++)
	{
		real_banks[i].i = i;
		real_banks[i].bank = i/8;
		real_banks[i].index = i - real_banks[i].bank*8;
	}

	for(i = 20;i < 40;i++)
	{
		real_banks[i].i = i;
		real_banks[i].bank = (i+4)/8;
		real_banks[i].index = i - (real_banks[i].bank*8)+4;
	}

	i = NUM_PORT_BITS;
	isize = sizeof(I_DATA);
	isize *= i;
*/
	i = NUM_PORT_BITS;
	//printf("no. port bits: %d\r\n",i);
	osize = sizeof(O_DATA);
	osize *= i;
	//printf("osize: %d\r\n",osize);
	i = NO_CLLIST_RECS;
	//printf("no. port bits: %d\r\n",i);
//	csize = sizeof(C_DATA);
	csize *= i;

	trunning_days = trunning_hours = trunning_minutes = trunning_seconds = 0;
/*
	trunning_days = 1;
	trunning_hours = 5;
	trunning_minutes = 14;
	trunning_seconds = 0;
*/
#endif
//	program_start_time = curtime();
	printf("starting host: %d\n",this_client_id);
	ollist_init(&oll);
	strcpy(oFileName, "odata.dat\0");
	if(access(oFileName,F_OK) != -1)
	{
		olLoadConfig(oFileName,&oll,osize,errmsg);
		if(rc > 0)
		{
			printf("%s\r\n",errmsg);
		}
	}
	msg.mtype = msgtype;

	while(TRUE)
	{
		cmd = 0;
		if(close_program == 1)
		{
			printf("shutting down cmd host\r\n");
			//free(stp);
			return 0;
		}
//		uSleep(0,TIME_DELAY/10);abort
		// msg from sock 
//		printf("\n..\n");
		msg.mtype = msgtype;
		printf("start msgrcv\n");
		if (msgrcv(main_qid, (void *) &msg, sizeof(msg.mtext), msgtype, MSG_NOERROR) == -1) 
		{
			if (errno != ENOMSG) 
			{
				perror("msgrcv");
				printf("msgrcv error\n");
				exit(EXIT_FAILURE);
			}
			printf("%d\n",errno);
		}
		cmd = msg.mtext[0];
		print_cmd(cmd);
		msg_len = (int)msg.mtext[1];
		msg_len |= (int)(msg.mtext[2] << 4);
//		printf("sched cmd host %d\n",msg_len);

		//printf("msg_len: %d\n",msg_len);
		memset(tempx,0,sizeof(tempx));
		memcpy(tempx,&msg.mtext[3],msg_len);
		tempx[msg_len + 1] = 0;

		if(cmd > 0)
		{
			rc = 0;

			switch(cmd)
			{
#ifdef CLIENT_146
				case DESK_LIGHT:
				case EAST_LIGHT:
				case NORTHWEST_LIGHT:
				case SOUTHEAST_LIGHT:
				case MIDDLE_LIGHT:
				case WEST_LIGHT:
				case NORTHEAST_LIGHT:
				case SOUTHWEST_LIGHT:
				case WATER_HEATER:
				case WATER_PUMP:
				case WATER_VALVE1:
				case WATER_VALVE2:
				case WATER_VALVE3:
#endif
#ifdef CL_151
#warning "CL_151 defined"
				case COOP1_LIGHT:
				case COOP1_HEATER:
				case COOP2_LIGHT:
				case COOP2_HEATER:
				case OUTDOOR_LIGHT1:
				case OUTDOOR_LIGHT2:
				case UNUSED150_1:
				case UNUSED150_2:
				case UNUSED150_3:
				case UNUSED150_4:
				case UNUSED150_5:
				case UNUSED150_6:
				case UNUSED150_7:
				case UNUSED150_8:
				case UNUSED150_9:
				case UNUSED150_10:
#endif
#ifdef CL_147
#warning "CL_147 defined"
				case CHICK_LIGHT:
				case CHICK_HEATER:
				case BENCH_12V_1:
				case BENCH_12V_2:
				case BENCH_5V_1:
				case BENCH_5V_2:
				case BENCH_3V3_1:
				case BENCH_3V3_2:
				case BENCH_LIGHT1:
				case BENCH_LIGHT2:
				case BATTERY_HEATER:
#endif 
#ifdef CL_154
#warning "CL_154 defined"
				case CABIN_SOUTH:
				case CABIN_FILECB:
				case CABIN3:
				case CABIN_KITCHEN:
				case CABIN_DOOR:
				case CABIN_EAST:
				case CABIN7:
				case CABIN8:
#endif 
				case SHUTDOWN_IOBOX:
				case REBOOT_IOBOX:
				case SHELL_AND_RENAME:
				case EXIT_TO_SHELL:

					if(strncmp("ON",tempx,2) == 0)
						onoff = 1;
					else if(strncmp("OFF",tempx,3) == 0)
						onoff = 0;
					else onoff = 255;

					if(onoff == 1 || onoff == 0)
						add_msg_queue(cmd, onoff);
					else printf("onoff = -1 %0c\n",onoff);
					break;
				default:
					break;
			}

			if(cmd == SHELL_AND_RENAME || cmd == REBOOT_IOBOX || cmd == SHUTDOWN_IOBOX || cmd == EXIT_TO_SHELL)
			{
				printf("sending shutdown send sock msg: ");
				//print_cmd(cmd);
//				send_sock_msg(tempx, 1, cmd, _SERVER);
//				WriteParams("config.bin", &ps, password, errmsg);
				return 1;
			}

			switch(cmd)
			{
/*
				case SEND_MESSAGE2:
					for(i = 0;i < msg_len;i++)
						printf("%c",tempx[i]);
					break;
*/
				case SET_PROPERTIES:
					sprintf(label,"%s %s",tempx, cmd_array[cmd].cmd_str);
					printf("label: %s\n",label);
					msg_len = strlen(label);
					printf("len: %d\n",msg_len);
//					send_sock_msg(label, msg_len, SET_PROPERTIES, _158);
					break;

				case TURN_ALL_LIGHTS_OFF:
					//printf("%02x %02x\n",tempx[0], tempx[1]);
					trunning_seconds_off = (tempx[0] << 8) | tempx[1];
					//printf("trunning_seconds_off: %d\n",trunning_seconds_off);
					break;
#if 0
				case GET_TEMP4:
					cmd = DS1620_MSG;
//					for(i = 0;i < msg_len;i++)
//						printf("%c",tempx[i]);
					
//					printf("\n");
					for(i = 0;i < msg_len;i++)
					{
						dFileName[i] = tempx[i];
					}
					printf("%s\n",dFileName);
					printf("\n");
					
					if(access(dFileName,F_OK) != -1)
					{
						dlLoadConfig(dFileName,&dll,dsize,errmsg);
						if(rc > 0)
						{
							printf("%s\r\n",errmsg);
						}
						dllist_show(&dll);
					}else
					{
						memset(dtp,0,sizeof(D_DATA));
						printf("can't open %s\n",dFileName);
						break;
					}
					ds_index = dllist_get_size(&dll);

					for(i = 0;i < ds_index;i++)
					{
						dllist_find_data(i, dtpp, &dll);
						ival = convertFi(dtp->value);

						sprintf(tempx, "%d %0d %02d:%02d %d",this_client_id, dtp->sensor_no, dtp->hour, dtp->minute, ival);
						//sprintf(tempx,"%d:%d:%d - %sxxx",dtp->hour, dtp->minute, dtp->second, lookup_raw_data(dtp->value));
//						send_sock_msg(tempx, strlen(tempx), cmd, _158);
						//printf("test: %s\n",tempx);
						//uSleep(0,TIME_DELAY);
						uSleep(0,TIME_DELAY/4);
					}
					break;

				case UPDATE_CLIENT_TABLE:
					printf("update client table in cmd_tasks.c");
//					send_sock_msg(tempx, msg_len, SEND_CLIENT_LIST, _SERVER);
					break;

				case DLLIST_SAVE:
					dlWriteConfig("ddata.dat", &dll, index, errmsg);
					ds_reset = 1;
					break;

				case DLLIST_SHOW:
					printf("dllist show\n");
					dllist_show(&dll);
					break;

				case SET_DS_INTERVAL:
					ps.ds_interval = (int)tempx[0];
					if(ps.ds_interval == 8)
						ps.ds_enable = 0;
					else ps.ds_enable = 1;
					printf("ds interval: %d\n",ps.ds_interval);
					break;

				case SET_VALID_DS:
					printf("set valid ds: %d\n",tempx[0]);
					mask = 1;
					for(i = 0;i < 7;i++)
						ps.valid_ds[i] = 0;
					for(i = 0;i < 6;i++)
					{
						if((mask & tempx[0]) == mask)
							ps.valid_ds[i] = 1;
						mask <<= 1;
					}
					break;

				case SET_NEXT_CLIENT:
					next_client = tempx[0];
					if(next_client == 8)
					{
						next_client = 0;
						printf("stop\n");
					}else printf("next client: %s\n", client_table[next_client].label);
					j = 0;
					break;

				case SEND_NEXT_CLIENT:
					cmd = 0x21;
					for(i = 0;i < SERIAL_BUFF_SIZE;i++)
					{
						tempx[i] = cmd;
						if(++cmd > 0x7e)
							cmd = 0x21;
					}
					uSleep(0,TIME_DELAY/10);
					j++;
					if(j > 10)
						j = 0;
					break;
#endif
#if 1

				case SEND_TIMEUP:
					memset(tempx,0,sizeof(tempx));
					sprintf(tempx,"%d %d %d %d %d",this_client_id, trunning_days, trunning_hours, trunning_minutes, trunning_seconds);
					//printf("send timeup: %s\n",tempx);
					msg_len = strlen(tempx);
					uSleep(0,TIME_DELAY/4);
//					send_sock_msg(tempx, msg_len, UPTIME_MSG, _SERVER);
					break;

				case UPTIME_MSG:
					//printf("uptime msg: %s\n",tempx);
//					send_sock_msg(tempx, msg_len, UPTIME_MSG, _SERVER);
					break;

				case SEND_STATUS:
					k++;
					j += 10;

					memset(tempx,0,sizeof(tempx));
					tempx[0] = (UCHAR)k;
					tempx[1] = (UCHAR)(k >> 4);
					tempx[2] = (UCHAR)j;
					tempx[3] = (UCHAR)(j >> 4);
					tempx[4] = 0;
					cmd = SEND_MESSAGE;
					sprintf(tempx,"k: %d j: %dxxx",k,j);
					msg_len = strlen(tempx);
//					send_sock_msg(tempx, msg_len, cmd, _158);
					printf("%s\n",tempx);
					break;

				case SEND_MESSAGE2:
					//printf("SEND_MESSAGE\n");

					for(i = 0;i < msg_len;i++)
						printf("%c",tempx[i]);
					printf("\n");

//					send_sock_msg(tempx, msg_len, cmd, _158);
					break;

				case SET_TIME:	// could this be easier with strtok() ?
#ifndef SERVER_146
//					printf("set time\n");
					curtime2 = 0L;
					printf("%s\n",tempx);
					j = 0;

//						for(i = 2;i < msg_len;i+=2)
//							memcpy((void*)&tempx[j++],(char*)&msg_buf[i],1);
/*
					for(i = 0;i < msg_len/2+2;i++)
					{
						tempx[i] = msg_buf2[i];
//							write_serial2(tempx[i]);
					}
*/
					//tempx[msg_len-2] = 'M';
					memset(temp_time,0,sizeof(temp_time));
					i = 0;
					pch = &tempx[0];

					while(*(pch++) != '/' && i < msg_len)
					{
						i++;
//						printf("%c",*pch);
					}
					memcpy(&temp_time[0],&tempx[0],i);
					i = atoi(temp_time);
//					printf("\nmon: %d\n",i - 1);
					pt->tm_mon = i - 1;
					i = 0;

					while(*(pch++) != '/' && i < msg_len)
					{
						i++;
//						printf("%c",*pch);
					}
					memset(temp_time,0,sizeof(temp_time));
					memcpy(temp_time,pch-i-1,i);
//					printf("%s\n",temp_time);
					i = atoi(temp_time);
					pt->tm_mday = i;
//					printf("day: %d\r\n",i);
			//		return 0;

					i = 0;
					while(*(pch++) != ' ' && i < msg_len)
					{
						i++;
//						printf("%c\r\n",*pch);
					}

					memset(temp_time,0,sizeof(temp_time));
					memcpy(temp_time,pch-3,2);
					i = atoi(temp_time);
					i += 100;
					pt->tm_year = i;
//					printf("year: %d\r\n",i-100);
			//		return 0;
					i = 0;

					while(*(pch++) != ':' && i < msg_len)
						i++;
					memset(temp_time,0,sizeof(temp_time));
					memcpy(temp_time,pch-i-1,i);
//					printf("%s \n",temp_time);
					i = atoi(temp_time);
					pt->tm_hour = i;
//					printf("hour: %d\r\n",i);
			//		return 0;

					i = 0;
					while(*(pch++) != ':' && i < msg_len)
						i++;
					memset(temp_time,0,sizeof(temp_time));
					memcpy(temp_time,pch-3,2);
//					printf("%s \n",temp_time);
					i = atoi(temp_time);
					pt->tm_min = i;
//					printf("min: %d\r\n",i);

					i = 0;
					while(*(pch++) != ' ' && i < msg_len)
						i++;
					memset(temp_time,0,sizeof(temp_time));
					memcpy(temp_time,pch-3,2);
//					printf("%s \n",temp_time);
					i = atoi(temp_time);
					pt->tm_sec = i;
//					printf("sec: %d\r\n",i);
//					printf("%c %x\n",*pch,*pch);
					if(*pch == 'P')
					{
//						printf("PM\n");
						if(pt->tm_hour != 12)
							pt->tm_hour += 12;
					}else if(*pch == 'A' && pt->tm_hour == 12)
						pt->tm_hour -= 12;
//					printf("hour: %d\n",pt->tm_hour);

					curtime2 = mktime(pt);
// the following won't compile on Rpi (stime)
					stime(pcurtime2);
					uSleep(0,TIME_DELAY/3);
					gettimeofday(&mtv, NULL);
					curtime2 = mtv.tv_sec;
					strftime(tempx,30,"%m-%d-%Y %T\0",localtime(&curtime2));
					printf("%s\n",tempx);
#endif
					break;

				case GET_TIME:
					gettimeofday(&mtv, NULL);
					curtime2 = mtv.tv_sec;
					//strftime(tempx,30,"%m-%d-%Y %T\0",localtime(&curtime2));
					//printf(tempx);
					//strftime(tempx,30,"%H",localtime(&curtime2));  // show as 24-hour (00 -> 23)
					//printf(tempx);
					//printf("\n");
					//strftime(tempx,30,"%I",localtime(&curtime2));	// show as 12-hour (01 -> 12)
					//printf(tempx);
					//printf("\n");
					T = time(NULL);
					tm = *localtime(&T);
					memset(tempx,0,sizeof(tempx));
					sprintf(tempx,"%02d:%02d:%02d", tm.tm_hour, tm.tm_min, tm.tm_sec);
					printf("%s\n",tempx);
					msg_len = strlen(tempx);
					//printf("msg_len: %d\n",msg_len);
					cmd = SEND_MESSAGE;
//					send_sock_msg(tempx, msg_len, cmd, _158);
					break;

				case BAD_MSG:
//						close_program = 1;
					break;

				case DISCONNECT:
/*
					if(test_sock() > 0)
					{
						close_tcp();
						printf("disconnected\0");
					}
*/
					break;

				case GET_CONFIG2:
/*
					printf("ds_interval: %d\n",ps.ds_interval);
					printf("valid: \n");
					for(i = 0;i < 7;i++)
						printf("%d ",ps.valid_ds[i]);
					printf("\nenabled: %d\n",ps.ds_enable);
*/
					break;
#endif
				default:
					//printf("default in main loop\n");
					break;
			}								  // end of switch
		}									  // if rc > 0
	}
	return test + 1;
}