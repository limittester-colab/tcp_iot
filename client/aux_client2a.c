// aux_client2a.c - has to be run on whatever client the msg applies to 
// if msg is BENCH_LIGHT it has to be run a session of 147 so dest doesn't apply like 
// the aux_client2a in multi-client
#if 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h> // bzero()
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <assert.h>
#include <time.h>
#include <sys/time.h>
#include <ctype.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <sys/types.h>
#include "mytypes.h"
#include "cmd_types.h"

extern void *memcpy(void *dest, const void *src, size_t n);

typedef unsigned char UCHAR;
typedef unsigned int UINT;
typedef UCHAR* PUCHAR;
typedef unsigned long ULONG;

extern CMD_STRUCT cmd_array[];

#define SEND_CMD_HOST_QKEY	1303

struct msgqbuf
{
	long mtype;
	UCHAR mtext[200];
};

void print_cmd(UCHAR cmd)
{
	if(cmd > NO_CMDS)
		printf("unknown cmd: %d\n",cmd);
	printf("%d %s ",cmd, cmd_array[cmd].cmd_str);
}
#endif
/*********************************************************************/
int main(int argc, char **argv)
{
	int i;
	UCHAR dest;
	int sock_qid;
	key_t sock_key;
	UCHAR cmd;
	struct msgqbuf msg;
	int msg_len;
	UCHAR str[20];

	int msgtype = 1;
	msg.mtype = msgtype;

	sock_key = SEND_CMD_HOST_QKEY;
	sock_qid = msgget(sock_key, IPC_CREAT | 0666);

	if(argc < 3)
	{
		printf("usage: %s <cmd> <string>\n",argv[0]);
		printf("%s will do a cmd on this client with string param \n",argv[0]);
		exit(1);
	}
	cmd = atoi(argv[1]);
	print_cmd(cmd);
	memset(str,0,sizeof(str));
	strcpy(str,argv[2]);
	msg_len = strlen(str);
//	printf("len: %d string: %s\n", msg_len, str);
	msg.mtype = msgtype;
	memset(msg.mtext,0,sizeof(msg.mtext));
	msg.mtext[0] = cmd;
	msg.mtext[1] = (UCHAR)msg_len;
	msg.mtext[2] = (UCHAR)(msg_len >> 4);

	memcpy(&msg.mtext[3],str,msg_len);

//	printf("str_len: %d\n",strlen(str));
/*
	for(i = 0;i < msg_len;i++)
		printf("%c",msg.mtext[i+3]);
	printf("\n");

	for(i = 0;i < msg_len+5;i++)
		printf("%02x ",msg.mtext[i]);
	printf("\n");
*/
	if (msgsnd(sock_qid, (void *) &msg, sizeof(msg.mtext), MSG_NOERROR) == -1) 
	{
		printf("queue failed\n");
		perror("msgsnd error");
		exit(EXIT_FAILURE);
	}

    return 0;
}

