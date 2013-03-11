#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>

#include <sys/types.h>
#include <egg3/Egg3.h>
#include "eggServiceServerConfig.h"
#include <egg3/net/eggNetType.h>
#include <stdio.h>
#include <assert.h>
#include <sys/select.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>
#include <glib.h>

struct eggMemWorker;
typedef struct eggMemWorker EGGMEMWORKER;
typedef struct eggMemWorker* HEGGMEMWORKER;

#define EGGMEMSERVER_LISTENNUM 50
#define EGGMEMSERVER_FDNUM 2
#define EGGWORKERSERVER_FDNUM 1024

const int EGGMEMIO_IN = 1;
const int EGGMEMIO_OUT = 2;
const int EGGMEMIO_EXCEPT = 4;

typedef struct eggMemServer EGGMEMSERVER;
typedef struct eggMemServer* HEGGMEMSERVER;

struct eggMemServer
{
    int sockfd[EGGMEMSERVER_FDNUM];    //listen fd. 0 is unix; 1 is inet;
    
    HEGGMEMWORKER worker[EGGWORKERSERVER_FDNUM];
    int requestCnt;
    
};

HEGGMEMSERVER g_mem_server = EGG_NULL;
static int config_forceOverride = 0;
HEGGSERVICESERVERCONFIG g_eggMemServerConfig = EGG_NULL; /* config format is the same as eggServiceServer.cfg */

typedef struct eggMemEggHandle EGGMEMEGGHANDLE;
typedef struct eggMemEggHandle* HEGGMEMEGGHANDLE;

struct eggMemEggHandle
{
    char *eggPath;
    char *analyzerName;
    HEGGHANDLE hEggHandle;
    HEGGINDEXSEARCHER hSearcher;
    HEGGINDEXREADER hReader;
    HEGGINDEXWRITER hWriter;
};
HEGGMEMEGGHANDLE g_hEggMemEggHandle;
EBOOL eggMemEggHandle_init(HEGGMEMEGGHANDLE hEggMemEggHandle, char *eggPath, char *analyzerName);
EBOOL eggMemEggHandle_release(HEGGMEMEGGHANDLE hEggMemEggHandle);

    
PRIVATE HEGGNETPACKAGE eggMemServer_processing(HEGGNETPACKAGE hNetPackage);


    
struct eggMemWorker
{
    int id;


    
    int fd;
    int fdflag;
    
    int status;
    
    char *buff_in;
    int buff_in_count;
    int buff_in_size;
    int buff_in_needtofill;

    char **pool_out;
    int *pool_out_buffcount;
    int *pool_out_buffsize;
    int pool_out_count;
    int pool_out_size;
    
};
enum {EGGMEMWORKER_STAT_ERROR = -2, EGGMEMWORKER_STAT_ERRORIO = -1,
      EGGMEMWORKER_STAT_READY = 0,
      EGGMEMWORKER_STAT_HADSIZE, EGGMEMWORKER_STAT_PROCESSING, EGGMEMWORKER_STAT_CLOSED};
#define eggMemServer_worker_isError(hWorker) ((hWorker)->status == EGGMEMWORKER_STAT_ERROR || (hWorker)->status == EGGMEMWORKER_STAT_ERRORIO)
#define eggMemServer_worker_isEnd(hWorker) ((hWorker)->status == EGGMEMWORKER_STAT_ERROR || (hWorker)->status == EGGMEMWORKER_STAT_ERRORIO || (hWorker)->status == EGGMEMWORKER_STAT_CLOSED)
HEGGMEMWORKER eggMemServer_worker_new(int fd);
int eggMemServer_worker_delete(HEGGMEMWORKER hWorker);
int eggMemServer_worker_in(HEGGMEMWORKER hWorker);
int eggMemServer_worker_out(HEGGMEMWORKER hWorker);
int eggMemServer_worker_except(HEGGMEMWORKER hWorker);


PRIVATE HEGGNETPACKAGE eggMemServer_processing_loadEgg(HEGGNETPACKAGE hNetPackage);
PRIVATE HEGGNETPACKAGE eggMemServer_processing_optimize(HEGGNETPACKAGE hNetPackage);

int id= 0;
HEGGMEMWORKER eggMemServer_worker_new(int fd)
{
    assert(fd > 0);
    
    HEGGMEMWORKER hWorker = calloc(1, sizeof(*hWorker));
    hWorker->fd = fd;

    hWorker->id = ++id;
    return hWorker;
}
int eggMemServer_worker_delete(HEGGMEMWORKER hWorker)
{
    if (!hWorker)
    {
        return 0;
    }
    
    assert(hWorker->fd);
    
    close(hWorker->fd);
    free(hWorker->buff_in);
    int i;
    for (i = 0; i < hWorker->pool_out_count; i++)
    {
        free(hWorker->pool_out[i]);
    }
    free(hWorker->pool_out);
    free(hWorker->pool_out_buffcount);
    free(hWorker->pool_out_buffsize);    
    free(hWorker);
    return 0;
}

int eggMemServer_worker_fillbuffin(HEGGMEMWORKER hWorker)
{
    if (!hWorker)
    {
        return 0;
    }
    if (hWorker->status == EGGMEMWORKER_STAT_ERROR
        || hWorker->status == EGGMEMWORKER_STAT_ERRORIO)
    {
        return -1;
    }
    int fd = hWorker->fd;

    char *buf = hWorker->buff_in + hWorker->buff_in_count;
    int n = hWorker->buff_in_size - hWorker->buff_in_count;
    n = n > hWorker->buff_in_needtofill ? hWorker->buff_in_needtofill : n;
    int nn;
    if ((nn = read(fd, buf, n)) < 0)
    {
        if (errno != EAGAIN && errno != EWOULDBLOCK)
        {
            fprintf(stderr, "ERR read fd[%d]: %s\n", fd, strerror(errno));
            hWorker->status = EGGMEMWORKER_STAT_ERRORIO;
        }
    }
    else if (nn == 0)
    {
        hWorker->status = EGGMEMWORKER_STAT_CLOSED;
    }
    else
    {
        hWorker->buff_in_count += nn;
        hWorker->buff_in_needtofill -= nn;
    }
    
    return 0;
    
}
int eggMemServer_worker_fillbuffout(HEGGMEMWORKER hWorker, char *buf, int bufsz)
{
    if (!hWorker)
    {
        return 0;
    }
    if (hWorker->pool_out == NULL
        || hWorker->pool_out_size == 0)
    {
        hWorker->pool_out_count = 0;
        hWorker->pool_out_size = 100;
        hWorker->pool_out = (char **)malloc(hWorker->pool_out_size * sizeof(char *));
        assert(hWorker->pool_out);
        hWorker->pool_out_buffcount = (int *)malloc(hWorker->pool_out_size * sizeof(int));
        assert(hWorker->pool_out_buffcount);
        hWorker->pool_out_buffsize = (int *)malloc(hWorker->pool_out_size * sizeof(int));
        assert(hWorker->pool_out_buffsize);
    }

    if (hWorker->pool_out_count == hWorker->pool_out_size)
    {
        hWorker->pool_out_size += 100;
        hWorker->pool_out = (char **)realloc(hWorker->pool_out,
                                             hWorker->pool_out_size * sizeof(char *));
        assert(hWorker->pool_out);
        hWorker->pool_out_buffcount = (int *)realloc(hWorker->pool_out_buffcount,
                                                     hWorker->pool_out_size * sizeof(int));
        assert(hWorker->pool_out_buffcount);
        hWorker->pool_out_buffsize = (int *)realloc(hWorker->pool_out_buffsize,
                                                    hWorker->pool_out_size * sizeof(int));
        assert(hWorker->pool_out_buffsize);
        
    }
    hWorker->pool_out[hWorker->pool_out_count] = buf;
    hWorker->pool_out_buffcount[hWorker->pool_out_count] = 0;
    hWorker->pool_out_buffsize[hWorker->pool_out_count] = bufsz;
    hWorker->pool_out_count++;    
    return 0;
    
}
int eggMemServer_worker_in(HEGGMEMWORKER hWorker)
{
    int ret = 0;
    int fd = hWorker->fd;

    if (hWorker->status == EGGMEMWORKER_STAT_READY)
    {
        if (hWorker->buff_in == NULL
            || hWorker->buff_in_size == 0)
        {
            HEGGNETPACKAGE hNetPackage = eggNetPackage_new(0);
            hWorker->buff_in = (char*)hNetPackage;
            hWorker->buff_in_count = 0;
            hWorker->buff_in_size = sizeof(EGGNETPACKAGE);
            hWorker->buff_in_needtofill = sizeof(EGGNETPACKAGE);
        }
        eggMemServer_worker_fillbuffin(hWorker);
        if (hWorker->buff_in_needtofill == 0)
        {
            HEGGNETPACKAGE lp_recv_package = (HEGGNETPACKAGE)hWorker->buff_in;
            hWorker->buff_in_size = eggNetPackage_get_packagesize(lp_recv_package);
            hWorker->buff_in = realloc(hWorker->buff_in, hWorker->buff_in_size);
            assert(hWorker->buff_in);
            hWorker->buff_in_needtofill = hWorker->buff_in_size - sizeof(EGGNETPACKAGE);
            hWorker->status = EGGMEMWORKER_STAT_HADSIZE;
        }
        
    }
    
    if (hWorker->status == EGGMEMWORKER_STAT_HADSIZE)
    {
        eggMemServer_worker_fillbuffin(hWorker);
        if (hWorker->buff_in_needtofill == 0)
        {
            hWorker->status = EGGMEMWORKER_STAT_PROCESSING;
            
            HEGGNETPACKAGE lp_send_package;
            lp_send_package = eggMemServer_processing((HEGGNETPACKAGE)hWorker->buff_in);
            eggMemServer_worker_fillbuffout(hWorker, (char*)lp_send_package,
                                            eggNetPackage_get_packagesize(lp_send_package));
            hWorker->fdflag |= EGGMEMIO_OUT;
            eggNetPackage_delete((HEGGNETPACKAGE)hWorker->buff_in);
            hWorker->buff_in = NULL;
            hWorker->buff_in_size = 0;
            hWorker->buff_in_count = 0;
            hWorker->buff_in_needtofill = 0;
            hWorker->status = EGGMEMWORKER_STAT_READY;
            
        }
    }
    return 0;
}
int eggMemServer_worker_out(HEGGMEMWORKER hWorker)
{
    if (!hWorker)
    {
        return 0;
    }

    if (hWorker->status == EGGMEMWORKER_STAT_ERRORIO)
    {
        return -1;
    }
    
    if (hWorker->pool_out == NULL
        || hWorker->pool_out_size == 0
        || hWorker->pool_out_count <= 0)
    {
        return 0;
    }

    int fd = hWorker->fd;
    int nn = 1;
    char *buf;
    int *p_count;
    int *p_size;
    
    while (nn > 0 && hWorker->pool_out_count > 0)
    {
        
        buf = hWorker->pool_out[0];
        p_count = &hWorker->pool_out_buffcount[0];
        p_size = &hWorker->pool_out_buffsize[0];

        if (*p_count < *p_size)
        {
            if ((nn = write(fd, buf + *p_count, *p_size - *p_count)) < 0)
            {
                if (errno != EAGAIN && errno != EWOULDBLOCK)
                {
                    fprintf(stderr, "ERR write fd[%d]: %s\n", fd, strerror(errno));
                    hWorker->status = EGGMEMWORKER_STAT_ERRORIO;
                }
            }
            else
            {
                *p_count += nn;
            }
            
        }
        if (*p_count >= *p_size)
        {
            free(buf);
            memmove(&hWorker->pool_out[0],
                    &hWorker->pool_out[1],
                    sizeof(hWorker->pool_out[0]) * (hWorker->pool_out_count - 1));
            memmove(&hWorker->pool_out_buffcount[0],
                    &hWorker->pool_out_buffcount[1],
                    sizeof(hWorker->pool_out_buffcount[0]) * (hWorker->pool_out_count - 1));
            memmove(&hWorker->pool_out_buffsize[0],
                    &hWorker->pool_out_buffsize[1],
                    sizeof(hWorker->pool_out_buffsize[0]) * (hWorker->pool_out_count - 1));
            hWorker->pool_out_count--;
        }
    }
    
    if (hWorker->pool_out_count == 0)
    {
        hWorker->fdflag &= ~EGGMEMIO_OUT;
    }
    
    return 0;
}
int eggMemServer_worker_except(HEGGMEMWORKER hWorker)
{
    int fd = hWorker->fd;
    fprintf(stderr, "ERR fd[%d]: %s\n", fd, strerror(errno));
    hWorker->status = EGGMEMWORKER_STAT_ERROR;
    return 0;
}



void sigPipeHandler(int signo)
{
    (void)signo;
    fprintf(stderr, "SIGPIPE signal !!!\n");
    return ;
}

HEGGMEMSERVER eggMemServer_new(HEGGSERVICESERVERCONFIG p_eggConfig)
{
    if(POINTER_IS_INVALID(p_eggConfig))
    {
        return EGG_NULL;
    }

    int inetfd = -1, unixfd = -1;
	

    if(p_eggConfig->socketfile)
    {
        unlink (p_eggConfig->socketfile);   
        struct sockaddr_un addr;

        unixfd = socket(AF_UNIX, SOCK_STREAM, 0);
        
        if (unixfd == -1)
        {
            perror("socket");
            exit(EXIT_FAILURE);
        }
        
        memset(&addr, 0, sizeof(struct sockaddr_un));
                                 /* Clear structure */
        addr.sun_family = AF_UNIX;
        
        strncpy(addr.sun_path, p_eggConfig->socketfile,
                sizeof(addr.sun_path) - 1);

        fcntl(inetfd, F_SETFL, O_NONBLOCK);
        
        if (bind(unixfd, (struct sockaddr *) &addr,
                 sizeof(struct sockaddr_un)) == -1)
        {
            perror("bind");
            exit(EXIT_FAILURE);
        }
        
        listen(unixfd, EGGMEMSERVER_LISTENNUM);
    }
    
    if(p_eggConfig->ip)
    {
        struct sockaddr_in servaddr;

        inetfd = socket(AF_INET, SOCK_STREAM, 0);
        if (inetfd == -1)
        {
            perror("socket");
            exit(EXIT_FAILURE);
        }
        
        bzero(&servaddr, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        
        inet_pton(AF_INET, p_eggConfig->ip, (void *)(&(servaddr.sin_addr.s_addr)));
        
        servaddr.sin_port = htons(p_eggConfig->port);

        int sock_reuse=1; 
        
        if(setsockopt(inetfd, SOL_SOCKET,SO_REUSEADDR,(char *)&sock_reuse,sizeof(sock_reuse)) != 0)
        {
            printf("套接字可重用设置失败!/n");
            perror("setsockopt");
            exit(1);
        }
        fcntl(inetfd, F_SETFL, O_NONBLOCK);

        
        if( -1 == bind(inetfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) )
        {
            perror("bind");
            exit(EXIT_FAILURE);
        }
        
        listen(inetfd, EGGMEMSERVER_LISTENNUM);
        
    }

    HEGGMEMSERVER lp_mem_server = (HEGGMEMSERVER)malloc(sizeof(EGGMEMSERVER));
    memset(lp_mem_server, 0, sizeof(EGGMEMSERVER));
    
    lp_mem_server->sockfd[0] = unixfd;
    lp_mem_server->sockfd[1] = inetfd;

    return lp_mem_server;
}


EBOOL eggMemServer_delete(HEGGMEMSERVER pMemServer)
{
    index_t idx = 0;
    while( idx < EGGMEMSERVER_FDNUM)
    {
        close(pMemServer->sockfd[idx]);
        idx ++;
    }
    free(pMemServer);
    return EGG_TRUE;
}

EBOOL eggMemServer_get_request(HEGGMEMSERVER hServer)
{
    if(POINTER_IS_INVALID(hServer))
    {
        return EGG_FALSE;
    }
    int idx = 0;
    int rtidx = 0;
    int maxfd = -1;
    fd_set readfds, writefds, exceptfds;
    FD_ZERO(&readfds);
    FD_ZERO(&writefds);
    FD_ZERO(&exceptfds);
    
    for(idx = 0; idx < EGGMEMSERVER_FDNUM; idx++)
    {
        if (hServer->sockfd[idx] > 0)
        {
            maxfd = maxfd > hServer->sockfd[idx] ? maxfd : hServer->sockfd[idx];
            FD_SET(hServer->sockfd[idx], &readfds);
            FD_SET(hServer->sockfd[idx], &exceptfds);
        }
    }
    for (idx = 0; idx < hServer->requestCnt; idx++)
    {
        int fd;
        int fdflag;
        fd = hServer->worker[idx]->fd;
        fdflag = hServer->worker[idx]->fdflag;
        maxfd = maxfd > fd ? maxfd : fd;
        if (fdflag & EGGMEMIO_IN)
        {
            FD_SET(fd, &readfds);
        }
        if (fdflag & EGGMEMIO_OUT)
        {
            FD_SET(fd, &writefds);
        }
        if (fdflag & EGGMEMIO_EXCEPT)
        {
            FD_SET(fd, &exceptfds);
        }
    }

    switch (select(maxfd + 1, &readfds, &writefds, &exceptfds, EGG_NULL))
    {
    case -1:
        fprintf(stderr, "select error!\n");
        perror("select");
        exit(-1);
        
    case 0:
        fprintf(stdout, "select timeout!\n");
        break; //再次轮询 
        
    default:

        /* check listen fd */
        for(idx = 0; idx < EGGMEMSERVER_FDNUM; idx++)
        {
            if(hServer->sockfd[idx] > 0 && FD_ISSET(hServer->sockfd[idx], &readfds))
            {
                int requestfd = -1;
                if ((requestfd = accept(hServer->sockfd[idx], EGG_NULL, EGG_NULL)) < 0)
                {
                    fprintf(stderr, "ERR accept: %s", strerror(errno));
                    continue;
                }
                if (fcntl(requestfd, F_SETFL, O_NONBLOCK) < 0)
                {
                    fprintf(stderr, "ERR set fd[%d] O_NONBLOCK: %s", requestfd, strerror(errno));
                    close(requestfd);
                    continue;
                }
                if (hServer->requestCnt == EGGWORKERSERVER_FDNUM)
                {
                    fprintf(stderr, "ERR client connect > MAX[%d]", EGGWORKERSERVER_FDNUM);
                    close(requestfd);
                    continue;
                }
                hServer->worker[hServer->requestCnt] = eggMemServer_worker_new(requestfd);
                hServer->worker[hServer->requestCnt]->fdflag = EGGMEMIO_IN | EGGMEMIO_EXCEPT;
                hServer->requestCnt++;
            }
        }

        /* check data fd */
        for(idx = 0; idx < hServer->requestCnt; idx++)
        {
            int fd;
            fd = hServer->worker[idx]->fd;
            if(FD_ISSET(fd, &readfds))
            {
                eggMemServer_worker_in(hServer->worker[idx]);
            }
            if(FD_ISSET(fd, &writefds))
            {
                eggMemServer_worker_out(hServer->worker[idx]);
            }
            if(FD_ISSET(fd, &exceptfds))
            {
                eggMemServer_worker_except(hServer->worker[idx]);
            }
            if(eggMemServer_worker_isEnd(hServer->worker[idx]))
            {
                eggMemServer_worker_delete(hServer->worker[idx]);
                memmove(&hServer->worker[idx],
                        &hServer->worker[idx+1],
                        sizeof(hServer->worker[idx]) * (hServer->requestCnt - idx - 1));
                hServer->requestCnt--;
            }
            
        }
    }
    return EGG_TRUE;
    
}


PRIVATE HEGGNETPACKAGE eggMemServer_processing(HEGGNETPACKAGE hNetPackage)
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

    HEGGNETSERVER hNetServer = NULL;
    switch(hNetPackage->op)
    {
        
    case EGG_PACKAGE_LOADEGG:
        lp_res_package = eggMemServer_processing_loadEgg(hNetPackage);
        return lp_res_package;
        
    case EGG_PACKAGE_OPTIMIZE:
        lp_res_package = eggMemServer_processing_optimize(hNetPackage);
        return lp_res_package;
    default:
        hNetServer = eggNetServer_new(NULL, NULL, NULL, NULL);
        hNetServer->hSearcher = g_hEggMemEggHandle->hSearcher;
        hNetServer->hReader = g_hEggMemEggHandle->hReader;
        hNetServer->hWriter = g_hEggMemEggHandle->hWriter;
        lp_res_package = eggNetServer_processing(hNetServer, hNetPackage);
        eggNetServer_delete(hNetServer);
        break;
    }
    

    return lp_res_package;
}

extern HEGGANALYZER g_hAnalyzer;


EBOOL eggMemEggHandle_init(HEGGMEMEGGHANDLE hEggMemEggHandle, char *eggPath, char *analyzerName)
{
    if (!hEggMemEggHandle)
    {
        return EGG_FALSE;
    }
    char *eggPath_extr;
    int n;
    n = strlen(eggPath);
    if (eggPath[n-1] != '/')
    {
        eggPath_extr = malloc(n + 2);
        assert(eggPath_extr);
        sprintf(eggPath_extr, "%s/", eggPath);
    }
    else
    {
        eggPath_extr = strdup(eggPath);
        assert(eggPath_extr);
    }
    hEggMemEggHandle->eggPath = eggPath_extr;
    assert(hEggMemEggHandle->eggPath);
    hEggMemEggHandle->analyzerName = strdup(analyzerName);
    assert(hEggMemEggHandle->analyzerName);    
    hEggMemEggHandle->hEggHandle = (HEGGHANDLE)eggDirectory_open(eggPath);
    hEggMemEggHandle->hWriter = eggIndexWriter_open(hEggMemEggHandle->hEggHandle, analyzerName);
    hEggMemEggHandle->hReader = eggIndexWriter_init_reader(hEggMemEggHandle->hWriter);
    hEggMemEggHandle->hSearcher = eggIndexSearcher_new(hEggMemEggHandle->hReader);
    
    if (hEggMemEggHandle->hEggHandle && hEggMemEggHandle->hWriter
        && hEggMemEggHandle->hReader && hEggMemEggHandle->hSearcher)
    {
        return EGG_TRUE;
    }
    else
    {
        return EGG_FALSE;
    }
}
EBOOL eggMemEggHandle_release(HEGGMEMEGGHANDLE hEggMemEggHandle)
{
    if (!hEggMemEggHandle)
    {
        return EGG_FALSE;
    }
    eggIndexSearcher_delete(hEggMemEggHandle->hSearcher);
    eggIndexReader_close(hEggMemEggHandle->hReader);
    eggIndexWriter_close(hEggMemEggHandle->hWriter);
    eggPath_close(hEggMemEggHandle->hEggHandle);
    free(hEggMemEggHandle->analyzerName);
    free(hEggMemEggHandle->eggPath);
    memset(hEggMemEggHandle, 0, sizeof(*hEggMemEggHandle));
    return EGG_TRUE;
}

int usage(char *prg)
{
    fprintf(stderr,
            "%-20s [--force] --unixsock sockfile /egg/path/ analyzer_name\n"
            "%-20s [--force] --ip 10.0.0.1 --port 10 /egg/path/ analyzer_name\n"
            "\n"
            "%-20s  --force means kill exited process\n",
            prg, prg, "");
    exit(0);
}

HEGGSERVICESERVERCONFIG readcommandline(HEGGSERVICESERVERCONFIG hEggServiceServerConfig,
                                        int *argcp,
                                        char *argv[])
{
    if (!hEggServiceServerConfig)
    {
        hEggServiceServerConfig = eggServiceServerConfig_new(NULL);
    }

    int argc = *argcp;
    int argc_copy;
    char **argv_copy;
    argc_copy = 1;
    argv_copy = (char **)calloc(argc, sizeof(char *));
    assert(argv_copy);
    argv_copy[0] = argv[0];
    
    int i, j;
    
    for (j = 1, i = 1; i < argc; )
    {
        if (strcmp(argv[i], "--unixsock") == 0)
        {
            if (i == argc - 1)
            {
                fprintf(stderr, "ERR --unixsock need a argument\n");
                exit(-1);
            }
            free(hEggServiceServerConfig->socketfile);
            hEggServiceServerConfig->socketfile = strdup(argv[i+1]);
            assert(hEggServiceServerConfig->socketfile);

            i += 2;
        }
        else if (strcmp(argv[i], "--ip") == 0)
        {
            if (i == argc - 1)
            {
                fprintf(stderr, "ERR --ip need a argument\n");
                exit(-1);
            }
            
            free(hEggServiceServerConfig->ip);
            hEggServiceServerConfig->ip = strdup(argv[i+1]);
            assert(hEggServiceServerConfig->ip);
            
            i += 2;
        }
        else if (strcmp(argv[i], "--port") == 0)
        {
            if (i == argc - 1)
            {
                fprintf(stderr, "ERR --port need a argument\n");
                exit(-1);
            }
            
            hEggServiceServerConfig->port = strtol(argv[i+1], NULL, 10);

            i += 2;
        }
        else if (strcmp(argv[i], "--force") == 0)
        {
            config_forceOverride = 1;
            i += 1;
        }
        else if (strcmp(argv[i], "--help") == 0)
        {
            usage(argv[0]);
            i += 1;
        }

        else if (strncmp(argv[i], "-", 1) == 0)
        {
            fprintf(stderr, "WARN unknown option %s\n", argv[i]);
            
            i += 1;
        }
        else
        {
            argc_copy++;
            argv_copy[j] = argv[i];
            j++;
            
            i++;
        }
    }

    *argcp = argc_copy;
    memcpy(argv, argv_copy, (argc_copy+1) * sizeof(argv[0]));
    free(argv_copy);
    return hEggServiceServerConfig;
}

static int checkSingleton(char *workdir)
{
    int n;
    char *pidfile;
    
    n = strlen(workdir);
    pidfile = malloc(n + 10);
    assert(pidfile);
    sprintf(pidfile, "%s/.pid", workdir);
    int fd = open(pidfile, O_RDWR|O_CREAT, 0644);
    if (fd < 0)
    {
        fprintf(stderr, "%s:%d:%s ERR open[%s] %s\n",
                __FILE__, __LINE__, __func__, pidfile, strerror(errno));
        exit(-1);
    }
    {
        long flag;
        if ((flag = fcntl(fd, F_GETFD)) < 0)
        {
            fprintf(stderr, "%s:%d:%s ERR fcntl([%s], GETFD) %s\n",
                    __FILE__, __LINE__, __func__, pidfile, strerror(errno));
            exit(-1);
        }
        flag |= FD_CLOEXEC;
        if (fcntl(fd, F_SETFD, flag) < 0)
        {
            fprintf(stderr, "%s:%d:%s ERR fcntl([%s], SETFD) %s\n",
                    __FILE__, __LINE__, __func__, pidfile, strerror(errno));
            exit(-1);
        }
    }
    
    struct flock flck;
    flck.l_type = F_WRLCK;
    flck.l_whence = SEEK_SET;
    flck.l_start = 0;
    flck.l_len = 0;
    char buf[30] = "";
retry:    
    if (fcntl(fd, F_SETLK, &flck) < 0)
    {
        lseek(fd, 0, SEEK_SET);
        read(fd, buf, sizeof(buf));
        if (!config_forceOverride)
        {
            fprintf(stderr, "already run pid %s\n", buf);
            exit(-1);
        }
        pid_t pid;
        pid = atoi(buf);
        if (kill(pid, SIGKILL) < 0)
        {
            fprintf(stderr, "already run pid %d, CANNOT kill it\n", (int)pid);
            exit(-1);
        }
        waitpid(pid, NULL, 0);
        goto retry;
    }
    
    snprintf(buf, sizeof(buf), "%d ", (int)getpid());
    lseek(fd, 0, SEEK_SET);
    write(fd, buf, strlen(buf));
    /* DONOT close fd */
    free(pidfile);
    return 0;
}

int main(int argc ,char* argv[])
{

    signal(SIGPIPE, sigPipeHandler);
    
    g_eggMemServerConfig = readcommandline(NULL, &argc, argv);

    char *eggPath;
    char *analyzerName;
    eggPath = realpath(argv[1], NULL);
    analyzerName = argv[2];
    
    checkSingleton(eggPath);
    
    g_mem_server = eggMemServer_new(g_eggMemServerConfig);

    g_hEggMemEggHandle = calloc(1, sizeof(*g_hEggMemEggHandle));
    assert(g_hEggMemEggHandle);
    if (argc < 3)
    {
        fprintf(stderr, "Usage: %s eggPath analyzerName\n", argv[0]);
        exit(-1);
    }

    if (EGG_FALSE == eggMemEggHandle_init(g_hEggMemEggHandle, eggPath, analyzerName))
    {
        exit(-2);
    }

    while (eggMemServer_get_request(g_mem_server))
    {
        ;
    }

    eggMemEggHandle_release(g_hEggMemEggHandle);
    free(g_hEggMemEggHandle);
    g_hEggMemEggHandle = NULL;
    eggMemServer_delete(g_mem_server);
    eggServiceServerConfig_delete(g_eggMemServerConfig);


    return 0;
}


PRIVATE HEGGNETPACKAGE eggMemServer_processing_loadEgg(HEGGNETPACKAGE hNetPackage)
{
    HEGGNETPACKAGE lp_res_package = eggNetPackage_new(EGG_PACKAGE_LOADEGG);
    EBOOL ret = EGG_TRUE;
    if(POINTER_IS_INVALID(hNetPackage))
    {
        ret = EGG_NET_IVDHANDLE;
        lp_res_package = eggNetPackage_add(lp_res_package, &ret, sizeof(ret), EGG_PACKAGE_RET);
        return lp_res_package;
    }
    int n_path_len = 0;
    char* lp_path_sign = EGG_NULL;
    /* 忽略,客户端通过端口连接后,就认为eggpath正确 */
    /* eggNetPackage_fetch(hNetPackage, 2, &lp_path_sign, &n_path_len); */
    
    /* if (strcmp(lp_path_sign, g_hEggMemEggHandle->eggPath) != 0) */
    /* { */
    /*     ret = EGG_FALSE; */
    /* } */
    
    lp_res_package = eggNetPackage_add(lp_res_package, &ret, sizeof(ret), EGG_PACKAGE_RET);
    
    return lp_res_package;

}


PRIVATE HEGGNETPACKAGE eggMemServer_processing_optimize(HEGGNETPACKAGE hNetPackage)
{
    HEGGNETPACKAGE lp_res_package = eggNetPackage_new(EGG_PACKAGE_OPTIMIZE);
    EBOOL ret;
    
    if(POINTER_IS_INVALID(hNetPackage))
    {
        ret = EGG_NET_IVDHANDLE;
        lp_res_package = eggNetPackage_add(lp_res_package, &ret, sizeof(ret), EGG_PACKAGE_RET);
        return lp_res_package;
    }
 
    
    size32_t n_iter_sz = 0;
    char* lp_data_str = (char*)(hNetPackage + 1);
    struct timeval tvstart,tvend;
    gettimeofday(&tvstart, 0);
    count_t a_cnt = 0;
    
    
    while(n_iter_sz != hNetPackage->eSize)
    {
        HEGGNETUNITPACKAGE lp_unit_package = (HEGGNETUNITPACKAGE)(lp_data_str + n_iter_sz);
        if(lp_unit_package->ty == EGG_PACKAGE_OPTIMIZE_ADD)
        {
            HEGGDOCNODE lp_doc_node = (HEGGDOCNODE)malloc(lp_unit_package->size);
            memcpy(lp_doc_node, lp_unit_package + 1, lp_unit_package->size);
            HEGGDOCUMENT lp_document = eggDocument_unserialization(lp_doc_node);
            
            eggIndexWriter_add_document(g_hEggMemEggHandle->hWriter, lp_document);

            eggDocument_delete(lp_document);
	    // printf("count doc : %d\n", ++a_cnt);
        }
        else if(lp_unit_package->ty == EGG_PACKAGE_OPTIMIZE_DELETE)
        {
            EGGDID dId = {0};
            memcpy(&dId, lp_unit_package + 1, lp_unit_package->size);
            eggIndexWriter_delete_document(g_hEggMemEggHandle->hWriter, dId );
            
        }
        else if(lp_unit_package->ty == EGG_PACKAGE_OPTIMIZE_MODIFY)   //modify
        {
            EGGDID dId = {0};
            memcpy(&dId, lp_unit_package + 1, sizeof(dId));

            HEGGDOCNODE lp_doc_node = (HEGGDOCNODE)malloc(lp_unit_package->size - sizeof(dId));
            memcpy(lp_doc_node, (char*)(lp_unit_package + 1) + sizeof(dId), lp_unit_package->size - sizeof(dId));
            HEGGDOCUMENT lp_document = eggDocument_unserialization(lp_doc_node);
            
            eggIndexWriter_modify_document(g_hEggMemEggHandle->hWriter, dId, lp_document);
            
            //eggDocument_delete(lp_document);
        }
        else
        {
            EGGDID dId = {0};
            memcpy(&dId, lp_unit_package + 1, sizeof(dId));
            
            HEGGDOCNODE lp_doc_node = (HEGGDOCNODE)malloc(lp_unit_package->size - sizeof(dId));
            memcpy(lp_doc_node, (char*)(lp_unit_package + 1) + sizeof(dId), lp_unit_package->size - sizeof(dId));
            HEGGDOCUMENT lp_document = eggDocument_unserialization(lp_doc_node);
            
            eggIndexWriter_incrementmodify_document(g_hEggMemEggHandle->hWriter, dId, lp_document);
            
//            eggDocument_delete(lp_document);
        }
        
        n_iter_sz += sizeof(EGGNETUNITPACKAGE) + lp_unit_package->size;
    }
    gettimeofday(&tvend, 0);

    fprintf(stderr, "ADD doc time %f\n" , (tvend.tv_sec - tvstart.tv_sec) + (double)(tvend.tv_usec - tvstart.tv_usec)/1000000 );
    
    /* gettimeofday(&tvstart, 0); */
    /* ret = eggIndexWriter_optimize(hNetServer->hWriter); */
    /* gettimeofday(&tvend, 0); */

    /* fprintf(stderr, "optimize doc time %f\n", (tvend.tv_sec - tvstart.tv_sec) + (double)(tvend.tv_usec - tvstart.tv_usec)/1000000 ); */
    ret = EGG_TRUE;

    
    lp_res_package = eggNetPackage_add(lp_res_package, &ret, sizeof(ret), EGG_PACKAGE_RET);

    return lp_res_package;
}

