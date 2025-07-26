/********************************************************
 * An example source module to accompany...
 *
 * "Using POSIX Threads: Programming with Pthreads"
 *     by Brad nichols, Dick Buttlar, Jackie Farrell
 *     O'Reilly & Associates, Inc.
 *
 ********************************************************
 * llist_threads_rw.h --
 *
 * Include file for linked list with reader/writer locks
 * for threads support
 */
#include <pthread.h>
#include "../mytypes.h"
//#include "../tasks.h"
#include "rdwr.h"

#ifndef TRUE
#define TRUE    1
#endif

#ifndef FALSE
#define FALSE   0
#endif

typedef struct o_data
{
	char label[OLABELSIZE];
	int port;
	int onoff;			// current state: 1 if on; 0 if off
	int input_port;		// input port which affects this output (if not set to 0xFF)
	int input_type;		// 
							// 
	int type;				// see below
	int time_delay;		// when type 2-4 this is used as the time delay
	int time_left;			// gets set to time_delay and then counts down
	int pulse_time;		// not used
	int reset;			// used to make 2nd pass
} O_DATA;

/*
type:
0) regular - on/off state responds to assigned input (affected_output)
1) goes on/off and stays that way until another on/off
2) on for time_delay seconds and then it goes back off
3) goes on/off every second until time_delay is up
4) if on, turn off and wait for msg (serial or tcp) to turn back on
5) goes on/off at pulse_time rate in 10ths of a second then
	goes off when time_delay is up

*/

typedef struct ollist_node {
  int index;
  O_DATA *datap;
  struct llist_node *nextp;
} ollist_node_t;

typedef struct ollist {
  ollist_node_t *first;
  pthread_rdwr_t rwlock;
} ollist_t;

int ollist_init (ollist_t *llistp);
int ollist_insert_data (int index, ollist_t *llistp,O_DATA *datap);
int ollist_remove_data(int index, O_DATA **datapp, ollist_t *llistp);
int ollist_removeall_data(ollist_t *llistp);
int ollist_find_data(int index, O_DATA **datapp, ollist_t *llistp);
int ollist_find_data_ip(int index, O_DATA **datapp, ollist_t *llistp);
int ollist_find_data_op(int index, int port, O_DATA **datapp, ollist_t *llistp);
int ollist_toggle_output(int index, ollist_t *llistp);
int ollist_change_output(int index, ollist_t *llistp, int onoff);
int ollist_change_data(int index, O_DATA *datap, ollist_t *llistp);
int ollist_show(ollist_t *llistp);
int ollist_printfile(int fp, ollist_t *llistp);

