
#include "eggRWSIntServer.h"
#include "eggRWSBakerManager.h"
#include "eggRWSMemManager.h"
#include "eggRWSMergePackage.h"
#include "eggRWSCommon.h"
#include "eggRWSLog.h"
#include <assert.h>
#include <arpa/inet.h>
#include <signal.h>
#include <netinet/tcp.h>


#define LOG_INFO(who, ...) eggRWSLog_log_line(g_integrator_log, EGGRWSLOG_INFO, who, __VA_ARGS__)
#define LOG_WARN(who, ...) eggRWSLog_log_line(g_integrator_log, EGGRWSLOG_WARN, who, __VA_ARGS__)
#define LOG_ERR(who, ...) eggRWSLog_log_line(g_integrator_log, EGGRWSLOG_ERROR, who, __VA_ARGS__)
#define LOG_CLAIM(who, ...) eggRWSLog_log_line(g_integrator_log, EGGRWSLOG_CLAIM, who, __VA_ARGS__)

static void *eggRWSIntServer_worker(void *arg);


#define eggSocketFd_max_alive 1800
typedef struct eggSocketFd EGGSOCKETFD;
typedef struct eggSocketFd *HEGGSOCKETFD;
struct eggSocketFd {
    int fd;
    struct timeval last_alive;
};
HEGGSOCKETFD eggSocketFd_new(int fd);
int eggSocketFd_get_fd(HEGGSOCKETFD hSocketFd);
int eggSocketFd_set_alive(HEGGSOCKETFD hSocketFd, struct timeval alive_time);
int eggSocketFd_is_stale(HEGGSOCKETFD hSocketFd, HEGGRWSINTSERVER hIntServer);
int eggSocketFd_delete(HEGGSOCKETFD hSocketFd);


#define fd_setflag_cloexec(fd)                                          \
    {                                                                   \
        long flags;                                                     \
        if ((flags = fcntl(fd, F_GETFD)) < 0)                           \
        {                                                               \
            fprintf(stderr, "%s:%d:%s fcntl F_GETFD ERR %s\n",          \
                    __FILE__, __LINE__, __func__, strerror(errno));     \
            exit(EXIT_FAILURE);                                         \
        }                                                               \
        flags |= FD_CLOEXEC;                                            \
        if (fcntl(fd, F_SETFD, flags) < 0)                              \
        {                                                               \
            fprintf(stderr, "%s:%d:%s fcntl F_SETFD(fd_CLOEXEC) ERR %s\n", \
                    __FILE__, __LINE__, __func__, strerror(errno));     \
            exit(EXIT_FAILURE);                                         \
        }                                                               \
    }


/* stoplisten: stop writer. allow reader.
 */
#define EGGRWSINTSERVER_FLAG_STOPLISTEN 1
#define EGGRWSINTSERVER_FLAG_QUIT 2
#define EGGRWSINTSERVER_FLAG_STOPLISTEN_SIGNAL 4
int eggRWSIntServer_flag_setStopListen_nolock(HEGGRWSINTSERVER hIntServer)
{
    if (!hIntServer)
    {
        return 0;
    }
    ((hIntServer)->flag = (hIntServer)->flag | EGGRWSINTSERVER_FLAG_STOPLISTEN);
    return 0;
}
int eggRWSIntServer_flag_setStopListen(HEGGRWSINTSERVER hIntServer)
{
    if (!hIntServer)
    {
        return 0;
    }
    pthread_mutex_lock(&hIntServer->mutex);
    eggRWSIntServer_flag_setStopListen_nolock(hIntServer);
    pthread_mutex_unlock(&hIntServer->mutex);
    return 0;
}
int eggRWSIntServer_flag_clearStopListen_nolock(HEGGRWSINTSERVER hIntServer)
{
    if (!hIntServer)
    {
        return 0;
    }
    ((hIntServer)->flag = (hIntServer)->flag & ~EGGRWSINTSERVER_FLAG_STOPLISTEN);
    return 0;
}
int eggRWSIntServer_flag_clearStopListen(HEGGRWSINTSERVER hIntServer)
{
    if (!hIntServer)
    {
        return 0;
    }
    pthread_mutex_lock(&hIntServer->mutex);
    eggRWSIntServer_flag_clearStopListen_nolock(hIntServer);
    pthread_mutex_unlock(&hIntServer->mutex);
    return 0;
}
int eggRWSIntServer_flag_setStopListenSignal_nolock(HEGGRWSINTSERVER hIntServer)
{
    if (!hIntServer)
    {
        return 0;
    }
    ((hIntServer)->flag = (hIntServer)->flag | EGGRWSINTSERVER_FLAG_STOPLISTEN_SIGNAL);
    return 0;
}
int eggRWSIntServer_flag_setStopListenSignal(HEGGRWSINTSERVER hIntServer)
{
    if (!hIntServer)
    {
        return 0;
    }
    pthread_mutex_lock(&hIntServer->mutex);
    eggRWSIntServer_flag_setStopListenSignal_nolock(hIntServer);
    pthread_mutex_unlock(&hIntServer->mutex);
    return 0;
}
int eggRWSIntServer_flag_clearStopListenSignal_nolock(HEGGRWSINTSERVER hIntServer)
{
    if (!hIntServer)
    {
        return 0;
    }
    ((hIntServer)->flag = (hIntServer)->flag & ~EGGRWSINTSERVER_FLAG_STOPLISTEN_SIGNAL);
    return 0;
}
int eggRWSIntServer_flag_clearStopListenSignal(HEGGRWSINTSERVER hIntServer)
{
    if (!hIntServer)
    {
        return 0;
    }
    pthread_mutex_lock(&hIntServer->mutex);
    eggRWSIntServer_flag_clearStopListenSignal_nolock(hIntServer);
    pthread_mutex_unlock(&hIntServer->mutex);
    return 0;
}
int eggRWSIntServer_flag_isStopListen(HEGGRWSINTSERVER hIntServer)
{
    if (!hIntServer)
    {
        return 0;
    }
    pthread_mutex_lock(&hIntServer->mutex);
    int r;
    r = (((hIntServer)->flag) & (EGGRWSINTSERVER_FLAG_STOPLISTEN
                                 | EGGRWSINTSERVER_FLAG_STOPLISTEN_SIGNAL));
    pthread_mutex_unlock(&hIntServer->mutex);
    return r;
}
int eggRWSIntServer_flag_setQuit_nolock(HEGGRWSINTSERVER hIntServer)
{
    if (!hIntServer)
    {
        return 0;
    }
    ((hIntServer)->flag = (hIntServer)->flag | EGGRWSINTSERVER_FLAG_QUIT);
    return 0;
}
int eggRWSIntServer_flag_setQuit(HEGGRWSINTSERVER hIntServer)
{
    if (!hIntServer)
    {
        return 0;
    }
    pthread_mutex_lock(&hIntServer->mutex);
    eggRWSIntServer_flag_setQuit_nolock(hIntServer);
    pthread_mutex_unlock(&hIntServer->mutex);
    return 0;
}
int eggRWSIntServer_flag_isQuit(HEGGRWSINTSERVER hIntServer)
{
    if (!hIntServer)
    {
        return 0;
    }
    pthread_mutex_lock(&hIntServer->mutex);
    int r;
    r = (((hIntServer)->flag) & EGGRWSINTSERVER_FLAG_QUIT);
    pthread_mutex_unlock(&hIntServer->mutex);
    return r;
}
int eggRWSIntServer_flag_clearQuit_nolock(HEGGRWSINTSERVER hIntServer)
{
    if (!hIntServer)
    {
        return 0;
    }
    ((hIntServer)->flag = (hIntServer)->flag & ~EGGRWSINTSERVER_FLAG_QUIT);
    return 0;
}
int eggRWSIntServer_flag_clearQuit(HEGGRWSINTSERVER hIntServer)
{
    if (!hIntServer)
    {
        return 0;
    }
    pthread_mutex_lock(&hIntServer->mutex);
    eggRWSIntServer_flag_clearQuit_nolock(hIntServer);
    pthread_mutex_unlock(&hIntServer->mutex);
    return 0;
}


HEGGRWSINTSERVER eggRWSIntServer_new(HEGGRWSBAKERCFG hBakerCfg)
{
    /*     unlink (p_eggConfig->socketfile);    */
    /*     struct sockaddr_un addr; */

    /*     unixfd = socket(AF_UNIX, SOCK_STREAM, 0); */
        
    /*     if (unixfd == -1) */
    /*     { */
    /*         perror("socket"); */
    /*         exit(EXIT_FAILURE); */
    /*     } */
        
    /*     memset(&addr, 0, sizeof(struct sockaddr_un)); */
    /*                              /\* Clear structure *\/ */
    /*     addr.sun_family = AF_UNIX; */
        
    /*     strncpy(addr.sun_path, p_eggConfig->socketfile, */
    /*             sizeof(addr.sun_path) - 1); */

        
    /*     if (bind(unixfd, (struct sockaddr *) &addr, */
    /*              sizeof(struct sockaddr_un)) == -1) */
    /*     { */
    /*         perror("bind"); */
    /*         exit(EXIT_FAILURE); */
    /*     } */
        
    /*     listen(unixfd, EGGSERVICESERVER_LISTENNUM); */
    /* } */
    HEGGRWSINTSERVER hIntServer = NULL;
    hIntServer = calloc(1, sizeof(*hIntServer));
    assert(hIntServer);
    hIntServer->hBakerCfg = hBakerCfg;
    if(hBakerCfg->ip)
    {
        struct sockaddr_in servaddr;
        int inetfd;

        inetfd = socket(AF_INET, SOCK_STREAM, 0);
        if (inetfd == -1)
        {
            perror("socket");
            exit(EXIT_FAILURE);
        }
        
        bzero(&servaddr, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        
        inet_pton(AF_INET, hBakerCfg->ip, (void *)(&(servaddr.sin_addr.s_addr)));
        
        servaddr.sin_port = htons(hBakerCfg->port);

        int sock_reuse=1; 
	int chOpt = 1;

	int nErr = setsockopt(inetfd, IPPROTO_TCP, TCP_NODELAY, &chOpt, sizeof(int));        
	if(nErr == -1)
	  {
	    printf("error num : %d\n", errno);
            perror("setsockopt");
	    exit(1);
	  }
        if(setsockopt(inetfd, SOL_SOCKET,SO_REUSEADDR,(char *)&sock_reuse,sizeof(sock_reuse)) != 0)
        {
            printf("套接字可重用设置失败!/n");
            perror("setsockopt");
            exit(1);
        }
        long flags;
        if ((flags = fcntl(inetfd, F_GETFD)) < 0)
        {
            fprintf(stderr, "%s:%d:%s fcntl F_GETFD ERR %s\n", __FILE__, __LINE__, __func__,
                    strerror(errno));
            exit(EXIT_FAILURE);
        }
        flags |= FD_CLOEXEC;
        if (fcntl(inetfd, F_SETFD, flags) < 0)
        {
            fprintf(stderr, "%s:%d:%s fcntl F_SETFD(fd_CLOEXEC) ERR %s\n", __FILE__, __LINE__, __func__,
                    strerror(errno));
            exit(EXIT_FAILURE);
        }
        
        if( -1 == bind(inetfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) )
        {
            fprintf(stderr, "%s:%d:%s bind ERR %s\n", __FILE__, __LINE__, __func__,
                    strerror(errno));
            exit(EXIT_FAILURE);
        }
        
        listen(inetfd, 100);
        hIntServer->listen_tcp_fd = inetfd;
    }

    if (pipe(hIntServer->listen_wakeup_fd) < 0)
    {
        fprintf(stderr, "%s:%d:%s pipe ERR %s\n", __FILE__, __LINE__, __func__,
                strerror(errno));
        exit(EXIT_FAILURE);
    }
    fd_setflag_cloexec(hIntServer->listen_wakeup_fd[0]);
    fd_setflag_cloexec(hIntServer->listen_wakeup_fd[1]);    
    
    HEGGRWSMEMSERVERMANAGER hMemServerManager = NULL;
    hMemServerManager = eggRWSMemServerManager_new(hBakerCfg, hIntServer);

    HEGGRWSBAKERMANAGER hBakerManager = NULL;
    hBakerManager = eggRWSBakerManager_new(hBakerCfg, 0);
    hIntServer->hBakerManager = hBakerManager;


    if (hBakerCfg->connectThreadNum <= 0)
    {
        hIntServer->n_connectthread = hBakerCfg->bakerCnt;
    }
    else
    {
        hIntServer->n_connectthread = hBakerCfg->connectThreadNum;
    }
    hIntServer->connectthread_threadid = malloc(hIntServer->n_connectthread * sizeof(pthread_t));
    assert(hIntServer->connectthread_threadid);
    hIntServer->connectthread_flag = calloc(hIntServer->n_connectthread, sizeof(int));
    assert(hIntServer->connectthread_flag);
    hIntServer->data_queue = eggRWSCircularQueue_new(hIntServer->n_connectthread * 100);
    int i;
    pthread_attr_t threadattr;
    pthread_attr_init(&threadattr);
    pthread_attr_setdetachstate(&threadattr, PTHREAD_CREATE_DETACHED);
    for (i = 0; i < hIntServer->n_connectthread; i++)
    {
        if (pthread_create(&hIntServer->connectthread_threadid[i],
                           &threadattr, eggRWSIntServer_worker, (void*)hIntServer) != 0)
        {
            fprintf(stderr, "pthread_create ERR: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    hIntServer->datafds = eggRWSSet_new(300);

    pthread_mutex_init(&hIntServer->mutex, NULL);

    return hIntServer;
}

int eggRWSIntServer_delete(HEGGRWSINTSERVER hIntServer)
{
    if (hIntServer->listen_tcp_fd > 0)
    {
        close(hIntServer->listen_tcp_fd);
    }
    close(hIntServer->listen_wakeup_fd[0]);
    close(hIntServer->listen_wakeup_fd[1]);
    
    eggRWSMemServerManager_delete(hIntServer->hMemServerManager);
    
    eggRWSBakerManager_delete(hIntServer->hBakerManager);

    free(hIntServer->connectthread_threadid);
    free(hIntServer->connectthread_flag);
    eggRWSCircularQueue_delete(hIntServer->data_queue); /* --------------close fd */
    
    eggRWSSet_delete(hIntServer->datafds);

    pthread_mutex_destroy(&hIntServer->mutex);
    
    free(hIntServer);

    return 0;
}

int eggRWSIntServer_wakeUp(HEGGRWSINTSERVER  hIntServer)
{
    int r;
    if ((r = write(hIntServer->listen_wakeup_fd[1], "1", 1)) != 1)
    {
        char buf[100];
        strerror_r(errno, buf, sizeof(buf));

        fprintf(stderr, "%s:%d:%s write wakeup fd[%d]: %s. EXIT\n",
                __FILE__, __LINE__, __func__, hIntServer->listen_wakeup_fd[1], buf);
        
	
        LOG_ERR("Listenfd", "%s:%d:%s write wakeup fd[%d]: %s. EXIT\n",
                __FILE__, __LINE__, __func__, hIntServer->listen_wakeup_fd[1], buf);
        
        exit(EXIT_FAILURE);
    }
    
    return 0;
}


static int eggRWSIntServer_statis_clientfd_count(HEGGRWSINTSERVER hIntServer);
#define EGGRWSINTSERVER_CLIENTFD_MAX 800

static sigset_t s_signal_mask;
static void *eggRWSIntServer_signalhandler(void *arg);
static void eggRWSIntServer_sigfunc_TERM(int signo);
static void eggRWSIntServer_sigfunc_USR1(int signo);
static void eggRWSIntServer_sigfunc_USR2(int signo);
static int eggRWSIntServer_cleanUpMemServer(HEGGRWSINTSERVER hIntServer);
static int eggRWSIntServer_registerFd(HEGGRWSINTSERVER hIntServer, void *data);
static int eggRWSIntServer_ifFdSetRead(HEGGRWSINTSERVER hIntServer, void *data);
static int eggRWSIntServer_ifFdSetExcept(HEGGRWSINTSERVER hIntServer, void *data);
static int eggRWSIntServer_pushFdToWorker(HEGGRWSINTSERVER hIntServer, void *data);
static int eggRWSIntServer_closeFdExcept(HEGGRWSINTSERVER hIntServer, void *data);
static int eggRWSIntServer_ifFdStale(HEGGRWSINTSERVER hIntServer, void *data);
static int eggRWSIntServer_closeFdStale(HEGGRWSINTSERVER hIntServer, void *data);


HEGGRWSLOG g_integrator_log;
HEGGRWSINTSERVER  g_integrator_server;
int main(int argc ,char* argv[])
{
    
    {
        sigset_t *p_sigmask = &s_signal_mask;
        sigemptyset(p_sigmask);
        
        sigaddset(p_sigmask, SIGPIPE);
        if (pthread_sigmask(SIG_BLOCK, p_sigmask, NULL) != 0)
        {
            fprintf(stderr, "pthread_sigmask(SIGPIPE) < 0: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        sigaddset(p_sigmask, SIGTERM);
        if (pthread_sigmask(SIG_BLOCK, p_sigmask, NULL) != 0)
        {
            fprintf(stderr, "pthread_sigmask(SIGTERM) < 0: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        sigaddset(p_sigmask, SIGUSR1);
        if (pthread_sigmask(SIG_BLOCK, p_sigmask, NULL) != 0)
        {
            fprintf(stderr, "pthread_sigmask(SIGHUP) < 0: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        sigaddset(p_sigmask, SIGUSR2);
        if (pthread_sigmask(SIG_BLOCK, p_sigmask, NULL) != 0)
        {
            fprintf(stderr, "pthread_sigmask(SIGCONT) < 0: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        
        pthread_t threadid_sighandler;
        if (pthread_create(&threadid_sighandler, NULL, eggRWSIntServer_signalhandler, p_sigmask) != 0)
        {
            fprintf(stderr, "pthread_create < 0: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    char *configfile = "/etc/egg3/rws-eggd.cfg";

    HEGGRWSBAKERCFG hBakerConfig;
    hBakerConfig = eggRWSBakerCfg_new(configfile);
    
    if (argc < 2)
    {
        eggRWSBakerCfg_loadFile(hBakerConfig, NULL);
    }
    else
    {
        eggRWSBakerCfg_loadFile(hBakerConfig, argv[1]);
    }
    
    g_integrator_log = eggRWSLog_init(hBakerConfig->logFile);

    g_integrator_server = eggRWSIntServer_new(hBakerConfig);

    HEGGRWSINTSERVER  hIntServer = g_integrator_server;
    HEGGRWSMEMSERVERMANAGER hMemServerManager;
    hMemServerManager = hIntServer->hMemServerManager;

    if (!hBakerConfig->noWaitCleanUpMemServer
        && eggRWSMemServerManager_isNotClean(hMemServerManager))
    {
        pthread_mutex_lock(&hIntServer->mutex);
        eggRWSIntServer_flag_setStopListen_nolock(hIntServer);
        hIntServer->call = eggRWSIntServer_cleanUpMemServer;
        pthread_mutex_unlock(&hIntServer->mutex);
        
        LOG_CLAIM("eggRWSIntServer", "start with UNCLEAN. Beg Cleanup MemServer. STOP listen");
    }
    else
    {
        eggRWSMemServerManager_new_memServerCreator(hIntServer->hMemServerManager);
    }

    while (!eggRWSIntServer_flag_isQuit(hIntServer))
    {

        if (eggRWSMemServerManager_isTooHeavy(hMemServerManager))
        {
            eggRWSMemServerManager_delete_memServerCreator(hMemServerManager);
            pthread_mutex_lock(&hIntServer->mutex);
            eggRWSIntServer_flag_setStopListen_nolock(hIntServer);
            hIntServer->call = eggRWSIntServer_cleanUpMemServer;
            pthread_mutex_unlock(&hIntServer->mutex);
            
            LOG_CLAIM("eggRWSIntServer", "MemServer too HEAVY. Beg Cleanup MemServer. STOP listen");
        }
        
        int listenfd = hIntServer->listen_tcp_fd;
        int wakeupfd = hIntServer->listen_wakeup_fd[0];
        
        int *p_maxfd = &hIntServer->maxfd;
        fd_set *p_readfds = &hIntServer->readfds;
        fd_set *p_exceptfds = &hIntServer->exceptfds;
        FD_ZERO(p_readfds);
        FD_ZERO(p_exceptfds);

        /* DO NOT stop listen, but stop Writing.
        if (eggRWSIntServer_flag_isStopListen(hIntServer))
        {
            listenfd = -1;
        }
        else
        */
        {
            FD_SET(listenfd, p_readfds);
        }
        FD_SET(wakeupfd, p_readfds);
        *p_maxfd = listenfd > wakeupfd ? listenfd : wakeupfd;

        {
            eggRWSSet_remove(hIntServer->datafds,
                             (int (*)(void *, void *))eggRWSIntServer_ifFdStale,
                             (void *)hIntServer,
                             (int (*)(void *, void *))eggRWSIntServer_closeFdStale,
                             (void *)hIntServer);
        }
        
        {
            eggRWSSet_doAll(hIntServer->datafds,
                            (int (*)(void *, void *))eggRWSIntServer_registerFd, (void *)hIntServer);
        }
        
        struct timeval tv;
        struct timeval timeout;
        struct timeval *p_timeout = NULL;
        HEGGRWSMEMSERVERDESCRIPTOR hMemServerD;
        
        hMemServerD = eggRWSMemServerManager_getRecentTimeoutMemServer(hMemServerManager, &tv);
        
        if (hMemServerD)
        {
            struct timeval now;
            now = eggRWSMemServerManager_getTime(hMemServerManager);
            timeout = timeval_delta_nonnegative(tv, now);
            p_timeout = &timeout;
        }
        
        int r;
        r = select(*p_maxfd + 1, p_readfds, NULL, p_exceptfds, p_timeout);
        if (r == -1)
        {
	    LOG_ERR("main", "%s:%d:%s select %s. EXIT\n",
                    __FILE__, __LINE__, __func__, strerror(errno));
            exit(-1);
        }


        if (FD_ISSET(listenfd, p_readfds))
        {
            int n;
            int clientfd;
            if ((n = eggRWSIntServer_statis_clientfd_count(hIntServer)) >= EGGRWSINTSERVER_CLIENTFD_MAX)
            {
                LOG_WARN("Listenfd", "DENY connection, because of  n_clientfd[%d] >= EGGRWSINTSERVER_CLIENTFD_MAX[%d]",
                         n, EGGRWSINTSERVER_CLIENTFD_MAX);
            }
            else if ((clientfd = accept(listenfd, EGG_NULL, EGG_NULL)) < 0)
            {
                char buff[100] = "";
                strerror_r(errno, buff, sizeof(buff));
                LOG_ERR("Listenfd", "accept ERR: %s\n", buff);
		assert(0);
            }
            else
            {
                struct sockaddr peer_addr;
                socklen_t peer_addrlen = sizeof(struct sockaddr);
                getpeername(clientfd, &peer_addr, &peer_addrlen);
                char peer_host[30] = "";
                char peer_serv[20] = "";
                getnameinfo(&peer_addr, peer_addrlen, peer_host, sizeof(peer_host),
                            peer_serv, sizeof(peer_serv), NI_NUMERICHOST|NI_NUMERICSERV);
                
                LOG_INFO("Listenfd", "accept fd[%d] from [%s:%s]", clientfd, peer_host, peer_serv);
              
                int chOpt = 1;
                int nErr = setsockopt(clientfd, IPPROTO_TCP, TCP_NODELAY, &chOpt, sizeof(int));        
                if(nErr == -1)
                {
                    LOG_ERR("Listenfd", "sttsockopt: %s. EXIT\n", strerror(errno));
                    exit(1);
                }

                long flag;
                flag = fcntl(clientfd, F_GETFL);
                flag |= O_NONBLOCK;
                if (fcntl(clientfd, F_SETFL, flag) < 0)
                {
                    char buff[100] = "";
                    strerror_r(errno, buff, sizeof(buff));
                    LOG_ERR("Listenfd", "fcntl(%d, F_SETFL) < 0: %s. EXIT\n", clientfd, buff);
                    exit(-1);
                }
                fd_setflag_cloexec(clientfd);

                /* not support */
                /* flag = 1; */
                /* if (setsockopt(clientfd, SOL_SOCKET, MSG_NOSIGNAL, &flag, sizeof(flag)) < 0) */
                /* { */
                /*     char buff[100] = ""; */
                /*     strerror_r(errno, buff, sizeof(buff)); */
                /*     LOG_ERR("Listenfd", "setsockopt(%d, MSG_NOSIGNAL) < 0: %s. EXIT\n", clientfd, buff); */
                /*     exit(-1); */
                /* } */
                
                HEGGSOCKETFD hSocketFd = eggSocketFd_new(clientfd);
                struct timeval now;
                gettimeofday(&now, NULL);
                eggSocketFd_set_alive(hSocketFd, now);
                eggRWSSet_add(hIntServer->datafds, hSocketFd);
            }
        }

        {
            eggRWSSet_remove(hIntServer->datafds,
                             (int (*)(void *, void *))eggRWSIntServer_ifFdSetExcept,
                             (void *)hIntServer,
                             (int (*)(void *, void *))eggRWSIntServer_closeFdExcept,
                             (void *)hIntServer);
            
            eggRWSSet_remove(hIntServer->datafds,
                             (int (*)(void *, void *))eggRWSIntServer_ifFdSetRead,
                             (void *)hIntServer,
                             (int (*)(void *, void *))eggRWSIntServer_pushFdToWorker,
                             (void *)hIntServer);
        }
        
        if (FD_ISSET(wakeupfd, p_readfds))
        {
            char c;
            int r;
            r = read(wakeupfd, &c, 1);
            if (r!= 1)
            {
                char buf[100];
                strerror_r(errno, buf, sizeof(buf));

                fprintf(stderr, "%s:%d:%s read wakeup fd[%d]: %s. EXIT\n",
                        __FILE__, __LINE__, __func__, wakeupfd, buf);
		
                LOG_ERR("Listenfd", "%s:%d:%s read wakeup fd[%d]: %s. EXIT\n",
                        __FILE__, __LINE__, __func__, wakeupfd, buf);
                exit(EXIT_FAILURE);
            }
        }

        for ( ; ; )
        {
            
            hMemServerD = eggRWSMemServerManager_getRecentTimeoutMemServer(hMemServerManager, &tv);
            if (!hMemServerD)
            {
                break;
            }
            struct timeval now;
            now = eggRWSMemServerManager_getTime(hMemServerManager);
            timeout = timeval_delta_nonnegative(tv, now);
            if (timeout.tv_sec == 0 && timeout.tv_usec == 0)
            {
                eggRWSMemServerManager_trigger(hMemServerManager, hMemServerD);
            }
            else
            {
                break;
            }
        }

        if (hIntServer->call)
        {
            (*hIntServer->call)(hIntServer);
        }

        struct timeval time_next;
        gettimeofday(&time_next, NULL);
        eggRWSMemServerManager_setTime(hMemServerManager, time_next);

    }

    eggRWSIntServer_delete(g_integrator_server);
    eggRWSLog_uninit(g_integrator_log);
    eggRWSBakerCfg_delete(hBakerConfig);
    
    return 0;
}

HEGGSOCKETFD eggSocketFd_new(int fd)
{
    HEGGSOCKETFD hSocketFd = calloc(1, sizeof(*hSocketFd));
    assert(hSocketFd);
    hSocketFd->fd = fd;
    return hSocketFd;
}
int eggSocketFd_get_fd(HEGGSOCKETFD hSocketFd)
{
    if (!hSocketFd)
    {
        return -1;
    }
    return hSocketFd->fd;
    
}
int eggSocketFd_set_alive(HEGGSOCKETFD hSocketFd, struct timeval alive_time)
{
    if (!hSocketFd)
    {
        return -1;
    }
    hSocketFd->last_alive = alive_time;
    return 0;
}
int eggSocketFd_is_stale(HEGGSOCKETFD hSocketFd, HEGGRWSINTSERVER hIntServer)
{
    if (!hSocketFd)
    {
        return 0;
    }
    struct timeval now;
    gettimeofday(&now, NULL);
    if (now.tv_sec - hSocketFd->last_alive.tv_sec > eggSocketFd_max_alive)
    {
        return 1;
    }
    return 0;
}
int eggSocketFd_delete(HEGGSOCKETFD hSocketFd)
{
    if (!hSocketFd)
    {
        return 0;
    }
    
    free(hSocketFd);
    return 0;
}


static int eggRWSIntServer_registerFd(HEGGRWSINTSERVER hIntServer, void *data)
{
    if (!hIntServer)
    {
        return 0;
    }
    HEGGSOCKETFD hSocketFd = (HEGGSOCKETFD)data;

    assert(hSocketFd);
    
    int fd = eggSocketFd_get_fd(hSocketFd);
    
    int *p_maxfd = &hIntServer->maxfd;
    fd_set *p_readfds = &hIntServer->readfds;
    fd_set *p_exceptfds = &hIntServer->exceptfds;
    FD_SET(fd, p_readfds);
    FD_SET(fd, p_exceptfds);
    *p_maxfd = fd > *p_maxfd ? fd : *p_maxfd;
    return 0;
}

static int eggRWSIntServer_unregisterFd(HEGGRWSINTSERVER hIntServer, void *data)
{
    if (!hIntServer)
    {
        return 0;
    }

    HEGGSOCKETFD hSocketFd = (HEGGSOCKETFD)data;
    
    assert(hSocketFd);
    
    int fd = eggSocketFd_get_fd(hSocketFd);
    
    fd_set *p_readfds = &hIntServer->readfds;
    fd_set *p_exceptfds = &hIntServer->exceptfds;
    FD_CLR(fd, p_readfds);
    FD_CLR(fd, p_exceptfds);
    return 0;
}

static int eggRWSIntServer_ifFdSetRead(HEGGRWSINTSERVER hIntServer, void *data)
{
    if (!hIntServer)
    {
        return 0;
    }

    HEGGSOCKETFD hSocketFd = (HEGGSOCKETFD)data;
    
    assert(hSocketFd);
    
    int fd = eggSocketFd_get_fd(hSocketFd);
    
    fd_set *p_readfds = &hIntServer->readfds;
    return FD_ISSET(fd, p_readfds);
    
}

static int eggRWSIntServer_ifFdSetExcept(HEGGRWSINTSERVER hIntServer, void *data)
{
    if (!hIntServer)
    {
        return 0;
    }

    HEGGSOCKETFD hSocketFd = (HEGGSOCKETFD)data;

    assert(hSocketFd);
    
    int fd = eggSocketFd_get_fd(hSocketFd);

    
    fd_set *p_exceptfds = &hIntServer->exceptfds;
    return FD_ISSET(fd, p_exceptfds);
    
}

static int eggRWSIntServer_ifFdStale(HEGGRWSINTSERVER hIntServer, void *data)
{
    if (!hIntServer)
    {
        return 0;
    }

    HEGGSOCKETFD hSocketFd = (HEGGSOCKETFD)data;

    assert(hSocketFd);
    
    return eggSocketFd_is_stale(hSocketFd, hIntServer);
    
}

static int eggRWSIntServer_pushFdToWorker(HEGGRWSINTSERVER hIntServer, void *data)
{
    if (!hIntServer)
    {
        return 0;
    }
    eggRWSIntServer_unregisterFd(hIntServer, data);
    eggRWSCircularQueue_push(hIntServer->data_queue, data);
    return 0;
}

static int eggRWSIntServer_closeFdExcept(HEGGRWSINTSERVER hIntServer, void *data)
{
    if (!hIntServer)
    {
        return 0;
    }
    eggRWSIntServer_unregisterFd(hIntServer, data);

    HEGGSOCKETFD hSocketFd = (HEGGSOCKETFD)data;
    assert(hSocketFd);
    
    int fd = eggSocketFd_get_fd(hSocketFd);
    eggSocketFd_delete(hSocketFd);
    close(fd);
    LOG_INFO("eggRWSIntServer", "close fd[%d] because except", fd);
    return 0;
}

static int eggRWSIntServer_closeFdStale(HEGGRWSINTSERVER hIntServer, void *data)
{
    if (!hIntServer)
    {
        return 0;
    }
    
    eggRWSIntServer_unregisterFd(hIntServer, data);

    HEGGSOCKETFD hSocketFd = (HEGGSOCKETFD)data;
    assert(hSocketFd);
    
    int fd = eggSocketFd_get_fd(hSocketFd);
    eggSocketFd_delete(hSocketFd);
    close(fd);
    LOG_INFO("eggRWSIntServer", "close fd[%d] because stale", fd);
    return 0;
}

static void *eggRWSIntServer_signalhandler(void *arg)
{
    sigset_t *p_sigmask = (sigset_t *)arg;
    int       signo;
    
    for ( ; ; )
    {
        sigwait(p_sigmask, &signo);
        switch (signo)
        {
        case SIGPIPE:
            LOG_INFO("eggRWSIntServer", "recieve SIGPIPE");
            break;
            
        case SIGTERM:
            eggRWSIntServer_sigfunc_TERM(signo);
            break;
            
        case SIGUSR1:
            eggRWSIntServer_sigfunc_USR1(signo);
            break;

        case SIGUSR2:
            eggRWSIntServer_sigfunc_USR2(signo);
            break;
            
        default:
            break;
        }
    }
    return NULL;
}

static int eggRWSIntServer_shutdown(HEGGRWSINTSERVER hIntServer)
{
    HEGGRWSMEMSERVERMANAGER hMemServerManager;
    hMemServerManager = hIntServer->hMemServerManager;
    eggRWSMemServerManager_delete_memServerCreator(hMemServerManager);
    eggRWSMemServerManager_makeFlush_allMemServer(hMemServerManager);
    
    if (hMemServerManager->nMemServer > 0)
    {
        return 0;
    }
    LOG_CLAIM("eggRWSIntServer", "shutdown eggRWSIntServer");

    pthread_mutex_lock(&hIntServer->mutex);
    eggRWSIntServer_flag_setQuit_nolock(hIntServer);
    hIntServer->call = 0;
    pthread_mutex_unlock(&hIntServer->mutex);
    return 0;
}

static void eggRWSIntServer_sigfunc_TERM(int signo)
{
    LOG_CLAIM("eggRWSIntServer", "recieve SIGTERM");
    (void)signo;
    HEGGRWSINTSERVER  hIntServer;
    hIntServer = g_integrator_server;

    pthread_mutex_lock(&hIntServer->mutex);
    
    eggRWSIntServer_flag_setStopListen_nolock(hIntServer);
    hIntServer->call = eggRWSIntServer_shutdown;
    
    pthread_mutex_unlock(&hIntServer->mutex);

    eggRWSIntServer_wakeUp(hIntServer);
}

static void eggRWSIntServer_sigfunc_USR1(int signo)
{
    LOG_CLAIM("eggRWSIntServer", "recieve SIGUSR1");
    (void)signo;
    HEGGRWSINTSERVER  hIntServer;
    hIntServer = g_integrator_server;

    pthread_mutex_lock(&hIntServer->mutex);
    
    eggRWSIntServer_flag_setStopListenSignal_nolock(hIntServer);

    LOG_CLAIM("eggRWSIntServer", "SIGUSR1: STOP listen");
    
    pthread_mutex_unlock(&hIntServer->mutex);

    eggRWSIntServer_wakeUp(hIntServer);
}

static void eggRWSIntServer_sigfunc_USR2(int signo)
{
    LOG_CLAIM("eggRWSIntServer", "recieve SIGUSR2");
    (void)signo;
    HEGGRWSINTSERVER  hIntServer;
    hIntServer = g_integrator_server;

    pthread_mutex_lock(&hIntServer->mutex);
    
    eggRWSIntServer_flag_clearStopListenSignal_nolock(hIntServer);

    LOG_CLAIM("eggRWSIntServer", "SIGUSR2: START listen");
    
    pthread_mutex_unlock(&hIntServer->mutex);

    eggRWSIntServer_wakeUp(hIntServer);
}

static int eggRWSIntServer_cleanUpMemServer(HEGGRWSINTSERVER hIntServer)
{
    HEGGRWSMEMSERVERMANAGER hMemServerManager;
    hMemServerManager = hIntServer->hMemServerManager;
    
    if (hMemServerManager->nMemServer > 0)
    {
        return 0;
    }
    
    eggRWSMemServerManager_new_memServerCreator(hMemServerManager);

    pthread_mutex_lock(&hIntServer->mutex);
    
    eggRWSIntServer_flag_clearStopListen_nolock(hIntServer);
    hIntServer->call = 0;

    pthread_mutex_unlock(&hIntServer->mutex);

    LOG_CLAIM("eggRWSIntServer", "End Cleanup MemServer. START listen");
    return 0;
}

static int eggRWSIntServer_statis_clientfd_count(HEGGRWSINTSERVER hIntServer)
{
    if (!hIntServer)
    {
        return 0;
    }
    int n;
    pthread_mutex_lock(&hIntServer->mutex);
    n = eggRWSSet_count(hIntServer->datafds);
    pthread_mutex_unlock(&hIntServer->mutex);
    return n;
}


PRIVATE HEGGNETPACKAGE eggRWSIntServer_processing(HEGGRWSINTSERVER hIntServer, HEGGNETPACKAGE hNetPackage);
static const int RECEIVE_TRY = 10;
static int receivePackage(int fd, void *buf, int size)
{
    char *p = buf;
    int n = size;
    int receive_try = 0;
    while(n > 0)
    {
        int nn;
        if ((nn = read(fd, p, n)) > 0)
        {
            p += nn;
            n -= nn;
        }
        else if (nn < 0)
        {
            int errno_r = errno;
            if (errno_r == EAGAIN || errno_r == EWOULDBLOCK)
            {
                if (n == size)
                {
                    LOG_INFO("receivePackage", "fd[%d] read EAGAIN", fd);
                    return 0;
                }
                else
                {
                    if (receive_try >= RECEIVE_TRY)
                    {
                        LOG_ERR("receivePackage", "fd[%d] receive_try %d >= %d", fd, receive_try, RECEIVE_TRY);
                        return -1;
                    }

                    receive_try++;
                    sleep((receive_try / 1000) % 10 + 1);
                    continue;
                }
            }
            char buff[100] = "";
            strerror_r(errno_r, buff, sizeof(buff));

            LOG_ERR("receivePackage", "fd[%d] receive_try: %d read socket ERR: %s", fd, receive_try, buff);
            return -1;
        }
        else
        {
            LOG_INFO("receivePackage", "fd[%d] peer closed", fd);
            return -1;
        }
    }
    LOG_INFO("receivePackage", "fd[%d] receive %d", fd, size);
    return size;
}
static int sendPackage(int fd, void *buf, int size)
{
    char* p = buf;
    int n = size;
    while (n > 0)
    {
        int nn;
        if ((nn = write(fd, p, n)) > 0)
        {
            p += nn;
            n -= nn;
        }
        else if (nn < 0)
        {
            int errno_r = errno;
            if (errno_r == EAGAIN || errno_r == EWOULDBLOCK)
            {
                sleep(1);
                continue;
            }
            char buff[100] = "";
            strerror_r(errno_r, buff, sizeof(buff));
            
            LOG_ERR("sendPackage", "fd[%d] write socket ERR: %s\n", fd, buff);
            return -1;
        }
        else
        {
            ;
        }
    }
    LOG_INFO("sendPackage", "fd[%d] send %d", fd, size);
    return size;
}
int g_pthread=0;
void *eggRWSIntServer_worker(void *arg)
{
    HEGGRWSINTSERVER hIntServer = (HEGGRWSINTSERVER)arg;

    pthread_t threadid;
    threadid = pthread_self();
    int *p_threadflag = NULL;
    int ind;
    for (ind = 0; ind < hIntServer->n_connectthread; ind++)
    {
        if (hIntServer->connectthread_threadid[ind] == threadid)
        {
            p_threadflag = &hIntServer->connectthread_flag[ind];
            break;
        }
    }
    assert(p_threadflag);

    HEGGSOCKETFD hSocketFd;
    int clientfd;
    HEGGRWSCIRCULARQUEUE hQueue = hIntServer->data_queue;
    while (!eggRWSIntServer_flag_isQuit(hIntServer))
    {
        hSocketFd = (HEGGSOCKETFD)eggRWSCircularQueue_pop(hQueue);

        assert(hSocketFd);

        clientfd = eggSocketFd_get_fd(hSocketFd);

        LOG_INFO("IntServer_worker", "fd[%d] Beg processing", clientfd);
        for ( ; ; )
        {
        
            HEGGNETPACKAGE lp_recv_package = eggNetPackage_new(0);
            HEGGNETPACKAGE lp_send_package = EGG_NULL;
            int r;
            if ((r = receivePackage(clientfd,  lp_recv_package, sizeof(EGGNETPACKAGE))) < 0)
            {
                eggNetPackage_delete(lp_recv_package);
                goto peer_closed;
            }
            else if (r == 0)
            {
                eggNetPackage_delete(lp_recv_package);

                struct timeval now;
                gettimeofday(&now, 0);
                
                eggSocketFd_set_alive(hSocketFd, now);
                
                eggRWSSet_add(hIntServer->datafds, hSocketFd);
                eggRWSIntServer_wakeUp(hIntServer);
                LOG_INFO("IntServer_worker", "fd[%d] peer_blocked", clientfd);
                goto peer_blocked;
            }
            
            lp_recv_package = (HEGGNETPACKAGE)realloc(lp_recv_package, eggNetPackage_get_packagesize(lp_recv_package));
            if (eggNetPackage_get_packagesize(lp_recv_package) - sizeof(EGGNETPACKAGE) > 0)
            {
                if (receivePackage(clientfd,  lp_recv_package+1,
                                   eggNetPackage_get_packagesize(lp_recv_package) - sizeof(EGGNETPACKAGE)) <= 0)
                {
                    eggNetPackage_delete(lp_recv_package);
                    goto peer_closed;
                }
            }
            
            if (eggRWSIntServer_flag_isStopListen(hIntServer))
            {
                if (lp_recv_package->op == EGG_PACKAGE_OPTIMIZE)
                {
                    EBOOL ret = EGG_ERR_TRYAGAIN;
                    lp_send_package = eggNetPackage_new(0);
                    lp_send_package = eggNetPackage_add(lp_send_package, &ret, sizeof(ret), EGG_PACKAGE_RET);

                    LOG_INFO("IntServer_worker", "fd[%d] return TRYAGAIN, because too HEAVY ", clientfd);
                    goto processing_done;
                }
            }


            struct timeval time1, time2;
            LOG_INFO("IntServer_worker", "fd[%d] do processing beg ", clientfd);
            gettimeofday(&time1, 0);

            lp_send_package = eggRWSIntServer_processing(g_integrator_server, lp_recv_package);

            gettimeofday(&time2, 0);
            LOG_INFO("IntServer_worker", "fd[%d] do processing end [%f]", clientfd,
                     (time2.tv_sec - time1.tv_sec) + (time2.tv_usec - time1.tv_usec)/1000000.);
            time1 = time2;

        processing_done:

            LOG_INFO("IntServer_worker", "fd[%d] do sendPackage beg ", clientfd);
            gettimeofday(&time1, 0);

            if (sendPackage(clientfd, lp_send_package, eggNetPackage_get_packagesize(lp_send_package))
                < 0)
            {
                eggNetPackage_delete(lp_recv_package);
                eggNetPackage_delete(lp_send_package);
                LOG_ERR("IntServer_worker", "fd[%d] do sendPackage end sendPackage < 0", clientfd);
                goto peer_closed;
            }
            
            gettimeofday(&time2, 0);
            LOG_INFO("IntServer_worker", "fd[%d] do sedPackage end [%lu] [%f]", clientfd, (long unsigned)eggNetPackage_get_packagesize(lp_send_package), (time2.tv_sec - time1.tv_sec) + (time2.tv_usec - time1.tv_usec)/1000000.);
            
            
            eggNetPackage_delete(lp_recv_package);
            eggNetPackage_delete(lp_send_package);
        }
        
    peer_blocked:
        continue;

    peer_closed:
        LOG_INFO("IntServer_worker", "fd[%d] End processing", clientfd);
        
        LOG_INFO("IntServer_worker", "close fd[%d]", clientfd);
        
        close(clientfd);
        eggSocketFd_delete(hSocketFd);
        continue;
    }

    return (void *)-1;
}

PRIVATE HEGGNETPACKAGE eggRWSIntServer_processing_loadEgg(HEGGRWSINTSERVER hIntServer, HEGGNETPACKAGE hNetPackage);
PRIVATE HEGGNETPACKAGE eggRWSIntServer_processing_optimize(HEGGRWSINTSERVER hIntServer, HEGGNETPACKAGE hNetPackage);
PRIVATE HEGGNETPACKAGE eggRWSIntServer_processing_query_documents(HEGGRWSINTSERVER hIntServer, HEGGNETPACKAGE hNetPackage);
PRIVATE HEGGNETPACKAGE eggRWSIntServer_processing_query_count_documents(HEGGRWSINTSERVER hIntServer, HEGGNETPACKAGE hNetPackage);
PRIVATE HEGGNETPACKAGE
eggRWSIntServer_processing_query_documents_with_sort(HEGGRWSINTSERVER hIntServer, HEGGNETPACKAGE hNetPackage);
PRIVATE HEGGNETPACKAGE
eggRWSIntServer_processing_filter_documents(HEGGRWSINTSERVER hIntServer, HEGGNETPACKAGE hNetPackage);
PRIVATE HEGGNETPACKAGE
eggRWSIntServer_processing_query_documents(HEGGRWSINTSERVER hIntServer, HEGGNETPACKAGE hNetPackage);
PRIVATE HEGGNETPACKAGE
eggRWSIntServer_processing_query_documents_with_iter(HEGGRWSINTSERVER hIntServer, HEGGNETPACKAGE hNetPackage);
PRIVATE HEGGNETPACKAGE
eggRWSIntServer_processing_get_document(HEGGRWSINTSERVER hIntServer, HEGGNETPACKAGE hNetPackage);
PRIVATE HEGGNETPACKAGE eggRWSIntServer_processing_get_documentSet(HEGGRWSINTSERVER hIntServer, HEGGNETPACKAGE hNetPackage);
PRIVATE HEGGNETPACKAGE eggRWSIntServer_processing_export_document(HEGGRWSINTSERVER hIntServer, HEGGNETPACKAGE hNetPackage);
PRIVATE HEGGNETPACKAGE eggRWSIntServer_processing_get_docTotalCnt(HEGGRWSINTSERVER hIntServer, HEGGNETPACKAGE hNetPackage);
PRIVATE HEGGNETPACKAGE
eggRWSIntServer_processing_get_fieldNameInfo(HEGGRWSINTSERVER hIntServer, HEGGNETPACKAGE hNetPackage);
PRIVATE HEGGNETPACKAGE
eggRWSIntServer_processing_get_singleFieldNameInfo(HEGGRWSINTSERVER hIntServer, HEGGNETPACKAGE hNetPackage);
PRIVATE HEGGNETPACKAGE
eggRWSIntServer_processing_add_field(HEGGRWSINTSERVER hIntServer, HEGGNETPACKAGE hNetPackage);
PRIVATE HEGGNETPACKAGE
eggRWSIntServer_processing_modify_field(HEGGRWSINTSERVER hIntServer, HEGGNETPACKAGE hNetPackage);
PRIVATE HEGGNETPACKAGE        
eggRWSIntServer_processing_delete_field(HEGGRWSINTSERVER hIntServer, HEGGNETPACKAGE hNetPackage);



PRIVATE HEGGNETPACKAGE eggRWSIntServer_processing(HEGGRWSINTSERVER hIntServer, HEGGNETPACKAGE hNetPackage)
{
    
    HEGGNETPACKAGE lp_res_package = EGG_NULL;
    EBOOL ret;
    if(POINTER_IS_INVALID(hNetPackage))
    {
        ret = EGG_NET_IVDHANDLE;
        lp_res_package = eggNetPackage_new(0);
        lp_res_package = eggNetPackage_add(lp_res_package, &ret, sizeof(ret), EGG_PACKAGE_RET);
        return lp_res_package;
    }
    
    switch(hNetPackage->op)
    {
        
    case EGG_PACKAGE_LOADEGG:
        lp_res_package = eggRWSIntServer_processing_loadEgg(hIntServer, hNetPackage);break;
        
    case EGG_PACKAGE_OPTIMIZE:
        lp_res_package = eggRWSIntServer_processing_optimize(hIntServer, hNetPackage);break;
        
    case EGG_PACKAGE_SEARCH:
        lp_res_package = eggRWSIntServer_processing_query_documents(hIntServer, hNetPackage);break;

    case EGG_PACKAGE_SEARCH_COUNT:
        lp_res_package = eggRWSIntServer_processing_query_count_documents(hIntServer, hNetPackage);break;
        
    case EGG_PACKAGE_SEARCH_SORT:
        
        lp_res_package = eggRWSIntServer_processing_query_documents_with_sort(hIntServer, hNetPackage);break;
        
    case EGG_PACKAGE_SEARCHFILTER:
        lp_res_package = eggRWSIntServer_processing_filter_documents(hIntServer, hNetPackage);break;
        
    case EGG_PACKAGE_SEARCH_ITER:
        lp_res_package = eggRWSIntServer_processing_query_documents_with_iter(hIntServer, hNetPackage);break;
        
    case EGG_PACKAGE_GETDOC:
        lp_res_package = eggRWSIntServer_processing_get_document(hIntServer, hNetPackage);break;
        
    case EGG_PACKAGE_GETDOCSET:
        lp_res_package = eggRWSIntServer_processing_get_documentSet(hIntServer, hNetPackage);break;
        
    case EGG_PACKAGE_EXPORTDOC:
        lp_res_package = eggRWSIntServer_processing_export_document(hIntServer, hNetPackage);break;

    case EGG_PACKAGE_GETDOCTOTALCNT:
        lp_res_package = eggRWSIntServer_processing_get_docTotalCnt(hIntServer, hNetPackage);break;
        
    case EGG_PACKAGE_GETFIELDNAMEINFO:
        lp_res_package = eggRWSIntServer_processing_get_fieldNameInfo(hIntServer, hNetPackage);break;
        
    case EGG_PACKAGE_GETSINGLEFIELDNAMEINFO:
        lp_res_package = eggRWSIntServer_processing_get_singleFieldNameInfo(hIntServer, hNetPackage);break;
        
    case EGG_PACKAGE_ADDFIELD:
        lp_res_package = eggRWSIntServer_processing_add_field(hIntServer, hNetPackage);break;

    case EGG_PACKAGE_MODIFYFIELD:
        lp_res_package = eggRWSIntServer_processing_modify_field(hIntServer, hNetPackage);break;
        
    case EGG_PACKAGE_DELETEFIELD:
        lp_res_package = eggRWSIntServer_processing_delete_field(hIntServer, hNetPackage);break;
        
    default:
        ret = EGG_NET_IVDOP;
        lp_res_package = eggNetPackage_new(hNetPackage->op);
        lp_res_package = eggNetPackage_add(lp_res_package, &ret, sizeof(ret), EGG_PACKAGE_RET);
    }

    return lp_res_package;
}


PRIVATE HEGGNETPACKAGE eggRWSIntServer_processing_loadEgg(HEGGRWSINTSERVER hIntServer, HEGGNETPACKAGE hNetPackage)
{
    HEGGNETPACKAGE lp_res_package = eggNetPackage_new(EGG_PACKAGE_SEARCH);
    EBOOL ret = EGG_TRUE;

    LOG_WARN("IntServer_worker", "WARN loadEgg does Not supported\n");
    lp_res_package = eggNetPackage_add(lp_res_package, &ret, sizeof(ret), EGG_PACKAGE_RET);
    return lp_res_package;
}
PRIVATE HEGGNETPACKAGE eggRWSIntServer_processing_optimize(HEGGRWSINTSERVER hIntServer, HEGGNETPACKAGE hNetPackage)
{
    HEGGNETPACKAGE lp_res_package;
        
    HEGGRWSMEMSERVERMANAGER hMemServerManager = hIntServer->hMemServerManager;

    HEGGRWSMEMSERVERDESCRIPTOR pmsd;
    for ( ; ; )
    {
        
        pmsd = eggRWSMemServerManager_getWritableMemServer(hMemServerManager);
        if (pmsd)
        {
            break;
        }
        int n_sleep = 1;
        LOG_WARN("IntServer_worker", "WARN optimize No MemServer. Wait %ds\n", n_sleep);
        sleep(n_sleep);
    }
    
    EGGRWSJOBSPEC *p_job;
    p_job = EGGRWSJOBSPEC_new();
    EGGRWSJOBSPEC_setMemJob(p_job, hNetPackage, (void**)&pmsd, 1);
    eggRWSMemClient_pushJob(&pmsd, 1, p_job);
    EGGRWSJOBSPEC_waitJob(p_job);
    lp_res_package = (HEGGNETPACKAGE)EGGRWSJOBSPEC_getMemJobOutput(p_job, pmsd);
    EGGRWSJOBSPEC_delete(p_job);

    eggRWSMemServerDescriptor_userDecrease(pmsd);
    return lp_res_package;
    
}
PRIVATE HEGGRWSFIFOQUEUE eggRWSIntServer_processing_query(HEGGRWSINTSERVER hIntServer, HEGGNETPACKAGE hNetPackage)
{

    LOG_INFO("IntServer_worker", "eggRWSIntServer_processing_query beg");
    
    struct timeval time1, time2;
    HEGGRWSFIFOQUEUE hQueue;
    gettimeofday(&time1, 0);

    hQueue = eggRWSFifoQueue_new();

    HEGGRWSMEMSERVERMANAGER hMemServerManager = hIntServer->hMemServerManager;
    HEGGRWSMEMSERVERDESCRIPTOR *pp_msd = NULL;
    int n_msd = 0;
    EGGRWSMEMSERVERID n_min_id = (EGGRWSMEMSERVERID)eggRWSBakerManager_timestamp(hIntServer->hBakerManager);
    EGGRWSJOBSPEC *p_job = NULL;

    /* 不查内存,内存只用于同步
     */
    //pp_msd = eggRWSMemServerManager_getReadableMemServer(hMemServerManager, &n_msd, n_min_id);
    
    if (n_msd > 0)
    {
        p_job = EGGRWSJOBSPEC_new();        
        EGGRWSJOBSPEC_setMemJob(p_job, hNetPackage, (void **)pp_msd, n_msd);
        eggRWSMemClient_pushJob(pp_msd, n_msd, p_job);
    }


    LOG_INFO("IntServer_worker", "eggRWSIntServer_processing_query: memServer[%d]", n_msd);

    HEGGNETPACKAGE p_resultpackage_baker;
    HEGGRWSREQINFO hRWSReqInfo = eggRWSReqInfo_new(pthread_self(), 0);

    eggRWSBakerManager_getRespon(hIntServer->hBakerManager, hRWSReqInfo);

    HEGGRWSBAKER hBaker = hRWSReqInfo->baker;
    HEGGNETSERVER hNetServer = eggNetServer_new(NULL, NULL, NULL, NULL);
    hNetServer->hSearcher = hBaker->hSearcher;
    hNetServer->hReader = hBaker->hReader;
    hNetServer->hWriter = hBaker->hWriter;
    p_resultpackage_baker = eggNetServer_processing(hNetServer, hNetPackage);
    
    LOG_INFO("IntServer_worker",
	     "eggRWSIntServer_processing_query: baker index[%lu] timestamp[%llu] flag [%u] thrd[%llu] ",
	     (long unsigned)hBaker->bakIdx, (long long unsigned)hRWSReqInfo->timestamp,
	     (unsigned)hRWSReqInfo->flag, (long long unsigned)hRWSReqInfo->thrId);
    


    eggRWSBakerManager_free_baker(hIntServer->hBakerManager, hBaker);

    eggRWSReqInfo_delete(hRWSReqInfo);
    eggNetServer_delete(hNetServer);


    if (n_msd != 0)
    {
        EGGRWSJOBSPEC_waitJob(p_job);
    }
    int i;
    for (i = 0; i < n_msd; i++)
    {
        void *p_resultpackage_mem;
        p_resultpackage_mem = (HEGGNETPACKAGE)EGGRWSJOBSPEC_getMemJobOutput(p_job, (void*)pp_msd[i]);
        eggRWSFifoQueue_push(hQueue, p_resultpackage_mem);
    }
    if (n_msd != 0)
    {
        EGGRWSJOBSPEC_delete(p_job);
    }

    
    eggRWSFifoQueue_push(hQueue, p_resultpackage_baker);
        
    
    for (i = 0; i < n_msd; i++)
    {
        eggRWSMemServerDescriptor_userDecrease(pp_msd[i]);
    }
    free(pp_msd);
    gettimeofday(&time2, 0);

    LOG_INFO("IntServer_worker", "eggRWSIntServer_processing_query end. all time : %f", (double)(time2.tv_sec - time1.tv_sec) + (double)(time2.tv_usec - time1.tv_usec)/1000000);
    
    return hQueue;
    
}
PRIVATE HEGGNETPACKAGE eggRWSIntServer_processing_query_documents(HEGGRWSINTSERVER hIntServer, HEGGNETPACKAGE hNetPackage)
{
    HEGGRWSFIFOQUEUE hQueue;
    hQueue = eggRWSIntServer_processing_query(hIntServer, hNetPackage);
    
    HEGGNETPACKAGE p_resultpackage = NULL;
    p_resultpackage = eggRWSMergePackage_query(hQueue);
    eggRWSFifoQueue_delete(hQueue);

    return p_resultpackage;
}
PRIVATE HEGGNETPACKAGE eggRWSIntServer_processing_query_count_documents(HEGGRWSINTSERVER hIntServer, HEGGNETPACKAGE hNetPackage)
{
    HEGGRWSFIFOQUEUE hQueue;
    hQueue = eggRWSIntServer_processing_query(hIntServer, hNetPackage);
    
    HEGGNETPACKAGE p_resultpackage = NULL;
    p_resultpackage = eggRWSMergePackage_query(hQueue);
    eggRWSFifoQueue_delete(hQueue);

    return p_resultpackage;
}
PRIVATE HEGGNETPACKAGE
eggRWSIntServer_processing_query_documents_with_sort(HEGGRWSINTSERVER hIntServer, HEGGNETPACKAGE hNetPackage)
{
    HEGGRWSFIFOQUEUE hQueue;
    hQueue = eggRWSIntServer_processing_query(hIntServer, hNetPackage);

    HEGGNETPACKAGE p_resultpackage = NULL;
    p_resultpackage = eggRWSMergePackage_query(hQueue);
    eggRWSFifoQueue_delete(hQueue);

    return p_resultpackage;
}
PRIVATE HEGGNETPACKAGE
eggRWSIntServer_processing_filter_documents(HEGGRWSINTSERVER hIntServer, HEGGNETPACKAGE hNetPackage)
{
    HEGGNETPACKAGE lp_res_package = eggNetPackage_new(EGG_PACKAGE_SEARCHFILTER);
    EBOOL ret = EGG_FALSE;
    
    lp_res_package = eggNetPackage_add(lp_res_package, &ret, sizeof(ret), EGG_PACKAGE_RET);
    LOG_WARN("IntServer_worker", "WARN filter_documents Not implemented\n");
    return lp_res_package;
}
PRIVATE HEGGNETPACKAGE
eggRWSIntServer_processing_query_documents_with_iter(HEGGRWSINTSERVER hIntServer, HEGGNETPACKAGE hNetPackage)
{
    HEGGRWSFIFOQUEUE hQueue;
    hQueue = eggRWSIntServer_processing_query(hIntServer, hNetPackage);
    
    HEGGNETPACKAGE p_resultpackage = NULL;
    // p_resultpackage = eggRWSMergePackage_query_iter(hQueue);
    p_resultpackage = eggRWSFifoQueue_pop(hQueue);
    eggRWSFifoQueue_delete(hQueue);
    
    return p_resultpackage;
}
PRIVATE HEGGNETPACKAGE
eggRWSIntServer_processing_get_document(HEGGRWSINTSERVER hIntServer, HEGGNETPACKAGE hNetPackage)
{

    LOG_INFO("IntServer_worker", "eggRWSIntServer_processing_get_document beg");
    
    HEGGNETUNITPACKAGE lp_unit_package = (HEGGNETUNITPACKAGE)(hNetPackage + 1);
    EGGDID n_doc_id = *(EGGDID*)(lp_unit_package + 1);
    HEGGDOCUMENT p_document = EGG_NULL;

    HEGGRWSMEMSERVERDESCRIPTOR p_msd = NULL;
    /* 不查内存,内存只用于同步
     */
    //p_msd = eggRWSMemServerManager_getMemServerById(hIntServer->hMemServerManager, n_doc_id.cluster.chunkId);
    
    if (p_msd)
    {
        EGGRWSJOBSPEC *p_job;
        p_job = EGGRWSJOBSPEC_new();
        EGGRWSJOBSPEC_setMemJob(p_job, hNetPackage, (void **)&p_msd, 1);
        eggRWSMemClient_pushJob(&p_msd, 1, p_job);
        EGGRWSJOBSPEC_waitJob(p_job);
        void *p_resultpackage_mem;
        p_resultpackage_mem = (HEGGNETPACKAGE)EGGRWSJOBSPEC_getMemJobOutput(p_job, p_msd);
        EGGRWSJOBSPEC_delete(p_job);
        
        eggRWSMemServerDescriptor_userDecrease(p_msd);


        LOG_INFO("IntServer_worker", "eggRWSIntServer_processing_get_document from memServer");
        
        return (HEGGNETPACKAGE)p_resultpackage_mem;
    }


    LOG_INFO("IntServer_worker", "eggRWSIntServer_processing_get_document from baker");    
    
    HEGGNETPACKAGE p_resultpackage_baker;
    HEGGRWSREQINFO hRWSReqInfo = eggRWSReqInfo_new(pthread_self(), 0);
    eggRWSBakerManager_getRespon(hIntServer->hBakerManager, hRWSReqInfo);
    HEGGRWSBAKER hBaker = hRWSReqInfo->baker;
    HEGGNETSERVER hNetServer = eggNetServer_new(NULL, NULL, NULL, NULL);
    hNetServer->hSearcher = hBaker->hSearcher;
    hNetServer->hReader = hBaker->hReader;
    hNetServer->hWriter = hBaker->hWriter;
    p_resultpackage_baker = eggNetServer_processing(hNetServer, hNetPackage);
    eggRWSBakerManager_free_baker(hIntServer->hBakerManager, hBaker);
    eggRWSReqInfo_delete(hRWSReqInfo);
    eggNetServer_delete(hNetServer);

    LOG_INFO("IntServer_worker", "eggRWSIntServer_processing_get_document end");
    
    return p_resultpackage_baker;
    
}
PRIVATE HEGGNETPACKAGE
eggRWSIntServer_processing_get_documentSet(HEGGRWSINTSERVER hIntServer, HEGGNETPACKAGE hNetPackage)
{

    HEGGNETPACKAGE lp_res_package = eggNetPackage_new(EGG_PACKAGE_GETDOCSET);
    EBOOL ret;
    if(POINTER_IS_INVALID(hNetPackage))
    {
        ret = EGG_NET_IVDHANDLE;
        lp_res_package = eggNetPackage_add(lp_res_package, &ret, sizeof(ret), EGG_PACKAGE_RET);
        return lp_res_package;
    }
    
    HEGGNETUNITPACKAGE lp_unit_package = (HEGGNETUNITPACKAGE)(hNetPackage + 1);
    char* lp_net_res_org = (char*)malloc(lp_unit_package->size);
    char* lp_net_res = lp_net_res_org;
    memcpy(lp_net_res_org, lp_unit_package + 1, lp_unit_package->size);
    
    count_t n_scoreDoc_cnt = *((count_t*)(lp_net_res));
    lp_net_res += sizeof(count_t);
    HEGGSCOREDOC lp_score_doc = (HEGGSCOREDOC)lp_net_res;

    ret = EGG_TRUE;
    lp_res_package = eggNetPackage_add(lp_res_package, &ret, sizeof(ret), EGG_PACKAGE_RET);

    index_t n_scoreDoc_idx = 0;
    while(n_scoreDoc_idx < n_scoreDoc_cnt)
    {
        HEGGDOCUMENT p_document = EGG_NULL;
        HEGGDOCNODE lp_doc_node = EGG_NULL;


	HEGGNETPACKAGE p_package;
	p_package = eggNetPackage_new(EGG_PACKAGE_GETDOC);
	EGGDID dId = EGGSCOREDOC_ID_I(lp_score_doc, n_scoreDoc_idx);
	p_package = eggNetPackage_add(p_package, &dId, sizeof(dId), EGG_PACKAGE_ID);

	HEGGNETPACKAGE p_res_package_singledoc;
	p_res_package_singledoc = eggRWSIntServer_processing_get_document(hIntServer, p_package);
	
	eggNetPackage_delete(p_package);
	
	lp_res_package = eggNetPackage_add(lp_res_package, lp_doc_node, lp_doc_node->size, EGG_PACKAGE_DOC);

        n_scoreDoc_idx++;
    }
    
    free(lp_net_res_org);
    return lp_res_package;
    
}
PRIVATE HEGGNETPACKAGE
eggRWSIntServer_processing_export_document(HEGGRWSINTSERVER hIntServer, HEGGNETPACKAGE hNetPackage)
{
    HEGGNETPACKAGE lp_res_package = eggNetPackage_new(EGG_PACKAGE_GETFIELDNAMEINFO);
    EBOOL ret;

    LOG_WARN("IntServer_worker", "WARN export_document does Not supported\n");
    ret = EGG_NET_IVDHANDLE;
    lp_res_package = eggNetPackage_add(lp_res_package, &ret, sizeof(ret), EGG_PACKAGE_RET);
    return lp_res_package;
    
}
PRIVATE HEGGNETPACKAGE
eggRWSIntServer_processing_get_docTotalCnt(HEGGRWSINTSERVER hIntServer, HEGGNETPACKAGE hNetPackage)
{
    HEGGRWSFIFOQUEUE hQueue;
    hQueue = eggRWSFifoQueue_new();


    HEGGRWSMEMSERVERMANAGER hMemServerManager = hIntServer->hMemServerManager;
    HEGGRWSMEMSERVERDESCRIPTOR *pp_msd = NULL;
    int n_msd = 0;
    EGGRWSMEMSERVERID n_min_id = (EGGRWSMEMSERVERID)eggRWSBakerManager_timestamp(hIntServer->hBakerManager);

    /* 不查内存,内存只用于同步
     */
    //pp_msd = eggRWSMemServerManager_getReadableMemServer(hMemServerManager, &n_msd, n_min_id);
    
    EGGRWSJOBSPEC *p_job = NULL;
    if (pp_msd)
    {
        p_job = EGGRWSJOBSPEC_new();
        EGGRWSJOBSPEC_setMemJob(p_job, hNetPackage, (void **)pp_msd, n_msd);
        eggRWSMemClient_pushJob(pp_msd, n_msd, p_job);
    }

    HEGGNETPACKAGE p_resultpackage_baker;
    HEGGRWSREQINFO hRWSReqInfo = eggRWSReqInfo_new(pthread_self(), 0);
    eggRWSBakerManager_getRespon(hIntServer->hBakerManager, hRWSReqInfo);
    HEGGRWSBAKER hBaker = hRWSReqInfo->baker;
    HEGGNETSERVER hNetServer = eggNetServer_new(NULL, NULL, NULL, NULL);
    hNetServer->hSearcher = hBaker->hSearcher;
    hNetServer->hReader = hBaker->hReader;
    hNetServer->hWriter = hBaker->hWriter;
    p_resultpackage_baker = eggNetServer_processing(hNetServer, hNetPackage);
    eggRWSBakerManager_free_baker(hIntServer->hBakerManager, hBaker);
    eggRWSReqInfo_delete(hRWSReqInfo);
    eggNetServer_delete(hNetServer);


    if (n_msd != 0)
    {
        EGGRWSJOBSPEC_waitJob(p_job);
    }
    int i;
    for (i = 0; i < n_msd; i++)
    {
        void *p_resultpackage_mem;
        p_resultpackage_mem = (HEGGNETPACKAGE)EGGRWSJOBSPEC_getMemJobOutput(p_job, (void*)pp_msd[i]);
        eggRWSFifoQueue_push(hQueue, p_resultpackage_mem);
    }
    if (pp_msd)
    {
        EGGRWSJOBSPEC_delete(p_job);
    }

    
    eggRWSFifoQueue_push(hQueue, p_resultpackage_baker);
        
    
    HEGGNETPACKAGE p_resultpackage = NULL;
    p_resultpackage = eggRWSMergePackage_get_docTotalCnt(hQueue);
    eggRWSFifoQueue_delete(hQueue);

    for (i = 0; i < n_msd; i++)
    {
        eggRWSMemServerDescriptor_userDecrease(pp_msd[i]);
    }
    free(pp_msd);
    
    return p_resultpackage;

}
PRIVATE HEGGNETPACKAGE
eggRWSIntServer_processing_get_fieldNameInfo(HEGGRWSINTSERVER hIntServer, HEGGNETPACKAGE hNetPackage)
{
    HEGGNETPACKAGE lp_res_package = eggNetPackage_new(EGG_PACKAGE_GETFIELDNAMEINFO);
    EBOOL ret;

    LOG_WARN("IntServer_worker", "WARN get_fieldNameInfo does Not supported\n");
    ret = EGG_NET_IVDHANDLE;
    lp_res_package = eggNetPackage_add(lp_res_package, &ret, sizeof(ret), EGG_PACKAGE_RET);
    return lp_res_package;
}
PRIVATE HEGGNETPACKAGE
eggRWSIntServer_processing_get_singleFieldNameInfo(HEGGRWSINTSERVER hIntServer, HEGGNETPACKAGE hNetPackage)
{
    (void)hIntServer;
    HEGGNETPACKAGE lp_res_package = eggNetPackage_new(EGG_PACKAGE_GETSINGLEFIELDNAMEINFO);
    EBOOL ret;

    LOG_WARN("IntServer_worker", "WARN get_singleFieldNameInfo does Not supported\n");
    ret = EGG_NET_IVDHANDLE;
    lp_res_package = eggNetPackage_add(lp_res_package, &ret, sizeof(ret), EGG_PACKAGE_RET);
    return lp_res_package;
}
PRIVATE HEGGNETPACKAGE
eggRWSIntServer_processing_add_field(HEGGRWSINTSERVER hIntServer, HEGGNETPACKAGE hNetPackage)
{
    (void)hIntServer;
    HEGGNETPACKAGE lp_res_package = eggNetPackage_new(EGG_PACKAGE_GETSINGLEFIELDNAMEINFO);
    EBOOL ret;

    LOG_WARN("IntServer_worker", "WARN add_field does Not supported\n");
    ret = EGG_NET_IVDHANDLE;
    lp_res_package = eggNetPackage_add(lp_res_package, &ret, sizeof(ret), EGG_PACKAGE_RET);
    return lp_res_package;
}
PRIVATE HEGGNETPACKAGE
eggRWSIntServer_processing_modify_field(HEGGRWSINTSERVER hIntServer, HEGGNETPACKAGE hNetPackage)
{
    (void)hIntServer;
    HEGGNETPACKAGE lp_res_package = eggNetPackage_new(EGG_PACKAGE_GETSINGLEFIELDNAMEINFO);
    EBOOL ret;

    LOG_WARN("IntServer_worker", "WARN modify_field does Not supported\n");
    ret = EGG_NET_IVDHANDLE;
    lp_res_package = eggNetPackage_add(lp_res_package, &ret, sizeof(ret), EGG_PACKAGE_RET);
    return lp_res_package;
    
}
PRIVATE HEGGNETPACKAGE
eggRWSIntServer_processing_delete_field(HEGGRWSINTSERVER hIntServer, HEGGNETPACKAGE hNetPackage)
{
    (void)hIntServer;
    HEGGNETPACKAGE lp_res_package = eggNetPackage_new(EGG_PACKAGE_GETSINGLEFIELDNAMEINFO);
    EBOOL ret;

    LOG_WARN("IntServer_worker", "WARN delete_field Not supported\n");
    ret = EGG_NET_IVDHANDLE;
    lp_res_package = eggNetPackage_add(lp_res_package, &ret, sizeof(ret), EGG_PACKAGE_RET);
    return lp_res_package;
    
}

