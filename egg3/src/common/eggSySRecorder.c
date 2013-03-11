#include "../eggSySRecorder.h"

#include "eggPath.h"
#include "eggIndexReader.h"
#include "eggIndexWriter.h"
#include "eggIndexSearcher.h"
#include "eggDirectory.h"
struct eggSySRecorder
{
    char* path;
    HEGGINDEXWRITER hIndexWriter;
    HEGGINDEXREADER hIndexReader;
    HEGGINDEXSEARCHER hIndexSearcher;
    pthread_mutex_t mutex;
};

/*
  document format
  
  field1 : infotype;//
  
  //下面的域自定义
  field2
  ...
  ...
  fieldn
 */

HEGGSYSRECORDER g_sysRecorder_handle = EGG_NULL;


PUBLIC EBOOL EGGAPI eggSySRecorder_init()
{
    if(!POINTER_IS_INVALID(g_sysRecorder_handle))
    {
        exit(-1);
    }
    
    g_sysRecorder_handle = (HEGGSYSRECORDER)malloc(sizeof(EGGSYSRECORDER));
    memset(g_sysRecorder_handle, 0, sizeof(EGGSYSRECORDER));
    pthread_mutex_init( &g_sysRecorder_handle->mutex, NULL);
    
    if(access(EGG_SYSRECORD_PATH, F_OK) != 0)
    {
        exit(-1);
    }
    g_sysRecorder_handle->path = (char*)malloc(1024);
    g_sysRecorder_handle->path[0] = 0;
    strcat(g_sysRecorder_handle->path, "/%%%");
    strcat(g_sysRecorder_handle->path, EGG_SYSRECORD_PATH);
    
    HEGGHANDLE hHandle = eggDirectory_open(g_sysRecorder_handle->path);

    g_sysRecorder_handle->hIndexReader = eggIndexReader_open(hHandle);
    g_sysRecorder_handle->hIndexSearcher = eggIndexSearcher_new(g_sysRecorder_handle->hIndexReader);
    g_sysRecorder_handle->hIndexWriter = eggIndexWriter_open(hHandle, "");
    
    eggPath_close(hHandle);
    return EGG_TRUE;
}


EBOOL EGGAPI eggSySRecorder_destroy()
{
    if(POINTER_IS_INVALID(g_sysRecorder_handle))
    {
        exit(-1);
    }
    
    free(g_sysRecorder_handle->path);
    eggIndexSearcher_delete(g_sysRecorder_handle->hIndexSearcher);
    eggIndexReader_close(g_sysRecorder_handle->hIndexReader);
    eggIndexWriter_close(g_sysRecorder_handle->hIndexWriter);
    
    pthread_mutex_destroy( &g_sysRecorder_handle->mutex);
    free(g_sysRecorder_handle);
    return EGG_TRUE;
}


void* EGGAPI eggSySRecorder_get_records(char* infotype )
{
    if(POINTER_IS_INVALID(g_sysRecorder_handle))
    {
        return EGG_NULL;
    }
    
    if(POINTER_IS_INVALID(infotype))
    {
        return EGG_NULL;
    }
    pthread_mutex_lock( &g_sysRecorder_handle->mutex);

    HEGGTOPCOLLECTOR hTopCollector = eggTopCollector_new(0);
    
    HEGGQUERY hQuery = eggQuery_new_string(EGG_SYS_TYPEINFO, infotype, strlen(infotype), "");
    
    eggIndexSearcher_search_with_query(g_sysRecorder_handle->hIndexSearcher, hTopCollector, hQuery);
    
    eggQuery_delete(hQuery);
    
    pthread_mutex_unlock( &g_sysRecorder_handle->mutex);

}



void* EGGAPI eggSySRecorder_get_dict(char* infotype, char* analyName )
{
    if(POINTER_IS_INVALID(g_sysRecorder_handle))
    {
        return EGG_NULL;
    }
    
    if(POINTER_IS_INVALID(infotype))
    {
        return EGG_NULL;
    }
    if(POINTER_IS_INVALID(analyName))
    {
        return EGG_NULL;
    }
    
    pthread_mutex_lock( &g_sysRecorder_handle->mutex);

    HEGGTOPCOLLECTOR hTopCollector = eggTopCollector_new(0);
    
    HEGGQUERY hQuery1 = eggQuery_new_string(EGG_SYS_TYPEINFO, infotype, strlen(infotype), "");

    HEGGQUERY hQuery2 = eggQuery_new_string(EGG_SYS_ANALYNAME, analyName, strlen(analyName), "");

    hQuery1 = eggQuery_and(hQuery1, hQuery2);
    
    eggIndexSearcher_search_with_query(g_sysRecorder_handle->hIndexSearcher, hTopCollector, hQuery1);
    
    eggQuery_delete(hQuery1);
    
    pthread_mutex_unlock( &g_sysRecorder_handle->mutex);
    
    return hTopCollector;
}

void*  EGGAPI eggSySRecorder_alloc_reader()
{
    if(POINTER_IS_INVALID(g_sysRecorder_handle))
    {
        return EGG_FALSE;
    }
    pthread_mutex_lock( &g_sysRecorder_handle->mutex);

    return g_sysRecorder_handle->hIndexReader;
}

EBOOL EGGAPI eggSySRecorder_free_reader(void** hIndexReader)
{
    if(POINTER_IS_INVALID(g_sysRecorder_handle))
    {
        return EGG_FALSE;
    }
    *hIndexReader = EGG_NULL;
    pthread_mutex_unlock( &g_sysRecorder_handle->mutex);

    return EGG_TRUE;
}
