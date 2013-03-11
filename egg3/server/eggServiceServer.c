#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>

#include <sys/types.h>
#include <egg3/Egg3.h>
#include "eggServiceServerConfig.h"
#include <egg3/net/eggNetType.h>
#include <egg3/log/eggPrtLog.h>
#include <stdio.h>
#include <assert.h>
#include <sys/select.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>
#include <glib.h>


#define EGGSERVICESERVER_LISTENNUM 50
#define EGGSERVICESERVER_FDNUM 2
#define EGGWORKERSERVER_FDNUM 1024

#define  SIGNETEXIT SIGUSR1 
typedef struct eggServiceServer EGGSERVICESERVER;
typedef struct eggServiceServer* HEGGSERVICESERVER;

struct eggServiceServer
{
    //0 is unix; 1 is inet;
    int sockfd[EGGSERVICESERVER_FDNUM];
    int requestfd[EGGSERVICESERVER_FDNUM];
    int requestCnt;
    
    pthread_t workerfd[EGGWORKERSERVER_FDNUM];
    int  workerCnt;

    HEGGPRTLOG  hPrtLog;
};

extern HEGGNETINDEXLIST g_eggNet_list ;
HEGGSERVICESERVER g_service_server = EGG_NULL;
HEGGSERVICESERVERCONFIG g_eggServiceServerConfig = EGG_NULL;

PRIVATE EBOOL eggServiceServer_recv(HEGGNETSERVER hNetServer, epointer ePointer, size32_t size, int flags);

PRIVATE EBOOL eggServiceServer_send(HEGGNETSERVER hNetServer, epointer ePointer, size32_t size, int flags);

PRIVATE EBOOL eggServiceServer_processing(HEGGTHREADTASKDETAILS workerInfo);

EBOOL eggServiceServer_clean_workerId(HEGGSERVICESERVER hServiceServer, pthread_t workerId);

EBOOL eggServiceServer_add_workerId(HEGGSERVICESERVER hServiceServer, pthread_t workerId);

EBOOL eggServiceServer_wait_worker(HEGGSERVICESERVER hServiceServer);

void sigPipeHandler(int i)
{
    printf("SIGPIPE signal !!!\n");
    return ;
}

HEGGSERVICESERVER eggServiceServer_new(HEGGSERVICESERVERCONFIG p_eggConfig)
{
    if(POINTER_IS_INVALID(p_eggConfig))
    {
        return EGG_NULL;
    }

    int inetfd, unixfd;
    HEGGPRTLOG hPrtLog = EGG_NULL;
    if(p_eggConfig->logfile)
    {
        hPrtLog = eggPrtLog_init(p_eggConfig->logfile);
        eggPrtLog_set_level(hPrtLog, p_eggConfig->loglevel);
    }
    
    if(p_eggConfig->socketfile)
    {
        unlink (p_eggConfig->socketfile);   
        struct sockaddr_un addr;

        unixfd = socket(AF_UNIX, SOCK_STREAM, 0);
        
        if (unixfd == -1)
        {
            eggPrtLog_log_line(hPrtLog, EGGPRTLOG_ERROR, "eggServiceServer", "eggServiceServer_new socket is error[%s] [%d] [%s]\n",__func__, __LINE__, strerror(errno));

            exit(EXIT_FAILURE);
        }
        
        memset(&addr, 0, sizeof(struct sockaddr_un));
                                 /* Clear structure */
        addr.sun_family = AF_UNIX;
        
        strncpy(addr.sun_path, p_eggConfig->socketfile,
                sizeof(addr.sun_path) - 1);

        
        if (bind(unixfd, (struct sockaddr *) &addr,
                 sizeof(struct sockaddr_un)) == -1)
        {
            eggPrtLog_log_line(hPrtLog, EGGPRTLOG_ERROR, "eggServiceServer", "eggServiceServer_new bind is error[%s] [%d] [%s]\n",__func__, __LINE__, strerror(errno));
            exit(EXIT_FAILURE);
        }
        
        listen(unixfd, EGGSERVICESERVER_LISTENNUM);
    }
    
    if(p_eggConfig->ip)
    {
        struct sockaddr_in servaddr;

        inetfd = socket(AF_INET, SOCK_STREAM, 0);
        if (inetfd == -1)
        {
            eggPrtLog_log_line(hPrtLog, EGGPRTLOG_ERROR, "eggServiceServer", "eggServiceServer_new socket is error[%s] [%d] [%s]\n",__func__, __LINE__, strerror(errno));
            exit(EXIT_FAILURE);
        }
        
        bzero(&servaddr, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        
        inet_pton(AF_INET, p_eggConfig->ip, (void *)(&(servaddr.sin_addr.s_addr)));
        
        servaddr.sin_port = htons(p_eggConfig->port);

        int sock_reuse=1; 
        
        if(setsockopt(inetfd, SOL_SOCKET,SO_REUSEADDR,(char *)&sock_reuse,sizeof(sock_reuse)) != 0)
        {
            eggPrtLog_log_line(hPrtLog, EGGPRTLOG_ERROR, "eggServiceServer", "eggServiceServer_new setsockopt is error[%s] [%d] [%s]\n",__func__, __LINE__, strerror(errno));
            exit(1);
        } 

        
        if( -1 == bind(inetfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) )
        {
            eggPrtLog_log_line(hPrtLog, EGGPRTLOG_ERROR, "eggServiceServer", "eggServiceServer_new bind is error[%s] [%d] [%s]\n",__func__, __LINE__, strerror(errno));
            exit(EXIT_FAILURE);
        }
        
        listen(inetfd, EGGSERVICESERVER_LISTENNUM);
        
    }

    
    HEGGSERVICESERVER lp_service_server = (HEGGSERVICESERVER)malloc(sizeof(EGGSERVICESERVER));
    memset(lp_service_server, 0, sizeof(EGGSERVICESERVER));
    
    lp_service_server->sockfd[0] = unixfd;
    lp_service_server->sockfd[1] = inetfd;
    lp_service_server->hPrtLog = hPrtLog;
    return lp_service_server;
}


EBOOL eggServiceServer_delete(HEGGSERVICESERVER pServiceServer)
{
    index_t idx = 0;
    while( idx < EGGSERVICESERVER_FDNUM)
    {
        close(pServiceServer->sockfd[idx]);
        idx ++;
    }
    free(pServiceServer);
    return EGG_TRUE;
}


EBOOL eggServiceServer_get_request(HEGGSERVICESERVER hServiceServer)
{
    if(POINTER_IS_INVALID(hServiceServer))
    {
        return EGG_FALSE;
    }
    
    int idx = 0;
    int rtidx = 0;
    int maxfd = -1;
    fd_set fds;
    HEGGPRTLOG hPrtLog = hServiceServer->hPrtLog;
    
    
    FD_ZERO(&fds);
    
    for(idx = 0; idx < EGGSERVICESERVER_FDNUM; idx++)
    {
        maxfd = maxfd > hServiceServer->sockfd[idx] ? maxfd : hServiceServer->sockfd[idx];
        FD_SET(hServiceServer->sockfd[idx], &fds); 
    }
    maxfd ++;
    
    switch (select(maxfd, &fds, EGG_NULL, EGG_NULL, EGG_NULL))
    {
    case -1:
        eggPrtLog_log_line(hPrtLog, EGGPRTLOG_ERROR, "eggServiceServer", "eggServiceServer_get_request select is error [%s] [%d] [%s]\n",__func__, __LINE__, strerror(errno));
     exit(-1);
        
    case 0:
        eggPrtLog_log_line(hPrtLog, EGGPRTLOG_WARN, "eggServiceServer", "eggServiceServer_get_request select is timeout! [%s] [%d] \n",__func__, __LINE__);
        break; //再次轮询 
        
    default:
        rtidx = 0;
        for(idx = 0; idx < EGGSERVICESERVER_FDNUM; idx++)
        {
            if(FD_ISSET(hServiceServer->sockfd[idx], &fds)) //测试sock是否可读，即是否网络上有数据 
            {
                int fd;
                fd = accept(hServiceServer->sockfd[idx], EGG_NULL, EGG_NULL);
                hServiceServer->requestfd[rtidx++] = fd;
            }
        }
        hServiceServer->requestCnt = rtidx;
    }
    return EGG_TRUE;
    
}



EBOOL eggServiceServer_scheduleWorker(HEGGSERVICESERVER hServiceServer)
{
    if(POINTER_IS_INVALID(hServiceServer))
    {
        return EGG_FALSE;
    }

    HEGGPRTLOG hPrtLog = hServiceServer->hPrtLog;
    
    HEGGTHREADTASKDETAILS* lplp_details = (HEGGTHREADTASKDETAILS*)malloc(sizeof(HEGGTHREADTASKDETAILS) * hServiceServer->requestCnt);
    
    index_t n_request_idx = 0;
    pthread_t pthread_ids;
    int n_thr_ret = 0;
    while(n_request_idx < hServiceServer->requestCnt)
    {
//        printf("hServiceServer->requestId %d\n", hServiceServer->requestfd[n_request_idx]);
        int* p_workerId = (int*)malloc(sizeof(int));
        *p_workerId = (int)hServiceServer->requestfd[n_request_idx];
            
        lplp_details[n_request_idx] = eggThreadPool_taskdetails_init(1,
                                                                     p_workerId );
//       eggServiceServer_processing(lplp_details[n_request_idx]);
	pthread_attr_t threadattr;
	pthread_attr_init(&threadattr);
	pthread_attr_setdetachstate(&threadattr, PTHREAD_CREATE_DETACHED);

        n_thr_ret = pthread_create(&pthread_ids, &threadattr, eggServiceServer_processing,  lplp_details[n_request_idx]);

        if(n_thr_ret)
        {
            eggPrtLog_log_line(hPrtLog, EGGPRTLOG_ERROR, "eggServiceServer", "eggServiceServer_scheduleWorker pthread_create is error[%s] [%d] [%s]\n",__func__, __LINE__, strerror(errno));

            eggThreadPool_taskdetails_destroy(lplp_details[n_request_idx]);
            close(*p_workerId);
            free(p_workerId);
        }
        else
        {
//        sleep(1);
            eggServiceServer_add_workerId(g_service_server, pthread_ids);
        }
        n_request_idx++;
    }
    
    hServiceServer->requestCnt = 0;
    free(lplp_details);
    return EGG_TRUE;
}


PRIVATE EBOOL eggServiceServer_processing(HEGGTHREADTASKDETAILS workerInfo)
{
    signal(SIGPIPE, sigPipeHandler);
//    sleep(1);
    int* p_workerId = (int*)(workerInfo->details[0]);
    
    HEGGNETSERVER lp_server = eggNetServer_new (p_workerId, p_workerId, eggServiceServer_send, eggServiceServer_recv);
    int count = 0;
    while(1)
    {

        EBOOL ret = EGG_FALSE;
        HEGGNETPACKAGE lp_recv_package = eggNetPackage_new(0);
        HEGGNETPACKAGE lp_send_package = EGG_NULL;

         
        ret = eggNetServer_recv(lp_server, lp_recv_package, sizeof(EGGNETPACKAGE), 0);
        if(!ret)
        {

            eggNetPackage_delete(lp_recv_package);
               eggNetServer_destory_egg(lp_server);
            eggNetServer_delete (lp_server);
            eggThreadPool_taskdetails_destroy(workerInfo);
            close(*p_workerId);
            free(p_workerId);
            eggServiceServer_clean_workerId(g_service_server, pthread_self());
            return EGG_FALSE;
        }

        
        lp_recv_package = (HEGGNETPACKAGE)realloc(lp_recv_package, eggNetPackage_get_packagesize(lp_recv_package));

        ret = eggNetServer_recv(lp_server, lp_recv_package + 1,
                                eggNetPackage_get_packagesize(lp_recv_package) - sizeof(EGGNETPACKAGE), 0);
        
        if(!ret)
        {
            
            eggNetPackage_delete(lp_recv_package);
            eggNetServer_destory_egg(lp_server);
            eggNetServer_delete (lp_server);           
            close(*p_workerId);
            free(p_workerId);
            eggThreadPool_taskdetails_destroy(workerInfo);
            eggServiceServer_clean_workerId(g_service_server, pthread_self());
            return EGG_FALSE;
        }

        
        lp_send_package = eggNetServer_processing(lp_server, lp_recv_package);
        
        eggNetServer_send(lp_server, lp_send_package, eggNetPackage_get_packagesize(lp_send_package), 0);
         
        ret = *(EBOOL*)((HEGGNETUNITPACKAGE)(lp_send_package + 1) + 1);
        
        eggNetPackage_delete(lp_send_package);
        eggNetPackage_delete(lp_recv_package);
//        printf("*****************--------ret %d [%d] count %d\n", ret, pthread_self(), count++);
        
        /* if(!ret) */
        /* { */
        /*     eggNetServer_destory_egg(lp_server); */
        /*     eggNetServer_delete (lp_server); */
        /*     eggThreadPool_taskdetails_destroy(workerInfo); */
        /*     close(*p_workerId); */
        /*     free(p_workerId);             */
        /*     eggServiceServer_clean_workerId(g_service_server, pthread_self()); */
        /*     printf("thread over 4444444444444\n"); */

        /*     return EGG_FALSE; */
        /* } */
            
    }
}




PRIVATE EBOOL eggServiceServer_recv(HEGGNETSERVER hNetServer, epointer ePointer, size32_t size, int flags)
{
    if(POINTER_IS_INVALID(hNetServer))
    {
        return EGG_FALSE;
    }
    
    char* lp_str = (char*)ePointer;
    int fd = *(int*)(hNetServer->hOutStream);
    int n_rrecv_cnt = 3;
    while(size)
    {
       
        int n_recv = recv(fd, lp_str, size, 0);
        if(n_recv == -1)
        {
       

            
            return EGG_FALSE;
        }
        else if(n_recv == 0 )
        {

            return EGG_FALSE;
        }
        else
        {
            size -= n_recv;
            lp_str += n_recv;
        }
    }


    return EGG_TRUE;
}

PRIVATE EBOOL eggServiceServer_send(HEGGNETSERVER hNetServer, epointer ePointer, size32_t size, int flags)
{
    if(POINTER_IS_INVALID(hNetServer))
    {
        return EGG_FALSE;
    }
    
    char* lp_str = (char*)ePointer;
    int fd = *(int*)(hNetServer->hInStream);

    while(size)
    {
       
      int n_send = send(fd, lp_str, size, 0);
      if(n_send == -1)
      {


            return EGG_FALSE;
      }
      else if(n_send == 0 )
      {

          return EGG_FALSE;
      }
      else
      {
          size -= n_send;
          lp_str += n_send;
      }
    }


    return EGG_TRUE;
}
EBOOL eggServiceServer_clean_workerId(HEGGSERVICESERVER hServiceServer, pthread_t workerId)
{
    index_t n_pthread_idx = 0;
    
    while (n_pthread_idx < EGGWORKERSERVER_FDNUM)
    {
        if(workerId == hServiceServer->workerfd[n_pthread_idx])
        {
            hServiceServer->workerfd[n_pthread_idx] = 0;
            hServiceServer->workerCnt --;
    
            break;
        }
        n_pthread_idx++;
    }

    if(n_pthread_idx == EGGWORKERSERVER_FDNUM)
    {
        //exit(-1);
    }
    
    return EGG_TRUE;
    
}

EBOOL eggServiceServer_add_workerId(HEGGSERVICESERVER hServiceServer, pthread_t workerId)
{
    index_t n_pthread_idx = 0;
    
    while (n_pthread_idx < EGGWORKERSERVER_FDNUM)
    {
        if(0 == hServiceServer->workerfd[n_pthread_idx])
        {
            hServiceServer->workerfd[n_pthread_idx] = workerId;
            hServiceServer->workerCnt ++;
    
            break;
        }
        n_pthread_idx++;
    }
    
    
    if(n_pthread_idx == EGGWORKERSERVER_FDNUM)
    {
        exit(-1);
    }
    return EGG_TRUE;
    
}

EBOOL eggServiceServer_wait_worker(HEGGSERVICESERVER hServiceServer)
{
    index_t n_pthread_idx = 0;
    while (n_pthread_idx < EGGWORKERSERVER_FDNUM)
    {
        
        if(hServiceServer->workerfd[n_pthread_idx] == 0)
        {
        n_pthread_idx++;
            continue;
        }
        pthread_join(hServiceServer->workerfd[n_pthread_idx], 0);
        n_pthread_idx++;
    }
    return EGG_TRUE;
    
}

void eggServiceServer_exit(int sig)
{
    close(g_service_server->sockfd[0]);
    close(g_service_server->sockfd[1]);
    eggServiceServer_wait_worker(g_service_server);
    
    eggNetIndexList_optimize(g_eggNet_list);
    eggNetIndexList_delete(g_eggNet_list);
    //sleep(100);
    exit(0) ;
}

extern HEGGANALYZER g_hAnalyzer;




HEGGSERVICESERVERCONFIG readcommandline(HEGGSERVICESERVERCONFIG hEggServiceServerConfig,
                                        int argc,
                                        char *argv[])
{
    if (!hEggServiceServerConfig)
    {
        hEggServiceServerConfig = eggServiceServerConfig_new(NULL);
    }
    
    int i;
    for (i = 1; i < argc; )
    {
        if (strcmp(argv[i], "--unixsock") == 0)
        {
            free(hEggServiceServerConfig->socketfile);
            hEggServiceServerConfig->socketfile = strdup(argv[i+1]);
            assert(hEggServiceServerConfig->socketfile);
            i += 2;
        }
        else if (strcmp(argv[i], "--ip") == 0)
        {
            free(hEggServiceServerConfig->ip);
            hEggServiceServerConfig->ip = strdup(argv[i+1]);
            assert(hEggServiceServerConfig->ip);
            i += 2;
        }
        else if (strcmp(argv[i], "--port") == 0)
        {
            hEggServiceServerConfig->port = strtol(argv[i+1], NULL, 10);
            i += 2;            
        }
        else
        {
            fprintf(stderr, "WARN unrecognize command option: %s\n",
                    argv[i]);
            i++;
        }
    }
    return hEggServiceServerConfig;
}


int main(int argc ,char* argv[])
{
    
    int rc = 0;

    char *configfile = "/etc/egg3/eggd.cfg";
    g_eggServiceServerConfig = eggServiceServerConfig_new(configfile);

    g_eggServiceServerConfig = readcommandline(g_eggServiceServerConfig, argc, argv);
    
    if(! g_thread_supported()) g_thread_init(NULL);
//       signal(SIGNETEXIT, eggServiceServer_exit);
    //  signal(SIGINT, eggServiceServer_exit);
//    signal(SIGSTOP, eggServiceServer_exit);
    
    g_service_server = eggServiceServer_new(g_eggServiceServerConfig);
    
    g_eggNet_list = eggNetIndexList_new();
    int count = 0;
    while(eggServiceServer_get_request(g_service_server) )
    {
        eggServiceServer_scheduleWorker(g_service_server);
        count++;
    }
    eggServiceServer_wait_worker(g_service_server);    
    eggServiceServer_delete(g_service_server);
    eggNetIndexList_optimize(g_eggNet_list);
    
    eggServiceServerConfig_delete(g_eggServiceServerConfig);
    return 0;
}

