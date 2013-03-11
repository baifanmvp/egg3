#ifndef EGGRWSINTSERVER_H_
#define EGGRWSINTSERVER_H_

#include "eggRWSBakerManager.h"
#include "eggRWSMemManager.h"
#include "eggRWSLog.h"
#include "eggRWSCommon.h"

typedef struct eggRWSIntServer EGGRWSINTSERVER;
typedef struct eggRWSIntServer *HEGGRWSINTSERVER;
struct eggRWSIntServer {
    HEGGRWSBAKERCFG hBakerCfg;

    fd_set readfds;
    fd_set exceptfds;
    int maxfd;
    int listen_tcp_fd;
    int listen_wakeup_fd[2];
    HEGGRWSSET datafds;
    
    int n_connectthread;
    pthread_t *connectthread_threadid;
    int *connectthread_flag;
    HEGGRWSCIRCULARQUEUE data_queue;

    pthread_mutex_t mutex;
    
    int flag;
    int (*call)(struct eggRWSIntServer *);

    char *workdir;
    HEGGRWSMEMSERVERMANAGER hMemServerManager;

    HEGGRWSBAKERMANAGER hBakerManager;
};

int eggRWSIntServer_flag_setStopListen(HEGGRWSINTSERVER hIntServer);
int eggRWSIntServer_flag_isStopListen(HEGGRWSINTSERVER hIntServer);
int eggRWSIntServer_flag_clearStopListen(HEGGRWSINTSERVER hIntServer);
int eggRWSIntServer_flag_setQuit(HEGGRWSINTSERVER hIntServer);
int eggRWSIntServer_flag_isQuit(HEGGRWSINTSERVER hIntServer);
int eggRWSIntServer_flag_clearQuit(HEGGRWSINTSERVER hIntServer);

extern HEGGRWSLOG g_integrator_log;
extern HEGGRWSINTSERVER  g_integrator_server;

int eggRWSIntServer_wakeUp(HEGGRWSINTSERVER  hIntServer);

#endif
