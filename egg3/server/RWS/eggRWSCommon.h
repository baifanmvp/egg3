#ifndef EGGRWSCOMMON_H_
#define EGGRWSCOMMON_H_
#include <sys/time.h>
#include <pthread.h>
#include <limits.h>


enum {TIMEVAL_TIMEMAX = INT_MAX};

struct timeval timeval_delta_nonnegative(struct timeval tv1, struct timeval tv2);

int timeval_isRecentInstance(char *dateString, int age_minutes, int *remainAge_seconds, time_t now);
char *timeval_getRecentInstanceString(int age_minutes, time_t now);
time_t timeval_getRecentInstanceId(int age_minutes, time_t now);

struct timeval timeval_someTimeLater(time_t nsec, struct timeval now);

typedef struct eggRWSListItem EGGRWSLISTITEM;
typedef struct eggRWSListItem *HEGGRWSLISTITEM;
struct eggRWSListItem {
    void *data;
    struct eggRWSListItem *next;
};
typedef struct eggRWSListHead EGGRWSLISTHEAD;
typedef struct eggRWSListHead *HEGGRWSLISTHEAD;
struct eggRWSListHead {
    int count;
    struct eggRWSListItem *head;
    struct eggRWSListItem *tail;
};
#define fifolist_push(listhead, p) {                                    \
        (listhead)->count++;                                            \
        if ((listhead)->count == 1) (listhead)->head = (listhead)->tail = (p); \
        else (listhead)->tail->next = (p), (listhead)->tail = (p); }
#define fifolist_pop(listhead, p) {                                     \
        if (!(listhead)->count) (p) = NULL;                             \
        else {                                                          \
            (listhead)->count--; (p) = (listhead)->head;                \
            if (!(listhead)->count) (listhead)->head = (listhead)->tail = NULL; \
            else (listhead)->head = (listhead)->head->next; } }


typedef HEGGRWSLISTHEAD HEGGRWSFIFOQUEUE;
HEGGRWSFIFOQUEUE eggRWSFifoQueue_new();
int eggRWSFifoQueue_count(HEGGRWSFIFOQUEUE hQueue);
int eggRWSFifoQueue_push(HEGGRWSFIFOQUEUE hQueue, void *data);
void *eggRWSFifoQueue_pop(HEGGRWSFIFOQUEUE hQueue);
int eggRWSFifoQueue_delete(HEGGRWSFIFOQUEUE hQueue);


typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t cond;

    int count_job;

    int job_mem_count;
    void **job_mem_id;
    void **job_mem_input;
    void **job_mem_output;
    
} EGGRWSJOBSPEC;

EGGRWSJOBSPEC *EGGRWSJOBSPEC_new();
int EGGRWSJOBSPEC_delete(EGGRWSJOBSPEC *p_job);
int EGGRWSJOBSPEC_setMemJob(EGGRWSJOBSPEC *p_job,
                            void *job_mem_input, void **job_mem_id,
                            int job_mem_count);
void *EGGRWSJOBSPEC_getMemJobInput(EGGRWSJOBSPEC *p_job, void *job_mem_id);
void *EGGRWSJOBSPEC_getMemJobOutput(EGGRWSJOBSPEC *p_job, void *job_mem_id);
int EGGRWSJOBSPEC_yieldMemJobOutput(EGGRWSJOBSPEC *p_job,
                                    void *job_mem_output, void *job_mem_id);
int EGGRWSJOBSPEC_waitJob(EGGRWSJOBSPEC *p_job);

typedef struct eggRWSCircularQueue EGGRWSCIRCULARQUEUE;
typedef struct eggRWSCircularQueue *HEGGRWSCIRCULARQUEUE;
struct eggRWSCircularQueue
{
    int count;
    int head;
    int tail;
    int size;
    void **data;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
};
HEGGRWSCIRCULARQUEUE eggRWSCircularQueue_new(int queuesize_guess);
int eggRWSCircularQueue_count(HEGGRWSCIRCULARQUEUE hCircularQueue);
int eggRWSCircularQueue_push(HEGGRWSCIRCULARQUEUE hCircularQueue, void *data);
void *eggRWSCircularQueue_pop(HEGGRWSCIRCULARQUEUE hCircularQueue);
void *eggRWSCircularQueue_pop_nowait(HEGGRWSCIRCULARQUEUE hCircularQueue);
void *eggRWSCircularQueue_look(HEGGRWSCIRCULARQUEUE hCircularQueue);
int eggRWSCircularQueue_delete(HEGGRWSCIRCULARQUEUE hCircularQueue);


typedef struct eggRWSSet EGGRWSSET;
typedef struct eggRWSSet *HEGGRWSSET;
struct eggRWSSet
{
    int count;
    int size;
    void **data;
    pthread_mutex_t mutex;
};
HEGGRWSSET eggRWSSet_new(int size_guess);
int eggRWSSet_add(HEGGRWSSET hSet, void *key);
int eggRWSSet_count(HEGGRWSSET hSet);
int eggRWSSet_doAll(HEGGRWSSET hSet, int (*func)(void *arg, void *key), void *arg);
int eggRWSSet_identFunc(void *to_be_qual, void *key);
int eggRWSSet_remove(HEGGRWSSET hSet,
                     int (*ifremove_func)(void *ifremove_arg, void *key), void *ifremove_arg,
                     int (*do_func)(void *do_arg, void *key), void *do_arg);
int eggRWSSet_delete(HEGGRWSSET hSet);


#endif
