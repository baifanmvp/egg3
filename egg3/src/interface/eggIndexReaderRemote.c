#include "./eggIndexReaderRemote.h"
#include "../eggAnalyzer.h"
#include "../eggQuery.h"
#include "../net/eggNetPackage.h"
#include "../log/eggPrtLog.h"
#include <assert.h>

struct eggIndexReaderRemote
{
    HEGGHANDLE hEggHandle;      /* must be first */
};
#define EGGINDEXREADER_IS_INVALID(hReader)  ((!hReader) ? EGG_TRUE : EGG_FALSE)

HEGGINDEXREADER EGGAPI eggIndexReader_open_remote(void *hEggHandle_)
{
    HEGGHANDLE hEggHandle = (HEGGHANDLE)hEggHandle_;
    EGGINDEXREADERREMOTE *hIndexReader = (EGGINDEXREADERREMOTE *)calloc(1, sizeof(EGGINDEXREADERREMOTE));
    assert(hIndexReader);
    hIndexReader->hEggHandle = hEggHandle->eggHandle_dup(hEggHandle);
    
    return (HEGGINDEXREADER)hIndexReader;

}

EBOOL EGGAPI eggIndexReader_close_remote(HEGGINDEXREADER hIndexReader_)
{
    EGGINDEXREADERREMOTE *hIndexReader= (EGGINDEXREADERREMOTE *)hIndexReader_;
    hIndexReader->hEggHandle->eggHandle_delete(hIndexReader->hEggHandle);
    hIndexReader->hEggHandle = 0;    
    free(hIndexReader);
    return EGG_TRUE;
}

EBOOL EGGAPI eggIndexReader_get_document_remote(HEGGINDEXREADER hIndexReader_, EGGDID dId, HEGGDOCUMENT* ppeggDocument)
{
    EGGINDEXREADERREMOTE *hIndexReader = (EGGINDEXREADERREMOTE *)hIndexReader_;
    if (EGGINDEXREADER_IS_INVALID(hIndexReader))
    {
        return EGG_FALSE;
    }

    HEGGNETPACKAGE lp_net_package;
    lp_net_package = eggNetPackage_new(EGG_PACKAGE_GETDOC);
    lp_net_package = eggNetPackage_add(lp_net_package, &dId, sizeof(dId), EGG_PACKAGE_ID);
    EBOOL r;
    r = hIndexReader->hEggHandle->eggRemote_send(hIndexReader->hEggHandle,
                                                 lp_net_package,
                                                 eggNetPackage_get_packagesize(lp_net_package),
                                                 0);
    eggNetPackage_delete(lp_net_package);
    if (r == EGG_FALSE)
    {
        return EGG_FALSE;
    }

    char *lp_doc_node = EGG_NULL;
    int doc_node_size = 0;

    char *retbuf = 0;
    int retbufsz = 0;
    
    r = hIndexReader->hEggHandle->eggRemote_receive(hIndexReader->hEggHandle,
                                                    &retbuf, &retbufsz, 0);
    if (r == EGG_FALSE)
    {
        eggNetPackage_delete(retbuf);
        return EGG_FALSE;
    }
    
    EBOOL* p_ret = EGG_NULL;
    EBOOL ret = EGG_FALSE;
    
    size32_t n_len_ret;
    eggNetPackage_fetch((HEGGNETPACKAGE)retbuf, 4, &p_ret, &n_len_ret, &lp_doc_node, &doc_node_size);
    ret = *p_ret;
    
    if(ret == EGG_TRUE)
    {
        HEGGDOCNODE lp_doc_tmp = (HEGGDOCNODE)malloc(doc_node_size);
        memcpy(lp_doc_tmp, lp_doc_node, doc_node_size);
        *ppeggDocument = eggDocument_unserialization(lp_doc_tmp);
    }
    eggNetPackage_delete(retbuf);
    
    
    
    return ret;
}

EBOOL EGGAPI eggIndexReader_get_documentset_remote(HEGGINDEXREADER hIndexReader_, HEGGSCOREDOC hScoreDoc, count_t nDocCnt, HEGGDOCUMENT** pppeggDocument)
{

    EGGINDEXREADERREMOTE *hIndexReader = (EGGINDEXREADERREMOTE *)hIndexReader_;
    if (EGGINDEXREADER_IS_INVALID(hIndexReader))
    {
        return EGG_FALSE;
    }

    HEGGNETPACKAGE lp_net_package;
    lp_net_package = eggNetPackage_new(EGG_PACKAGE_GETDOCSET);
    
    char* lp_send_buf = (char*)malloc(sizeof(EGGSCOREDOC) * nDocCnt + sizeof(nDocCnt));
    
    memcpy(lp_send_buf, &nDocCnt, sizeof(nDocCnt));
    memcpy(lp_send_buf + sizeof(nDocCnt), hScoreDoc, sizeof(EGGSCOREDOC) * nDocCnt);
    
    
    lp_net_package = eggNetPackage_add(lp_net_package, lp_send_buf,
                                       sizeof(EGGSCOREDOC) * nDocCnt + sizeof(nDocCnt), EGG_PACKAGE_SCOREDOCSET);

    EBOOL r;
    r = hIndexReader->hEggHandle->eggRemote_send(hIndexReader->hEggHandle,
                                                 lp_net_package,
                                                 eggNetPackage_get_packagesize(lp_net_package),
                                                 0);
    free(lp_send_buf);
    eggNetPackage_delete(lp_net_package);
    if (r == EGG_FALSE)
    {
        return EGG_FALSE;
    }

    char *lp_doc_set = EGG_NULL;
    int doc_set_size = 0;

    char *retbuf = 0;
    int retbufsz = 0;
    
    r = hIndexReader->hEggHandle->eggRemote_receive(hIndexReader->hEggHandle,
                                                    &retbuf, &retbufsz, 0);
    if (r == EGG_FALSE)
    {
        eggNetPackage_delete(retbuf);
        return EGG_FALSE;
    }


    lp_net_package = (HEGGNETPACKAGE)retbuf;
    HEGGNETUNITPACKAGE lp_net_unit = 0;;
    
    char *lp_data_iter = lp_net_package + 1;
    int n_data_sz = lp_net_package->eSize;
    EBOOL ret = EGG_FALSE;

    {
        lp_net_unit = lp_data_iter;
        memcpy(&ret, lp_net_unit + 1, lp_net_unit->size);
        lp_data_iter += sizeof(EGGNETUNITPACKAGE) + lp_net_unit->size;
        n_data_sz -= sizeof(EGGNETUNITPACKAGE) + lp_net_unit->size;
    }
    
    if(ret == EGG_TRUE)
    {
        *pppeggDocument = (HEGGDOCUMENT*)malloc(sizeof(HEGGDOCUMENT)*nDocCnt);
        index_t n_doc_idx = 0;
        while(n_data_sz)
        {
            lp_net_unit = lp_data_iter;
            if(lp_net_unit->size)
            {
                HEGGDOCNODE lp_doc_tmp = (HEGGDOCNODE)malloc(lp_net_unit->size);
                memcpy(lp_doc_tmp, lp_net_unit + 1, lp_net_unit->size);
                (*pppeggDocument)[n_doc_idx] = eggDocument_unserialization(lp_doc_tmp);
//                free(lp_doc_tmp);
            }
            else
            {
                (*pppeggDocument)[n_doc_idx] = EGG_NULL;
            }
            
            lp_data_iter += sizeof(EGGNETUNITPACKAGE) + lp_net_unit->size;
            n_data_sz -= sizeof(EGGNETUNITPACKAGE) + lp_net_unit->size;

            n_doc_idx++;
        }
    }
    eggNetPackage_delete(retbuf);
    
    
    
    return ret;
}


HEGGDOCUMENT EGGAPI eggIndexReader_export_document_remote(HEGGINDEXREADER hIndexReader_, offset64_t* pCursor)
{
    EGGINDEXREADERREMOTE *hIndexReader = (EGGINDEXREADERREMOTE *)hIndexReader_;
    if (EGGINDEXREADER_IS_INVALID(hIndexReader))
    {
        return EGG_NULL;
    }

    HEGGNETPACKAGE lp_net_package;
    lp_net_package = eggNetPackage_new(EGG_PACKAGE_EXPORTDOC);
    lp_net_package = eggNetPackage_add(lp_net_package, pCursor, sizeof(*pCursor), EGG_PACKAGE_ID);

    EBOOL r;
    r = hIndexReader->hEggHandle->eggRemote_send(hIndexReader->hEggHandle,
                                                 lp_net_package,
                                                 eggNetPackage_get_packagesize(lp_net_package),
                                                 0);
    eggNetPackage_delete(lp_net_package);
    if (r == EGG_FALSE)
    {
        return EGG_NULL;
    }


    char *lp_doc_node = EGG_NULL;
    int doc_node_size = 0;

    char *retbuf = 0;
    int retbufsz = 0;
    EBOOL* p_ret = 0;
    EBOOL ret = EGG_FALSE;
    
    offset64_t* p_cursor_tmp = EGG_NULL;
    size32_t n_ret_sz = 0;
    size32_t n_cursor_sz = 0;
    
    HEGGDOCUMENT lp_document = EGG_NULL;
    r = hIndexReader->hEggHandle->eggRemote_receive(hIndexReader->hEggHandle,
                                                    &retbuf, &retbufsz, 0);

    if (r == EGG_FALSE)
    {
        eggNetPackage_delete(retbuf);
        return EGG_NULL;
    }
    
    
    eggNetPackage_fetch((HEGGNETPACKAGE)retbuf, 6,
                        &p_ret, &n_ret_sz,
                        &p_cursor_tmp, &n_cursor_sz,
                        &lp_doc_node, &doc_node_size);
    ret = *p_ret;
    
    if(ret == EGG_TRUE)
    {
        HEGGDOCNODE lp_doc_tmp = (HEGGDOCNODE)malloc(doc_node_size);
        memcpy(lp_doc_tmp, lp_doc_node, doc_node_size);
        lp_document = eggDocument_unserialization(lp_doc_tmp);
        *pCursor = *p_cursor_tmp;
    }
    eggNetPackage_delete(retbuf);
    
    return lp_document;
}

EBOOL EGGAPI eggIndexReader_get_fieldnameinfo_remote(HEGGINDEXREADER hIndexReader_, HEGGFIELDNAMEINFO *hhFieldNameInfo, count_t *lpCntFieldNameInfo)
{
    EGGINDEXREADERREMOTE * hIndexReader = (EGGINDEXREADERREMOTE *)hIndexReader_;
    if (EGGINDEXREADER_IS_INVALID(hIndexReader))
    {
        *hhFieldNameInfo = NULL;
        *lpCntFieldNameInfo = 0;
        return 0;
    }
    *hhFieldNameInfo = NULL;
    *lpCntFieldNameInfo = 0;
    
    HEGGNETPACKAGE lp_net_package;
    lp_net_package = eggNetPackage_new(EGG_PACKAGE_GETFIELDNAMEINFO);
    EBOOL r;
    r = hIndexReader->hEggHandle->eggRemote_send(hIndexReader->hEggHandle,
                                                 lp_net_package,
                                                 eggNetPackage_get_packagesize(lp_net_package),
                                                 0);
    eggNetPackage_delete(lp_net_package);
    if (r == EGG_FALSE)
    {
        return EGG_FALSE;
    }

    char *retbuf = 0;
    int retbufsz = 0;
    
    r = hIndexReader->hEggHandle->eggRemote_receive(hIndexReader->hEggHandle,
                                                    &retbuf, &retbufsz, 0);

    if (r == EGG_FALSE)
    {
        eggNetPackage_delete(retbuf);
        return EGG_FALSE;
    }


    EBOOL* p_ret = 0;
    EBOOL retv = EGG_FALSE;
    size32_t n_ret = 0;
    char* pNameInfo = NULL;
    size32_t szNameInfo = 0;
    
    eggNetPackage_fetch((HEGGNETPACKAGE)retbuf, 4, &p_ret, &n_ret, &pNameInfo, &szNameInfo);

    retv = *p_ret;
    
    if (retv == EGG_TRUE)
    {
        *hhFieldNameInfo = eggFieldView_unserialise_fieldnameinfo(pNameInfo, szNameInfo, lpCntFieldNameInfo);
    }
    else
    {
        *hhFieldNameInfo = NULL;
        *lpCntFieldNameInfo = 0;
    }

    eggNetPackage_delete(retbuf);
    
    return retv;
}

EBOOL EGGAPI eggIndexReader_get_singlefieldnameinfo_remote(HEGGINDEXREADER hIndexReader_, char *fieldName, HEGGFIELDNAMEINFO *hhFieldNameInfo)
{
    EGGINDEXREADERREMOTE * hIndexReader = (EGGINDEXREADERREMOTE *)hIndexReader_;
    if (EGGINDEXREADER_IS_INVALID(hIndexReader))
    {
        *hhFieldNameInfo = NULL;
        return 0;
    }
    *hhFieldNameInfo = NULL;
    
    HEGGNETPACKAGE lp_net_package;
    lp_net_package = eggNetPackage_new(EGG_PACKAGE_GETSINGLEFIELDNAMEINFO);
    lp_net_package = eggNetPackage_add(lp_net_package, fieldName, strlen(fieldName)+1, EGG_PACKAGE_FIELDNAME);

    EBOOL r;
    r = hIndexReader->hEggHandle->eggRemote_send(hIndexReader->hEggHandle,
                                                 lp_net_package,
                                                 eggNetPackage_get_packagesize(lp_net_package),
                                                 0);
    eggNetPackage_delete(lp_net_package);

    if (r == EGG_FALSE)
    {
        return EGG_FALSE;
    }
    
    char *retbuf = 0;
    int retbufsz = 0;
    
    r = hIndexReader->hEggHandle->eggRemote_receive(hIndexReader->hEggHandle,
                                                    &retbuf, &retbufsz, 0);
    if (r == EGG_FALSE)
    {
        eggNetPackage_delete(retbuf);
        return EGG_FALSE;
    }
    
    EBOOL* p_ret = 0;
    EBOOL retv = EGG_FALSE;
    size32_t n_ret = 0;
    char* pNameInfo = NULL;
    size32_t szNameInfo = 0;
    
    eggNetPackage_fetch((HEGGNETPACKAGE)retbuf, 4, &p_ret, &n_ret, &pNameInfo, &szNameInfo);
    retv = *p_ret;
    
    if (retv == EGG_TRUE)
    {
        count_t cntFieldNameInfo = 0;
        *hhFieldNameInfo = eggFieldView_unserialise_fieldnameinfo(pNameInfo, szNameInfo, &cntFieldNameInfo);
        if (cntFieldNameInfo != 1)
        {
            /* fprintf(stderr, "%s:%d:%s ERR cntFieldNameInfo[%u] != 1\n", */
            /*         __FILE__, __LINE__, __func__, */
            /*         (unsigned)cntFieldNameInfo); */
            eggPrtLog_error("eggIndexReaderRemote", "%s:%d:%s ERR cntFieldNameInfo[%u] != 1\n",
                    __FILE__, __LINE__, __func__,
                    (unsigned)cntFieldNameInfo);

            eggFieldView_delete_fieldnameinfo(*hhFieldNameInfo,
                                              cntFieldNameInfo);
            *hhFieldNameInfo = NULL;
        }
    }
    else
    {
        *hhFieldNameInfo = NULL;
    }

    eggNetPackage_delete(retbuf);
    
    return retv;
}

count_t EGGAPI eggIndexReader_get_doctotalcnt_remote(HEGGINDEXREADER hIndexReader_)
{
    EGGINDEXREADERREMOTE * hIndexReader = (EGGINDEXREADERREMOTE *)hIndexReader_;
    if (EGGINDEXREADER_IS_INVALID(hIndexReader))
    {
        return 0;
    }

    HEGGNETPACKAGE lp_net_package;
    lp_net_package = eggNetPackage_new(EGG_PACKAGE_GETDOCTOTALCNT);

    EBOOL r;
    r = hIndexReader->hEggHandle->eggRemote_send(hIndexReader->hEggHandle,
                                                 lp_net_package,
                                                 eggNetPackage_get_packagesize(lp_net_package),
                                                 0);
    eggNetPackage_delete(lp_net_package);
    if (r == EGG_FALSE)
    {
        return 0;
    }
    

    char *lp_doc_node = EGG_NULL;
    int doc_node_size = 0;

    char *retbuf = 0;
    int retbufsz = 0;
    
    r = hIndexReader->hEggHandle->eggRemote_receive(hIndexReader->hEggHandle,
                                                    &retbuf, &retbufsz, 0);

    if (r == EGG_FALSE)
    {
        eggNetPackage_delete(retbuf);
        return 0;
    }
    
    EBOOL* p_ret = 0;
    size32_t n_ret;    
    count_t* p_cnt = NULL;
    count_t cnt = 0;
    size32_t n_cnt;
    
    eggNetPackage_fetch((HEGGNETPACKAGE)retbuf, 4, &p_ret, &n_ret, &p_cnt, &n_cnt);
    if (*p_ret == EGG_TRUE)
    {
        cnt = *p_cnt;
    }
    else
    {
        cnt = 0;
    }

    eggNetPackage_delete(retbuf);
    
    return cnt;
}


HEGGINDEXREADER EGGAPI eggIndexReader_alloc_remote() 
{ 
    return malloc(sizeof(EGGINDEXREADERREMOTE));
} 

EBOOL EGGAPI eggIndexReader_free_remote(HEGGINDEXREADER hIndexReader_)
{
    EGGINDEXREADERREMOTE *hIndexReader = (EGGINDEXREADERREMOTE *)hIndexReader_;
//    hIndexReader->hEggHandle->eggHandle_delete(hIndexReader->hEggHandle);
    free(hIndexReader);
    return EGG_TRUE;
}

EBOOL EGGAPI eggIndexReader_set_handle_remote(HEGGINDEXREADERREMOTE hIndexReaderRm, void *hEggHandle_)
{
    HEGGHANDLE hEggHandle = (HEGGHANDLE)hEggHandle_;
    hIndexReaderRm->hEggHandle = hEggHandle;
    
    return EGG_TRUE;

}

