#ifndef __TASKS_H
#define  __TASKS_H

typedef struct
{
	int i;
	int bank;
	int index;
}REAL_BANKS;

typedef struct 
{
	int sock;
	char client_name[30];
}CLIENT_NAME;

typedef struct
{
	pthread_t listen_thread;
	pthread_t read_queue_thread;
	key_t main_key;
	int qid;
	int sock;
	char client_name[30];
}THREADS;

struct msgqbuf
{
	long mtype;
	UCHAR mtext[300];
};

// global variables

#define DEFAULT                 0
#define TIME_SLICE              1
#define FIFO                    2
#define PRIOR                   3
#define PTIME_SLICE             4
#define PFIFO                   5
#define INHERIT                 6
#define MAIN_KEY				1303
#define BASIC_CONTROLS_KEY		1400
#define PROTOPORT				5193				  /* default protocol port number */
#define QLEN					6					  /* size of request queue        */
#define MAX_THREADS				6
int uSleep(time_t sec, long nanosec);

#define _1SEC	 	1000000 
#define _100MS		100000 	
#define _10MS		10000 	
#define _1MS		1000 	

#define _2SEC	 	2000000 
#define _200MS		200000 	
#define _20MS		20000 	
#define _2MS		2000 	

#define _5SEC	 	5000000 
#define _500MS		500000 	
#define _50MS		50000 	
#define _5MS		5000 	

static int main_qid;
static key_t main_key;
static int basic_controls_qid;
static key_t basic_controls_key;
static int close_program;
static int this_client_id;
UCHAR get_host_cmd_task(int *test);
UCHAR get_host_cmd_task2(int *test);
UCHAR basic_controls_task(int *test);

#ifndef SERVER_146
#warning "SERVER_146 not defined"
#define NUM_SOCK_TASKS			3
#define NUM_SCHED_TASKS			8

UCHAR recv_msg_task(int test);
//void send_serialother2(UCHAR cmd, int size, UCHAR *buf);
int put_sock(UCHAR *buf,int buflen, int block, char *errmsg);
int get_sock(UCHAR *buf, int buflen, int block, char *errmsg);
void set_sock(int open);
void send_msg(int msg_len, UCHAR *msg, UCHAR msg_type, UCHAR dest);
void *work_routine(void *arg);
int send_tcp(UCHAR *str,int len);
int recv_tcp(UCHAR *str, int strlen,int block);
int test_sock(void);
int get_msg(void);

//UCHAR upload_buf[UPLOAD_BUFF_SIZE];
#endif

#ifdef SERVER_146
#warning "SERVER_146 defined"


// params for usleep()

// uSleep(0,100000000L); - roughly 100ms using uSleep();

int put_sock(int sd, UCHAR *buf,int buflen, int block, char *errmsg);
int get_sock(int sd, UCHAR *buf, int buflen, int block, char *errmsg);
void send_msg(int sd, int msg_len, UCHAR *msg, UCHAR msg_type);
int get_msgb(int sd);
void send_msgb(int sd, int msg_len, UCHAR *msg, UCHAR msg_type);
int send_tcp(int sd, UCHAR *str,int len);
int recv_tcp(int sd, UCHAR *str, int strlen,int block);
int get_msg(int sd);

#endif

#endif
