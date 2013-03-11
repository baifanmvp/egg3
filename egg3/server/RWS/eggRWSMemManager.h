#ifndef EGGRWSMEMMANAGER_H_
#define EGGRWSMEMMANAGER_H_

#include "eggRWSMemDptor.h"
#include "eggRWSBakerCfg.h"

struct eggRWSIntServer;
typedef struct eggRWSMemServerManager EGGRWSMEMSERVERMANAGER;
typedef struct eggRWSMemServerManager *HEGGRWSMEMSERVERMANAGER;
struct eggRWSMemServerManager
{
    int nMemServer;
    EGGRWSMEMSERVERDESCRIPTOR memServerD_head;
    pthread_mutex_t mutex_head;
    
    int age_newMemServer_minutes;
    struct timeval now;
    struct eggRWSIntServer *hIntServer;

    pthread_mutex_t mutex_dumping;
    HEGGRWSMEMSERVERDESCRIPTOR p_dumping;
    
    int NUM_MEMSERVER_MAX;
};

HEGGRWSMEMSERVERMANAGER eggRWSMemServerManager_new(HEGGRWSBAKERCFG hConfig, struct eggRWSIntServer *hIntServer);
int eggRWSMemServerManager_delete(HEGGRWSMEMSERVERMANAGER hManager);

int eggRWSMemServerManager_isNotClean(HEGGRWSMEMSERVERMANAGER hManager);

int eggRWSMemServerManager_isTooHeavy(HEGGRWSMEMSERVERMANAGER hManager);

struct timeval eggRWSMemServerManager_getTime(HEGGRWSMEMSERVERMANAGER hMemServerManager);
int eggRWSMemServerManager_setTime(HEGGRWSMEMSERVERMANAGER hMemServerManager, struct timeval now);

HEGGRWSMEMSERVERDESCRIPTOR eggRWSMemServerManager_new_memServerCreator(HEGGRWSMEMSERVERMANAGER hManager);
int eggRWSMemServerManager_delete_memServerCreator(HEGGRWSMEMSERVERMANAGER hManager);

int eggRWSMemServerManager_makeFlush_allMemServer(HEGGRWSMEMSERVERMANAGER hManager);

int eggRWSMemServerManager_addMemServer(HEGGRWSMEMSERVERMANAGER hMemServerManager,
                                            HEGGRWSMEMSERVERDESCRIPTOR hMemServerD);
int eggRWSMemServerManager_addMemServer_nolock(HEGGRWSMEMSERVERMANAGER hMemServerManager,
                                            HEGGRWSMEMSERVERDESCRIPTOR hMemServerD);

int eggRWSMemServerManager_delMemServer(HEGGRWSMEMSERVERMANAGER hMemServerManager,
                                     HEGGRWSMEMSERVERDESCRIPTOR hMemServerD);
int eggRWSMemServerManager_delMemServer_nolock(HEGGRWSMEMSERVERMANAGER hMemServerManager,
                                               HEGGRWSMEMSERVERDESCRIPTOR hMemServerD);

int eggRWSMemServerManager_setupTimeout_memServer(HEGGRWSMEMSERVERMANAGER hMemServerManager,
                                                      HEGGRWSMEMSERVERDESCRIPTOR hMemServerD);
int eggRWSMemServerManager_setupTimeout_memServer_nolock(HEGGRWSMEMSERVERMANAGER hManager,
                                                                HEGGRWSMEMSERVERDESCRIPTOR hMemServerD);

HEGGRWSMEMSERVERDESCRIPTOR
eggRWSMemServerManager_getMemServerById(HEGGRWSMEMSERVERMANAGER hMemServerManager, EGGRWSMEMSERVERID id);

HEGGRWSMEMSERVERDESCRIPTOR
eggRWSMemServerManager_getRecentTimeoutMemServer(HEGGRWSMEMSERVERMANAGER hMemServerManager, struct timeval *p_tv);

HEGGRWSMEMSERVERDESCRIPTOR
eggRWSMemServerManager_getNextDumpingMemServer(HEGGRWSMEMSERVERMANAGER hMemServerManager);

HEGGRWSMEMSERVERDESCRIPTOR
eggRWSMemServerManager_getNextDumpingMemServer_nolock(HEGGRWSMEMSERVERMANAGER hMemServerManager);

HEGGRWSMEMSERVERDESCRIPTOR
eggRWSMemServerManager_getWritableMemServer(HEGGRWSMEMSERVERMANAGER hMemServerManager);

HEGGRWSMEMSERVERDESCRIPTOR *
eggRWSMemServerManager_getReadableMemServer(HEGGRWSMEMSERVERMANAGER hMemServerManager, int *count, EGGRWSMEMSERVERID minId);


int eggRWSMemServerManager_trigger(HEGGRWSMEMSERVERMANAGER hMemServerManager,
                                HEGGRWSMEMSERVERDESCRIPTOR hMemServerD);


int eggRWSMemServerManager_askDump(HEGGRWSMEMSERVERMANAGER hMemServerManager, HEGGRWSMEMSERVERDESCRIPTOR hMemServerD);
int eggRWSMemServerManager_unaskDump(HEGGRWSMEMSERVERMANAGER hManager, HEGGRWSMEMSERVERDESCRIPTOR hMemServerD);

int eggRWSMemServerManager_signalRetry(HEGGRWSMEMSERVERMANAGER hMemServerManager);

#endif
