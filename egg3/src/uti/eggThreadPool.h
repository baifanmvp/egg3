#ifndef EGG_THREAD_POOL_H_
#define EGG_THREAD_POOL_H_
#include "../EggDef.h"
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdlib.h>
#include <unistd.h>
E_BEGIN_DECLS
typedef struct tpool_work {
	void (*routine) (void *);
	void *arg;
	struct tpool_work *next;
} tpool_work_t;


typedef struct tpool EGGTHREADPOOL;
typedef struct tpool * HEGGTHREADPOOL;

typedef struct tpool {
	int num_threads;
	int max_queue_size;
	int do_not_block_when_full;
	pthread_t *threads;
	int cur_queue_size;
	tpool_work_t *queue_head;
	tpool_work_t *queue_tail;
	int queue_closed;
	int shutdown;
	pthread_mutex_t queue_lock;
	pthread_cond_t queue_not_empty;
	pthread_cond_t queue_not_full;
	pthread_cond_t queue_empty;
} *tpool_t;

typedef struct eggThreadTaskDetails EGGTHREADTASKDETAILS;
typedef struct eggThreadTaskDetails* HEGGTHREADTASKDETAILS;
struct eggThreadTaskDetails
{
    count_t cnt;
    void** details;
};

void eggThreadPool_init(HEGGTHREADPOOL * heggThreadPool,
        int num_threads,
        int max_queue_size,int do_not_block_when_full);
int eggThreadPool_add_work(HEGGTHREADPOOL heggThreadPool,void (*routine)(void *),void *arg);
int eggThreadPool_destroy(HEGGTHREADPOOL heggThreadPool,int finish);


HEGGTHREADTASKDETAILS eggThreadPool_taskdetails_init(size32_t detailCnt, ...);

EBOOL eggThreadPool_taskdetails_destroy(HEGGTHREADTASKDETAILS hThreadTaskDetails);
E_END_DECLS

#endif
