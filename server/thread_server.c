#if 1
#include<stdio.h>
#include<string.h>	//strlen
#include<stdlib.h>	//strlen
#include<sys/socket.h>
#include<arpa/inet.h>	//inet_addr
#include<unistd.h>	//write
#include<pthread.h> //for threading , link with lpthread
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>
//#include <pigpio.h>

#include "cmd_types.h"
#include "mytypes.h"
#include "tasks.h"
#define MAX 80

static UCHAR pre_preamble[] = {0xF8,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0x00};

void *listen_thread(void *);
void *send_thread(void *);
void *read_queue_thread(void *);
void *new_sock_thread(void *);
void *timer_thread(void *);
void *check_time_thread(void *);
//void *tester_thread(void *);
pthread_t cmd_task_thread;
pthread_t pbasic_controls_task_thread;

int no_threads;

THREADS pthreads_list[MAX_THREADS];

/* void add_client_queue(char client_name)
{
	printf("client name: %s\n",client_name);
} */
#endif
int main(int argc , char *argv[])
{
//	int socket_desc ,  c;
//	struct sockaddr_in server;
	char *message;
	int msg_len;
	char tempx[30];
	int ret;
	void *pret;
	int i;
	int test;
	pthread_t sock_thread;
	pthread_t time_thread;
	pthread_t test_thread;
	pthread_t c_time_thread;
	//Create socket
	no_threads = 0;
	basic_controls_key = BASIC_CONTROLS_KEY;
//	main_key = MAIN_KEY;
//	main_qid = msgget(main_key, IPC_CREAT | 0666);
	basic_controls_qid = msgget(basic_controls_key, IPC_CREAT | 0666);
/*
	if(gpioInitialise() < 0)
	{
		printf("init failed\n");
		exit(1);
	}
	rc = serOpen("/dev/ttyAMA0",115200,0);
	if(rc < 0)
	{
		printf("could not open port\n");
		exit(1);
	}
	handle = (unsigned int)rc;
	xbyte = 0x21;
*/
	for(i = 0;i < MAX_THREADS;i++)
	{
		pthreads_list[i].sock = -1;
		pthreads_list[i].time_stamp = 0;
//		memset(pthreads_list[i].client_name,0,30);
	}

	if( pthread_create( &sock_thread , NULL ,  new_sock_thread , (void*) pret) < 0)
	{
		perror("could not create thread");
//		return 1;
	}
/*
	if( pthread_create( &test_thread , NULL ,  tester_thread , (void*) socket_desc) < 0)
	{
		perror("could not create thread");
		return 1;
	}
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
*/
 	if( pthread_create( &time_thread , NULL ,  timer_thread , (void*) pret) < 0)
	{
		perror("could not create thread");
//		return 1;
	}
 	if( pthread_create( &c_time_thread , NULL ,  check_time_thread , (void*) pret) < 0)
	{
		perror("could not create thread");
//		return 1;
	}

/*
	if (new_socket<0)
	{
		perror("accept failed");
		return 1;
	}
*/
//	printf("total no_threads 1: %d\n",no_threads);
	while(no_threads < 1)
	{
		uSleep(1,0);
		printf("sleep ");
	}
//	printf("total no_threads 2: %d\n",no_threads);

	for(i = 0;i < no_threads;i++)
	{
		if(pthread_join(pthreads_list[i].listen_thread, NULL) != 0)
		{
			perror("join failed on listen_thread join");
			exit(1);
		}
//		pthread_kill(pthreads_list[i].read_queue_thread,0);
	}
	pthread_kill(sock_thread, 0);
//	pthread_kill(test_thread, 0);
	pthread_kill(time_thread, 0);
	pthread_kill(c_time_thread, 0);
	printf("done\n");
	return 0;
}

void *new_sock_thread(void *ret)
{
	int socket_desc , new_socket , c , *new_sock;
	struct sockaddr_in server , client;

	int msg_len;
	UCHAR tempx[100];
	int i,j;
	char address_string[30];
	int win_cl = 0;

	memset(tempx, 0, sizeof(tempx));

	socket_desc = socket(AF_INET , SOCK_STREAM , 0);
	if (socket_desc == -1)
	{
		printf("Could not create socket");
	}
	
	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons( 5193 );
	
	//Bind
	if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
	{
		puts("bind failed");
		exit(1);
	}
	puts("bind done");
	
	//Listen
	listen(socket_desc , 3);
	
	//Accept and incoming connection
	puts("Waiting for incoming connections...");
	c = sizeof(struct sockaddr_in);
	while( (new_socket = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) )
	{
		strncpy(address_string,inet_ntoa(client.sin_addr),sizeof(address_string));
//		printf("%s\n",address_string);
		j = i = 0;
		while(i < 3 && j < 17 && address_string[j] != 0)
		{
			if(address_string[j] == '.')
				i++;
//			printf("%c",address_string[j]);
			j++;
		}
		memset(tempx,0,sizeof(tempx));
		strncpy(tempx,&address_string[j],3);
//		printf("%s\n",tempx);
		if(strncmp(tempx,"158",3) == 0)
		{
			win_cl = 1;
//			printf("found win_cl\n");
		}else win_cl = 0;

//		printf("no_threads: %d\n",no_threads);
		new_sock = malloc(1);
		*new_sock = new_socket;
//		printf("start %d\n",new_socket);

		if( pthread_create( &(pthreads_list[no_threads].listen_thread) , NULL ,  listen_thread , (void*) new_sock) < 0)
		{
			perror("could not create sniffer_thread");
			exit(1);
		}
/*
		if( pthread_create( &(pthreads_list[no_threads].read_queue_thread) , NULL ,  read_queue_thread , (void*) new_sock) < 0)
		{
			perror("could not create thread");
			exit(1);
		}
*/
		pthreads_list[no_threads].sock = new_socket;
		pthreads_list[no_threads].main_key = main_key + no_threads;
		pthreads_list[no_threads].qid = msgget(pthreads_list[no_threads].main_key, IPC_CREAT | 0666);
		pthreads_list[no_threads].win_cl = win_cl;
		strncpy(pthreads_list[no_threads].ipadd,tempx,3);
		if(strncmp(tempx,"158",3) == 0)
		{
			pthreads_list[no_threads].index = 0;
		}
		if(strncmp(tempx,"154",3) == 0)
		{
			pthreads_list[no_threads].index = 1;
		}
		if(strncmp(tempx,"147",3) == 0)
		{
			pthreads_list[no_threads].index = 2;
		}
		if(strncmp(tempx,"146",3) == 0)
		{
			pthreads_list[no_threads].index = 3;
		}
		if(strncmp(tempx,"151",3) == 0)
		{
			pthreads_list[no_threads].index = 4;
		}
//		if(strncmp(tempx,"241",3) == 0)
//			pthreads_list[no_threads].index = 5;

		printf("start: %d %s %d\n",pthreads_list[no_threads].index, pthreads_list[no_threads].ipadd, pthreads_list[no_threads].sock);
		printf("no_threads: %d\n",no_threads);
//		printf("sock: %d\n",new_socket);
		no_threads++;
/*
		if( pthread_create( &sender_thread , NULL ,  send_thread , (void*) new_sock) < 0)
		{
			perror("could not create sender_thread");
			return 1;
		}
*/
		//Now join the thread , so that we dont terminate before the thread
//		puts("Handler assigned");
//		pthread_join( sniffer_thread , NULL);
	}
	
}

void *listen_thread(void *socket_desc)
{
	int msgtype = 1;
	struct msgqbuf msg;
	UCHAR tempx[100];
	UCHAR tempx2[100];
	UCHAR tempx3[10];
	int msg_len;
	int ret;
	UCHAR cmd;
	UCHAR dest;
	int i,j,k;
	char client_name[30];
	CLIENT_NAME *cl;
	msg.mtype = msgtype;
	int index = -1;
	int win_cl_index = -1;
	struct tm t;
	struct tm tm;
	time_t now;

	//Get the socket descriptor
	int sock = *(int*)socket_desc;
	int read_size;

	//Receive a message from client
//	printf("start %d\n", sock);
	msg_len = 1;
	memset(client_name, 0, sizeof(client_name));

	for(;;)
	{
		index = 0;
//		printf("sock: %d\n",sock);
		for(i = 0;i < MAX_THREADS;i++)
		{
			if(sock == pthreads_list[i].sock)
				index = i;
		}
/*
		for(i = 0;i < MAX_THREADS;i++)
		{
			printf("index: %d sock: %d %d %s\n",index,pthreads_list[i].sock,pthreads_list[i].win_cl,pthreads_list[i].ipadd);
		}
*/
		memset(tempx,0,sizeof(tempx));
//		printf("start get_msg\n");
		
		if(pthreads_list[index].win_cl == 1)
		{
//			printf("win cl\n");
			msg_len = get_msgb(sock);
			
//			printf("msg_len: %d\n",msg_len);
			ret = recv_tcp(sock, &tempx[0],msg_len,1);
//			printf("ret: %d\n",ret);
			if(win_cl_index < 0)
			{
				win_cl_index = index;
				printf("win_cl_index: %d\n",win_cl_index);
			}
		}
		else
		{
			msg_len = get_msg(sock);
//			printf("msg_len: %d\n",msg_len);
			if(msg_len >= 0)
				ret = recv_tcp(sock, &tempx[0],msg_len+2,1);
			else uSleep(5,0);
//			printf("ret: %d\n",ret);
		}

//		for(i = 0;i < msg_len;i++)
//			printf("%02x ",tempx[i]);
//		printf("\n");

		cmd = tempx[0];
//		print_cmd(cmd);

/*
		for(i = 0;i < msg_len;i++)
			printf("%02x ",tempx[i]);
		printf("\n");

		for(i = 2;i < msg_len+2;i++)
			printf("%02x ",tempx[i]);
		printf("\n");
*/

		if(pthreads_list[index].win_cl == 1)
		{
			dest = tempx[2];
			memset(tempx2, 0, sizeof(tempx2));
			k = 0;
			for(j = 4;j < msg_len+4;j+=2)
				tempx2[k++] = tempx[j];
			msg_len /= 2;
			msg_len -= 3;
			memmove(tempx,tempx2,msg_len);
//			printf("win_cl tempx: %s\n",tempx);
		}else
		{
			dest = tempx[1];
			memmove(tempx,tempx+2,msg_len);
			tempx[msg_len] = 0;
		}
//		printf("cmd: %d dest: %d\n",cmd,dest);

		if(cmd == SET_CLIENT_NAME)
		{
			strcpy(client_name, tempx);
			printf("this client is called: %s\n\n",client_name);
//			cl = (CLIENT_NAME *)malloc(sizeof(CLIENT_NAME));
			//add_client_queue(client_name);
			strcpy(pthreads_list[index].client_name,client_name);
		}else if(cmd == SEND_CLIENT_LIST && win_cl_index > 0)// SEND_CLIENT_LIST
		{
			for(i = 0;i < MAX_THREADS;i++)
			{
//				printf("%d %s\n",pthreads_list[i].sock, pthreads_list[i].ipadd);
				if(pthreads_list[i].sock > 0 && pthreads_list[i].win_cl == 0)
				{
					sprintf(tempx,"%d %s %d", pthreads_list[i].index, pthreads_list[i].ipadd, pthreads_list[i].sock);
					printf("%s\n",tempx);
//					printf("%d\n",pthreads_list[win_cl_index].sock);
					msg_len = strlen(tempx);
					send_msgb(pthreads_list[win_cl_index].sock, msg_len*2, tempx, cmd);
					uSleep(0,TIME_DELAY/2);
				}
			}
		}else if(cmd == UPDATE_STATUS)
		{
//			printf("staus: %s\n",tempx);
/*
			gettimeofday(&mtv, NULL);
			curtime2 = mtv.tv_sec;
*/
			now = time(NULL);
			tm = *localtime(&now);
			memset(tempx2,0,sizeof(tempx));
			sprintf(tempx2,"%02d:%02d:%02d", tm.tm_hour, tm.tm_min, tm.tm_sec);
			printf("%s\n",tempx2);

			for(i = 0;i < MAX_THREADS;i++)
			{
				if(strcmp(pthreads_list[i].client_name,tempx) == 0)
				{
//					printf("%s\n", pthreads_list[i].time_stamp);
					pthreads_list[i].time_stamp = now;
				}
			}
		}else
		{
			// EVERYTHING ELSE
			i = 0;
			while(dest != pthreads_list[i].index && i < MAX_THREADS)
				i++;

			printf("sock: %d index %d\n",pthreads_list[i].sock,pthreads_list[i].index);
//			printf("%s\n",tempx);

			if(i < no_threads && pthreads_list[index].sock > 0 && index > 0)
				send_msg(pthreads_list[i].sock, msg_len, tempx, cmd);
//			pthreads_list[i].sock = -1;
//			pthread_kill(pthreads_list[i].listen_thread,0);
		}
		if(msg_len < 0)
		{
			printf("bad msg_len: %d\n",msg_len);
			exit(1);
		}
		index = 0;
	}

	if(msg_len == 0)
	{
		puts("Client disconnected");
		pthreads_list[index].sock = -1;
		pthreads_list[index].qid = -1;
//		fflush(stdout);
	}

	else if(msg_len == -1)
	{
		perror("recv failed");
	}

	//Free the socket pointer

	return 0;
}

void *send_thread(void *socket_desc)
{
	char buff[MAX];
    int n;
	int cmd = 44;
	int sock = *(int*)socket_desc;
	printf("start send_thread\n");
    for (;;) 
	{
        bzero(buff, sizeof(buff));
        printf("Enter the string : ");
        n = 0;
        while ((buff[n++] = getchar()) != '\n');
		n--;
		printf("msg_len: %d\n",n);
		send_msg(sock, n, buff, cmd);
	}
}

void *read_queue_thread(void *socket_desc)
{
	int msgtype = 1;
	struct msgqbuf msg;
	msg.mtype = msgtype;
	memset(msg.mtext,0,sizeof(msg.mtext));
	printf("queue thread started\n");
	ssize_t ret = 1;
	int index = 0;
	int qid;
	int sock = *(int*)socket_desc;

	do
	{
		index = 0;
		while(pthreads_list[index].sock != sock && index < no_threads)
		{
			printf("index: %d sock: %d\n", index, pthreads_list[index].sock);
			index++;
		}
		ret = msgrcv(pthreads_list[index].qid, (void *) &msg, sizeof(msg.mtext), msgtype, MSG_NOERROR);
		if(ret == -1)
		{
			if(errno != ENOMSG)
			{
				perror("msgrcv");
				printf("msgrcv error\n");
				exit(EXIT_FAILURE);
			}
//			printf("ret: %ld %d\n",ret,errno);
		}
		printf("read queue thread: %s\n",msg.mtext);
	}
	while(ret > 0);
	free(socket_desc);
}

void *timer_thread(void *ret)
{
	char buff[40];
	int n;
	int val;
	int cmd = SEND_STATUS;
	int i;
	int j;

	strcpy(buff,"timer \0");

	uSleep(20,0);
	j = 0;
	printf("timer thread started\n");
	for(;;)
	{
		uSleep(30,0);
		for(i = 0;i < no_threads;i++)
		{
			if(pthreads_list[i].sock > 0)
			{
				printf("%s %d\n",pthreads_list[i].ipadd, pthreads_list[i].sock);
				val = i;
//				sprintf(buff2, "%s %d %d",buff, val,j);
//				printf("%s\n",buff2);
				n = strlen(buff);
				if(pthreads_list[i].win_cl == 1)
					send_msgb(pthreads_list[i].sock, n*2, buff, cmd);
				else
					send_msg(pthreads_list[i].sock, n, buff, cmd);
				j++;
				uSleep(15,0);
			}
			uSleep(30,0);
		}

	}
}

void *check_time_thread(void *ret)
{
	struct tm tm;
	time_t now, then;
	char tempx[20];
	double time_diff;
	int i;

	uSleep(60,0);
	i = 0;
	printf("start check time thread\n");
	for(;;)
	{
		uSleep(10,0);
		then = pthreads_list[i].time_stamp;
		printf("checking %s\n",pthreads_list[i].ipadd);
		if(++i >= no_threads)
			i = 0;
		if(then != 0)
		{
			now = time(NULL);
			time_diff = difftime(now,then);
			printf("diff: %.0f\n",time_diff);
			tm = *localtime(&now);
			memset(tempx,0,sizeof(tempx));
			sprintf(tempx,"%02d:%02d:%02d", tm.tm_hour, tm.tm_min, tm.tm_sec);
			printf("%s\n",tempx);
			uSleep(60,0);
		}else memset(tempx,0,sizeof(tempx));
	}
}

void *tester_thread(void *socket_desc)
{
	char buff[30];
	int key;
	int msg_len;
	UCHAR cmd;
	int sock = 4;
	int i,j;
//	int sock = *(int *)socket_desc;
//	printf("sock: %d\n",sock);
//	for(;;)
//		uSleep(1,0);

	do
	{
		key = getc(stdin);
//		printf("key: %c\n",key);
		switch(key)
		{
			case 'a':
				strcpy(buff,"ON\0");
				msg_len = strlen(buff);
				cmd = CHICK_LIGHT;
				sock = 4;
				send_msg(sock,msg_len,buff,cmd);
			break;
			case 'b':
				strcpy(buff,"OFF\0");
				msg_len = strlen(buff);
				cmd = CHICK_LIGHT;
				sock = 4;
				send_msg(sock,msg_len,buff,cmd);
			break;
			case 'c':
				strcpy(buff,"ON\0");
				msg_len = strlen(buff);
				cmd = CHICK_HEATER;
				sock = 4;
				send_msg(sock,msg_len,buff,cmd);
			break;
			case 'd':
				strcpy(buff,"OFF\0");
				msg_len = strlen(buff);
				cmd = CHICK_HEATER;
				sock = 4;
				send_msg(sock,msg_len,buff,cmd);
			break;
			case 'e':
				strcpy(buff,"ON\0");
				msg_len = strlen(buff);
				cmd = BENCH_12V_1;
				sock = 4;
				send_msg(sock,msg_len,buff,cmd);
			break;
			case 'f':
				strcpy(buff,"OFF\0");
				msg_len = strlen(buff);
				cmd = BENCH_12V_1;
				sock = 4;
				send_msg(sock,msg_len,buff,cmd);
			break;

			case 'g':
				strcpy(buff,"ON\0");
				msg_len = strlen(buff);
				cmd = CABIN_SOUTH;
				sock = 5;
				send_msg(sock,msg_len,buff,cmd);
			break;
			case 'h':
				strcpy(buff,"OFF\0");
				msg_len = strlen(buff);
				cmd = CABIN_SOUTH;
				sock = 5;
				send_msg(sock,msg_len,buff,cmd);
			break;
			case 'i':
				strcpy(buff,"ON\0");
				msg_len = strlen(buff);
				cmd = CABIN_KITCHEN;
				sock = 5;
				send_msg(sock,msg_len,buff,cmd);
			break;
			case 'j':
				strcpy(buff,"OFF\0");
				msg_len = strlen(buff);
				cmd = CABIN_KITCHEN;
				sock = 5;
				send_msg(sock,msg_len,buff,cmd);
			break;
			case 'k':
				strcpy(buff,"ON\0");
				msg_len = strlen(buff);
				cmd = CABIN_EAST;
				sock = 5;
				send_msg(sock,msg_len,buff,cmd);
			break;
			case 'l':
				strcpy(buff,"OFF\0");
				msg_len = strlen(buff);
				cmd = CABIN_EAST;
				sock = 5;
				send_msg(sock,msg_len,buff,cmd);
			break;
			case 'm':
				strcpy(buff,"ON\0");
				msg_len = strlen(buff);
				cmd = CABIN_DOOR;
				sock = 5;
				send_msg(sock,msg_len,buff,cmd);
			break;
			case 'n':
				strcpy(buff,"OFF\0");
				msg_len = strlen(buff);
				cmd = CABIN_DOOR;
				sock = 5;
				send_msg(sock,msg_len,buff,cmd);
			break;
			case 'o':
				strcpy(buff,"ON\0");
				msg_len = strlen(buff);
				cmd = BENCH_LIGHT1;
				sock = 4;
				send_msg(sock,msg_len,buff,cmd);
			break;
			case 'p':
				strcpy(buff,"OFF\0");
				msg_len = strlen(buff);
				cmd = BENCH_LIGHT1;
				sock = 4;
				send_msg(sock,msg_len,buff,cmd);
			break;
			case 'q':
				strcpy(buff,"closing client program\0");
				msg_len = strlen(buff);
				cmd = DISCONNECT;
				sock = 4;
				send_msg(sock,msg_len,buff,cmd);
				sock = 5;
				send_msg(sock,msg_len,buff,cmd);
				sock = 6;
				send_msg(sock,msg_len,buff,cmd);
			break;
			case 'r':
				sock = 4;
				cmd = COOP1_LIGHT;
				
				for(i = 0;i < 16;i++)
				{
					strcpy(buff,"ON\0");
					msg_len = strlen(buff);
					send_msg(sock,msg_len,buff,cmd);
					uSleep(1,0);
					strcpy(buff,"OFF\0");
					msg_len = strlen(buff);
					send_msg(sock,msg_len,buff,cmd);
					uSleep(1,0);
					cmd++;
				}
			break;
			case 's':
				sock = 6;
				uSleep(5,0);
				cmd = COOP1_LIGHT;
				for(i = 0;i < 16;i++)	// there's 4 of these that aren't wired
				{
					strcpy(buff,"ON\0");
					msg_len = strlen(buff);
					send_msg(sock,msg_len,buff,cmd);
					uSleep(0,TIME_DELAY/2);
//					uSleep(1,0);
					strcpy(buff,"OFF\0");
					msg_len = strlen(buff);
					send_msg(sock,msg_len,buff,cmd);
					uSleep(0,TIME_DELAY/2);
//					uSleep(1,0);
					cmd++;
				}
			break;

		}
//		printf("%s\n",buff);

	}while(key != 'q' && key != 'Q');
}
#if 1
/*********************************************************************/
/*********************************************************************/
// get preamble & msg len from client
// preamble is: {0xF8,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0x00,
// msg_len(lowbyte),msg_len(highbyte),0x00,0x00,0x00,0x00,0x00,0x00}
// returns message length
int get_msg(int sd)
{
	int len;
	UCHAR low, high;
	int ret;
	int i;

	UCHAR preamble[10];
	ret = recv_tcp(sd, preamble,8,1);
//	printf("ret: %d\n",ret);
	if(ret < 0)
	{
//		printHexByte(ret);
		printf("%d\n",ret);
	}
	if(memcmp(preamble,pre_preamble,8) != 0)
	{
		printf("bad preamble\n");
		for(i = 0;i < 20;i++)
			printf("%02x ",preamble[i]);
		printf("\n");
		uSleep(1,0);
		return -1;
	}
	ret = recv_tcp(sd, &low,1,1);
	ret = recv_tcp(sd, &high,1,1);
	//printf("%02x %02x\n",low,high);
	len = 0;
	len = (int)(high);
	len <<= 4;
	len |= (int)low;

	return len;
}
/*********************************************************************/
void send_msg(int sd, int msg_len, UCHAR *msg, UCHAR msg_type)
{
	int ret;
	int i;
	UCHAR temp[2];

	ret = send_tcp(sd, &pre_preamble[0],8);
	temp[0] = (UCHAR)(msg_len & 0x0F);
	temp[1] = (UCHAR)((msg_len & 0xF0) >> 4);
//	printf("%02x %02x\n",temp[0],temp[1]);
	send_tcp(sd, (UCHAR *)&temp[0],1);
	send_tcp(sd, (UCHAR *)&temp[1],1);
	send_tcp(sd, (UCHAR *)&msg_type,1);

	for(i = 0;i < msg_len;i++)
		send_tcp(sd, (UCHAR *)&msg[i],1);
}
/*********************************************************************/
// get/send_msgb is what the old server used to communicate with the
// windows client - it gets each relavent byte as 2 bytes from the 
// windows machine since that's how the tcp libraries that I'm using
// are done - took me forever to figure this out
int get_msgb(int sd)
{
	int len;
	UCHAR low, high;
	int ret;
	int i;

	UCHAR preamble[20];
	ret = recv_tcp(sd, preamble,16,1);
	if(ret < 0)
	{
//		printHexByte(ret);
		printf("%02x ",ret);
	}
	if(memcmp(preamble,pre_preamble,8) != 0)
		return -1;
	low = preamble[8];
	high = preamble[9];
	len = (int)(high);
	len <<= 8;
	len |= (int)low;
	return len;
}
/*********************************************************************/
void send_msgb(int sd, int msg_len, UCHAR *msg, UCHAR msg_type)
{
	int len;
	int ret;
	int i;

	ret = send_tcp(sd, &pre_preamble[0],8);
	msg_len++;
	send_tcp(sd, (UCHAR *)&msg_len,1);
	ret = 0;
	send_tcp(sd, (UCHAR *)&ret,1);

	for(i = 0;i < 6;i++)
		send_tcp(sd, (UCHAR *)&ret,1);

	send_tcp(sd, (UCHAR *)&msg_type,1);

	ret = 0;
	send_tcp(sd, (UCHAR *)&ret,1);

	for(i = 0;i < msg_len;i++)
	{
		send_tcp(sd, (UCHAR *)&msg[i],1);
		send_tcp(sd, (UCHAR *)&ret,1);
	}
}

/*********************************************************************/
int recv_tcp(int sd, UCHAR *str, int strlen,int block)
{
	int ret = -1;
	char errmsg[20];
	memset(errmsg,0,20);
//		printf("start get_sock\n");
//		pthread_mutex_lock( &tcp_read_lock);
	ret = get_sock(sd, str,strlen,block,&errmsg[0]);
//		pthread_mutex_unlock(&tcp_read_lock);
//		printf("end get_sock\n");
//printf("%s\n",str);
	if(ret < 0 && (strcmp(errmsg,"Success") != 0))
	{
		printf(errmsg);
		uSleep(1,0);
	}
	return ret;
}

/*********************************************************************/
int send_tcp(int sd, UCHAR *str,int len)
{
	int ret = 0;
	char errmsg[60];
	memset(errmsg,0,60);
//	pthread_mutex_lock( &tcp_write_lock);
	ret = put_sock(sd, str,len,1,&errmsg[0]);
//	pthread_mutex_unlock(&tcp_write_lock);
	if(ret < 0 && (strcmp(errmsg,"Success") != 0))
	{
		printf(errmsg);
	}
	return ret;
}

/*********************************************************************/
int put_sock(int sd, UCHAR *buf,int buflen, int block, char *errmsg)
{
	int rc = 0;
	char extra_msg[10];
	if(block)
// block
		rc = send(sd,buf,buflen,MSG_WAITALL);
	else
// don't block
		rc = send(sd,buf,buflen,MSG_DONTWAIT);
	//if(rc < 0 && errno != 11)
	if(rc < 0)
	{
		//printf("sd: %d\n",sd);
		strcpy(errmsg,strerror(errno));
		sprintf(extra_msg," %d\n",errno);
		strcat(errmsg,extra_msg);
		strcat(errmsg,"put_sock\n");
//		close_tcp();
	}else strcpy(errmsg,"Success\0");
	return rc;
}

/*********************************************************************/
int get_sock(int sd, UCHAR *buf, int buflen, int block, char *errmsg)
{
	int rc;
	char extra_msg[10];
	int i;
	if(block)
		rc = recv(sd,buf,buflen,MSG_WAITALL);
	else
		rc = recv(sd,buf,buflen,MSG_DONTWAIT);
//	if(rc < 0 && errno != 11)
/*
	for(i = 0;i < rc;i++)
	{
		printf("%02x ",buf[i]);
	}
	printf("\n");
*/
	if(rc < 0)
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