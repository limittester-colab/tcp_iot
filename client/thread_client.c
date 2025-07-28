#if 1
#include <arpa/inet.h> // inet_addr()
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h> // bzero()
#include <sys/socket.h>
#include <unistd.h> // read(), write(), close()
#include<pthread.h> //for threading , link with lpthread
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "cmd_types.h"
#include "mytypes.h"
#include "tasks.h"

#define MAX 80
#define PORT 5193
#define SA struct sockaddr

static int global_socket;

void *listen_thread(void *);
void *send_queue_thread(void *);
pthread_t plisten_thread;
pthread_t queue_thread;
pthread_t cmd_task_thread;
pthread_t pbasic_controls_task_thread;

static UCHAR pre_preamble[] = {0xF8,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0x00};
#endif

/****************************************************************************************/
void *send_queue_thread(void *buff)
{
	struct msgqbuf msg;
	int msgtype = 1;
	printf("queue thread started\n");
	ssize_t ret = 1;
	UCHAR cmd;
	int msg_len;
	UCHAR dest = 0;
	UCHAR tempx[100];
	int i;
	
	for(;;)
	{
		memset(msg.mtext,0,sizeof(msg.mtext));
		ret = msgrcv(main_qid, (void *) &msg, sizeof(msg.mtext), msgtype, MSG_NOERROR);
		printf("ret: %ld errno: %d\n",ret,errno);
		if(ret == -1)
		{
			if(errno != ENOMSG)
			{
				perror("msgrcv");
				printf("msgrcv error\n");
				exit(EXIT_FAILURE);
			}
		}
//		printf("read queue thread: %s\n",msg.mtext);
		cmd = msg.mtext[0];
//		printf("cmd: %d\n",cmd);
		// msg is low byte of msg_len 1st
		// then high byte

		msg_len = (int)msg.mtext[1];
		msg_len |= (int)(msg.mtext[2] << 4);
		memset(tempx,0,sizeof(tempx));
		memcpy(tempx,msg.mtext + 3,msg_len);
		printf("cmd: %d msg_len: %d\n",cmd, msg_len);
		printf("%s\n",tempx);

		for(i = 0;i < msg_len+3;i++)
		{
			printf("%02x ",msg.mtext[i]);
		}
		printf("\n");
	}
}

/****************************************************************************************/
void *listen_thread(void *socket_desc)
{
	char tempx[200];
	int msg_len;
	int ret;
	UCHAR cmd;
	int i;
	struct msgqbuf msg;
	msg.mtype = 1;

	//Get the socket descriptor
	int read_size;
	
	//Receive a message from client
//	printf("start %d\n", global_socket);
	msg_len = 1;
	printf("main_qid: %d\n",main_qid);

	while(msg_len > 0)
	{
//		printf("sock: %d\n",global_socket);
		msg_len = get_msg();
//		printf("msg_len: %d\n",msg_len);
		if(msg_len > 0)
		{
	//		printf("msg_len: %d\n",msg_len);
			ret = recv_tcp(&tempx[0],msg_len+1,1);
	//		printf("\n\nret: %d msg_len: %d\n",ret,msg_len);
	//		currently the ret is just 1 more than msg_len 
			cmd = tempx[0];
			if(cmd == 99)
			{
				printf("closing program\n");
				memset(tempx,0,sizeof(tempx));
				send_msg(0, tempx,cmd, 0);
				msg_len = 0;
				pthread_kill(plisten_thread,NULL);
				close(global_socket);
				close_program = 1;
				return;
			}

	/*
			for(i = 0;i < msg_len;i++)
				printf("%02x ",tempx[i]);
			printf("\n");

			for(i = 1;i < msg_len+2;i++)
				printf("%02x ",tempx[i]);
			printf("\n");
	*/
			for(i = 1;i < msg_len+1;i++)
				printf("%c",tempx[i]);
			printf("\n");

//			print_cmd(cmd);

			memmove(tempx,tempx+1,msg_len);
			//printf("\n");
/*
			for(i = 0;i < msg_len;i++)
				printf("%02x ",tempx[i]);
*/
//			printf("\n");

			uSleep(1,0);
			memset(msg.mtext,0,sizeof(msg.mtext));
			msg.mtext[0] = cmd;
			msg.mtext[1] = (UCHAR)msg_len;
			msg.mtext[2] = (UCHAR)(msg_len >> 4);
			memcpy(msg.mtext + 3,tempx,msg_len);
//			for(i = 0;i < msg_len + 3;i++)
//				printf("%02x ",msg.mtext[i]);
//			printf("\n");
			ret = msgsnd(main_qid, (void *) &msg, sizeof(msg.mtext), MSG_NOERROR);
			if(ret == -1)
			{
				perror("msgsnd error");
			}
//			printf("msgsnd ret: %d\n\n",ret);
		}
	}

	if(msg_len == 0)
	{
		puts("Client disconnected");
	}
	else if(msg_len == -1)
	{
		perror("recv failed");
	}
		
	return 0;
}
/****************************************************************************************/
int main(int argc, char *argv[])
{
    int sockfd, connfd;
    struct sockaddr_in servaddr, cli;
	char buff[200];
	char buff2[20];
	int n = 0;
	char client_name[30];
	close_program = 0;
	int test;

	if(argc != 2)
	{
		printf("must enter the name of the client so it can be sent to the server\n");
		exit(1);
	}

	main_key = MAIN_KEY;
	main_qid = msgget(main_key, IPC_CREAT | 0666);
	if(main_qid < 0)
	{
		printf("errno: %d\n",errno);
		exit(1);
	}
	printf("main_qid: %d\n",main_qid);

	basic_controls_key = BASIC_CONTROLS_KEY;
	basic_controls_qid = msgget(basic_controls_key, IPC_CREAT | 0666);
	if(basic_controls_qid < 0)
	{
		printf("errno: %d\n",errno);
		exit(1);
	}
	printf("basic_controls_qid: %d\n",main_qid);

	strcpy(buff2,"test send\0");
	memset(client_name, 0, sizeof(client_name));
	printf("%s\n",argv[1]);
	strcpy(client_name, argv[1]);
	printf("client name: %s\n",client_name);
    // socket create and verification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("socket creation failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully created..\n");
    bzero(&servaddr, sizeof(servaddr));

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("192.168.88.239");
    servaddr.sin_port = htons(PORT);

    // connect the client socket to server socket
    if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr))
        != 0) {
        printf("connection with the server failed...\n");
        exit(0);
    }
    else
        printf("connected to the server..\n");
	global_socket = sockfd;
        n = 0;
//	when client logs in, it sends the socket (as an int)
//	then it sends 30 bytes as the name of the client (can be padded with zeros)		

//	getchar();
	printf("start...\n");
	
	if( pthread_create( &plisten_thread , NULL ,  listen_thread , (void*) global_socket) < 0)
	{
		perror("could not create thread");
		return 1;
	}
/*
	if( pthread_create( &queue_thread , NULL ,  send_queue_thread , (void*) buff2) < 0)
	{
		perror("could not create thread");
		return 1;
	}
*/
	if( pthread_create( &cmd_task_thread , NULL ,  get_host_cmd_task , (void*) test) < 0)
	{
		perror("could not create thread");
		return 1;
	}

	if( pthread_create( &pbasic_controls_task_thread , NULL ,  basic_controls_task , (void*) test) < 0)
	{
		perror("could not create thread");
		return 1;
	}
	// send the client name to the server 
	send_msg(30, client_name, 0, 0);

	while(close_program == 0)
	{
		uSleep(1,0);
	}
	return 0;
	if(pthread_join(plisten_thread, NULL) != 0)
	{
		perror("pthread_join plisten_thread");
		exit(1);
	}
	if(pthread_join(queue_thread, NULL) != 0)
	{
		perror("pthread_join queue_thread");
		exit(1);
	}
	printf("1 closing program\n");
    // close the socket
}
#if 1
int test_sock(void)
{
	return 1;
}
/*********************************************************************/
// get preamble & msg len from client
// preamble is: {0xF8,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0x00,
// msg_len(lowbyte),msg_len(highbyte),0x00,0x00,0x00,0x00,0x00,0x00}
// returns message length
int get_msg(void)
{
	int len;
	UCHAR low, high;
	int ret;
	int i;

	UCHAR preamble[10];
	ret = recv_tcp(preamble,8,1);
//	printf("ret: %d\n",ret);
	if(ret < 0)
	{
		printf("ret < 0");
	}
	if(memcmp(preamble,pre_preamble,8) != 0)
	{
		printf("bad preamble\n");
//		uSleep(2,0);
		return -1;
	}
	ret = recv_tcp(&low,1,1);
	ret = recv_tcp(&high,1,1);
//	printf("%02x %02x\n",low,high);
	len = 0;
	len = (int)(high);
	len <<= 4;
	len |= (int)low;

	return len;
}
/*********************************************************************/
// send the preamble, msg len, msg_type & dest (dest is index into client table)
void send_msg(int msg_len, UCHAR *msg, UCHAR msg_type, UCHAR dest)
{
	int ret;
	int i;
	UCHAR temp[2];

//	if(dest > MAX_CLIENTS)
//		return;

	if(test_sock())
	{
		ret = send_tcp(&pre_preamble[0],8);
		temp[0] = (UCHAR)(msg_len & 0x0F);
		temp[1] = (UCHAR)((msg_len & 0xF0) >> 4);
//		printf("%02x %02x\n",temp[0],temp[1]);
		send_tcp((UCHAR *)&temp[0],1);
		send_tcp((UCHAR *)&temp[1],1);
		send_tcp((UCHAR *)&msg_type,1);
		send_tcp((UCHAR *)&dest,1);

		for(i = 0;i < msg_len;i++)
		{
			send_tcp((UCHAR *)&msg[i],1);
//			send_tcp((UCHAR *)&ret,1);
		}
//		printf("%d ",msg_len);
	}
}
/*********************************************************************/
int recv_tcp(UCHAR *str, int strlen,int block)
{
	int ret = 0;
	char errmsg[20];
	memset(errmsg,0,20);
	if(test_sock())
	{
//		pthread_mutex_lock( &tcp_read_lock);
		ret = get_sock(str,strlen,block,&errmsg[0]);
		//printf("ret: %d\n",ret);
//		pthread_mutex_unlock(&tcp_read_lock);
		if(ret < 0 && (strcmp(errmsg,"Success") != 0))
		{
			printf(errmsg);
		}
	}
	else
	{
		strcpy(errmsg,"sock closed");
		printf(errmsg);
		ret = -1;
	}
	return ret;
}
/*********************************************************************/
int send_tcp(UCHAR *str,int len)
{
	int ret = 0;
	char errmsg[30];
	memset(errmsg,0,30);
	//pthread_mutex_lock( &tcp_write_lock);
	ret = put_sock(str,len,1,&errmsg[0]);
	//pthread_mutex_unlock(&tcp_write_lock);
	if(ret < 0 && (strcmp(errmsg,"Success") != 0))
	{
		printf(errmsg);
	}
	return ret;
}
/*********************************************************************/
int put_sock(UCHAR *buf,int buflen, int block, char *errmsg)
{
	int rc = 0;
	char extra_msg[10];
	if(test_sock())
	{
		if(block)
// block
			rc = send(global_socket,buf,buflen,MSG_WAITALL);
		else
// don't block
			rc = send(global_socket,buf,buflen,MSG_DONTWAIT);
		if(rc < 0 && errno != 11)
		{
			strcpy(errmsg,strerror(errno));
			sprintf(extra_msg," %d",errno);
			strcat(errmsg,extra_msg);
			strcat(errmsg," put_sock");
//			close_tcp();
			//printf("closing tcp socket\n");
		}else strcpy(errmsg,"Success\0");
	}
	else
	{
// this keeps printing out until the client logs on
		strcpy(errmsg,"sock closed");
		rc = -1;
	}
	return rc;
}
/*********************************************************************/
int get_sock(UCHAR *buf, int buflen, int block, char *errmsg)
{
	int rc;
	char extra_msg[10];
	if(block)
		rc = recv(global_socket,buf,buflen,MSG_WAITALL);
	else
		rc = recv(global_socket,buf,buflen,MSG_DONTWAIT);
	//printf("rc: %d\n",rc);
	if(rc < 0 && errno != 11)
	{
		strcpy(errmsg,strerror(errno));
		sprintf(extra_msg," %d",errno);
		strcat(errmsg,extra_msg);
		strcat(errmsg," get_sock");
	}else strcpy(errmsg,"Success\0");
	return rc;
}
/*********************************************************************/

#endif