#include "eggRWSMemManager.h"
#include "eggRWSMemDptor.h"
#include "eggRWSIntServer.h"
#include "eggRWSLog.h"
#include <dirent.h>
#include <assert.h>
#include <pthread.h>


#define LOG_INFO(...) eggRWSLog_log_line(g_integrator_log, EGGRWSLOG_INFO, "MemManager", __VA_ARGS__)
#define LOG_WARN(...) eggRWSLog_log_line(g_integrator_log, EGGRWSLOG_WARN, "MemManager", __VA_ARGS__)
#define LOG_ERROR(...) eggRWSLog_log_line(g_integrator_log, EGGRWSLOG_ERROR, "MemManager", __VA_ARGS__)
#define LOG_CLAIM(...) eggRWSLog_log_line(g_integrator_log, EGGRWSLOG_CLAIM, "MemManager", __VA_ARGS__)

static int isdir(char *path)
{
    struct stat st;
    
    if (stat(path, &st) < 0)
    {
        return 0;
    }
    return S_ISDIR(st.st_mode);
}

HEGGRWSMEMSERVERMANAGER eggRWSMemServerManager_new(HEGGRWSBAKERCFG hConfig, HEGGRWSINTSERVER hIntServer)
{
    
    char *dirpath;
    dirpath = hConfig->workDir;
    char *memServerExeName = hConfig->memServerExeName;
    char *documentExportExeName = hConfig->documentExportExeName;

    HEGGRWSMEMSERVERMANAGER hManager;    
    if (!memServerExeName || !memServerExeName[0])
    {
        hManager = NULL;
        goto end;
    }
    

    hManager = calloc(1, sizeof(*hManager));
    assert(hManager);
    if (hConfig->memServerAge_minutes > 0)
    {
        hManager->age_newMemServer_minutes = hConfig->memServerAge_minutes;
    }
    else
    {
        hManager->age_newMemServer_minutes = 60 * 24; /* one day */
    }
    if (hConfig->numMemServerMax >= 3)
    {
        hManager->NUM_MEMSERVER_MAX = hConfig->numMemServerMax;
    }
    else
    {
        hManager->NUM_MEMSERVER_MAX = 12;
    }
    
    gettimeofday(&hManager->now, NULL);
    pthread_mutex_init(&hManager->mutex_head, NULL);
    HEGGRWSMEMSERVERDESCRIPTOR hMemServerD_head;
    hMemServerD_head = &hManager->memServerD_head;
    hMemServerD_head->name = realpath(dirpath, NULL);
    if (!hMemServerD_head->name)
    {
        fprintf(stderr, "%s Not exist\n", dirpath);
        assert(hMemServerD_head->name);
    }
    hMemServerD_head->exeName = memServerExeName;
    hMemServerD_head->dumpExeName = documentExportExeName;
    hMemServerD_head->next_name = hMemServerD_head;
    hMemServerD_head->prev_name = hMemServerD_head;
    hMemServerD_head->next_timeout = hMemServerD_head;
    hMemServerD_head->prev_timeout = hMemServerD_head;

    char *p_buff = malloc(PATH_MAX);
    assert(p_buff);
    DIR *dirp = opendir(dirpath);
    struct dirent *dentry;
    while ((dentry = readdir(dirp)))
    {
        if (eggRWSMemServerDescriptor_isNameMatch(dentry->d_name))
        {
            snprintf(p_buff, PATH_MAX, "%s/%s", dirpath, dentry->d_name);
            if (isdir(p_buff))
            {
                HEGGRWSMEMSERVERDESCRIPTOR p_msd;
                p_msd = calloc(1, sizeof(*p_msd));
                assert(p_msd);
                p_msd->name = strdup(dentry->d_name);
                assert(p_msd->name);
                eggRWSMemServerDescriptor_initialise(p_msd, hManager);
                statusinstruct_set(&p_msd->si_head, EGGRWSMEMSERVERSTAT_CLEANUP, 0);
                eggRWSMemServerManager_addMemServer(hManager, p_msd);
            }
        }
    }
    closedir(dirp);
    free(p_buff);    

    HEGGRWSMEMSERVERDESCRIPTOR p_msd = hMemServerD_head->next_name;
    while (p_msd != hMemServerD_head)
    {
        eggRWSMemServerManager_setupTimeout_memServer_nolock(hManager, p_msd);
        
        p_msd = p_msd->next_name;
    }
    
    hManager->hIntServer = hIntServer;
    pthread_mutex_init(&hManager->mutex_dumping, NULL);
end:
    hIntServer->hMemServerManager = hManager;
    return hManager;

}

int eggRWSMemServerManager_delete(HEGGRWSMEMSERVERMANAGER hManager)
{
    if (!hManager)
    {
        return 0;
    }


    HEGGRWSMEMSERVERDESCRIPTOR pmsd, pmsd_end, pmsd_tmp;
    
    pmsd = hManager->memServerD_head.next_name;
    pmsd_end = &hManager->memServerD_head;
    while (pmsd != pmsd_end)
    {
        pmsd_tmp = pmsd;
        pmsd = pmsd->next_name;
        
        eggRWSMemServerDescriptor_release(pmsd_tmp);
        free(pmsd_tmp);
    }
    
    eggRWSMemServerDescriptor_release(&hManager->memServerD_head);

    pthread_mutex_destroy(&hManager->mutex_head);
    pthread_mutex_destroy(&hManager->mutex_dumping);
    
    free(hManager);

    return 0;

}

int eggRWSMemServerManager_askDump(HEGGRWSMEMSERVERMANAGER hManager, HEGGRWSMEMSERVERDESCRIPTOR hMemServerD)
{
    if (!hManager)
    {
        return 0;
    }
    int rv;
    
    pthread_mutex_lock(&hManager->mutex_dumping);
    if (!hManager->p_dumping)
    {
        HEGGRWSMEMSERVERDESCRIPTOR p_msd;
        p_msd = eggRWSMemServerManager_getNextDumpingMemServer_nolock(hManager);
        assert(p_msd);
        /* dump by name */
        if (p_msd == hMemServerD)
        {
            hManager->p_dumping = hMemServerD;
            rv = 1;
        }
        else
        {
            rv = 0;
        }
    }
    else if (hManager->p_dumping == hMemServerD)
    {
        rv = 1;
    }
    else
    {
        rv = 0;
    }
    
    pthread_mutex_unlock(&hManager->mutex_dumping);

    return rv;
}

int eggRWSMemServerManager_unaskDump(HEGGRWSMEMSERVERMANAGER hManager, HEGGRWSMEMSERVERDESCRIPTOR hMemServerD)
{
    if (!hManager)
    {
        return 0;
    }

    pthread_mutex_lock(&hManager->mutex_dumping);
    if (hManager->p_dumping == hMemServerD)
    {
        hManager->p_dumping = NULL;

        eggRWSMemServerManager_signalRetry_nolock(hManager);

    }
    pthread_mutex_unlock(&hManager->mutex_dumping);
    
    return 0;
}

int eggRWSMemServerManager_isNotClean(HEGGRWSMEMSERVERMANAGER hManager)
{
    if (!hManager)
    {
        return 0;
    }

    pthread_mutex_lock(&hManager->mutex_head);
    
    if (hManager->nMemServer <= 0)
    {
        pthread_mutex_unlock(&hManager->mutex_head);
        return 0;
    }

    HEGGRWSMEMSERVERDESCRIPTOR pmsd;
    HEGGRWSMEMSERVERDESCRIPTOR pmsd_end;
    pmsd_end = &hManager->memServerD_head;
    
    pmsd = hManager->memServerD_head.next_name;
    while(pmsd != pmsd_end)
    {
        if (eggRWSMemServerDescriptor_is_memServerCreator(pmsd))
        {
            pthread_mutex_unlock(&hManager->mutex_head);
	    return 0;
        }
        pmsd = pmsd->next_name;
    }

    pthread_mutex_unlock(&hManager->mutex_head);
    return 1;

}

int eggRWSMemServerManager_isTooHeavy(HEGGRWSMEMSERVERMANAGER hMemServerManager)
{
    if (!hMemServerManager)
    {
        return 0;
    }
    if (hMemServerManager->nMemServer < hMemServerManager->NUM_MEMSERVER_MAX)
    {
        return 0;
    }
    
    pthread_mutex_lock(&hMemServerManager->mutex_head);
    
    HEGGRWSMEMSERVERDESCRIPTOR pmsd, pmsd_end;
    pmsd = hMemServerManager->memServerD_head.prev_name;
    pmsd_end = &hMemServerManager->memServerD_head;
    while (pmsd != pmsd_end)
    {
        if (!eggRWSMemServerDescriptor_is_memServerCreator(pmsd))
        {
            if (eggRWSMemServerStat_is_writeable(pmsd->si_head.stat))
            {
                pmsd->si_head.timeToLive = 0;
                break;
            }
        }
        pmsd = pmsd->prev_name;
    }
    
    pthread_mutex_unlock(&hMemServerManager->mutex_head);

    LOG_CLAIM("Too Heavy have nMemServer[%d] >= %d",
              hMemServerManager->nMemServer, hMemServerManager->NUM_MEMSERVER_MAX);
    return 1;
    
}

struct timeval eggRWSMemServerManager_getTime(HEGGRWSMEMSERVERMANAGER hMemServerManager)
{
    struct timeval null = {};
    if (!hMemServerManager)
        return null;
    return hMemServerManager->now;
}
int eggRWSMemServerManager_setTime(HEGGRWSMEMSERVERMANAGER hMemServerManager, struct timeval now)
{
    if (!hMemServerManager)
        return -1;
    hMemServerManager->now = now;
    return 0;
}


HEGGRWSMEMSERVERDESCRIPTOR eggRWSMemServerManager_new_memServerCreator(HEGGRWSMEMSERVERMANAGER hManager)
{
    if (!hManager)
    {
        return NULL;
    }
    
    HEGGRWSMEMSERVERDESCRIPTOR p_msd;
    
    p_msd = calloc(1, sizeof(*p_msd));
    assert(p_msd);
    eggRWSMemServerDescriptor_initialise_memServerCreator(p_msd, hManager);
    eggRWSMemServerManager_addMemServer(hManager, p_msd);
    eggRWSMemServerManager_setupTimeout_memServer(hManager, p_msd);
    return p_msd;
}

int eggRWSMemServerManager_delete_memServerCreator(HEGGRWSMEMSERVERMANAGER hManager)
{


    pthread_mutex_lock(&hManager->mutex_head);
    
    HEGGRWSMEMSERVERDESCRIPTOR pmsd;
    HEGGRWSMEMSERVERDESCRIPTOR pmsd_end;
    pmsd_end = &hManager->memServerD_head;
    
    pmsd = hManager->memServerD_head.next_name;
    while(pmsd != pmsd_end)
    {
        if (eggRWSMemServerDescriptor_is_memServerCreator(pmsd))
        {
            eggRWSMemServerManager_delMemServer_nolock(hManager, pmsd);
            eggRWSMemServerDescriptor_release(pmsd);
            free(pmsd);

            pthread_mutex_unlock(&hManager->mutex_head);
	    return 0;
        }
        pmsd = pmsd->next_name;
    }

    pthread_mutex_unlock(&hManager->mutex_head);
    return 0;
}

int eggRWSMemServerManager_makeFlush_allMemServer(HEGGRWSMEMSERVERMANAGER hManager)
{
    pthread_mutex_lock(&hManager->mutex_head);
    
    HEGGRWSMEMSERVERDESCRIPTOR pmsd;
    HEGGRWSMEMSERVERDESCRIPTOR pmsd_end;
    pmsd_end = &hManager->memServerD_head;
    
    pmsd = hManager->memServerD_head.next_name;
    while(pmsd != pmsd_end)
    {
        eggRWSMemServerDescriptor_makeFlush(pmsd);
        
        pmsd = pmsd->next_name;
    }
    
    pthread_mutex_unlock(&hManager->mutex_head);
    return 0;
}


#define doublelink_insertbefore(p, link, PREVLINKNAME, NEXTLINKNAME) ((p)->PREVLINKNAME = (link)->PREVLINKNAME, (p)->NEXTLINKNAME = (link), (link)->PREVLINKNAME = (p), (p)->PREVLINKNAME->NEXTLINKNAME = (p))
#define doublelink_unlink(p, PREVLINKNAME, NEXTLINKNAME)                \
    {                                                                   \
        if ((p)->PREVLINKNAME && (p)->NEXTLINKNAME)                     \
            ((p)->PREVLINKNAME->NEXTLINKNAME = (p)->NEXTLINKNAME, (p)->NEXTLINKNAME->PREVLINKNAME = (p)->PREVLINKNAME, (p)->PREVLINKNAME = (p)->NEXTLINKNAME = NULL); \
    }
/* add asc */
#define doublelink_add(head, end, item, tmp, PREVLINKNAME, NEXTLINKNAME, cmp) \
    {                                                                   \
        (tmp) = (head);                                                 \
        while ((tmp) != (end)) {                                        \
            if (cmp((item), (tmp)) < 0) { doublelink_insertbefore((item), (tmp), PREVLINKNAME, NEXTLINKNAME); break; } \
            (tmp) = (tmp)->NEXTLINKNAME;                                \
        }                                                               \
        if ((tmp) == (end)) doublelink_insertbefore((item), (end), PREVLINKNAME, NEXTLINKNAME); \
    }

int eggRWSMemServerManager_addMemServer_nolock(HEGGRWSMEMSERVERMANAGER hMemServerManager,
                                               HEGGRWSMEMSERVERDESCRIPTOR hMemServerD)
{
    if (!hMemServerManager)
    {
        return 0;
    }

    LOG_CLAIM("nMemServer[%d] +add MemServer[%s]", hMemServerManager->nMemServer, hMemServerD->name);

    hMemServerManager->nMemServer++;
    HEGGRWSMEMSERVERDESCRIPTOR pmsd;
    HEGGRWSMEMSERVERDESCRIPTOR pmsd_tmp;
    HEGGRWSMEMSERVERDESCRIPTOR pmsd_end;

    /* sort by timeout */
    pmsd = hMemServerManager->memServerD_head.next_timeout;
    pmsd_end = &hMemServerManager->memServerD_head;
    doublelink_add(pmsd, pmsd_end, hMemServerD, pmsd_tmp,
                   prev_timeout, next_timeout,
                   eggRWSMemServerDescriptor_cmpTimeout);

    /* sort by name  */
    pmsd = hMemServerManager->memServerD_head.next_name;
    pmsd_end = &hMemServerManager->memServerD_head;
    doublelink_add(pmsd, pmsd_end, hMemServerD, pmsd_tmp,
                   prev_name, next_name,
                   eggRWSMemServerDescriptor_cmpName);
    return 0;
}

int eggRWSMemServerManager_addMemServer(HEGGRWSMEMSERVERMANAGER hMemServerManager,
                                            HEGGRWSMEMSERVERDESCRIPTOR hMemServerD)
{
    if (!hMemServerManager)
    {
        return 0;
    }

    pthread_mutex_lock(&hMemServerManager->mutex_head);
    eggRWSMemServerManager_addMemServer_nolock(hMemServerManager, hMemServerD);
    pthread_mutex_unlock(&hMemServerManager->mutex_head);

    return 0;
}

int eggRWSMemServerManager_delMemServer_nolock(HEGGRWSMEMSERVERMANAGER hMemServerManager,
                                               HEGGRWSMEMSERVERDESCRIPTOR hMemServerD)
{
    if (!hMemServerManager)
    {
        return 0;
    }
    
    LOG_CLAIM("nMemServer[%d] -del MemServer[%s]", hMemServerManager->nMemServer, hMemServerD->name);

    hMemServerManager->nMemServer--;

    doublelink_unlink(hMemServerD, prev_timeout, next_timeout);
    doublelink_unlink(hMemServerD, prev_name, next_name);
    
    return 0;

}

int eggRWSMemServerManager_delMemServer(HEGGRWSMEMSERVERMANAGER hMemServerManager,
                                            HEGGRWSMEMSERVERDESCRIPTOR hMemServerD)
{
    if (!hMemServerManager)
    {
        return 0;
    }
    
    pthread_mutex_lock(&hMemServerManager->mutex_head);
    
    eggRWSMemServerManager_delMemServer_nolock(hMemServerManager, hMemServerD);
    
    pthread_mutex_unlock(&hMemServerManager->mutex_head);
    return 0;

}


HEGGRWSMEMSERVERDESCRIPTOR
eggRWSMemServerManager_getMemServerById(HEGGRWSMEMSERVERMANAGER hMemServerManager, EGGRWSMEMSERVERID id)
{
    if (!hMemServerManager)
    {
        return NULL;
    }

    pthread_mutex_lock(&hMemServerManager->mutex_head);
    
    HEGGRWSMEMSERVERDESCRIPTOR pmsd;
    HEGGRWSMEMSERVERDESCRIPTOR pmsd_end;
    pmsd_end = &hMemServerManager->memServerD_head;
    

    pmsd = hMemServerManager->memServerD_head.next_name;
    while(pmsd != pmsd_end)
    {
        if (pmsd->id == id)
        {
            eggRWSMemServerDescriptor_userIncrease(pmsd);
            pthread_mutex_unlock(&hMemServerManager->mutex_head);
	    return pmsd;
        }
        pmsd = pmsd->next_name;
    }

    pthread_mutex_unlock(&hMemServerManager->mutex_head);
    
    return NULL;
    
}

HEGGRWSMEMSERVERDESCRIPTOR
eggRWSMemServerManager_getRecentTimeoutMemServer(HEGGRWSMEMSERVERMANAGER hMemServerManager, struct timeval *p_tv)
{
    if (!hMemServerManager)
    {
        return NULL;
    }


    pthread_mutex_lock(&hMemServerManager->mutex_head);
    
    HEGGRWSMEMSERVERDESCRIPTOR pmsd;
    HEGGRWSMEMSERVERDESCRIPTOR pmsd_end;
    pmsd = hMemServerManager->memServerD_head.next_timeout;
    pmsd_end = &hMemServerManager->memServerD_head;
    if (pmsd == pmsd_end)
    {
        pthread_mutex_unlock(&hMemServerManager->mutex_head);
        return NULL;
    }
    if (p_tv)
    {
        *p_tv = pmsd->timeout;
    }

    pthread_mutex_unlock(&hMemServerManager->mutex_head);
    return pmsd;
}

HEGGRWSMEMSERVERDESCRIPTOR
eggRWSMemServerManager_getNextDumpingMemServer_nolock(HEGGRWSMEMSERVERMANAGER hMemServerManager)
{
    if (!hMemServerManager)
    {
        return NULL;
    }

    HEGGRWSMEMSERVERDESCRIPTOR pmsd;
    HEGGRWSMEMSERVERDESCRIPTOR pmsd_end;
    pmsd = hMemServerManager->memServerD_head.next_name;
    pmsd_end = &hMemServerManager->memServerD_head;
    while(pmsd != pmsd_end)
    {
        int stat;
        stat = pmsd->si_head.stat;
        if (eggRWSMemServerStat_getStat(stat) == EGGRWSMEMSERVERSTAT_CLEANUP
            || eggRWSMemServerStat_getStat(stat) == EGGRWSMEMSERVERSTAT_NORMAL)
        {

            return pmsd;
        }
        pmsd = pmsd->next_name;
    }

    return NULL;
}

HEGGRWSMEMSERVERDESCRIPTOR
eggRWSMemServerManager_getNextDumpingMemServer(HEGGRWSMEMSERVERMANAGER hMemServerManager)
{
    if (!hMemServerManager)
    {
        return NULL;
    }

    pthread_mutex_lock(&hMemServerManager->mutex_head);
    eggRWSMemServerManager_getNextDumpingMemServer_nolock(hMemServerManager);
    pthread_mutex_unlock(&hMemServerManager->mutex_head);
    return NULL;
}


HEGGRWSMEMSERVERDESCRIPTOR eggRWSMemServerManager_getWritableMemServer(HEGGRWSMEMSERVERMANAGER hMemServerManager)
{
    if (!hMemServerManager)
    {
        return NULL;
    }

    pthread_mutex_lock(&hMemServerManager->mutex_head);
    
    HEGGRWSMEMSERVERDESCRIPTOR pmsd, pmsd_end;
    pmsd = hMemServerManager->memServerD_head.prev_name;
    pmsd_end = &hMemServerManager->memServerD_head;
    while (pmsd != pmsd_end)
    {
        if (!eggRWSMemServerDescriptor_is_memServerCreator(pmsd))
        {
            if (eggRWSMemServerStat_is_writeable(pmsd->si_head.stat))
            {
                eggRWSMemServerDescriptor_userIncrease(pmsd);
                
                pthread_mutex_unlock(&hMemServerManager->mutex_head);
                return pmsd;
            }
        }
        pmsd = pmsd->prev_name;
    }
    
    pthread_mutex_unlock(&hMemServerManager->mutex_head);
    
    LOG_CLAIM("No Writable MemServer");
    
    return NULL;
}

HEGGRWSMEMSERVERDESCRIPTOR *
eggRWSMemServerManager_getReadableMemServer(HEGGRWSMEMSERVERMANAGER hMemServerManager, int *count, EGGRWSMEMSERVERID minId)
{
    if (!hMemServerManager)
    {
        *count = 0;
        return NULL;
    }

    pthread_mutex_lock(&hMemServerManager->mutex_head);

    
    HEGGRWSMEMSERVERDESCRIPTOR pmsd;
    HEGGRWSMEMSERVERDESCRIPTOR pmsd_end;
    pmsd_end = &hMemServerManager->memServerD_head;
    

    pmsd = hMemServerManager->memServerD_head.next_name;
    *count = 0;
    while(pmsd != pmsd_end)
    {
        if (eggRWSMemServerStat_is_readable(pmsd->si_head.stat) && eggRWSMemServerDescriptor_id(pmsd) > minId)
        {
            eggRWSMemServerDescriptor_userIncrease(pmsd);
            ++*count;
        }
        pmsd = pmsd->next_name;
    }
    if (*count == 0)
    {
        LOG_CLAIM("No Readable MemServer");
        
        pthread_mutex_unlock(&hMemServerManager->mutex_head);
        return NULL;
    }

    
    int i = 0;
    HEGGRWSMEMSERVERDESCRIPTOR *pp_retmsd;
    pp_retmsd = malloc(*count * sizeof(*pp_retmsd));
    assert(pp_retmsd);
    pmsd = hMemServerManager->memServerD_head.next_name;
    while(pmsd != pmsd_end)
    {
        if (eggRWSMemServerStat_is_readable(pmsd->si_head.stat) && eggRWSMemServerDescriptor_id(pmsd) > minId)
        {
            pp_retmsd[i] = pmsd;
            i++;
        }
        pmsd = pmsd->next_name;
    }

    pthread_mutex_unlock(&hMemServerManager->mutex_head);
    return pp_retmsd;
}

static int eggRWSMemServerManager_sortTimeout_memServer(HEGGRWSMEMSERVERMANAGER hMemServerManager,
                                                        HEGGRWSMEMSERVERDESCRIPTOR hMemServerD)
{
    if (!hMemServerManager)
    {
        return 0;
    }
    
    
    doublelink_unlink(hMemServerD, prev_timeout, next_timeout);
    
    HEGGRWSMEMSERVERDESCRIPTOR pmsd;
    HEGGRWSMEMSERVERDESCRIPTOR pmsd_tmp;    
    HEGGRWSMEMSERVERDESCRIPTOR pmsd_end;
    pmsd = hMemServerManager->memServerD_head.next_timeout;
    pmsd_end = &hMemServerManager->memServerD_head;
    doublelink_add(pmsd, pmsd_end, hMemServerD, pmsd_tmp,
                   prev_timeout, next_timeout,
                   eggRWSMemServerDescriptor_cmpTimeout);
    return 0;
}

int eggRWSMemServerManager_setupTimeout_memServer_nolock(HEGGRWSMEMSERVERMANAGER hManager,
                                                         HEGGRWSMEMSERVERDESCRIPTOR hMemServerD)
{
    if (!hManager)
    {
        return 0;
    }

    hMemServerD->timeout = timeval_someTimeLater(hMemServerD->si_head.timeToLive,
                                                 hManager->now);
    eggRWSMemServerManager_sortTimeout_memServer(hManager, hMemServerD);

    return 0;
}

int eggRWSMemServerManager_setupTimeout_memServer(HEGGRWSMEMSERVERMANAGER hManager,
                                                  HEGGRWSMEMSERVERDESCRIPTOR hMemServerD)
{
    if (!hManager)
    {
        return 0;
    }

    pthread_mutex_lock(&hManager->mutex_head);
    eggRWSMemServerManager_setupTimeout_memServer_nolock(hManager, hMemServerD);
    pthread_mutex_unlock(&hManager->mutex_head);
    return 0;
}

int eggRWSMemServerManager_trigger(HEGGRWSMEMSERVERMANAGER hMemServerManager,
                                          HEGGRWSMEMSERVERDESCRIPTOR hMemServerD)
{
    if (!hMemServerManager)
    {
        return 0;
    }

    pthread_mutex_lock(&hMemServerManager->mutex_head);
    
    STATUSINSTRUCT *psi;
    psi = &hMemServerD->si_head;
    int (*trigger)(HEGGRWSMEMSERVERDESCRIPTOR);
    trigger = psi->trigger;

    (*trigger)(hMemServerD);

    pthread_mutex_unlock(&hMemServerManager->mutex_head);

    return 0;
}

int eggRWSMemServerManager_signalRetry_nolock(HEGGRWSMEMSERVERMANAGER hMemServerManager)
{
    if (!hMemServerManager)
    {
        return 0;
    }

    HEGGRWSMEMSERVERDESCRIPTOR pmsd;
    HEGGRWSMEMSERVERDESCRIPTOR pmsd_end;
    pmsd = hMemServerManager->memServerD_head.next_name;
    pmsd_end = &hMemServerManager->memServerD_head;
    while(pmsd != pmsd_end)
    {
        if (eggRWSMemServerStat_is_retry(pmsd->si_head.stat))
        {
            eggRWSMemServerStat_clear_retry(pmsd->si_head.stat);
            pmsd->si_head.timeToLive = 0;
            eggRWSMemServerManager_setupTimeout_memServer_nolock(hMemServerManager, pmsd);
        }

        pmsd = pmsd->next_name;
    }

    eggRWSIntServer_wakeUp(hMemServerManager->hIntServer);

    return 0;
}

int eggRWSMemServerManager_signalRetry(HEGGRWSMEMSERVERMANAGER hMemServerManager)
{
    if (!hMemServerManager)
    {
        return 0;
    }

    pthread_mutex_lock(&hMemServerManager->mutex_head);
    eggRWSMemServerManager_signalRetry_nolock(hMemServerManager);
    pthread_mutex_unlock(&hMemServerManager->mutex_head);

    return 0;
}
