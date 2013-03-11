#include "./eggIndexReaderServiceClient.h"
#include "../eggAnalyzer.h"
#include "../eggQuery.h"
#include "../net/eggNetPackage.h"
#include "../eggServiceClient.h"
#include "../log/eggPrtLog.h"
#include <assert.h>

struct eggIndexReaderServiceClient
{
    HEGGHANDLE hEggHandle;      /* must be first */
    char *eggDirPathName;

};
#define EGGINDEXREADER_IS_INVALID(hReader)  ((!hReader) ? EGG_TRUE : EGG_FALSE)


#define RETRY_MAX_SND 3
#define RETRY_MAX_RCV 3

HEGGINDEXREADER EGGAPI eggIndexReader_open_serviceclient(void *hEggHandle_)
{
    HEGGHANDLE hEggHandle = (HEGGHANDLE)hEggHandle_;
    EGGINDEXREADERSERVICECLIENT *hIndexReader = (EGGINDEXREADERSERVICECLIENT *)calloc(1, sizeof(EGGINDEXREADERSERVICECLIENT));
    assert(hIndexReader);
    hIndexReader->hEggHandle = hEggHandle->eggHandle_dup(hEggHandle);
    if (!hIndexReader->hEggHandle)
    {
        free(hIndexReader);
        return NULL;
    }
    
    char *eggDirPathName = eggServiceClient_get_eggdirpathname((HEGGSERVICECLIENT)hEggHandle);
    if (eggDirPathName)
    {
        hIndexReader->eggDirPathName = strdup(eggDirPathName);
        assert(hIndexReader->eggDirPathName);
    }
    
    
    return (HEGGINDEXREADER)hIndexReader;

}

EBOOL EGGAPI eggIndexReader_close_serviceclient(HEGGINDEXREADER hIndexReader_)
{
    EGGINDEXREADERSERVICECLIENT *hIndexReader= (EGGINDEXREADERSERVICECLIENT *)hIndexReader_;
    hIndexReader->hEggHandle->eggHandle_delete(hIndexReader->hEggHandle);
    hIndexReader->hEggHandle = 0;
    free(hIndexReader->eggDirPathName);
    free(hIndexReader);
    return EGG_TRUE;
}

EBOOL EGGAPI eggIndexReader_get_document_serviceclient(HEGGINDEXREADER hIndexReader_, EGGDID dId, HEGGDOCUMENT* ppeggDocument)
{
    EGGINDEXREADERSERVICECLIENT *hIndexReader = (EGGINDEXREADERSERVICECLIENT *)hIndexReader_;
    if (EGGINDEXREADER_IS_INVALID(hIndexReader))
    {
        return EGG_FALSE;
    }


    int snd_retry = 0;
    int rcv_retry = 0;

retry:
    if (snd_retry >= RETRY_MAX_SND)
    {
        eggPrtLog_error("eggIndexReaderServiceClient", "%s:%d:%s ERR snd_retry >= %d", __FILE__, __LINE__, __func__, RETRY_MAX_SND);
        
        return EGG_FALSE;
    }


    if (eggServiceClient_change_eggdirpath((HEGGSERVICECLIENT)hIndexReader->hEggHandle,
                                              hIndexReader->eggDirPathName) == EGG_FALSE)
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
        snd_retry++;
        eggPrtLog_info("eggIndexReaderServiceClient", "%s:%d:%s send fail retry[%d]", __FILE__, __LINE__, __func__, snd_retry);
        goto retry;
    }

    snd_retry = 0;

    char *lp_doc_node = EGG_NULL;
    int doc_node_size = 0;

    char *retbuf = 0;
    int retbufsz = 0;
    
    r = hIndexReader->hEggHandle->eggRemote_receive(hIndexReader->hEggHandle,
                                                    &retbuf, &retbufsz, 0);
    if (r == EGG_FALSE)
    {
        eggNetPackage_delete(retbuf);

        if (rcv_retry < RETRY_MAX_RCV)
        {
            rcv_retry++;
            eggPrtLog_info("eggIndexReaderServiceClient", "%s:%d:%s recieve fail retry[%d]", __FILE__, __LINE__, __func__, rcv_retry);
            goto retry;
        }


        eggPrtLog_error("eggIndexReaderServiceClient", "%s:%d:%s ERR receive fail rcv_retry >= %d", __FILE__, __LINE__, __func__, RETRY_MAX_RCV);
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

EBOOL EGGAPI eggIndexReader_get_documentset_serviceclient(HEGGINDEXREADER hIndexReader_, HEGGSCOREDOC hScoreDoc, count_t nDocCnt, HEGGDOCUMENT** pppeggDocument)
{

    EGGINDEXREADERSERVICECLIENT *hIndexReader = (EGGINDEXREADERSERVICECLIENT *)hIndexReader_;
    if (EGGINDEXREADER_IS_INVALID(hIndexReader))
    {
        return EGG_FALSE;
    }

    
    int snd_retry = 0;
    int rcv_retry = 0;

retry:
    if (snd_retry >= RETRY_MAX_SND)
    {
        eggPrtLog_error("eggIndexReaderServiceClient", "%s:%d:%s ERR snd_retry >= %d", __FILE__, __LINE__, __func__, RETRY_MAX_SND);
        
        return EGG_FALSE;
    }

    if (eggServiceClient_change_eggdirpath((HEGGSERVICECLIENT)hIndexReader->hEggHandle,
                                           hIndexReader->eggDirPathName) == EGG_FALSE)
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
        snd_retry++;
        eggPrtLog_info("eggIndexReaderServiceClient", "%s:%d:%s send fail retry[%d]", __FILE__, __LINE__, __func__, snd_retry);
        goto retry;
    }

    snd_retry = 0;

    char *lp_doc_set = EGG_NULL;
    int doc_set_size = 0;

    char *retbuf = 0;
    int retbufsz = 0;
    
    r = hIndexReader->hEggHandle->eggRemote_receive(hIndexReader->hEggHandle,
                                                    &retbuf, &retbufsz, 0);
    if (r == EGG_FALSE)
    {
        eggNetPackage_delete(retbuf);

        if (rcv_retry < RETRY_MAX_RCV)
        {
            rcv_retry++;
            eggPrtLog_info("eggIndexReaderServiceClient", "%s:%d:%s recieve fail retry[%d]", __FILE__, __LINE__, __func__, rcv_retry);
            goto retry;
        }

        eggPrtLog_error("eggIndexReaderServiceClient", "%s:%d:%s ERR receive fail rcv_retry >= %d", __FILE__, __LINE__, __func__, RETRY_MAX_RCV);
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


HEGGDOCUMENT EGGAPI eggIndexReader_export_document_serviceclient(HEGGINDEXREADER hIndexReader_, offset64_t* pCursor)
{
    EGGINDEXREADERSERVICECLIENT *hIndexReader = (EGGINDEXREADERSERVICECLIENT *)hIndexReader_;
    if (EGGINDEXREADER_IS_INVALID(hIndexReader))
    {
        return EGG_NULL;
    }

    int snd_retry = 0;
    int rcv_retry = 0;

retry:
    if (snd_retry >= RETRY_MAX_SND)
    {
        eggPrtLog_error("eggIndexReaderServiceClient", "%s:%d:%s ERR snd_retry >= %d", __FILE__, __LINE__, __func__, RETRY_MAX_SND);
        
        return EGG_NULL;
    }

    
    if (eggServiceClient_change_eggdirpath((HEGGSERVICECLIENT)hIndexReader->hEggHandle,
                                              hIndexReader->eggDirPathName) == EGG_FALSE)
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
        snd_retry++;
        eggPrtLog_info("eggIndexReaderServiceClient", "%s:%d:%s send fail retry[%d]", __FILE__, __LINE__, __func__, snd_retry);
        goto retry;
    }
    
    snd_retry = 0;

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
        
        if (rcv_retry < RETRY_MAX_RCV)
        {
            rcv_retry++;
            eggPrtLog_info("eggIndexReaderServiceClient", "%s:%d:%s recieve fail retry[%d]", __FILE__, __LINE__, __func__, rcv_retry);
            goto retry;
        }

        eggPrtLog_error("eggIndexReaderServiceClient", "%s:%d:%s ERR receive fail rcv_retry >= %d", __FILE__, __LINE__, __func__, RETRY_MAX_RCV);

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

EBOOL EGGAPI eggIndexReader_get_fieldnameinfo_serviceclient(HEGGINDEXREADER hIndexReader_, HEGGFIELDNAMEINFO *hhFieldNameInfo, count_t *lpCntFieldNameInfo)
{
    EGGINDEXREADERSERVICECLIENT * hIndexReader = (EGGINDEXREADERSERVICECLIENT *)hIndexReader_;
    if (EGGINDEXREADER_IS_INVALID(hIndexReader))
    {
        *hhFieldNameInfo = NULL;
        *lpCntFieldNameInfo = 0;
        return EGG_FALSE;
    }
    *hhFieldNameInfo = NULL;
    *lpCntFieldNameInfo = 0;
    

    int snd_retry = 0;
    int rcv_retry = 0;

retry:
    if (snd_retry >= RETRY_MAX_SND)
    {
        eggPrtLog_error("eggIndexReaderServiceClient", "%s:%d:%s ERR snd_retry >= %d", __FILE__, __LINE__, __func__, RETRY_MAX_SND);
        
        return EGG_FALSE;
    }

    
    if (eggServiceClient_change_eggdirpath((HEGGSERVICECLIENT)hIndexReader->hEggHandle,
                                              hIndexReader->eggDirPathName) == EGG_FALSE)
    {
        return EGG_FALSE;
    }

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
        snd_retry++;
        eggPrtLog_info("eggIndexReaderServiceClient", "%s:%d:%s send fail retry [%d]", __FILE__, __LINE__, __func__, snd_retry);
        goto retry;
    }
    
    snd_retry = 0;
    
    char *retbuf = 0;
    int retbufsz = 0;
    
    r = hIndexReader->hEggHandle->eggRemote_receive(hIndexReader->hEggHandle,
                                                    &retbuf, &retbufsz, 0);

    if (r == EGG_FALSE)
    {
        eggNetPackage_delete(retbuf);

        if (rcv_retry < RETRY_MAX_RCV)
        {
            rcv_retry++;
            eggPrtLog_info("eggIndexReaderServiceClient", "%s:%d:%s recieve fail retry[%d]", __FILE__, __LINE__, __func__, rcv_retry);
            goto retry;
        }
        
        eggPrtLog_error("eggIndexReaderServiceClient", "%s:%d:%s receive fail rcv_retry >= %d", __FILE__, __LINE__, __func__, RETRY_MAX_RCV);
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

EBOOL EGGAPI eggIndexReader_get_singlefieldnameinfo_serviceclient(HEGGINDEXREADER hIndexReader_, char *fieldName, HEGGFIELDNAMEINFO *hhFieldNameInfo)
{
    EGGINDEXREADERSERVICECLIENT * hIndexReader = (EGGINDEXREADERSERVICECLIENT *)hIndexReader_;
    if (EGGINDEXREADER_IS_INVALID(hIndexReader))
    {
        *hhFieldNameInfo = NULL;
        return EGG_FALSE;
    }
    *hhFieldNameInfo = NULL;


    int snd_retry = 0;
    int rcv_retry = 0;

retry:
    if (snd_retry >= RETRY_MAX_SND)
    {
        eggPrtLog_error("eggIndexReaderServiceClient", "%s:%d:%s ERR snd_retry >= %d", __FILE__, __LINE__, __func__, RETRY_MAX_SND);
        
        return EGG_FALSE;
    }

    if (eggServiceClient_change_eggdirpath((HEGGSERVICECLIENT)hIndexReader->hEggHandle,
                                           hIndexReader->eggDirPathName) == EGG_FALSE)
    {
        return EGG_FALSE;
    }


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
        snd_retry++;
        eggPrtLog_info("eggIndexReaderServiceClient", "%s:%d:%s send fail retry[%d]", __FILE__, __LINE__, __func__, snd_retry);
        goto retry;
    }

    snd_retry = 0;

    char *retbuf = 0;
    int retbufsz = 0;
    
    r = hIndexReader->hEggHandle->eggRemote_receive(hIndexReader->hEggHandle,
                                                    &retbuf, &retbufsz, 0);

    if (r == EGG_FALSE)
    {
        eggNetPackage_delete(retbuf);


        if (rcv_retry < RETRY_MAX_RCV)
        {
            rcv_retry++;
            eggPrtLog_info("eggIndexReaderServiceClient", "%s:%d:%s recieve fail retry[%d]", __FILE__, __LINE__, __func__, rcv_retry);
            goto retry;
        }
        
        eggPrtLog_error("eggIndexReaderServiceClient", "%s:%d:%s ERR receive fail rcv_retry >= %d", __FILE__, __LINE__, __func__, RETRY_MAX_RCV);
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
            eggPrtLog_error("eggIndexReaderServiceClient", "%s:%d:%s ERR cntFieldNameInfo[%u] != 1\n",
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

count_t EGGAPI eggIndexReader_get_doctotalcnt_serviceclient(HEGGINDEXREADER hIndexReader_)
{
    EGGINDEXREADERSERVICECLIENT * hIndexReader = (EGGINDEXREADERSERVICECLIENT *)hIndexReader_;
    if (EGGINDEXREADER_IS_INVALID(hIndexReader))
    {
        return 0;
    }



    int snd_retry = 0;
    int rcv_retry = 0;

retry:
    if (snd_retry >= RETRY_MAX_SND)
    {
        eggPrtLog_error("eggIndexReaderServiceClient", "%s:%d:%s ERR snd_retry >= %d", __FILE__, __LINE__, __func__, RETRY_MAX_SND);
        
        return 0;
    }
    
    if (eggServiceClient_change_eggdirpath((HEGGSERVICECLIENT)hIndexReader->hEggHandle,
                                           hIndexReader->eggDirPathName) == EGG_FALSE)
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
        snd_retry++;
        eggPrtLog_info("eggIndexReaderServiceClient", "%s:%d:%s send fail retry[%d]", __FILE__, __LINE__, __func__, snd_retry);
        goto retry;
    }

    snd_retry = 0;

    char *lp_doc_node = EGG_NULL;
    int doc_node_size = 0;

    char *retbuf = 0;
    int retbufsz = 0;
    
    r = hIndexReader->hEggHandle->eggRemote_receive(hIndexReader->hEggHandle,
                                                    &retbuf, &retbufsz, 0);
    if (r == EGG_FALSE)
    {
        eggNetPackage_delete(retbuf);


        if (rcv_retry < RETRY_MAX_RCV)
        {
            rcv_retry++;
            eggPrtLog_info("eggIndexReaderServiceClient", "%s:%d:%s recieve fail retry[%d]", __FILE__, __LINE__, __func__, rcv_retry);
            goto retry;
        }
        

        eggPrtLog_error("eggIndexReaderServiceClient", "%s:%d:%s receive fail rcv_retry >= %d", __FILE__, __LINE__, __func__, RETRY_MAX_RCV);
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


HEGGINDEXREADER EGGAPI eggIndexReader_alloc_serviceclient() 
{ 
    return malloc(sizeof(EGGINDEXREADERSERVICECLIENT));
} 

EBOOL EGGAPI eggIndexReader_free_serviceclient(HEGGINDEXREADER hIndexReader_)
{
    EGGINDEXREADERSERVICECLIENT *hIndexReader = (EGGINDEXREADERSERVICECLIENT *)hIndexReader_;
//    hIndexReader->hEggHandle->eggHandle_delete(hIndexReader->hEggHandle);
    free(hIndexReader);
    return EGG_TRUE;
}

EBOOL EGGAPI eggIndexReader_set_handle_serviceclient(HEGGINDEXREADERSERVICECLIENT hIndexReaderRm, void *hEggHandle_)
{
    HEGGHANDLE hEggHandle = (HEGGHANDLE)hEggHandle_;
    hIndexReaderRm->hEggHandle = hEggHandle;
    
    hIndexReaderRm->eggDirPathName = eggServiceClient_get_eggdirpathname((HEGGSERVICECLIENT)hEggHandle);
    return EGG_TRUE;

}

