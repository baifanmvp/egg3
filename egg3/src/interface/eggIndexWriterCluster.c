#include "eggIndexWriterCluster.h"
#include "eggIndexWriterRemote.h"
#include "eggIndexReaderCluster.h"
#include "../eggHttp.h"
#include "../log/eggPrtLog.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <pthread.h>
#include <signal.h>


struct eggIndexWriterCluster
{
    HEGGHANDLE hEggHandle;
    
    HEGGCHUNKHAND hChunkHands;
    count_t chunkCnt;
    
    char* spanFieldName;
};
PRIVATE EBOOL eggIndexWriter_init_chunkHands_cluster(HEGGINDEXWRITERCLUSTER hIndexWriterCt);

PRIVATE EBOOL eggIndexWriter_destroy_chunkHands_cluster(HEGGINDEXWRITERCLUSTER hIndexWriterCt);

HEGGINDEXWRITER EGGAPI eggIndexWriter_open_cluster(void *hEggHandle_, char *lpSpanField)
{
    if(POINTER_IS_INVALID(hEggHandle_))
    {
        return EGG_NULL;
    }

    HEGGCLUSTER hEggCluster = (HEGGCLUSTER)hEggHandle_;
    HEGGINDEXWRITERCLUSTER hIndexWriterCt = (HEGGINDEXWRITERCLUSTER)calloc(1, sizeof(EGGINDEXWRITERCLUSTER));

    hIndexWriterCt->spanFieldName = strdup(lpSpanField);
    
    assert(hIndexWriterCt);
    
    hIndexWriterCt->hEggHandle = ((HEGGHANDLE)hEggHandle_)->eggHandle_dup(hEggHandle_);//hEggHandle;
    
    eggIndexWriter_init_chunkHands_cluster(hIndexWriterCt);    

    return (HEGGINDEXWRITER)hIndexWriterCt;
    
}

EBOOL EGGAPI eggIndexWriter_close_cluster(HEGGINDEXWRITER hEggIndexWriter)
{
    HEGGINDEXWRITERCLUSTER hIndexWriterCt = (HEGGINDEXWRITERCLUSTER)hEggIndexWriter;
    if(POINTER_IS_INVALID(hIndexWriterCt))
    {
        return EGG_FALSE;
    }
    
    free(hIndexWriterCt->spanFieldName);
    
    hIndexWriterCt->hEggHandle->eggHandle_delete(hIndexWriterCt->hEggHandle);

    eggIndexWriter_destroy_chunkHands_cluster(hIndexWriterCt);

    free(hIndexWriterCt);
    return EGG_TRUE;
}

EBOOL EGGAPI eggIndexWriter_add_document_cluster(HEGGINDEXWRITER hEggIndexWriter, HEGGDOCUMENT hDocument)
{
    HEGGINDEXWRITERCLUSTER hIndexWriterCt = (HEGGINDEXWRITERCLUSTER)hEggIndexWriter;
    if(POINTER_IS_INVALID(hIndexWriterCt))
    {
        return EGG_FALSE;
    }
    
    if(POINTER_IS_INVALID(hDocument))
    {
        return EGG_FALSE;
    }

    
    HEGGFIELD  lp_span_field = eggDocument_get_field(hDocument, hIndexWriterCt->spanFieldName);
    type_t span_field_ty;
    SPANPOINT n_doc_spanpoint  = 0;
    size32_t n_field_sz = 0;
    if(!lp_span_field || (span_field_ty = eggField_get_type(lp_span_field)) & EGG_INDEX_STRING)
    {
        return EGG_FALSE;
    }
    else if(span_field_ty & EGG_INDEX_INT32)
    {
        n_doc_spanpoint = (SPANPOINT)(*(int*)eggField_get_value(lp_span_field, &n_field_sz));
    }
    else if(span_field_ty & EGG_INDEX_INT64)
    {
        n_doc_spanpoint = (SPANPOINT)(*(long long*)eggField_get_value(lp_span_field, &n_field_sz));
    }
    else if(span_field_ty & EGG_INDEX_DOUBLE)
    {
        n_doc_spanpoint = (SPANPOINT)(*(double*)eggField_get_value(lp_span_field, &n_field_sz));
    }
    else
    {
        return EGG_FALSE;
    }

    /* ==0, 存储在所有的机器上 */
    if (n_doc_spanpoint == 0)
    {
        EBOOL ret = EGG_TRUE;
        index_t n_spanpoint_idx = 0;
        while (n_spanpoint_idx < hIndexWriterCt->chunkCnt)
        {
            ret = eggIndexWriter_add_document(hIndexWriterCt->hChunkHands[n_spanpoint_idx].hChunkObj, hDocument);
            if (ret == EGG_FALSE)
            {
                //fprintf(stderr, "%s: eggIndexWriter_add_document == EGG_FALSE", __func__);
                eggPrtLog_error("eggIndexWriterCluster", "%s: eggIndexWriter_add_document == EGG_FALSE", __func__);
            }
            n_spanpoint_idx++;
        }
        return ret;
    }

    index_t n_spanpoint_idx = eggChunkHand_find_spanpoint(hIndexWriterCt->hChunkHands,
                                                             hIndexWriterCt->chunkCnt, n_doc_spanpoint);

    if(EGG_SPANPOINT_NOT_FIND == n_spanpoint_idx)
    {
        return EGG_FALSE;
    }

    EBOOL ret = eggIndexWriter_add_document(hIndexWriterCt->hChunkHands[n_spanpoint_idx].hChunkObj, hDocument);
    
    return ret;
}


EBOOL EGGAPI eggIndexWriter_optimize_cluster(HEGGINDEXWRITER hEggIndexWriter)
{
    HEGGINDEXWRITERCLUSTER hIndexWriterCt = (HEGGINDEXWRITERCLUSTER)hEggIndexWriter;
    
    if(POINTER_IS_INVALID(hIndexWriterCt))
    {
        return EGG_FALSE;
    }
    index_t n_chunkhands_idx = 0;
    count_t n_pthread_cnt = hIndexWriterCt->chunkCnt;
    pthread_t* pthread_ids = (pthread_t*)malloc(sizeof(pthread_t) * n_pthread_cnt);

    while (n_chunkhands_idx < hIndexWriterCt->chunkCnt)
    {
        pthread_create(pthread_ids + n_chunkhands_idx, EGG_NULL,
                       eggIndexWriter_optimize,  hIndexWriterCt->hChunkHands[n_chunkhands_idx].hChunkObj);
        
        n_chunkhands_idx++;
    }

    n_chunkhands_idx = 0;
    while (n_chunkhands_idx < hIndexWriterCt->chunkCnt)
    {
        EBOOL ret = 0;
        pthread_join(pthread_ids[n_chunkhands_idx], 0);
        n_chunkhands_idx++;
    }
    free(pthread_ids);
    return EGG_TRUE;
}




EBOOL EGGAPI eggIndexWriter_delete_document_cluster(HEGGINDEXWRITER hEggIndexWriter, EGGDID dId)
{
    HEGGINDEXWRITERCLUSTER hIndexWriterCt = (HEGGINDEXWRITERCLUSTER)hEggIndexWriter;
    
    if(POINTER_IS_INVALID(hIndexWriterCt))
    {
        return EGG_FALSE;
    }
    
    index_t n_spanpoint_idx = eggChunkHand_find_spanpoint(hIndexWriterCt->hChunkHands,
                                                             hIndexWriterCt->chunkCnt, dId.cluster.chunkId);
    if(EGG_SPANPOINT_NOT_FIND == n_spanpoint_idx)
    {
        return EGG_FALSE;
    }
    
    EBOOL ret = eggIndexWriter_delete_document(hIndexWriterCt->hChunkHands[n_spanpoint_idx].hChunkObj, dId);

    return ret;
    
}

EBOOL EGGAPI eggIndexWriter_modify_document_cluster(HEGGINDEXWRITER hEggIndexWriter, EGGDID dId, HEGGDOCUMENT hEggDocument)
{
    HEGGINDEXWRITERCLUSTER hIndexWriterCt = (HEGGINDEXWRITERCLUSTER)hEggIndexWriter;
    
    if(POINTER_IS_INVALID(hIndexWriterCt))
    {
        return EGG_FALSE;
    }
    
    index_t n_spanpoint_idx = eggChunkHand_find_spanpoint(hIndexWriterCt->hChunkHands,
                                                             hIndexWriterCt->chunkCnt, dId.cluster.chunkId);
    
    HEGGFIELD  lp_span_field = eggDocument_get_field(hEggDocument, hIndexWriterCt->spanFieldName);

    if(!lp_span_field && n_spanpoint_idx != -1)
    {
        return eggIndexWriter_modify_document(hIndexWriterCt->hChunkHands[n_spanpoint_idx].hChunkObj, dId, hEggDocument);        
    }
    type_t span_field_ty;
    SPANPOINT n_doc_spanpoint  = 0;
    size32_t n_field_sz = 0;
    if((span_field_ty = eggField_get_type(lp_span_field)) & EGG_INDEX_STRING)
    {
        return EGG_FALSE;
    }
    else if(span_field_ty & EGG_INDEX_INT32)
    {
        n_doc_spanpoint = (SPANPOINT)(*(int*)eggField_get_value(lp_span_field, &n_field_sz));
    }
    else if(span_field_ty & EGG_INDEX_INT64)
    {
        n_doc_spanpoint = (SPANPOINT)(*(long long*)eggField_get_value(lp_span_field, &n_field_sz));
    }
    else if(span_field_ty & EGG_INDEX_DOUBLE)
    {
        n_doc_spanpoint = (SPANPOINT)(*(double*)eggField_get_value(lp_span_field, &n_field_sz));
    }
    else
    {
        return EGG_FALSE;
    }

    
    if(EGG_SPANPOINT_NOT_FIND == n_spanpoint_idx)
    {
        return EGG_FALSE;
    }
    
    EBOOL ret = EGG_FALSE;
    if(hIndexWriterCt->hChunkHands[n_spanpoint_idx].start <= n_doc_spanpoint && n_doc_spanpoint < hIndexWriterCt->hChunkHands[n_spanpoint_idx].end)
    {
        ret = eggIndexWriter_modify_document(hIndexWriterCt->hChunkHands[n_spanpoint_idx].hChunkObj, dId, hEggDocument);
    }
    else
    {
        index_t  n_spanpoint_new_idx = eggChunkHand_find_spanpoint(hIndexWriterCt->hChunkHands,
                                                                  hIndexWriterCt->chunkCnt, n_doc_spanpoint);

        HEGGDOCUMENT lp_org_document = EGG_NULL;
        HEGGINDEXREADER lp_index_reader  = eggIndexWriter_init_reader(hIndexWriterCt->hChunkHands[n_spanpoint_idx].hChunkObj);
        eggIndexReader_get_document(lp_index_reader, dId, &lp_org_document);
        
        HEGGFIELD lp_field_iter = eggDocument_get_field(hEggDocument, EGG_NULL);

        while (lp_field_iter)
        {
            
            eggDocument_remove_field_byname(lp_org_document, eggField_get_name(hEggDocument));
          
            HEGGFIELD lp_field_next = eggField_get_next(lp_field_iter);
            eggDocument_add(lp_org_document, lp_field_iter);
        
            lp_field_iter = lp_field_next;

        }

        eggIndexWriter_delete_document(hIndexWriterCt->hChunkHands[n_spanpoint_idx].hChunkObj, dId);
        
        eggIndexWriter_add_document(hIndexWriterCt->hChunkHands[n_spanpoint_new_idx].hChunkObj, lp_org_document);
        
        ret = EGG_MODIFY_IDINVALID;
        
        eggDocument_delete(lp_org_document);
    }
    return ret;

}

EBOOL EGGAPI eggIndexWriter_incrementmodify_document_cluster(HEGGINDEXWRITER hEggIndexWriter, EGGDID dId, HEGGDOCUMENT hEggDocument)
{
    HEGGINDEXWRITERCLUSTER hIndexWriterCt = (HEGGINDEXWRITERCLUSTER)hEggIndexWriter;
    
    if(POINTER_IS_INVALID(hIndexWriterCt))
    {
        return EGG_FALSE;
    }
    
    index_t n_spanpoint_idx = eggChunkHand_find_spanpoint(hIndexWriterCt->hChunkHands,
                                                             hIndexWriterCt->chunkCnt, dId.cluster.chunkId);
    if(EGG_SPANPOINT_NOT_FIND == n_spanpoint_idx)
    {
        return EGG_FALSE;
    }
    
    EBOOL ret = eggIndexWriter_incrementmodify_document(hIndexWriterCt->hChunkHands[n_spanpoint_idx].hChunkObj, dId, hEggDocument);

    return ret;
    
}

EBOOL EGGAPI eggIndexWriter_add_field_cluster(HEGGINDEXWRITER hEggIndexWriter, char *fieldName, type_t fieldType, ... /* char *analyzerName */)
{
    HEGGINDEXWRITERCLUSTER hIndexWriterCt = (HEGGINDEXWRITERCLUSTER)hEggIndexWriter;
    
    if(POINTER_IS_INVALID(hIndexWriterCt))
    {
        return EGG_FALSE;
    }
//    fprintf(stderr, "%s Not supported\n", __func__);
    eggPrtLog_warn("eggIndexWriterCluster", "%s Not supported\n", __func__);
    return EGG_FALSE;
}
EBOOL EGGAPI eggIndexWriter_modify_field_cluster(HEGGINDEXWRITER hEggIndexWriter, char *oldFieldName, char *fieldName, type_t fieldType, ... /* char *analyzerName */)
{
    HEGGINDEXWRITERCLUSTER hIndexWriterCt = (HEGGINDEXWRITERCLUSTER)hEggIndexWriter;
    
    if(POINTER_IS_INVALID(hIndexWriterCt))
    {
        return EGG_FALSE;
    }
    //fprintf(stderr, "%s Not supported\n", __func__);
    eggPrtLog_warn("eggIndexWriterCluster", "%s Not supported\n", __func__);
    return EGG_FALSE;
}
EBOOL EGGAPI eggIndexWriter_delete_field_cluster(HEGGINDEXWRITER hEggIndexWriter, char *fieldName)
{
    HEGGINDEXWRITERCLUSTER hIndexWriterCt = (HEGGINDEXWRITERCLUSTER)hEggIndexWriter;
    
    if(POINTER_IS_INVALID(hIndexWriterCt))
    {
        return EGG_FALSE;
    }
    //fprintf(stderr, "%s Not supported\n", __func__);
    eggPrtLog_warn("eggIndexWriterCluster", "%s Not supported\n", __func__);
    return EGG_FALSE;

}

HEGGINDEXREADER EGGAPI eggIndexWriter_init_reader_cluster(HEGGINDEXWRITER hIndexWriter)
{
    HEGGINDEXWRITERCLUSTER hIndexWriterCt = (HEGGINDEXWRITERCLUSTER)hIndexWriter;
    
    if(POINTER_IS_INVALID(hIndexWriterCt))
    {
        return EGG_NULL;
    }
    /*
      !!!!!!!!!!!!!!
     */
    HEGGINDEXREADERCLUSTER  hIndexReaderCt = (HEGGINDEXREADERCLUSTER)eggIndexReader_alloc_cluster();
    if(!eggIndexReader_set_handle_cluster(hIndexReaderCt, hIndexWriterCt->hEggHandle))
    {
        return EGG_NULL;
    }

    if(!eggIndexReader_set_chunkhandles_cluster(hIndexReaderCt, hIndexWriterCt->hChunkHands, hIndexWriterCt->chunkCnt))
    {
        return EGG_NULL;
    }
    
    return (HEGGINDEXREADER)hIndexReaderCt;
}


PRIVATE EBOOL eggIndexWriter_init_chunkHands_cluster(HEGGINDEXWRITERCLUSTER hIndexWriterCt)
{
    if(POINTER_IS_INVALID(hIndexWriterCt))
    {
        return EGG_FALSE;
    }

    index_t i = 0;
    count_t n_chunk_cnt = 0;
    HEGGCHUNKHAND lp_chunk_hands = eggCluster_get_chunkhands((HEGGCLUSTER)(hIndexWriterCt->hEggHandle), &n_chunk_cnt);
    
    hIndexWriterCt->chunkCnt = n_chunk_cnt;
    hIndexWriterCt->hChunkHands = (HEGGCHUNKHAND)malloc(sizeof(EGGCHUNKHAND) * hIndexWriterCt->chunkCnt);

    //copy start and end
    memcpy(hIndexWriterCt->hChunkHands, lp_chunk_hands, sizeof(EGGCHUNKHAND) * hIndexWriterCt->chunkCnt);
    
    while(i < n_chunk_cnt)
    {
        hIndexWriterCt->hChunkHands[i].hChunkObj = (HEGGCHUNKOBJ)eggIndexWriter_open((HEGGHTTP)lp_chunk_hands[i].hChunkObj, "");
        i++;
    }
    
    return EGG_TRUE;
    
}


PRIVATE EBOOL eggIndexWriter_destroy_chunkHands_cluster(HEGGINDEXWRITERCLUSTER hIndexWriterCt)
{
    if(POINTER_IS_INVALID(hIndexWriterCt))
    {
        return EGG_FALSE;
    }

    index_t i = 0;
    
    
    while(i < hIndexWriterCt->chunkCnt)
    {
        eggIndexWriter_close((HEGGINDEXWRITER) hIndexWriterCt->hChunkHands[i].hChunkObj);
        i++;
    }

    free(hIndexWriterCt->hChunkHands);
    
    return EGG_TRUE;
    
}
