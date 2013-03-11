#include "eggIndexWriterLocal.h"
#include "eggIndexReaderLocal.h"
#include "../eggDirectory.h"
#include "../storage/eggRecoveryLog.h"
#include "../log/eggPrtLog.h"
#include <assert.h>
#include <pthread.h> 
extern pthread_mutex_t counter_mutex;
struct eggIndexWriterLocal
{
    HEGGHANDLE hEggHandle;      /* must be first */
    HEGGINDEXCACHE hIndexCache;
    HEGGINDEXVIEW hIndexView;
    HEGGIDVIEW hIdView;
    HEGGDOCVIEW hDocView;
    HEGGFIELDVIEW hFieldView;
    HEGGFIELDWEIGHT hFieldWeight;    
    
    HEGGRECOVERYHANDLE hRecoveryHandle;
    ActInfo *hActInfo;
    char *analyzerName;
};
typedef struct eggIndexWriterLocal EGGINDEXWRITERLOCAL;


#define EGGINDEXWRITER_IS_INVALID(hWriter)  ((!hWriter) ? EGG_TRUE : EGG_FALSE)

#define EGGINDEXWRITER_ANALYER(hWriter) ( (hWriter)->hAnalyzer )
#define EGGINDEXWRITER_INDEXCACHE(hWriter) ( (hWriter)->hIndexCache )
#define EGGINDEXWRITER_INDEXVIEW(hWriter) ( (hWriter)->hIndexView )
#define EGGINDEXWRITER_IDVIEW(hWriter) ( (hWriter)->hIdView )
#define EGGINDEXWRITER_DOCVIEW(hWriter) ( (hWriter)->hDocView )
#define EGGINDEXWRITER_FIELDVIEW(hWriter) ( (hWriter)->hFieldView )


PRIVATE EBOOL  eggIndexWriter_add_indexId(EGGINDEXWRITERLOCAL *hEggIndexWriter, HEGGINDEXCACHEKEY lp_cache_key, HEGGIDNODE hIdNodes, count_t nIdCnt);

PRIVATE EBOOL  eggIndexWriter_add_indexCache(EGGINDEXWRITERLOCAL *hEggIndexWriter, HEGGFIELD hField, HEGGIDNODE hIdNode);
PRIVATE EBOOL  eggIndexWriter_endAct(EGGINDEXWRITERLOCAL *hEggIndexWriter);

PRIVATE EBOOL  eggIndexWriter_startAct(EGGINDEXWRITERLOCAL *hEggIndexWriter);


PRIVATE EBOOL eggIndexWriter_add_fieldWeight(HEGGINDEXWRITER hEggIndexWriter_, HEGGFIELD hField, did_t nDocId);

PRIVATE EBOOL eggIndexWriter_remove_fieldWeight(HEGGINDEXWRITER hEggIndexWriter_, HEGGFIELD hField, did_t nDocId);



PRIVATE EBOOL  eggIndexWriter_add_indexId_normal(EGGINDEXWRITERLOCAL *hEggIndexWriter, HEGGINDEXCACHEKEY lp_cache_key, HEGGIDNODE hIdNodes, count_t nIdCnt,fdid_t fid);


PRIVATE EBOOL  eggIndexWriter_add_indexId_range(EGGINDEXWRITERLOCAL *hEggIndexWriter, HEGGINDEXCACHEKEY lp_cache_key, HEGGIDNODE hIdNodes, count_t nIdCnt,fdid_t fid);

PRIVATE EBOOL eggIndexWriter_registerField(HEGGINDEXWRITER hEggIndexWriter, HEGGDOCUMENT hDocument);

HEGGINDEXWRITER EGGAPI eggIndexWriter_open_local(void *hEggHandle_, char *analyzerName)
{
    HEGGDIRECTORY hEggDirectory = (HEGGDIRECTORY)hEggHandle_;
    if(POINTER_IS_INVALID(hEggDirectory))
    {
        return EGG_NULL;
    }
        
    EGGINDEXWRITERLOCAL *lp_index_writer = (EGGINDEXWRITERLOCAL*)malloc(sizeof(EGGINDEXWRITERLOCAL));

    lp_index_writer->hEggHandle = eggDirectory_dup(hEggDirectory);
    
    eggDirectory_init(hEggDirectory);

    if (analyzerName)
    {
        lp_index_writer->analyzerName = strdup(analyzerName);
        assert(lp_index_writer->analyzerName);
    }
    else
    {
        lp_index_writer->analyzerName = strdup("");
        assert(lp_index_writer->analyzerName);
    }
    
    lp_index_writer->hIndexCache = eggIndexCache_new();
    
    HEGGIDTABLE hIdTable = eggIdTable_new(eggDirectory_get_file(hEggDirectory, EGG_FDAT_IDT));
    
    lp_index_writer->hIndexView = eggIndexView_new(eggDirectory_get_file(hEggDirectory, EGG_FIDX), EGG_NULL);
    
    lp_index_writer->hIdView = eggIdView_new(eggDirectory_get_file(hEggDirectory, EGG_FID));
    
    lp_index_writer->hDocView = eggDocView_new(eggDirectory_get_file(hEggDirectory, EGG_FDAT), hIdTable);
    
    lp_index_writer->hFieldView = eggFieldView_new(eggDirectory_get_file(hEggDirectory, EGG_FFD));
    
    lp_index_writer->hFieldWeight = eggFieldWeight_new(eggDirectory_get_file(hEggDirectory, EGG_FFW), lp_index_writer->hFieldView);
    
    lp_index_writer->hRecoveryHandle = eggRecoveryLog_init(ViewStream_name(lp_index_writer->hIndexView->hViewStream));
    ((EGGFILE*)((VIEWSTREAM*)lp_index_writer->hIndexView->hViewStream)->hEggFile)->hEggRecoveryHandle = lp_index_writer->hRecoveryHandle;
    ((EGGFILE*)((VIEWSTREAM*)lp_index_writer->hIdView->hViewStream)->hEggFile)->hEggRecoveryHandle = lp_index_writer->hRecoveryHandle;
    ((EGGFILE*)((VIEWSTREAM*)lp_index_writer->hDocView->hViewStream)->hEggFile)->hEggRecoveryHandle = lp_index_writer->hRecoveryHandle;
    ((EGGFILE*)lp_index_writer->hFieldView->hEggFile)->hEggRecoveryHandle = lp_index_writer->hRecoveryHandle;    

    
    return (HEGGINDEXWRITER)lp_index_writer;
}

EBOOL EGGAPI eggIndexWriter_ref_indexcache_local(HEGGINDEXWRITER EGGAPI hEggIndexWriter_, HEGGINDEXCACHE hIndexCache)
{
    EGGINDEXWRITERLOCAL *hEggIndexWriter = (EGGINDEXWRITERLOCAL *)hEggIndexWriter_;
    
    if(EGGINDEXWRITER_IS_INVALID(hEggIndexWriter))
    {
        return EGG_FALSE;
    }
    if(EGGINDEXWRITER_IS_INVALID(hEggIndexWriter->hIndexCache))
    {
       eggIndexCache_delete(hEggIndexWriter->hIndexCache); 
    }
    
    eggIndexCache_ref(&hEggIndexWriter->hIndexCache, hIndexCache);


    return EGG_TRUE;
}

EBOOL EGGAPI eggIndexWriter_set_analyzer_local(HEGGINDEXWRITER EGGAPI hEggIndexWriter_, char *analyzerName)
{
    EGGINDEXWRITERLOCAL *hEggIndexWriter = (EGGINDEXWRITERLOCAL *)hEggIndexWriter_;
    if(EGGINDEXWRITER_IS_INVALID(hEggIndexWriter))
    {
        return EGG_FALSE;
    }
    free(hEggIndexWriter->analyzerName);
    if (analyzerName)
    {
        hEggIndexWriter->analyzerName = strdup(analyzerName);
        assert(hEggIndexWriter->analyzerName);
    }
    else
    {
        hEggIndexWriter->analyzerName = strdup("");
        assert(hEggIndexWriter->analyzerName);
    }
    
    return EGG_TRUE;
}

EBOOL EGGAPI eggIndexWriter_close_local(HEGGINDEXWRITER hEggIndexWriter_)
{
    EGGINDEXWRITERLOCAL *hEggIndexWriter = (EGGINDEXWRITERLOCAL *)hEggIndexWriter_;
    if(EGGINDEXWRITER_IS_INVALID(hEggIndexWriter))
    {
        return EGG_FALSE;
    }
    eggIndexWriter_optimize_local(hEggIndexWriter);
    
    eggRecoveryLog_destroy(hEggIndexWriter->hRecoveryHandle);

    eggDirectory_delete(hEggIndexWriter->hEggHandle);
    
    eggIndexCache_delete(hEggIndexWriter->hIndexCache);
    
    eggIndexView_delete(hEggIndexWriter->hIndexView);
    eggIdView_delete(hEggIndexWriter->hIdView);
    eggDocView_delete(hEggIndexWriter->hDocView);
    eggFieldView_delete(hEggIndexWriter->hFieldView);
    eggFieldWeight_delete(hEggIndexWriter->hFieldWeight);
    free(hEggIndexWriter->analyzerName);
    free(hEggIndexWriter);
    hEggIndexWriter = EGG_NULL;
    
    
    return EGG_TRUE;
}


EBOOL EGGAPI eggIndexWriter_add_document_local(HEGGINDEXWRITER hEggIndexWriter_, HEGGDOCUMENT hEggDocument)
{
    EGGINDEXWRITERLOCAL *hEggIndexWriter = (EGGINDEXWRITERLOCAL *)hEggIndexWriter_;
    if(EGGINDEXWRITER_IS_INVALID(hEggIndexWriter))
    {
        return EGG_FALSE;
    }
    if(!hEggDocument)
    {
        return EGG_FALSE;
    }
    if(!eggIndexWriter_registerField(hEggIndexWriter, hEggDocument))
    {
        return EGG_FALSE;
    }
    
    HEGGDOCNODE lp_doc_node  = eggDocument_serialization(hEggDocument);
     pthread_mutex_lock(&counter_mutex);

    eggIndexWriter_startAct(hEggIndexWriter);
    
    did_t n_doc_id = n_doc_id = eggDocView_add(hEggIndexWriter->hDocView, lp_doc_node);
    
    eggIndexWriter_endAct(hEggIndexWriter);
    pthread_mutex_unlock(&counter_mutex);
    
    free(lp_doc_node);

    HEGGFIELD lp_field_iter = eggDocument_get_field(hEggDocument, EGG_NULL);

    HEGGIDNODE lp_id_node = (HEGGIDNODE)malloc(sizeof(EGGIDNODE));

    lp_id_node->id = n_doc_id;
    lp_id_node->weight = eggDocument_get_weight(hEggDocument);
    lp_id_node->flag = EGG_IDNODE_VALID;
    index_t i=0;
    while (lp_field_iter)
    {
        if(EGG_FALSE == eggIndexWriter_add_indexCache(hEggIndexWriter, lp_field_iter, lp_id_node))
        {
            return EGG_FALSE;
        }
        
        eggIndexWriter_add_fieldWeight(hEggIndexWriter, lp_field_iter, n_doc_id);
        
        lp_field_iter = eggField_get_next(lp_field_iter);

    }
    free(lp_id_node);

    
    return EGG_TRUE;;
}
EBOOL EGGAPI eggIndexWriter_modify_document_local(HEGGINDEXWRITER hEggIndexWriter_, EGGDID dId, HEGGDOCUMENT hNewDocument)
{
    EGGINDEXWRITERLOCAL *hEggIndexWriter = (EGGINDEXWRITERLOCAL *)hEggIndexWriter_;
    
    if(EGGINDEXWRITER_IS_INVALID(hEggIndexWriter))
    {
        return EGG_FALSE;
    }
    if(!eggIndexWriter_registerField(hEggIndexWriter, hNewDocument))
    {
        return EGG_FALSE;
    }

    did_t id = EGGDID_DOCID(&dId);
    HEGGDOCNODE lp_doc_node = EGG_NULL;

    eggDocView_query(hEggIndexWriter->hDocView, id, &lp_doc_node);
    
    if(!lp_doc_node)
    {
        return EGG_FALSE;
    }

    HEGGDOCUMENT hOrgDocument = eggDocument_unserialization(lp_doc_node);

    if(!hOrgDocument)
    {
        return EGG_FALSE;
    }

//    free(lp_doc_node);
    //
 //
    HEGGFIELD lp_field_iter = eggDocument_get_field(hNewDocument, EGG_NULL);
    HEGGIDNODE lp_id_node = (HEGGIDNODE)malloc(sizeof(EGGIDNODE));

    lp_id_node->id = id;
    lp_id_node->weight = eggDocument_get_weight(hOrgDocument);
    

    /*
      该循环作用
        1 把新field或者修改的field的key加入id列表，并删除修改的field对应的老field的key
        2 顺带删除了document里老的field, 把新field或者修改的field加入doument中
     */
    while (lp_field_iter)
    {
        HEGGFIELD lp_field_tmp = EGG_NULL;
        if( lp_field_tmp = eggDocument_remove_field_byname(hOrgDocument, eggField_get_name(lp_field_iter)) )
        {
            lp_id_node->flag = EGG_IDNODE_INVALID;
            if(EGG_FALSE == eggIndexWriter_add_indexCache(hEggIndexWriter, lp_field_tmp, lp_id_node))
            {
                free(lp_id_node);
                return EGG_FALSE;
            }

            eggIndexWriter_remove_fieldWeight(hEggIndexWriter, lp_field_tmp, id);
            
            eggField_delete(lp_field_tmp);
        }
        
        lp_id_node->flag = EGG_IDNODE_VALID;
        if(EGG_FALSE == eggIndexWriter_add_indexCache(hEggIndexWriter, lp_field_iter, lp_id_node))
        {
            free(lp_id_node);
            return EGG_FALSE;
        }

        eggIndexWriter_add_fieldWeight(hEggIndexWriter, lp_field_iter, id);
        
        HEGGFIELD lp_field_next = eggField_get_next(lp_field_iter);
        
        eggDocument_add(hOrgDocument, lp_field_iter);
        
        lp_field_iter = lp_field_next;

    }
    free(lp_id_node);
    free(hNewDocument);
    lp_doc_node  = eggDocument_serialization(hOrgDocument);

    eggDocView_update(hEggIndexWriter->hDocView, id, lp_doc_node);

    eggDocument_delete(hOrgDocument);
    free(lp_doc_node);
    return EGG_TRUE;
}


EBOOL EGGAPI eggIndexWriter_delete_document_local(HEGGINDEXWRITER hEggIndexWriter_, EGGDID dId )
{
    EGGINDEXWRITERLOCAL *hEggIndexWriter = (EGGINDEXWRITERLOCAL *)hEggIndexWriter_;
    if(EGGINDEXWRITER_IS_INVALID(hEggIndexWriter))
    {
        return EGG_FALSE;
    }
    HEGGDOCNODE lp_doc_node = EGG_NULL;
    did_t id = EGGDID_DOCID(&dId);
    if(!eggDocView_query(hEggIndexWriter->hDocView, id, &lp_doc_node))
    {
        return EGG_FALSE;
    }
    

    HEGGDOCUMENT hEggDocument = eggDocument_unserialization(lp_doc_node);

    if(!hEggDocument)
    {
        return EGG_FALSE;
    }

//    free(lp_doc_node);
    

    did_t n_doc_id = id;


    HEGGFIELD lp_field_iter = eggDocument_get_field(hEggDocument, EGG_NULL);
    HEGGIDNODE lp_id_node = (HEGGIDNODE)malloc(sizeof(EGGIDNODE));

    lp_id_node->id = n_doc_id;
    lp_id_node->weight = eggDocument_get_weight(hEggDocument);
    lp_id_node->flag = EGG_IDNODE_INVALID;
    
    while (lp_field_iter)
    {
        if(EGG_FALSE == eggIndexWriter_add_indexCache(hEggIndexWriter, lp_field_iter, lp_id_node))
        {
            return EGG_FALSE;
        }
        
        eggIndexWriter_remove_fieldWeight(hEggIndexWriter, lp_field_iter, n_doc_id);

        
        lp_field_iter = eggField_get_next(lp_field_iter);

    }
    
    eggDocView_remove(hEggIndexWriter->hDocView, id);
    
    if(!lp_doc_node)
    {
        return EGG_FALSE;
    }

    eggDocument_delete(hEggDocument);
    free(lp_id_node);
    return n_doc_id;
}



EBOOL EGGAPI eggIndexWriter_reindex_document_local(HEGGINDEXWRITER hEggIndexWriter_, HEGGDOCUMENT hEggDocument, EGGDID dId)
{
    EGGINDEXWRITERLOCAL *hEggIndexWriter = (EGGINDEXWRITERLOCAL *)hEggIndexWriter_;
    if(EGGINDEXWRITER_IS_INVALID(hEggIndexWriter))
    {
        return EGG_FALSE;
    }
    if(!hEggDocument)
    {
        return EGG_FALSE;
    }

//    if(EGG_FALSE == eggIndexWriter_add_indexCache(hEggIndexWriter, hEggDocument, dId))
    {
        return EGG_FALSE;
    }

}

EBOOL EGGAPI eggIndexWriter_incrementmodify_document_local(HEGGINDEXWRITER hEggIndexWriter_, EGGDID dId, HEGGDOCUMENT hNewDocument)
{
    EGGINDEXWRITERLOCAL *hEggIndexWriter = (EGGINDEXWRITERLOCAL *)hEggIndexWriter_;
    
    if(EGGINDEXWRITER_IS_INVALID(hEggIndexWriter))
    {
        return EGG_FALSE;
    }
    
    HEGGDOCNODE lp_doc_node = EGG_NULL;
    did_t id = EGGDID_DOCID(&dId);
    eggDocView_wrlock_id(hEggIndexWriter->hDocView, id);
    
    eggDocView_query(hEggIndexWriter->hDocView, id, &lp_doc_node);
    
    if(!lp_doc_node)
    {
        eggDocView_unlock_id(hEggIndexWriter->hDocView, id);
        return EGG_FALSE;
    }

    HEGGDOCUMENT hOrgDocument = eggDocument_unserialization(lp_doc_node);

    if(!hOrgDocument)
    {
        eggDocView_unlock_id(hEggIndexWriter->hDocView, id);

        return EGG_FALSE;
    }

//    eggDocument_set_fieldName(eggDocument_get_hTable(hOrgDocument),hOrgDocument);
    
//    free(lp_doc_node);
    
    
    HEGGFIELD lp_field_iter = eggDocument_get_field(hNewDocument, EGG_NULL);
    HEGGIDNODE lp_id_node = (HEGGIDNODE)malloc(sizeof(EGGIDNODE));

    lp_id_node->id = id;
    lp_id_node->weight = eggDocument_get_weight(hOrgDocument);
    

    while (lp_field_iter)
    {
        HEGGFIELD lp_field_tmp = EGG_NULL;
        
        if((lp_field_tmp = eggDocument_get_field(hOrgDocument, eggField_get_name(lp_field_iter))) &&
           eggField_get_type(lp_field_iter) == eggField_get_type(lp_field_tmp))
        {
            if (eggField_get_type(lp_field_tmp) & EGG_INDEX_STRING)
            {
                /* 字符串追加,不修改原数据,只将新的内容进行索引 */
                int n_increment_sz = 0;
                char* lp_increment = (char*)eggField_get_value(lp_field_iter, &n_increment_sz);
                eggField_append_value(lp_field_tmp, lp_increment, n_increment_sz);
                
                lp_id_node->flag = EGG_IDNODE_VALID;
                /* 只索引新内容 */
                if(EGG_FALSE == eggIndexWriter_add_indexCache(hEggIndexWriter, lp_field_iter, lp_id_node))
                {
                    eggDocView_unlock_id(hEggIndexWriter->hDocView, id);
                    
                    free(lp_id_node);
                    return EGG_FALSE;
                }                
            }
            else
            {
                lp_id_node->flag = EGG_IDNODE_INVALID;
                if(EGG_FALSE == eggIndexWriter_add_indexCache(hEggIndexWriter, lp_field_tmp, lp_id_node))
                {
                    eggDocView_unlock_id(hEggIndexWriter->hDocView, id);
                    free(lp_id_node);

                    return EGG_FALSE;
                }
                eggIndexWriter_remove_fieldWeight(hEggIndexWriter, lp_field_tmp, id);
            
                if(eggField_get_type(lp_field_tmp) & EGG_INDEX_INT32)
                {
                    int* lp_org = (int*)eggField_get_value(lp_field_tmp, EGG_NULL);
                    int* lp_increment = (int*)eggField_get_value(lp_field_iter, EGG_NULL);
                    *lp_org += *lp_increment;
                    

                }
                else if(eggField_get_type(lp_field_tmp) & EGG_INDEX_INT64)
                {
                    long long* lp_org = (long long*)eggField_get_value(lp_field_tmp, EGG_NULL);
                    long long* lp_increment = (long long*)eggField_get_value(lp_field_iter, EGG_NULL);
                    *lp_org += *lp_increment;
                
                }
                else if(eggField_get_type(lp_field_tmp) & EGG_INDEX_DOUBLE)
                {
                    double* lp_org = (double*)eggField_get_value(lp_field_tmp, EGG_NULL);
                    double* lp_increment = (double*)eggField_get_value(lp_field_iter, EGG_NULL);
                    *lp_org += *lp_increment;
                }
                else
                {
                    ;
                }
            
                lp_id_node->flag = EGG_IDNODE_VALID;
                if(EGG_FALSE == eggIndexWriter_add_indexCache(hEggIndexWriter, lp_field_tmp, lp_id_node))
                {
                    eggDocView_unlock_id(hEggIndexWriter->hDocView, id);
                    free(lp_id_node);

                    return EGG_FALSE;
                }
                
                eggIndexWriter_add_fieldWeight(hEggIndexWriter, lp_field_tmp, id);
            
            }
        }
        
        
        lp_field_iter = eggField_get_next(lp_field_iter);

    }
    
    eggDocument_delete(hNewDocument);
    
    lp_doc_node  = eggDocument_serialization(hOrgDocument);

    eggDocView_update(hEggIndexWriter->hDocView, id, lp_doc_node);
    
    eggDocView_unlock_id(hEggIndexWriter->hDocView, id);

    eggDocument_delete(hOrgDocument);
    free(lp_doc_node);
    free(lp_id_node);
    return EGG_TRUE;
}

EBOOL EGGAPI eggIndexWriter_add_field_local(HEGGINDEXWRITER hEggIndexWriter_, char *fieldName, type_t fieldType, ... /* char *analyzerName */)
{
    EBOOL retv;
    EGGINDEXWRITERLOCAL *hEggIndexWriter = (EGGINDEXWRITERLOCAL *)hEggIndexWriter_;
    if (fieldType & EGG_OTHER_ANALYZED)
    {
        char *analyzerName = NULL;
        
        char *p = NULL;
        va_list ap_arg;
        va_start(ap_arg, fieldType);
        p = va_arg(ap_arg, char*);
        if (p && p[0])
        {
            analyzerName = p;
        }

        retv = eggFieldView_register(hEggIndexWriter->hFieldView, fieldName, fieldType, analyzerName);
        va_end(ap_arg);
    }
    else
    {
        retv = eggFieldView_register(hEggIndexWriter->hFieldView, fieldName, fieldType);
    }
    return retv;
}
EBOOL EGGAPI eggIndexWriter_modify_field_local(HEGGINDEXWRITER hEggIndexWriter_, char *oldFieldName, char *fieldName, type_t fieldType, ... /* char *analyzerName */)
{
    EBOOL retv;
    EGGINDEXWRITERLOCAL *hEggIndexWriter = (EGGINDEXWRITERLOCAL *)hEggIndexWriter_;
    
    retv = eggFieldView_unregister(hEggIndexWriter->hFieldView, oldFieldName);
    if (retv == EGG_FALSE)
    {
        return retv;
    }
    
    if (fieldType & EGG_OTHER_ANALYZED)
    {
        char *analyzerName = NULL;
        
        char *p = NULL;
        va_list ap_arg;
        va_start(ap_arg, fieldType);
        p = va_arg(ap_arg, char*);
        if (p && p[0])
        {
            analyzerName = p;
        }
        retv = eggFieldView_register(hEggIndexWriter->hFieldView, fieldName, fieldType, analyzerName);
        va_end(ap_arg);
    }
    else
    {
        retv = eggFieldView_register(hEggIndexWriter->hFieldView, fieldName, fieldType);
    }
    return retv;
    
}
EBOOL EGGAPI eggIndexWriter_delete_field_local(HEGGINDEXWRITER hEggIndexWriter_, char *fieldName)
{
    EBOOL retv;
    EGGINDEXWRITERLOCAL *hEggIndexWriter = (EGGINDEXWRITERLOCAL *)hEggIndexWriter_;
    retv = eggFieldView_unregister(hEggIndexWriter->hFieldView, fieldName);
    
    return retv;
}

EBOOL EGGAPI eggIndexWriter_optimize_local(HEGGINDEXWRITER hEggIndexWriter_)
{
    EGGINDEXWRITERLOCAL *hEggIndexWriter = (EGGINDEXWRITERLOCAL *)hEggIndexWriter_; 
    char* lp_egg_path = eggDirectory_get_name((HEGGDIRECTORY)(hEggIndexWriter->hEggHandle));
    if(EGGINDEXWRITER_IS_INVALID(hEggIndexWriter))
    {
        return EGG_FALSE;
    }
    
    HEGGINDEXCACHE lp_index_cache = hEggIndexWriter->hIndexCache;

    HEGGINDEXCACHEITER lp_cache_iter = eggIndexCacheIter_new(lp_index_cache);
    if (lp_index_cache == EGG_NULL)
    {
        
        return EGG_FALSE;
    }

    HEGGINDEXCACHEKEY lp_cache_key = EGG_NULL;
    HEGGINDEXCACHEVAL lp_cache_val = EGG_NULL;

    eggRecoveryLog_makeclean_checkpoint(hEggIndexWriter->hRecoveryHandle);

    count_t count = 0;

    while (eggIndexCacheIter_next(lp_cache_iter, &lp_cache_key, &lp_cache_val))
    {
        // add to index

        pthread_mutex_lock(&counter_mutex);
        if(Uti_vector_count(lp_cache_val))
            eggIndexWriter_add_indexId(hEggIndexWriter, lp_cache_key,Uti_vector_data(lp_cache_val), Uti_vector_count(lp_cache_val));
        
        pthread_mutex_unlock(&counter_mutex);

        //  printf(" eggIndexWriter_add_indexId time : %f\n", (tv_end.tv_sec - tv_start.tv_sec) + (double)(tv_end.tv_usec - tv_start.tv_usec)/1000000 );
        //root node update
        
        count++;

        if (count % 5000 == 0)
        {
            eggRecoveryLog_makeclean_checkpoint(hEggIndexWriter->hRecoveryHandle);
        }
//
    }
    
    eggRecoveryLog_makeclean_checkpoint(hEggIndexWriter->hRecoveryHandle);

    
    
    eggIndexCacheIter_delete(lp_cache_iter);
    
    eggDocView_update_info(hEggIndexWriter->hDocView);
    //eggIndexCache_delete(hEggIndexWriter->hIndexCache);
    //hEggIndexWriter->hIndexCache = eggIndexCache_new();
    eggIndexCache_reinit(hEggIndexWriter->hIndexCache);


    return EGG_TRUE;
}


PRIVATE EBOOL  eggIndexWriter_add_indexId(EGGINDEXWRITERLOCAL *hEggIndexWriter, HEGGINDEXCACHEKEY lp_cache_key, HEGGIDNODE hIdNodes, count_t nIdCnt)
{
    if (EGGINDEXWRITER_IS_INVALID(hEggIndexWriter))
    {
        return EGG_FALSE;
    }
    /////////////////////////////////
    fdid_t fid = (fdid_t)lp_cache_key->mask;
    eggFieldView_xlock(hEggIndexWriter->hFieldView, fid);
    
    if(lp_cache_key->type & EGG_RANGE_INDEX)
    {
        eggIndexWriter_add_indexId_range(hEggIndexWriter, lp_cache_key, hIdNodes, nIdCnt, fid);
    }
    else
    {
        eggIndexWriter_add_indexId_normal(hEggIndexWriter, lp_cache_key, hIdNodes, nIdCnt, fid);
    }
    
    eggFieldView_unlock(hEggIndexWriter->hFieldView, fid);

    return EGG_TRUE;
}




PRIVATE EBOOL  eggIndexWriter_add_indexId_normal(EGGINDEXWRITERLOCAL *hEggIndexWriter, HEGGINDEXCACHEKEY lp_cache_key, HEGGIDNODE hIdNodes, count_t nIdCnt, fdid_t fid)
{
    if (EGGINDEXWRITER_IS_INVALID(hEggIndexWriter))
    {
        return EGG_FALSE;
    }
    /////////////////////////////////
    eggIndexWriter_startAct(hEggIndexWriter);

    HEGGINDEXINFO lp_index_info = eggFieldView_get_indexinfo(hEggIndexWriter->hFieldView, fid);
    offset64_t n_root_off = lp_index_info->rootOff;
    
    eggIndexView_load_info(hEggIndexWriter->hIndexView, lp_index_info, hEggIndexWriter->hFieldView);
    /////////////////////////////////
    /////////////////////////////////
    
    offset64_t n_nd_pos = 0;
    index_t n_rd_idx = 0;
    char* lpKey = (echar*)(lp_cache_key + 1) ;
    size16_t kSz = lp_cache_key->size - sizeof(EGGINDEXCACHEKEY) - 1;
    HEGGINDEXRECORD lp_index_record = eggIndexView_locate(hEggIndexWriter->hIndexView, lpKey, kSz, 0, 0, &n_nd_pos, &n_rd_idx);
    offset64_t n_nodelist_off = 0;
    
    if(lp_index_record)
    {
        n_nodelist_off = *(offset64_t*)EGGINDEXRECORD_VAL(lp_index_record);
        
        if(n_nodelist_off < 65500)
        {
            return EGG_FALSE;
        }
         
        eggIdView_load(hEggIndexWriter->hIdView, n_nodelist_off);
        eggIndexRecord_delete(lp_index_record);

        eggIdView_add(hEggIndexWriter->hIdView, hIdNodes, nIdCnt);

    
        
    }
    else
    {
        size16_t n_info_sz = sizeof(EGGLISTINF);
        if(EGGINDEXVIEW_RECORD_OVERFLOW(hEggIndexWriter->hIndexView, kSz, sizeof(offset64_t)))
        {
            n_info_sz += kSz;
        }
        
        HEGGLISTINF lp_list_info = (HEGGLISTINF)malloc(n_info_sz);
        memset(lp_list_info, 0, n_info_sz);
        lp_list_info->aSz = n_info_sz;
        lp_list_info->nodeSz = sizeof(EGGIDNODE);
                      if(EGGINDEXVIEW_RECORD_OVERFLOW(hEggIndexWriter->hIndexView, kSz, sizeof(offset64_t)))
        {
            memcpy(lp_list_info + 1, lpKey, kSz);
        }

        
        eggIdView_reg(hEggIndexWriter->hIdView, lp_list_info);
        
        n_nodelist_off = hEggIndexWriter->hIdView->hEggListView->hInfo->ownOff;
        
        eggIndexView_insert_pos(hEggIndexWriter->hIndexView, lpKey, kSz,
                                &(n_nodelist_off), sizeof(offset64_t),
                                n_nd_pos, n_rd_idx);
        
        if( n_root_off != lp_index_info->rootOff)
            eggFieldView_release_indexinfo(hEggIndexWriter->hFieldView, fid, lp_index_info);
        
        free(lp_list_info);
        eggIdView_add(hEggIndexWriter->hIdView, hIdNodes, nIdCnt);
    

    }
    eggIndexWriter_endAct(hEggIndexWriter);

    return EGG_TRUE;
}




PRIVATE EBOOL  eggIndexWriter_add_indexId_range(EGGINDEXWRITERLOCAL *hEggIndexWriter, HEGGINDEXCACHEKEY lp_cache_key, HEGGIDNODE hIdNodes, count_t nIdCnt, fdid_t fid)
{
    if (EGGINDEXWRITER_IS_INVALID(hEggIndexWriter))
    {
        return EGG_FALSE;
    }
    /////////////////////////////////
    eggIndexWriter_startAct(hEggIndexWriter);
    
    HEGGINDEXINFO lp_index_info = eggFieldView_get_indexinfo(hEggIndexWriter->hFieldView, fid);
    offset64_t n_root_off = lp_index_info->rootOff;
    
    eggIndexView_load_info(hEggIndexWriter->hIndexView, lp_index_info, hEggIndexWriter->hFieldView);
    /////////////////////////////////
    /////////////////////////////////
    index_t n_idx = 0;
    while(n_idx < nIdCnt)
    {
        offset64_t n_nd_pos = 0;
        index_t n_rd_idx = 0;
        char* lpKey = (echar*)(lp_cache_key + 1) ;
        size16_t kSz = lp_cache_key->size - sizeof(EGGINDEXCACHEKEY) - 1;
    
        struct eggRecordDocId docId = {0};
        docId.did = hIdNodes[n_idx].id;
        docId.flag = (flag_t)hIdNodes[n_idx].flag;
        //printf("key [%s] id %llu\n", lpKey, docId.did);
        eggIndexView_insert(hEggIndexWriter->hIndexView, lpKey, kSz, (void*)&docId, sizeof(docId));

        n_idx++;
    }
    
    if( n_root_off != lp_index_info->rootOff)
        eggFieldView_release_indexinfo(hEggIndexWriter->hFieldView, fid, lp_index_info);
    
    eggIndexWriter_endAct(hEggIndexWriter);

    return EGG_TRUE;
}







PRIVATE EBOOL  eggIndexWriter_add_indexCache(EGGINDEXWRITERLOCAL *hEggIndexWriter, HEGGFIELD hField, HEGGIDNODE hIdNode)
{
    if (EGGINDEXWRITER_IS_INVALID(hEggIndexWriter))
    {
        return EGG_FALSE;
    }
    
    HEGGINDEXCACHE lp_index_cache = hEggIndexWriter->hIndexCache;
    HEGGFIELDVIEW lp_field_view = hEggIndexWriter->hFieldView;
    HEGGFIELD lp_field_iter = hField;
    HEGGIDNODE lp_id_node = hIdNode;

    HEGGANALYZER hAnalyzer = EGG_NULL;
    
    
    type_t field_type = eggField_get_type(lp_field_iter);
    if (field_type & EGG_NOT_INDEX)
    {
        lp_field_iter = eggField_get_next(lp_field_iter);
        return EGG_TRUE;
    }
//    return 1;
     /* if(!hAnalyzer) */
    hAnalyzer = eggAnalyzer_get(eggField_get_analyzername(lp_field_iter));
    
        
    size32_t n_field_sz = 0;
    echar* lp_field_val = eggField_get_value(lp_field_iter, &n_field_sz);
    echar* lp_field_name = eggField_get_name(lp_field_iter);
    echar* lp_field_dict = eggField_get_dictname(lp_field_iter);
    fdid_t fid = (fdid_t)eggField_get_mask(hField);
    
    if(!fid)
    {
        pthread_mutex_lock(&counter_mutex);
        eggIndexWriter_startAct(hEggIndexWriter);
        if (field_type & EGG_OTHER_ANALYZED)
        {
            fid = eggFieldView_register(lp_field_view, lp_field_name, field_type,
                                        eggField_get_analyzername(lp_field_iter));
        }
        else
        {
            fid = eggFieldView_register(lp_field_view, lp_field_name, field_type);
        }
        eggIndexWriter_endAct(hEggIndexWriter);
        pthread_mutex_unlock(&counter_mutex);
    }
    
    if(fid == 0)
    {
        return EGG_FALSE;
    }
    
    lp_id_node->mask = (size16_t)fid;
        
   
    if (lp_field_val == NULL || n_field_sz == 0)
    {
        ;
    }
    else if (hAnalyzer)
//    if (g_hAnalyzer)
    {
        ImTokenList* lp_key_tokens = EGG_NULL;
            
        count_t n_tokens_cnt = 0;
        index_t n_tokens_idx = 0;
//        printf("hAnalyzer %p\n", hAnalyzer);
        
//        pthread_mutex_lock(&counter_mutex);
        char* p_tokenize_buf = strndup(lp_field_val, n_field_sz);
        hAnalyzer->fnTokenize(hAnalyzer->pHandle, p_tokenize_buf, &lp_key_tokens, lp_field_dict);
        free(p_tokenize_buf);
//        printf("lp_field_val [%s]\n", lp_field_val);
        //ImCwsLexAnalyzer_tokenize(g_hAnalyzer, lp_field_val, &lp_key_tokens);
        //return 1;
        //      pthread_mutex_unlock(&counter_mutex);
        ImTokenList_get_size(lp_key_tokens, &n_tokens_cnt);
        ImToken* lp_token_iter = EGG_NULL;
        lp_id_node->keyCnt = ImTokenList_get_wordNum(lp_key_tokens);
            
	    GList* lp_list_key = lp_key_tokens->m_pTokens;
 
        while(lp_list_key)
        {

            lp_token_iter = lp_list_key->data;
            echar* lp_key = EGG_NULL;
            size16_t* lp_key_pos = EGG_NULL;
            size16_t n_key_len = ImToken_get_dataLength(lp_token_iter);
            size16_t n_pos_len = 0;
              
            ImToken_get_data(lp_token_iter, &lp_key);
            ImToken_get_offsets(lp_token_iter, &lp_key_pos);
            n_pos_len = sizeof(size16_t) * ImToken_get_offsetNumber(lp_token_iter);
	    //	    printf("[%s]\n", lp_key);
            HEGGINDEXCACHEKEY lp_cache_key = eggIndexCacheKey_new(field_type, lp_id_node->mask,
                                                                  lp_key, n_key_len);
            memset(EGGIDNODE_POS(lp_id_node), 0, EGG_POS_SPACE);
            memcpy(EGGIDNODE_POS(lp_id_node), lp_key_pos, n_pos_len);
	      
            eggIndexCache_insert(lp_index_cache, lp_cache_key, lp_id_node);
            eggIndexCacheKey_delete(lp_cache_key);
            free(lp_key);
            lp_list_key = g_list_next(lp_list_key);
            
            
            }
//        printf("lp_key_tokens %p \n", lp_key_tokens);
        ImTokenList_delete(lp_key_tokens);
        

    }
    else
    {
        HEGGINDEXCACHEKEY lp_cache_key = eggIndexCacheKey_new(field_type, lp_id_node->mask,
                                                              lp_field_val, n_field_sz);
        size16_t* lp_pos = EGGIDNODE_POS(lp_id_node);
        *lp_pos = 1;
        lp_pos[1] = 0;
        lp_id_node->keyCnt = 1;
        eggIndexCache_insert(lp_index_cache, lp_cache_key, lp_id_node);
        eggIndexCacheKey_delete(lp_cache_key);
    }
        
    return EGG_TRUE;
}


HEGGINDEXREADER  EGGAPI eggIndexWriter_init_reader_local(HEGGINDEXWRITER hIndexWriter_)
{
    EGGINDEXWRITERLOCAL *hIndexWriter = (EGGINDEXWRITERLOCAL *)hIndexWriter_;
    HEGGINDEXREADER hIndexReader = eggIndexReader_alloc_local();
    if (EGGINDEXWRITER_IS_INVALID(hIndexWriter))
    {
        return EGG_FALSE;
    }

    eggIndexReader_set_handle_local(hIndexReader, hIndexWriter->hEggHandle);

    eggIndexReader_set_indexview_local(hIndexReader, hIndexWriter->hIndexView);
    eggIndexReader_set_idview_local(hIndexReader, hIndexWriter->hIdView);

    eggIndexReader_set_docview_local(hIndexReader, hIndexWriter->hDocView);
    
    eggIndexReader_set_fieldview_local(hIndexReader, hIndexWriter->hFieldView);

    eggIndexReader_set_fieldweight_local(hIndexReader, hIndexWriter->hFieldWeight);
    
    eggIndexReader_set_indexcache_local(hIndexReader, hIndexWriter->hIndexCache);

    return hIndexReader;
}


PRIVATE EBOOL  eggIndexWriter_startAct(EGGINDEXWRITERLOCAL *hEggIndexWriter)
{
    if (EGGINDEXWRITER_IS_INVALID(hEggIndexWriter))
    {
        return EGG_FALSE;
    }

    ActInfo *hActInfo;
    hActInfo = eggRecoveryLog_beginact(hEggIndexWriter->hRecoveryHandle);
    hEggIndexWriter->hActInfo = hActInfo;
    eggFieldView_set_actinfo(hEggIndexWriter->hFieldView, hActInfo);
    eggFieldWeight_set_actinfo(hEggIndexWriter->hFieldWeight, hActInfo);    
    eggIndexView_set_actinfo(hEggIndexWriter->hIndexView, hActInfo);
    eggIdView_set_actinfo(hEggIndexWriter->hIdView, hActInfo);
    eggDocView_set_actinfo(hEggIndexWriter->hDocView, hActInfo);
    return EGG_TRUE;
}

PRIVATE EBOOL  eggIndexWriter_endAct(EGGINDEXWRITERLOCAL *hEggIndexWriter)
{
    if (EGGINDEXWRITER_IS_INVALID(hEggIndexWriter))
    {
        return EGG_FALSE;
    }

    eggRecoveryLog_endact(hEggIndexWriter->hRecoveryHandle,
                          hEggIndexWriter->hActInfo);
    hEggIndexWriter->hActInfo = NULL;
    eggFieldView_clean_actinfo(hEggIndexWriter->hFieldView, NULL);
    eggFieldWeight_clean_actinfo(hEggIndexWriter->hFieldWeight, NULL);
    eggIndexView_clean_actinfo(hEggIndexWriter->hIndexView, NULL);
    eggIdView_clean_actinfo(hEggIndexWriter->hIdView, NULL);
    eggDocView_clean_actinfo(hEggIndexWriter->hDocView, NULL);
    
    return EGG_TRUE;
}




PRIVATE EBOOL eggIndexWriter_add_fieldWeight(HEGGINDEXWRITER hEggIndexWriter_, HEGGFIELD hField, did_t nDocId)
{                                                    
    EGGINDEXWRITERLOCAL *hEggIndexWriter = (EGGINDEXWRITERLOCAL *)hEggIndexWriter_;

    type_t type;                                                    

    type = eggField_get_type(hField);
    if ((type & (EGG_INDEX_INT32 | EGG_INDEX_INT64 | EGG_INDEX_DOUBLE)) && !(type & EGG_RANGE_INDEX) )
    {
            
        pthread_mutex_lock(&counter_mutex);                

        fdid_t fid;
        eggFieldView_find(hEggIndexWriter->hFieldView, 
                          eggField_get_name(hField),
                          &fid);
        if (fid > 0)
        {
            eggFieldView_xlock(hEggIndexWriter->hFieldView, fid);

            eggIndexWriter_startAct(hEggIndexWriter);            
    
            eggFieldWeight_add(hEggIndexWriter->hFieldWeight, hField, nDocId);

            eggFieldView_unlock(hEggIndexWriter->hFieldView, fid);
            
            eggIndexWriter_endAct(hEggIndexWriter);
            
            pthread_mutex_unlock(&counter_mutex);
            
            return EGG_TRUE;

        }
            
        pthread_mutex_unlock(&counter_mutex);
            
    }

    return EGG_FALSE;        
}
//////////////////////

PRIVATE EBOOL eggIndexWriter_remove_fieldWeight(HEGGINDEXWRITER hEggIndexWriter_, HEGGFIELD hField, did_t nDocId)
{                                                    
    EGGINDEXWRITERLOCAL *hEggIndexWriter = (EGGINDEXWRITERLOCAL *)hEggIndexWriter_;

    type_t type;
    type = eggField_get_type(hField);
    if (type & (EGG_INDEX_INT32 | EGG_INDEX_INT64 | EGG_INDEX_DOUBLE) && !(type & EGG_RANGE_INDEX) )
    {
            
        pthread_mutex_lock(&counter_mutex);                
        fdid_t fid;
        eggFieldView_find(hEggIndexWriter->hFieldView, 
                          eggField_get_name(hField),
                          &fid);
        if (fid > 0)
        {
            eggFieldView_xlock(hEggIndexWriter->hFieldView, fid);
                
            eggIndexWriter_startAct(hEggIndexWriter);            
            
            eggFieldWeight_remove(hEggIndexWriter->hFieldWeight, hField, nDocId);
                
            eggFieldView_unlock(hEggIndexWriter->hFieldView, fid);
            
            eggIndexWriter_endAct(hEggIndexWriter);
            
            pthread_mutex_unlock(&counter_mutex);
            
            return EGG_TRUE;
        }
        pthread_mutex_unlock(&counter_mutex);

    }            
    return EGG_FALSE;

}

PRIVATE EBOOL eggIndexWriter_registerField(HEGGINDEXWRITER hEggIndexWriter_, HEGGDOCUMENT hDocument)
{
    if (EGGINDEXWRITER_IS_INVALID(hEggIndexWriter_))
    {
        return EGG_FALSE;
    }
    EGGINDEXWRITERLOCAL *hEggIndexWriter = (EGGINDEXWRITERLOCAL *)hEggIndexWriter_;

    HEGGFIELD lp_field_iter = eggDocument_get_field(hDocument, EGG_NULL);
    fdid_t fid = 0;

    pthread_mutex_lock(&counter_mutex);
    eggIndexWriter_startAct(hEggIndexWriter);
    
    
    while (lp_field_iter)
    {
        echar* lp_field_name = eggField_get_name(lp_field_iter);
        type_t type = eggField_get_type(lp_field_iter);
        if (type & EGG_NOT_INDEX)
        {
            lp_field_iter = eggField_get_next(lp_field_iter);
            continue;
        }
        
        
        if (type & EGG_OTHER_ANALYZED)
        {
            fid = eggFieldView_register(hEggIndexWriter->hFieldView, lp_field_name, type,
                                        eggField_get_analyzername(lp_field_iter));
        }
        else
        {
            fid = eggFieldView_register(hEggIndexWriter->hFieldView, lp_field_name, type);
        }
        
        if(!fid)
        {
            //printf("field type isn't same![file : %s ][line : %d ] \n", __FILE__, __LINE__);
            eggPrtLog_error("eggIndexWriterLocal", "field type isn't same![file : %s ][line : %d ] \n", __FILE__, __LINE__);
            eggIndexWriter_endAct(hEggIndexWriter);
            pthread_mutex_unlock(&counter_mutex);
            return EGG_FALSE;
        }
        
        eggField_set_mask(lp_field_iter, (size16_t) fid);
        
        lp_field_iter = eggField_get_next(lp_field_iter);

    }

    eggIndexWriter_endAct(hEggIndexWriter);
    pthread_mutex_unlock(&counter_mutex);

    return EGG_TRUE;
}
