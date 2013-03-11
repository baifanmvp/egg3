#include "eggRWSIntServer.h"
#include "eggRWSMemDptor.h"
#include "eggRWSMemManager.h"
#include "eggRWSBakerManager.h"
#include "eggRWSPackBuf.h"
#include "eggRWSLog.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <assert.h>
#include <stdio.h>
#include <time.h>
#include <ctype.h>


#define LOG_INFO(who, ...) eggRWSLog_log_line(g_integrator_log, EGGRWSLOG_INFO, who, __VA_ARGS__)
#define LOG_WARN(who, ...) eggRWSLog_log_line(g_integrator_log, EGGRWSLOG_WARN, who, __VA_ARGS__)
#define LOG_ERR(who, ...) eggRWSLog_log_line(g_integrator_log, EGGRWSLOG_ERROR, who, __VA_ARGS__)
#define LOG_CLAIM(who, ...) eggRWSLog_log_line(g_integrator_log, EGGRWSLOG_CLAIM, who, __VA_ARGS__)


static EGGRWSMEMSERVERID eggRWSMemServerDescriptor_castToId(char *name);


char *eggRWSMemServerStat_str(enum eggRWSMemServerStat state)
{
    static char statstr[100];
    statstr[0] = '\0';
    if (eggRWSMemServerStat_getStat(state) == EGGRWSMEMSERVERSTAT_ERROR)
    {
        strcat(statstr, "Error&");
    }
    if (eggRWSMemServerStat_getStat(state) == EGGRWSMEMSERVERSTAT_UNFORKED)
    {
        strcat(statstr, "Unforked&");
    }
    if (eggRWSMemServerStat_getStat(state) == EGGRWSMEMSERVERSTAT_NORMAL)
    {
        strcat(statstr, "Normal&");
    }
    if (eggRWSMemServerStat_getStat(state) == EGGRWSMEMSERVERSTAT_CLEANUP)
    {
        strcat(statstr, "Cleanup&");
    }
    if (eggRWSMemServerStat_getStat(state) == EGGRWSMEMSERVERSTAT_DUMPING)
    {
        strcat(statstr, "Dumping&");
    }
    if (eggRWSMemServerStat_getStat(state) == EGGRWSMEMSERVERSTAT_HALTING)
    {
        strcat(statstr, "Halting&");
    }
    
    if (eggRWSMemServerStat_is_writeable(state))
    {
        strcat(statstr, "Writeable&");
    }
    if (eggRWSMemServerStat_is_readable(state))
    {
        strcat(statstr, "Reable&");
    }
    if (eggRWSMemServerStat_is_retry(state))
    {
        strcat(statstr, "Retry&");
    }
    int e = 0;
    e = strlen(statstr);
    if (statstr[e-1] == '&')
    {
        statstr[e-1] = '\0';
    }
    return statstr;
}


static HEGGNETPACKAGE eggRWSMemClient_processing(HEGGRWSMEMSERVERDESCRIPTOR hMemServerD,
                                                 HEGGNETPACKAGE hNetPackage);
static void *eggRWSMemClient(void *arg);


static EBOOL isProcessOk(int pid)
{
    int pid_status = 0;
    
    if (waitpid(pid, &pid_status, WNOHANG) < 0)
    {
         return EGG_FALSE;
    }
    if (pid_status == 0)
    {
        if (kill(pid, 0) < 0)
        {
            LOG_INFO("MemServerD", "pid[%d] exit normally 0", (int)pid);
            return EGG_FALSE;
        }
        return EGG_TRUE;
    }
    if (WIFEXITED(pid_status))
    {
        LOG_INFO("MemServerD", "pid[%d] exit normally %d", (int)pid, WEXITSTATUS(pid_status));
        return EGG_FALSE;
    }
    if (WIFSIGNALED(pid_status))
    {
        LOG_INFO("MemServerD", "pid[%d] receive signal %d", (int)pid, WTERMSIG(pid_status));
        return EGG_FALSE;
    }
    if (WIFSTOPPED(pid_status))
    {
        LOG_INFO("MemServerD", "pid[%d] stop by signal %d", (int)pid, WSTOPSIG(pid_status));
        return EGG_FALSE;
    }

    return EGG_TRUE;
}

static int eggRWSMemServerDescriptor_killBadPid(HEGGRWSMEMSERVERDESCRIPTOR hMemServerD)
{
    pid_t pid;
    int pid_status = 0;
    pid = hMemServerD->pid;
    if (pid > 0)
    {
        waitpid(pid, &pid_status, 0);
    }
        LOG_INFO("MemServerD", "pid[%d] exit [%s]",
                 (int)hMemServerD->pid,
                 hMemServerD->name);

    
    hMemServerD->pid = 0;
    STATUSINSTRUCT *psi;    
    psi = &hMemServerD->si_head;
    psi->timeToLive = 30;

    return 0;
}

const time_t TIMETOLIVE_WAITFOREVER = (time_t)INT_MAX;
static const time_t TIMETOLIVE_DUMPING_MIN = (time_t)3;
static const time_t TIMETOLIVE_HALTING_MIN = (time_t)0;


int statusinstruct_set(STATUSINSTRUCT *psi, int stat, time_t timeToLive)
{
    if (stat == EGGRWSMEMSERVERSTAT_UNFORKED)
    {
        eggRWSMemServerStat_setStat(psi->stat, stat);
        eggRWSMemServerStat_clear_writeable(psi->stat);
        eggRWSMemServerStat_clear_readable(psi->stat);
        eggRWSMemServerStat_clear_retry(psi->stat);
        
        psi->timeToLive = 0;
        psi->trigger = eggRWSMemServerDescriptor_fork;
    }
    else if (stat == EGGRWSMEMSERVERSTAT_NORMAL)
    {
        eggRWSMemServerStat_setStat(psi->stat, stat);
        eggRWSMemServerStat_set_writeable(psi->stat);
        eggRWSMemServerStat_set_readable(psi->stat);
        eggRWSMemServerStat_clear_retry(psi->stat);

        psi->timeToLive = timeToLive;
        psi->trigger = eggRWSMemServerDescriptor_dumping;
    }
    else if (stat == EGGRWSMEMSERVERSTAT_CLEANUP)
    {
        eggRWSMemServerStat_setStat(psi->stat, stat);
        eggRWSMemServerStat_clear_writeable(psi->stat);
        eggRWSMemServerStat_clear_readable(psi->stat);
        eggRWSMemServerStat_clear_retry(psi->stat);

        psi->timeToLive = timeToLive;
        psi->trigger = eggRWSMemServerDescriptor_dumping;
    }
    else if (stat == EGGRWSMEMSERVERSTAT_DUMPING)
    {
        eggRWSMemServerStat_setStat(psi->stat, stat);
        eggRWSMemServerStat_clear_writeable(psi->stat);
        eggRWSMemServerStat_set_readable(psi->stat);
        eggRWSMemServerStat_clear_retry(psi->stat);

        psi->timeToLive = timeToLive;
        psi->trigger = eggRWSMemServerDescriptor_halting;
    }
    else if (stat == EGGRWSMEMSERVERSTAT_HALTING)
    {
        eggRWSMemServerStat_setStat(psi->stat, stat);
        eggRWSMemServerStat_clear_writeable(psi->stat);
        eggRWSMemServerStat_clear_readable(psi->stat);
        eggRWSMemServerStat_clear_retry(psi->stat);
        
        psi->timeToLive = timeToLive;
        psi->trigger = eggRWSMemServerDescriptor_end;
    }
    else if (stat == EGGRWSMEMSERVERSTAT_ERROR)
    {
        eggRWSMemServerStat_setStat(psi->stat, stat);
    }
    
    return 0;
}

char *statusinstruct_str(STATUSINSTRUCT *psi)
{
    if (!psi)
    {
        return "";
    }
    static char buf[200];
    snprintf(buf, sizeof(buf), "%s %ld",
             eggRWSMemServerStat_str(psi->stat),
             (long)psi->timeToLive);
    return buf;
    
}



int eggRWSMemServerDescriptor_fork(HEGGRWSMEMSERVERDESCRIPTOR hMemServerD)
{
    
    LOG_INFO("MemServerD", "-end of status [%s] count_user [%d] [%s]",
             statusinstruct_str(&hMemServerD->si_head),
             hMemServerD->count_user,
             hMemServerD->name);
    
    STATUSINSTRUCT *psi;
    int pid;
    if ((pid = fork()) < 0)
    {

        LOG_ERR("MemServerD", "fork ERROR %s [%s]",
                strerror(errno),
                hMemServerD->name);
        
    error_fork:
        psi = &hMemServerD->si_head;
        eggRWSMemServerStat_set_retry(psi->stat);
        psi->timeToLive = 30;  /* retry later */
        eggRWSMemServerManager_setupTimeout_memServer_nolock(hMemServerD->hManager,
                                                             hMemServerD);
        
        LOG_INFO("MemServerD", "+beg of status [%s] count_user [%d] [%s]",
                 statusinstruct_str(&hMemServerD->si_head),                 
                 hMemServerD->count_user,                 
                 hMemServerD->name);
        
        return -1;
    }
    else if (pid > 0)
    {
        LOG_INFO("MemServerD", "fork pid[%d] [%s]",
                (int)pid,
                hMemServerD->name);
        
        hMemServerD->pid = pid;
        
        if (!isProcessOk(hMemServerD->pid))
        {
            eggRWSMemServerDescriptor_killBadPid(hMemServerD);
            goto error_fork;
        }

        if (eggRWSMemServerDescriptor_openClient(hMemServerD) < 0)
        {
            eggRWSMemServerDescriptor_killBadPid(hMemServerD);
            goto error_fork;
        }
        
        eggRWSMemServerDescriptor_popStatInstruct(hMemServerD);
        eggRWSMemServerManager_setupTimeout_memServer_nolock(hMemServerD->hManager,
                                                             hMemServerD);

        LOG_INFO("MemServerD", "+beg of status [%s] count_user [%d] [%s]",
                 statusinstruct_str(&hMemServerD->si_head),
                 hMemServerD->count_user,                 
                 hMemServerD->name);
            
        return 0;
    }
    else
    {
        int ret;
        if ((ret = execve(hMemServerD->forkArgv[0], hMemServerD->forkArgv, NULL)) < 0)
        {
        
            fprintf(stderr, "%s:%d:%s ERR execve(%s): %s\n",
                    __FILE__, __LINE__, __func__,
                    hMemServerD->forkArgv[0], strerror(errno));
        }
        exit(-1);
    }
             
}

int eggRWSMemServerDescriptor_doDumpDocument(HEGGRWSMEMSERVERDESCRIPTOR hMemServerD);
int eggRWSMemServerDescriptor_dumping(HEGGRWSMEMSERVERDESCRIPTOR hMemServerD)
{

    LOG_INFO("MemServerD", "-end of status [%s] count_user [%d] [%s]",
             statusinstruct_str(&hMemServerD->si_head),             
             hMemServerD->count_user,
             hMemServerD->name);
    
    STATUSINSTRUCT *psi;
    psi = &hMemServerD->si_head;

    if (!(hMemServerD->count_user == 0 && eggRWSMemServerManager_askDump(hMemServerD->hManager, hMemServerD)))
    {
        eggRWSMemServerStat_clear_writeable(psi->stat);
        eggRWSMemServerStat_set_readable(psi->stat);
        eggRWSMemServerStat_set_retry(psi->stat);
        psi->timeToLive = TIMETOLIVE_WAITFOREVER; /* retry time */
    }
    else
    {
        statusinstruct_set(psi, EGGRWSMEMSERVERSTAT_DUMPING, TIMETOLIVE_DUMPING_MIN);
        eggRWSMemServerDescriptor_userIncrease(hMemServerD);
        eggRWSMemServerDescriptor_doDumpDocument(hMemServerD);
    }
    
    if (hMemServerD->threadid == 0)
    {
        eggRWSMemServerStat_clear_writeable(psi->stat);
        eggRWSMemServerStat_clear_readable(psi->stat);
    }

    eggRWSMemServerManager_setupTimeout_memServer_nolock(hMemServerD->hManager,
                                                         hMemServerD);

    LOG_INFO("MemServerD", "+beg of status [%s] count_user [%d] [%s]",
             statusinstruct_str(&hMemServerD->si_head),             
             hMemServerD->count_user,             
             hMemServerD->name);

    return 0;
    
}

int eggRWSMemServerDescriptor_halting(HEGGRWSMEMSERVERDESCRIPTOR hMemServerD)
{
    LOG_INFO("MemServerD", "-end of status [%s] count_user [%d] [%s]",
             statusinstruct_str(&hMemServerD->si_head),
             hMemServerD->count_user,
             hMemServerD->name);
    
    STATUSINSTRUCT *psi;
    psi = &hMemServerD->si_head;

    if (hMemServerD->count_user > 0)
    {
        eggRWSMemServerStat_set_retry(psi->stat);
        psi->timeToLive = TIMETOLIVE_WAITFOREVER; /* retry time */
    }
    else
    {
        eggRWSMemServerManager_unaskDump(hMemServerD->hManager, hMemServerD);
        
        statusinstruct_set(psi, EGGRWSMEMSERVERSTAT_HALTING, TIMETOLIVE_HALTING_MIN);
    }
    
    eggRWSMemServerManager_setupTimeout_memServer_nolock(hMemServerD->hManager,
                                                         hMemServerD);

    LOG_INFO("MemServerD", "+beg of status [%s] count_user [%d] [%s]",
             statusinstruct_str(&hMemServerD->si_head),             
             hMemServerD->count_user,             
             hMemServerD->name);
    
    return 0;
}

static int eggRWSMemServerDescriptor_deleteData(HEGGRWSMEMSERVERDESCRIPTOR hMemServerD);
int eggRWSMemServerDescriptor_end(HEGGRWSMEMSERVERDESCRIPTOR hMemServerD)
{
    LOG_INFO("MemServerD", "-end of status [%s] count_user [%d] [%s]",
             statusinstruct_str(&hMemServerD->si_head),
             hMemServerD->count_user,
             hMemServerD->name);

    LOG_INFO("MemServerD", "stop thread [%lu] [%s]", hMemServerD->threadid,
             hMemServerD->name);
    eggRWSMemClient_stop(hMemServerD);
    
    eggRWSMemServerManager_delMemServer_nolock(hMemServerD->hManager, hMemServerD);
    eggRWSMemServerDescriptor_deleteData(hMemServerD);
    eggRWSMemServerDescriptor_release(hMemServerD);
    free(hMemServerD);

    return 0;
}

int eggRWSMemServerDescriptor_makeFlush(HEGGRWSMEMSERVERDESCRIPTOR hMemServerD)
{
    if (eggRWSMemServerDescriptor_is_memServerCreator(hMemServerD))
    {
        return 0;
    }

    STATUSINSTRUCT *psi;
    psi = &hMemServerD->si_head;
    int stat;
    stat = eggRWSMemServerStat_getStat(psi->stat);

    psi->timeToLive = 0;    
    if (stat == EGGRWSMEMSERVERSTAT_UNFORKED)
    {
        eggRWSMemServerStat_clear_writeable(psi->stat);
        eggRWSMemServerStat_clear_readable(psi->stat);
        eggRWSMemServerStat_clear_retry(psi->stat);
        
        psi->trigger = eggRWSMemServerDescriptor_dumping;
    }
    else if (stat == EGGRWSMEMSERVERSTAT_NORMAL)
    {
        eggRWSMemServerStat_clear_writeable(psi->stat);
        eggRWSMemServerStat_clear_readable(psi->stat);
        eggRWSMemServerStat_clear_retry(psi->stat);
        
        psi->trigger = eggRWSMemServerDescriptor_dumping;
    }

    eggRWSMemServerManager_setupTimeout_memServer_nolock(hMemServerD->hManager,
                                                         hMemServerD);
    
    return 0;
}

static int delete_directory(char *dirname)
{
    char *cmd;
    cmd = malloc(strlen(dirname) + 30);
    assert(cmd);
    sprintf(cmd, "rm -r %s", dirname);
    int r;
    r = system(cmd);
    free(cmd);
    return r;
}

static int eggRWSMemServerDescriptor_deleteData(HEGGRWSMEMSERVERDESCRIPTOR hMemServerD)
{
    eggRWSMemServerDescriptor_closeClient(hMemServerD);
    if (hMemServerD->pid > 0)
    {
        kill(hMemServerD->pid, SIGKILL);
        LOG_INFO("MemServerD", "kill pid[%d] [%s]", (int)hMemServerD->pid, hMemServerD->name);
        waitpid(hMemServerD->pid, NULL, 0);
        hMemServerD->pid = 0;
    }
    char *eggdir;
    eggdir = malloc(strlen(hMemServerD->dir) + strlen(hMemServerD->name) + 2);
    assert(eggdir);
    sprintf(eggdir, "%s/%s", hMemServerD->dir, hMemServerD->name);
    delete_directory(eggdir);
    free(eggdir);
    
    return 0;
    
}

int eggRWSMemServerDescriptor_isNameMatch(char *name)
{
    if (!name)
    {
        return 0;               /* false */
    }

    /* year4+month2+day2+hour2+min2 */
    if (strlen(name) != 12)
    {
        return 0;
    }
    int y = 0, m = 0, d = 0, h = 0, min = 0;
    sscanf(name, "%04d%02d%02d%02d%02d", &y, &m, &d, &h, &min);
    if (y < 1900)
    {
        return 0;
    }
    if (!(1 <= m && m <= 12))
    {
        return 0;
    }
    if (!(1 <= d && d <= 31))
    {
        return 0;
    }
    if (!(0 <= h && h <= 23))
    {
        return 0;
    }
    if (!(0 <= min && min <= 59))
    {
        return 0;
    }
    return 1;
    
}


static EGGRWSMEMSERVERID eggRWSMemServerDescriptor_castToId(char *name)
{
    EGGRWSMEMSERVERID id;
    struct tm tm = {};
    int y = 0, m = 0, d = 0, h = 0, min = 0;
    sscanf(name, "%04d%02d%02d%02d%02d", &y, &m, &d, &h, &min);
    tm.tm_year = y - 1900;
    tm.tm_mon = m - 1;
    tm.tm_mday = d;
    tm.tm_hour = h;
    tm.tm_min = min;
    id = mktime(&tm);
    return id;
}

int eggRWSMemServerDescriptor_cmpName(HEGGRWSMEMSERVERDESCRIPTOR p, HEGGRWSMEMSERVERDESCRIPTOR q)
{
    return strcmp(p->name, q->name);
}
int eggRWSMemServerDescriptor_cmpTimeout(HEGGRWSMEMSERVERDESCRIPTOR p, HEGGRWSMEMSERVERDESCRIPTOR q)
{
    if (p->timeout.tv_sec < q->timeout.tv_sec)
        return -1;
    if (p->timeout.tv_sec > q->timeout.tv_sec)
        return 1;
    if (p->timeout.tv_usec < p->timeout.tv_usec)
        return -1;
    if (p->timeout.tv_usec > p->timeout.tv_usec)
        return 1;
    return 0;
}

int eggRWSMemServerDescriptor_initialise(HEGGRWSMEMSERVERDESCRIPTOR hMemServerD,
                                     struct eggRWSMemServerManager *hManager)
{
    char *p;
    int n;
    if (hMemServerD->id == 0)
    {
        hMemServerD->id = eggRWSMemServerDescriptor_castToId(hMemServerD->name);
    }
    
    hMemServerD->hManager = hManager;
    hMemServerD->dir = hManager->memServerD_head.name;
    
    hMemServerD->analyzerName = strdup("weightfield");
    assert(hMemServerD->analyzerName);

    n = 2 *(strlen(hMemServerD->dir) + 1 + strlen(hMemServerD->name)) + 100;
    p = malloc(n); assert(p);

    snprintf(p, n, "%s/%s/.dumpProgress", hMemServerD->dir, hMemServerD->name);
    hMemServerD->fileName_dumpProgress = strdup(p); assert(hMemServerD->fileName_dumpProgress);
    
    hMemServerD->dumpExeName = hManager->memServerD_head.dumpExeName;
    
    snprintf(p, n, "%s/%s/egg.sock", hMemServerD->dir, hMemServerD->name);
    hMemServerD->unixsock = strdup(p); assert(hMemServerD->unixsock);

    char **argv = calloc(8, sizeof(char*));
    argv[0] = strdup(hManager->memServerD_head.exeName); assert(argv[0]);
    argv[1] = strdup("--unixsock"); assert(argv[1]);
    argv[2] = strdup(hMemServerD->unixsock);
    snprintf(p, n, "%s/%s/", hMemServerD->dir, hMemServerD->name);
    mkdir(p, 0755);
    argv[3] = strdup(p); assert(argv[3]);
    argv[4] = strdup(hMemServerD->analyzerName); assert(argv[4]);
    argv[5] = strdup("--force"); assert(argv[5]);
    hMemServerD->forkArgv = argv;
    
    snprintf(p, n, "unixsock://%s:/%%%%%%%s/%s/", hMemServerD->unixsock,
             hMemServerD->dir, hMemServerD->name);
    hMemServerD->eggPath = p;

    hMemServerD->si_head.belongto = hMemServerD;

    LOG_INFO("MemServerD", "come [%s]", hMemServerD->name);
    return 0;
}

int eggRWSMemServerDescriptor_release(HEGGRWSMEMSERVERDESCRIPTOR hMemServerD)
{
    
    LOG_INFO("MemServerD", "gone [%s]", hMemServerD->name);

    free(hMemServerD->name);    
    free(hMemServerD->analyzerName);
    free(hMemServerD->fileName_dumpProgress);
    free(hMemServerD->unixsock);
    char **p;
    p = hMemServerD->forkArgv;
    if (p)
        for (; *p; p++)
            free(*p);
    free(hMemServerD->forkArgv);
    free(hMemServerD->eggPath);
    
    STATUSINSTRUCT *psi, *psi_tmp;
    psi = hMemServerD->si_head.next;
    while (psi)
    {
        psi_tmp = psi;
        psi = psi->next;
        free(psi_tmp);
    }

    return 0;
}

static int eggRWSMemServerDescriptor_doCreateMemServer(HEGGRWSMEMSERVERDESCRIPTOR p_memServerCreator)
{
    struct eggRWSMemServerManager *hManager;
    hManager = p_memServerCreator->hManager;

    HEGGRWSMEMSERVERDESCRIPTOR p_msd;
    p_msd = calloc(1, sizeof(*p_msd));
    assert(p_msd);
    p_msd->id = p_memServerCreator->id;
    p_msd->name = timeval_getRecentInstanceString(hManager->age_newMemServer_minutes,
                                                  p_msd->id);
    eggRWSMemServerDescriptor_initialise(p_msd, hManager);
    statusinstruct_set(&p_msd->si_head, EGGRWSMEMSERVERSTAT_NORMAL, hManager->age_newMemServer_minutes * 60);
    eggRWSMemServerDescriptor_prependStatInstruct(p_msd, EGGRWSMEMSERVERSTAT_UNFORKED, 0);
    eggRWSMemServerManager_addMemServer_nolock(hManager, p_msd);
    eggRWSMemServerManager_setupTimeout_memServer_nolock(hManager, p_msd);
    
    p_memServerCreator->id += hManager->age_newMemServer_minutes * 60;
    p_memServerCreator->si_head.timeToLive = hManager->age_newMemServer_minutes * 60;
    eggRWSMemServerManager_setupTimeout_memServer_nolock(hManager, p_memServerCreator);
    
    return 0;

}

int eggRWSMemServerDescriptor_initialise_memServerCreator(HEGGRWSMEMSERVERDESCRIPTOR hMemServerD,
                                                          struct eggRWSMemServerManager *hManager)
{
    
    hMemServerD->name = strdup("memServerCreator");
    hMemServerD->id = (EGGRWSMEMSERVERID)timeval_getRecentInstanceId(hManager->age_newMemServer_minutes,
                                                                     hManager->now.tv_sec);
    hMemServerD->hManager = hManager;
    
    hMemServerD->si_head.belongto = hMemServerD;
    hMemServerD->si_head.timeToLive = 0;
    hMemServerD->si_head.trigger = eggRWSMemServerDescriptor_doCreateMemServer;

    return 0;
}

int eggRWSMemServerDescriptor_is_memServerCreator(HEGGRWSMEMSERVERDESCRIPTOR hMemServerD)
{
    if (!hMemServerD)
        return 0;
    if (strcmp(hMemServerD->name, "memServerCreator") == 0)
    {
        return 1;
    }
    return 0;
}


int eggRWSMemServerDescriptor_openClient(HEGGRWSMEMSERVERDESCRIPTOR hMemServerD)
{
    hMemServerD->hEggHandle = eggPath_open(hMemServerD->eggPath);
    hMemServerD->hWriter = eggIndexWriter_open(hMemServerD->hEggHandle,
					       hMemServerD->analyzerName);
    hMemServerD->hReader = eggIndexWriter_init_reader(hMemServerD->hWriter);
    hMemServerD->hSearcher = eggIndexSearcher_new(hMemServerD->hReader);
    if (!(hMemServerD->hEggHandle && hMemServerD->hWriter
          && hMemServerD->hReader && hMemServerD->hSearcher))
    {
        return -1;
    }
    pthread_mutex_init(&hMemServerD->thread_input_mutex, NULL);
    pthread_cond_init(&hMemServerD->thread_input_cond, NULL);
    if (pthread_create(&hMemServerD->threadid, NULL, eggRWSMemClient, hMemServerD) < 0)
    {
        fprintf(stderr, "%s:%d:%s pthread_create ERR %s\n",
                __FILE__, __LINE__, __func__, strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    return 0;
    
}

int eggRWSMemServerDescriptor_closeClient(HEGGRWSMEMSERVERDESCRIPTOR hMemServerD)
{
    if (hMemServerD->threadid == 0)
    {
        return 0;
    }
    
    pthread_join(hMemServerD->threadid, NULL);
    hMemServerD->threadid = 0;
    
    pthread_mutex_destroy(&hMemServerD->thread_input_mutex);
    pthread_cond_destroy(&hMemServerD->thread_input_cond);
    
    eggIndexSearcher_delete(hMemServerD->hSearcher);
    hMemServerD->hSearcher = NULL;
    eggIndexReader_free(hMemServerD->hReader);
    hMemServerD->hReader = NULL;
    eggIndexWriter_close(hMemServerD->hWriter);
    hMemServerD->hWriter = NULL;
    eggPath_close(hMemServerD->hEggHandle);
    hMemServerD->hEggHandle = NULL;
    
    return 0;
}


int eggRWSMemServerDescriptor_userIncrease(HEGGRWSMEMSERVERDESCRIPTOR hMemServerD)
{
    if (!hMemServerD)
    {
        return 0;
    }
    pthread_mutex_lock(&hMemServerD->thread_input_mutex);
    hMemServerD->count_user++;
    pthread_mutex_unlock(&hMemServerD->thread_input_mutex);
    return 0;
}
int eggRWSMemServerDescriptor_userDecrease(HEGGRWSMEMSERVERDESCRIPTOR hMemServerD)
{
    if (!hMemServerD)
    {
        return 0;
    }
    
    pthread_mutex_lock(&hMemServerD->thread_input_mutex);
    hMemServerD->count_user--;
    pthread_mutex_unlock(&hMemServerD->thread_input_mutex);

    eggRWSMemServerManager_signalRetry(hMemServerD->hManager);

    return 0;
}


int eggRWSMemServerDescriptor_prependStatInstruct(HEGGRWSMEMSERVERDESCRIPTOR hMemServerD,
                                                  int newstat, time_t timeToLive)
{
    STATUSINSTRUCT *psi_old;
    STATUSINSTRUCT *psi_new = &hMemServerD->si_head;
    
    psi_old = malloc(sizeof(*psi_old));
    assert(psi_old);
    *psi_old = *psi_new;
    psi_new->next = psi_old;

    psi_new->stat = 0;
    statusinstruct_set(psi_new, newstat, timeToLive);
    
    return 0;
}
int eggRWSMemServerDescriptor_popStatInstruct(HEGGRWSMEMSERVERDESCRIPTOR hMemServerD)
{
    STATUSINSTRUCT *psi;
    psi = hMemServerD->si_head.next;
    hMemServerD->si_head = *psi;
    free(psi);
    return 0;
}

enum {THREAD_CONTROL_FLAG_stop = 1};
int eggRWSMemClient_stop(HEGGRWSMEMSERVERDESCRIPTOR hMemServerD)
{
    if (hMemServerD->threadid == 0)
    {
        return 0;
    }
    hMemServerD->thread_control_flag = THREAD_CONTROL_FLAG_stop;
    pthread_cond_signal(&hMemServerD->thread_input_cond);
    return 0;
}

int eggRWSMemClient_pushJob(HEGGRWSMEMSERVERDESCRIPTOR *pp_msd,
                                      int n_msd,
                                      EGGRWSJOBSPEC *p_job)
{
    HEGGRWSMEMSERVERDESCRIPTOR hMemServerD;

    int i;
    for (i = 0; i < n_msd; i++)
    {
        hMemServerD = pp_msd[i];
        
        pthread_mutex_lock(&hMemServerD->thread_input_mutex);
        EGGRWSLISTITEM *p_listitem = calloc(1, sizeof(EGGRWSLISTITEM));
        assert(p_listitem);
        p_listitem->data = p_job;
        fifolist_push(&hMemServerD->thread_input, p_listitem);
        if (hMemServerD->thread_input.count > 0)
        {
            pthread_cond_signal(&hMemServerD->thread_input_cond);
        }
        pthread_mutex_unlock(&hMemServerD->thread_input_mutex);
        
    }
    return 0;
}

EGGRWSJOBSPEC *eggRWSMemClient_popJob(HEGGRWSMEMSERVERDESCRIPTOR hMemServerD)
{
    EGGRWSJOBSPEC *p_job = NULL;
    pthread_mutex_lock(&hMemServerD->thread_input_mutex);

    while (hMemServerD->thread_control_flag != THREAD_CONTROL_FLAG_stop)
    {
        EGGRWSLISTITEM *p_listitem = NULL;
        fifolist_pop(&hMemServerD->thread_input, p_listitem);
        if (p_listitem)
        {
            p_job = p_listitem->data;
            free(p_listitem);
            break;

        }
        pthread_cond_wait(&hMemServerD->thread_input_cond,
                          &hMemServerD->thread_input_mutex);
    }

    pthread_mutex_unlock(&hMemServerD->thread_input_mutex);
    return p_job;
}

static void *eggRWSMemClient(void *arg)
{
    LOG_INFO("MemServerClient", "start");

    HEGGRWSMEMSERVERDESCRIPTOR hMemServerD = (HEGGRWSMEMSERVERDESCRIPTOR)arg;

    for (; ;)
    {
        EGGRWSJOBSPEC *p_job;
        p_job = eggRWSMemClient_popJob(hMemServerD);
        if (!p_job)
        {
            break;
        }
        
        HEGGNETPACKAGE hNetPackage;
        HEGGNETPACKAGE lp_res_package;

        
        hNetPackage = (HEGGNETPACKAGE)EGGRWSJOBSPEC_getMemJobInput(p_job, hMemServerD);
        lp_res_package = eggRWSMemClient_processing(hMemServerD, hNetPackage);
        EGGRWSJOBSPEC_yieldMemJobOutput(p_job, lp_res_package, hMemServerD);
    }
    
    LOG_INFO("MemServerClient", "end");
    return NULL;
}

static HEGGNETPACKAGE eggRWSMemClient_processing_loadEgg(HEGGRWSMEMSERVERDESCRIPTOR hMemServerD, HEGGNETPACKAGE hNetPackage);
static HEGGNETPACKAGE eggRWSMemClient_processing_query_documents(HEGGRWSMEMSERVERDESCRIPTOR hMemServerD, HEGGNETPACKAGE hNetPackage);
static HEGGNETPACKAGE eggRWSMemClient_processing_query_documents_with_sort(HEGGRWSMEMSERVERDESCRIPTOR hMemServerD, HEGGNETPACKAGE hNetPackage);
static HEGGNETPACKAGE eggRWSMemClient_processing_filter_documents(HEGGRWSMEMSERVERDESCRIPTOR hMemServerD, HEGGNETPACKAGE hNetPackage);
static HEGGNETPACKAGE eggRWSMemClient_processing_query_documents_with_iter(HEGGRWSMEMSERVERDESCRIPTOR hMemServerD, HEGGNETPACKAGE hNetPackage);

static HEGGNETPACKAGE eggRWSMemClient_processing(HEGGRWSMEMSERVERDESCRIPTOR hMemServerD, HEGGNETPACKAGE hNetPackage)
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
        lp_res_package = eggRWSMemClient_processing_loadEgg(hMemServerD, hNetPackage);
        return lp_res_package;
	
    case EGG_PACKAGE_SEARCH:
        lp_res_package = eggRWSMemClient_processing_query_documents(hMemServerD, hNetPackage);
        return lp_res_package;
        
    case EGG_PACKAGE_SEARCH_SORT:
        lp_res_package = eggRWSMemClient_processing_query_documents_with_sort(hMemServerD, hNetPackage);
        return lp_res_package;	
        
    case EGG_PACKAGE_SEARCHFILTER:
        lp_res_package = eggRWSMemClient_processing_filter_documents(hMemServerD, hNetPackage);
        return lp_res_package;	
        
    case EGG_PACKAGE_SEARCH_ITER:
        lp_res_package = eggRWSMemClient_processing_query_documents_with_iter(hMemServerD, hNetPackage);
        return lp_res_package;
	
    }

    HEGGNETSERVER hNetServer;
    hNetServer = eggNetServer_new (NULL, NULL, NULL, NULL);
    hNetServer->hSearcher = hMemServerD->hSearcher;
    hNetServer->hReader = hMemServerD->hReader;
    hNetServer->hWriter = hMemServerD->hWriter;
    
    lp_res_package = eggNetServer_processing(hNetServer, hNetPackage);
    eggNetServer_delete(hNetServer);

    return lp_res_package;
}

static HEGGNETPACKAGE eggRWSMemClient_processing_loadEgg(HEGGRWSMEMSERVERDESCRIPTOR hMemServerD,
                                                       HEGGNETPACKAGE hNetPackage)
{
    HEGGNETPACKAGE lp_res_package = eggNetPackage_new(EGG_PACKAGE_LOADEGG);
    EBOOL ret;

    int n_path_len = 0;
    char* lp_path = EGG_NULL;
    eggNetPackage_fetch(hNetPackage, 2, &lp_path, &n_path_len);
    fprintf(stderr, "%s:%d:%s loadEgg Not support\n",
            __FILE__, __LINE__, __func__);
    ret = EGG_TRUE;
    lp_res_package = eggNetPackage_add(lp_res_package, &ret, sizeof(ret), EGG_PACKAGE_RET);
    return lp_res_package;
}

static HEGGNETPACKAGE eggRWSMemClient_processing_query_documents(HEGGRWSMEMSERVERDESCRIPTOR hMemServerD, HEGGNETPACKAGE hNetPackage)
{
    HEGGNETPACKAGE p_res_package;
    HEGGNETSERVER hNetServer;
    hNetServer = eggNetServer_new (NULL, NULL, NULL, NULL);
    hNetServer->hSearcher = hMemServerD->hSearcher;
    hNetServer->hReader = hMemServerD->hReader;
    hNetServer->hWriter = hMemServerD->hWriter;
    
    p_res_package = eggNetServer_processing(hNetServer, hNetPackage);
    eggNetServer_delete(hNetServer);

    return p_res_package;

}
static HEGGNETPACKAGE eggRWSMemClient_processing_query_documents_with_sort(HEGGRWSMEMSERVERDESCRIPTOR hMemServerD, HEGGNETPACKAGE hNetPackage)
{
    HEGGNETPACKAGE p_res_package;
    
    HEGGNETSERVER hNetServer;
    hNetServer = eggNetServer_new (NULL, NULL, NULL, NULL);
    hNetServer->hSearcher = hMemServerD->hSearcher;
    hNetServer->hReader = hMemServerD->hReader;
    hNetServer->hWriter = hMemServerD->hWriter;
    
    p_res_package = eggNetServer_processing(hNetServer, hNetPackage);
    eggNetServer_delete(hNetServer);
    
    
    HEGGTOPCOLLECTOR p_top_collector = NULL;
    EBOOL *p_ret = EGG_NULL;
    size32_t retsz = 0;
    char *collectorbuf = NULL;
    int collectorsize = 0;

    eggNetPackage_fetch((HEGGNETPACKAGE)p_res_package, 4, &p_ret, &retsz,
                        &collectorbuf, &collectorsize);
    
    if(*p_ret == EGG_TRUE)
    {
        p_top_collector = eggTopCollector_unserialization((HEGGTOPCTORCHUNK)collectorbuf);

        HEGGNETPACKAGE p_res_package_real = eggNetPackage_new(EGG_PACKAGE_SEARCH);
    
        p_res_package_real = eggNetPackage_add(p_res_package_real, p_ret, sizeof(*p_ret), EGG_PACKAGE_RET);
        
        eggTopCollector_set_chunkid_cluster(p_top_collector, hMemServerD->id);
        
        HEGGTOPCTORCHUNK lp_topctor_chunk = eggTopCollector_serialization(p_top_collector);
        p_res_package_real = eggNetPackage_add(p_res_package_real, lp_topctor_chunk, lp_topctor_chunk->size, EGG_PACKAGE_TOPCOLLECTOR);

        eggTopCollector_delete(p_top_collector);
        free(lp_topctor_chunk);
        eggNetPackage_delete(p_res_package);        
        p_res_package = p_res_package_real;
    }
    
    return p_res_package;
    
}
static HEGGNETPACKAGE eggRWSMemClient_processing_filter_documents(HEGGRWSMEMSERVERDESCRIPTOR hMemServerD, HEGGNETPACKAGE hNetPackage)
{
    HEGGNETPACKAGE p_res_package;
    
    HEGGNETSERVER hNetServer;
    hNetServer = eggNetServer_new (NULL, NULL, NULL, NULL);
    hNetServer->hSearcher = hMemServerD->hSearcher;
    hNetServer->hReader = hMemServerD->hReader;
    hNetServer->hWriter = hMemServerD->hWriter;
    
    p_res_package = eggNetServer_processing(hNetServer, hNetPackage);
    eggNetServer_delete(hNetServer);
    
    
    HEGGTOPCOLLECTOR p_top_collector = NULL;
    EBOOL *p_ret = EGG_NULL;
    size32_t retsz = 0;
    char *collectorbuf = NULL;
    int collectorsize = 0;

    eggNetPackage_fetch((HEGGNETPACKAGE)p_res_package, 4, &p_ret, &retsz,
                        &collectorbuf, &collectorsize);
    
    if(*p_ret == EGG_TRUE)
    {
        p_top_collector = eggTopCollector_unserialization((HEGGTOPCTORCHUNK)collectorbuf);

        HEGGNETPACKAGE p_res_package_real = eggNetPackage_new(EGG_PACKAGE_SEARCH);
    
        p_res_package_real = eggNetPackage_add(p_res_package_real, p_ret, sizeof(*p_ret), EGG_PACKAGE_RET);
        
        eggTopCollector_set_chunkid_cluster(p_top_collector, hMemServerD->id);
        
        HEGGTOPCTORCHUNK lp_topctor_chunk = eggTopCollector_serialization(p_top_collector);
        p_res_package_real = eggNetPackage_add(p_res_package_real, lp_topctor_chunk, lp_topctor_chunk->size, EGG_PACKAGE_TOPCOLLECTOR);

        free(lp_topctor_chunk);
        eggNetPackage_delete(p_res_package);        
        p_res_package = p_res_package_real;
    }
    
    return p_res_package;
}
static HEGGNETPACKAGE eggRWSMemClient_processing_query_documents_with_iter(HEGGRWSMEMSERVERDESCRIPTOR hMemServerD, HEGGNETPACKAGE hNetPackage)
{
    HEGGNETPACKAGE p_res_package;
    
    HEGGNETSERVER hNetServer;
    hNetServer = eggNetServer_new (NULL, NULL, NULL, NULL);
    hNetServer->hSearcher = hMemServerD->hSearcher;
    hNetServer->hReader = hMemServerD->hReader;
    hNetServer->hWriter = hMemServerD->hWriter;
    
    p_res_package = eggNetServer_processing(hNetServer, hNetPackage);
    eggNetServer_delete(hNetServer);
    
    
    HEGGTOPCOLLECTOR p_top_collector = NULL;
    EBOOL *p_ret = EGG_NULL;
    size32_t retsz = 0;
    char *collectorbuf = NULL;
    int collectorsize = 0;

    eggNetPackage_fetch((HEGGNETPACKAGE)p_res_package, 4, &p_ret, &retsz,
                        &collectorbuf, &collectorsize);
    
    if(*p_ret == EGG_TRUE)
    {
        p_top_collector = eggTopCollector_unserialization((HEGGTOPCTORCHUNK)collectorbuf);

        HEGGNETPACKAGE p_res_package_real = eggNetPackage_new(EGG_PACKAGE_SEARCH);
    
        p_res_package_real = eggNetPackage_add(p_res_package_real, p_ret, sizeof(*p_ret), EGG_PACKAGE_RET);
        
        eggTopCollector_set_chunkid_cluster(p_top_collector, hMemServerD->id);
        
        HEGGTOPCTORCHUNK lp_topctor_chunk = eggTopCollector_serialization(p_top_collector);
        p_res_package_real = eggNetPackage_add(p_res_package_real, lp_topctor_chunk, lp_topctor_chunk->size, EGG_PACKAGE_TOPCOLLECTOR);

        free(lp_topctor_chunk);
        eggNetPackage_delete(p_res_package);
        p_res_package = p_res_package_real;
    }
    
    return p_res_package;
}




void *eggRWSMemServerDescriptor_documentExporter(void *);
int eggRWSMemServerDescriptor_doDumpDocument(HEGGRWSMEMSERVERDESCRIPTOR hMemServerD)
{
    HEGGRWSMEMSERVERMANAGER hMemServerManager = hMemServerD->hManager;
    HEGGRWSINTSERVER hIntServer = hMemServerManager->hIntServer;
    HEGGRWSBAKERMANAGER hRWSBakerManager = hIntServer->hBakerManager;

    char *fileName_dumpProgress;
    fileName_dumpProgress = hMemServerD->fileName_dumpProgress;
    FILE *fp_dumpProgress;
    fp_dumpProgress = fopen(fileName_dumpProgress, "w");
    assert(fp_dumpProgress);

    /* 将所有待同步的Baker存至文件 */
    HEGGRWSBAKERINFO* p_bakerInfos;
    count_t i;
    count_t cnt_bakerInfo;
    p_bakerInfos = eggRWSBakerManager_getBakerInfo(hRWSBakerManager, &cnt_bakerInfo);
    for (i = 0; i < cnt_bakerInfo; i++)
    {
	HEGGRWSBAKER lp_baker;
	lp_baker = p_bakerInfos[i]->baker;

        eggBakerId_t id_baker;
        id_baker = lp_baker->bakIdx;
        char *eggPath_baker;
        eggPath_baker = hRWSBakerManager->hBakTab->infos[id_baker]->path;
        
        fprintf(fp_dumpProgress, "%d %s\n", (int)id_baker, eggPath_baker);
        
    }
    
    fclose(fp_dumpProgress);
    
    pthread_t threadid;
    pthread_attr_t threadattr;
    pthread_attr_init(&threadattr);
    pthread_attr_setdetachstate(&threadattr, PTHREAD_CREATE_DETACHED);
    pthread_create(&threadid, &threadattr, eggRWSMemServerDescriptor_documentExporter, hMemServerD);
    
    return 0;
}

void *eggMemExport(void *arg);
void *eggRWSMemServerDescriptor_documentExporter(void *arg)
{
    HEGGRWSMEMSERVERDESCRIPTOR hMemServerD = (HEGGRWSMEMSERVERDESCRIPTOR)arg;
    
    HEGGRWSMEMSERVERMANAGER hMemServerManager = hMemServerD->hManager;
    HEGGRWSINTSERVER hIntServer = hMemServerManager->hIntServer;
    HEGGRWSBAKERMANAGER hRWSBakerManager = hIntServer->hBakerManager;
    int i;
    char *cmd;
    int size_cmd;
    size_cmd = strlen(hMemServerD->dumpExeName) + 2 *(strlen(hMemServerD->dir) + strlen(hMemServerD->name)) + 1 * strlen(hMemServerD->analyzerName) + 2500 + 100;
    cmd = malloc(size_cmd);
    assert(cmd);

    char *fileName_dumpProgress;
    fileName_dumpProgress = hMemServerD->fileName_dumpProgress;


    LOG_INFO("documentExporter", "[%s] beg", hMemServerD->name);

    
    {
        size64_t sz_packBuf;
        
        char *eggpath = cmd;
        sprintf(eggpath, "%s/%s/", hMemServerD->dir, hMemServerD->name);

        HEGGRWSPACKBUF hRWSPackBuf = eggRWSPackBuf_new(eggpath);
        sz_packBuf = eggRWSPackBuf_get_fsize(hRWSPackBuf);
        eggRWSPackBuf_delete(hRWSPackBuf);
        LOG_INFO("documentExporter", "size_doc[%llu] [%s]", (unsigned long long)sz_packBuf, hMemServerD->name);

        if (sz_packBuf == 0)
        {
            goto end;
        }
    }

    
    FILE *fp;
    fp = fopen(fileName_dumpProgress, "r");


    int count_bakerId = 0;
    char **p_eggPath_baker = NULL;
    eggBakerId_t *p_bakerId = NULL;
    
    size_t n_line_size = 0;
    char* lp_line_buf = NULL;
    while (getline(&lp_line_buf, &n_line_size, fp) != -1)
    {
        count_bakerId++;
    }
    
    if (count_bakerId == 0)
    {
        free(lp_line_buf);
        fclose(fp);
        goto end;
    }
    
    p_eggPath_baker = malloc(count_bakerId *sizeof(*p_eggPath_baker));
    assert(p_eggPath_baker);
    p_bakerId = malloc(count_bakerId *sizeof(*p_bakerId));
    assert(p_bakerId);

    i = 0;
    fseek(fp, 0, SEEK_SET);
    while (getline(&lp_line_buf, &n_line_size, fp) != -1)
    {
        char *p, *q;
        p = lp_line_buf;
        
        eggBakerId_t id_baker;

        id_baker = strtol(p, &q, 0);

        while(isspace(*q))
        {
            q++;
        }
        p = q;
        while (*q && !isspace(*q))
        {
            q++;
        }
        *q = '\0';

        p_bakerId[i] = id_baker;
        p_eggPath_baker[i] = strdup(p);
        assert(p_eggPath_baker[i]);

        i++;
    }

    free(lp_line_buf);
    fclose(fp);

    int count_batch_export;
    count_batch_export = count_bakerId / 2;    

    i = 0;
    {
        int j;
        HEGGRWSBAKERGROUP lp_bakers;        
        
        eggRWSBakerManager_moveBakersByIds(hRWSBakerManager, p_bakerId, count_batch_export);
        eggRWSBakerManager_waitWBakers(hRWSBakerManager);
        lp_bakers = eggRWSBakerManager_getWGroup(hRWSBakerManager);


        pthread_t *p_threadid_exe;
        p_threadid_exe = malloc(count_batch_export * sizeof(pthread_t));
        assert(p_threadid_exe);

        struct timeval start, end;
        gettimeofday(&start, NULL);

        for (j = 0; j < count_batch_export; j++)
        {
            //snprintf(cmd, size_cmd, "exec %s --continue --workdir %s/%s %s/%s/ %s %s",
            snprintf(cmd, size_cmd, "cd /tmp/; ulimit -c unlimited; echo %s --workdir %s/%s %s/%s/ %s %s >>/tmp/eggExportDoc.1.log; exec %s --workdir %s/%s %s/%s/ %s %s >>/tmp/eggExportDoc.1.log 2>>/tmp/eggExportDoc.1.log",
                     hMemServerD->dumpExeName,
                     hMemServerD->dir, hMemServerD->name,
                     hMemServerD->dir, hMemServerD->name,
                     p_eggPath_baker[i + j],
                     hMemServerD->analyzerName,
                     
                     hMemServerD->dumpExeName,
                     hMemServerD->dir, hMemServerD->name,
                     hMemServerD->dir, hMemServerD->name,
                     p_eggPath_baker[i + j],
                     hMemServerD->analyzerName);

            
            char *cmd_thread;
            cmd_thread = strdup(cmd);
            assert(cmd_thread);
            
            pthread_create(&p_threadid_exe[j], NULL, eggMemExport, cmd_thread);
            
        }


        for (j = 0; j < count_batch_export; j++)
        {
            void *ret_thread;
            pthread_join(p_threadid_exe[j], &ret_thread);
            
            
        }

        free(p_threadid_exe);

        gettimeofday(&end, NULL);
        LOG_INFO("documentExporter", "[%s] phase1 export threads count=%d, time: %f",
                 hMemServerD->name,
                 count_batch_export,
                 (end.tv_usec-start.tv_usec)/1000000. + end.tv_sec-start.tv_sec);


    }
    i += count_batch_export;
    count_batch_export = count_bakerId - count_batch_export;

    {
        int j;
        HEGGRWSBAKERGROUP lp_bakers;
        long timestamp;
        
        timestamp = hMemServerD->id;
        eggRWSBakerManager_alter_group(hRWSBakerManager, timestamp);
        eggRWSBakerManager_waitWBakers(hRWSBakerManager);
        lp_bakers = eggRWSBakerManager_getWGroup(hRWSBakerManager);

        pthread_t *p_threadid_exe;
        p_threadid_exe = malloc(count_batch_export * sizeof(pthread_t));
        assert(p_threadid_exe);


        struct timeval start, end;
        gettimeofday(&start, NULL);
        
        for (j = 0; j < count_batch_export; j++)
        {
            //snprintf(cmd, size_cmd, "exec %s --continue --workdir %s/%s %s/%s/ %s %s",
            snprintf(cmd, size_cmd, "cd /tmp/; ulimit -c unlimited; echo %s --workdir %s/%s %s/%s/ %s %s >>/tmp/eggExportDoc.1.log; exec %s --workdir %s/%s %s/%s/ %s %s >>/tmp/eggExportDoc.1.log 2>>/tmp/eggExportDoc.1.log",
                     hMemServerD->dumpExeName,
                     hMemServerD->dir, hMemServerD->name,
                     hMemServerD->dir, hMemServerD->name,
                     p_eggPath_baker[i + j],
                     hMemServerD->analyzerName,
                     
                     hMemServerD->dumpExeName,
                     hMemServerD->dir, hMemServerD->name,
                     hMemServerD->dir, hMemServerD->name,
                     p_eggPath_baker[i + j],
                     hMemServerD->analyzerName);            
            
            char *cmd_thread;
            cmd_thread = strdup(cmd);
            assert(cmd_thread);
            
            pthread_create(&p_threadid_exe[j], NULL, eggMemExport, cmd_thread);

        }

        for (j = 0; j < count_batch_export; j++)
        {
            void *ret_thread;
            pthread_join(p_threadid_exe[j], &ret_thread);
            
        }


        gettimeofday(&end, NULL);
        LOG_INFO("documentExporter", "[%s] phase2 export threads count=%d, time: %f",
                 hMemServerD->name,                 
                 count_batch_export,
                 (end.tv_usec-start.tv_usec)/1000000. + end.tv_sec-start.tv_sec);
        

        free(p_threadid_exe);
        
    }

    for (i = 0; i < count_bakerId; i++)
    {
        free(p_eggPath_baker[i]);
    }
    free(p_eggPath_baker);
    free(p_bakerId);

    eggRWSBakerManager_WToR_Group(hRWSBakerManager);
    
end:

    LOG_INFO("documentExporter", "[%s] end", hMemServerD->name);
    
    free(cmd);
    remove(fileName_dumpProgress);
    eggRWSMemServerDescriptor_userDecrease(hMemServerD);
    
    return NULL;
}

void *eggMemExport(void *arg)
{
    char *cmd = (char *)arg;
    if (!cmd || !cmd[0])
    {
        return NULL;
    }
    
    LOG_INFO("documentExporter", cmd);
    
    int r;
    r = system(cmd);
    char buff[100];
    if (r == -1)
    {
        strerror_r(errno, buff, sizeof(buff));
        LOG_ERR("documentExporter", "%s", buff);
    }
    else
    {
        if (WIFSIGNALED(r))
        {
            LOG_ERR("documentExporter", "receive sigal %d", WTERMSIG(r));
        }
        else
        {
            LOG_INFO("documentExporter", "terminal code %d", WEXITSTATUS(r));
        }
    }
    free(cmd);
    return (void *)r;
}
