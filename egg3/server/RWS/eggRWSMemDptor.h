#ifndef EGGRWSMEMDPTOR_H_
#define EGGRWSMEMDPTOR_H_
#include "egg3/Egg3.h"
#include "eggRWSCommon.h"
#include <time.h>
#include <stdint.h>

struct eggRWSMemServerManager;

typedef struct eggRWSMemServerDescriptor EGGRWSMEMSERVERDESCRIPTOR;
typedef struct eggRWSMemServerDescriptor *HEGGRWSMEMSERVERDESCRIPTOR;

extern const time_t TIMETOLIVE_WAITFOREVER;

typedef struct statusInstruct STATUSINSTRUCT;
struct statusInstruct {
    void *belongto;
    int stat;
    time_t timeToLive;
    int (*trigger)(HEGGRWSMEMSERVERDESCRIPTOR);
    struct statusInstruct *next;
};
int statusinstruct_set(STATUSINSTRUCT *psi, int stat, time_t timeToLive);
char *statusinstruct_str(STATUSINSTRUCT *psi);

typedef uint64_t EGGRWSMEMSERVERID;
struct eggRWSMemServerDescriptor
{
    char *name;
    EGGRWSMEMSERVERID id;
    int count_user;
    
    struct eggRWSMemServerManager *hManager;
    char *dir;
    
    char *analyzerName;

    char *dumpExeName;
    char *fileName_dumpProgress;
    
    /* memserver */
    char *exeName;
    char *unixsock;
    char **forkArgv;
    pid_t pid;

    /* client to memserver */
    char *eggPath;
    HEGGHANDLE hEggHandle;
    HEGGINDEXSEARCHER hSearcher;
    HEGGINDEXREADER hReader;
    HEGGINDEXWRITER hWriter;
    pthread_t threadid;
    pthread_mutex_t thread_input_mutex;
    pthread_cond_t thread_input_cond;
    EGGRWSLISTHEAD thread_input;
    int thread_control_flag;

    STATUSINSTRUCT si_head;
    struct timeval timeout;

    struct eggRWSMemServerDescriptor *next_timeout;
    struct eggRWSMemServerDescriptor *prev_timeout;

    struct eggRWSMemServerDescriptor *next_name;
    struct eggRWSMemServerDescriptor *prev_name;

};

int eggRWSMemServerDescriptor_isNameMatch(char *name);

#define eggRWSMemServerDescriptor_id(desTor) ((desTor)->id)
enum eggRWSMemServerStat {EGGRWSMEMSERVERSTAT_ERROR = -1, EGGRWSMEMSERVERSTAT_UNFORKED = 0, EGGRWSMEMSERVERSTAT_NORMAL, EGGRWSMEMSERVERSTAT_CLEANUP, EGGRWSMEMSERVERSTAT_DUMPING, EGGRWSMEMSERVERSTAT_HALTING, EGGRWSMEMSERVERSTAT_MASK = 0xff, EGGRWSMEMSERVERSTAT_WRITEABLE = 0x100, EGGRWSMEMSERVERSTAT_READABLE = 0x200, EGGRWSMEMSERVERSTAT_RETRY = 0x400};
char *eggRWSMemServerStat_str(enum eggRWSMemServerStat state);
#define eggRWSMemServerStat_getStat(stat) ((stat) & EGGRWSMEMSERVERSTAT_MASK)
#define eggRWSMemServerStat_setStat(stat, status) ((stat) = ((stat) & ~EGGRWSMEMSERVERSTAT_MASK) | ((status) & EGGRWSMEMSERVERSTAT_MASK))
#define eggRWSMemServerStat_is_readable(stat) ((stat) & EGGRWSMEMSERVERSTAT_READABLE)
#define eggRWSMemServerStat_is_writeable(stat) ((stat) & EGGRWSMEMSERVERSTAT_WRITEABLE)
#define eggRWSMemServerStat_is_retry(stat) ((stat) & EGGRWSMEMSERVERSTAT_RETRY)
#define eggRWSMemServerStat_clear_readable(stat) ((stat) = (stat) & ~EGGRWSMEMSERVERSTAT_READABLE)
#define eggRWSMemServerStat_clear_writeable(stat) ((stat) = (stat) & ~EGGRWSMEMSERVERSTAT_WRITEABLE)
#define eggRWSMemServerStat_clear_retry(stat) ((stat) = (stat) & ~EGGRWSMEMSERVERSTAT_RETRY)
#define eggRWSMemServerStat_set_readable(stat) ((stat) = ((stat) & ~EGGRWSMEMSERVERSTAT_READABLE) | EGGRWSMEMSERVERSTAT_READABLE)
#define eggRWSMemServerStat_set_writeable(stat) ((stat) = ((stat) & ~EGGRWSMEMSERVERSTAT_WRITEABLE) | EGGRWSMEMSERVERSTAT_WRITEABLE)
#define eggRWSMemServerStat_set_retry(stat) ((stat) = ((stat) & ~EGGRWSMEMSERVERSTAT_RETRY) | EGGRWSMEMSERVERSTAT_RETRY)


int eggRWSMemServerDescriptor_fork(HEGGRWSMEMSERVERDESCRIPTOR hMemServerD);
int eggRWSMemServerDescriptor_dumping(HEGGRWSMEMSERVERDESCRIPTOR hMemServerD);
int eggRWSMemServerDescriptor_halting(HEGGRWSMEMSERVERDESCRIPTOR hMemServerD);
int eggRWSMemServerDescriptor_end(HEGGRWSMEMSERVERDESCRIPTOR hMemServerD);

int eggRWSMemServerDescriptor_makeFlush(HEGGRWSMEMSERVERDESCRIPTOR hMemServerD);


int eggRWSMemClient_pushJob(HEGGRWSMEMSERVERDESCRIPTOR *pp_msd,
                                      int n_msd,
                                      EGGRWSJOBSPEC *p_job);
EGGRWSJOBSPEC *eggRWSMemClient_popJob(HEGGRWSMEMSERVERDESCRIPTOR hMemServerD);
int eggRWSMemClient_stop(HEGGRWSMEMSERVERDESCRIPTOR hMemServerD);


int eggRWSMemServerDescriptor_initialise(HEGGRWSMEMSERVERDESCRIPTOR hMemServerD,
                                     struct eggRWSMemServerManager *hManager);
int eggRWSMemServerDescriptor_release(HEGGRWSMEMSERVERDESCRIPTOR hMemServerD);

int eggRWSMemServerDescriptor_initialise_memServerCreator(HEGGRWSMEMSERVERDESCRIPTOR hMemServerD,
                                                         struct eggRWSMemServerManager *hManager);
int eggRWSMemServerDescriptor_is_memServerCreator(HEGGRWSMEMSERVERDESCRIPTOR hMemServerD);


int eggRWSMemServerDescriptor_cmpName(HEGGRWSMEMSERVERDESCRIPTOR p, HEGGRWSMEMSERVERDESCRIPTOR q);
int eggRWSMemServerDescriptor_cmpTimeout(HEGGRWSMEMSERVERDESCRIPTOR p, HEGGRWSMEMSERVERDESCRIPTOR q);

int eggRWSMemServerDescriptor_openClient(HEGGRWSMEMSERVERDESCRIPTOR hMemServerD);
int eggRWSMemServerDescriptor_closeClient(HEGGRWSMEMSERVERDESCRIPTOR hMemServerD);

int eggRWSMemServerDescriptor_prependStatInstruct(HEGGRWSMEMSERVERDESCRIPTOR hMemServerD,
                                                  int newstat, time_t timeToLive);
int eggRWSMemServerDescriptor_popStatInstruct(HEGGRWSMEMSERVERDESCRIPTOR hMemServerD);

int eggRWSMemServerDescriptor_userIncrease(HEGGRWSMEMSERVERDESCRIPTOR hMemServerD);
int eggRWSMemServerDescriptor_userDecrease(HEGGRWSMEMSERVERDESCRIPTOR hMemServerD);

#endif
