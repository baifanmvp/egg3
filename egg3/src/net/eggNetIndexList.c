#include "eggNetIndexList.h"


HEGGNETINDEXLIST eggNetIndexList_new()
{
    HEGGNETINDEXLIST hNetIndexList = (HEGGNETINDEXLIST)malloc(sizeof(EGGNETINDEXLIST));
    memset(hNetIndexList, 0, sizeof(EGGNETINDEXLIST));
    
    pthread_mutex_init(&hNetIndexList->mutex, EGG_NULL);
    
    return hNetIndexList;
}

EBOOL eggNetIndexList_delete(HEGGNETINDEXLIST hNetIndexList)
{
    HEGGNETINDEXNODE lp_iter_node  = hNetIndexList->head;

    pthread_mutex_lock(&hNetIndexList->mutex);
    while(lp_iter_node)
    {
        eggIndexReader_free(lp_iter_node->hReader);
        eggIndexWriter_close(lp_iter_node->hWriter);
        eggIndexSearcher_delete(lp_iter_node->hSearcher);
        eggPath_close(lp_iter_node->hHandle);
        hNetIndexList->head = lp_iter_node;
        lp_iter_node = lp_iter_node->next;
        free(hNetIndexList->head);
    }
    pthread_mutex_unlock(&hNetIndexList->mutex);
    
    pthread_mutex_destroy(&hNetIndexList->mutex);
    
    free(hNetIndexList);
    return EGG_TRUE;
}

HEGGNETINDEXNODE eggNetIndexList_create_indexhandle(HEGGNETINDEXLIST hNetIndexList, const char* pEggName, const char* pAnalyzerName)
{
    if(POINTER_IS_INVALID(pEggName))
    {
        return EGG_NULL;
    }
    HEGGDIRECTORY lp_directory = eggDirectory_open(pEggName);
    if(!lp_directory) return EGG_NULL;
    
    HEGGINDEXWRITER lp_writer = eggIndexWriter_open(lp_directory, pAnalyzerName);
    if(!lp_writer) return EGG_NULL;
    
    HEGGINDEXREADER lp_reader = eggIndexWriter_init_reader(lp_writer);
    if(!lp_reader) return EGG_NULL;
    
    HEGGINDEXSEARCHER lp_searcher = eggIndexSearcher_new(lp_reader);
    if(!lp_searcher) return EGG_NULL;
    
    HEGGNETINDEXNODE lp_res_node = (HEGGNETINDEXNODE)malloc(sizeof(EGGNETINDEXNODE));
    
    lp_res_node->hHandle = lp_directory;
    lp_res_node->hWriter = lp_writer;
    lp_res_node->hReader = lp_reader;
    lp_res_node->hSearcher = lp_searcher;
    lp_res_node->analyzerName = pAnalyzerName;
    
    pthread_mutex_lock(&hNetIndexList->mutex);
    lp_res_node->next = hNetIndexList->head;
    hNetIndexList->head = lp_res_node;
    pthread_mutex_unlock(&hNetIndexList->mutex);
    
    return lp_res_node;
    
}

HEGGNETINDEXNODE eggNetIndexList_find_indexhandle(HEGGNETINDEXLIST hNetIndexList, const char* path)
{
    if(POINTER_IS_INVALID(path))
    {
        return EGG_NULL;
    }
    pthread_mutex_lock(&hNetIndexList->mutex);

    char realPath[PATH_MAX];
    realpath(path, realPath);
    strcat(realPath, "/");
    
    HEGGNETINDEXNODE lp_iter_node  = hNetIndexList->head;
    
    while(lp_iter_node)
    {
        if(strcmp(realPath, lp_iter_node->hHandle->eggHandle_getName(lp_iter_node->hHandle)) == 0)
            break;
        
        lp_iter_node = lp_iter_node->next;
    }
    
    pthread_mutex_unlock(&hNetIndexList->mutex);

    return lp_iter_node;
}



EBOOL eggNetIndexList_optimize(HEGGNETINDEXLIST hNetIndexList)
{
    if(POINTER_IS_INVALID(hNetIndexList))
    {
        return EGG_FALSE;
    }
    pthread_mutex_lock(&hNetIndexList->mutex);
        
    HEGGNETINDEXNODE lp_iter_node  = hNetIndexList->head;
    
    
    while(lp_iter_node)
    {
        eggIndexWriter_optimize_local(lp_iter_node->hWriter);
        lp_iter_node = lp_iter_node->next;
    }
    pthread_mutex_unlock(&hNetIndexList->mutex);

    return EGG_TRUE;

}


