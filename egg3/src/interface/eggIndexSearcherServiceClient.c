#include "eggIndexSearcherServiceClient.h"
#include "eggIndexReaderServiceClient.h"
#include "../net/eggNetPackage.h"
#include "../eggTopCollector.h"
#include "../eggServiceClient.h"
#include "../log/eggPrtLog.h"
#include <assert.h>
struct eggIndexSearcherServiceClient
{
    HEGGHANDLE hEggHandle;      /* must be first */
    char *eggDirPathName;
    
};
typedef struct eggIndexSearcherServiceClient EGGINDEXSEARCHERSERVICECLIENT;

#define RETRY_MAX_SND 3
#define RETRY_MAX_RCV 3

PRIVATE EBOOL  eggIndexSearcher_search_with_query_basic_serviceClient(HEGGINDEXSEARCHER hIndexSearcher_, HEGGTOPCOLLECTOR hTopCollector, HEGGQUERY hQuery, type_t op);

HEGGINDEXSEARCHER EGGAPI eggIndexSearcher_new_serviceclient(HEGGINDEXREADER hIndexReader)
{
    EGGINDEXSEARCHERSERVICECLIENT *hIndexSearcher = (EGGINDEXSEARCHERSERVICECLIENT *)calloc(1, sizeof(EGGINDEXSEARCHERSERVICECLIENT));
    assert(hIndexSearcher);
    hIndexSearcher->hEggHandle = hIndexReader->hEggHandle->eggHandle_dup(hIndexReader->hEggHandle);

    char *eggDirPathName = eggServiceClient_get_eggdirpathname((HEGGSERVICECLIENT)hIndexSearcher->hEggHandle);
    if (eggDirPathName)
    {
        hIndexSearcher->eggDirPathName = strdup(eggDirPathName);
        assert(hIndexSearcher->eggDirPathName);
    }
    
    return hIndexSearcher;
    
}

EBOOL EGGAPI eggIndexSearcher_delete_serviceclient(HEGGINDEXSEARCHER hIndexSearcher_)
{
    EGGINDEXSEARCHERSERVICECLIENT *hIndexSearcher = (EGGINDEXSEARCHERSERVICECLIENT *)hIndexSearcher_;
    hIndexSearcher->hEggHandle->eggHandle_delete(hIndexSearcher->hEggHandle);
    free(hIndexSearcher->eggDirPathName);
    hIndexSearcher->hEggHandle = 0;
    free(hIndexSearcher);
    return EGG_TRUE;
    
}

HEGGSEARCHITER EGGAPI eggIndexSearcher_get_queryiter_serviceclient(HEGGINDEXSEARCHER hIndexSearcher)
{
    return eggSearchIter_new();
}

EBOOL EGGAPI eggIndexSearcher_search_with_query_serviceclient(HEGGINDEXSEARCHER hIndexSearcher_, HEGGTOPCOLLECTOR hTopCollector, HEGGQUERY hQuery)
{
    return eggIndexSearcher_search_with_query_basic_serviceClient(hIndexSearcher_, hTopCollector, hQuery, EGG_PACKAGE_SEARCH_SORT);
}


EBOOL EGGAPI eggIndexSearcher_search_with_query_sort_serviceclient(HEGGINDEXSEARCHER hIndexSearcher_, HEGGTOPCOLLECTOR hTopCollector, HEGGQUERY hQuery)
{
    return 1;//eggIndexSearcher_search_with_query_basic_serviceclient(hIndexSearcher_, hTopCollector, hQuery, EGG_PACKAGE_SEARCH_SORT);
}

EBOOL EGGAPI eggIndexSearcher_count_with_query_serviceclient(HEGGINDEXSEARCHER hIndexSearcher_, HEGGTOPCOLLECTOR hTopCollector, HEGGQUERY hQuery)
{
    return eggIndexSearcher_search_with_query_basic_serviceClient(hIndexSearcher_, hTopCollector, hQuery, EGG_PACKAGE_SEARCH_COUNT);
}


EBOOL EGGAPI eggIndexSearcher_filter_serviceclient(HEGGINDEXSEARCHER hIndexSearcher_, HEGGTOPCOLLECTOR hTopCollector, HEGGQUERY hQuery, int iforderbyit)
{
    EGGINDEXSEARCHERSERVICECLIENT *hIndexSearcher = (EGGINDEXSEARCHERSERVICECLIENT *)hIndexSearcher_;
    if (EGGINDEXSEARCHER_IS_INVAILD(hIndexSearcher))
    {
        return EGG_FALSE;
    }


    int snd_retry = 0;
    int rcv_retry = 0;

retry:
    if (snd_retry >= RETRY_MAX_SND)
    {
        eggPrtLog_error("eggIndexSearcherServiceClient", "%s:%d:%s ERR snd_retry >= %d", __FILE__, __LINE__, __func__, RETRY_MAX_SND);
        
        return EGG_FALSE;
    }

    
    if (eggServiceClient_change_eggdirpath((HEGGSERVICECLIENT)hIndexSearcher->hEggHandle,
                                           hIndexSearcher->eggDirPathName) == EGG_FALSE)
    {
        return EGG_FALSE;
    }
    
    char *collectorbuf = NULL;
    int collectorsize = 0;
    char *querybuf = NULL;
    int querysize = 0;
    querybuf = eggQuery_serialise(hQuery, &querysize);
    HEGGTOPCTORCHUNK lp_topctor_chunk = eggTopCollector_serialization(hTopCollector);
    if (!querybuf || !lp_topctor_chunk)
    {
        free(querybuf);
        free(lp_topctor_chunk);
        return EGG_FALSE;
    }
    HEGGNETPACKAGE lp_net_package;
    char flag = iforderbyit;
    lp_net_package = eggNetPackage_new(EGG_PACKAGE_SEARCHFILTER);
    lp_net_package = eggNetPackage_add(lp_net_package, lp_topctor_chunk, lp_topctor_chunk->size, EGG_PACKAGE_TOPCOLLECTOR);
    lp_net_package = eggNetPackage_add(lp_net_package, querybuf, querysize, EGG_PACKAGE_QUERY);
    lp_net_package = eggNetPackage_add(lp_net_package, &flag, sizeof(flag), EGG_PACKAGE_FLAG);

    EBOOL r;

    r = hIndexSearcher->hEggHandle->eggRemote_send(hIndexSearcher->hEggHandle,
                                                   lp_net_package,
                                                   eggNetPackage_get_packagesize(lp_net_package),
                                                   0);
    eggNetPackage_delete(lp_net_package);
    free(lp_topctor_chunk);
    free(querybuf);
    if (r == EGG_FALSE)
    {
        snd_retry++;
        eggPrtLog_info("eggIndexSearcherServiceClient", "%s:%d:%s send fail retry[%d]", __FILE__, __LINE__, __func__, snd_retry);
        goto retry;
    }
    
    snd_retry = 0;
    
    char *retpackage = 0;
    int retpackagesz = 0;
    EBOOL* p_ret = EGG_NULL;
    size32_t retsz = 0;
    r = hIndexSearcher->hEggHandle->eggRemote_receive(hIndexSearcher->hEggHandle,
                                                      &retpackage, &retpackagesz, 0);
    if (r == EGG_FALSE)
    {
        free(retpackage);
        
        if (rcv_retry < RETRY_MAX_RCV)
        {
            rcv_retry++;
            eggPrtLog_info("eggIndexSearcherServiceClient", "%s:%d:%s recieve fail retry[%d]", __FILE__, __LINE__, __func__, rcv_retry);
            goto retry;
        }
        
        eggPrtLog_error("eggIndexSearcherServiceClient", "%s:%d:%s ERR receive fail rcv_retry >= %d", __FILE__, __LINE__, __func__, RETRY_MAX_RCV);
        return EGG_FALSE;
    }
    
    eggNetPackage_fetch((HEGGNETPACKAGE)retpackage, 4, &p_ret, &retsz,
                        &collectorbuf, &collectorsize);
    EBOOL ret = *p_ret;
    if(ret == EGG_TRUE) 
    {
        HEGGTOPCOLLECTOR hTopCollector_result;
        hTopCollector_result = eggTopCollector_unserialization(collectorbuf);
        eggTopCollector_merge_with_ref(hTopCollector, hTopCollector_result);
        
    }
    
    free(retpackage);
    return ret;
}
EBOOL EGGAPI eggIndexSearcher_search_with_queryiter_serviceclient(HEGGINDEXSEARCHER hIndexSearcher_, HEGGTOPCOLLECTOR hTopCollector, HEGGQUERY hQuery, HEGGSEARCHITER hIter)
{
    EGGINDEXSEARCHERSERVICECLIENT *hIndexSearcherRm = (EGGINDEXSEARCHERSERVICECLIENT *)hIndexSearcher_;
    if (EGGINDEXSEARCHER_IS_INVAILD(hIndexSearcherRm))
    {
        return EGG_FALSE;
    }


    int snd_retry = 0;
    int rcv_retry = 0;
retry:
    if (snd_retry >= RETRY_MAX_SND)
    {
        eggPrtLog_error("eggIndexSearcherServiceClient", "%s:%d:%s ERR snd_retry >= %d", __FILE__, __LINE__, __func__, RETRY_MAX_SND);
        
        return EGG_FALSE;
    }

    
    if (eggServiceClient_change_eggdirpath((HEGGSERVICECLIENT)hIndexSearcherRm->hEggHandle,
                                           hIndexSearcherRm->eggDirPathName) == EGG_FALSE)
    {
        return EGG_FALSE;
    }
    
    char *collectorbuf = NULL;
    int collectorsize = 0;
    
    char *iterbuf = NULL;
    int itersize = 0;
    
    char *querybuf = NULL;
    
    int querysize = 0;
    querybuf = eggQuery_serialise(hQuery, &querysize);
    
    HEGGTOPCTORCHUNK lp_topctor_chunk = eggTopCollector_serialization(hTopCollector);
    if (!querybuf || !lp_topctor_chunk)
    {
        free(querybuf);
        free(lp_topctor_chunk);
        return EGG_FALSE;
    }
    count_t n_unit_cnt = hIter->unitCnt;

    HEGGNETPACKAGE lp_net_package;
    lp_net_package = eggNetPackage_new(EGG_PACKAGE_SEARCH_ITER);
    lp_net_package = eggNetPackage_add(lp_net_package, hIter, sizeof(EGGSEARCHITER), EGG_PACKAGE_ITER);
    lp_net_package = eggNetPackage_add(lp_net_package, lp_topctor_chunk, lp_topctor_chunk->size, EGG_PACKAGE_TOPCOLLECTOR);
    lp_net_package = eggNetPackage_add(lp_net_package, querybuf, querysize, EGG_PACKAGE_QUERY);
    

    EBOOL r;
    
    r = hIndexSearcherRm->hEggHandle->eggRemote_send(hIndexSearcherRm->hEggHandle,
                                                     lp_net_package,
                                                     eggNetPackage_get_packagesize(lp_net_package),
                                                     0);
    eggNetPackage_delete(lp_net_package);
    free(lp_topctor_chunk);
    free(querybuf);

    if (r == EGG_FALSE)
    {
        snd_retry++;
        eggPrtLog_info("eggIndexSearcherServiceClient", "%s:%d:%s send fail retry[%d]", __FILE__, __LINE__, __func__, snd_retry);
        goto retry;
    }
    snd_retry = 0;
    
    char *retpackage = 0;
    int retpackagesz = 0;
    EBOOL* p_ret = EGG_NULL;
    size32_t retsz = 0;
    r = hIndexSearcherRm->hEggHandle->eggRemote_receive(hIndexSearcherRm->hEggHandle,
                                                        &retpackage, &retpackagesz, 0);

    if (r == EGG_FALSE)
    {
        free(retpackage);

        if (rcv_retry < RETRY_MAX_RCV)
        {
            rcv_retry++;
            eggPrtLog_info("eggIndexSearcherServiceClient", "%s:%d:%s recieve fail retry[%d]", __FILE__, __LINE__, __func__, rcv_retry);
            goto retry;
        }
        
        eggPrtLog_error("eggIndexSearcherServiceClient", "%s:%d:%s ERR receive fail rcv_retry >= %d", __FILE__, __LINE__, __func__, RETRY_MAX_RCV);
        return EGG_FALSE;
    }

    eggNetPackage_fetch((HEGGNETPACKAGE)retpackage, 6, &p_ret, &retsz,
                        &iterbuf, &itersize,
                        &collectorbuf, &collectorsize);
    
    eggSearchIter_dup(hIter, (HEGGSEARCHITER)iterbuf);
    //memcpy(hIter, iterbuf, itersize);
    //free(iterbuf);

    EBOOL ret = *p_ret;
    if(ret != EGG_FALSE) 
    {

        if (hIter->scoreDocIdx == EGG_PAGEFRONT)
        {
            ret = EGG_PAGEFIRST;
        }
        else if (hIter->scoreDocIdx == EGG_PAGEBACK) 
        {
            ret = EGG_PAGELAST;
        }
        else
        {
            ret = EGG_TRUE;
        }
            
        
        HEGGTOPCOLLECTOR hTopCollector_result;
        hTopCollector_result = eggTopCollector_unserialization(collectorbuf);
        eggTopCollector_merge_with_ref(hTopCollector, hTopCollector_result);
        
    }
    
    free(retpackage);


    return ret; 
   
}

PRIVATE EBOOL eggIndexSearcher_search_with_query_basic_serviceClient(HEGGINDEXSEARCHER hIndexSearcher_, HEGGTOPCOLLECTOR hTopCollector, HEGGQUERY hQuery, type_t op)
{
    EGGINDEXSEARCHERSERVICECLIENT *hIndexSearcher = (EGGINDEXSEARCHERSERVICECLIENT *)hIndexSearcher_;
    if (EGGINDEXSEARCHER_IS_INVAILD(hIndexSearcher))
    {
        return EGG_FALSE;
    }


    int snd_retry = 0;
    int rcv_retry = 0;    

retry:
    if (snd_retry >= RETRY_MAX_SND)
    {
        eggPrtLog_error("eggIndexSearcherServiceClient", "%s:%d:%s ERR snd_retry >= %d", __FILE__, __LINE__, __func__, RETRY_MAX_SND);
        
        return EGG_FALSE;
    }

    
    if (eggServiceClient_change_eggdirpath((HEGGSERVICECLIENT)hIndexSearcher->hEggHandle,
                                              hIndexSearcher->eggDirPathName) == EGG_FALSE)
    {
        return EGG_FALSE;
    }
    
    char *collectorbuf = NULL;
    int collectorsize = 0;
    char *querybuf = NULL;
    int querysize = 0;
    querybuf = eggQuery_serialise(hQuery, &querysize);
    HEGGTOPCTORCHUNK lp_topctor_chunk = eggTopCollector_serialization(hTopCollector);
    if (!querybuf || !lp_topctor_chunk)
    {
        free(querybuf);
        free(lp_topctor_chunk);
        return EGG_FALSE;
    }
    HEGGNETPACKAGE lp_net_package;
    lp_net_package = eggNetPackage_new(op);
    lp_net_package = eggNetPackage_add(lp_net_package, lp_topctor_chunk, lp_topctor_chunk->size, EGG_PACKAGE_TOPCOLLECTOR);
    lp_net_package = eggNetPackage_add(lp_net_package, querybuf, querysize, EGG_PACKAGE_QUERY);

    //////////////////

    EBOOL r;

    r = hIndexSearcher->hEggHandle->eggRemote_send(hIndexSearcher->hEggHandle,
                                                   lp_net_package,
                                                   eggNetPackage_get_packagesize(lp_net_package),
                                                   0);
    eggNetPackage_delete(lp_net_package);
    free(lp_topctor_chunk);
    free(querybuf);
    if (r == EGG_FALSE)
    {
        snd_retry++;
        eggPrtLog_info("eggIndexSearcherServiceClient", "%s:%d:%s send fail retry[%d]", __FILE__, __LINE__, __func__, snd_retry);
        goto retry;
    }

    snd_retry = 0;
    
    char *retpackage = 0;
    int retpackagesz = 0;
    EBOOL* p_ret = EGG_NULL;
    size32_t retsz;
    r = hIndexSearcher->hEggHandle->eggRemote_receive(hIndexSearcher->hEggHandle,
                                                      &retpackage, &retpackagesz, 0);

    if (r == EGG_FALSE)
    {
        free(retpackage);

        if (rcv_retry < RETRY_MAX_RCV)
        {
            rcv_retry++;
            eggPrtLog_info("eggIndexSearcherServiceClient", "%s:%d:%s recieve fail retry[%d]", __FILE__, __LINE__, __func__, rcv_retry);
            goto retry;

        }
        
        eggPrtLog_error("eggIndexSearcherServiceClient", "%s:%d:%s ERR receive fail rcv_retry >= %d", __FILE__, __LINE__, __func__, RETRY_MAX_RCV);
        
        return EGG_FALSE;
    }
    
    eggNetPackage_fetch((HEGGNETPACKAGE)retpackage, 4, &p_ret, &retsz,
                        &collectorbuf, &collectorsize);

    ////////////////

    EBOOL ret = *p_ret;
    if(ret == EGG_TRUE) 
    {
        HEGGTOPCOLLECTOR hTopCollector_result;
        hTopCollector_result = eggTopCollector_unserialization(collectorbuf);
        eggTopCollector_merge_with_ref(hTopCollector, hTopCollector_result);
        
    }
    
    free(retpackage);


    return ret;
}

