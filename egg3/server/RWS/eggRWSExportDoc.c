#include <egg3/Egg3.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/file.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "eggRWSPackBuf.h"
char *config_statusfilename;
int config_optimizefreq;
int config_ifcontinue;
int config_ifoverride;
char *config_workdir;
offset64_t config_startdoc;
offset64_t config_enddoc;
int usage(char *prg)
{
    fprintf(stderr, "%-20s [--optimize-freq #] [--override] [--continue] [--workdir DIR] [--start-doc #] [--end-doc #] src_eggPath dst_eggPath [weight_field]\n", prg);
    fprintf(stderr, "\n");
    fprintf(stderr, "%-20s --start-doc #     : export from document No.#. default 1\n", "");
    fprintf(stderr, "%-20s --end-doc #       : export to document No.#. default 0 end of file\n", "");
    fprintf(stderr, "%-20s --optimize-freq # : optimize every # documents. default 5000\n", "");
    fprintf(stderr, "%-20s --continue        : resume interrupted work. default no\n", "");
    fprintf(stderr, "%-20s --override        : kill already run process. default no\n", "");
    fprintf(stderr, "%-20s --workdir DIR     : save status file (.%s) in DIR. default current dir\n", "",
            config_statusfilename);
    fprintf(stderr, "\n");
    fprintf(stderr, "%-20s eggPath           : e.g. tcp:10.0.0.1:10000/tmp/egg-data/, /tmp/egg-data/, ... \n", "");
    exit(0);
}
int config_init()
{
    config_statusfilename = strdup(".eggDocExport");
    assert(config_statusfilename);
    config_optimizefreq = 5000;
    config_ifcontinue = 0;
    config_workdir = realpath(".", NULL);
    assert(config_workdir);
    config_startdoc = 1;
    config_enddoc = (offset64_t)-1;
    return 0;
}
int config_uninit()
{
    free(config_statusfilename);
    free(config_workdir);
    return 0;
}
int readcommandline(int *argcp, char *argv[])
{

    config_init();

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
        if (strcmp(argv[i], "--optimize-freq") == 0)
        {
            if (i == argc - 1)
            {
                fprintf(stderr, "ERR --optimize-freq need a argument\n");
                usage(argv[0]);
            }
            int t;
            t = atoi(argv[i+1]);
            if (t > 0)
            {
                config_optimizefreq = t;
            }
            
            i += 2;
        }
        if (strcmp(argv[i], "--start-doc") == 0)
        {
            if (i == argc - 1)
            {
                fprintf(stderr, "ERR --start-doc need a argument\n");
                usage(argv[0]);
            }
            int t;
            t = atoi(argv[i+1]);
            if (t > 0)
            {
                config_startdoc = t;
            }
            
            i += 2;
        }
        if (strcmp(argv[i], "--end-doc") == 0)
        {
            if (i == argc - 1)
            {
                fprintf(stderr, "ERR --end-doc need a argument\n");
                usage(argv[0]);
            }
            int t;
            t = atoi(argv[i+1]);
            if (t > 0)
            {
                config_enddoc = t;
            }
            
            i += 2;
        }        
        else if (strcmp(argv[i], "--continue") == 0)
        {
            config_ifcontinue = 1;
            i += 1;
        }
        else if (strcmp(argv[i], "--override") == 0)
        {
            config_ifoverride = 1;
            i += 1;
        }
        else if (strcmp(argv[i], "--workdir") == 0)
        {
            if (i == argc - 1)
            {
                fprintf(stderr, "ERR --workdir need a argument\n");
                usage(argv[0]);
            }

            free(config_workdir);
            config_workdir = realpath(argv[i+1], NULL);
            assert(config_workdir);

            i += 2;
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


    return 0;
}

typedef struct EXPORTINGDICT EXPORTINGDICT;
EXPORTINGDICT *EXPORTINGDICT_new(char *workdir, char *statusfile_templatename, int isoverride,
                                 char *src_eggpath, char *dst_eggpath, offset64_t n_cursor_end);
int EXPORTINGDICT_delete(EXPORTINGDICT *hDict);
offset64_t EXPORTINGDICT_getresumedoc(EXPORTINGDICT *hDict);
offset64_t EXPORTINGDICT_setresumedoc(EXPORTINGDICT *hDict, offset64_t n_cursor_start);
int EXPORTINGDICT_reportdoc(EXPORTINGDICT *hDict,
                            offset64_t n_cursor, offset64_t n_cursor_end_real);



PRIVATE EBOOL eggRWSExport_process(HEGGINDEXWRITER hWriter, HEGGNETPACKAGE hNetPackage);

void sigPipeHandler(int signo)
{
    (void)signo;
    fprintf(stderr, "SIGPIPE signal !!!\n");
    return ;
}


int main(int argc, char** argv)
{
    printf("start export , pid %d\n",  getpid());
//    sleep(10);
    
    char *src_eggpath = NULL;
    char *dst_eggpath = NULL;
    char *weight_field = NULL;
    
    readcommandline(&argc, argv);
    if(argc <  3)
    {
        usage(argv[0]);
    }
    if (argc >= 4)
    {
        weight_field = argv[3];
    }
    
    src_eggpath = argv[1];
    dst_eggpath = argv[2];
    offset64_t n_cursor = config_startdoc;
    offset64_t n_cursor_end = config_enddoc;
    
    
    EXPORTINGDICT *hStatusDict = EXPORTINGDICT_new(config_workdir, config_statusfilename, config_ifoverride,
                                                   src_eggpath, dst_eggpath, n_cursor_end);

    if (config_ifcontinue)
    {
        n_cursor = EXPORTINGDICT_getresumedoc(hStatusDict);
    }
    n_cursor = 0;               /* eggRWSPackBuf_fetch不支持 */
    EXPORTINGDICT_setresumedoc(hStatusDict, n_cursor);
    printf("%s startdoc %llu endoc %llu, pid %d\n", src_eggpath,
           (long long unsigned)n_cursor, (long long unsigned)n_cursor_end, getpid());
    

//    HEGGHANDLE hSrcHandle = eggPath_open(src_eggpath);
    HEGGHANDLE hDstHandle = eggPath_open(dst_eggpath);
  
    HEGGRWSPACKBUF hRWSPackBuf =  eggRWSPackBuf_new(src_eggpath);
    
    HEGGINDEXWRITER hDstWriter = eggIndexWriter_open(hDstHandle, weight_field);

    HEGGDOCUMENT lp_eggDocument = EGG_NULL;

    
    size64_t n_process_sz = 0;
    offset64_t n_cursor_next = n_process_sz;
    HEGGNETPACKAGE lp_pack_res = EGG_NULL;
    count_t n_total_packsz = eggRWSPackBuf_get_fsize(hRWSPackBuf);;

    while (lp_pack_res = eggRWSPackBuf_fetch(hRWSPackBuf) )
    {

        EBOOL ret = eggRWSExport_process(hDstWriter, lp_pack_res);
        
        n_process_sz += (size64_t)eggNetPackage_get_packagesize(lp_pack_res);
      
	/*
        if (n_process_sz % config_optimizefreq == 0
            || n_cursor + n_process_sz - 1 == n_cursor_end
            || n_cursor + n_process_sz - 1 == n_total_packsz)
	*/
        if (n_process_sz > config_optimizefreq
            || n_cursor + n_process_sz == n_cursor_end
            || n_cursor + n_process_sz == n_total_packsz)
        {
            eggIndexWriter_optimize(hDstWriter);
            n_cursor += n_process_sz;
            n_process_sz = 0;
            EXPORTINGDICT_reportdoc(hStatusDict, n_process_sz, n_total_packsz);
        }
        
        
        eggNetPackage_delete(lp_pack_res);

    }

    printf("\nlast document %llu\n", (long long unsigned)n_cursor -1);

    
    eggIndexWriter_close(hDstWriter);
    eggRWSPackBuf_delete(hRWSPackBuf);
    
//    eggPath_close(hDstHandle);

    EXPORTINGDICT_delete(hStatusDict);
    return 0;
}


PRIVATE EBOOL eggRWSExport_process(HEGGINDEXWRITER hWriter, HEGGNETPACKAGE hNetPackage)
{

    if(POINTER_IS_INVALID(hWriter) || POINTER_IS_INVALID(hNetPackage))
    {
        return EGG_FALSE;
    }
    if(hNetPackage->op != EGG_PACKAGE_OPTIMIZE)
    {
        return EGG_FALSE;
    }
    
    size32_t n_iter_sz = 0;
    char* lp_data_str = (char*)(hNetPackage + 1);
    
    count_t a_cnt = 0;
    
    
    while(n_iter_sz != hNetPackage->eSize)
    {
        HEGGNETUNITPACKAGE lp_unit_package = (HEGGNETUNITPACKAGE)(lp_data_str + n_iter_sz);
        if(lp_unit_package->ty == EGG_PACKAGE_OPTIMIZE_ADD)
        {
            HEGGDOCNODE lp_doc_node = (HEGGDOCNODE)malloc(lp_unit_package->size);
            memcpy(lp_doc_node, lp_unit_package + 1, lp_unit_package->size);
            HEGGDOCUMENT lp_document = eggDocument_unserialization(lp_doc_node);
            
            eggIndexWriter_add_document(hWriter, lp_document);

            eggDocument_delete(lp_document);
	    // printf("count doc : %d\n", ++a_cnt);
        }
        else if(lp_unit_package->ty == EGG_PACKAGE_OPTIMIZE_DELETE)
        {
            EGGDID dId = {0};
            memcpy(&dId, lp_unit_package + 1, lp_unit_package->size);
            eggIndexWriter_delete_document(hWriter, dId );
            
        }
        else if(lp_unit_package->ty == EGG_PACKAGE_OPTIMIZE_MODIFY)   //modify
        {
            EGGDID dId = {0};
            memcpy(&dId, lp_unit_package + 1, sizeof(dId));

            HEGGDOCNODE lp_doc_node = (HEGGDOCNODE)malloc(lp_unit_package->size - sizeof(dId));
            memcpy(lp_doc_node, (char*)(lp_unit_package + 1) + sizeof(dId), lp_unit_package->size - sizeof(dId));
            HEGGDOCUMENT lp_document = eggDocument_unserialization(lp_doc_node);
            
            eggIndexWriter_modify_document(hWriter, dId, lp_document);
            
            //eggDocument_delete(lp_document);
        }
        else
        {
            EGGDID dId = {0};
            memcpy(&dId, lp_unit_package + 1, sizeof(dId));
            
            HEGGDOCNODE lp_doc_node = (HEGGDOCNODE)malloc(lp_unit_package->size - sizeof(dId));
            memcpy(lp_doc_node, (char*)(lp_unit_package + 1) + sizeof(dId), lp_unit_package->size - sizeof(dId));
            HEGGDOCUMENT lp_document = eggDocument_unserialization(lp_doc_node);
            
            eggIndexWriter_incrementmodify_document(hWriter, dId, lp_document);
            
//            eggDocument_delete(lp_document);
        }
        
        n_iter_sz += sizeof(EGGNETUNITPACKAGE) + lp_unit_package->size;
    }
    
    return EGG_TRUE;
}



struct EXPORTINGDICT {
    char *statusfile;
    FILE *fp;
    
    char *src_eggpath;
    char *dst_eggpath;
    offset64_t n_cursor_end;
    offset64_t n_cursor;
    time_t time_touch;
    pid_t pid;
    
    offset64_t n_cursor_start;
    int time_start;
    
};

int EXPORTINGDICT_flush(EXPORTINGDICT *hDict);

int EXPORTINGDICT_check(EXPORTINGDICT *hDict)
{
    if (!hDict)
    {
        return 0;
    }
    time_t now;
    now = time(NULL);
    if (hDict->time_touch < now && now - hDict->time_touch > 3600)
    {
        /* status file is bad */
        hDict->n_cursor_start = 1;
        hDict->n_cursor = 1;
    }
    else
    {
        hDict->n_cursor_start = hDict->n_cursor;
    }
    hDict->time_start = now;
    hDict->time_touch = now;
    hDict->pid = getpid();
    return 0;
}

int makevalid_filename(char *filename)
{
    char *p = filename;
    for (; *p; p++)
    {
        if (*p == '/')
        {
            *p = '_';
        }
    }
    return 0;
}
#define SKIPSPACE(p) while(isspace(*p)) p++;
#define JUMPTOSPACE(p) while(*p && !isspace(*p)) p++;
#define NEXTWORD(p) { JUMPTOSPACE(p); SKIPSPACE(p); }
EXPORTINGDICT *EXPORTINGDICT_new(char *workdir, char *statusfile_templatename, int isoverride,
                                 char *src_eggpath, char *dst_eggpath, offset64_t n_cursor_end)
{
    EXPORTINGDICT *hDict = calloc(1, sizeof(*hDict));
    assert(hDict);

    char *statusfile = NULL;
    statusfile = malloc(strlen(workdir) + 1 + strlen(statusfile_templatename)
                        + 1 + strlen(src_eggpath)
                        + 1 + strlen(dst_eggpath)
                        + 30);
    assert(statusfile);
    sprintf(statusfile, "%s/%s_%s_%s_%llu", workdir, statusfile_templatename,
            src_eggpath, dst_eggpath, (long long unsigned)n_cursor_end);
    makevalid_filename(statusfile + strlen(workdir)+1);
    hDict->statusfile = statusfile;

    FILE *fp;
    
    if (!(fp = fopen(statusfile, "r+")))
    {
        if (!(fp = fopen(statusfile, "w+")))
        {
            fprintf(stderr, "WARN cannot create %s: %s\n",
                    statusfile, strerror(errno));
            exit(-1);
            return NULL;
        }
    }
    hDict->fp = fp;
    
retry:
    fseek(fp, 0, SEEK_SET);
    size_t n_line_sz = 0;
    char* p_line_str = NULL;
    if (getline(&p_line_str, &n_line_sz, fp) != -1)
    {
        char *src_eggpath = NULL;
        char *dst_eggpath = NULL;
        offset64_t n_cursor_end = 0;
        offset64_t n_cursor = 0;
        time_t time_touch = 0;
        pid_t pid = 0;
        char *p, *q;
        p = q = p_line_str;
        
        src_eggpath = p;
        JUMPTOSPACE(p);
        q = p;
        NEXTWORD(p);
        *q = '\0';

        dst_eggpath = p;
        JUMPTOSPACE(p);
        q = p;
        NEXTWORD(p);
        *q = '\0';

        n_cursor_end = strtoull(p, &q, 10);
        SKIPSPACE(q);
        p = q;
        
        n_cursor = strtoull(p, &q, 10);
        SKIPSPACE(q);
        p = q;

        time_touch = strtol(p, &q, 10);
        SKIPSPACE(q);
        p = q;

        pid = strtol(p, &q, 10);
        SKIPSPACE(q);
        p = q;

        free(hDict->src_eggpath);
        hDict->src_eggpath = strdup(src_eggpath); assert(hDict->src_eggpath);
        free(hDict->dst_eggpath);        
        hDict->dst_eggpath = strdup(dst_eggpath); assert(hDict->dst_eggpath);
        hDict->time_touch = time_touch;
        hDict->n_cursor = n_cursor;
        hDict->n_cursor_end = n_cursor_end;
        hDict->pid = pid;
        
    }
    free(p_line_str);

    int fd;
    fd = fileno(fp);
    if (flock(fd, LOCK_EX|LOCK_NB) < 0)
    {
        if (isoverride)
        {
            if (kill(hDict->pid, SIGKILL) < 0)
            {
                fprintf(stderr, "already run pid[%d]. src_eggpath[%s] dst_eggpath[%s]. CANNOT kill : %s\n",
                        (int)hDict->pid, hDict->src_eggpath, hDict->dst_eggpath, strerror(errno));
                exit(-1);
            }
            waitpid(hDict->pid, NULL, 0);
            goto retry;
        }
        else
        {
            fprintf(stderr, "already run pid[%d]. src_eggpath[%s] dst_eggpath[%s].\n",
                    hDict->pid, hDict->src_eggpath, hDict->dst_eggpath);
            exit(-1);
        }
    }

    if (hDict->src_eggpath && strcmp(hDict->src_eggpath, src_eggpath) == 0
        && hDict->dst_eggpath && strcmp(hDict->dst_eggpath, dst_eggpath) == 0
        && hDict->n_cursor_end && hDict->n_cursor_end == n_cursor_end)
    {
        EXPORTINGDICT_check(hDict);
    }
    else
    {
        free(hDict->src_eggpath);
        hDict->src_eggpath = strdup(src_eggpath); assert(hDict->src_eggpath);
        free(hDict->dst_eggpath);
        hDict->dst_eggpath = strdup(dst_eggpath); assert(hDict->dst_eggpath);
        hDict->n_cursor_end = n_cursor_end;
        hDict->n_cursor = 1;
        hDict->n_cursor_start = 1;
        time_t now = time(NULL);
        hDict->time_start = now;
        hDict->time_touch = now;
        hDict->pid = getpid();
    }
    
    EXPORTINGDICT_flush(hDict);

    return hDict;
}
int EXPORTINGDICT_flush(EXPORTINGDICT *hDict)
{
    FILE *fp = hDict->fp;
    int fd;
    fd = fileno(fp);
    ftruncate(fd, 0);
    fseek(fp, 0, SEEK_SET);

    fprintf(fp, "%s %s %llu %llu %ld %d\n",
            hDict->src_eggpath, hDict->dst_eggpath,
            (long long unsigned)hDict->n_cursor_end,
            (long long unsigned)hDict->n_cursor,
            (long)hDict->time_touch,
            (int)hDict->pid);
    
    fflush(fp);
    return 0;
    
}
int EXPORTINGDICT_delete(EXPORTINGDICT *hDict)
{

    remove(hDict->statusfile);
    
    
    if (hDict->fp)
    {
        fclose(hDict->fp);
    }
    free(hDict->statusfile);
    free(hDict->src_eggpath);
    free(hDict->dst_eggpath);
    free(hDict);
    return 0;
}
offset64_t EXPORTINGDICT_getresumedoc(EXPORTINGDICT *hDict)
{
    
    return hDict->n_cursor_start;
}
offset64_t EXPORTINGDICT_setresumedoc(EXPORTINGDICT *hDict, offset64_t n_cursor_start)
{

    hDict->n_cursor = n_cursor_start;
    hDict->n_cursor_start = n_cursor_start;
    return 0;
}

int print_processing(offset64_t n_cursor_start, offset64_t n_cursor, offset64_t n_cursor_end_real,
                     time_t time_start, time_t time_now)
{
    static time_t time_print;
    
    int time_need = 0;
    if (time_start >= time_now || n_cursor == n_cursor_start)
    {
        time_need = -1;
    }
    else if (n_cursor >= n_cursor_end_real)
    {
        time_need = 0;
    }
    else
    {
        time_need = (n_cursor_end_real - n_cursor)
            * (time_now - time_start) / (n_cursor - n_cursor_start);
    }

    if (time_now - time_print > 5)
    {
        time_print = time_now;
        
        printf("[%10llu/%llu] eta [%10d]s\n",
               (long long unsigned)n_cursor, (long long unsigned)n_cursor_end_real,
               time_need);
    }
}

int EXPORTINGDICT_reportdoc(EXPORTINGDICT *hDict,
                            offset64_t n_cursor, offset64_t n_cursor_end_real)
{
    time_t time_touch = time(NULL);

    hDict->time_touch = time_touch;
    hDict->n_cursor = n_cursor;
    EXPORTINGDICT_flush(hDict);
    print_processing(hDict->n_cursor_start, hDict->n_cursor, n_cursor_end_real, hDict->time_start, hDict->time_touch);
    return 0;
}
