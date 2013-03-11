#include "eggIndexReaderLocal.h"
#include "../eggDirectory.h"
#include "../eggDocument.h"
#include "../eggAnalyzer.h"
#include "../eggQuery.h"
#include "../index/eggFieldWeight.h"
#include <assert.h>

extern pthread_mutex_t counter_mutex ;

struct eggIndexReaderLocal
{
    HEGGHANDLE hEggHandle;      /* must be first */
    
    HEGGINDEXCACHE refIndexCache;
    HEGGINDEXVIEW hIndexView;
    HEGGIDVIEW hIdView;
    HEGGDOCVIEW hDocView;
    HEGGFIELDVIEW hFieldView;
    HEGGFIELDWEIGHT hFieldWeight;

};
typedef struct eggIndexReaderLocal EGGINDEXREADERLOCAL;

#define EGGINDEXREADER_IS_INVALID(hReader)  ((!hReader) ? EGG_TRUE : EGG_FALSE)
PRIVATE  fdid_t eggIndexReader_set_indexView_with_field(EGGINDEXREADERLOCAL *hIndexReader, char* fieldName, type_t* ptype);

PRIVATE EBOOL eggIndexReader_query_with_cache(EGGINDEXREADERLOCAL *hIndexReader, HEGGINDEXCACHEKEY hCacheKey, HEGGIDNODE* hIdNodes, count_t* lpCntDoc);


PRIVATE EBOOL eggIndexReader_query_with_single(EGGINDEXREADERLOCAL *hIndexReader, fdid_t fdid, type_t ftype, echar* lpKey, size16_t kSz, HEGGIDNODE* hIdNodes, count_t* lpCntDoc);

PRIVATE EBOOL eggIndexReader_query_with_range(EGGINDEXREADERLOCAL *hIndexReader,  fdid_t fdid, type_t ftype, echar* lpStaKey, size16_t kStaSz, echar* lpEndKey, size16_t kEndSz, HEGGINDEXREADERRESULT hReaderResult);


struct eggIndexReaderResult {
    HEGGIDNODE hIdNodes;
    count_t nIdNodes;
    HEGGWRESULT hRangeCache;
    count_t nRangeCache;
    
};



HEGGINDEXREADER EGGAPI eggIndexReader_open_local(void *hEggHandle_)
{
    HEGGDIRECTORY hDirectory = (HEGGDIRECTORY)hEggHandle_;
    if( !hDirectory)
    {
        return EGG_NULL;
    }

    EGGINDEXREADERLOCAL *lp_index_reader = (EGGINDEXREADERLOCAL*)malloc(sizeof(EGGINDEXREADERLOCAL));
    
    lp_index_reader->hEggHandle = eggDirectory_dup(hDirectory);
    
    eggDirectory_init(hDirectory);

    lp_index_reader->refIndexCache = EGG_NULL;
    
    HEGGIDTABLE hIdTable = eggIdTable_new(eggDirectory_get_file(hDirectory, EGG_FDAT_IDT));

    
    lp_index_reader->hIndexView = eggIndexView_new(eggDirectory_get_file(hDirectory, EGG_FIDX), EGG_NULL);
        
    /* recovery */
    HEGGRECOVERYHANDLE hRecoveryHandle = eggRecoveryLog_init(ViewStream_name(lp_index_reader->hIndexView->hViewStream));
    eggRecoveryLog_destroy(hRecoveryHandle);
        
    lp_index_reader->hIdView = eggIdView_new(eggDirectory_get_file(hDirectory, EGG_FID));
    lp_index_reader->hDocView = eggDocView_new(eggDirectory_get_file(hDirectory, EGG_FDAT), hIdTable);
    lp_index_reader->hFieldView = eggFieldView_new(eggDirectory_get_file(hDirectory, EGG_FFD));
    lp_index_reader->hFieldWeight = eggFieldWeight_new(eggDirectory_get_file(hDirectory, EGG_FFW), lp_index_reader->hFieldView);

    return (HEGGINDEXREADER)lp_index_reader;

}

EBOOL EGGAPI eggIndexReader_close_local(HEGGINDEXREADER hIndexReader_)
{
    EGGINDEXREADERLOCAL *hIndexReader = (EGGINDEXREADERLOCAL *)hIndexReader_;
    if(EGGINDEXREADER_IS_INVALID(hIndexReader))
    {
        return EGG_FALSE;
    }

    eggDirectory_delete(hIndexReader->hEggHandle);
    
    eggIndexView_delete(hIndexReader->hIndexView);
    eggIdView_delete(hIndexReader->hIdView);
    eggDocView_delete(hIndexReader->hDocView);
    eggFieldView_delete(hIndexReader->hFieldView);
    eggFieldWeight_delete(hIndexReader->hFieldWeight);
    free(hIndexReader);
    hIndexReader = EGG_NULL;
    
    return EGG_TRUE;
}

EBOOL EGGAPI eggIndexReader_ref_indexcache_local(HEGGINDEXREADER EGGAPI hEggIndexReader_, HEGGINDEXCACHE hIndexCache)
{
    EGGINDEXREADERLOCAL *hEggIndexReader = (EGGINDEXREADERLOCAL *)hEggIndexReader_;
    
    if(EGGINDEXREADER_IS_INVALID(hEggIndexReader))
    {
        return EGG_FALSE;
    }
    if(EGGINDEXREADER_IS_INVALID(hEggIndexReader->refIndexCache))
    {
       eggIndexCache_delete(hEggIndexReader->refIndexCache); 
    }

    eggIndexCache_ref(&hEggIndexReader->refIndexCache, hIndexCache);

    return EGG_TRUE;
}


EBOOL EGGAPI eggIndexReader_query_documents_local(HEGGINDEXREADER hIndexReader_, HEGGQUERYEXP hQueryExp, HEGGINDEXREADERRESULT *hhReaderResult)
{
    *hhReaderResult = eggIndexReaderResult_new();
    if (!*hhReaderResult)
    {
        return EGG_FALSE;
    }
    HEGGIDNODE* hIdNodes = &(*hhReaderResult)->hIdNodes;
    count_t* lpCntDoc = &(*hhReaderResult)->nIdNodes;
    
    EGGINDEXREADERLOCAL *hIndexReader = (EGGINDEXREADERLOCAL *)hIndexReader_;
    if (EGGINDEXREADER_IS_INVALID(hIndexReader) || !hQueryExp)
    {
        *lpCntDoc = 0;
        return EGG_FALSE;
    }
    fdid_t fdid = 0;
    type_t ftype = 0;
    if ((fdid = eggIndexReader_set_indexView_with_field(hIndexReader, hQueryExp->fieldName, &ftype)) == 0)
    {
        *lpCntDoc = 0;
        return EGG_FALSE;
    }

    if (hQueryExp->opt == EGGQUERYOPT_TERM || hQueryExp->opt == EGGQUERYOPT_TERMOR || hQueryExp->opt == EGGQUERYOPT_PHRASE)
    {
        HEGGIDNODE lp_idnode_cache = EGG_NULL;
        HEGGIDNODE lp_idnode_single = EGG_NULL;
        count_t n_cache_cnt = 0;
        count_t n_single_cnt = *lpCntDoc;

        if(hIndexReader->refIndexCache)
        {
            size16_t n_field_mask = (size16_t)fdid;
            HEGGINDEXCACHEKEY lp_cache_key = eggIndexCacheKey_new(0,
                                                                  n_field_mask, hQueryExp->key1, hQueryExp->key1Sz);
        
            eggIndexReader_query_with_cache(hIndexReader, lp_cache_key, &lp_idnode_cache, &n_cache_cnt);
            eggIndexCacheKey_delete(lp_cache_key);
        }

        eggIndexReader_query_with_single(hIndexReader, fdid, ftype, hQueryExp->key1, hQueryExp->key1Sz, &lp_idnode_single, &n_single_cnt);

        if(n_cache_cnt)
        {
            *lpCntDoc = n_cache_cnt + n_single_cnt;
            *hIdNodes = (HEGGIDNODE)malloc((*lpCntDoc) * sizeof(EGGIDNODE));
            if(n_cache_cnt)
            {
                memcpy(*hIdNodes, lp_idnode_cache, n_cache_cnt * sizeof(EGGIDNODE));
                free(lp_idnode_cache);
            }
            if(n_single_cnt)
            {
                memcpy((*hIdNodes) + n_cache_cnt, lp_idnode_single, n_single_cnt * sizeof(EGGIDNODE));
                free(lp_idnode_single);
            }
            
        }
        else
        {
            *hIdNodes = lp_idnode_single;
            *lpCntDoc = n_single_cnt;
        }
        if(*lpCntDoc)
        {
            Uti_sedgesort (*hIdNodes, *lpCntDoc, sizeof(EGGIDNODE), eggIdNode_cmp_id);
            eggIdNode_filter_repeat(*hIdNodes, lpCntDoc);
        }
    }
    else if (hQueryExp->opt == EGGQUERYOPT_RANGE)
    {
        eggIndexReader_query_with_range(hIndexReader,
                                        fdid, ftype,
                                        hQueryExp->key1, hQueryExp->key1Sz, hQueryExp->key2, hQueryExp->key2Sz,
                                        *hhReaderResult);
    }
    
    return EGG_TRUE;
}

void* mallocret()
{
  return malloc(256);
}
EBOOL EGGAPI eggIndexReader_get_document_local(HEGGINDEXREADER hIndexReader_, EGGDID dId, HEGGDOCUMENT* ppeggDocument)
{
    EGGINDEXREADERLOCAL *hIndexReader = (EGGINDEXREADERLOCAL *)hIndexReader_;
    if(EGGINDEXREADER_IS_INVALID(hIndexReader))
    {
        return EGG_FALSE;
    }
    HEGGDOCNODE lp_doc_node = EGG_NULL;
    did_t id = EGGDID_DOCID(&dId);
       pthread_mutex_lock(&counter_mutex);

    EBOOL ret = eggDocView_query(hIndexReader->hDocView, id, &lp_doc_node);
    pthread_mutex_unlock(&counter_mutex);

     if(!lp_doc_node)
     {
         return ret;
     }

    *ppeggDocument = eggDocument_unserialization(lp_doc_node);

    return EGG_TRUE;
}


EBOOL EGGAPI eggIndexReader_get_documentset_local(HEGGINDEXREADER hIndexReader_, HEGGSCOREDOC hScoreDoc, count_t nDocCnt, HEGGDOCUMENT** pppeggDocument)
{
    EGGINDEXREADERLOCAL *hIndexReader = (EGGINDEXREADERLOCAL *)hIndexReader_;
    if(EGGINDEXREADER_IS_INVALID(hIndexReader))
    {
        return EGG_FALSE;
    }

    *pppeggDocument = (HEGGDOCUMENT*)malloc(sizeof(HEGGDOCUMENT)*nDocCnt);
    index_t n_doc_idx = 0;
    while(n_doc_idx < nDocCnt)
    {
        HEGGDOCNODE lp_doc_node = EGG_NULL;
        
        did_t id = EGGDID_DOCID(&EGGSCOREDOC_ID_I(hScoreDoc, n_doc_idx));
    
        eggDocView_query(hIndexReader->hDocView, id, &lp_doc_node);
        
        if(!lp_doc_node)
        {
            (*pppeggDocument)[n_doc_idx] = EGG_NULL;
        }
        else
        {
            (*pppeggDocument)[n_doc_idx] = eggDocument_unserialization(lp_doc_node);
        }
        n_doc_idx++;
    }
    return EGG_TRUE;
}



HEGGDOCUMENT EGGAPI eggIndexReader_export_document_local(HEGGINDEXREADER hIndexReader_, offset64_t* pCursor)
{
    EGGINDEXREADERLOCAL *hIndexReader = (EGGINDEXREADERLOCAL *)hIndexReader_;
    if(EGGINDEXREADER_IS_INVALID(hIndexReader))
    {
        return EGG_NULL;
    }

    offset64_t n_doc_off = 0;
    do
    {
        n_doc_off = eggIdTable_get_off(hIndexReader->hDocView->hIdTable, *pCursor);
        
        if(n_doc_off == TABLE_ID_OVERFLOW) return EGG_NULL;
        
        (*pCursor)++;
    }while(n_doc_off == TABLE_ID_DELETE);


    

    HEGGDOCUMENT lp_document = EGG_NULL;
    EBOOL eRet = EGG_FALSE;
    EGGDOCNODE st_dn_info = {0};
        
        
    eRet = ViewStream_read(hIndexReader->hDocView->hViewStream,
                           &st_dn_info,
                           sizeof(EGGDOCNODE),
                           (offset64_t)n_doc_off);
        
    if(eRet == EGG_FALSE) return EGG_NULL;
        
    HEGGDOCNODE lp_doc_node = (HEGGDOCNODE)malloc(st_dn_info.size);
    
    ViewStream_read(hIndexReader->hDocView->hViewStream,
                    lp_doc_node,
                    st_dn_info.size,
                    (offset64_t)n_doc_off);



    lp_document = eggDocument_unserialization(lp_doc_node);

    
        
    return lp_document;
    
}

EBOOL EGGAPI eggIndexReader_get_fieldnameinfo_local(HEGGINDEXREADER hIndexReader_, HEGGFIELDNAMEINFO *hhFieldNameInfo, count_t *lpCntFieldNameInfo)
{
    EGGINDEXREADERLOCAL * hIndexReader = (EGGINDEXREADERLOCAL *)hIndexReader_;
    *hhFieldNameInfo = eggFieldView_get_fieldnameinfo(hIndexReader->hFieldView, lpCntFieldNameInfo);
    return EGG_TRUE;
}

EBOOL EGGAPI eggIndexReader_get_singlefieldnameinfo_local(HEGGINDEXREADER hIndexReader_, char *fieldName, HEGGFIELDNAMEINFO *hhFieldNameInfo)
{
    EGGINDEXREADERLOCAL * hIndexReader = (EGGINDEXREADERLOCAL *)hIndexReader_;
    *hhFieldNameInfo = eggFieldView_get_singlefieldnameinfo(hIndexReader->hFieldView, fieldName);
    if (!*hhFieldNameInfo)
    {
        return EGG_FALSE;
    }
    return EGG_TRUE;
}

count_t EGGAPI eggIndexReader_get_doctotalcnt_local(HEGGINDEXREADER hIndexReader_)
{
    EGGINDEXREADERLOCAL * hIndexReader = (EGGINDEXREADERLOCAL *)hIndexReader_;
    return eggDocView_get_doccnt(hIndexReader->hDocView);
}

EBOOL  EGGAPI eggIndexReader_clean_cache(HEGGINDEXREADER hIndexReader_)
{
    EGGINDEXREADERLOCAL * hIndexReader = (EGGINDEXREADERLOCAL *)hIndexReader_;
    eggFieldWeight_clean_cache(hIndexReader->hFieldWeight);
    return EGG_TRUE;
}

/* count_t EGGAPI eggIndexReader_get_docCnt_with_key_local(HEGGINDEXREADER hIndexReader_, char* lpFieldName, char* lpKey, size16_t kSz) */
/* { */
/*     EGGINDEXREADERLOCAL * hIndexReader =(EGGINDEXREADERLOCAL *)hIndexReader_; */
/*     if(EGGINDEXREADER_IS_INVALID(hIndexReader)) */
/*     { */
/*         return EGG_FALSE; */
/*     } */
    
/*     fdid_t fdid = 0; */
/*     if ((fdid = eggIndexReader_set_indexView_with_field(hIndexReader, lpFieldName)) == 0) */
/*     { */
/*         return EGG_FALSE; */
/*     } */

/*     HEGGINDEXRECORD lp_index_rd = eggIndexView_fetch(hIndexReader->hIndexView, lpKey, kSz); */
/*     if(!lp_index_rd) */
/*     { */
/*         return EGG_FALSE; */
/*     } */
    
/*     offset64_t n_id_off = *(offset64_t*)EGGINDEXRECORD_VAL(lp_index_rd); */
/*     if(eggIdView_load(hIndexReader->hIdView, n_id_off)) */
/*     { */
/*         return hIndexReader->hIdView->hEggListView->hInfo->nodeCnt; */
/*     } */
/*     else */
/*     { */
/*         return EGG_FALSE; */
/*     } */

/* } */


PRIVATE  fdid_t eggIndexReader_set_indexView_with_field(EGGINDEXREADERLOCAL *hIndexReader, char* fieldName, type_t* pftype)
{
    if(EGGINDEXREADER_IS_INVALID(hIndexReader))
    {
        return EGG_FALSE;
    }
    
    fdid_t fid = 0;
    
    if(eggFieldView_find(hIndexReader->hFieldView, fieldName, &fid))
    {
//        eggFieldView_release_indexinfo(hIndexReader->hFieldView, fid, lp_index_info);
        EGGINDEXVIEW_FIELDVIEW(hIndexReader->hIndexView) = hIndexReader->hFieldView;
        *pftype = eggFieldView_get_type(hIndexReader->hFieldView, fieldName);
        return fid;
    }
    return EGG_FALSE;
    
}


PRIVATE EBOOL eggIndexReader_query_with_cache(EGGINDEXREADERLOCAL *hIndexReader, HEGGINDEXCACHEKEY hCacheKey, HEGGIDNODE* hIdNodes, count_t* lpCntDoc)
{
    if(EGGINDEXREADER_IS_INVALID(hIndexReader))
    {
        return EGG_FALSE;
    }
    if(EGGINDEXCACHE_IS_INVALID(hIndexReader->refIndexCache))
    {
        return EGG_FALSE; 
    }

    HEGGINDEXCACHEVAL lp_cache_val = eggIndexCache_lookup(hIndexReader->refIndexCache, hCacheKey);
    if(!lp_cache_val)
    {
        return EGG_FALSE; 
    }
        
    UTIVECTOR* lp_vector = Uti_vector_create(sizeof(EGGIDNODE));

    Uti_vector_push(lp_vector, Uti_vector_data(lp_cache_val), Uti_vector_count(lp_cache_val));
        
    *hIdNodes = Uti_vector_data(lp_vector);
    *lpCntDoc = Uti_vector_count(lp_vector);

    eggIdNode_set_timestamp(*hIdNodes, *lpCntDoc, (u32)(-1) - *lpCntDoc );
    Uti_vector_destroy(lp_vector, EGG_FALSE);

    return EGG_TRUE;
}

PRIVATE EBOOL eggIndexReader_query_with_single(EGGINDEXREADERLOCAL *hIndexReader, fdid_t fdid, type_t ftype, echar* lpKey, size16_t kSz, HEGGIDNODE* hIdNodes, count_t* lpCntDoc)
{
    if(EGGINDEXREADER_IS_INVALID(hIndexReader))
    {
        *lpCntDoc = 0;
        return EGG_FALSE;
    }
    
    pthread_mutex_lock(&counter_mutex);    
    if(ftype & EGG_RANGE_INDEX)
    {
        eggFieldView_slock(hIndexReader->hFieldView, fdid);

        HEGGINDEXRANGE lp_rangeids_ret = eggIndexView_fetch_docid_withfid(hIndexReader->hIndexView, fdid, lpKey, kSz, 0, sizeof(did_t));
        if(lp_rangeids_ret && lp_rangeids_ret->cnt)
        {
        
            *lpCntDoc = lp_rangeids_ret->cnt;

            
            *hIdNodes = calloc(*lpCntDoc,
                                   sizeof(EGGIDNODE));
            assert(*hIdNodes);
            index_t idx = 0;
            while (idx != *lpCntDoc)
            {
                (*hIdNodes)[idx].id = lp_rangeids_ret->dids[idx].did;
                (*hIdNodes)[idx].mask = (size16_t)fdid;
                (*hIdNodes)[idx].flag = EGG_IDNODE_VALID;
                *(size16_t*)(*hIdNodes)[idx].pos = (size16_t)1;
                idx++;
            }
            free(lp_rangeids_ret->dids);

        }
        else
        {
            *lpCntDoc = 0;
            *hIdNodes = NULL;
        }
        free(lp_rangeids_ret);
        eggFieldView_unlock(hIndexReader->hFieldView, fdid);        
        
    }
    else
    {
        HEGGINDEXRECORD lp_index_rd = eggIndexView_fetch_withfid(hIndexReader->hIndexView, fdid, lpKey, kSz, 0, sizeof(EGGRECORDDOCID));
    
        if(!lp_index_rd)
        {
            *lpCntDoc = 0;
            pthread_mutex_unlock(&counter_mutex);    
            return EGG_FALSE;
        }
        
        eggIdView_find(hIndexReader->hIdView, *(offset64_t*)EGGINDEXRECORD_VAL(lp_index_rd), hIdNodes, lpCntDoc);
    
        eggIndexRecord_delete(lp_index_rd);
    
    }
    
    pthread_mutex_unlock(&counter_mutex);    

    return EGG_TRUE;
}

PRIVATE EBOOL eggIndexReader_query_with_range(EGGINDEXREADERLOCAL *hIndexReader,  fdid_t fdid, type_t ftype, echar* lpStaKey, size16_t kStaSz, echar* lpEndKey, size16_t kEndSz, HEGGINDEXREADERRESULT hReaderResult)
{
    HEGGIDNODE* hIdNodes = &hReaderResult->hIdNodes;
    count_t* lpCntDoc = &hReaderResult->nIdNodes;

    
    if(EGGINDEXREADER_IS_INVALID(hIndexReader))
    {
        *lpCntDoc = 0;
        return EGG_FALSE;
    }
    pthread_mutex_lock(&counter_mutex);    

    if(ftype & EGG_RANGE_INDEX)
    {
        eggFieldView_slock(hIndexReader->hFieldView, fdid);
        
        HEGGINDEXRANGE lp_rangeids_ret = eggIndexView_range_withfid(hIndexReader->hIndexView, fdid, lpStaKey, kStaSz, lpEndKey, kEndSz);
        if(lp_rangeids_ret && lp_rangeids_ret->cnt)
        {
            *lpCntDoc = lp_rangeids_ret->cnt;
            
            *hIdNodes = calloc(*lpCntDoc,
                               sizeof(EGGIDNODE));
            HEGGWRESULT hWeightResult = malloc((*lpCntDoc) * sizeof(EGGWRESULT));
            assert(*hIdNodes);
            index_t idx = 0;
            while (idx != *lpCntDoc)
            {
                (*hIdNodes)[idx].id = lp_rangeids_ret->dids[idx].did;
                (*hIdNodes)[idx].mask = (size16_t)fdid;
                *(size16_t*)(*hIdNodes)[idx].pos = (size16_t)1;

                idx++;
            }
            memcpy(hWeightResult, lp_rangeids_ret->dids, sizeof(EGGWRESULT) * (*lpCntDoc));
            free(lp_rangeids_ret->dids);

            free(hReaderResult->hRangeCache);
            hReaderResult->hRangeCache = hWeightResult;
            hReaderResult->nRangeCache = *lpCntDoc;
            
        }
        else
        {
            *lpCntDoc = 0;
            *hIdNodes = NULL;
        }
        free(lp_rangeids_ret);
        
        eggFieldView_unlock(hIndexReader->hFieldView, fdid);
        
    }
    else
    {

	
        count_t cnt = 0;
        HEGGWRESULT hWeightResult = NULL;

        if (fdid > 0)
        {
            eggFieldView_slock(hIndexReader->hFieldView, fdid);
            HEGGFIELDNAMEINFO lp_fieldName_info = eggFieldView_get_singlefieldnameinfo_byfid(hIndexReader->hFieldView, fdid);
            hWeightResult = eggFieldWeight_get_withname(hIndexReader->hFieldWeight, lp_fieldName_info->name, &cnt);
            eggFieldView_delete_fieldnameinfo(lp_fieldName_info, 1);

            eggFieldView_unlock(hIndexReader->hFieldView, fdid);        
        }
        
	
        if ((ftype & (EGG_INDEX_INT32 | EGG_INDEX_INT64 | EGG_INDEX_DOUBLE))
            && hWeightResult)
        {
            
            HEGGWRESULT hWeightResultStart = NULL;
            HEGGWRESULT hWeightResultEnd = NULL;
            if (ftype & EGG_INDEX_INT32)
            {

                Uti_sedgesort (hWeightResult, cnt, sizeof(EGGWRESULT), eggWResult_cmpval_int32_desc);
                
                
		
                EGGWRESULT k;
                *(int32_t *)k.val = *(int32_t *)lpStaKey;
                hWeightResultStart = Uti_binary_searchleft(&k, hWeightResult, cnt, sizeof(EGGWRESULT), eggWResult_cmpval_int32_asc);
                *(int32_t *)k.val = *(int32_t *)lpEndKey;
                hWeightResultEnd = Uti_binary_searchright(&k, hWeightResult, cnt, sizeof(EGGWRESULT), eggWResult_cmpval_int32_asc);

            }
            else if (ftype & EGG_INDEX_INT64)
            {
                Uti_sedgesort (hWeightResult, cnt, sizeof(EGGWRESULT), eggWResult_cmpval_int64_desc);
                
                EGGWRESULT k;
                *(int64_t *)k.val = *(int64_t *)lpStaKey;
                hWeightResultStart = Uti_binary_searchleft(&k, hWeightResult, cnt, sizeof(EGGWRESULT), eggWResult_cmpval_int64_asc);
                *(int64_t *)k.val = *(int64_t *)lpEndKey;                
                hWeightResultEnd = Uti_binary_searchright(&k, hWeightResult, cnt, sizeof(EGGWRESULT), eggWResult_cmpval_int64_asc);
            }
            else if (ftype & EGG_INDEX_DOUBLE)
            {
                Uti_sedgesort (hWeightResult, cnt, sizeof(EGGWRESULT), eggWResult_cmpval_double_desc);

                EGGWRESULT k;
                *(double *)k.val = *(double *)lpStaKey;
                hWeightResultStart = Uti_binary_searchleft(&k, hWeightResult, cnt, sizeof(EGGWRESULT), eggWResult_cmpval_double_asc);
                *(double *)k.val = *(double *)lpEndKey;                
                hWeightResultEnd = Uti_binary_searchright(&k, hWeightResult, cnt, sizeof(EGGWRESULT), eggWResult_cmpval_double_asc);
            }

            *lpCntDoc = hWeightResultEnd-hWeightResultStart;
            if (*lpCntDoc)
            {

                *hIdNodes = calloc(*lpCntDoc,
                                   sizeof(EGGIDNODE));
                assert(*hIdNodes);
                index_t idx = 0;
                while (hWeightResultStart != hWeightResultEnd)
                {
                    (*hIdNodes)[idx].id = hWeightResultStart->id;
                    (*hIdNodes)[idx].mask = (size16_t)fdid;
                    *(size16_t*)(*hIdNodes)[idx].pos = (size16_t)1;
                    idx++;
                    hWeightResultStart++;
                }

                
            }
            else
            {
                *hIdNodes = NULL;
            }
        }

        free(hReaderResult->hRangeCache);
        hReaderResult->hRangeCache = hWeightResult;
        hReaderResult->nRangeCache = cnt;
        //free(hWeightResult);
 
        
    }

    pthread_mutex_unlock(&counter_mutex);    
    
    return EGG_TRUE;
}

HEGGINDEXRANGE eggIndexReader_query_idsrange_local(HEGGINDEXREADER hIndexReader_,  fdid_t fid)
{
    EGGINDEXREADERLOCAL *hIndexReader = (EGGINDEXREADERLOCAL *)hIndexReader_;
    
    pthread_mutex_lock(&counter_mutex);
    eggFieldView_slock(hIndexReader->hFieldView, fid);
        
    struct eggIndexInfo * lp_index_info = eggFieldView_get_indexinfo(hIndexReader->hFieldView, fid);
    eggIndexView_load_info(hIndexReader->hIndexView, lp_index_info, hIndexReader->hFieldView);
        
    HEGGINDEXRANGE lp_index_range = eggIndexView_export_ids(hIndexReader->hIndexView);
    
    eggFieldView_unlock(hIndexReader->hFieldView, fid);
    pthread_mutex_unlock(&counter_mutex);
    
    return lp_index_range;
}

HEGGINDEXREADER EGGAPI eggIndexReader_alloc_local()
{
    return malloc(sizeof(EGGINDEXREADERLOCAL));
}

EBOOL EGGAPI eggIndexReader_free_local(HEGGINDEXREADER hIndexReader_)
{
    EGGINDEXREADERLOCAL *hIndexReader = (EGGINDEXREADERLOCAL *)hIndexReader_;
    free(hIndexReader);
    return EGG_TRUE;
}

EBOOL EGGAPI eggIndexReader_set_indexview_local(HEGGINDEXREADER hIndexReader_, HEGGINDEXVIEW hIndexView)
{
    EGGINDEXREADERLOCAL *hIndexReader = (EGGINDEXREADERLOCAL *)hIndexReader_;
    
    hIndexReader->hIndexView = hIndexView;
    return EGG_TRUE;
}

EBOOL EGGAPI eggIndexReader_set_idview_local(HEGGINDEXREADER hIndexReader_, HEGGIDVIEW hIdView)
{
    EGGINDEXREADERLOCAL *hIndexReader = (EGGINDEXREADERLOCAL *)hIndexReader_;
    hIndexReader->hIdView = hIdView;
    return EGG_TRUE;
}

EBOOL EGGAPI eggIndexReader_set_docview_local(HEGGINDEXREADER hIndexReader_, HEGGDOCVIEW hDocView)
{
    EGGINDEXREADERLOCAL *hIndexReader = (EGGINDEXREADERLOCAL *)hIndexReader_;
    hIndexReader->hDocView = hDocView;
    return EGG_TRUE;
    
}

EBOOL EGGAPI eggIndexReader_set_fieldview_local(HEGGINDEXREADER hIndexReader_, HEGGFIELDVIEW hFieldView)
{
    EGGINDEXREADERLOCAL *hIndexReader = (EGGINDEXREADERLOCAL *)hIndexReader_;
    hIndexReader->hFieldView = hFieldView;
    return EGG_TRUE;
    
}

EBOOL EGGAPI eggIndexReader_set_fieldweight_local(HEGGINDEXREADER hIndexReader_, HEGGFIELDWEIGHT hFieldWeight)
{
    EGGINDEXREADERLOCAL *hIndexReader = (EGGINDEXREADERLOCAL *)hIndexReader_;
    hIndexReader->hFieldWeight = hFieldWeight;
    return EGG_TRUE;
    
}

EBOOL EGGAPI eggIndexReader_set_indexcache_local(HEGGINDEXREADER hIndexReader_, HEGGINDEXCACHE hIndexCache)
{
    EGGINDEXREADERLOCAL *hIndexReader = (EGGINDEXREADERLOCAL *)hIndexReader_;
    hIndexReader->refIndexCache = hIndexCache;
    return EGG_TRUE;
    
}

EBOOL EGGAPI eggIndexReader_set_handle_local(HEGGINDEXREADER hIndexReader_, void *hEggHandle_)
{
    EGGINDEXREADERLOCAL *hIndexReader = (EGGINDEXREADERLOCAL *)hIndexReader_;
     
    hIndexReader->hEggHandle = (HEGGHANDLE)hEggHandle_;
    return EGG_TRUE;
    
}

HEGGFIELDVIEW EGGAPI eggIndexReader_get_fieldview_local(HEGGINDEXREADER hIndexReader_)
{
    if (!hIndexReader_)
    {
        return NULL;
    }
    EGGINDEXREADERLOCAL *hIndexReader = (EGGINDEXREADERLOCAL *)hIndexReader_;
    return hIndexReader->hFieldView;
}

HEGGFIELDWEIGHT EGGAPI eggIndexReader_get_fieldweight_local(HEGGINDEXREADER hIndexReader_)
{
    if (!hIndexReader_)
    {
        return NULL;
    }
    EGGINDEXREADERLOCAL *hIndexReader = (EGGINDEXREADERLOCAL *)hIndexReader_;
    return hIndexReader->hFieldWeight;
    
}




HEGGINDEXREADERRESULT eggIndexReaderResult_new()
{
    HEGGINDEXREADERRESULT hReaderResult = calloc(1, sizeof(*hReaderResult));
    assert(hReaderResult);
    return hReaderResult;
}
EBOOL eggIndexReaderResult_delete(HEGGINDEXREADERRESULT hReaderResult)
{
    free(hReaderResult->hIdNodes);
    free(hReaderResult->hRangeCache);
    free(hReaderResult);
    return EGG_TRUE;
}
EBOOL eggIndexReaderResult_pop_idnodes(HEGGINDEXREADERRESULT hReaderResult, HEGGIDNODE *hIdNodes, count_t *lpCntDoc)
{
    *hIdNodes = hReaderResult->hIdNodes;
    hReaderResult->hIdNodes = NULL;    
    *lpCntDoc = hReaderResult->nIdNodes;
    hReaderResult->nIdNodes = 0;
    return EGG_TRUE;
    
}
EBOOL eggIndexReaderResult_pop_rangecache(HEGGINDEXREADERRESULT hReaderResult, HEGGWRESULT *hhWeightResult, count_t *p_nWeightResult)
{
    *hhWeightResult = hReaderResult->hRangeCache;
    hReaderResult->hRangeCache = NULL;
    *p_nWeightResult = hReaderResult->nRangeCache;
    hReaderResult->nRangeCache = 0;
    return EGG_TRUE;
}

