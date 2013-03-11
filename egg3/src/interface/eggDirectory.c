#include "../eggDirectory.h"
#include "../EggDef.h"
#include "../Egg3.h"
#include "eggIndexWriterLocal.h"
#include "eggIndexReaderLocal.h"
#include "eggIndexSearcherLocal.h"
#include "../eggHandle.h"
#include "../log/eggPrtLog.h"
#include "../uti/eggReg.h"
#include <errno.h>
#include <assert.h>

struct eggDirectory
{
    EGGHANDLE eggHandle;
    path_t* path;
};

#define EGGDIRECTORY_IS_INVALID(hEggDirectory)  \
    (!hEggDirectory ? EGG_TRUE : EGG_FALSE)

PRIVATE EBOOL eggDirectory_make_file(HEGGDIRECTORY hEggDirectory, file_t fid);
extern HEGGCONFIG g_config_handle;

PRIVATE char* eggDirectory_get_realpath(const path_t* filepath);

HEGGDIRECTORY EGGAPI eggDirectory_open(const path_t* filepath)
{
    char* lp_real_path = eggDirectory_get_realpath(filepath);
    if(!lp_real_path)
    {
        eggPrtLog_error("eggDirectory realpath is error", "[%s] [%s]\n", __func__, strerror(errno));
        
        return EGG_NULL;
    }
    HEGGDIRECTORY lp_directory = eggDirectory_load_path(lp_real_path);
    free(lp_real_path);
    return lp_directory;
}

HEGGDIRECTORY EGGAPI eggDirectory_load_path(const path_t* filepath)
{
    
    HEGGDIRECTORY lp_directory = (HEGGDIRECTORY)calloc(1, sizeof(EGGDIRECTORY));
    if (!lp_directory)
    {
        return EGG_NULL;
    }


//    length_t length = strlen(filepath);
    lp_directory->path = (path_t*)strdup(filepath);
    assert(lp_directory->path);
    
    if(access(lp_directory->path, F_OK) != 0)
    {
        return EGG_NULL;
    }
    
    

    lp_directory->eggHandle.eggHandle_dup = eggDirectory_dup;
    lp_directory->eggHandle.eggHandle_delete = eggDirectory_delete;
    lp_directory->eggHandle.eggHandle_getName = eggDirectory_get_name;
    
    lp_directory->eggHandle.eggIndexReader_open = eggIndexReader_open_local;
    lp_directory->eggHandle.eggIndexReader_close = eggIndexReader_close_local;
    lp_directory->eggHandle.eggIndexReader_get_document = eggIndexReader_get_document_local;
    lp_directory->eggHandle.eggIndexReader_get_documentset = eggIndexReader_get_documentset_local;
    lp_directory->eggHandle.eggIndexReader_export_document = eggIndexReader_export_document_local;
    lp_directory->eggHandle.eggIndexReader_get_doctotalcnt = eggIndexReader_get_doctotalcnt_local;
    lp_directory->eggHandle.eggIndexReader_get_fieldnameinfo = eggIndexReader_get_fieldnameinfo_local;
    lp_directory->eggHandle.eggIndexReader_get_singlefieldnameinfo = eggIndexReader_get_singlefieldnameinfo_local;
    lp_directory->eggHandle.eggIndexReader_free = eggIndexReader_free_local;
    
    lp_directory->eggHandle.eggIndexSearcher_new = eggIndexSearcher_new_local;
    lp_directory->eggHandle.eggIndexSearcher_delete = eggIndexSearcher_delete_local;
    lp_directory->eggHandle.eggIndexSearcher_search_with_queryiter = eggIndexSearcher_search_with_queryiter_local;
    lp_directory->eggHandle.eggIndexSearcher_get_queryiter = eggIndexSearcher_get_queryiter_local;
    lp_directory->eggHandle.eggIndexSearcher_search_with_query = eggIndexSearcher_search_with_query_local;
    lp_directory->eggHandle.eggIndexSearcher_count_with_query = eggIndexSearcher_count_with_query_local;
    lp_directory->eggHandle.eggIndexSearcher_filter = eggIndexSearcher_filter_local;
    
    lp_directory->eggHandle.eggIndexWriter_open = eggIndexWriter_open_local;
    lp_directory->eggHandle.eggIndexWriter_close = eggIndexWriter_close_local;
    lp_directory->eggHandle.eggIndexWriter_ref_indexcache = eggIndexWriter_ref_indexcache_local;    
    lp_directory->eggHandle.eggIndexWriter_set_analyzer = eggIndexWriter_set_analyzer_local;    
    lp_directory->eggHandle.eggIndexWriter_add_document = eggIndexWriter_add_document_local;
    lp_directory->eggHandle.eggIndexWriter_optimize = eggIndexWriter_optimize_local;
    lp_directory->eggHandle.eggIndexWriter_init_reader = eggIndexWriter_init_reader_local;
    lp_directory->eggHandle.eggIndexWriter_reindex_document = eggIndexWriter_reindex_document_local;
    lp_directory->eggHandle.eggIndexWriter_modify_document = eggIndexWriter_modify_document_local;
    lp_directory->eggHandle.eggIndexWriter_incrementmodify_document = eggIndexWriter_incrementmodify_document_local;    
    lp_directory->eggHandle.eggIndexWriter_delete_document = eggIndexWriter_delete_document_local;
    lp_directory->eggHandle.eggIndexWriter_add_field = eggIndexWriter_add_field_local;
    lp_directory->eggHandle.eggIndexWriter_modify_field = eggIndexWriter_modify_field_local;
    lp_directory->eggHandle.eggIndexWriter_delete_field = eggIndexWriter_delete_field_local;
    
    lp_directory->eggHandle.eggHandle_close = eggDirectory_close;

    
    return lp_directory;
}

HEGGDIRECTORY eggDirectory_dup(HEGGDIRECTORY hEggDirectory)
{
    if (!hEggDirectory)
    {
        return NULL;
    }
    return eggDirectory_load_path(hEggDirectory->path);
}

EBOOL eggDirectory_delete(HEGGDIRECTORY hEggDirectory)
{
    if (EGGDIRECTORY_IS_INVALID(hEggDirectory))
    {
        return EGG_FALSE;
    }


    free(hEggDirectory->path);
    free(hEggDirectory);

    return EGG_TRUE;
}

path_t* EGGAPI eggDirectory_get_name(HEGGDIRECTORY hEggDirectory)
{
    if (!hEggDirectory)
    {
        return EGG_NULL;
    }
    return hEggDirectory->path;
}

EBOOL EGGAPI eggDirectory_close(HEGGDIRECTORY hEggDirectory)
{
    int ret;
    ret = eggDirectory_delete(hEggDirectory);
    
    return ret;
}



EBOOL EGGAPI eggDirectory_init(HEGGDIRECTORY hEggDirectory)
{
    if (EGGDIRECTORY_IS_INVALID(hEggDirectory))
    {
        return EGG_FALSE;
    }


    echar rgsz_file_path[2048] = {0};

    sprintf(rgsz_file_path, "%s%s", hEggDirectory->path, EGG_FIDX);
    if(access(rgsz_file_path, F_OK) != 0)
    {
        file_t fid = open(rgsz_file_path, O_RDWR | O_CREAT, 00666);

        if (fid < 0)
        {
            //fprintf(stderr, "[%s] [%s][%s]\n", __func__, strerror(errno), rgsz_file_path);
            eggPrtLog_error("eggDirectory", "[%s] [%s][%s]\n", __func__, strerror(errno), rgsz_file_path);
        }
        
        eggDirectory_make_file(hEggDirectory, fid);
        
        close(fid);
    }

    sprintf(rgsz_file_path, "%s%s", hEggDirectory->path, EGG_FID);
    if(access(rgsz_file_path, F_OK) != 0)
    {
        file_t fid = open(rgsz_file_path, O_RDWR | O_CREAT, 00666);

        if (fid < 0)
        {
            //fprintf(stderr, "[%s] [%s][%s]\n", __func__, strerror(errno), rgsz_file_path);
            eggPrtLog_error("eggDirectory", "[%s] [%s][%s]\n", __func__, strerror(errno), rgsz_file_path);
        }

        eggDirectory_make_file(hEggDirectory, fid);
        
        close(fid);
    }

    sprintf(rgsz_file_path, "%s%s", hEggDirectory->path, EGG_FDAT);
    if(access(rgsz_file_path, F_OK) != 0)
    {
        file_t fid = open(rgsz_file_path, O_RDWR | O_CREAT, 00666);
        
        if (fid < 0)
        {
            //fprintf(stderr, "[%s] [%s][%s]\n", __func__, strerror(errno), rgsz_file_path);
            eggPrtLog_error("eggDirectory", "[%s] [%s][%s]\n", __func__, strerror(errno), rgsz_file_path);
        }
        
        eggDirectory_make_file(hEggDirectory, fid);
        
        close(fid);
    }

    sprintf(rgsz_file_path, "%s%s", hEggDirectory->path, EGG_FFD);
    if(access(rgsz_file_path, F_OK) != 0)
    {
        file_t fid = open(rgsz_file_path, O_RDWR | O_CREAT, 00666);
        
        if (fid < 0)
        {
            //fprintf(stderr, "[%s] [%s][%s]\n", __func__, strerror(errno), rgsz_file_path);
            eggPrtLog_error("eggDirectory", "[%s] [%s][%s]\n", __func__, strerror(errno), rgsz_file_path);
        }
        
        eggDirectory_make_file(hEggDirectory, fid);
        
        close(fid);
    }

    sprintf(rgsz_file_path, "%s%s", hEggDirectory->path, EGG_FFW);
    if(access(rgsz_file_path, F_OK) != 0)
    {
        file_t fid = open(rgsz_file_path, O_RDWR | O_CREAT, 00666);
        
        if (fid < 0)
        {
            //fprintf(stderr, "[%s] [%s][%s]\n", __func__, strerror(errno), rgsz_file_path);
            eggPrtLog_error("eggDirectory", "[%s] [%s][%s]\n", __func__, strerror(errno), rgsz_file_path);
        }
        
        eggDirectory_make_file(hEggDirectory, fid);
        
        close(fid);
    }    

    sprintf(rgsz_file_path, "%s%s", hEggDirectory->path, EGG_FDAT_IDT);
    if(access(rgsz_file_path, F_OK) != 0)
    {
        file_t fid = open(rgsz_file_path, O_RDWR | O_CREAT, 00666);
        
        if (fid < 0)
        {
            //fprintf(stderr, "[%s] [%s][%s]\n", __func__, strerror(errno), rgsz_file_path);
            eggPrtLog_error("eggDirectory", "[%s] [%s][%s]\n", __func__, strerror(errno), rgsz_file_path);
        }
        
//        eggDirectory_make_file(hEggDirectory, fid);
        
        close(fid);
    }

    return EGG_TRUE;
}

HEGGFILE EGGAPI eggDirectory_get_file(HEGGDIRECTORY hEggDirectory, const echar* lpFileName)
{
    if (EGGDIRECTORY_IS_INVALID(hEggDirectory))
    {
        return EGG_NULL;
    }

    
    echar rgsz_file_path[2048] = {0};

    sprintf(rgsz_file_path, "%s%s", hEggDirectory->path, lpFileName);
    
    if(access(rgsz_file_path, F_OK) != 0)
    {
        return EGG_NULL;
    }
    return EggFile_open(rgsz_file_path);
}


PRIVATE EBOOL eggDirectory_make_file(HEGGDIRECTORY hEggDirectory, file_t fid)
{
    epointer lp_init_tmp = (epointer)malloc(MAP_VIEW_OFFSET);
    memset(lp_init_tmp, 0, MAP_VIEW_OFFSET);
#ifdef __CYGWIN__
    lseek(fid, 0, SEEK_SET);
#else
    lseek64(fid, 0, SEEK_SET);
#endif
    size16_t nRet = write(fid, lp_init_tmp, MAP_VIEW_OFFSET);

    if (nRet < 0)
    {
        //fprintf(stderr, "[%s] [%s]\n", __func__, strerror(errno));
        eggPrtLog_error("eggDirectory", "[%s] [%s]\n", __func__, strerror(errno));
    }

    free(lp_init_tmp);
    
    return EGG_TRUE;
}


PRIVATE char* eggDirectory_get_realpath(const path_t* filepath)
{
    char* lp_real_path = (char*)malloc(PATH_MAX+1);
    assert(lp_real_path);
    memset(lp_real_path, 0, PATH_MAX+1);

    if(eggReg_is_relPath(filepath) == EGG_TRUE || eggReg_is_dbName(filepath) == EGG_TRUE)
    {
        eggCfgVal* p_list_val = eggConfig_get_cfg(g_config_handle, "basepath");
        GList* list_value_head = g_list_first(p_list_val);
        if(list_value_head == EGG_NULL)
        {
            eggPrtLog_error("eggBasePath is not set", "[%s]\n", __func__ );
            free(lp_real_path);
            return NULL;
        }
        char* lp_base_path = (char*)list_value_head->data;

        if(access(lp_base_path, F_OK))
        {
            eggPrtLog_error("eggBasePath", "[%s] [%s][%s]\n", __func__, strerror(errno), lp_base_path);
            free(lp_real_path);
            return NULL;
        }
        
        lp_real_path[0] = '\0';
        strcat(lp_real_path, lp_base_path);
        strcat(lp_real_path, "/");
        strcat(lp_real_path, filepath);
        strcat(lp_real_path, "/");
        if(access(lp_real_path, F_OK) )
        {
            char cmd[512];
            sprintf(cmd, "mkdir -m 0777 -p %s", lp_real_path);
            if(system(cmd) == -1)
            {
                eggPrtLog_error("mkdir error", "[%s] [%s][%s]\n", __func__, strerror(errno), lp_real_path);
                free(lp_real_path);
                return NULL;
            }
        }
    }
    else if (eggReg_is_absPath(filepath) == EGG_TRUE)
    {
        int n_path_len = strlen(filepath);
        while(n_path_len)
        {
            if (filepath[n_path_len - 1] == '%')break;
            n_path_len--;
        }
        if(filepath[n_path_len])
            realpath(filepath + n_path_len, lp_real_path);
        
        strcat(lp_real_path, "/");
        if(access(lp_real_path, F_OK))
        {
        //fprintf(stderr, "[%s] [%s][%s]\n", __func__, strerror(errno), lp_directory->path);
            eggPrtLog_error("eggDirectory", "[%s] [%s][%s]\n", __func__, strerror(errno), lp_real_path);
            free(lp_real_path);
            return NULL;
        }
        
    }
    else
    {
        free(lp_real_path);
        return NULL;
    }
    return lp_real_path;
}
