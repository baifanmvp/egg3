#include "eggIndexReaderCluster.h"
#include "eggIndexReaderRemote.h"
#include "../eggHttp.h"
#include "../uti/eggThreadPool.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <pthread.h>
#include <signal.h>

struct eggIndexReaderCluster
{
    
    HEGGHANDLE hEggHandle;
    
    HEGGCHUNKHAND hChunkHands;
    count_t chunkCnt;
    
    index_t idxExport;
};
PRIVATE EBOOL eggIndexReader_init_chunkHands_cluster(HEGGINDEXREADERCLUSTER hIndexReaderCt);


PRIVATE EBOOL eggIndexReader_destroy_chunkHands_cluster(HEGGINDEXREADERCLUSTER hIndexReaderCt);

PRIVATE EBOOL eggIndexReader_get_documentset_pthread_cluster(HEGGTHREADTASKDETAILS htaskDetails);

HEGGINDEXREADER EGGAPI eggIndexReader_open_cluster(void *hEggHandle_)
{
    if(POINTER_IS_INVALID(hEggHandle_))
    {
        return EGG_NULL;
    }
    HEGGCLUSTER hEggCluster = (HEGGCLUSTER)hEggHandle_;
    HEGGINDEXREADERCLUSTER hIndexReaderCt = (HEGGINDEXREADERCLUSTER)calloc(1, sizeof(EGGINDEXREADERCLUSTER));

    
    assert(hIndexReaderCt);
    
    hIndexReaderCt->hEggHandle = ((HEGGHANDLE)hEggHandle_)->eggHandle_dup(hEggHandle_);//hEggHandle;
    hIndexReaderCt->idxExport = 0;
    eggIndexReader_init_chunkHands_cluster(hIndexReaderCt);    

    return (HEGGINDEXREADER)hIndexReaderCt;

}

EBOOL EGGAPI eggIndexReader_close_cluster(HEGGINDEXREADER hIndexReader)
{
    HEGGINDEXREADERCLUSTER hIndexReaderCt = (HEGGINDEXREADERCLUSTER)hIndexReader;

    if(POINTER_IS_INVALID(hIndexReaderCt))
    {
        return EGG_FALSE;
    }

    eggIndexReader_destroy_chunkHands_cluster(hIndexReaderCt);
    
    hIndexReaderCt->hEggHandle->eggHandle_delete(hIndexReaderCt->hEggHandle);
    free( hIndexReaderCt->hChunkHands);
    free(hIndexReaderCt);

    return EGG_TRUE;

}

EBOOL EGGAPI eggIndexReader_get_document_cluster(HEGGINDEXREADER hEggIndexReader, EGGDID dId, HEGGDOCUMENT* ppeggDocument)
{
    HEGGINDEXREADERCLUSTER hIndexReaderCt = (HEGGINDEXREADERCLUSTER)hEggIndexReader;

    if(POINTER_IS_INVALID(hIndexReaderCt))
    {
        return EGG_NULL;
    }
    
    index_t n_spanpoint_idx = eggChunkHand_find_spanpoint(hIndexReaderCt->hChunkHands,
                                                             hIndexReaderCt->chunkCnt, dId.cluster.chunkId);

    EBOOL ret = eggIndexReader_get_document(hIndexReaderCt->hChunkHands[n_spanpoint_idx].hChunkObj, dId, ppeggDocument);
    
    return ret;
}

//record orgin sort
typedef struct documentSortInfo  DOCUMENTSORTINFO;
typedef struct documentSortInfo* HDOCUMENTSORTINFO;

struct documentSortInfo
{
    HEGGDOCUMENT hDocument;
    int weight;
};

int documentSortInfo_sort_asc(HDOCUMENTSORTINFO pSrc, HDOCUMENTSORTINFO pDest)
{
    if (pSrc->weight < pDest->weight)
    {
        return 1;
    }
    else if (pSrc->weight > pDest->weight)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}

EBOOL EGGAPI eggIndexReader_get_documentset_cluster(HEGGINDEXREADER hIndexReader, HEGGSCOREDOC hScoreDoc, count_t nDocCnt, HEGGDOCUMENT** pppeggDocument)
{
    
    HEGGINDEXREADERCLUSTER hIndexReaderCt = (HEGGINDEXREADERCLUSTER)hIndexReader;

    if(POINTER_IS_INVALID(hIndexReaderCt))
    {
        return EGG_FALSE;
    }
    //
    UTIVECTOR** lplp_vectors = (UTIVECTOR**)malloc(sizeof(UTIVECTOR*) * hIndexReaderCt->chunkCnt);
    index_t n_vector_idx = 0;
    index_t n_scoredoc_idx = 0;
    UTIVECTOR** lplp_docSort_vectors = (UTIVECTOR**)malloc(sizeof(UTIVECTOR*) * hIndexReaderCt->chunkCnt);
    
    while (n_vector_idx < hIndexReaderCt->chunkCnt)
    {
        lplp_vectors[n_vector_idx] = Uti_vector_create(sizeof(EGGSCOREDOC));
        lplp_docSort_vectors[n_vector_idx] = Uti_vector_create(sizeof(DOCUMENTSORTINFO));
        n_vector_idx++;
    }
    /*
      记录原始的顺序
      把同一个chunk的scoredoc放同一个vector里 
     */
    int sort_weight = 0;
    while (n_scoredoc_idx < nDocCnt)
    {
        index_t n_hand_idx = 0;
        while(n_hand_idx != hIndexReaderCt->chunkCnt)
        {
            if(hIndexReaderCt->hChunkHands[n_hand_idx].start<=EGGDID_CHUNKID(&(hScoreDoc[n_scoredoc_idx].idDoc))
               && EGGDID_CHUNKID(&(hScoreDoc[n_scoredoc_idx].idDoc)) < hIndexReaderCt->hChunkHands[n_hand_idx].end)
            {
                Uti_vector_push(lplp_vectors[n_hand_idx], hScoreDoc + n_scoredoc_idx, 1);
                
                DOCUMENTSORTINFO st_documentsort_info = {0};
                st_documentsort_info.weight = sort_weight++;
                Uti_vector_push(lplp_docSort_vectors[n_hand_idx], &st_documentsort_info, 1);
                
                
                break;
            }
            n_hand_idx++;
        }

        if(n_hand_idx == hIndexReaderCt->chunkCnt)
        {
            n_vector_idx = 0;
            while (n_vector_idx < hIndexReaderCt->chunkCnt)
            {
                
                Uti_vector_destroy(lplp_vectors[n_vector_idx], EGG_TRUE);
                n_vector_idx++;
            }
            free(lplp_vectors);
            return EGG_FALSE;
        }
        
        n_scoredoc_idx++;
    }

    /*
      多线程取document，把document把进记录过原始顺序的sort_info里面
     */
    HEGGDOCUMENT* lplp_eggDocument_set = (HEGGDOCUMENT*)malloc(sizeof(HEGGDOCUMENT)*hIndexReaderCt->chunkCnt);
    pthread_t* pthread_ids = (pthread_t*)malloc(sizeof(pthread_t) * hIndexReaderCt->chunkCnt);
    HEGGTHREADTASKDETAILS* lplp_details = (HEGGTHREADTASKDETAILS*)malloc(sizeof(HEGGTHREADTASKDETAILS) * hIndexReaderCt->chunkCnt);
    
    n_vector_idx = 0;
    while (n_vector_idx < hIndexReaderCt->chunkCnt)
    {
        lplp_details[n_vector_idx] = eggThreadPool_taskdetails_init(4,
                                                                    &(hIndexReaderCt->hChunkHands[n_vector_idx].hChunkObj),
                                                                    &(Uti_vector_data(lplp_vectors[n_vector_idx])),
                                                                    &(Uti_vector_count(lplp_vectors[n_vector_idx])),
                                                                    &(lplp_docSort_vectors[n_vector_idx]));
        
        pthread_create(pthread_ids + n_vector_idx, EGG_NULL,
                       eggIndexReader_get_documentset_pthread_cluster,  lplp_details[n_vector_idx]);
        n_vector_idx++;
    }
    
    n_vector_idx = 0;
    while (n_vector_idx < hIndexReaderCt->chunkCnt)
    {
        EBOOL ret = 0;
        pthread_join(pthread_ids[n_vector_idx], &ret);
        eggThreadPool_taskdetails_destroy(lplp_details[n_vector_idx]);  
        n_vector_idx++;
    }

    /*
      把多个代表chunk的sort_info合并后排序（按原始的顺序）
     */
    HDOCUMENTSORTINFO lp_sortinfo_total = (HDOCUMENTSORTINFO)malloc(sizeof(DOCUMENTSORTINFO) * nDocCnt);
    count_t n_sortinfo_total = 0;
    
    n_vector_idx = 0;
    while (n_vector_idx < hIndexReaderCt->chunkCnt)
    {
        HDOCUMENTSORTINFO lp_sortinfo_tmp = Uti_vector_data(lplp_docSort_vectors[n_vector_idx]);
        count_t n_sortinfo_cnt = Uti_vector_count(lplp_docSort_vectors[n_vector_idx]);
        
        memcpy(lp_sortinfo_total + n_sortinfo_total, lp_sortinfo_tmp, sizeof(DOCUMENTSORTINFO) * n_sortinfo_cnt);
        
        n_sortinfo_total += n_sortinfo_cnt;
        Uti_vector_destroy(lplp_docSort_vectors[n_vector_idx], EGG_TRUE);
        Uti_vector_destroy(lplp_vectors[n_vector_idx], EGG_TRUE);
        n_vector_idx++;
        
    }

    Uti_sedgesort (lp_sortinfo_total, n_sortinfo_total, sizeof(DOCUMENTSORTINFO), documentSortInfo_sort_asc);
    
    *pppeggDocument = (HEGGDOCUMENT*)malloc(sizeof(HEGGDOCUMENT) * nDocCnt);
    
    n_scoredoc_idx = 0;
    while (n_scoredoc_idx < nDocCnt)
    {
        (*pppeggDocument)[n_scoredoc_idx] = lp_sortinfo_total[n_scoredoc_idx].hDocument;
        n_scoredoc_idx++;
    }

    free(lp_sortinfo_total);
    free(lplp_vectors);
    free(lplp_docSort_vectors);
    return EGG_TRUE;

}



HEGGDOCUMENT EGGAPI eggIndexReader_export_document_cluster(HEGGINDEXREADER hIndexReader, offset64_t* pCursor)
{
    HEGGINDEXREADERCLUSTER hIndexReaderCt = (HEGGINDEXREADERCLUSTER)hIndexReader;

    if(POINTER_IS_INVALID(hIndexReaderCt))
    {
        return EGG_NULL;
    }
    
    HEGGCHUNKHAND lp_Chunk_hands = hIndexReaderCt->hChunkHands;
    count_t n_chunk_cnt = hIndexReaderCt->chunkCnt;
    index_t n_chunk_idx =  0;
    
    if(*pCursor == 0)
    {
        hIndexReaderCt->idxExport = 0;
    }
    
    HEGGDOCUMENT lp_document = eggIndexReader_export_document(lp_Chunk_hands[hIndexReaderCt->idxExport].hChunkObj, pCursor);    
    if(lp_document == EGG_NULL && ++hIndexReaderCt->idxExport < n_chunk_cnt)
    {
        *pCursor = 0;
        lp_document = eggIndexReader_export_document(lp_Chunk_hands[hIndexReaderCt->idxExport].hChunkObj, pCursor);        
    }
    return lp_document;

}

EBOOL EGGAPI eggIndexReader_get_fieldnameinfo_cluster(HEGGINDEXREADER hIndexReader, HEGGFIELDNAMEINFO *hhFieldNameInfo, count_t *lpCntFieldNameInfo)
{

    HEGGINDEXREADERCLUSTER hIndexReaderCt = (HEGGINDEXREADERCLUSTER)hIndexReader;

    if(POINTER_IS_INVALID(hIndexReaderCt))
    {
        return EGG_NULL;
    }
    
    EBOOL retv = EGG_FALSE;
    *hhFieldNameInfo = NULL;
    *lpCntFieldNameInfo = 0;
    
    HEGGCHUNKHAND lp_Chunk_hands = hIndexReaderCt->hChunkHands;
    count_t n_chunk_cnt = hIndexReaderCt->chunkCnt;
    index_t n_chunk_idx =  0;
    while (n_chunk_idx < n_chunk_cnt)
    {
        retv = eggIndexReader_get_fieldnameinfo(lp_Chunk_hands[n_chunk_idx].hChunkObj,
                                                hhFieldNameInfo, lpCntFieldNameInfo);
        if (retv == EGG_TRUE)
        {
            break;              /* fetch first */
        }
        n_chunk_idx++;
    }
    
    return retv;
}

EBOOL EGGAPI eggIndexReader_get_singlefieldnameinfo_cluster(HEGGINDEXREADER hIndexReader, char *fieldName, HEGGFIELDNAMEINFO *hhFieldNameInfo)
{
    HEGGINDEXREADERCLUSTER hIndexReaderCt = (HEGGINDEXREADERCLUSTER)hIndexReader;

    if(POINTER_IS_INVALID(hIndexReaderCt))
    {
        return EGG_NULL;
    }
    
    EBOOL retv = EGG_FALSE;
    *hhFieldNameInfo = NULL;
    
    HEGGCHUNKHAND lp_Chunk_hands = hIndexReaderCt->hChunkHands;
    count_t n_chunk_cnt = hIndexReaderCt->chunkCnt;
    index_t n_chunk_idx =  0;
    while (n_chunk_idx < n_chunk_cnt)
    {
        retv = eggIndexReader_get_singlefieldnameinfo(lp_Chunk_hands[n_chunk_idx].hChunkObj, fieldName, hhFieldNameInfo);
        if (retv == EGG_TRUE)
        {
            break;              /* fetch first */
        }
        n_chunk_idx++;
    }

    return retv;
}

count_t EGGAPI eggIndexReader_get_doctotalcnt_cluster(HEGGINDEXREADER hIndexReader)
{
    HEGGINDEXREADERCLUSTER hIndexReaderCt = (HEGGINDEXREADERCLUSTER)hIndexReader;
    if(POINTER_IS_INVALID(hIndexReaderCt))
    {
        return EGG_NULL;
    }
    
    HEGGCHUNKHAND lp_Chunk_hands = hIndexReaderCt->hChunkHands;
    count_t n_chunk_cnt = hIndexReaderCt->chunkCnt;
    index_t n_chunk_idx =  0;
    count_t n_count = 0;
    while (n_chunk_idx < n_chunk_cnt)
    {
        n_count += eggIndexReader_get_doctotalcnt(lp_Chunk_hands[n_chunk_idx].hChunkObj);
        n_chunk_idx++;
    }
    return n_count;

}



PUBLIC EBOOL EGGAPI eggIndexReader_set_handle_cluster(HEGGINDEXREADERCLUSTER hIndexReader, void *hEggHandle_)
{
    HEGGINDEXREADERCLUSTER hIndexReaderCt = (HEGGINDEXREADERCLUSTER)hIndexReader;

    if(POINTER_IS_INVALID(hIndexReaderCt))
    {
        return EGG_FALSE;
    }
    
    hIndexReaderCt->hEggHandle = (HEGGHANDLE)hEggHandle_;
    return EGG_TRUE;
}

PUBLIC EBOOL EGGAPI eggIndexReader_set_chunkhandles_cluster(HEGGINDEXREADERCLUSTER hIndexReader, HEGGCHUNKHAND hChunkHands, count_t chunkCnt)
{
    HEGGINDEXREADERCLUSTER hIndexReaderCt = (HEGGINDEXREADERCLUSTER)hIndexReader;
    if(POINTER_IS_INVALID(hIndexReaderCt))
    {
        return EGG_FALSE;
    }
    
    
    index_t i = 0;
    hIndexReaderCt->chunkCnt = chunkCnt;

    hIndexReaderCt->hChunkHands = (HEGGCHUNKHAND)malloc(sizeof(EGGCHUNKHAND) * chunkCnt);

    //copy start and end
    memcpy(hIndexReaderCt->hChunkHands, hChunkHands, sizeof(EGGCHUNKHAND) * chunkCnt);
    
    while(i < chunkCnt)
    {
        hIndexReaderCt->hChunkHands[i].hChunkObj = (HEGGCHUNKOBJ)eggIndexWriter_init_reader(hChunkHands[i].hChunkObj);
        i++;
    }

    
    
    return EGG_TRUE;
}

HEGGINDEXREADER EGGAPI eggIndexReader_alloc_cluster()
{
    return malloc(sizeof(EGGINDEXREADERCLUSTER));
}


EBOOL EGGAPI eggIndexReader_free_cluster(HEGGINDEXREADER hIndexReader_)
{
    EGGINDEXREADERCLUSTER *hIndexReaderCt = (EGGINDEXREADERCLUSTER *)hIndexReader_;
//    hIndexReader->hEggHandle->eggHandle_delete(hIndexReader->hEggHandle);

    index_t i = 0;

    //copy start and end
    count_t chunkCnt = hIndexReaderCt->chunkCnt;
    while(i < chunkCnt)
    {
        eggIndexReader_free(hIndexReaderCt->hChunkHands[i].hChunkObj);
        i++;
    }


    free(hIndexReaderCt->hChunkHands);
    free(hIndexReaderCt);
    return EGG_TRUE;
}

count_t EGGAPI eggIndexReader_get_chunkcnt_cluster(HEGGINDEXREADERCLUSTER hIndexReaderCt)
{
    if(POINTER_IS_INVALID(hIndexReaderCt))
    {
        return EGG_NULL;
    }
    return hIndexReaderCt->chunkCnt;
}

HEGGCHUNKHAND EGGAPI eggIndexReader_get_chunkhands_cluster(HEGGINDEXREADERCLUSTER hIndexReaderCt)
{
    if(POINTER_IS_INVALID(hIndexReaderCt))
    {
        return EGG_NULL;
    }
    return hIndexReaderCt->hChunkHands;
    
}


PRIVATE EBOOL eggIndexReader_init_chunkHands_cluster(HEGGINDEXREADERCLUSTER hIndexReaderCt)
{
    if(POINTER_IS_INVALID(hIndexReaderCt))
    {
        return EGG_FALSE;
    }

    index_t i = 0;
    count_t n_chunk_cnt = 0;
    HEGGCHUNKHAND lp_chunk_hands = eggCluster_get_chunkhands((HEGGCLUSTER)(hIndexReaderCt->hEggHandle), &n_chunk_cnt);
    
    hIndexReaderCt->chunkCnt = n_chunk_cnt;
    hIndexReaderCt->hChunkHands = (HEGGCHUNKHAND)malloc(sizeof(EGGCHUNKHAND) * hIndexReaderCt->chunkCnt);

    //copy start and end
    memcpy(hIndexReaderCt->hChunkHands, lp_chunk_hands, sizeof(EGGCHUNKHAND) * hIndexReaderCt->chunkCnt);
    
    while(i < n_chunk_cnt)
    {
        hIndexReaderCt->hChunkHands[i].hChunkObj = (HEGGCHUNKOBJ)eggIndexReader_open((HEGGHTTP)lp_chunk_hands[i].hChunkObj);
        i++;
    }
    
    return EGG_TRUE;
    
}


PRIVATE EBOOL eggIndexReader_destroy_chunkHands_cluster(HEGGINDEXREADERCLUSTER hIndexReaderCt)
{
    if(POINTER_IS_INVALID(hIndexReaderCt))
    {
        return EGG_FALSE;
    }

    index_t i = 0;
    
    
    while(i < hIndexReaderCt->chunkCnt)
    {
        eggIndexReader_close((HEGGINDEXREADER) hIndexReaderCt->hChunkHands[i].hChunkObj);
        i++;
    }
    
    return EGG_TRUE;
    
}

PRIVATE EBOOL eggIndexReader_get_documentset_pthread_cluster(HEGGTHREADTASKDETAILS htaskDetails)
{
    if(POINTER_IS_INVALID(htaskDetails))
    {
        return EGG_FALSE;
    }
    
    HDOCUMENTSORTINFO lp_sort_info = Uti_vector_data(*(UTIVECTOR**)htaskDetails->details[3]);
    count_t n_doc_cnt = *(count_t*)(htaskDetails->details[2]);
    HEGGSCOREDOC lp_score_doc = *(HEGGSCOREDOC*)(htaskDetails->details[1]);
    HEGGINDEXREADER lp_index_reader = *(HEGGINDEXREADER*)(htaskDetails->details[0]);
    
    HEGGDOCUMENT* lplp_document = EGG_NULL;
    
    EBOOL ret =eggIndexReader_get_documentset(lp_index_reader,
                                                     lp_score_doc,
                                                     n_doc_cnt,
                                                     &lplp_document);
    
    index_t n_doc_idx = 0;
    
    while(n_doc_idx < n_doc_cnt)
    {
        lp_sort_info[n_doc_idx].hDocument = lplp_document[n_doc_idx];
        
        n_doc_idx++;
    }
    
    free(lplp_document);
    return ret;
}
