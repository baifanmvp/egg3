#include "eggRWSCommon.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static inline time_t timediff_local_utc()
{
    struct tm tm;
    time_t t = 0;
    localtime_r(&t, &tm);
    time_t r;
    if (tm.tm_mday == 31)
    {
        r = tm.tm_sec + tm.tm_min*60 + tm.tm_hour*3600 - 86400;
    }
    else if (tm.tm_mday == 1)
    {
        r = tm.tm_sec + tm.tm_min*60 + tm.tm_hour*3600;
    }
    else
    {
        assert(!"localtime_r");
    }
    return r;
}

struct timeval timeval_delta_nonnegative(struct timeval tv1, struct timeval tv2)
{
    struct timeval ret;
    if (tv1.tv_usec < tv2.tv_usec)
    {
        tv1.tv_sec--;
        ret.tv_usec = 1000000 - tv2.tv_usec + tv1.tv_usec;
    }
    else
    {
        ret.tv_usec = tv1.tv_usec - tv2.tv_usec;
    }
    if ((ret.tv_sec = tv1.tv_sec - tv2.tv_sec) < 0)
    {
        ret.tv_sec = ret.tv_usec = 0;
    }
    return ret;
}

int timeval_isRecentInstance(char *dateString, int age_minutes, int *remainAge_seconds, time_t now)
{

    time_t t;
    if (now == 0)
    {
        time(&now);
    }
    t = now + timediff_local_utc();
    t = t - t % (age_minutes * 60);
    t -= timediff_local_utc();
    
    struct tm tm;
    localtime_r(&t, &tm);
    char buf[20];
    snprintf(buf, sizeof(buf), "%04d%02d%02d%02d%02d",
             tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday, tm.tm_hour, tm.tm_min);
    if (strcmp(dateString, buf))
    {
        return 0;               /* false */
    }
    if (remainAge_seconds)
    {
        *remainAge_seconds = (age_minutes * 60 ) - (now - t);
    }
    return 1;                   /* true */
}

char *timeval_getRecentInstanceString(int age_minutes, time_t now)
{
    time_t t;
    if (now == 0)
    {
        time(&t);
    }
    else
    {
        t = now;
    }
    t += timediff_local_utc();
    t = t - t % (age_minutes * 60);
    t -= timediff_local_utc();
    struct tm tm;
    localtime_r(&t, &tm);
    char *buf = malloc(20);
    snprintf(buf, 20, "%04d%02d%02d%02d%02d",
             tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday, tm.tm_hour, tm.tm_min);

    return buf;
}

time_t timeval_getRecentInstanceId(int age_minutes, time_t now)
{
    time_t t;
    if (now == 0)
    {
        time(&t);
    }
    else
    {
        t = now;
    }
    t += timediff_local_utc();
    t = t - t % (age_minutes * 60);
    t -= timediff_local_utc();    
    return t;
}

struct timeval timeval_someTimeLater(time_t nsec, struct timeval now)
{
    if (now.tv_sec == 0)
    {
        gettimeofday(&now, NULL);
    }
    if (nsec > 0
        && now.tv_sec > TIMEVAL_TIMEMAX - nsec)
    {
        now.tv_sec = TIMEVAL_TIMEMAX;
    }
    else
    {
        now.tv_sec = now.tv_sec + nsec;
    }
    return now;
}


HEGGRWSFIFOQUEUE eggRWSFifoQueue_new()
{
    HEGGRWSFIFOQUEUE hQueue = calloc(1, sizeof(*hQueue));
    assert(hQueue);
    return hQueue;
}
int eggRWSFifoQueue_count(HEGGRWSFIFOQUEUE hQueue)
{
    if (!hQueue)
    {
        return 0;
    }
    return hQueue->count;
}
int eggRWSFifoQueue_push(HEGGRWSFIFOQUEUE hQueue, void *data)
{
    EGGRWSLISTITEM *p_item = calloc(1, sizeof(EGGRWSLISTITEM));
    assert(p_item);
    p_item->data = data;
    fifolist_push(hQueue, p_item);
    return 0;
}
void *eggRWSFifoQueue_pop(HEGGRWSFIFOQUEUE hQueue)
{
    void *data = NULL;
    EGGRWSLISTITEM *p_item = NULL;
    fifolist_pop(hQueue, p_item);
    if (p_item)
    {
        data = p_item->data;
        free(p_item);
    }
    return data;
}
int eggRWSFifoQueue_delete(HEGGRWSFIFOQUEUE hQueue)
{
    if (!hQueue)
    {
        return 0;
    }

    while (hQueue->count > 0)
    {
        void *data = NULL;
        eggRWSFifoQueue_pop(hQueue);
        free(data);
    }
    free(hQueue);
    return 0;
}



EGGRWSJOBSPEC *EGGRWSJOBSPEC_new()
{
    EGGRWSJOBSPEC *p_job;
    p_job = calloc(1, sizeof(*p_job)); assert(p_job);
    pthread_mutex_init(&p_job->mutex, NULL);
    pthread_cond_init(&p_job->cond, NULL);
    return p_job;
}

int EGGRWSJOBSPEC_delete(EGGRWSJOBSPEC *p_job)
{
    pthread_mutex_destroy(&p_job->mutex);
    pthread_cond_destroy(&p_job->cond);
    if (p_job->job_mem_count)
    {
        free(p_job->job_mem_id);
        free(p_job->job_mem_input);
        free(p_job->job_mem_output);
    }
        
    free(p_job);
    return 0;
}

int EGGRWSJOBSPEC_setMemJob(EGGRWSJOBSPEC *p_job,
                            void *job_mem_input, void **job_mem_id,
                            int job_mem_count)
{
    
    pthread_mutex_lock(&p_job->mutex);

    p_job->job_mem_count = job_mem_count;
    p_job->job_mem_id = malloc(sizeof(void *)*job_mem_count);
    assert(p_job->job_mem_id);
    p_job->job_mem_input = malloc(sizeof(void *)*job_mem_count);
    assert(p_job->job_mem_input);
    p_job->job_mem_output = malloc(sizeof(void *)*job_mem_count);
    assert(p_job->job_mem_output);
    int i;
    for (i = 0; i < job_mem_count; i++)
    {
        p_job->job_mem_id[i] = job_mem_id[i];
        p_job->job_mem_input[i] = job_mem_input;
        p_job->job_mem_output[i] = NULL;

        p_job->count_job++;
    }
    
    pthread_mutex_unlock(&p_job->mutex);
    return 0;
}

void *EGGRWSJOBSPEC_getMemJobInput(EGGRWSJOBSPEC *p_job, void *job_mem_id)
{
    pthread_mutex_lock(&p_job->mutex);
    void *retv = NULL;

    int i;
    for (i = 0; i < p_job->job_mem_count; i++)
    {
        if (p_job->job_mem_id[i] == job_mem_id)
        {
            retv = p_job->job_mem_input[i];
            p_job->job_mem_input[i] = NULL;
            break;
        }

    }
    
    pthread_mutex_unlock(&p_job->mutex);
    return retv;
    
}

void *EGGRWSJOBSPEC_getMemJobOutput(EGGRWSJOBSPEC *p_job, void *job_mem_id)
{
    pthread_mutex_lock(&p_job->mutex);
    void *retv = NULL;

    int i;
    for (i = 0; i < p_job->job_mem_count; i++)
    {
        if (p_job->job_mem_id[i] == job_mem_id)
        {
            retv = p_job->job_mem_output[i];
            p_job->job_mem_output[i] = NULL;
            break;
        }

    }
    
    pthread_mutex_unlock(&p_job->mutex);
    return retv;
    
}

int EGGRWSJOBSPEC_yieldMemJobOutput(EGGRWSJOBSPEC *p_job,
                                    void *job_mem_output, void *job_mem_id)
{
    pthread_mutex_lock(&p_job->mutex);
    int retv = -1;
    int i;
    for (i = 0; i < p_job->job_mem_count; i++)
    {
        if (p_job->job_mem_id[i] == job_mem_id)
        {
            p_job->job_mem_input[i] = NULL;
            p_job->job_mem_output[i] = job_mem_output;
            
            p_job->count_job--;
            if (p_job->count_job == 0)
            {
                pthread_cond_signal(&p_job->cond);
            }
            retv = 0;
            break;
        }

    }

    pthread_mutex_unlock(&p_job->mutex);
    return retv;
}

int EGGRWSJOBSPEC_waitJob(EGGRWSJOBSPEC *p_job)
{
    pthread_mutex_lock(&p_job->mutex);
    while (p_job->count_job > 0)
    {
        pthread_cond_wait(&p_job->cond, &p_job->mutex);
    }
    pthread_mutex_unlock(&p_job->mutex);
    return 0;
}


HEGGRWSCIRCULARQUEUE eggRWSCircularQueue_new(int queuesize_guess)
{
    HEGGRWSCIRCULARQUEUE hCircularQueue;
    hCircularQueue = calloc(1, sizeof(*hCircularQueue));
    assert(hCircularQueue);

    hCircularQueue->size = queuesize_guess;
    if (hCircularQueue->size <= 0)
    {
        hCircularQueue->size = 100;
    }
    hCircularQueue->data = malloc(hCircularQueue->size * sizeof(void *));
    assert(hCircularQueue->data);
    pthread_mutex_init(&hCircularQueue->mutex, NULL);
    pthread_cond_init(&hCircularQueue->cond, NULL);
    return hCircularQueue;
}
int eggRWSCircularQueue_push(HEGGRWSCIRCULARQUEUE hCircularQueue, void *data)
{
    if (!hCircularQueue)
    {
        return 0;
    }
    pthread_mutex_lock(&hCircularQueue->mutex);
    ++hCircularQueue->count;
    hCircularQueue->data[hCircularQueue->tail] = data;
    hCircularQueue->tail = (hCircularQueue->tail + 1) % hCircularQueue->size;
    if (hCircularQueue->tail == hCircularQueue->head)
    {
        int newsize = 2 * hCircularQueue->size;
        hCircularQueue->data = realloc(hCircularQueue->data,
                                       newsize * sizeof(void *));
        assert(hCircularQueue->data);
        int i, j;
        for (i = 0, j = hCircularQueue->size; i < hCircularQueue->head; i++)
        {
            hCircularQueue->data[j] = hCircularQueue->data[i];

            j = (j+1) % newsize;
        }
        hCircularQueue->tail = j;
        hCircularQueue->size = newsize;
        
    }
    pthread_cond_signal(&hCircularQueue->cond);
    
    pthread_mutex_unlock(&hCircularQueue->mutex);
    return 0;
}
void *eggRWSCircularQueue_pop(HEGGRWSCIRCULARQUEUE hCircularQueue)
{
    if (!hCircularQueue)
    {
        return 0;
    }
    void *r;
    pthread_mutex_lock(&hCircularQueue->mutex);
    while (hCircularQueue->head == hCircularQueue->tail)
    {
        pthread_cond_wait(&hCircularQueue->cond, &hCircularQueue->mutex);
    }
    --hCircularQueue->count;
    r = hCircularQueue->data[hCircularQueue->head];
    hCircularQueue->head = (hCircularQueue->head + 1) % hCircularQueue->size;
    pthread_mutex_unlock(&hCircularQueue->mutex);
    return r;
    
}

void *eggRWSCircularQueue_pop_nowait(HEGGRWSCIRCULARQUEUE hCircularQueue)
{
    if (!hCircularQueue)
    {
        return NULL;
    }
    void *r;
    pthread_mutex_lock(&hCircularQueue->mutex);
    if (hCircularQueue->head == hCircularQueue->tail)
    {
        r = NULL;
    }
    else
    {
        --hCircularQueue->count;
        r = hCircularQueue->data[hCircularQueue->head];
        hCircularQueue->head = (hCircularQueue->head + 1) % hCircularQueue->size;
    }
    pthread_mutex_unlock(&hCircularQueue->mutex);
    return r;
    
}
void *eggRWSCircularQueue_look(HEGGRWSCIRCULARQUEUE hCircularQueue)
{
    if (!hCircularQueue)
    {
        return NULL;
    }
    void *r;
    pthread_mutex_lock(&hCircularQueue->mutex);
    if (hCircularQueue->head == hCircularQueue->tail)
    {
        r = NULL;
    }
    else
    {
        r = hCircularQueue->data[hCircularQueue->head];
    }
    pthread_mutex_unlock(&hCircularQueue->mutex);
    return r;
    
}

int eggRWSCircularQueue_count(HEGGRWSCIRCULARQUEUE hCircularQueue)
{
    if (!hCircularQueue)
    {
        return 0;
    }

    return hCircularQueue->count;
    
}
int eggRWSCircularQueue_delete(HEGGRWSCIRCULARQUEUE hCircularQueue)
{
    if (!hCircularQueue)
    {
        return 0;
    }
    
    pthread_cond_destroy(&hCircularQueue->cond);
    pthread_mutex_destroy(&hCircularQueue->mutex);
    free(hCircularQueue->data);
    free(hCircularQueue);

    return 0;
}



HEGGRWSSET eggRWSSet_new(int size_guess)
{
    HEGGRWSSET hSet;
    hSet = calloc(1, sizeof(*hSet));
    assert(hSet);

    hSet->size = size_guess;
    if (hSet->size <= 0)
    {
        hSet->size = 100;
    }
    hSet->data = malloc(hSet->size * sizeof(void *));
    assert(hSet->data);
    pthread_mutex_init(&hSet->mutex, NULL);
    return hSet;
    
}

int eggRWSSet_delete(HEGGRWSSET hSet)
{
    free(hSet->data);
    pthread_mutex_destroy(&hSet->mutex);
    free(hSet);
    return 0;
}

int eggRWSSet_add(HEGGRWSSET hSet, void *key)
{
    if (!hSet)
    {
        return 0;
    }

    pthread_mutex_lock(&hSet->mutex);
    
    int count = hSet->count;
    void **data = hSet->data;
    int i;
    for (i = 0; i < count; i++)
    {
        if (key == data[i])
            goto end;
    }

    data[count] = key;
    ++hSet->count;
    if (hSet->count == hSet->size)
    {
        int new_size = hSet->size * 2;
        
        hSet->data = realloc(hSet->data, new_size * sizeof(void *));
        assert(hSet->data);
        hSet->size = new_size;
    }
    
end:
    pthread_mutex_unlock(&hSet->mutex);
    
    return 0;    
}

int eggRWSSet_identFunc(void *to_be_equal, void *key)
{
    return to_be_equal == key;
}

int eggRWSSet_remove(HEGGRWSSET hSet,
                     int (*ifremove_func)(void *ifremove_arg, void *key), void *ifremove_arg,
                     int (*do_func)(void *do_arg, void *key), void *do_arg)
{
    if (!hSet)
    {
        return 0;
    }

    pthread_mutex_lock(&hSet->mutex);
    
    int count = hSet->count;
    void **data = hSet->data;
    int i, j;
    for (i = 0, j = 0; i < count; i++)
    {
        if (ifremove_func(ifremove_arg, data[i]))
        {
            if (do_func)
            {
                do_func(do_arg, data[i]);
            }
        }
        else
        {
            data[j] = data[i];
            j++;
        }
    }
    hSet->count = j;
    pthread_mutex_unlock(&hSet->mutex);
    return 0;
}

int eggRWSSet_doAll(HEGGRWSSET hSet, int (*func)(void *arg, void *key), void *arg)
{
    if (!hSet)
    {
        return 0;
    }

    pthread_mutex_lock(&hSet->mutex);
    
    int count = hSet->count;
    void **data = hSet->data;
    int i;
    for (i = 0; i < count; i++)
    {
        func(arg, data[i]);
    }

    pthread_mutex_unlock(&hSet->mutex);
    
    return 0;
}
        
int eggRWSSet_count(HEGGRWSSET hSet)
{
    if (!hSet)
    {
        return -1;
    }

    pthread_mutex_lock(&hSet->mutex);
    int count = hSet->count;
    pthread_mutex_unlock(&hSet->mutex);
    
    return count;

}
