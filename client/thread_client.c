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
#include <sys/ipc.h>
#include <sys/msg.h>

#include "mytypes.h"
#include "tasks.h"

#define MAX 80
#define PORT 5193
#define SA struct sockaddr

int main_qid;
key_t main_key;
struct msgqbuf msg;
int msgtype = 1;
static int close_program;

void *read_thread(void *);

static UCHAR pre_preamble[] = {0xF8,0xF0,0xF0,0xF0,0xF0,0xF0,0xF0,0x00};

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

void func(void)
{
    char buff[MAX];
    int n;
	int cmd = 44;
	msg.mtype = msgtype;
	int ret;
	
    for (;;) 
	{
        bzero(buff, sizeof(buff));
        //printf("Enter the string : ");
        n = 0;
        while ((buff[n++] = getchar()) != '\n');
		
		n--;
		printf("msg_len: %d\n",n);
		send_msg(n, buff,cmd, 0);
/*
		memset(msg.mtext,0,sizeof(msg.mtext));
		memcpy(msg.mtext,buff,n);
		ret = 0;
		ret = msgsnd(main_qid, (void *) &msg, sizeof(msg.mtext), MSG_NOERROR);
		if(ret == -1)
		{
			perror("msgsnd error");
		}
*/
		printf("ret: %d\n",ret);
		if(n == 0)
			return;
    }
}

void *read_queue_thread(void *socket_desc)
{
	msg.mtype = msgtype;
	memset(msg.mtext,0,sizeof(msg.mtext));
	printf("queue thread started\n");
	ssize_t ret = 1;
	
	for(;;)
	{
		ret = msgrcv(main_qid, (void *) &msg, sizeof(msg.mtext), msgtype, MSG_NOERROR);
		printf("ret: %ld\n",ret);
		if(ret == -1)
		{
			if(errno != ENOMSG)
			{
				perror("msgrcv");
				printf("msgrcv error\n");
				exit(EXIT_FAILURE);
			}
		}
		printf("read queue thread: %s\n",msg.mtext);
	}
}

void *read_thread(void *socket_desc)
{
	char tempx[200];
	int msg_len;
	int ret;
	UCHAR cmd;
	int i;

	//Get the socket descriptor
	int read_size;
	
	//Receive a message from client
//	printf("start %d\n", global_socket);
	msg_len = 1;
	while(msg_len > 0)
	{
		printf("sock: %d\n",global_socket);
		msg_len = get_msg();
		if(msg_len > 0)
		{
	//		printf("msg_len: %d\n",msg_len);
			ret = recv_tcp(&tempx[0],msg_len+1,1);
	//		printf("\n\nret: %d msg_len: %d\n",ret,msg_len);
	//		currently the ret is just 1 more than msg_len 
			cmd = tempx[0];

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

			printf("cmd: %d\n",cmd);
	//		print_cmd(cmd);

			memmove(tempx,tempx+1,msg_len);
			//printf("\n");

			for(i = 0;i < msg_len;i++)
				printf("%02x ",tempx[i]);

			printf("\n");
		}
	}

	if(msg_len == 0)
	{
		puts("Client disconnected");
		fflush(stdout);
		printf("done\n");
	}
	else if(msg_len == -1)
	{
		perror("recv failed");
	}
		
	close_program = 1;
	return 0;
}


int main(int argc, char *argv[])
{
    int sockfd, connfd;
    struct sockaddr_in servaddr, cli;
	char buff[200];
	int n = 0;
	close_program = 0;
	char client_name[30];

	main_qid = msgget(main_key, IPC_CREAT | 0666);

	if(argc != 2)
	{
		printf("must enter the name of the client so it can be sent to the server\n");
		exit(1);
	}
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
    servaddr.sin_addr.s_addr = inet_addr("192.168.88.147");
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
	pthread_t sniffer_thread;
	pthread_t queue_thread;
	
	if( pthread_create( &sniffer_thread , NULL ,  read_thread , (void*) global_socket) < 0)
	{
		perror("could not create thread");
		return 1;
	}
/*
	if( pthread_create( &queue_thread , NULL ,  read_queue_thread , (void*) global_socket) < 0)
	{
		perror("could not create thread");
		return 1;
	}
*/;
	// send the client name to the server 
	send_msg(30, client_name, 0, 0);

    func();

    close(global_socket);
	pthread_kill(sniffer_thread, 0);
//	pthread_kill(read_queue_thread, 0);
	printf("closing program\n");
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
	printf("ret: %d\n",ret);
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