#include "./eggIndexSearcherCluster.h"
#include "eggIndexReaderCluster.h"
#include "./eggIndexSearcherRemote.h"
#include "eggIndexReaderRemote.h"
#include "../net/eggNetPackage.h"
#include "../eggTopCollector.h"
#include "../uti/eggThreadPool.h"
#include "../cluster/eggClusterCommon.h"
#include <assert.h>

struct eggIndexSearcherCluster
{
    HEGGHANDLE hEggHandle;

    HEGGCHUNKHAND hChunkHands;
    count_t chunkCnt;
};

PRIVATE EBOOL eggIndexSearcher_init_chunkHands_cluster(HEGGINDEXSEARCHERCLUSTER hIndexSearcherCt, HEGGINDEXREADERCLUSTER hIndexReaderCt);
PRIVATE EBOOL eggIndexSearcher_searchWithQuery_thread_cluster(HEGGTHREADTASKDETAILS htaskDetails);
PRIVATE EBOOL eggIndexSearcher_countWithQuery_thread_cluster(HEGGTHREADTASKDETAILS htaskDetails);

PRIVATE EBOOL eggIndexSearcher_destroy_chunkHands_cluster(HEGGINDEXSEARCHERCLUSTER hIndexSearcherCt);

HEGGINDEXSEARCHER EGGAPI eggIndexSearcher_new_cluster(HEGGINDEXREADER hIndexReader)
{
    if(POINTER_IS_INVALID(hIndexReader))
    {
        return EGG_NULL;
    }
    
    HEGGINDEXREADERCLUSTER hIndexReaderCt = (HEGGINDEXREADERCLUSTER)hIndexReader;

    HEGGINDEXSEARCHERCLUSTER hIndexSearcherCt = (HEGGINDEXSEARCHERCLUSTER)malloc(sizeof(EGGINDEXSEARCHERCLUSTER));
    memset(hIndexSearcherCt, 0, sizeof(EGGINDEXSEARCHERCLUSTER));
    
    assert(hIndexSearcherCt);
    

    eggIndexSearcher_init_chunkHands_cluster(hIndexSearcherCt, hIndexReaderCt);
    
    hIndexSearcherCt->hEggHandle = hIndexReader->hEggHandle->eggHandle_dup(hIndexReader->hEggHandle);//hEggHandle;

    return (HEGGINDEXSEARCHERCLUSTER)hIndexSearcherCt;

}

EBOOL EGGAPI eggIndexSearcher_delete_cluster(HEGGINDEXSEARCHER hIndexSearcher)
{
    if(POINTER_IS_INVALID(hIndexSearcher))
    {
        return EGG_FALSE;
    }
    
    HEGGINDEXSEARCHERCLUSTER hIndexSearcherCt = (HEGGINDEXSEARCHERCLUSTER)hIndexSearcher;
    
    eggIndexSearcher_destroy_chunkHands_cluster(hIndexSearcherCt);
    hIndexSearcherCt->hEggHandle->eggHandle_delete(hIndexSearcherCt->hEggHandle);

    free(hIndexSearcherCt);
    return EGG_TRUE;
}

HEGGSEARCHITER EGGAPI eggIndexSearcher_get_queryiter_cluster(HEGGINDEXSEARCHER hIndexSearcher)
{
    return eggSearchIter_new();
}

EBOOL EGGAPI eggIndexSearcher_search_with_query_cluster(HEGGINDEXSEARCHER hIndexSearcher, HEGGTOPCOLLECTOR hTopCollector, HEGGQUERY hQuery)
{
    HEGGINDEXSEARCHERCLUSTER hIndexSearcherCt = (HEGGINDEXSEARCHERCLUSTER)hIndexSearcher;
    
    if(POINTER_IS_INVALID(hIndexSearcherCt))
    {
        return EGG_FALSE;
    }
    
    HEGGTOPCOLLECTOR* lplp_topcollector = (HEGGTOPCOLLECTOR*)malloc(sizeof(HEGGTOPCOLLECTOR) * hIndexSearcherCt->chunkCnt);
    HEGGTHREADTASKDETAILS* lplp_details = (HEGGTHREADTASKDETAILS*)malloc(sizeof(HEGGTHREADTASKDETAILS) * hIndexSearcherCt->chunkCnt);
    pthread_t* pthread_ids = (pthread_t*)malloc(sizeof(pthread_t) * hIndexSearcherCt->chunkCnt);
    index_t n_chunkhand_idx = 0;

    while(n_chunkhand_idx < hIndexSearcherCt->chunkCnt)
    {
        lplp_topcollector[n_chunkhand_idx] = eggTopCollector_dup(hTopCollector);
;
        
        lplp_details[n_chunkhand_idx] = eggThreadPool_taskdetails_init(3,
                                                                       &(hIndexSearcherCt->hChunkHands[n_chunkhand_idx].hChunkObj),
                                                                       &(lplp_topcollector[n_chunkhand_idx]),
                                                                       &(hQuery));
        
	pthread_create(pthread_ids + n_chunkhand_idx, EGG_NULL,
		       eggIndexSearcher_searchWithQuery_thread_cluster,  lplp_details[n_chunkhand_idx]);
    
        n_chunkhand_idx++;
    }
    
    n_chunkhand_idx = 0;
    while(n_chunkhand_idx < hIndexSearcherCt->chunkCnt)
    {
        EBOOL ret = 0;
        pthread_join(pthread_ids[n_chunkhand_idx], 0);
        
        eggThreadPool_taskdetails_destroy(lplp_details[n_chunkhand_idx]);
        
        eggTopCollector_set_chunkid_cluster(lplp_topcollector[n_chunkhand_idx], hIndexSearcherCt->hChunkHands[n_chunkhand_idx].start);
            
        n_chunkhand_idx++;
    }


    /*
    HEGGTOPCOLLECTOR lp_sum_topCollector = eggTopCollector_merge_with_cluster(hIndexSearcherCt->chunkCnt, lplp_topcollector);
    
    hTopCollector = eggTopCollector_merge_with_ref(hTopCollector, lp_sum_topCollector);
    */
    eggTopCollector_merge_with_cluster(hTopCollector, hIndexSearcherCt->chunkCnt, lplp_topcollector);
    
    free(pthread_ids);
    free(lplp_details);

    n_chunkhand_idx = 0;
    while(n_chunkhand_idx < hIndexSearcherCt->chunkCnt)
      {
          eggTopCollector_delete(lplp_topcollector[n_chunkhand_idx]);
          n_chunkhand_idx++;
      }
    free(lplp_topcollector);
    if(eggTopCollector_total_hits(hTopCollector))
        return EGG_TRUE;
    else
        return EGG_FALSE;
}


EBOOL EGGAPI eggIndexSearcher_count_with_query_cluster(HEGGINDEXSEARCHER hIndexSearcher, HEGGTOPCOLLECTOR hTopCollector, HEGGQUERY hQuery)
{
    HEGGINDEXSEARCHERCLUSTER hIndexSearcherCt = (HEGGINDEXSEARCHERCLUSTER)hIndexSearcher;
    
    if(POINTER_IS_INVALID(hIndexSearcherCt))
    {
        return EGG_FALSE;
    }
    
    HEGGTOPCOLLECTOR* lplp_topcollector = (HEGGTOPCOLLECTOR*)malloc(sizeof(HEGGTOPCOLLECTOR) * hIndexSearcherCt->chunkCnt);
    HEGGTHREADTASKDETAILS* lplp_details = (HEGGTHREADTASKDETAILS*)malloc(sizeof(HEGGTHREADTASKDETAILS) * hIndexSearcherCt->chunkCnt);
    pthread_t* pthread_ids = (pthread_t*)malloc(sizeof(pthread_t) * hIndexSearcherCt->chunkCnt);
    index_t n_chunkhand_idx = 0;

    while(n_chunkhand_idx < hIndexSearcherCt->chunkCnt)
    {
        lplp_topcollector[n_chunkhand_idx] = eggTopCollector_dup(hTopCollector);
;
        
        lplp_details[n_chunkhand_idx] = eggThreadPool_taskdetails_init(3,
                                                                       &(hIndexSearcherCt->hChunkHands[n_chunkhand_idx].hChunkObj),
                                                                       &(lplp_topcollector[n_chunkhand_idx]),
                                                                       &(hQuery));
        
	pthread_create(pthread_ids + n_chunkhand_idx, EGG_NULL,
		       eggIndexSearcher_countWithQuery_thread_cluster,  lplp_details[n_chunkhand_idx]);
    
        n_chunkhand_idx++;
    }
    
    n_chunkhand_idx = 0;
    while(n_chunkhand_idx < hIndexSearcherCt->chunkCnt)
    {
        EBOOL ret = 0;
        pthread_join(pthread_ids[n_chunkhand_idx], 0);
        
        eggThreadPool_taskdetails_destroy(lplp_details[n_chunkhand_idx]);
        
        eggTopCollector_set_chunkid_cluster(lplp_topcollector[n_chunkhand_idx], hIndexSearcherCt->hChunkHands[n_chunkhand_idx].start);
            
        n_chunkhand_idx++;
    }


    /*
    HEGGTOPCOLLECTOR lp_sum_topCollector = eggTopCollector_merge_with_cluster(hIndexSearcherCt->chunkCnt, lplp_topcollector);
    
    hTopCollector = eggTopCollector_merge_with_ref(hTopCollector, lp_sum_topCollector);
    */
    eggTopCollector_merge_with_cluster(hTopCollector, hIndexSearcherCt->chunkCnt, lplp_topcollector);
    
    free(pthread_ids);
    free(lplp_details);

    n_chunkhand_idx = 0;
    while(n_chunkhand_idx < hIndexSearcherCt->chunkCnt)
      {
          eggTopCollector_delete(lplp_topcollector[n_chunkhand_idx]);
          n_chunkhand_idx++;
      }
    free(lplp_topcollector);
    
    return EGG_TRUE;
}

EBOOL EGGAPI eggIndexSearcher_search_with_queryiter_cluster(HEGGINDEXSEARCHER hIndexSearcher, HEGGTOPCOLLECTOR hTopCollector, HEGGQUERY hQuery, HEGGSEARCHITER hIter)
{
    HEGGINDEXSEARCHERCLUSTER hIndexSearcherCt = (HEGGINDEXSEARCHERCLUSTER)hIndexSearcher;
    HEGGCHUNKHAND hChunkHands=hIndexSearcherCt->hChunkHands;
    count_t chunkCnt=hIndexSearcherCt->chunkCnt;
    
    HEGGTOPCOLLECTOR* eggtopcollector=malloc(2*chunkCnt*sizeof(HEGGTOPCOLLECTOR));
    count_t eggtopcollector_count=0;
    index_t i;
    

    count_t n_unit_cnt = hIter->unitCnt;
    i = hIter->chunkIdx;
    
    EBOOL ret;
    
    while(1)
    {
//        struct EGGHANDLE *hEggHandle = (struct EGGHANDLE*)hChunkHands[i].hChunkObj;

        HEGGTOPCOLLECTOR hetc = eggTopCollector_dup(hTopCollector);
        ret = eggIndexSearcher_search_with_queryiter             \
            (hChunkHands[i].hChunkObj, hetc, hQuery, hIter);
        
        eggTopCollector_set_chunkid_cluster(hetc, hChunkHands[i].start);
        eggtopcollector [eggtopcollector_count++] = hetc;
        
        
        if (hIter->scoreDocIdx == EGG_PAGEFIRST  || hIter->scoreDocIdx == EGG_PAGEFRONT)
        {
            if(i==0)
            {
                hIter->scoreDocIdx = EGG_PAGEFIRST;
                break;
            }
            i--;
            hIter->scoreDocIdx = EGG_PAGEFRONT;
        }
        else if (hIter->scoreDocIdx == EGG_PAGELAST || hIter->scoreDocIdx == EGG_PAGEBACK) 
        {
            if(i==chunkCnt-1)
            {
                hIter->scoreDocIdx = EGG_PAGELAST;
                break;
            }
            
            i++;
            if(hIter->unitCnt != 0) //if not get all data 
            {
                hIter->scoreDocIdx = EGG_PAGEBACK;
            }
            else //data is full exactly
            {
                hIter->scoreDocIdx = 0;
            }
        }
        else
        {
            break;
        }
    }

    /*
    HEGGTOPCOLLECTOR tcx = eggTopCollector_merge_with_cluster(eggtopcollector_count, eggtopcollector);
    eggTopCollector_merge_with_ref(hTopCollector, tcx);
    */
    eggTopCollector_merge_with_cluster(hTopCollector, eggtopcollector_count, eggtopcollector);
    
    hIter->chunkIdx = i;
    
     hIter->unitCnt = n_unit_cnt ;
    i = 0;
    while(i < eggtopcollector_count)
    {
        eggTopCollector_delete(eggtopcollector[i]);  
        i++;
    }
    free(eggtopcollector);
    
    if(!eggTopCollector_total_hits(hTopCollector))
        ret = EGG_FALSE;
    else
        ret = EGG_TRUE;
    
    return ret;
}


PRIVATE EBOOL eggIndexSearcher_init_chunkHands_cluster(HEGGINDEXSEARCHERCLUSTER hIndexSearcherCt, HEGGINDEXREADERCLUSTER hIndexReaderCt)
{
    if(POINTER_IS_INVALID(hIndexSearcherCt))
    {
        return EGG_FALSE;
    }
    
    HEGGCHUNKHAND lp_reader_chunkhands = eggIndexReader_get_chunkhands_cluster(hIndexReaderCt);
    
    count_t reader_chunk_cnt = eggIndexReader_get_chunkcnt_cluster(hIndexReaderCt);
    
    hIndexSearcherCt->chunkCnt = reader_chunk_cnt;
    hIndexSearcherCt->hChunkHands = (HEGGCHUNKHAND)malloc(sizeof(EGGCHUNKHAND) * reader_chunk_cnt);
    memcpy(hIndexSearcherCt->hChunkHands, lp_reader_chunkhands, sizeof(EGGCHUNKHAND) * reader_chunk_cnt);
    
    index_t n_chunk_idx = 0;
    
    while(n_chunk_idx < reader_chunk_cnt)
    {
        hIndexSearcherCt->hChunkHands[n_chunk_idx].hChunkObj = eggIndexSearcher_new(lp_reader_chunkhands[n_chunk_idx].hChunkObj);
        n_chunk_idx++;
    }
    
    return EGG_TRUE;
}

PRIVATE EBOOL eggIndexSearcher_destroy_chunkHands_cluster(HEGGINDEXSEARCHERCLUSTER hIndexSearcherCt)
{
    if(POINTER_IS_INVALID(hIndexSearcherCt))
    {
        return EGG_FALSE;
    }
    
    index_t n_chunk_idx = 0;
    
    while(n_chunk_idx < hIndexSearcherCt->chunkCnt)
    {
        eggIndexSearcher_delete(hIndexSearcherCt->hChunkHands[n_chunk_idx].hChunkObj);
        n_chunk_idx++;
    }
    free(hIndexSearcherCt->hChunkHands);
    hIndexSearcherCt->hChunkHands = EGG_NULL;
    return EGG_TRUE;
}

PRIVATE EBOOL eggIndexSearcher_searchWithQuery_thread_cluster(HEGGTHREADTASKDETAILS htaskDetails)
{
    if(POINTER_IS_INVALID(htaskDetails))
    {
        return EGG_FALSE;
    }
    HEGGQUERY lp_query = *(HEGGQUERY*)(htaskDetails->details[2]);
    
    HEGGTOPCOLLECTOR lp_topcollector = *(HEGGTOPCOLLECTOR*)(htaskDetails->details[1]);
    HEGGINDEXREADER lp_index_reader = *(HEGGINDEXREADER*)(htaskDetails->details[0]);
    
    HEGGDOCUMENT* lplp_document = EGG_NULL;
    
    EBOOL ret = eggIndexSearcher_search_with_query(lp_index_reader, lp_topcollector, lp_query);
    return ret;
}

PRIVATE EBOOL eggIndexSearcher_countWithQuery_thread_cluster(HEGGTHREADTASKDETAILS htaskDetails)
{
    if(POINTER_IS_INVALID(htaskDetails))
    {
        return EGG_FALSE;
    }
    HEGGQUERY lp_query = *(HEGGQUERY*)(htaskDetails->details[2]);
    
    HEGGTOPCOLLECTOR lp_topcollector = *(HEGGTOPCOLLECTOR*)(htaskDetails->details[1]);
    HEGGINDEXREADER lp_index_reader = *(HEGGINDEXREADER*)(htaskDetails->details[0]);
    
    HEGGDOCUMENT* lplp_document = EGG_NULL;
    
    EBOOL ret = eggIndexSearcher_count_with_query(lp_index_reader, lp_topcollector, lp_query);
    return ret;
}

EBOOL EGGAPI eggIndexSearcher_filter_cluster(HEGGINDEXSEARCHER hIndexSearcher, HEGGTOPCOLLECTOR hTopCollector, HEGGQUERY hQuery, int iforderbyit)
{
    HEGGINDEXSEARCHERCLUSTER hIndexSearcherCt = (HEGGINDEXSEARCHERCLUSTER)hIndexSearcher;

    if(POINTER_IS_INVALID(hIndexSearcherCt))
    {
        return EGG_FALSE;
    }
    /* ..... */
    return EGG_FALSE;
}
