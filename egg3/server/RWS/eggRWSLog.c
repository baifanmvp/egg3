#define _FILE_OFFSET_BITS 64
#include "eggRWSLog.h"
#include <bzlib.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#define LOGFILE_SIZE (16 * 1024*1024)
#define LOGFILE_NUM 5
struct eggRWSLog {
    pthread_mutex_t mutex;
    enum EGGRWSLOGLEVEL level;
    char *filename;
    FILE *fp;
};

static const char *strlevel(enum EGGRWSLOGLEVEL level);
static int eggRWSLog_check(HEGGRWSLOG hLog);
static int eggRWSLog_openfile(HEGGRWSLOG hLog);
static int eggRWSLog_recyclelog(HEGGRWSLOG hLog);

static int eggRWSLog_openfile(HEGGRWSLOG hLog)
{
    if (!hLog)
    {
        return 0;
    }
    if (hLog->filename && hLog->filename[0])
    {
        hLog->fp = fopen(hLog->filename, "a+");
        if (!hLog->fp)
        {
            goto use_default;
        }
    }
    else
    {
    use_default:
        free(hLog->filename);
        hLog->filename = NULL;
        hLog->fp = stderr;
    }
    return 0;
}

HEGGRWSLOG eggRWSLog_init(char *logfile)
{
    HEGGRWSLOG hLog;
    hLog = calloc(1, sizeof(*hLog));
    assert(hLog);
    pthread_mutex_init(&hLog->mutex, NULL);

    if (logfile && logfile[0])
    {
        hLog->filename = strdup(logfile);
        assert(hLog->filename);
    }
    eggRWSLog_openfile(hLog);
    
    time_t t = time(NULL);
    eggRWSLog_log_line(hLog, EGGRWSLOG_INFO, "eggRWSLog", "\n\n===========================START pid[%d] %s ",
                       (int)getpid(), ctime(&t));
    return hLog;
}

int eggRWSLog_uninit(HEGGRWSLOG hLog)
{
    if (!hLog)
    {
        return 0;
    }
    time_t t = time(NULL);
    eggRWSLog_log_line(hLog, EGGRWSLOG_INFO, "eggRWSLog", "\n\n===========================END pid[%d] %s ",
                       (int)getpid(), ctime(&t));
    
    if (hLog->filename && hLog->fp)
    {
        fclose(hLog->fp);
    }
    free(hLog->filename);
    pthread_mutex_destroy(&hLog->mutex);
    free(hLog);
    return 0;
}

int eggRWSLog_setLevel(HEGGRWSLOG hLog, enum EGGRWSLOGLEVEL loglevel)
{
    if (!hLog)
    {
        return 0;
    }
    hLog->level = loglevel;
    return 0;
}

static const char *strlevel(enum EGGRWSLOGLEVEL level)
{
    switch(level)
    {
    case EGGRWSLOG_INFO:
        return "INFO";
    case EGGRWSLOG_WARN:
        return "WARN";
    case EGGRWSLOG_ERROR:
        return "ERROR";
    case EGGRWSLOG_CLAIM:
        return "CLAIM";
    default:
        return "";
        
    }
}
int eggRWSLog_log_line(HEGGRWSLOG hLog, enum EGGRWSLOGLEVEL level, char *who,
                       const char *format, ...)
{
    if (!hLog)
    {
        return 0;
    }
    if (level < hLog->level)
    {
        return 0;
    }
    pthread_mutex_lock(&hLog->mutex);
    eggRWSLog_check(hLog);

    struct timeval tv;
    gettimeofday(&tv, NULL);
    char *fmt;
    fmt = malloc(strlen(who) + strlen(format) + 100);
    sprintf(fmt, "[%f:%lu:%d][%s][%s]: %s\n", tv.tv_sec+tv.tv_usec/1000000.,
            (long unsigned)pthread_self(), (int)getpid(), strlevel(level), who, format);
    
    va_list ap;
    va_start(ap, format);
    vfprintf(hLog->fp, fmt, ap);
    va_end(ap);
    fflush(hLog->fp);
    
    /* va_start(ap, format);     */
    /* vfprintf(stderr, fmt, ap); */
    /* va_end(ap); */
    /* fflush(stderr);     */

    free(fmt);
    pthread_mutex_unlock(&hLog->mutex);    
    return 0;
}

static off_t eggRWSLog_getfilesize(HEGGRWSLOG hLog)
{
    off_t offset_old, file_size;
    offset_old = ftello(hLog->fp);
    fseeko(hLog->fp, 0, SEEK_END);
    file_size = ftello(hLog->fp);
    fseeko(hLog->fp, offset_old, SEEK_SET);
    return file_size;
}
static int eggRWSLog_check(HEGGRWSLOG hLog)
{
    if (!hLog)
    {
        return 0;
    }
    off_t file_size;
    file_size = eggRWSLog_getfilesize(hLog);
    if (file_size > LOGFILE_SIZE)
    {
        fclose(hLog->fp);
        eggRWSLog_recyclelog(hLog);
        truncate(hLog->filename, 0);
        eggRWSLog_openfile(hLog);
    }
    return 0;
}

static int eggRWSLog_recyclelog(HEGGRWSLOG hLog)
{
    if (!(hLog->filename && hLog->filename[0]))
    {
        return 0;
    }

    int retv = 0;

    char *buf_tmp;
    buf_tmp = malloc(2 * (strlen(hLog->filename)+20) + 2);
    assert(buf_tmp);
    
    int i;
    for (i = LOGFILE_NUM - 1; i > 0; i--)
    {
        char *oldname, *newname;
        oldname = buf_tmp;
        sprintf(oldname, "%s.%d.bz2", hLog->filename, i);
        newname = buf_tmp + strlen(oldname) + 1;
        sprintf(newname, "%s.%d.bz2", hLog->filename, i+1);
        rename(oldname, newname);
    }
    char *archive_name;
    archive_name = buf_tmp;
    sprintf(archive_name, "%s.%d.bz2", hLog->filename, 1);
    
    
    FILE *fp_archive;
    fp_archive = fopen(archive_name, "wb");
    if (!fp_archive)
    {
        fprintf(stderr, "%s:%d:%s %s\n", __FILE__, __LINE__, __func__, strerror(errno));
        retv = -1;
        goto end;
    }

    FILE *fp_data;
    fp_data = fopen(hLog->filename, "rb");
    if (!fp_data)
    {
        fprintf(stderr, "%s:%d:%s %s\n", __FILE__, __LINE__, __func__, strerror(errno));
        retv = -1;
        goto end2;
    }

    BZFILE *bzfp;
    int bzerror;
    bzfp = BZ2_bzWriteOpen(&bzerror, fp_archive, 6, 0, 0);
    if ( bzerror != BZ_OK )
    {
        goto err;
    }
    char buf[1024];
    int not_end = 1;
    while (not_end)
    {
        int n_read;
        n_read = fread(buf, 1, sizeof(buf), fp_data);
        if (feof(fp_data))
        {
            not_end = 0;
        }
        else if (ferror(fp_data))
        {
            goto err;
        }
        
        BZ2_bzWrite(&bzerror, bzfp, buf, n_read);
        if (bzerror == BZ_IO_ERROR)
        {
            goto err;
        }
    }

err:
    BZ2_bzWriteClose(&bzerror, bzfp, 0, NULL, NULL);
    fclose(fp_data);
end2:
    fclose(fp_archive);
end:
    free(buf_tmp);
    return retv;
}
