#include <egg3/net/eggSpanUnit.h>
#include <egg3/conf/eggConfig.h>
#include <egg3/log/eggPrtLog.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <stdint.h>
#include <ctype.h>


typedef struct EggSpanUnitListElement EggSpanUnitListElement;
typedef struct EggSpanUnitListElement *HEGGSPANUNITLISTELEMENT;
struct EggSpanUnitListElement{
    eggSpanUnit eggSpanUnit;
    HEGGSPANUNITLISTELEMENT next[3];
    HEGGSPANUNITLISTELEMENT prev[3];
};
HEGGSPANUNITLISTELEMENT allEggSpanUnit;
HEGGSPANUNITLISTELEMENT allHostAddress;
HEGGSPANUNITLISTELEMENT allEggDirPath;
HEGGSPANUNIT registerEggSpanUnit(char *eggdirpath, char *hostaddress,
                                 SPANRANGE range);

typedef struct {
    char *infoFile;    
    char *ip;
    short port;
    char *logpath;
    int loglevel;
} eggInfoServerConfig;
int parseCmd(eggInfoServerConfig *pConf, int *argc, char *argv[]);
int readInfoServFile(eggInfoServerConfig *pConf);

typedef struct ProcUnit ProcUnit;
typedef struct ScheUnit ScheUnit;
typedef struct ProcUNIT ProcUNIT;
typedef struct Scheduler Scheduler;
struct ScheUnit {
    ProcUnit *proc;
    long rank;
};

#define FDIN 0x01
#define FDOUT 0x02
#define FDEXCEPT 0x04
#define FDFLAG_IS_IN(flag) (flag & FDIN)
#define FDFLAG_SET_IN(flag) (flag = (flag & ~FDIN) | FDIN)
#define FDFLAG_CLR_IN(flag) (flag = (flag & ~FDIN))
#define FDFLAG_IS_OUT(flag) (flag & FDOUT)
#define FDFLAG_SET_OUT(flag) (flag = (flag & ~FDOUT) | FDOUT)
#define FDFLAG_CLR_OUT(flag) (flag = (flag & ~FDOUT))
#define FDFLAG_IS_EXCEPT(flag) (flag & FDEXCEPT)
#define FDFLAG_SET_EXCEPT(flag) (flag = (flag & ~FDEXCEPT) | FDEXCEPT)
#define FDFLAG_CLR_EXCEPT(flag) (flag = (flag & ~FDEXCEPT))
struct ProcUnit {
    Scheduler *hScheduler;
    int fd;
    int flag;
    int timeout;
    void *data;
    int (*inHandle)();
    int (*outHandle)();
    int (*errHandle)();
    int (*defaultHandle)();
};
struct ProcUNIT {
    int i;
    int touchCnt;
    ProcUnit p;
};
#define getProcUNIT(proc) ( (ProcUNIT *) \
                            ((char *)proc - offsetof(struct ProcUNIT, p)) )

Scheduler gScheduler;

int scheduler_init(Scheduler *hScheduler);
int scheduler_destroy(Scheduler *hScheduler);
ProcUnit *scheduler_newProcUnit(Scheduler *hScheduler);
int scheduler_modProcUnit(Scheduler *hScheduler, ProcUnit *procUnit);
int scheduler_delProcUnit(Scheduler *hScheduler, ProcUnit *procUnit);
int scheduler_start(Scheduler *hScheduler);
/* internal */
int scheduler_allocProcUNITs(Scheduler *hScheduler);
int scheduler_addSchEntry(Scheduler *hScheduler, ProcUnit *procUnit);
int scheduler_removeSchEntry(Scheduler *hScheduler, ProcUnit *procUnit);

int infoServAccept(ProcUnit *proc);
int infoServError(ProcUnit *proc);
int inquireInput(ProcUnit *proc);
int inquireOutput(ProcUnit *proc);
int inquireErr(ProcUnit *proc);
int inquireCleanUp(ProcUnit *proc);
int inquire_execute(char *buf, int *len, char **outStr, size_t *outStrSz);

int initListener(eggInfoServerConfig *conf);

typedef struct {
    char *inbuf;
    int inbufsz;
    int inbuflen;
    
    char **outbuf;
    int *outbufsz;
    int outputcount;
    int outputcur;

    int totalin;
    int totalout;
} CacheData;
int cacheData_resetInput(CacheData *cache, int usedCount);
int cacheData_extendInput(CacheData *cache, int hintSz);
int cacheData_resetOutput(CacheData *cache, int usedCount);
int cacheData_extendOutput(CacheData *cache, int hintSz);
CacheData *cacheData_new(int hintInputSz, int hintOutputSz);
int cacheData_release(CacheData *cache);

HEGGPRTLOG g_infoserver_log_handle;

static eggInfoServerConfig configure;
int main(int argc, char *argv[])
{
    parseCmd(&configure, &argc, argv);
    readInfoServFile(&configure);

    {
        g_infoserver_log_handle =  eggPrtLog_init(configure.logpath);
        if (!g_infoserver_log_handle)
        {
            fprintf(stderr, "ERR eggPrtLog_init(%s) == NULL\n", configure.logpath);
            exit(-1);
        }
        eggPrtLog_set_level(g_infoserver_log_handle, configure.loglevel);
    }
    
    int servFd;
    if ((servFd = initListener(&configure)) < 0)
    {
        return -1;
    }
    
    Scheduler *scheduler = &gScheduler;
    scheduler_init(scheduler);
    ProcUnit *servListener = scheduler_newProcUnit(scheduler);
    {
        servListener->fd = servFd;
        FDFLAG_SET_IN(servListener->flag);
        FDFLAG_SET_EXCEPT(servListener->flag);
        servListener->inHandle = infoServAccept;
        servListener->errHandle = infoServError;
    }
    scheduler_modProcUnit(scheduler, servListener);
    scheduler_start(scheduler);

    scheduler_destroy(scheduler);

    eggPrtLog_uninit(g_infoserver_log_handle);
    return 0;
}

void usage(char *prog)
{
    printf("%s [-B 192.168.1.1:12345] [cluster-eggd.cfg]\n",
           prog);
    exit(0);
}
int parseCmd(eggInfoServerConfig *pConf, int *argc, char *argv[])
{
    memset(pConf, 0, sizeof(*pConf));
    int i;
    for (i = 1; i < *argc; )
    {
        if (strcmp(argv[i], "-B") == 0)
        {
            char *ip = strdup(argv[i+1]);
            assert(ip);
            char*port = strchr(ip, ':');
            if (!port)
            {
                usage(argv[0]);
            }
            *port++ = '\0';
            pConf->ip = ip;
            char *e;
            pConf->port = strtol(port, &e, 10);
            if (port == e)
            {
                pConf->port = 0;
                /* port == 0 Not allowed */
                usage(argv[0]);
            }
            i += 2;
        }
        else
        {
            pConf->infoFile = argv[i++];
            break;
        }
    }
    *argc -= i;
    memmove(argv+1, argv + i, (*argc + 1) * sizeof(char*));
    ++*argc;                    /* include progname */

    if (!pConf->infoFile)
    {
        pConf->infoFile = "/etc/egg3/cluster-eggd.cfg";
    }
           
    return 0;
}

int readInfoServFile(eggInfoServerConfig *pConf)
{
    char *name = pConf->infoFile;
    int ret = 0;

    HEGGCONFIG hConf;
    eggCfgVal* p_list_val;
    GList* list_value_iter;
    hConf = eggConfig_load_config(name);


    p_list_val = eggConfig_get_cfg(hConf, "logpath");
    if(p_list_val)
    {
        list_value_iter = g_list_first(p_list_val);
        char *logpath = list_value_iter->data;
        if (!pConf->logpath)
        {
            pConf->logpath = strdup(logpath);
            assert(pConf->logpath);
        }
    }

    pConf->loglevel = -1;
    p_list_val = eggConfig_get_cfg(hConf, "loglevel");
    if(p_list_val)
    {
        list_value_iter = g_list_first(p_list_val);
        
        int level;
        char* loglevel = (char*)list_value_iter->data;
        char *e;
        level = strtol(loglevel, &e, 10);
        if (e == loglevel)
        {
            level = -1;
        }
        pConf->loglevel = level;
    }

    p_list_val = eggConfig_get_cfg(hConf, "listen");
    if(p_list_val)
    {
        list_value_iter = g_list_first(p_list_val);
        char *ip = list_value_iter->data;
        if (!pConf->ip)
        {
            pConf->ip = strdup(ip);
        }
        list_value_iter = g_list_next(list_value_iter);
        char *port = list_value_iter->data;
        if (!pConf->port)
        {
            pConf->port = strtol(port, NULL, 10);
        }
    }

    char buf_tmp[4096];
    p_list_val = eggConfig_get_cfg(hConf, "dbName");
    if (p_list_val)
    {
        list_value_iter = g_list_first(p_list_val);
        
        do {
            char* dbname = list_value_iter->data;
            snprintf(buf_tmp, sizeof(buf_tmp),
                     "%s:eggpath", dbname);

            {
                GList* list_value_iter2;
                eggCfgVal* p_list_val2;
                p_list_val2 = eggConfig_get_cfg(hConf, buf_tmp);
                if (p_list_val2)
                {
                    SPANPOINT nextpoint = SPANPOINTMIN;

                    list_value_iter2 = g_list_first(p_list_val2);
                    do {
                        SPANRANGE range = {};

                        char* start = list_value_iter2->data;
                        if (!strcmp(start, "NULL"))
                        {
                            range.start = nextpoint;                            
                        }
                        else
                        {
                            range.start = strtoull(start, NULL, 10);

                        }
                            
                        list_value_iter2 = g_list_next(list_value_iter2);
                        char *end = list_value_iter2->data;
                        if (!strcmp(end, "NULL"))
                        {
                            range.end = SPANPOINTMAX;
                        }
                        else
                        {
                            range.end = strtoull(end, NULL, 10);

                        }

                        if (range.end == SPANPOINTMAX)
                        {
                            nextpoint = SPANPOINTMIN;
                        }
                        else
                        {
                            nextpoint = range.end + 1;
                        }
                            
                        list_value_iter2 = g_list_next(list_value_iter2);
                        char *hostaddress = list_value_iter2->data;
                            
                        if (!registerEggSpanUnit(strdup(dbname),
                                                 strdup(hostaddress),
                                                 range))
                        {
                            fprintf(stderr, "Error: configure file syntax error\n");
                            ret = -1;
                        }
                            
                    } while ((list_value_iter2 = g_list_next(list_value_iter2)) != 0);
                }
            }
            
        } while ((list_value_iter = g_list_next(list_value_iter)) != 0);

    }
    
    eggConfig_delete(hConf);
    
    /* FILE *fp = fopen(name, "r"); */
    /* if (!fp) */
    /* { */
    /*     fprintf(stderr, "%s:%d: %s %s\n", __func__, __LINE__, */
    /*             strerror(errno), name); */
    /*     return -1; */
    /* } */
    /* char *hostaddress = 0; */
    /* char *eggdirpath = malloc(1024); */
    /* assert(eggdirpath); */
    /* eggdirpath[1023] = '\0'; */
    /* SPANRANGE range = {}; */
    /* SPANPOINT nextpoint = SPANPOINTMIN; */
    /* char *line = NULL; */
    /* int ret = 0; */
    /* int nl; */
    /* size_t n = 0; */

    /* for (nl = 1; getline(&line, &n, fp) != -1; nl++) */
    /* { */
    /*     /\* eggdirpath\n[start1,end1] hostaddress1\n *\/ */
    /*     char *e = line; */
    /*     while (isspace(*e)) */
    /*         e++; */
        
    /*     if (*e == '#') */
    /*     {                       /\* comment *\/ */
    /*         ; */
    /*     } */
    /*     else if (strncasecmp(e, "Listen", 6) == 0) */
    /*     { */
	/*     if (pConf->ip) */
	/* 	continue; */
    /*         char *ee; */
    /*         e += 6; */
    /*         while (isspace(*e)) e++; */
    /*         ee = strchr(e, ':'); */
    /*         if (!ee) */
    /*         { */
    /*             fprintf(stderr, "%s SHOULD BE \"Listen 192.168.1.135:4000\"\n", */
    /*                      line); */
    /*             usage(""); */
    /*         } */
    /*         *ee = '\0'; */
    /*         pConf->ip = strdup(e); */
    /*         e = ee + 1; */
    /*         pConf->port = strtol(e, &ee, 10); */
    /*         if (e == ee) */
    /*         { */
    /*             fprintf(stderr, "%s SHOULD BE \"Listen 192.168.1.135:4000\"\n", */
    /*                      line); */
    /*             usage(""); */
    /*         } */
    /*     } */
    /*     else if (*e == '[' && strchr(e+1, ']')) */
    /*     {                       /\* [start,end] hostaddress\n *\/ */
    /*         char *ee = ++e; */
    /*         int comma = 0; */
    /*         range.start = strtoull(e, &ee, 10); */
    /*         if (e == ee) */
    /*         { */
    /*             range.start = nextpoint; */
    /*         } */
    /*         while (isspace(*ee)) ee++; */
    /*         if (*ee == ',') */
    /*         { */
    /*             ee++; */
    /*             comma = 1; */
    /*         } */
    /*         e = ee; */
    /*         range.end = strtoull(e, &ee, 10); */
    /*         if (e == ee) */
    /*         { */
    /*             if (comma) */
    /*             { */
    /*                 range.end = SPANPOINTMAX; */
    /*             } */
    /*             else */
    /*             { */
    /*                 range.end = range.start; */
    /*             } */
    /*         } */
            
    /*         if (range.end == SPANPOINTMAX) */
    /*         { */
    /*             nextpoint = SPANPOINTMIN; */
    /*         } */
    /*         else */
    /*         { */
    /*             nextpoint = range.end + 1; */
    /*         } */
    /*         while (isspace(*ee)) ee++; */
    /*         e = ee + 1;         /\* skip ']' *\/ */
    /*         while (isspace(*e)) e++; */
    /*         if (*e) */
    /*         { */
    /*             ee = e + 1; */
    /*             while (*ee && !isspace(*ee))                 */
    /*             { */
    /*                 ee++; */
    /*             } */
    /*             *ee = '\0'; */
    /*         } */
    /*         hostaddress = strdup(e); */
    /*         assert(hostaddress); */

    /*         /\* type:ip:port/path *\/ */
    /*         if (!(e = strchr(hostaddress, ':')) || */
    /*             !(e = strchr(e+1, ':')) || */
    /*             !(e = strchr(e+1, '/')) || */
    /*             !registerEggSpanUnit(strdup(eggdirpath), hostaddress, range)) */
    /*         { */
    /*             fprintf(stderr, "Error: configure file syntax error Line%d\n", */
    /*                     nl); */
    /*             ret = -1; */
    /*         } */
    /*     } */
    /*     else if (*e && *e != '[') */
    /*     { */
    /*         char *ee = e; */
    /*         do { */
    /*             ee++; */
    /*         } while (*ee && !isspace(*ee)); */
    /*         ee[0] = '\0'; */
    /*         strncpy(eggdirpath, e, 1023); */
    /*     } */
    /*     else */
    /*     { */
    /*         ; */
    /*     } */

    /* } */
    /* free(line); */
    /* free(eggdirpath); */
    /* if (ferror(fp)) */
    /* { */
    /*     fprintf(stderr, "Error: file read error Line%d, %s\n", */
    /*             nl, strerror(errno)); */
    /*     ret = -1; */
    /* } */
    /* fclose(fp); */


    return ret;
}


int eggSpanUnitList_register(HEGGSPANUNITLISTELEMENT hUnitListElement);
int hostAddressList_register(HEGGSPANUNITLISTELEMENT hUnitListElement);
int eggDirPathList_register(HEGGSPANUNITLISTELEMENT hUnitListElement);

HEGGSPANUNIT registerEggSpanUnit(char *eggdirpath, char *hostaddress,
                                 SPANRANGE range)
{
	if (!eggdirpath || !eggdirpath[0])
    {
		//fprintf(stderr, "eggdirpath is null\n");
        eggPrtLog_log_line(g_infoserver_log_handle, EGGPRTLOG_ERROR, "",
                           "eggdirpath is null\n");
		return NULL;
    }
	if (!hostaddress || !hostaddress[0])
    {
		//fprintf(stderr, "hostaddress is null\n");
        eggPrtLog_log_line(g_infoserver_log_handle, EGGPRTLOG_ERROR, "",
                           "hostaddress is null\n");
		return NULL;
	}
	if (!isValidSPANRANGE(range))
    {
        /*
		fprintf(stderr, "[%llu, %llu] must be in range [%llu, %llu]\n",
				(long long unsigned)range.start,
				(long long unsigned)range.end,
				(long long unsigned)SPANPOINTMIN,
				(long long unsigned)SPANPOINTMAX);
        */
        eggPrtLog_log_line(g_infoserver_log_handle, EGGPRTLOG_ERROR, "",
                           "[%llu, %llu] must be in range [%llu, %llu]\n",
                           (long long unsigned)range.start,
                           (long long unsigned)range.end,
                           (long long unsigned)SPANPOINTMIN,
                           (long long unsigned)SPANPOINTMAX);
		return NULL;
    }

    HEGGSPANUNITLISTELEMENT hUnitListElement;
    hUnitListElement = calloc(1, sizeof(*hUnitListElement));
    assert(hUnitListElement);
    hUnitListElement->eggSpanUnit.eggDirPath = eggdirpath;
    hUnitListElement->eggSpanUnit.hostAddress = hostaddress;
    hUnitListElement->eggSpanUnit.range = range;

    if (eggSpanUnitList_register(hUnitListElement) < 0)
    {
        return NULL;
    }
    if (hostAddressList_register(hUnitListElement) < 0)
    {
        return NULL;        
    }
    if (eggDirPathList_register(hUnitListElement) < 0)
    {
        return NULL;        
    }
    
    return (HEGGSPANUNIT)hUnitListElement;
}


#define PTR_UNIT 0
#define PTR_HOST 1
#define PTR_PATH 2
#define eggSpanUnitList_insertBefore(p, q) (HEGGSPANUNITLISTELEMENT)    \
    eggDoubleList_insertBefore(offsetof(EggSpanUnitListElement, prev[PTR_UNIT]), \
                               offsetof(EggSpanUnitListElement, next[PTR_UNIT]), \
                               p, q)
#define eggSpanUnitList_insertAfter(p, q) (HEGGSPANUNITLISTELEMENT)     \
    eggDoubleList_insertAfter(offsetof(EggSpanUnitListElement, prev[PTR_UNIT]), \
                              offsetof(EggSpanUnitListElement, next[PTR_UNIT]), \
                              p, q)
#define eggSpanUnitList_delete(p) (HEGGSPANUNITLISTELEMENT)             \
    eggDoubleList_delete(offsetof(EggSpanUnitListElement, prev[PTR_UNIT]), \
                         offsetof(EggSpanUnitListElement, next[PTR_UNIT]), \
                         p)
#define eggSpanUnitList_prev(p) (HEGGSPANUNITLISTELEMENT)               \
    eggDoubleList_prev(offsetof(EggSpanUnitListElement, prev[PTR_UNIT]), \
                       offsetof(EggSpanUnitListElement, next[PTR_UNIT]), \
                       p)
#define eggSpanUnitList_next(p) (HEGGSPANUNITLISTELEMENT)               \
    eggDoubleList_next(offsetof(EggSpanUnitListElement, prev[PTR_UNIT]), \
                       offsetof(EggSpanUnitListElement, next[PTR_UNIT]), \
                       p)
#define hostAddressList_insertBefore(p, q) (HEGGSPANUNITLISTELEMENT)    \
    eggDoubleList_insertBefore(offsetof(EggSpanUnitListElement, prev[PTR_HOST]), \
                               offsetof(EggSpanUnitListElement, next[PTR_HOST]), \
                               p, q)
#define hostAddressList_insertAfter(p, q) (HEGGSPANUNITLISTELEMENT)     \
    eggDoubleList_insertAfter(offsetof(EggSpanUnitListElement, prev[PTR_HOST]), \
                              offsetof(EggSpanUnitListElement, next[PTR_HOST]), \
                              p, q)
#define hostAddressList_delete(p) (HEGGSPANUNITLISTELEMENT)             \
    eggDoubleList_delete(offsetof(EggSpanUnitListElement, prev[PTR_HOST]), \
                         offsetof(EggSpanUnitListElement, next[PTR_HOST]), \
                         p)
#define hostAddressList_prev(p) (HEGGSPANUNITLISTELEMENT)               \
    eggDoubleList_prev(offsetof(EggSpanUnitListElement, prev[PTR_HOST]), \
                       offsetof(EggSpanUnitListElement, next[PTR_HOST]), \
                       p)
#define hostAddressList_next(p) (HEGGSPANUNITLISTELEMENT)               \
    eggDoubleList_next(offsetof(EggSpanUnitListElement, prev[PTR_HOST]), \
                       offsetof(EggSpanUnitListElement, next[PTR_HOST]), \
                       p)
#define eggDirPathList_insertBefore(p, q) (HEGGSPANUNITLISTELEMENT)     \
    eggDoubleList_insertBefore(offsetof(EggSpanUnitListElement, prev[PTR_PATH]), \
                               offsetof(EggSpanUnitListElement, next[PTR_PATH]), \
                               p, q)
#define eggDirPathList_insertAfter(p, q) (HEGGSPANUNITLISTELEMENT)      \
    eggDoubleList_insertAfter(offsetof(EggSpanUnitListElement, prev[PTR_PATH]), \
                              offsetof(EggSpanUnitListElement, next[PTR_PATH]), \
                              p, q)
#define eggDirPathList_delete(p) (HEGGSPANUNITLISTELEMENT)              \
    eggDoubleList_delete(offsetof(EggSpanUnitListElement, prev[PTR_PATH]), \
                         offsetof(EggSpanUnitListElement, next[PTR_PATH]), \
                         p)
#define eggDirPathList_prev(p) (HEGGSPANUNITLISTELEMENT)                \
    eggDoubleList_prev(offsetof(EggSpanUnitListElement, prev[PTR_PATH]), \
                       offsetof(EggSpanUnitListElement, next[PTR_PATH]), \
                       p)
#define eggDirPathList_next(p) (HEGGSPANUNITLISTELEMENT)                \
    eggDoubleList_next(offsetof(EggSpanUnitListElement, prev[PTR_PATH]), \
                       offsetof(EggSpanUnitListElement, next[PTR_PATH]), \
                       p)
        
void *eggDoubleList_insertAfter(int prevOff, int nextOff,
                                void *element, void *new);
void *eggDoubleList_insertBefore(int prevOff, int nextOff,
                                 void *element, void *new);
void *eggDoubleList_delete(int prevOff, int nextOff, void *element);
void *eggDoubleList_prev(int prevOff, int nextOff, void *element);
void *eggDoubleList_next(int prevOff, int nextOff, void *element);

void *eggDoubleList_insertBefore(int prevOff, int nextOff,
                                 void *element, void *new)
{
    if (!element)
    {
        return new;
    }
    if (!new)
    {
        return element;
    }
    void **prev = (void **)((char *)element + prevOff);
    void **pnext = 0;
    if (*prev)
    {
        pnext = (void **)((char *)*prev + nextOff);
    }
    *(void **)((char*)new + prevOff) = *prev;
    *(void **)((char*)new + nextOff) = element;
    *prev = new;
    pnext ? *pnext = new : (void)0;
    return new;
}

void *eggDoubleList_insertAfter(int prevOff, int nextOff,
                                void *element, void *new)
{
    if (!element)
    {
        return new;
    }
    if (!new)
    {
        return element;
    }
    void **next = (void **)((char *)element + nextOff);
    void **nprev = 0;
    if (*next)
    {
        nprev = (void **)((char *)*next + prevOff);
    }
    *(void **)((char*)new + prevOff) = element;
    *(void **)((char*)new + nextOff) = *next;
    *next = new;
    nprev ? *nprev = new : (void)0;
    return new;
}

void *eggDoubleList_delete(int prevOff, int nextOff, void *element)
{
    if (!element)
    {
        return NULL;
    }
    
    void **prev = (void **)((char *)element + prevOff);
    void **next = (void **)((char *)element + nextOff);
    if (*prev)
    {
        *(void **)((char *)*prev + nextOff) = *next;
    }
    if (*next)
    {
        *(void **)((char*)next + prevOff) = *prev;        
    }

    *prev = *next = NULL;
    return element;
}

void *eggDoubleList_prev(int prevOff, int nextOff, void *element)
{
    if (!element)
        return NULL;
    return *(void **)((char *)element + prevOff);
    
}

void *eggDoubleList_next(int prevOff, int nextOff, void *element)
{
    if (!element)
        return NULL;
    return *(void **)((char *)element + nextOff);
    
}

int eggSpanUnitList_register(HEGGSPANUNITLISTELEMENT hUnitListElement)
{
    allEggSpanUnit = eggSpanUnitList_insertBefore(allEggSpanUnit,
                                                  hUnitListElement);
    return 0;
}

int hostAddressList_register(HEGGSPANUNITLISTELEMENT e)
{
    HEGGSPANUNITLISTELEMENT p = allHostAddress;
    if (!allHostAddress)
    {
        allHostAddress = e;
        return 0;
    }

    /* sorted list: (host, path) */
    
    while (p)
    {
        int cmphost = strcmp(e->eggSpanUnit.hostAddress,
                             p->eggSpanUnit.hostAddress);
        int cmppath = strcmp(e->eggSpanUnit.eggDirPath,
                             p->eggSpanUnit.eggDirPath);
        if (cmphost < 0 || (cmphost == 0 &&
                            cmppath < 0))
        {
            HEGGSPANUNITLISTELEMENT h;
            h = hostAddressList_insertBefore(p, e);
            if (p == allHostAddress)
            {
                allHostAddress = h;
            }
            return 0;
        }
        else if (cmphost == 0 && cmppath == 0)
        {
            /* override happened */
            /*
            fprintf(stderr, "Error: config overlap happened: %s %s\n",
                    p->eggSpanUnit.hostAddress,
                    p->eggSpanUnit.eggDirPath);
            */

            eggPrtLog_log_line(g_infoserver_log_handle, EGGPRTLOG_ERROR, "",
                               "Error: config overlap happened: %s %s\n",
                               p->eggSpanUnit.hostAddress,
                               p->eggSpanUnit.eggDirPath);
                           
            return -1;
        }
        else if (!hostAddressList_next(p))
        {
            hostAddressList_insertAfter(p, e);
            return 0;
        }
        p = hostAddressList_next(p);
    }
    return 0;
}

HEGGSPANUNIT hostAddressList_searchHost(char *hostAddress, char *eggDirPath,
                                        int *count)
{
    HEGGSPANUNITLISTELEMENT start = NULL;
    *count = 0;
    HEGGSPANUNITLISTELEMENT h = allHostAddress;
    int n = strlen(hostAddress);
    while (h)
    {
        HEGGSPANUNIT hUnit = &h->eggSpanUnit;
        int cmp;
        if ((cmp = strncmp(hostAddress, hUnit->hostAddress, n)) == 0)
        {
            if (!eggDirPath || !eggDirPath[0])
            {
                !start ? start = h : (void)0;
                ++*count;
            }
            else if ((cmp = strcmp(eggDirPath, hUnit->eggDirPath)) == 0)
            {
                !start ? start = h : (void)0;
                ++*count;
            }
        }
        if (cmp < 0)
        {
            break;
        }
        h = hostAddressList_next(h);
    }
    if (!start)
    {
        return NULL;
    }
    HEGGSPANUNIT hUnit = calloc(1, *count * sizeof(*hUnit));
    assert(hUnit);
    for (h = start, n = 0; n < *count; n++)
    {
        hUnit[n].hostAddress = h->eggSpanUnit.hostAddress;
        hUnit[n].eggDirPath = h->eggSpanUnit.eggDirPath;
        hUnit[n].range =  h->eggSpanUnit.range;
        h = hostAddressList_next(h);
    }
    return hUnit;
}

int eggDirPathList_register(HEGGSPANUNITLISTELEMENT e)
{
    if (!allEggDirPath)
    {
        allEggDirPath = e;
        return 0;
    }

    /* sorted list: (path, range) */
    HEGGSPANUNITLISTELEMENT p = allEggDirPath;
    while (p)
    {
        int cmppath = strcmp(e->eggSpanUnit.eggDirPath,
                             p->eggSpanUnit.eggDirPath);
        int cmprange;
        cmprange = cmpSPANRANGE(e->eggSpanUnit.range, p->eggSpanUnit.range);
        
        if (cmppath < 0 || (cmppath == 0 &&
                            cmprange < 0))
        {
            HEGGSPANUNITLISTELEMENT h;
            h = eggDirPathList_insertBefore(p, e);
            if (p == allEggDirPath)
            {
                allEggDirPath = h;
            }
            return 0;
        }
        else if (cmppath == 0 && cmprange == 0)
        {
            /* override happened */
            /*
            fprintf(stderr, "Error: config overlap happened: %s [%llu, %llu]\n",
                    p->eggSpanUnit.eggDirPath,
                    (unsigned long long)p->eggSpanUnit.range.start,
                    (unsigned long long)p->eggSpanUnit.range.end);
            */
            eggPrtLog_log_line(g_infoserver_log_handle, EGGPRTLOG_ERROR, "",
                               "Error: config overlap happened: %s [%llu, %llu]\n",
                               p->eggSpanUnit.eggDirPath,
                               (unsigned long long)p->eggSpanUnit.range.start,
                               (unsigned long long)p->eggSpanUnit.range.end);

            return -1;
        }
        else if (!eggDirPathList_next(p))
        {
            eggDirPathList_insertAfter(p, e);
            return 0;
        }
        p = eggDirPathList_next(p);
    }
    return 0;    
}

HEGGSPANUNIT eggDirPathList_searchDirPath(char *eggDirPath, SPANRANGE range,
                                        int *count)
{
    HEGGSPANUNITLISTELEMENT start = NULL;
    *count = 0;
    HEGGSPANUNITLISTELEMENT h = allEggDirPath;
    while (h)
    {
        HEGGSPANUNIT hUnit = &h->eggSpanUnit;
        int cmp;
        if ((cmp = strcmp(eggDirPath, hUnit->eggDirPath)) == 0)
        {
            if (range.start == 0)
            {
                !start ? start = h : (void)0;
                ++*count;
            }
            else if ((cmp = cmpSPANRANGE(range, hUnit->range)) == 0)
            {
                !start ? start = h : (void)0;
                ++*count;
            }
        }
        if (cmp < 0)
        {
            break;
        }
        h = eggDirPathList_next(h);
    }
    if (!start)
    {
        return NULL;
    }
    HEGGSPANUNIT hUnit = calloc(1, *count * sizeof(*hUnit));
    assert(hUnit);
    int i;
    for (h = start, i = 0; i < *count; i++)
    {
        hUnit[i].hostAddress = h->eggSpanUnit.hostAddress;
        hUnit[i].eggDirPath = h->eggSpanUnit.eggDirPath;
        hUnit[i].range =  h->eggSpanUnit.range;
        h = eggDirPathList_next(h);
    }
    return hUnit;
}

int initListener(eggInfoServerConfig *conf)
{
    int fd;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        /*
        fprintf(stderr, "%s:%d: %s\n", __func__, __LINE__,
                strerror(errno));
        */

        eggPrtLog_log_line(g_infoserver_log_handle, EGGPRTLOG_ERROR, "",
                           "%s:%d: %s\n", __func__, __LINE__,
                           strerror(errno));

        
        return -1;
    }

    struct sockaddr_in servAddr;
    memset(&servAddr, 0, sizeof(struct sockaddr_in));
    servAddr.sin_family = AF_INET;
    if (conf->ip[0])
    {
        inet_aton(conf->ip, &servAddr.sin_addr);
    }
    else
    {
        servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    }
    servAddr.sin_port = htons(conf->port);

    int reuseaddr = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, 
                   (char *) &reuseaddr, sizeof(reuseaddr)) < 0)
    {
        /* INADDR_ANY may not successful */
        /*
        fprintf(stderr, "%s:%d: %s\n", __func__, __LINE__,
                strerror(errno));
        */
        eggPrtLog_log_line(g_infoserver_log_handle, EGGPRTLOG_ERROR, "",
                           "%s:%d: %s\n", __func__, __LINE__,
                           strerror(errno));

        return -1;
    }

    long flag;
    flag = fcntl(fd, F_GETFL);
    flag |= O_NONBLOCK;
    if (fcntl(fd, F_SETFL, flag) < 0)
    {
        eggPrtLog_log_line(g_infoserver_log_handle, EGGPRTLOG_ERROR, "",
                           "%s:%d: %s\n", __func__, __LINE__,
                           strerror(errno));
        return -1;
    }

    if (bind(fd, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0)
    {
        /*
        fprintf(stderr, "%s:%d: %s\n", __func__, __LINE__,
                strerror(errno));
        */

        eggPrtLog_log_line(g_infoserver_log_handle, EGGPRTLOG_ERROR, "",
                           "%s:%d: %s\n", __func__, __LINE__,
                           strerror(errno));

        return -1;
    }
    
    if (listen(fd, 1024) < 0)
    {
        /*
        fprintf(stderr, "%s:%d: %s\n", __func__, __LINE__,
                strerror(errno));
        */

        eggPrtLog_log_line(g_infoserver_log_handle, EGGPRTLOG_ERROR, "",
                           "%s:%d: %s\n", __func__, __LINE__,
                           strerror(errno));

        return -1;
    }

    return fd;
}


int infoServError(ProcUnit *proc)
{
    /*
    fprintf(stderr, "%s:%d %s\n", __func__, __LINE__, strerror(errno));

    fprintf(stderr, "%s:%d info server error exit\n", __func__, __LINE__);

    */
    eggPrtLog_log_line(g_infoserver_log_handle, EGGPRTLOG_ERROR, "",
                           "%s:%d: %s\n", __func__, __LINE__,
                           strerror(errno));

    eggPrtLog_log_line(g_infoserver_log_handle, EGGPRTLOG_ERROR, "",
                       "%s:%d info server error exit\n", __func__, __LINE__);


    
    exit(-1);
    /*
    if (proc->fd >= 0)
    {
        close(proc->fd);
        proc->fd = -1;
    }
    */
    return 0;    
    
}
int infoServAccept(ProcUnit *proc)
{
    int listenfd;
    listenfd = proc->fd;
    struct sockaddr_in connaddr;
    int connaddrlen = sizeof(connaddr);
    int connfd;
    int n = 0;
    int ret = 0;

    for ( ; ; )
    {
        if ((connfd = accept(listenfd,
                             (struct sockaddr *)&connaddr, &connaddrlen)) < 0)
        {
            
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                ret = 0;
            }
            else
            {

                /*
                fprintf(stderr, "%s:%d: %s\n", __func__, __LINE__,
                        strerror(errno));
                */

                eggPrtLog_log_line(g_infoserver_log_handle, EGGPRTLOG_ERROR, "",
                                   "%s:%d: %s\n", __func__, __LINE__,
                                   strerror(errno));


                ret = -1;
            }
            break;
        }


        long flag;
        flag = fcntl(connfd, F_GETFL);
        flag |= O_NONBLOCK;
        if (fcntl(connfd, F_SETFL, flag) < 0)
        {
            
            eggPrtLog_log_line(g_infoserver_log_handle, EGGPRTLOG_ERROR, "",
                               "%s:%d: %s\n", __func__, __LINE__,
                               strerror(errno));
            close(connfd);
            continue;
        }
        

        ProcUnit *conn = scheduler_newProcUnit(proc->hScheduler);
        assert(conn);
        {
            conn->fd = connfd;
            FDFLAG_SET_IN(conn->flag);
            FDFLAG_SET_EXCEPT(conn->flag);
            conn->inHandle = inquireInput;
            conn->outHandle = inquireOutput;
            conn->errHandle = inquireErr;
            CacheData *cache;
            cache = cacheData_new(256, 0);
            assert(cache);
            conn->data = cache;
        }
        scheduler_modProcUnit(proc->hScheduler, conn);
        n++;
    }
    
    return ret;
}

int inquireInput(ProcUnit *proc)
{
    CacheData *cache;
    cache = proc->data;
    int n;
    int readClosed = 0;

    int fd = proc->fd;
    n = 0;
    char *p;
    do {
        cache->inbuflen += n;
        cacheData_extendInput(cache, 0);
        p = cache->inbuf + cache->inbuflen;
    } while ((n = read(fd, p, cache->inbufsz-cache->inbuflen)) > 0);
    if (n > 0 ||
        n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
    {
        ;
    }
    else if (n < 0)
    {
        //fprintf(stderr, "%s:%d %s\n", __func__, __LINE__, strerror(errno));
        eggPrtLog_log_line(g_infoserver_log_handle, EGGPRTLOG_ERROR, "",
                           "%s:%d: %s\n", __func__, __LINE__,
                           strerror(errno));

    }
    else
    {
        readClosed = 1;
    }

    int nsend = 1;
    char *buf = cache->inbuf;
    n = cache->inbuflen;
    while (nsend > 0 && buf < cache->inbuf + cache->inbuflen)
    {
        char *respStr;
        size_t respStrSz;
        int used = n;
        inquire_execute(buf, &used, &respStr, &respStrSz);
        if (used == 0)
        {                       /* need more data */
            break;
        }
        cacheData_extendOutput(cache, 0);
        cache->outbuf[cache->outputcur] = respStr;
        cache->outbufsz[cache->outputcur] = (int)respStrSz;
        cache->outputcur++;
        nsend = inquireOutput(proc);
        buf += used;
        n -= used;
    }
    cacheData_resetInput(cache, buf - cache->inbuf);

    if (nsend < 0 || readClosed)
    {
        //fprintf(stderr, "Clean fd %d \n", proc->fd);
        inquireCleanUp(proc);
    }
    
    return 0;
}

int inquireOutputTry(ProcUnit *proc);
int inquireOutput(ProcUnit *proc)
{
    int n;
    if ((n = inquireOutputTry(proc)) < 0)
    {
        ;
    }
    else if (n == 0)
    {
        if (!FDFLAG_IS_OUT(proc->flag))
        {
            FDFLAG_SET_OUT(proc->flag);
            scheduler_modProcUnit(proc->hScheduler, proc);
        }

    }
    else
    {
        if (FDFLAG_IS_OUT(proc->flag))
        {
            FDFLAG_CLR_OUT(proc->flag);
            scheduler_modProcUnit(proc->hScheduler, proc);
        }        
    }
    
    return n;
}

int inquireOutputTry(ProcUnit *proc)
{
    int fd = proc->fd;
    CacheData *cache = (CacheData *)proc->data;

    int n = 0;
    int totalbytes = 0;
    int i;
    for (i = 0; i < cache->outputcur; i++)
    {
        n = write(fd, cache->outbuf[i], cache->outbufsz[i]);
        totalbytes += n > 0 ? n : 0;
          
        if (n < 0)
        {
            //fprintf(stderr, "%s:%d %s\n", __func__, __LINE__, strerror(errno));
                    eggPrtLog_log_line(g_infoserver_log_handle, EGGPRTLOG_ERROR, "",
                           "%s:%d: %s\n", __func__, __LINE__,
                           strerror(errno));

            break;
        }
        else if (n == 0)
        {
            break;
        }
        else if (n < cache->outbufsz[i])
        {
            cache->outbufsz[i] -= n;
            memmove(cache->outbuf[i], cache->outbuf[i] + n, cache->outbufsz[i]);
            n = 0;
            break;
        }

        free(cache->outbuf[i]);
    }
    cacheData_resetOutput(cache, i);

    cache->totalout += totalbytes;
    return n;
}

int inquire_execute(char *buf, int *len, char **outStr, size_t *outStrSz)
{
    *outStr = NULL;
    *outStrSz = 0;
    int ret = 0;
    HEGGSPANUNIT hEggSpanUnit;
    hEggSpanUnit = eggSpanUnit_from_inquirestring(buf, len);
    if (!hEggSpanUnit && *len == 0)
    {
        return -1;
    }
    HEGGSPANUNIT result = NULL;
    int count = 0;
    if (!hEggSpanUnit)
    {
        ;
    }
    else if (hEggSpanUnit->hostAddress)
    {
        result = hostAddressList_searchHost(hEggSpanUnit->hostAddress,
                                            hEggSpanUnit->eggDirPath,
                                            &count);
    }
    else if (hEggSpanUnit->eggDirPath)
    {
        result = eggDirPathList_searchDirPath(hEggSpanUnit->eggDirPath,
                                              hEggSpanUnit->range,
                                              &count);
    }
    else
    {
        //fprintf(stderr, "%s:%d unsupported inquire\n", __func__, __LINE__);
        eggPrtLog_log_line(g_infoserver_log_handle, EGGPRTLOG_ERROR, "",
                           "%s:%d unsupported inquire\n", __func__, __LINE__);

        ret = -1;
    }
    
    *outStr = eggSpanUnit_to_string(result, count, outStrSz);
    
    eggSpanUnit_free(hEggSpanUnit, 1);
    free(result);
    return ret;
}

int inquireErr(ProcUnit *proc)
{
    //fprintf(stderr, "%s:%d %s\n", __func__, __LINE__, strerror(errno));
    eggPrtLog_log_line(g_infoserver_log_handle, EGGPRTLOG_ERROR, "",
                       "%s:%d: %s\n", __func__, __LINE__,
                       strerror(errno));

    inquireCleanUp(proc);
    return 0;
}

int inquireCleanUp(ProcUnit *proc)
{
    scheduler_delProcUnit(proc->hScheduler, proc);
    
    if (proc->fd >= 0)
    {
        close(proc->fd);
        proc->fd = -1;
    }
    FDFLAG_CLR_IN(proc->flag);
    FDFLAG_CLR_OUT(proc->flag);
    FDFLAG_CLR_EXCEPT(proc->flag);
    cacheData_release((CacheData *)proc->data);
    
    return 0;
}


int cacheData_extendInput(CacheData *cache, int hintSz)
{
    int delta = hintSz <= 0 ? 512 : hintSz;
    if (cache->inbufsz == cache->inbuflen)
    {
        cache->inbufsz += delta;
        cache->inbuf = realloc(cache->inbuf, cache->inbufsz);
        assert(cache->inbuf);
    }
    return 0;
}

int cacheData_extendOutput(CacheData *cache, int hintSz)
{
    int delta = hintSz <= 0 ? 100 : hintSz;
    
    if (cache->outputcount == cache->outputcur)
    {
        cache->outputcount += delta;
        cache->outbuf = realloc(cache->outbuf,
                                sizeof(*cache->outbuf)
                                * cache->outputcount);
        assert(cache->outbuf);
        cache->outbufsz = realloc(cache->outbufsz,
                                  sizeof(*cache->outbufsz)
                                  * cache->outputcount);
        assert(cache->outbufsz);
    }
    return 0;
}

int cacheData_resetInput(CacheData *cache, int usedCount)
{
    cache->inbuflen -= usedCount;
    memmove(cache->inbuf, cache->inbuf + usedCount, cache->inbuflen);
    return  0;
}

int cacheData_resetOutput(CacheData *cache, int usedCount)
{
    cache->outputcur -= usedCount;
    memmove(cache->outbuf, cache->outbuf + usedCount,
            cache->outputcur * sizeof(*cache->outbuf));
    memmove(cache->outbufsz, cache->outbufsz + usedCount,
            cache->outputcur * sizeof(*cache->outbufsz));
    
    return  0;
}

int cacheData_release(CacheData *cache)
{
    free(cache->inbuf);
    int i;
    for (i = 0; i < cache->outputcur; i++)
    {
        free(cache->outbuf[i]);
    }
    free(cache->outbuf);
    free(cache->outbufsz);
    free(cache);
    return 0;
}

CacheData *cacheData_new(int hintInputSz, int hintOutputSz)
{
    CacheData *cache;
    cache = calloc(1, sizeof(*cache));
    assert(cache);
    if (hintInputSz > 0)
    {
        cache->inbufsz = hintInputSz;
        cache->inbuf = malloc(cache->inbufsz);
        assert(cache->inbuf);
        cache->inbuflen = 0;
    }
    if (hintOutputSz > 0)
    {
        cache->outputcount = hintOutputSz;
        cache->outbuf = malloc(cache->outputcount
                               * sizeof(*cache->outbuf));
        cache->outbufsz = malloc(cache->outputcount
                                  * sizeof(*cache->outbufsz));
        cache->outputcur = 0;
    }
    return cache;
}


#ifdef HAVE_SYS_EPOLL_H
/*
 * epoll implement
 */

#include <sys/epoll.h>
struct Scheduler {
    int procAllocSz;
    ProcUNIT **procs;
    int szprocs;
    
    ScheUnit *schelist;
    int nschelist;
    int szschelist;
    
    int epollfd;
    struct epoll_event events[100];
};

int scheduler_init(Scheduler *hScheduler)
{
    if ((hScheduler->epollfd = epoll_create(100)) < 0)
    {
        //fprintf(stderr, "%s:%d %s", __func__, __LINE__, strerror(errno));

        eggPrtLog_log_line(g_infoserver_log_handle, EGGPRTLOG_ERROR, "",
                           "%s:%d: %s\n", __func__, __LINE__,
                           strerror(errno));

        exit(-1);
        goto err;
    }
    
    hScheduler->procAllocSz = 100;
    hScheduler->szprocs = 0;
    hScheduler->procs = NULL;
    hScheduler->szschelist = 1;
    hScheduler->nschelist = 1;  /* schelist[0] not used */
    hScheduler->schelist = NULL;    
    scheduler_allocProcUNITs(hScheduler);

    return 0;
err:
    return -1;
}

int scheduler_destroy(Scheduler *hScheduler)
{
    close(hScheduler->epollfd);
    
    int i;
    for (i = 1; i < hScheduler->nschelist; i++) /* schelist[0] not used */
    {
        ProcUnit *proc;
        proc = hScheduler->schelist[i].proc;
        if (proc->fd >= 0)
        {
            close(proc->fd);
        }
    }
    free(hScheduler->schelist);
    
    for (i = 0; i < hScheduler->szprocs; i++)
    {
        free(hScheduler->procs[i]);
    }
    free(hScheduler->procs);

    return 0;
}

int scheduler_delProcUnit(Scheduler *hScheduler, ProcUnit *procUnit)
{
    assert(procUnit->hScheduler == hScheduler);

    int ret = 0;
    struct epoll_event ev;
    if (epoll_ctl(hScheduler->epollfd, EPOLL_CTL_DEL, procUnit->fd, &ev) == -1)
    {
        /*
        fprintf(stderr, "%s:%d: %s", __func__, __LINE__,
                strerror(errno));
        */
        eggPrtLog_log_line(g_infoserver_log_handle, EGGPRTLOG_ERROR, "",
                           "%s:%d: %s\n", __func__, __LINE__,
                           strerror(errno));

        ret = -1;
        exit(-1);
    }
    scheduler_removeSchEntry(hScheduler, procUnit);
        
    return ret;
}

int scheduler_modProcUnit(Scheduler *hScheduler, ProcUnit *procUnit)
{
    assert(FDFLAG_IS_IN(procUnit->flag) || FDFLAG_IS_OUT(procUnit->flag));
    assert(procUnit->hScheduler == hScheduler);

    struct epoll_event ev = {};
    ev.data.ptr = procUnit;
    if (FDFLAG_IS_IN(procUnit->flag))
    {
        ev.events |= EPOLLIN | EPOLLET;
    }
    if (FDFLAG_IS_OUT(procUnit->flag))
    {
        ev.events |= EPOLLOUT | EPOLLET;
    }
    if (FDFLAG_IS_EXCEPT(procUnit->flag))
    {
        ev.events |= EPOLLERR | EPOLLET;
    }

    int ret;
    
    ProcUNIT *pUNIT = getProcUNIT(procUnit);
    assert(pUNIT->touchCnt > 0);
    if (pUNIT->touchCnt == 1)
    {
        ret = epoll_ctl(hScheduler->epollfd, EPOLL_CTL_ADD, procUnit->fd, &ev);
        if (ret < 0)
        {
            /*
            fprintf(stderr, "%s:%d: %s", __func__, __LINE__,
                strerror(errno));
            */
                    eggPrtLog_log_line(g_infoserver_log_handle, EGGPRTLOG_ERROR, "",
                           "%s:%d: %s\n", __func__, __LINE__,
                           strerror(errno));

            
            exit(-1);
        }
        scheduler_addSchEntry(hScheduler, procUnit);
    }
    else
    {
        ret = epoll_ctl(hScheduler->epollfd, EPOLL_CTL_MOD, procUnit->fd, &ev);
        if (ret < 0)
        {
            /*
            fprintf(stderr, "%s:%d: %s", __func__, __LINE__,
                    strerror(errno));
            */
                    eggPrtLog_log_line(g_infoserver_log_handle, EGGPRTLOG_ERROR, "",
                           "%s:%d: %s\n", __func__, __LINE__,
                           strerror(errno));

            exit(-1);
        }
        scheduler_removeSchEntry(hScheduler, procUnit);
        scheduler_addSchEntry(hScheduler, procUnit);
    }
    pUNIT->touchCnt++;


    return 0;

}

int scheduler_start(Scheduler *hScheduler)
{
    int epollfd = hScheduler->epollfd;
    struct epoll_event *events = hScheduler->events;
    int maxevents = sizeof(hScheduler->events) / sizeof(hScheduler->events[0]);
    
    for ( ; ; )
    {
        int nfds;
        nfds = epoll_wait(epollfd, events, maxevents, -1);
        if (nfds == -1)
        {
            if (errno == EINTR)
            {
                continue;
            }
            else
            {
                //fprintf(stderr, "%s:%d %s\n", __func__, __LINE__, strerror(errno));
                eggPrtLog_log_line(g_infoserver_log_handle, EGGPRTLOG_ERROR, "",
                                   "%s:%d: %s\n", __func__, __LINE__,
                                   strerror(errno));


                goto err;
            }
        }
        int i;
        for (i = 0; i < nfds; i++) {
            
            ProcUnit *proc = (ProcUnit *)events[i].data.ptr;
            
            if (FDFLAG_IS_IN(proc->flag)
                && events[i].events & EPOLLIN)
            {
                proc->inHandle(proc);
            }
            if (FDFLAG_IS_OUT(proc->flag)
                && events[i].events & EPOLLOUT)
            {
                proc->outHandle(proc);
            }
            if (FDFLAG_IS_EXCEPT(proc->flag)
                && events[i].events & EPOLLERR)
            {
                proc->errHandle(proc);
            }
        }
    }    
    return 0;
err:
    
    return -1;
}

#else
/*
 * select implement
 */
#include <sys/select.h>
struct Scheduler {
    int procAllocSz;
    ProcUNIT **procs;
    int szprocs;
    
    ScheUnit *schelist;
    int nschelist;
    int szschelist;

    fd_set readfds;
    fd_set writefds;
    fd_set exceptfds;
};

int scheduler_init(Scheduler *hScheduler)
{
    
    hScheduler->procAllocSz = 100;
    hScheduler->szprocs = 0;
    hScheduler->procs = NULL;
    hScheduler->szschelist = 1;
    hScheduler->nschelist = 1;  /* schelist[0] not used */
    hScheduler->schelist = NULL;    
    scheduler_allocProcUNITs(hScheduler);

    return 0;
}

int scheduler_destroy(Scheduler *hScheduler)
{
    int i;
    for (i = 1; i < hScheduler->nschelist; i++) /* schelist[0] not used */
    {
        ProcUnit *proc;
        proc = hScheduler->schelist[i].proc;
        if (proc->fd >= 0)
        {
            close(proc->fd);
        }
    }
    free(hScheduler->schelist);
    
    for (i = 0; i < hScheduler->szprocs; i++)
    {
        free(hScheduler->procs[i]);
    }
    free(hScheduler->procs);

    return 0;
}

int scheduler_delProcUnit(Scheduler *hScheduler, ProcUnit *procUnit)
{
    assert(procUnit->hScheduler == hScheduler);

    scheduler_removeSchEntry(hScheduler, procUnit);
        
    return 0;
}

int scheduler_modProcUnit(Scheduler *hScheduler, ProcUnit *procUnit)
{
    assert(FDFLAG_IS_IN(procUnit->flag) || FDFLAG_IS_OUT(procUnit->flag));    
    assert(procUnit->hScheduler == hScheduler);

    ProcUNIT *pUNIT = getProcUNIT(procUnit);

    assert(pUNIT->touchCnt > 0);
    if (pUNIT->touchCnt == 1)
    {
        scheduler_addSchEntry(hScheduler, procUnit);
    }
    else
    {
        scheduler_removeSchEntry(hScheduler, procUnit);
        scheduler_addSchEntry(hScheduler, procUnit);
    }
    pUNIT->touchCnt++;

    
    return 0;
}

int scheduler_start(Scheduler *hScheduler)
{
    int i;
    
    for ( ; ; )
    {
        FD_ZERO(&hScheduler->readfds);
        FD_ZERO(&hScheduler->writefds);
        FD_ZERO(&hScheduler->exceptfds);
        int maxfd = 0;
        for (i = 1; i < hScheduler->nschelist; i++) /* schelist[0] not used */
        {
            ProcUnit *proc;
            proc = hScheduler->schelist[i].proc;
            assert(proc->fd >= 0);

            if (proc->fd > maxfd)
            {
                maxfd = proc->fd;
            }
            
            if (FDFLAG_IS_IN(proc->flag))
            {
                FD_SET(proc->fd, &hScheduler->readfds);
            }
            if (FDFLAG_IS_OUT(proc->flag))
            {
                FD_SET(proc->fd, &hScheduler->writefds);
            }
            if (FDFLAG_IS_EXCEPT(proc->flag))
            {
                FD_SET(proc->fd, &hScheduler->exceptfds);
            }
        }
        
        int nfds;
        nfds = select(maxfd+1,
                      &hScheduler->readfds,
                      &hScheduler->writefds,
                      &hScheduler->exceptfds,
                      NULL);
                      
        if (nfds == -1)
        {
            if (errno == EINTR)
            {
                continue;
            }
            else
            {
                //fprintf(stderr, "%s:%d %s\n", __func__, __LINE__, strerror(errno));
                eggPrtLog_log_line(g_infoserver_log_handle, EGGPRTLOG_ERROR, "",
                                   "%s:%d: %s\n", __func__, __LINE__,
                                   strerror(errno));

                goto err;
            }
        }
        else if (nfds == 0)
        {
            continue;
        }

        for (i = 1; i < hScheduler->nschelist; i++) /* schelist[0] not used */
        {

            ProcUnit *proc;
            proc = hScheduler->schelist[i].proc;

            if (FDFLAG_IS_IN(proc->flag)
                && FD_ISSET(proc->fd, &hScheduler->readfds))
            {
                proc->inHandle(proc);
            }
            if (FDFLAG_IS_OUT(proc->flag)
                && FD_ISSET(proc->fd, &hScheduler->writefds))
            {
                proc->outHandle(proc);
            }
            if (FDFLAG_IS_EXCEPT(proc->flag)
                && FD_ISSET(proc->fd, &hScheduler->exceptfds))
            {
                proc->errHandle(proc);
            }
        }
    }
    return 0;
err:
    
    return -1;
}

#endif
    



ProcUnit *scheduler_newProcUnit(Scheduler *hScheduler)
{
    ProcUnit *procUnit;
    int i, ii;
    for (i = 0; i < hScheduler->szprocs; i++)
    {
        for (ii = 0; ii < hScheduler->procAllocSz; ii++)
        {
            if (hScheduler->procs[i][ii].i == 0)
            {
                hScheduler->procs[i][ii].i = -1;
                hScheduler->procs[i][ii].touchCnt = 1;
                procUnit = &hScheduler->procs[i][ii].p;
                goto found;
            }
        }
    }
    if  (i == hScheduler->szprocs)
    {
        scheduler_allocProcUNITs(hScheduler);
        i = hScheduler->szprocs - 1;
        hScheduler->procs[i][0].i = -1;
        hScheduler->procs[i][0].touchCnt = 1;
        procUnit = &hScheduler->procs[i][0].p;
    }
found:
    memset(procUnit, 0, sizeof(*procUnit));
    procUnit->fd = -1;
    procUnit->hScheduler = hScheduler;
    return procUnit;
}


int scheduler_allocProcUNITs(Scheduler *hScheduler)
{
    hScheduler->szprocs++;
    hScheduler->procs = realloc(hScheduler->procs,
                                hScheduler->szprocs
                                * sizeof(*hScheduler->procs));
    assert(hScheduler->procs);
    int i = hScheduler->szprocs - 1;
    hScheduler->procs[i] = calloc(hScheduler->procAllocSz,
                                  sizeof(*hScheduler->procs[0]));
    assert(hScheduler->procs[i]);
    hScheduler->szschelist += hScheduler->procAllocSz;
    hScheduler->schelist = realloc(hScheduler->schelist,
                                   hScheduler->szschelist
                                   * sizeof(*hScheduler->schelist));
    assert(hScheduler->schelist);

    return 0;
}


int scheduler_addSchEntry(Scheduler *hScheduler, ProcUnit *procUnit)
{
    /* append to end */
    int i = hScheduler->nschelist;
    hScheduler->schelist[i].proc = procUnit;
    hScheduler->nschelist++;
    ProcUNIT *pUNIT = getProcUNIT(procUnit);
    pUNIT->i = i;

    return 0;
}
int scheduler_removeSchEntry(Scheduler *hScheduler, ProcUnit *procUnit)
{
    int i;
    ProcUNIT *pUNIT = getProcUNIT(procUnit);
    i = pUNIT->i;
    assert(i > 0);

    int j;
    for (j = i + 1; j < hScheduler->nschelist; j++)
    {
        hScheduler->schelist[j-1] = hScheduler->schelist[j];
        ProcUNIT *pUNIT2 = getProcUNIT(hScheduler->schelist[j-1].proc);
        pUNIT2->i = j - 1;
        
    }
    hScheduler->nschelist--;
    pUNIT->i = 0;
    
    return 0;
}




