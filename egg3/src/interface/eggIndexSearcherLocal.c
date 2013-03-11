#include "eggIndexSearcherLocal.h"
#include "eggIndexReaderLocal.h"
#include <assert.h>

struct eggIndexSearcherLocal
{
    HEGGHANDLE hEggHandle;      /* must be first */
    HEGGINDEXREADER hIndexReader;
};
typedef struct eggIndexSearcherLocal EGGINDEXSEARCHERLOCAL;
typedef struct eggIndexSearcherLocal *HEGGINDEXSEARCHERLOCAL;

PRIVATE HEGGTOPCOLLECTOR EGGAPI eggIndexSearcher_score_document(HEGGINDEXSEARCHERLOCAL hIndexSearcher, HEGGIDNODE hIdNodes, count_t nIdCnt, HEGGTOPCOLLECTOR hTopCollector);

HEGGINDEXSEARCHER EGGAPI eggIndexSearcher_new_local(HEGGINDEXREADER hIndexReader)
{
    EGGINDEXSEARCHERLOCAL *lp_index_searcher = (EGGINDEXSEARCHERLOCAL*)malloc(sizeof(EGGINDEXSEARCHERLOCAL));
    if (EGGINDEXSEARCHER_IS_INVAILD(lp_index_searcher))
    {
        return EGG_NULL;
    }

    lp_index_searcher->hEggHandle = hIndexReader->hEggHandle;
    
    lp_index_searcher->hIndexReader = hIndexReader;

    return (HEGGINDEXSEARCHER)lp_index_searcher;
    
}

EBOOL EGGAPI eggIndexSearcher_delete_local(HEGGINDEXSEARCHER hIndexSearcher_)
{
    EGGINDEXSEARCHERLOCAL *hIndexSearcher = (EGGINDEXSEARCHERLOCAL *)hIndexSearcher_;
    if (EGGINDEXSEARCHER_IS_INVAILD(hIndexSearcher))
    {
        return EGG_FALSE;
    }
    
    free(hIndexSearcher);
    return EGG_TRUE;
    
}

HEGGSEARCHITER EGGAPI eggIndexSearcher_get_queryiter_local(HEGGINDEXSEARCHER hIndexSearcher)
{
    return eggSearchIter_new(EGG_SEARCHITER_SINGLE);
}

EBOOL EGGAPI eggIndexSearcher_search_with_queryiter_local(HEGGINDEXSEARCHER hIndexSearcher_, HEGGTOPCOLLECTOR hTopCollector, HEGGQUERY hQuery, HEGGSEARCHITER hIter)
{
    EGGINDEXSEARCHERLOCAL *hIndexSearcherLc = (EGGINDEXSEARCHERLOCAL *)hIndexSearcher_;
    if (EGGINDEXSEARCHER_IS_INVAILD(hIndexSearcherLc))
    {
        return EGG_FALSE;
    }

    EBOOL ret = eggIndexSearcher_search_with_query(hIndexSearcherLc, hTopCollector, hQuery);
    

    if(ret == EGG_TRUE)
    {
        count_t n_unit_cnt = hIter->unitCnt;
        
        eggTopCollector_cut_result(hTopCollector, hIter);
        
        if(n_unit_cnt == hIter->unitCnt)
        {
            ret = EGG_FALSE;
        }
        
        if (hIter->scoreDocIdx == EGG_PAGEFRONT)
        {
            hIter->scoreDocIdx = EGG_PAGEFIRST;
        }
        else if (hIter->scoreDocIdx == EGG_PAGEBACK)
        {
            hIter->scoreDocIdx = EGG_PAGELAST;
        }
        
    }
    else
    {
        if(hIter->iterCnt>=0)
        {
            hIter->scoreDocIdx = EGG_PAGELAST;
        }
        else
        {
            hIter->scoreDocIdx = EGG_PAGEFIRST;
        }
    }
    return ret;
}

EBOOL EGGAPI eggIndexSearcher_search_with_query_local(HEGGINDEXSEARCHER hIndexSearcher_, HEGGTOPCOLLECTOR hTopCollector, HEGGQUERY hQuery)
{
    EGGINDEXSEARCHERLOCAL *hIndexSearcher = (EGGINDEXSEARCHERLOCAL *)hIndexSearcher_;
    if (EGGINDEXSEARCHER_IS_INVAILD(hIndexSearcher))
    {
        return EGG_FALSE;
    }
    
    HEGGQUERYEXP lp_query_exp = eggQuery_init_exp(hQuery);
    
    
    
    while (lp_query_exp && lp_query_exp->opt)
    {
        HEGGIDNODE lp_idNodes_tmp = EGG_NULL;
        count_t n_tmp_cnt = eggTopCollector_total_hits(hTopCollector);

        HEGGINDEXREADERRESULT hReaderResult = NULL;
        eggIndexReader_query_documents_local(hIndexSearcher->hIndexReader, lp_query_exp, &hReaderResult);
        eggIndexReaderResult_pop_idnodes(hReaderResult, &lp_idNodes_tmp, &n_tmp_cnt);
        
        if(lp_query_exp->opt == EGGQUERYOPT_RANGE)
        {
            fdid_t fdid = 0;
            if (lp_idNodes_tmp)
            {
                fdid = lp_idNodes_tmp[0].mask;
            }
                
            HEGGWRESULT hWeightResult;
            count_t nWeightResult;
            eggIndexReaderResult_pop_rangecache(hReaderResult, &hWeightResult, &nWeightResult);
            
            eggTopCollector_add_idrange(hTopCollector,
                                        fdid,
                                        hWeightResult,
                                        nWeightResult);
            
            //sort with orderby -> sort with id
            if(n_tmp_cnt)
            {
                Uti_sedgesort (lp_idNodes_tmp, n_tmp_cnt, sizeof(EGGIDNODE), eggIdNode_cmp_id);
                Uti_filter_repeat(lp_idNodes_tmp, &n_tmp_cnt, sizeof(EGGIDNODE), eggIdNode_cmp_id);
            }
        }

        eggIndexReaderResult_delete(hReaderResult);

        lp_query_exp = eggQuery_next_exp(lp_query_exp, lp_idNodes_tmp, n_tmp_cnt);

    }

    count_t n_res_cnt = 0;
    HEGGFIELDKEY hEggFieldKey;
    HEGGIDNODE lp_res_idnodes = eggQuery_finalize_exp(lp_query_exp, &n_res_cnt, &hEggFieldKey);
     
    eggTopCollector_add_idnodes(hTopCollector, lp_res_idnodes, n_res_cnt);
    eggTopCollector_add_fieldkey(hTopCollector, hEggFieldKey);
    
    if (eggTopCollector_if_orderby(hTopCollector))
    {
        
        eggTopCollector_orderby(hTopCollector, hIndexSearcher->hIndexReader);
        
        
    }
    
        
    eggTopCollector_ultimatenormalized(hTopCollector);

    eggTopCollector_clean_idrange(hTopCollector);
    
        
    eggIndexReader_clean_cache(hIndexSearcher->hIndexReader);
    if(n_res_cnt)
        return EGG_TRUE;
    else
        return EGG_FALSE;
}


EBOOL EGGAPI eggIndexSearcher_count_with_query_local(HEGGINDEXSEARCHER hIndexSearcher_, HEGGTOPCOLLECTOR hTopCollector, HEGGQUERY hQuery)
{
    EBOOL ret;

    eggTopCollector_clean_orderby(hTopCollector);
    
    ret = eggIndexSearcher_search_with_query_local(hIndexSearcher_, hTopCollector, hQuery);

    if (ret == EGG_TRUE)
    {
        eggTopCollector_clean_fieldkey(hTopCollector);
        
        eggTopCollector_clean_idnodes(hTopCollector);

        eggTopCollector_clean_docs(hTopCollector);
    }
    
    return ret;
}


typedef struct {
    char *key;
    int size;
    EGGSCOREDOC doc;
} DOCKEY;
static int dockeycmp_int32(DOCKEY *a, DOCKEY *b)
{
    return *(int32_t*)(a->key) - *(int32_t*)(b->key);
}
static int dockeycmp_int64(DOCKEY *a, DOCKEY *b)
{
    return *(int64_t*)(a->key) - *(int64_t*)(b->key);
}
static int dockeycmp_double(DOCKEY *a, DOCKEY *b)
{
    return *(double*)(a->key) - *(double*)(b->key);
}
static int dockeycmp_string(DOCKEY *a, DOCKEY *b)
{
    return strncmp(a->key, b->key, a->size);
}
EBOOL EGGAPI eggIndexSearcher_filter_local(HEGGINDEXSEARCHER hIndexSearcher_, HEGGTOPCOLLECTOR hTopCollector, HEGGQUERY hQuery, int iforderbyit)
{
    EGGINDEXSEARCHERLOCAL *hIndexSearcher = (EGGINDEXSEARCHERLOCAL *)hIndexSearcher_;

    if (EGGINDEXSEARCHER_IS_INVAILD(hIndexSearcher))
    {
        return EGG_FALSE;
    }

    UTIVECTOR *lp_vector = Uti_vector_create(sizeof(DOCKEY));
    
    count_t cnt;
    cnt = eggTopCollector_total_hits(hTopCollector);
    HEGGSCOREDOC lp_score_doc = eggTopCollector_top_docs(hTopCollector);

    DOCKEY docKey;
    int (*funcmp)(DOCKEY *, DOCKEY *);
    int i;
    for (i = 0; i < cnt; i++)
    {
        
        HEGGQUERYEXP lp_query_exp = eggQuery_init_exp(hQuery);
        while (lp_query_exp && lp_query_exp->opt)
        {
            if(lp_query_exp->opt == EGGQUERYOPT_RANGE)
            {
                HEGGDOCUMENT lp_eggDocument = EGG_NULL;
                eggIndexReader_get_document(hIndexSearcher->hIndexReader,
                                            lp_score_doc[i].idDoc, &lp_eggDocument);
                HEGGFIELD lp_field = eggDocument_get_field(lp_eggDocument,
                                                           lp_query_exp->fieldName);
                unsigned len;
                void *pval = eggField_get_value(lp_field, &len);
                type_t fieldType;
                fieldType = eggField_get_type(lp_field);
                if (fieldType & EGG_INDEX_DOUBLE)
                {
                    if (*(double *)lp_query_exp->key1 <= *(double *)pval
                        && *(double *)pval <= *(double *)lp_query_exp->key2)
                    {
                        memcpy(&docKey.doc, &lp_score_doc[i], sizeof(EGGSCOREDOC));
                        docKey.key = malloc(sizeof(double));
                        assert(docKey.key);
                        *(double *)docKey.key = *(double *)pval;
                        docKey.size = sizeof(double);
                        Uti_vector_push(lp_vector, &docKey, 1);
                        funcmp = dockeycmp_double;
                    }
                }
                else if (fieldType & EGG_INDEX_INT64)
                {
                    if (*(int64_t*)lp_query_exp->key1 <= *(int64_t *)pval
                        && *(int64_t *)pval <= *(int64_t *)lp_query_exp->key2)
                    {
                        memcpy(&docKey.doc, &lp_score_doc[i], sizeof(EGGSCOREDOC));
                        docKey.key = malloc(8);
                        assert(docKey.key);
                        *(int64_t *)docKey.key = *(int64_t *)pval;
                        docKey.size = 8;
                        Uti_vector_push(lp_vector, &docKey, 1);
                        funcmp = dockeycmp_int64;
                    }

                }
                else if (fieldType & EGG_INDEX_INT32)
                {
                    if (*(int32_t*)lp_query_exp->key1 <= *(int32_t *)pval
                        && *(int32_t *)pval <= *(int32_t *)lp_query_exp->key2)
                    {
                        memcpy(&docKey.doc, &lp_score_doc[i], sizeof(EGGSCOREDOC));
                        docKey.key = malloc(4);
                        assert(docKey.key);
                        *(int32_t *)docKey.key = *(int32_t *)pval;
                        docKey.size = 4;
                        Uti_vector_push(lp_vector, &docKey, 1);
                        funcmp = dockeycmp_int32;
                    }
                }
                else if (fieldType & EGG_INDEX_STRING)
                {
                    if (strncmp((char *)lp_query_exp->key1, (char*)pval, lp_query_exp->key1Sz) <= 0 && strncmp((char*)pval, (char *)lp_query_exp->key2, lp_query_exp->key2Sz) <= 0)
                    {
                        memcpy(&docKey.doc, &lp_score_doc[i], sizeof(EGGSCOREDOC));
                        docKey.key = calloc(1, len+1);
                        assert(docKey.key);
                        memcpy(docKey.key, (char *)pval, len);
                        docKey.size = len;
                        Uti_vector_push(lp_vector, &docKey, 1);
                        funcmp = dockeycmp_string;
                    }
                }
                eggDocument_delete(lp_eggDocument);
            }
            lp_query_exp = eggQuery_next_exp(lp_query_exp, NULL, 0);
        }
        count_t n_res_cnt = 0;
        HEGGFIELDKEY hEggFieldKey;
        HEGGIDNODE lp_res_idnodes = eggQuery_finalize_exp(lp_query_exp, &n_res_cnt, &hEggFieldKey);
        free(lp_res_idnodes);
        eggFieldKey_del(hEggFieldKey);
    }

    if (iforderbyit)
    {
        Uti_sedgesort(Uti_vector_data(lp_vector), Uti_vector_count(lp_vector), sizeof(DOCKEY), funcmp);
        

    }

    DOCKEY *p_dockeys = Uti_vector_data(lp_vector);
    HEGGSCOREDOC p_scoreDocs;
    int n_scoreDocs = Uti_vector_count(lp_vector);
    if (n_scoreDocs == 0)
    { 
        p_scoreDocs = NULL; 
    }
    else
    {
        p_scoreDocs = malloc(sizeof(EGGSCOREDOC) * n_scoreDocs);
    }
    for (i = 0; i < n_scoreDocs; i++)
    {
        memcpy(&p_scoreDocs[i], &p_dockeys[i].doc, sizeof(EGGSCOREDOC));
        free(p_dockeys[i].key);
    }
    eggTopCollector_override_docs(hTopCollector, p_scoreDocs, n_scoreDocs);
    Uti_vector_destroy(lp_vector, EGG_TRUE);
    return EGG_TRUE;
}
