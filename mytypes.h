#ifndef MYTPYES_H
#define MYTPYES_H

typedef unsigned char UCHAR;
typedef unsigned int UINT;
typedef UCHAR* PUCHAR;
typedef unsigned long ULONG;

#define TIME_DELAY   990000000L

// this is the same as NUM_PORT_BITS (should be...)
#define NUM_DATA_RECS 20

#define NUM_DAT_NAMES 45
#define DAT_NAME_STR_LEN 25
//#define TDATE_STAMP_STR_LEN 25
#define TDATE_STAMP_STR_LEN 16
#define UPLOAD_BUFF_SIZE 200
#define NUM_ADC_CHANNELS 11
#define SERIAL_BUFF_SIZE 255
#define RAW_DATA_ARRAY_SIZE 15
#define MAX_CLIENTS	6
#define NO_CMDS 73
#define NO_CLLIST_RECS NUM_DATA_RECS
#define PASSWORD_SIZE 10
// format of message queue (msgsnd & msgrcv)

typedef struct
{
	int cmd;
	char cmd_str[30];
} CMD_STRUCT;

//CLIENT_TABLE client_table[MAX_CLIENTS];

#define OLABELSIZE 30
#define CLABELSIZE 15

typedef struct _ip
{
	int port;			// port which is controlled by the input
	int input;			// input port which controls output 
	int function;		// if this is > 0 then execute the function in cmd_array 
						// (CMD_STRUCT) instead of setting output 
	char label[OLABELSIZE];
}IP;

typedef struct params
{
	int ds_interval;
	int valid_ds[7];
	int ds_enable;
	int batt_box_temp;
	int test_bank;
	int password_timeout;
	int password_retries;
	int baudrate3;
}PARAM_STRUCT;

#endif
