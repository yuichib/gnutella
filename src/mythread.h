/*
 * mythread.h
 *
 *  Created on: 2009/04/28
 *      Author: yuichi
 */




//thread
#define MAX_THREAD_NUM 10
#define TYPE_GNU 1
#define TYPE_HTTP 2
#define TH_EMPTY NULL
#define FD_EMPTY -1
#define TYPE_EMPTY -1

typedef struct
{
	pthread_t tid;
	int fd;
	int type;

} Th;


void *Th_Gnufunc(void *arg);
void *Th_Httpfunc(void *arg);
