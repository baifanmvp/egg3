#define _FILE_OFFSET_BITS 64
#include <features.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "File.h"
#include "FileLock.h"
#include "../log/eggPrtLog.h"
#include <stdint.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <pthread.h>

PRIVATE EBOOL EggFile_file_length(HEGGFILE hEggFile);


PUBLIC HEGGFILE EggFile_open(const path_t* filepath)
{
    HEGGFILE hEggFile = (HEGGFILE)calloc(1, sizeof(EGGFILE));
// It seems fopen64 is used to manipulates large file in cygwin. But there is no open64, so 
// I just remove O_LARGEFILE from cygwin version. I think most senario in windows is just 
// for testing purpose, so we don't promise large file support in cygwin now.
#ifdef __CYGWIN__ 
    if ((EggFile_object(hEggFile)->hFile = open(filepath, O_RDWR |O_CREAT, 0666)) == -1)
#else
    if ((EggFile_object(hEggFile)->hFile = open(filepath, O_RDWR | O_LARGEFILE|O_CREAT, 0666)) == -1)
#endif
    {
        //fprintf(stderr, "%s: [%s][%s]\n", __func__, strerror(errno), filepath);
        eggPrtLog_error("File", "%s: [%s][%s]\n", __func__, strerror(errno), filepath);
        free(hEggFile);
        exit(-1);
    }
    
    EggFile_object(hEggFile)->name = strdup(filepath);
    assert(EggFile_object(hEggFile)->name);
    EggFile_object(hEggFile)->hActInfo = 0;
    
    EggFile_file_length(hEggFile);
    return hEggFile;
}

PUBLIC EBOOL EggFile_close(HEGGFILE hEggFile)
{
    if (EggFile_is_object(hEggFile))
    {
#ifdef WIN32
#else
        close(EggFile_object(hEggFile)->hFile);
        free(EggFile_object(hEggFile)->name);
        free(hEggFile);
#endif
        return EGG_TRUE;
    }
    
    return EGG_FALSE;
}

PUBLIC EBOOL EggFile_read(HEGGFILE hEggFile, epointer ePointer, length_t nLen, offset64_t nOff)
{
    if (!EggFile_is_object(hEggFile))
    {
        return EGG_FALSE;
    }

    
#ifdef __CYGWIN__    
    if (lseek(EggFile_object(hEggFile)->hFile,
#else
    if (lseek64(EggFile_object(hEggFile)->hFile,
#endif
                          nOff,
                          SEEK_SET) == (offset64_t)-1)
    {
        /* fprintf(stderr, "%s:%d:%s lseek64 %llu %s", */
        /*         __FILE__, __LINE__, __func__, (long long unsigned)nOff, strerror(errno)); */
        eggPrtLog_error("File", "%s:%d:%s lseek64 %llu %s",
                __FILE__, __LINE__, __func__, (long long unsigned)nOff, strerror(errno));
    }
    
    
    epointer p;
    size_t n;
    ssize_t nn;
    p = ePointer;
    n = nLen;
    while(n > 0)
    {
        if ((nn = read(EggFile_object(hEggFile)->hFile, p, n)) < 0)
        {
            /* fprintf(stderr, "%s:%d:%s ERR read[%llu:%s]: %s\n", */
            /*         __FILE__, __LINE__, __func__, */
            /*         (long long unsigned)nLen, EggFile_object(hEggFile)->name, */
            /*         strerror(errno)); */
            eggPrtLog_error("File", "%s:%d:%s ERR read[%llu:%s]: %s\n",
                    __FILE__, __LINE__, __func__,
                    (long long unsigned)nLen, EggFile_object(hEggFile)->name,
                    strerror(errno));
            return EGG_FALSE;
        }
        else if (nn == 0)
        {
            break;
        }
        p += nn;
        n -= nn;
    }

    if (p < ePointer + nLen)
    {
        /* fprintf(stderr, "%s:%d:%s read not enought read[%llu]/totoal[%llu]\n", */
        /*         __FILE__, __LINE__, __func__, */
        /*         (long long unsigned)(p-ePointer), (long long unsigned)nLen); */
        eggPrtLog_error("File", "%s:%d:%s read not enought read[%llu]/totoal[%llu]\n",
                __FILE__, __LINE__, __func__,
                (long long unsigned)(p-ePointer), (long long unsigned)nLen);
        return EGG_FALSE;
    }

    
    return EGG_TRUE;    
    
}

PUBLIC EBOOL EggFile_update(HEGGFILE hEggFile)
{
    return EGG_FALSE;
}

PUBLIC EBOOL EggFile_write(HEGGFILE hEggFile, ecpointer ecPointer, length_t nLen, offset64_t nOff)
{
    if (!EggFile_is_object(hEggFile))
    {
        return EGG_FALSE;
    }
    
#ifdef WIN32
#else
    if (nLen == 0)
    {
        return EGG_TRUE;
    }
    
    EBOOL retv = EGG_TRUE;
#ifdef __CYGWIN__    
    lseek(EggFile_object(hEggFile)->hFile,
#else
    lseek64(EggFile_object(hEggFile)->hFile,
#endif
            nOff,
            SEEK_SET);

#ifdef EGGRECOVERYLOG

    if (EggFile_object(hEggFile)->hActInfo)
    {
        EGGFILE *pEggFile = (EGGFILE *)hEggFile;
        char *data;
        data = malloc(nLen);
        assert(data);
        read(pEggFile->hFile, data, nLen);
        if (pEggFile->name[0] == '/')
        {
            eggRecoveryLog_writelog(pEggFile->hEggRecoveryHandle,
                                    pEggFile->hActInfo, EGGRECOVERYLOG_UNDO,
                                pEggFile->name, nOff, data, nLen);
            
        }
        else
        {
            char pathname[1024] = {0};
            char *filename;
            getcwd(pathname, 1024);
            filename = malloc(strlen(pathname)+2+strlen(pEggFile->name));
            assert(filename);
            sprintf(filename, "%s/%s", pathname, pEggFile->name);
            eggRecoveryLog_writelog(pEggFile->hEggRecoveryHandle,
                                    pEggFile->hActInfo, EGGRECOVERYLOG_UNDO,
                                filename, nOff, data, nLen);
            free(filename);
        }
        free(data);

#ifdef __CYGWIN__    
        lseek(pEggFile->hFile, nOff, SEEK_SET);
#else
        lseek64(pEggFile->hFile, nOff, SEEK_SET);
#endif
    }
#endif

    ecpointer p;
    size_t n;
    p = ecPointer;
    n = nLen;
    while(n > 0)
    {
        ssize_t nn;
        if ((nn = write(EggFile_object(hEggFile)->hFile, p, n)) < 0)
        {
            /* fprintf(stderr, "%s:%d:%s ERR write[%llu:%s]: %s left: %llu %p %p\n", */
            /*         __FILE__, __LINE__, __func__, */
            /*         (long long unsigned)nLen, EggFile_object(hEggFile)->name, */
            /*         strerror(errno), (long long unsigned)n, ecPointer, p); */
            eggPrtLog_error("File", "%s:%d:%s ERR write[%llu:%s]: %s left: %llu %p %p\n",
                    __FILE__, __LINE__, __func__,
                    (long long unsigned)nLen, EggFile_object(hEggFile)->name,
                    strerror(errno), (long long unsigned)n, ecPointer, p);
            retv = EGG_FALSE;
            break;
        }
        p += nn;
        n -= nn;
    }
    
    
#ifdef EGGRECOVERYLOG
    if (EggFile_object(hEggFile)->hActInfo)
    {
        EGGFILE *pEggFile = (EGGFILE *)hEggFile;
        if (pEggFile->name[0] == '/')
        {
            eggRecoveryLog_writelog_con(pEggFile->hEggRecoveryHandle,
                                       pEggFile->hActInfo, EGGRECOVERYLOG_REDO,
                                   pEggFile->name, nOff, ecPointer, nLen);
        }
        else
        {
            char pathname[1024] = {0};
            char *filename;
            
            //pathname = get_current_dir_name();
            getcwd(pathname, 1024);
            filename = malloc(strlen(pathname)+2+strlen(pEggFile->name));
            assert(filename);
            sprintf(filename, "%s/%s", pathname, pEggFile->name);
            eggRecoveryLog_writelog_con(pEggFile->hEggRecoveryHandle,
                                       pEggFile->hActInfo, EGGRECOVERYLOG_REDO,
                                   filename, nOff, ecPointer, nLen);
            free(filename);
        }
    }
#endif

#endif

    EggFile_file_length(hEggFile);
    return retv;
}

PRIVATE int checkLock(int fd, short whence, off_t start, off_t len)
{
    struct flock flck;
    flck.l_type = F_WRLCK;
    flck.l_whence = whence;
    flck.l_start = start;
    flck.l_len = len;
    if (fcntl(fd, F_GETLK, &flck) < 0) {
        /* fprintf(stderr, "%s: %d whence[%d]start[%llu]len[%llu] ERROR %s \n", */
		/* __func__, fd, */
		/* (int)whence, (long long unsigned)start, */
		/* (long long unsigned)len, */
        /*         strerror(errno)); */
        eggPrtLog_error("File", "%s: %d whence[%d]start[%llu]len[%llu] ERROR %s \n",
		__func__, fd,
		(int)whence, (long long unsigned)start,
		(long long unsigned)len,
                strerror(errno));
	return -1;
    }
//    printf("%hd[W] %hd[R] %hd[U]\n", F_WRLCK, F_RDLCK, F_UNLCK);
    eggPrtLog_info("File", "%hd[W] %hd[R] %hd[U]\n", F_WRLCK, F_RDLCK, F_UNLCK);
    /* printf("Type[%hd] Whence[%hd] Start[%ld] Len[%ld] Pid[%d]\n", */
    /*        flck.l_type, flck.l_whence, flck.l_start, flck.l_len, flck.l_pid); */
    eggPrtLog_info("File", "Type[%hd] Whence[%hd] Start[%ld] Len[%ld] Pid[%d]\n",
           flck.l_type, flck.l_whence, flck.l_start, flck.l_len, flck.l_pid);
    return  0;
}

PUBLIC EBOOL EggFile_lock_wr_wait(HEGGFILE hEggFile, short whence, off_t start, off_t len)
{
    EGGFILE *ef = (EGGFILE*)hEggFile;
    struct flock flck;

    
    flck.l_type = F_WRLCK;
    flck.l_whence = whence;
    flck.l_start = start;
    flck.l_len = len;
    if (fcntl(ef->hFile, F_SETLKW, &flck) < 0)
    {
        /* fprintf(stderr, "%s: %s %d whence[%d]start[%llu]len[%llu] ERROR %s \n",  */
		/* __func__, ef->name, ef->hFile, */
		/* (int)whence, (long long unsigned)start,  */
		/* (long long unsigned)len, */
        /*         strerror(errno)); */
        eggPrtLog_error("File", "%s: %s %d whence[%d]start[%llu]len[%llu] ERROR %s \n", 
		__func__, ef->name, ef->hFile,
		(int)whence, (long long unsigned)start, 
		(long long unsigned)len,
                strerror(errno));
	checkLock(ef->hFile, whence, start, len);
        return EGG_FALSE;
    }
    
    return EGG_TRUE;
}
PUBLIC EBOOL EggFile_lock_wr_try(HEGGFILE hEggFile, short whence, off_t start, off_t len)
{
    EGGFILE *ef = (EGGFILE*)hEggFile;
    struct flock flck;
    flck.l_type = F_WRLCK;
    flck.l_whence = whence;
    flck.l_start = start;
    flck.l_len = len;
    if (fcntl(ef->hFile, F_SETLK, &flck) < 0)
    {
//        fprintf(stderr, "%s: ERROR %s\n", __func__, strerror(errno));
        eggPrtLog_info("File", "%s: %s %d whence[%d]start[%llu]len[%llu] ERROR %s \n", 
		__func__, ef->name, ef->hFile,
		(int)whence, (long long unsigned)start, 
		(long long unsigned)len,
                strerror(errno));
        
        checkLock(ef->hFile, whence, start, len);
        
        return EGG_FALSE;
    }
    return EGG_TRUE;
}
PUBLIC EBOOL EggFile_lock_rd_wait(HEGGFILE hEggFile, short whence, off_t start, off_t len)
{
    EGGFILE *ef = (EGGFILE*)hEggFile;
    struct flock flck;
    
    flck.l_type = F_RDLCK;
    flck.l_whence = whence;
    flck.l_start = start;
    flck.l_len = len;
    if (fcntl(ef->hFile, F_SETLKW, &flck) < 0)
    {
//        fprintf(stderr, "%s: ERROR %s\n", __func__, strerror(errno));

        eggPrtLog_error("File", "%s: %s %d whence[%d]start[%llu]len[%llu] ERROR %s \n", 
                        __func__, ef->name, ef->hFile,
                        (int)whence, (long long unsigned)start, 
                        (long long unsigned)len,
                        strerror(errno));
        checkLock(ef->hFile, whence, start, len);
        
        return EGG_FALSE;
    }
    
    return EGG_TRUE;
}
PUBLIC EBOOL EggFile_lock_rd_try(HEGGFILE hEggFile, short whence, off_t start, off_t len)
{
    EGGFILE *ef = (EGGFILE*)hEggFile;
    struct flock flck;
    flck.l_type = F_RDLCK;
    flck.l_whence = whence;
    flck.l_start = start;
    flck.l_len = len;
   if (fcntl(ef->hFile, F_SETLK, &flck) < 0)
    {

        eggPrtLog_info("File", "%s: %s %d whence[%d]start[%llu]len[%llu] ERROR %s \n", 
                        __func__, ef->name, ef->hFile,
                        (int)whence, (long long unsigned)start, 
                        (long long unsigned)len,
                        strerror(errno));
        checkLock(ef->hFile, whence, start, len);

        
        return EGG_FALSE;
    }
    return EGG_TRUE;    
}
PUBLIC EBOOL EggFile_unlock(HEGGFILE hEggFile, short whence, off_t start, off_t len)
{
    EGGFILE *ef = (EGGFILE*)hEggFile;
    struct flock flck;
    flck.l_type = F_UNLCK;
    flck.l_whence = whence;
    flck.l_start = start;
    flck.l_len = len;
    if (fcntl(ef->hFile, F_SETLK, &flck) < 0)
    {
        /* fprintf(stderr, "%s: %d ERROR %s\n", __func__, ef->hFile, */
        /*         strerror(errno)); */
        eggPrtLog_error("File", "%s: %s %d whence[%d]start[%llu]len[%llu] ERROR %s \n", 
                       __func__, ef->name, ef->hFile,
                       (int)whence, (long long unsigned)start, 
                       (long long unsigned)len,
                       strerror(errno));
        
        return EGG_FALSE;
        
    }

    return EGG_TRUE;
}

PUBLIC size64_t EggFile_size(HEGGFILE hEggFile)
{
    if (EggFile_is_object(hEggFile))
    {
        EggFile_file_length(hEggFile);
        return EggFile_object(hEggFile)->size;
    }

    return 0;
}

PUBLIC size64_t EggFile_truncate(HEGGFILE hEggFile, uint64_t sz)
{
    if (EggFile_is_object(hEggFile))
    {
        
        EGGFILE *ef = (EGGFILE*)hEggFile;
        ftruncate(ef->hFile, sz);
    }

    return 0;
}


PRIVATE EBOOL EggFile_file_length(HEGGFILE hEggFile)
{
    if (!EggFile_is_object(hEggFile))
    {
        return EGG_FALSE;
    }

#ifdef WIN32
#else
    struct stat st_file_stat;
    fstat(EggFile_object(hEggFile)->hFile, &st_file_stat);
    EggFile_object(hEggFile)->size = st_file_stat.st_size;
#endif

    return EGG_TRUE;
}


PUBLIC char *EggFile_name(HEGGFILE hEggFile)
{
    if (EggFile_is_object(hEggFile))
    {
        return EggFile_object(hEggFile)->name;
    }

    return 0;
}

PUBLIC EBOOL EggFile_startlog(HEGGFILE hEggFile)
{
    EGGFILE *pEggFile = (EGGFILE *)hEggFile;
    
    pEggFile->hActInfo = eggRecoveryLog_beginact(pEggFile->hEggRecoveryHandle);    
    return EGG_TRUE;
}
PUBLIC EBOOL EggFile_endlog(HEGGFILE hEggFile)
{
    EGGFILE *pEggFile = (EGGFILE *)hEggFile;
    
    eggRecoveryLog_endact(pEggFile->hEggRecoveryHandle, pEggFile->hActInfo);
    pEggFile->hActInfo = 0;    
    return EGG_TRUE;
}
PUBLIC EBOOL EggFile_set_actinfo(HEGGFILE hEggFile,
                                ActInfo *hActInfo)
{
    EGGFILE *pEggFile = (EGGFILE *)hEggFile;
    
    pEggFile->hActInfo = hActInfo;
    return EGG_TRUE;
}
PUBLIC ActInfo *EggFile_get_actinfo(HEGGFILE hEggFile)
{
    EGGFILE *pEggFile = (EGGFILE *)hEggFile;
    return pEggFile->hActInfo;
}
PUBLIC HEGGRECOVERYHANDLE EggFile_get_recoveryhandle(HEGGFILE hEggFile)
{
    EGGFILE *pEggFile = (EGGFILE *)hEggFile;
    
    return pEggFile->hEggRecoveryHandle;
}
