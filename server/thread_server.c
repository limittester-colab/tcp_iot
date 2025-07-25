#include<stdio.h>
#include<string.h>	//strlen
#include<stdlib.h>	//strlen
#include<sys/socket.h>
#include<arpa/inet.h>	//inet_addr
#include<unistd.h>	//write
#include<pthread.h> //for threading , link with lpthread
#include <errno.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "mytypes.h"
#include "tasks.h"
#define MAX 80

static UCHAR pre_preamble[] = {0xF8,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0x00};

void *listen_thread(void *);
void *send_thread(void *);
void *read_queue_thread(void *);
void *new_sock_thread(void *);

key_t main_key;
struct msgqbuf msg;
int msgtype = 1;
int no_threads;

THREADS pthreads_list[5];

void add_client_queue(char client_name)
{
	printf("client name: %s\n",client_name);
}

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
				printf("uSleep error\0");
//             return -1;
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

int main(int argc , char *argv[])
{
	int socket_desc ,  c;
	struct sockaddr_in server;
	char *message;
	int msg_len;
	char tempx[30];
	int ret;
	int i;
	pthread_t sock_thread;
	//Create socket
	no_threads = 0;
	main_key = MAIN_QKEY;
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
		return 1;
	}
	puts("bind done");

	//Listen
	listen(socket_desc , 3);

	if( pthread_create( &sock_thread , NULL ,  new_sock_thread , (void*) socket_desc) < 0)
	{
		perror("could not create thread");
		return 1;
	}

/*
	if (new_socket<0)
	{
		perror("accept failed");
		return 1;
	}
*/
//	printf("total no_threads 1: %d\n",no_threads);
	while(no_threads < 1);
//	printf("total no_threads 2: %d\n",no_threads);

	for(i = 0;i < no_threads;i++)
	{
		if(pthread_join(pthreads_list[i].listen_thread, NULL) != 0)
		{
			perror("join failed on listen_thread join");
			exit(1);
		}
		pthread_kill(pthreads_list[i].read_queue_thread,0);
	}

//	printf("test1\n");
/*
	for(i = 0;i < no_threads;i++)
	{
		printf("wait for listen_thread to quit\n");
		if(pthread_join(pthreads_list[i].listen_thread, NULL) != 0)
		{
			perror("join failed on listen_thread join");
			exit(1);
		}
		printf("wait for read_queue_thread to quit\n");
		if(pthread_join(pthreads_list[i].read_queue_thread, NULL) != 0)
		{
			perror("join failed on read_queue_thread join");
			exit(1);
		}
		printf("quit thread %d\n",pthreads_list[i].sock);
	}
*/
	pthread_kill(sock_thread, 0);
	printf("done\n");

	return 0;
}

void *new_sock_thread(void *socket_desc)
{
	int c;
	int *new_sock;
	int new_socket;
	struct sockaddr_in client;

	puts("Waiting for incoming connections...");
	c = sizeof(struct sockaddr_in);
	while( (new_socket = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) )
	{
//		puts("Connection accepted");
		//Reply to the client
		//message = "Hello Client , I have received your connection. And now I will assign a handler for you\n";
		//write(new_socket , message , strlen(message));
		printf("no_threads: %d\n",no_threads);
		new_sock = malloc(1);
		*new_sock = new_socket;

//		msg_len = get_msg(new_sock);
//		printf("msg_len: %d\n",msg_len);
//		ret = recv_tcp(new_sock, &tempx[0],msg_len+2,1);
//		printf("%s logged in\n",tempx);

		if( pthread_create( &(pthreads_list[no_threads].listen_thread) , NULL ,  listen_thread , (void*) new_sock) < 0)
		{
			perror("could not create sniffer_thread");
			return 1;
		}

		if( pthread_create( &(pthreads_list[no_threads].read_queue_thread) , NULL ,  read_queue_thread , (void*) new_sock) < 0)
		{
			perror("could not create thread");
			return 1;
		}
		pthreads_list[no_threads].sock = new_socket;
		pthreads_list[no_threads].main_key = main_key + no_threads;
		pthreads_list[no_threads].qid = msgget(pthreads_list[no_threads].main_key, IPC_CREAT | 0666);
		printf("%d %d %d\n",pthreads_list[no_threads].qid, pthreads_list[no_threads].main_key, pthreads_list[no_threads].sock);
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
	char tempx[200];
	int msg_len;
	int ret;
	UCHAR cmd;
	UCHAR dest;
	int i;
	char client_name[30];
	CLIENT_NAME *cl;
	msg.mtype = msgtype;
	int skip = 0;
	int index = -1;
	int pindex;

	//Get the socket descriptor
	int sock = *(int*)socket_desc;
	int read_size;

	//Receive a message from client
//	printf("start %d\n", sock);
	msg_len = 1;
	memset(client_name, 0, sizeof(client_name));

	while(msg_len > 0)
	{
		if(index < 0)
		{
			while(sock != pthreads_list[index].sock && index++ > no_threads)
			printf("index: %d sock: %d\n",index,sock);
			pindex = index;
		}

//		printf("sock: %d\n",sock);
		msg_len = get_msg(sock);
//		printf("msg_len: %d\n",msg_len);
		ret = recv_tcp(sock, &tempx[0],msg_len+2,1);
//		printf("sock: %d\n",sock);
//		printf("\n\nret: %d msg_len: %d\n",ret,msg_len);
		cmd = tempx[0];
		dest = tempx[1];

//		printf("dest: %d\n",dest);
/*
		for(i = 0;i < msg_len;i++)
			printf("%02x ",tempx[i]);
		printf("\n");

		for(i = 2;i < msg_len+2;i++)
			printf("%02x ",tempx[i]);
		printf("\n");
*/
		for(i = 2;i < msg_len+2;i++)
			printf("%c",tempx[i]);
		printf("\n");

//		printf("cmd: %d\n",cmd);
//		print_cmd(cmd);

		memmove(tempx,tempx+2,msg_len);

		if(client_name[0] == 0)
		{
			strcpy(client_name, tempx);
			printf("this client is called: %s\n",client_name);
//			cl = (CLIENT_NAME *)malloc(sizeof(CLIENT_NAME));
			//add_client_queue(client_name);
		}else
		{
			printf("client name: %s\n",client_name);
		}
/*
		for(i = 0;i < msg_len;i++)
			printf("%02x ",tempx[i]);
*/
		printf("\n");

//		if(skip)
		if(msg_len > 0)
		{
			printf("send msg\n");

			memset(msg.mtext,0,sizeof(msg.mtext));
			memcpy(msg.mtext,tempx,msg_len);
			ret = 0;
			ret = msgsnd(pthreads_list[index].qid, (void *) &msg, sizeof(msg.mtext), MSG_NOERROR);
			if(ret == -1)
			{
				perror("msgsnd error");
			}
			printf("msgsnd ret: %d\n",ret);
			skip = 1;
		}
		index = 0;
	}

	if(msg_len == 0)
	{
		puts("Client disconnected");
		pthreads_list[pindex].sock = -1;
		pthreads_list[pindex].qid = -1;
//		fflush(stdout);
	}

	else if(msg_len == -1)
	{
		perror("recv failed");
	}

	//Free the socket pointer

	pthread_kill(pthreads_list[pindex].read_queue_thread,0);
	free(socket_desc);
	pthread_exit(NULL);

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
	printf("ret: %d\n",ret);
	if(ret < 0)
	{
//		printHexByte(ret);
		printf("%02x ",ret);
	}
	if(memcmp(preamble,pre_preamble,8) != 0)
	{
		printf("bad preamble\n");
		for(i = 0;i < 10;i++)
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
	printf("%02x %02x\n",temp[0],temp[1]);
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
		strcat(errmsg,"\nput_sock\n");
//		close_tcp();
	}else strcpy(errmsg,"Success\0");
	return rc;
}

/*********************************************************************/
int get_sock(int sd, UCHAR *buf, int buflen, int block, char *errmsg)
{
	int rc;
	char extra_msg[10];
	if(block)
		rc = recv(sd,buf,buflen,MSG_WAITALL);
	else
		rc = recv(sd,buf,buflen,MSG_DONTWAIT);
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