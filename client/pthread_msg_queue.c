#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>


//define declarations
#define SIZE     50


//structs
typedef struct msgbuf {
         long    mtype;
         int    data[SIZE];
         } data_buf;


//function declarations
void *send_data();
void *receive_data();

main()
{
     pthread_t thread1;
     pthread_t thread2;
     int ret_val_t1;
     int ret_val_t2;

     //create thread1
     ret_val_t1 = pthread_create( &thread1, NULL, send_data, NULL);
     if(ret_val_t1)
     {
         fprintf(stderr,"Error - pthread_create() return value: %d\n",ret_val_t1);
         exit(EXIT_FAILURE);
     }

     //create thread2
     ret_val_t2 = pthread_create( &thread2, NULL, receive_data, NULL);
     if(ret_val_t2)
     {
         fprintf(stderr,"Error - pthread_create() return value: %d\n",ret_val_t2);
         exit(EXIT_FAILURE);
     }

     printf("pthread_create() for thread 1 returns: %d\n",ret_val_t1);
     printf("pthread_create() for thread 2 returns: %d\n",ret_val_t2);

     //wait untill threads are done with their routines before continuing with main thread
     pthread_join( thread1, NULL);
     pthread_join( thread2, NULL); 

     exit(EXIT_SUCCESS);
}

void *send_data(){

    int msqid;
    int msgflg = IPC_CREAT | 0666;
    key_t key;
    data_buf sbuf;
    size_t buf_length;

    //get the message queue id for the key with value 1234
    key = 1234;

    (void) fprintf(stderr, "\nmsgget: Calling msgget(%#lx,\%#o)\n", key, msgflg);

    if ((msqid = msgget(key, msgflg )) < 0) {
        perror("msgget");
        exit(1);
    }
    else{ 
    (void) fprintf(stderr,"msgget: msgget succeeded: msqid = %d\n", msqid);
    }

    //send message type 1
    sbuf.mtype = 1;

    (void) fprintf(stderr,"msgget: msgget succeeded: msqid = %d\n", msqid);

    //fill the array that is to be sent from thread1 to thread2
    int i = 0;
    for(i = 0; i < SIZE; i++){
    sbuf.data[i] = i;
    }   

    (void) fprintf(stderr,"msgget: msgget succeeded: msqid = %d\n", msqid);

    buf_length = SIZE;

    //send data from thread1 to thread2
    if (msgsnd(msqid, &sbuf, buf_length, IPC_NOWAIT) < 0) {
        perror("msgsnd");
        exit(1);
    }

   else 
      printf("Data sent\n");
}

void *receive_data(){
    int msqid;
    key_t key;
    data_buf  rbuf;

    //get the message queue id for the key with value 1234
    key = 1234;

    if ((msqid = msgget(key, 0666)) < 0) {
        perror("msgget");
        exit(1);
    }

    //receive the answer with message type 1
    if (msgrcv(msqid, &rbuf, SIZE, 1, 0) < 0) {
        perror("msgrcv");
        exit(1);
    }


    //print received answer
    int j = 0;
    for(j = 0; j < SIZE; j++){
    printf("%d\n", rbuf.data[j]);
    }

}