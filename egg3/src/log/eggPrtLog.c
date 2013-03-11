#define _FILE_OFFSET_BITS 64
#include "eggPrtLog.h"
#include "../conf/eggConfig.h"
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

#define LOGFILE_SIZE (100 * 1024*1024)
#define LOGFILE_NUM 5
struct eggPrtLog {
    pthread_mutex_t mutex;
    enum EGGPRTLOGLEVEL level;
    char *filename;
    FILE *fp;
};


HEGGPRTLOG g_eggPrtLog_handle = EGG_NULL;

EBOOL eggPrtLog_build()
{
    eggCfgVal* p_list_val;
    GList* list_value_head;
    
    p_list_val = eggConfig_get_cfg(g_config_handle, "logpath");
    list_value_head = g_list_first(p_list_val);
    if(list_value_head == EGG_NULL)
    {
        printf("no logpath is set\n");
        return EGG_FALSE;
    }
    char* logname = (char*)list_value_head->data;

    g_eggPrtLog_handle =  eggPrtLog_init(logname);
    if (!g_eggPrtLog_handle)
    {
        return EGG_FALSE;
    }

    int level = -1;
    p_list_val = eggConfig_get_cfg(g_config_handle, "loglevel");
    list_value_head = g_list_first(p_list_val);
    if(list_value_head)
    {
        char* loglevel = (char*)list_value_head->data;
        char *e;
        level = strtol(loglevel, &e, 10);
        if (e == loglevel)
        {
            level = -1;
        }
    }
    
    if (level < EGGPRTLOG_DEBUG || level > EGGPRTLOG_CLAIM)
    {
        level = EGGPRTLOG_INFO;
    }
    
    eggPrtLog_set_level(g_eggPrtLog_handle, level);
    return EGG_TRUE;
}
EBOOL eggPrtLog_destroy()
{
    int rt;
    rt =  eggPrtLog_uninit(g_eggPrtLog_handle);
    if (rt < 0)
    {
        return EGG_FALSE;
    }
    g_eggPrtLog_handle = EGG_NULL;
    return EGG_TRUE;
}


static const char *strlevel(enum EGGPRTLOGLEVEL level);
static int eggPrtLog_check(HEGGPRTLOG hLog);
static int eggPrtLog_openfile(HEGGPRTLOG hLog);
static int eggPrtLog_recyclelog(HEGGPRTLOG hLog);

static int eggPrtLog_openfile(HEGGPRTLOG hLog)
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

HEGGPRTLOG eggPrtLog_init(char *logfile)
{
    HEGGPRTLOG hLog;
    hLog = calloc(1, sizeof(*hLog));
    assert(hLog);
    pthread_mutex_init(&hLog->mutex, NULL);

    if (logfile && logfile[0])
    {
        hLog->filename = strdup(logfile);
        assert(hLog->filename);
    }
    eggPrtLog_openfile(hLog);
    
    time_t t = time(NULL);
    struct tm t_m;
    localtime_r(&t, &t_m);
    char t_s[50];
    snprintf(t_s, sizeof(t_s), "%04d-%02d-%02d %02d:%02d:%02d",
             t_m.tm_year+1900, t_m.tm_mon+1, t_m.tm_mday,
             t_m.tm_hour, t_m.tm_min, t_m.tm_sec);
    
    eggPrtLog_log_line(hLog, EGGPRTLOG_INFO, "eggPrtLog", "Start pid[%d] %s ",
                       (int)getpid(), t_s);
    return hLog;
}

int eggPrtLog_uninit(HEGGPRTLOG hLog)
{
    if (!hLog)
    {
        return 0;
    }
    time_t t = time(NULL);
    struct tm t_m;
    localtime_r(&t, &t_m);
    char t_s[50];
    snprintf(t_s, sizeof(t_s), "%04d-%02d-%02d %02d:%02d:%02d",
             t_m.tm_year+1900, t_m.tm_mon+1, t_m.tm_mday,
             t_m.tm_hour, t_m.tm_min, t_m.tm_sec);
    
    eggPrtLog_log_line(hLog, EGGPRTLOG_INFO, "eggPrtLog", "End pid[%d] %s ",
                       (int)getpid(), t_s);
    
    if (hLog->filename && hLog->fp)
    {
        fclose(hLog->fp);
    }
    free(hLog->filename);
    pthread_mutex_destroy(&hLog->mutex);
    free(hLog);
    return 0;
}

int eggPrtLog_set_level(HEGGPRTLOG hLog, enum EGGPRTLOGLEVEL loglevel)
{
    if (!hLog)
    {
        return 0;
    }
    hLog->level = loglevel;
    return 0;
}

static const char *strlevel(enum EGGPRTLOGLEVEL level)
{
    switch(level)
    {
    case EGGPRTLOG_DEBUG:
        return "DEBUG";
    case EGGPRTLOG_INFO:
        return "INFO";
    case EGGPRTLOG_WARN:
        return "WARN";
    case EGGPRTLOG_ERROR:
        return "ERROR";
    case EGGPRTLOG_CLAIM:
        return "CLAIM";
    default:
        return "";
        
    }
}
int eggPrtLog_log_line(HEGGPRTLOG hLog, enum EGGPRTLOGLEVEL level, char *who,
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
    eggPrtLog_check(hLog);

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
    
    free(fmt);
    pthread_mutex_unlock(&hLog->mutex);    
    return 0;
}

static off_t eggPrtLog_getfilesize(HEGGPRTLOG hLog)
{
    off_t offset_old, file_size;
    offset_old = ftello(hLog->fp);
    fseeko(hLog->fp, 0, SEEK_END);
    file_size = ftello(hLog->fp);
    fseeko(hLog->fp, offset_old, SEEK_SET);
    return file_size;
}
static int eggPrtLog_check(HEGGPRTLOG hLog)
{
    if (!hLog)
    {
        return 0;
    }
    if (!hLog->filename)
    {
        return 0;
    }       
    off_t file_size;
    file_size = eggPrtLog_getfilesize(hLog);
    if (file_size > LOGFILE_SIZE)
    {
        fclose(hLog->fp);
        eggPrtLog_recyclelog(hLog);
        truncate(hLog->filename, 0);
        eggPrtLog_openfile(hLog);
    }
    return 0;
}

static int eggPrtLog_recyclelog(HEGGPRTLOG hLog)
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



