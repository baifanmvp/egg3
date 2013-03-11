#include "eggRecoveryLog.h"
#include "../log/eggPrtLog.h"
#include <pthread.h>

#ifdef EGGRECOVERYLOG

#define _FILE_OFFSET_BITS 64
#include <features.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <assert.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/time.h>

#if MEASURETIME
#define MEASURE_START struct timeval t1, t2; gettimeofday(&t1, 0); int measure_count = 0;
#define MEASURE_END     gettimeofday(&t2, 0); printf("[%d]%s: %.6f\n", ++measure_count, __func__, (t2.tv_sec - t1.tv_sec) + (t2.tv_usec - t1.tv_usec)/1000000.);
#else
#define MEASURE_START
#define MEASURE_END
#endif

static pthread_mutex_t s_writeMutex = PTHREAD_MUTEX_INITIALIZER;
static int s_inaction;
static pthread_mutex_t s_inactionMutex = PTHREAD_MUTEX_INITIALIZER;
struct eggRecoveryHandle
{
    char *baseName;
    int fd_recLogFile;
    int fd_recLogInfoFile;
    
};
    
typedef struct {
    uint64_t prevLsn;
    uint64_t undoNextLsn;
    uint32_t logType;
    uint32_t actId;
    uint64_t filePosition;
    uint64_t size;
    char *fileName;
    char *data;
} eggLogRecord;
typedef struct {
    uint64_t Lsn;
    uint64_t flag;
} eggLogInfo;
enum eggLogInfoFlag { LOGFLAG_WRITTEN = 'W', LOGFLAG_CHECKPOINT = 'K' };
struct ActInfo {
    uint64_t actId;
    uint64_t lastLsn;
    uint64_t undoNextLsn;
};

typedef struct ActIdList {
    uint64_t actId;
    uint64_t lastLsn;
    uint64_t undoNextLsn;
    struct ActIdList *next;
} ActIdList;
static eggLogRecord *getNextRecord(EGGRECOVERYHANDLE *pEggRecoveryHandle,
                                   int size, eggLogRecord *bufRecord);
static inline int freeRecord(eggLogRecord *p);
static int rewriteData(char *baseName, char *file, uint64_t pos, void *data, uint64_t sz);
static int ActId_put(ActIdList **ppactIds,
                     uint64_t actId, uint64_t lastLsn, uint64_t undoNextLsn);
static int ActId_remove(ActIdList **ppactIds, uint64_t actId);
static int ActId_removeAll(ActIdList **ppactIds);
static ActInfo *ActId_getMaxUndoNextLsn(ActIdList **ppactIds);
static uint64_t getNextLsn(EGGRECOVERYHANDLE *pEggRecoveryHandle);
static uint32_t getNewActId(EGGRECOVERYHANDLE *pEggRecoveryHandle);
static uint64_t getFileSize(int fd);
static int writeLog(EGGRECOVERYHANDLE *pEggRecoveryHandle,
                    ActInfo *actInfo,
                    uint32_t logType, char *fileName, uint64_t filePosition,
                    void *data, uint64_t size, int ifconfirmed);
static int markCheckpoint(EGGRECOVERYHANDLE *pEggRecoveryHandle);
static int logFile_check(EGGRECOVERYHANDLE *pEggRecoveryHandle);
static int eggRecoveryLog_recover(EGGRECOVERYHANDLE *pEggRecoveryHandle);
static int lockWrWait(int fd, short whence, off_t start, off_t len);
static int lockWrTry(int fd, short whence, off_t start, off_t len);
static int lockRdWait(int fd, short whence, off_t start, off_t len);
static int lockRdTry(int fd, short whence, off_t start, off_t len);
static int unlock(int fd, short whence, off_t start, off_t len);
static uint64_t getFileSize(int fd);
static inline int eggRecoveryLog_getfd(char *logName,
                                   int *fd_recLogFile,
                                   int *fd_recLogInfoFile);
static HEGGRECOVERYHANDLE eggRecoveryHandle_new(char *baseName);
static int eggRecoveryHandle_delete(HEGGRECOVERYHANDLE);

static int FTRUNCATE(int fd, off_t length);
static ssize_t READ(int fd, void *buf, size_t count);
static ssize_t WRITE(int fd, const void *buf, size_t count);
static void *MALLOC(size_t size);
static void FREE(void *ptr, size_t size);

static HEGGRECOVERYHANDLE eggRecoveryHandle_new(char *baseName)
{
    if (!baseName || !baseName[0])
    {
        return NULL;
    }
    struct eggRecoveryHandle *retp = calloc(1, sizeof(struct eggRecoveryHandle));
    retp->baseName = strdup(baseName);
    assert(retp->baseName);
    
    if (eggRecoveryLog_getfd(retp->baseName,
                             &retp->fd_recLogFile,
                             &retp->fd_recLogInfoFile) < 0)
    {
        return NULL;
    }
    return retp;
}
static int eggRecoveryHandle_delete(HEGGRECOVERYHANDLE pEggRecoveryHandle)
{
    if (!pEggRecoveryHandle)
    {
        return 0;
    }
    free(pEggRecoveryHandle->baseName);
    close(pEggRecoveryHandle->fd_recLogFile);
    
    eggPrtLog_info("eggRecoveryLog", "[%s]: close fd[%d]\n", __func__, pEggRecoveryHandle->fd_recLogFile);
    
    close(pEggRecoveryHandle->fd_recLogInfoFile);
    
    eggPrtLog_info("eggRecoveryLog", "[%s]: close fd[%d]\n", __func__, pEggRecoveryHandle->fd_recLogInfoFile);
    
    free(pEggRecoveryHandle);
    return 0;
}

    
EGGRECOVERYHANDLE *eggRecoveryLog_init(char *baseName)
{
    MEASURE_START;
    struct eggRecoveryHandle *retp;
    retp = eggRecoveryHandle_new(baseName);
    if (!retp)
    {
        return NULL;
    }
    eggRecoveryLog_makeclean_checkpoint(retp);
    
    MEASURE_END;
        
    return retp;
}
int eggRecoveryLog_destroy(EGGRECOVERYHANDLE *pEggRecoveryHandle)
{
    if (!pEggRecoveryHandle)
        return 0;
    eggRecoveryHandle_delete(pEggRecoveryHandle);
    return 0;
}
int eggRecoveryLog_make_checkpoint(EGGRECOVERYHANDLE *pEggRecoveryHandle)
{
    if (!pEggRecoveryHandle)
    {
        return 0;
    }
    
    int fd_recLogFile = pEggRecoveryHandle->fd_recLogFile;
    int fd_recLogInfoFile = pEggRecoveryHandle->fd_recLogInfoFile;

    int r;
    r = lockWrWait(fd_recLogFile, SEEK_SET, 0, 0);
    if (r < 0)
    {
        eggPrtLog_warn("eggRecoveryLog", "Warning: lockWrWait < 0: %s FAIL", __func__);
        return -1;
    }

    pthread_mutex_lock(&s_writeMutex);
    
    logFile_check(pEggRecoveryHandle);
    if (lockWrTry(fd_recLogInfoFile, SEEK_SET, 0, 1) == 0)
    {
        pthread_mutex_lock(&s_inactionMutex);
        
        if (s_inaction == 0)
        {
            eggRecoveryLog_recover(pEggRecoveryHandle);
        }
        pthread_mutex_unlock(&s_inactionMutex);
    }
    
    unlock(fd_recLogInfoFile, SEEK_SET, 0, 1);

    pthread_mutex_unlock(&s_writeMutex);
    
    unlock(fd_recLogFile, SEEK_SET, 0, 0);
    
    return 0;
}
int eggRecoveryLog_makeclean_checkpoint(EGGRECOVERYHANDLE *pEggRecoveryHandle)
{
    if (!pEggRecoveryHandle)
    {
        return 0;
    }
    
    int fd_recLogFile = pEggRecoveryHandle->fd_recLogFile;
    int fd_recLogInfoFile = pEggRecoveryHandle->fd_recLogInfoFile;
    
    int r;
    r = lockWrWait(fd_recLogFile, SEEK_SET, 0, 0);
    if (r < 0)
    {
        eggPrtLog_warn("eggRecoveryLog", "Warning: lockWrWait < 0: %s FAIL", __func__);
        return -1;
    }
    
    
    pthread_mutex_lock(&s_writeMutex);
    
    logFile_check(pEggRecoveryHandle);
    if (lockWrTry(fd_recLogInfoFile, SEEK_SET, 0, 1) == 0)
    {
        pthread_mutex_lock(&s_inactionMutex);
        if (s_inaction == 0)
        {
            eggRecoveryLog_recover(pEggRecoveryHandle);
            FTRUNCATE(fd_recLogFile, offsetof(eggLogRecord, fileName));
            FTRUNCATE(fd_recLogInfoFile, 2 * sizeof(eggLogInfo));
        }
        
        pthread_mutex_unlock(&s_inactionMutex);
    }
    unlock(fd_recLogInfoFile, SEEK_SET, 0, 1);

    pthread_mutex_unlock(&s_writeMutex);
    
    unlock(fd_recLogFile, SEEK_SET, 0, 0);

    return 0;
}

ActInfo *eggRecoveryLog_beginact(EGGRECOVERYHANDLE *pEggRecoveryHandle)
{
    if (!pEggRecoveryHandle)
    { return NULL; }

    int r;
    r = lockRdWait(pEggRecoveryHandle->fd_recLogInfoFile, SEEK_SET, 0, 1);
    if (r < 0)
    {
        eggPrtLog_error("eggRecoveryLog", "Error: lockRdWait < 0: %s FAIL, EXIT", __func__);
        exit(-1);
    }
    
    pthread_mutex_lock(&s_inactionMutex);
    s_inaction++;
    pthread_mutex_unlock(&s_inactionMutex);    
    
    ActInfo *actInfo;
    actInfo = calloc(1, sizeof(ActInfo));
    assert(actInfo);
    actInfo->actId  = getNewActId(pEggRecoveryHandle);
    actInfo->lastLsn = 0;
    actInfo->undoNextLsn = 0;

    
    return actInfo;
}

int eggRecoveryLog_endact(EGGRECOVERYHANDLE *pEggRecoveryHandle, ActInfo *actInfo)
{
    if (!pEggRecoveryHandle)
    {return 0;}
    eggRecoveryLog_writelog(pEggRecoveryHandle,
                            actInfo, EGGRECOVERYLOG_COMMIT, NULL, 0, NULL, 0);
    free(actInfo);

    pthread_mutex_lock(&s_inactionMutex);
    s_inaction--;
    pthread_mutex_unlock(&s_inactionMutex);    
    
    unlock(pEggRecoveryHandle->fd_recLogInfoFile, SEEK_SET, 0, 1);    
    return 0;
}

int eggRecoveryLog_recover(EGGRECOVERYHANDLE *pEggRecoveryHandle)
{
    int fd_recLogFile, fd_recLogInfoFile;
    fd_recLogFile = pEggRecoveryHandle->fd_recLogFile;
    fd_recLogInfoFile = pEggRecoveryHandle->fd_recLogInfoFile;
    
    int i;
    int n;
    /*
    if (getFileSize(fd_recLogFile) < (off_t)offsetof(eggLogRecord, fileName))
    {
        return 0;
    }
    */

    MEASURE_START
    uint64_t t;
    t = getFileSize(fd_recLogInfoFile);
    /* get checkpoint */
    if (t % sizeof(eggLogInfo))
    {
        t = t - t % sizeof(eggLogInfo);
        FTRUNCATE(fd_recLogInfoFile, t);
    }
    if (t == 0)
    {
        return 0;
    }
    size_t ploginfoSz = t;
    eggLogInfo *ploginfo = (eggLogInfo *)MALLOC(ploginfoSz);
    assert(ploginfo);
    lseek(fd_recLogInfoFile, 0, SEEK_SET);
    READ(fd_recLogInfoFile, ploginfo, t);
    n = t / sizeof(eggLogInfo);
    if (ploginfo[n-1].flag != 0)
    {
        ploginfo[n-1].flag = 0;
        lseek(fd_recLogInfoFile, (n - 1) * sizeof(eggLogInfo), SEEK_SET);
        WRITE(fd_recLogInfoFile, &ploginfo[n-1], sizeof(eggLogInfo));
    }
    for (i = n - 1; i >= 0; i--)
    {
        if (ploginfo[i].flag == LOGFLAG_CHECKPOINT)
        {
            break;
        }
    }
    i++;

    struct ActIdList *pactIds = 0;
    eggLogRecord tmpRecord;
    /* redo phase */
    for (; i < n - 1; i++)
    {
	if (ploginfo[i].flag == LOGFLAG_WRITTEN) /* already on disk */
        {
            continue;
        }

        eggLogRecord *plogrecord;
        uint64_t Lsn = ploginfo[i].Lsn;
        int recordsz = ploginfo[i+1].Lsn - ploginfo[i].Lsn;
        lseek(fd_recLogFile, ploginfo[i].Lsn, SEEK_SET);
        plogrecord = getNextRecord(pEggRecoveryHandle,
                                   recordsz,
                                   &tmpRecord);
        if (!plogrecord)
        {
            continue;
        }
	else if (plogrecord->logType == EGGRECOVERYLOG_REDO)
        {
            if (plogrecord->fileName[0] == '/')
            {
                rewriteData(pEggRecoveryHandle->baseName,
                            plogrecord->fileName, plogrecord->filePosition,
                            plogrecord->data, plogrecord->size);

            }
            else
            {
                char *pathName;
                char *p;
                int n = strlen(pEggRecoveryHandle->baseName);
                pathName = malloc(n + 2 + strlen(plogrecord->fileName));
                assert(pathName);
                p = strrchr(pEggRecoveryHandle->baseName, '/');
                p = p ? p : pEggRecoveryHandle->baseName + n;
                sprintf(pathName, "%.*s/%s",
                        (int)(p - pEggRecoveryHandle->baseName),
                        pEggRecoveryHandle->baseName,
                        plogrecord->fileName);
                rewriteData(pEggRecoveryHandle->baseName,
                            pathName, plogrecord->filePosition,
                            plogrecord->data, plogrecord->size);
                free(pathName);
            }

            ploginfo[i].flag = LOGFLAG_WRITTEN; /* update WRITTEN */
            {
                lseek(fd_recLogInfoFile, sizeof(eggLogInfo) * i, SEEK_SET);
                WRITE(fd_recLogInfoFile, &ploginfo[i], sizeof(eggLogInfo));
            }
            ActId_put(&pactIds, plogrecord->actId, Lsn, 0);
	}
        else if (plogrecord->logType == EGGRECOVERYLOG_COMMIT)
        {
            ActId_remove(&pactIds, plogrecord->actId);
        }
        else if (plogrecord->logType == EGGRECOVERYLOG_UNDO)
        {
            ActId_put(&pactIds,plogrecord->actId, Lsn, Lsn);
        }
        freeRecord(plogrecord);
    }

    FTRUNCATE(fd_recLogFile, ploginfo[n-1].Lsn);
    FREE(ploginfo, ploginfoSz);
    
    /* undo phase */
    ActInfo *pact;
    while ((pact = ActId_getMaxUndoNextLsn(&pactIds))
           && pact->undoNextLsn)
    {
        eggLogRecord *plogrecord;
        lseek(fd_recLogFile, pact->undoNextLsn, SEEK_SET);
        plogrecord = getNextRecord(pEggRecoveryHandle, 0,
                                   &tmpRecord);
	if (plogrecord->logType == EGGRECOVERYLOG_REDO)
        {
	    ActId_put(&pactIds, plogrecord->actId, 0, plogrecord->undoNextLsn);
        }
	else if (plogrecord->logType == EGGRECOVERYLOG_UNDO)
        {
            if (plogrecord->fileName[0] == '/')
            {
                rewriteData(pEggRecoveryHandle->baseName,
                            plogrecord->fileName, plogrecord->filePosition,
                            plogrecord->data, plogrecord->size);

            }
            else
            {
                char *pathName;
                char *p;
                int n = strlen(pEggRecoveryHandle->baseName);
                pathName = malloc(n + 2 + strlen(plogrecord->fileName));
                assert(pathName);
                p = strrchr(pEggRecoveryHandle->baseName, '/');
                p = p ? p : pEggRecoveryHandle->baseName + n;
                sprintf(pathName, "%.*s/%s",
                        (int)(p - pEggRecoveryHandle->baseName),
                        pEggRecoveryHandle->baseName,
                        plogrecord->fileName);
                rewriteData(pEggRecoveryHandle->baseName,
                            pathName, plogrecord->filePosition,
                            plogrecord->data, plogrecord->size);
                free(pathName);
            }
            pact->undoNextLsn = plogrecord->undoNextLsn;
	    writeLog(pEggRecoveryHandle,
                     pact, EGGRECOVERYLOG_REDO,
                     plogrecord->fileName, plogrecord->filePosition,
                     plogrecord->data, plogrecord->size, 0);
	}
        freeRecord(plogrecord);
    }
    ActId_removeAll(&pactIds);
    /* mark checkpoint */
    markCheckpoint(pEggRecoveryHandle);
    
    MEASURE_END
    return 0;
}

static inline int freeRecord(eggLogRecord *p)
{
    if (p)
    {
        free(p->fileName);
    }
    return 0;
}
static eggLogRecord *getNextRecord(EGGRECOVERYHANDLE *pEggRecoveryHandle,
                                   int size,
                                   eggLogRecord *bufRecord)
{
    int fd_recLogFile, fd_recLogInfoFile;
    fd_recLogFile = pEggRecoveryHandle->fd_recLogFile;
    fd_recLogInfoFile = pEggRecoveryHandle->fd_recLogInfoFile;

    if (size != 0 && size < offsetof(eggLogRecord, fileName))
    {
        /* printf("Warning: LogRecord size %d < %d", size, */
		/* 	   (int)offsetof(eggLogRecord, fileName)); */
        eggPrtLog_warn("eggRecoveryLog", "Warning: LogRecord size %d < %d", size,
			   (int)offsetof(eggLogRecord, fileName));

        return NULL;
    }

    uint64_t Lsn;
    Lsn = lseek(fd_recLogFile, 0, SEEK_CUR);
    memset(bufRecord, 0, sizeof(eggLogRecord));
    /* size == 0, according eggLogRecord */
    if (size == 0 || size == offsetof(eggLogRecord, fileName))
    {
        int n;
        n  = READ(fd_recLogFile, bufRecord, offsetof(eggLogRecord, fileName));
        if (n <= 0)          /* end of file */
        {
            return NULL;
        }
        else if (bufRecord->size == offsetof(eggLogRecord, fileName))
        {
            bufRecord->size = 0;
            return bufRecord;
        }
        else if (bufRecord->size > offsetof(eggLogRecord, fileName)
                 && bufRecord->prevLsn < Lsn && bufRecord->undoNextLsn < Lsn
                 && bufRecord->logType < 128)
        {
            char *p;
            p = malloc(bufRecord->size - offsetof(eggLogRecord, fileName));
            assert(p);
            READ(fd_recLogFile, p, bufRecord->size - offsetof(eggLogRecord, fileName));
            bufRecord->fileName = p;
            bufRecord->data = p + strlen(p) + 1;
            bufRecord->size -= offsetof(eggLogRecord, fileName) + strlen(p) + 1;
        }
        else
        {
            /* printf("%s:%d: bad record[%llu]\n", __FILE__, __LINE__, */
			/* 	   (long long unsigned)Lsn); */
            eggPrtLog_warn("eggRecoveryLog", "%s:%d: bad record[%llu]\n", __FILE__, __LINE__,
				   (long long unsigned)Lsn);

            return NULL;
        }
        return bufRecord;
    }

    char *p;
    p = malloc(size);
    assert(p);
    READ(fd_recLogFile, p, size);
    memcpy(bufRecord, p, offsetof(eggLogRecord, fileName));
    if (bufRecord->size > size)
    {
        free(p);
        /* printf("%s:%d: bad record[%llu]\n", __FILE__, __LINE__, */
		/* 	   (long long unsigned)Lsn); */
        eggPrtLog_warn("eggRecoveryLog", "%s:%d: bad record[%llu]\n", __FILE__, __LINE__,
			   (long long unsigned)Lsn);

        return NULL;
    }
    memmove(p, p + offsetof(eggLogRecord, fileName), size - offsetof(eggLogRecord, fileName));
    bufRecord->fileName = p;
    bufRecord->data = p + strlen(p) + 1;
    bufRecord->size -= offsetof(eggLogRecord, fileName) + strlen(p) + 1;
    return bufRecord;
}


static int rewriteData(char *baseName, char *file, uint64_t pos, void *data, uint64_t sz)
{
    assert(baseName[0] == '/' && file[0] == '/');
    char *realName;
    realName = calloc(strlen(baseName) + strlen(file) + 1, 1);
    assert(realName);
    char *p;
    p = strrchr(baseName, '/');
    while (p > baseName && *p == '/')
    {
        p--;
    }
    if (*p != '/')
    {
        p++;
    }
    strncpy(realName, baseName, p - baseName);
    realName[p-baseName] = '\0';
    p = strrchr(file, '/');
    p++;
    strcat(realName, "/");
    strcat(realName, p);
    
    int fd = open(realName, O_CREAT|O_RDWR, 0666);
    if (fd < 0)
    {
        eggPrtLog_warn("eggRecoveryLog", "Warn[%s][%s][%s]\n", __func__, strerror(errno), realName);
        free(realName);
        return 1;
    }
    lseek(fd, pos, SEEK_SET);
    WRITE(fd, data, sz);
    close(fd);
    free(realName);
    return 0;
}

static int ActId_put(ActIdList **ppactIds,
                     uint64_t actId, uint64_t lastLsn, uint64_t undoNextLsn)
{
    struct ActIdList *p = *ppactIds;
    struct ActIdList *plast = NULL;
    while (p)
    {
        if (p->actId == actId)
        {
            p->lastLsn = (lastLsn == 0 ? p->lastLsn : lastLsn);
            p->undoNextLsn = (undoNextLsn == 0 ? p->undoNextLsn : undoNextLsn);
            return 0;
        }
        plast = p;
        p = p->next;
    }
    p = malloc(sizeof(*p));
    assert(p);
    p->actId = actId;
    p->lastLsn = lastLsn;
    p->undoNextLsn = undoNextLsn;
    p->next = 0;
    if (plast)
    {
        plast->next = p;
    }
    else
    {
        *ppactIds = p;
    }
    return 0;
}
static int ActId_remove(ActIdList **ppactIds, uint64_t actId)
{
    if (!*ppactIds)
    {
        return 0;
    }
    else if ((*ppactIds)->actId == actId)
    {
        
        struct ActIdList *p = (*ppactIds)->next;
        free(*ppactIds);
        *ppactIds = p;
        return 0;
    }
    
    struct ActIdList *p = (*ppactIds)->next;
    struct ActIdList *plast = *ppactIds;
    while (p)
    {
        if (p->actId == actId)
        {
            plast->next = p->next;
            free(p);
            return 0;
        }
        plast = p;
        p = p->next;
    }
    return 0;
}
static int ActId_removeAll(ActIdList **ppactIds)
{
    
    struct ActIdList *p = *ppactIds;
    while (p)
    {
        struct ActIdList *pt = p->next;
        free(p);
        p = pt;
    }
    *ppactIds = 0;
    return 0;
}
static ActInfo *ActId_getMaxUndoNextLsn(ActIdList **ppactIds)
{
    if (!*ppactIds)
    {
        return 0;
    }
    
    uint64_t max = 0;
    struct ActIdList *p = *ppactIds;
    struct ActIdList *q = NULL;    
    while (p)
    {
        if (p->undoNextLsn > max)
        {
            q = p;
        }
        p = p->next;
    }
    return (ActInfo *)q;
}

static uint64_t getNextLsn(EGGRECOVERYHANDLE *pEggRecoveryHandle)
{
    int fd_recLogFile, fd_recLogInfoFile;
    fd_recLogFile = pEggRecoveryHandle->fd_recLogFile;
    fd_recLogInfoFile = pEggRecoveryHandle->fd_recLogInfoFile;

    lseek(fd_recLogInfoFile,  -(int)sizeof(eggLogInfo), SEEK_END);
    eggLogInfo t = {};
    READ(fd_recLogInfoFile, &t, sizeof(eggLogInfo));
    return t.Lsn;
}

static int logFile_check(EGGRECOVERYHANDLE *pEggRecoveryHandle)
{
    int fd_recLogFile, fd_recLogInfoFile;
    fd_recLogFile = pEggRecoveryHandle->fd_recLogFile;
    fd_recLogInfoFile = pEggRecoveryHandle->fd_recLogInfoFile;
    
//    static int isinit = 0;
    if (getFileSize(fd_recLogFile) 
        < (off_t)offsetof(eggLogRecord, fileName))
    {
        FTRUNCATE(fd_recLogFile, 0);
        FTRUNCATE(fd_recLogInfoFile, 0);
        lseek(fd_recLogFile, 0, SEEK_SET);
        eggLogRecord bufRecord = {};
        bufRecord.size = offsetof(eggLogRecord, fileName);
        WRITE(fd_recLogFile, &bufRecord, offsetof(eggLogRecord, fileName));
        lseek(fd_recLogInfoFile, 0, SEEK_SET);
        eggLogInfo t[2] = { 0, 0, offsetof(eggLogRecord, fileName), 0};
        WRITE(fd_recLogInfoFile, &t, sizeof(t));
    }

    return 0;
}

int eggRecoveryLog_writelog(EGGRECOVERYHANDLE *pEggRecoveryHandle,
                            ActInfo *actInfo,
                            uint32_t logType, char *fileName, uint64_t filePosition,
                            void *data, uint64_t size)
{
    if (!pEggRecoveryHandle)
    { return 0; }
    if (actInfo && !(logType == EGGRECOVERYLOG_COMMIT
                        || logType == EGGRECOVERYLOG_UNDO
                        || logType == EGGRECOVERYLOG_REDO))
    {
        return 0;
    }
    
    int fd_recLogFile = 0;
    int fd_recLogInfoFile = 0;
    if (!pEggRecoveryHandle || !(fd_recLogFile = pEggRecoveryHandle->fd_recLogFile)
        || !(fd_recLogInfoFile = pEggRecoveryHandle->fd_recLogInfoFile))
    {
        return 0;
    }

    int r;
    
    r = lockWrWait(fd_recLogFile, SEEK_SET, 0, 0);
    if (r < 0)
    {
        eggPrtLog_error("eggRecoveryLog", "Error: lockWrWait < 0: %s FAIL, EXIT", __func__);
        exit(-1);
    }
    
    pthread_mutex_lock(&s_writeMutex);
    
    writeLog(pEggRecoveryHandle,
             actInfo, logType, fileName, filePosition, data, size, 0);

    pthread_mutex_unlock(&s_writeMutex);
    
    unlock(fd_recLogFile, SEEK_SET, 0, 0);
    return 0;
}
int eggRecoveryLog_writelog_con(EGGRECOVERYHANDLE *pEggRecoveryHandle,
                               ActInfo *actInfo,
                            uint32_t logType, char *fileName, uint64_t filePosition,
                            void *data, uint64_t size)
{
    if (actInfo && !(logType == EGGRECOVERYLOG_COMMIT
                        || logType == EGGRECOVERYLOG_UNDO
                        || logType == EGGRECOVERYLOG_REDO))
    {
        return 0;
    }
    int fd_recLogFile = 0;
    int fd_recLogInfoFile = 0;
   if (!pEggRecoveryHandle || !(fd_recLogFile = pEggRecoveryHandle->fd_recLogFile)
        || !(fd_recLogInfoFile = pEggRecoveryHandle->fd_recLogInfoFile))
    {
        return 0;
    }

   int r;
   
   r = lockWrWait(fd_recLogFile, SEEK_SET, 0, 0);
   
   if (r < 0)
   {
       eggPrtLog_error("eggRecoveryLog", "Error: lockWrWait < 0: %s FAIL, EXIT", __func__);
       exit(-1);
   }


   pthread_mutex_lock(&s_writeMutex);
   
   writeLog(pEggRecoveryHandle,
            actInfo, logType, fileName, filePosition, data, size, 1);
   
   pthread_mutex_unlock(&s_writeMutex);
   
   unlock(fd_recLogFile, SEEK_SET, 0, 0);
    
    return 0;
}
int writeLog(EGGRECOVERYHANDLE *pEggRecoveryHandle,
             ActInfo *actInfo,
             uint32_t logType, char *fileName, uint64_t filePosition,
             void *data, uint64_t size, int ifconfirmed)
{
    int fd_recLogFile, fd_recLogInfoFile;
    fd_recLogFile = pEggRecoveryHandle->fd_recLogFile;
    fd_recLogInfoFile = pEggRecoveryHandle->fd_recLogInfoFile;
    
    MEASURE_START
    
    static eggLogRecord bufRecord = {};
    memset(&bufRecord, 0, sizeof(bufRecord));
    bufRecord.size = offsetof(eggLogRecord, fileName);
    if (actInfo)
    {
        eggLogRecord *p = (eggLogRecord *)&bufRecord;
        p->actId = actInfo->actId;
        p->logType = logType;    
        if (p->logType == EGGRECOVERYLOG_UNDO)
        {
            p->prevLsn = actInfo->lastLsn;
            p->undoNextLsn = actInfo->undoNextLsn;
            actInfo->lastLsn = getNextLsn(pEggRecoveryHandle);
            actInfo->undoNextLsn = actInfo->lastLsn;
        }
        else if (p->logType == EGGRECOVERYLOG_REDO)
        {
            p->prevLsn = actInfo->lastLsn;
            p->undoNextLsn = actInfo->undoNextLsn;
            actInfo->lastLsn = getNextLsn(pEggRecoveryHandle);
        }
        else if (p->logType == EGGRECOVERYLOG_COMMIT)
        {
            p->prevLsn = actInfo->lastLsn;
            p->undoNextLsn = actInfo->undoNextLsn;
            actInfo->lastLsn = 0;
            actInfo->undoNextLsn = 0;
        }
    }
    if (fileName && fileName[0] && size > 0)
    {
        bufRecord.size += strlen(fileName) + 1 + size;
        bufRecord.filePosition = filePosition;
    }
    
    
    off_t Lsn;
    ssize_t nw = 0, nn = 0;
    Lsn = lseek(fd_recLogFile, 0, SEEK_END);
    nw = WRITE(fd_recLogFile, &bufRecord, offsetof(eggLogRecord, fileName));
    assert(nw > 0);
    if (fileName && fileName[0] && size > 0)
    {
        nn = WRITE(fd_recLogFile, fileName, strlen(fileName)+1);
        assert(nn > 0);
        nw += nn;
        nn = WRITE(fd_recLogFile, data, size);
	assert(nn > 0);
        nw += nn;
    }

    /* update info */
    lseek(fd_recLogInfoFile, -(int)sizeof(eggLogInfo), SEEK_END);
    eggLogInfo t[2] = {};
    if (0)
    {
        READ(fd_recLogInfoFile, &t[0], sizeof(eggLogInfo));
        assert(t[0].Lsn == Lsn);
        lseek(fd_recLogInfoFile, -(int)sizeof(eggLogInfo), SEEK_END);
    }
    t[0].Lsn = Lsn;
    t[0].flag = ifconfirmed ? LOGFLAG_WRITTEN : 0;
    t[1].Lsn = Lsn + nw;
    WRITE(fd_recLogInfoFile, &t, sizeof(t));
    MEASURE_END;
    if (0)
    {
//     printf("@%llu:%llu>%s:%llu:%llu:%c\n", (long long unsigned)t[0].Lsn, (long long unsigned)t[1].Lsn, fileName, (long long unsigned)filePosition, (long long unsigned)size, (char)logType);
        eggPrtLog_info("eggRecoveryLog", "@%llu:%llu>%s:%llu:%llu:%c\n", (long long unsigned)t[0].Lsn, (long long unsigned)t[1].Lsn, fileName, (long long unsigned)filePosition, (long long unsigned)size, (char)logType);     
    }
    return 0;
}

static int markCheckpoint(EGGRECOVERYHANDLE *pEggRecoveryHandle)
{
    int fd_recLogFile, fd_recLogInfoFile;
    fd_recLogFile = pEggRecoveryHandle->fd_recLogFile;
    fd_recLogInfoFile = pEggRecoveryHandle->fd_recLogInfoFile;

    eggLogInfo t[3] = {};
    off_t lastpos;
    lastpos = lseek(fd_recLogInfoFile, - 2 * (int)sizeof(eggLogInfo), SEEK_END);
    if (lastpos < 0)
    {                           /* just after logFile formatted */
        t[0].flag = LOGFLAG_CHECKPOINT;
        t[1].Lsn = t[0].Lsn = offsetof(eggLogRecord, fileName);
        lseek(fd_recLogInfoFile, 0, SEEK_SET);
        WRITE(fd_recLogInfoFile, &t, 2 * sizeof(eggLogInfo));
        return 0;
    }
    READ(fd_recLogInfoFile, &t[0], 2 * sizeof(eggLogInfo));
    if (t[0].flag == LOGFLAG_CHECKPOINT)
    {                           /* just after last checkpoint */
        return 0;
    }
    t[1].flag = LOGFLAG_CHECKPOINT;
    t[2].Lsn = t[1].Lsn;
    lseek(fd_recLogInfoFile, lastpos, SEEK_SET);
    WRITE(fd_recLogInfoFile, &t, sizeof(t));
    return 0;
}

static uint32_t getNewActId(EGGRECOVERYHANDLE *pEggRecoveryHandle)
{
    int fd_recLogFile, fd_recLogInfoFile;
    fd_recLogFile = pEggRecoveryHandle->fd_recLogFile;
    fd_recLogInfoFile = pEggRecoveryHandle->fd_recLogInfoFile;
    
    if (fd_recLogFile == 0 || fd_recLogInfoFile == 0)
    {
        return 0;
    }

    int ret;
    
    ret = lockWrWait(fd_recLogFile, SEEK_SET, 0, offsetof(eggLogRecord, fileName));
    
    if (ret < 0)
    {
        eggPrtLog_error("eggRecoveryLog", "Error: lockWrWait < 0: %s FAIL, EXIT", __func__);
        exit(-1);
    }


    pthread_mutex_lock(&s_writeMutex);
    
    eggLogRecord bufRecord = {};
    lseek(fd_recLogFile, 0, SEEK_SET);
    READ(fd_recLogFile, &bufRecord, offsetof(eggLogRecord, fileName));
    uint32_t r;
    r = ++bufRecord.actId;
    lseek(fd_recLogFile, 0, SEEK_SET);
    WRITE(fd_recLogFile, &bufRecord, offsetof(eggLogRecord, fileName));

    pthread_mutex_unlock(&s_writeMutex);
    
    unlock(fd_recLogFile, SEEK_SET, 0, offsetof(eggLogRecord, fileName));

    return r;
}

static uint64_t getFileSize(int fd)
{
    struct stat st;
    fstat(fd, &st);
    return st.st_size;
}

static int lockWrWait(int fd, short whence, off_t start, off_t len)
{
    struct flock flck;
    if (whence == SEEK_END)
    {
        int ret;
        while ((ret = lockWrTry(fd, whence, start, len)) < 0)
        {
            ;
        }
        return ret;
    }
    else
    {
        flck.l_type = F_WRLCK;
        flck.l_whence = whence;
        flck.l_start = start;
        flck.l_len = len;
        int r;
        r = fcntl(fd, F_SETLKW, &flck);
        if (r < 0)
        {
            eggPrtLog_error("eggRecoveryLog", "%s:%d:%s fd[%d] whence[%hd] start[%lu] len[%lu] Error: [%s]", __FILE__, __LINE__, __func__, fd, whence, (long unsigned)start, (long unsigned)len, strerror(errno));
        }
        return r;
    }
}
static int lockRdTry(int fd, short whence, off_t start, off_t len)
{
    struct flock flck;
    flck.l_type = F_RDLCK;
    flck.l_whence = whence;
    flck.l_start = start;
    flck.l_len = len;
    int r;
    r = fcntl(fd, F_SETLK, &flck);
    if (r < 0)
    {
        eggPrtLog_info("eggRecoveryLog", "%s:%d:%s fd[%d] whence[%hd] start[%lu] len[%lu] Error: [%s]", __FILE__, __LINE__, __func__, fd, whence, (long unsigned)start, (long unsigned)len, strerror(errno));
    }
    return r;
}
static int lockRdWait(int fd, short whence, off_t start, off_t len)
{
    struct flock flck;
    if (whence == SEEK_END)
    {
        int ret;
        while ((ret = lockRdTry(fd, whence, start, len)) < 0)
        {
            ;
        }
        return ret;
    }
    else
    {
        flck.l_type = F_RDLCK;
        flck.l_whence = whence;
        flck.l_start = start;
        flck.l_len = len;
        int r;
        r = fcntl(fd, F_SETLKW, &flck);
        if (r < 0)
        {
            eggPrtLog_error("eggRecoveryLog", "%s:%d:%s fd[%d] whence[%hd] start[%lu] len[%lu] Error: [%s]", __FILE__, __LINE__, __func__, fd, whence, (long unsigned)start, (long unsigned)len, strerror(errno));            
        }
        return r;
    }
}
static int lockWrTry(int fd, short whence, off_t start, off_t len)
{
    struct flock flck;
    flck.l_type = F_WRLCK;
    flck.l_whence = whence;
    flck.l_start = start;
    flck.l_len = len;
    int r;
    r = fcntl(fd, F_SETLK, &flck);
    if (r < 0)
    {

        eggPrtLog_info("eggRecoveryLog", "%s:%d:%s fd[%d] whence[%hd] start[%lu] len[%lu] Error: [%s]", __FILE__, __LINE__, __func__, fd, whence, (long unsigned)start, (long unsigned)len, strerror(errno));
    }
    return r;
}
static int unlock(int fd, short whence, off_t start, off_t len)
{
    struct flock flck;
    flck.l_type = F_UNLCK;
    flck.l_whence = whence;
    flck.l_start = start;
    flck.l_len = len;
    int r;
    r = fcntl(fd, F_SETLK, &flck);
    if (r < 0)
    {

        eggPrtLog_error("eggRecoveryLog", "%s:%d:%s fd[%d] whence[%hd] start[%lu] len[%lu] Error: [%s]", __FILE__, __LINE__, __func__, fd, whence, (long unsigned)start, (long unsigned)len, strerror(errno));        
    }
    return r;
}

static inline int eggRecoveryLog_getfd(char *baseName,
                                   int *fd_recLogFile,
                                   int *fd_recLogInfoFile)
{
    if (!baseName || !baseName[0])
    {
        *fd_recLogFile = 0;
        *fd_recLogInfoFile = 0;
        return -1;
    }
    
    char *name = calloc(1, strlen(baseName) + 20);
    assert(name);
    char *p = 0;
    p = strrchr(baseName, '.');
    if (!p)
    {
        p = baseName + strlen(baseName);
    }
    sprintf(name, "%.*s%s", (int)(p-baseName), baseName, ".rlog");
    *fd_recLogFile = open(name, O_CREAT | O_RDWR, 0666);

    if (*fd_recLogFile < 0)
    {
//        fprintf(stderr, "[%s]: %s [%s]\n", __func__, baseName, strerror(errno));
        eggPrtLog_error("eggRecoveryLog", "[%s]: open[%s] ERROR: %s\n", __func__, name, strerror(errno));
        free(name);
        *fd_recLogFile = 0;
        *fd_recLogInfoFile = 0;
        return -1;
    }
    eggPrtLog_info("eggRecoveryLog", "[%s]: open[%s] fd[%d]\n", __func__, name, *fd_recLogFile);

    
    sprintf(name, "%.*s%s", (int)(p-baseName), baseName, ".rlog.info");    
    *fd_recLogInfoFile = open(name, O_CREAT | O_RDWR, 0666);
    if (*fd_recLogInfoFile < 0)
    {
        //fprintf(stderr, "[%s]: %s [%s]\n", __func__, baseName, strerror(errno));
        eggPrtLog_error("eggRecoveryLog", "[%s]: open[%s] ERROR: %s\n", __func__, name, strerror(errno));
        free(name);
        close(*fd_recLogFile);
        eggPrtLog_info("eggRecoveryLog", "[%s]: close fd[%d]\n", __func__, *fd_recLogFile);
        *fd_recLogFile = 0;
        *fd_recLogInfoFile = 0;
        return -1;
    }
    eggPrtLog_info("eggRecoveryLog", "[%s]: open[%s] fd[%d]\n", __func__, name, *fd_recLogInfoFile);

    
    free(name);
    return 0;
}

int eggRecoveryLog_readloginfo(char *baseName)
{
    int fd_recLogFile = 0;
    int fd_recLogInfoFile = 0;
    if (eggRecoveryLog_getfd(baseName,
                             &fd_recLogFile,
                             &fd_recLogInfoFile) < 0)
    {
        return -1;
    }

    printf("[%s].rlog.info: \n", baseName);
    off_t sz = getFileSize(fd_recLogInfoFile);
    lseek(fd_recLogInfoFile, 0, SEEK_SET);
    if (sz == 0)
    {
        close(fd_recLogFile);
        close(fd_recLogInfoFile);
        return 0;
    }
    eggLogInfo *p;
    size_t pSz = sz;
    p = (eggLogInfo *)MALLOC(pSz);
    assert(p);
    lseek(fd_recLogInfoFile, 0, SEEK_SET);
    READ(fd_recLogInfoFile, p, sz);
    int i, n;
    n = sz /sizeof(eggLogInfo);
    if (sz % sizeof(eggLogInfo))
    {
        printf(".rlog.info Not align with %d\n", (int)sizeof(eggLogInfo));
    }
    for (i = 0; i < n; i++)
    {
        printf("%020llu - %c\n", (long long unsigned)p[i].Lsn,
			   (char)p[i].flag);
    }
    FREE(p, pSz);
    printf("\n");
    close(fd_recLogFile);
    close(fd_recLogInfoFile);
    return 0;
}

int eggRecoveryLog_readlog(char *baseName, int readAll)
{
    
    HEGGRECOVERYHANDLE pEggRecoveryHandle = eggRecoveryHandle_new(baseName);
    if (!pEggRecoveryHandle)
    {
        return -1;
    }
    int fd_recLogFile = pEggRecoveryHandle->fd_recLogFile;
    int fd_recLogInfoFile = pEggRecoveryHandle->fd_recLogInfoFile;
    
    printf("[%s].rlog: \n", baseName);
    eggLogRecord tmpRecord;
    eggLogRecord *p = 0;    
    lseek(fd_recLogFile, 0, SEEK_SET);
    uint64_t Lsn;
    Lsn = lseek(fd_recLogFile, 0, SEEK_CUR);

    if (readAll)
    {
        while ((p = getNextRecord(pEggRecoveryHandle, 0,
                    &tmpRecord)))
        {
            printf("%020llu", (long long unsigned)Lsn);
            printf(" Prev[%020llu] UndoNext[%020llu]\n"
                   "                     Type[%c] ActId[%010u]\n"
                   "                     Pos[%020llu] Sz[%020llu] %s\n",
                   (long long unsigned)p->prevLsn,
				   (long long unsigned)p->undoNextLsn,
                   (char)p->logType, p->actId,
                   (long long unsigned)p->filePosition,
				   (long long unsigned)p->size,
                   p->fileName ? p->fileName : "");
            freeRecord(p);
            printf("\n");
            Lsn = lseek(fd_recLogFile, 0, SEEK_CUR);
        } 
        printf("\n");
        close(fd_recLogFile);
        close(fd_recLogInfoFile);
        return 0;
    }
    
    printf("Input file offset. -1 to exit\n");
    for (;;)
    {
        Lsn = -1LLU;
        scanf("%llu", &Lsn);
        if (Lsn == -1LLU)
            break;
        lseek(fd_recLogFile, Lsn, SEEK_SET);
        if ((p = getNextRecord(pEggRecoveryHandle, 0,
                 &tmpRecord)))
        {
            printf("%020llu", (long long unsigned)Lsn);
            printf(" Prev[%020llu] UndoNext[%020llu]\n"
                   "                     Type[%c] ActId[%010u]\n"
                   "                     Pos[%020llu] Sz[%020llu] %s\n",
                   (long long unsigned)p->prevLsn,
				   (long long unsigned)p->undoNextLsn,
                   (char)p->logType, p->actId,
                   (long long unsigned)p->filePosition,
				   (long long unsigned)p->size,
                   p->fileName ? p->fileName : "");
            freeRecord(p);
            printf("\n");
            Lsn = lseek(fd_recLogFile, 0, SEEK_CUR);
        }
    }

    eggRecoveryHandle_delete(pEggRecoveryHandle);
    
    return 0;
}



int eggRecoveryLog_inspect(char *fileName)
{
    eggRecoveryLog_readloginfo(fileName);
    eggRecoveryLog_readlog(fileName, 1);
    return 0;
}

static int FTRUNCATE(int fd, off_t length)
{
    int ret;
    if ((ret = ftruncate(fd, length)) < 0)
    {
        //fprintf(stderr, "ftruncate[%s]\n", strerror(errno));
        eggPrtLog_error("eggRecoveryLog", "ftruncate[%s]\n", strerror(errno));
    }
    return ret;
}

static ssize_t READ(int fd, void *buf, size_t count)
{
    size_t cnt = count;
    ssize_t n = 0;
    while ((n = read(fd, buf, cnt)) > 0)    
    {
        buf += n;
        cnt -= n;
    } 
    if (n < 0)
    {
//        fprintf(stderr, "read[%s]\n", strerror(errno));
        eggPrtLog_error("eggRecoveryLog", "read[%s]\n", strerror(errno));
        return -1;
    }
    else if (cnt > 0)
    {
        // fprintf(stderr, "read incomplete need %d\n", (int)cnt);
        return count - cnt;
    }
    return count;
}

static ssize_t WRITE(int fd, const void *buf, size_t count)
{
    size_t cnt = count;
    ssize_t n;
    while ((n = write(fd, buf, cnt)) > 0)    
    {
        buf += n;
        cnt -= n;
    } 
    if (n < 0)
    {
        //fprintf(stderr, "write[%s]\n", strerror(errno));
        eggPrtLog_error("eggRecoveryLog", "write[%s]\n", strerror(errno));
        return -1;
    }
    else if (cnt > 0)
    {
        //fprintf(stderr, "write incomplete need %d\n", (int)cnt);
        eggPrtLog_error("eggRecoveryLog", "write incomplete need %d\n", (int)cnt);
        return count - cnt;
    }
    return count;
}

static void *MALLOC(size_t size)
{
    void *addr = NULL;    
    
    if (size < 1024 * 1024)
    {
        addr = malloc(size);
        if (!addr)
        {
            //fprintf(stderr, "malloc[%s]\n", strerror(errno));
            eggPrtLog_error("eggRecoveryLog", "malloc[%s]\n", strerror(errno));
        }
        return addr;
    }

    addr = mmap(0, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
    if (addr == (void *) -1)
    {
        //fprintf(stderr, "mmap[%s]\n", strerror(errno));
        eggPrtLog_error("eggRecoveryLog", "mmap[%s]\n", strerror(errno));
        addr = NULL;
    }
    return addr;
    
}

static void FREE(void *ptr, size_t size)
{
    if (!ptr)
        return;
    if (size < 1024 * 1024)
    {
        free(ptr);
        return;
    }
    if (munmap(ptr, size) == -1)
    {
        //fprintf(stderr, "munmap[%s]\n", strerror(errno));
        eggPrtLog_error("eggRecoveryLog", "munmap[%s]\n", strerror(errno));
    }
    return;
}



#else

struct ActInfo {};

HEGGRECOVERYHANDLE eggRecoveryLog_init(char *baseName)
{ return 0; }
int eggRecoveryLog_destroy(HEGGRECOVERYHANDLE pEggRecoveryHandle)
{ return 0; }

ActInfo *eggRecoveryLog_beginact(HEGGRECOVERYHANDLE pEggRecoveryHandle)
{ return 0; }
int eggRecoveryLog_endact(HEGGRECOVERYHANDLE pEggRecoveryHandle,
                          ActInfo *actInfo)
{ return 0; }
int eggRecoveryLog_writelog(HEGGRECOVERYHANDLE pEggRecoveryHandle,
                            ActInfo *actInfo, uint32_t logType,
                            char *fileName, uint64_t filePosition,
                            void *data, uint64_t size)
{ return 0; }
int eggRecoveryLog_writelog_con(HEGGRECOVERYHANDLE pEggRecoveryHandle,
                               ActInfo *actInfo, uint32_t logType,
                               char *fileName, uint64_t filePosition,
                               void *data, uint64_t size)
{ return 0; }
int eggRecoveryLog_make_checkpoint(HEGGRECOVERYHANDLE pEggRecoveryHandle)
{ return 0; }
int eggRecoveryLog_makeclean_checkpoint(HEGGRECOVERYHANDLE pEggRecoveryHandle)
{ return 0; }

#endif  /* EGGRECOVERYLOG */
