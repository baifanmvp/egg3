#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>


#include "eggIndexWriterServiceClient.h"
#include "eggIndexReaderServiceClient.h"
#include "../net/eggNetPackage.h"
#include "../eggServiceClient.h"
#include "../log/eggPrtLog.h"
#include <assert.h>
extern EBOOL g_doc_storage;
struct eggIndexWriterServiceClient
{
    HEGGHANDLE hEggHandle;      /* must be first */
    char *eggDirPathName;
    HEGGNETPACKAGE hWriterCache;
    char *analyzerName;
};
extern pthread_mutex_t counter_mutex;

#define RETRY_MAX_SND 3
#define RETRY_MAX_RCV 3

HEGGINDEXWRITER EGGAPI eggIndexWriter_open_serviceclient(void *hEggHandle_, char *analyzerName)
{
    HEGGHANDLE hEggHandle = (HEGGHANDLE)hEggHandle_;
    HEGGINDEXWRITERSERVICECLIENT hEggIndexWriterRm = (HEGGINDEXWRITERSERVICECLIENT)calloc(1, sizeof(EGGINDEXWRITERSERVICECLIENT));
    
    assert(hEggIndexWriterRm);
    char *eggDirPathName = eggServiceClient_get_eggdirpathname((HEGGSERVICECLIENT)hEggHandle);
    if (eggDirPathName)
    {
        hEggIndexWriterRm->eggDirPathName = strdup(eggDirPathName);
        assert(hEggIndexWriterRm->eggDirPathName);
    }
    hEggIndexWriterRm->hEggHandle = hEggHandle->eggHandle_dup(hEggHandle);//hEggHandle;
    
    hEggIndexWriterRm->hWriterCache = eggNetPackage_new(EGG_PACKAGE_OPTIMIZE);
    

    return (HEGGINDEXWRITER)hEggIndexWriterRm;
}

EBOOL EGGAPI eggIndexWriter_close_serviceclient(HEGGINDEXWRITER hEggIndexWriter_)
{
    if (!hEggIndexWriter_)
    {
        return EGG_FALSE;
    }
    HEGGINDEXWRITERSERVICECLIENT hEggIndexWriterRm = (HEGGINDEXWRITERSERVICECLIENT)hEggIndexWriter_;
    hEggIndexWriterRm->hEggHandle->eggHandle_delete(hEggIndexWriterRm->hEggHandle);
    hEggIndexWriterRm->hEggHandle = 0;
    
    if(hEggIndexWriterRm->hWriterCache)
    {
        eggNetPackage_delete(hEggIndexWriterRm->hWriterCache);
        hEggIndexWriterRm->hWriterCache = EGG_NULL;
    }

    free(hEggIndexWriterRm->analyzerName);
    hEggIndexWriterRm->analyzerName = 0;        

    free(hEggIndexWriterRm->eggDirPathName);
    
    free(hEggIndexWriterRm);
    return EGG_TRUE;
}

EBOOL EGGAPI eggIndexWriter_set_analyzer_serviceclient(HEGGINDEXWRITER hEggIndexWriter_, char *analyzerName)
{
    if (!hEggIndexWriter_)
    {
        return EGG_FALSE;
    }
    HEGGINDEXWRITERSERVICECLIENT hEggIndexWriterRm = (HEGGINDEXWRITERSERVICECLIENT)hEggIndexWriter_;
    
    free(hEggIndexWriterRm->analyzerName);
    if (analyzerName)
    {
        hEggIndexWriterRm->analyzerName = strdup(analyzerName);
        assert(hEggIndexWriterRm->analyzerName);
    }
    else
    {
        hEggIndexWriterRm->analyzerName = strdup("");
        assert(hEggIndexWriterRm->analyzerName);
    }

    return EGG_TRUE;
}

EBOOL EGGAPI eggIndexWriter_add_document_serviceclient(HEGGINDEXWRITER hEggIndexWriter_, HEGGDOCUMENT hEggDocument)
{
    if (!hEggIndexWriter_)
    {
        return EGG_FALSE;
    }
    
    HEGGINDEXWRITERSERVICECLIENT hEggIndexWriterRm = (HEGGINDEXWRITERSERVICECLIENT)hEggIndexWriter_;
    g_doc_storage = EGG_TRUE;
    HEGGDOCNODE lp_doc_node  = eggDocument_serialization(hEggDocument);
    g_doc_storage = EGG_FALSE;
    
    hEggIndexWriterRm->hWriterCache = eggNetPackage_add(hEggIndexWriterRm->hWriterCache,
                                                                  lp_doc_node, lp_doc_node->size, EGG_PACKAGE_OPTIMIZE_ADD);
    free(lp_doc_node);
    
    return EGG_TRUE;
}

EBOOL EGGAPI eggIndexWriter_optimize_serviceclient(HEGGINDEXWRITER hEggIndexWriter_)
{
    if (!hEggIndexWriter_)
    {
        return EGG_FALSE;
    }
    HEGGINDEXWRITERSERVICECLIENT hEggIndexWriterRm = (HEGGINDEXWRITERSERVICECLIENT)hEggIndexWriter_;
    
    HEGGSERVICECLIENT hEggServiceClient = hEggIndexWriterRm->hEggHandle;
    struct timeval vstart, vend;
    gettimeofday(&vstart, 0);
    
    HEGGNETPACKAGE lp_send_package = hEggIndexWriterRm->hWriterCache;
    if(!lp_send_package->eSize)
    {
        return EGG_TRUE;
    }


    int snd_retry = 0;
    int rcv_retry = 0;

retry:
    if (snd_retry >= RETRY_MAX_SND)
    {
        eggPrtLog_error("eggIndexWriterServiceClient", "%s:%d:%s ERR snd_retry >= %d", __FILE__, __LINE__, __func__, RETRY_MAX_SND);
        eggNetPackage_delete(lp_send_package);
        hEggIndexWriterRm->hWriterCache = eggNetPackage_new(EGG_PACKAGE_OPTIMIZE);        
        return EGG_FALSE;
    }

     
    int n_send = eggNetPackage_get_packagesize(lp_send_package);
     
    if (eggServiceClient_change_eggdirpath(hEggIndexWriterRm->hEggHandle,
                                           hEggIndexWriterRm->eggDirPathName) == EGG_FALSE)
    {

        /* eggPrtLog_error("eggIndexWriterServiceClient", "%s:%d:%s ERR eggServiceClient_change_eggdirpath == EGG_FALSE eggDirPathName %s", __FILE__, __LINE__, __func__, hEggIndexWriterRm->eggDirPathName); */
        /* eggNetPackage_delete(lp_send_package); */
        /* hEggIndexWriterRm->hWriterCache = eggNetPackage_new(EGG_PACKAGE_OPTIMIZE); */
        
        return EGG_FALSE;
    }

    EBOOL* p_ret = EGG_NULL;
    EBOOL ret = EGG_FALSE;
    HEGGNETPACKAGE lp_recv_package = EGG_NULL;
    size32_t n_recv_sz = 0;
    size32_t n_rec_len;
    unsigned n_retry = 0;
    /* rws server 写压力大时,会拒绝新的写请求,发送EGG_ERR_TRYAGAIN.
     */
    do
    {
        if (n_retry > 0)
        {
            int nsec = 30 - n_retry % 30;
            //fprintf(stderr, "%s:%d:%s: server busy, sleep %d and retry %d\n", __func__, __LINE__, __FILE__, nsec, n_retry);
            eggPrtLog_info("eggIndexWriterServiceClient", "%s:%d:%s: server busy, sleep %d and retry %d\n", __func__, __LINE__, __FILE__, nsec, n_retry);
            sleep(nsec);
        }
        
        EBOOL r;
        
        r = hEggIndexWriterRm->hEggHandle->eggRemote_send(hEggIndexWriterRm->hEggHandle,
                                                          lp_send_package,
                                                          eggNetPackage_get_packagesize(
                                                              lp_send_package),
                                                          0);

        if (r == EGG_FALSE)
        {
            snd_retry++;
            eggPrtLog_info("eggIndexWriterServiceClient", "%s:%d:%s send fail snd_retry[%d]", __FILE__, __LINE__, __func__, snd_retry);
            goto retry;
        }
        
        snd_retry = 0;

        
        r = hEggIndexWriterRm->hEggHandle->eggRemote_receive(hEggIndexWriterRm->hEggHandle,
                                                             &lp_recv_package,
                                                             &n_recv_sz,
                                                             0);
        if (r == EGG_FALSE)
        {
            eggNetPackage_delete(lp_recv_package);

            if (rcv_retry < RETRY_MAX_RCV)
            {
                rcv_retry++;
                eggPrtLog_info("eggIndexWriterServiceClient", "%s:%d:%s recieve fail retry[%d]", __FILE__, __LINE__, __func__, rcv_retry);
                goto retry;
            }
            
            eggPrtLog_error("eggIndexWriterServiceClient", "%s:%d:%s receive fail rcv_retry >= %d", __FILE__, __LINE__, __func__, RETRY_MAX_RCV);
            
            eggNetPackage_delete(lp_send_package);            
            hEggIndexWriterRm->hWriterCache = eggNetPackage_new(EGG_PACKAGE_OPTIMIZE);
            return EGG_FALSE;            
        }
        
        eggNetPackage_fetch(lp_recv_package, 2,
                            &p_ret, &n_rec_len);

        if (p_ret)
        {
            ret = *p_ret;
        }
    
        eggNetPackage_delete(lp_recv_package);
        
        n_retry++;
    } while (ret == EGG_ERR_TRYAGAIN);
    eggNetPackage_delete(lp_send_package);

    hEggIndexWriterRm->hWriterCache = EGG_NULL;
    
    eggNetPackage_delete(hEggIndexWriterRm->hWriterCache);
    hEggIndexWriterRm->hWriterCache = eggNetPackage_new(EGG_PACKAGE_OPTIMIZE);

    gettimeofday(&vend, 0);
//    fprintf(stderr, "%f[%d][%lu] n_send %u, url : [%s],  time %f \n", vend.tv_sec+vend.tv_usec/1000000., (int)getpid(), (unsigned long)pthread_self(), n_send, (char*)eggServiceClient_get_name(hEggServiceClient), (double)(vend.tv_sec - vstart.tv_sec) + (double)(vend.tv_usec - vstart.tv_usec)/1000000 );
    eggPrtLog_info("eggIndexWriterServiceClient", "%f[%d][%lu] n_send %u, url : [%s],  time %f \n", vend.tv_sec+vend.tv_usec/1000000., (int)getpid(), (unsigned long)pthread_self(), n_send, (char*)eggServiceClient_get_name(hEggServiceClient), (double)(vend.tv_sec - vstart.tv_sec) + (double)(vend.tv_usec - vstart.tv_usec)/1000000 );

    return ret;
}

HEGGINDEXREADER EGGAPI eggIndexWriter_init_reader_serviceclient(HEGGINDEXWRITER hEggIndexWriter_)
{
    HEGGINDEXWRITERSERVICECLIENT hEggIndexWriterRm = (HEGGINDEXWRITERSERVICECLIENT)hEggIndexWriter_;
    
    HEGGINDEXREADER hEggIndexReaderRm  = eggIndexReader_alloc_serviceclient();
    eggIndexReader_set_handle_serviceclient(hEggIndexReaderRm, hEggIndexWriterRm->hEggHandle);
    return hEggIndexReaderRm;
}


EBOOL EGGAPI eggIndexWriter_reindex_document_serviceclient(HEGGINDEXWRITER hEggIndexWriter_, HEGGDOCUMENT hEggDocument, EGGDID dId)
{
    return EGG_TRUE;
}

EBOOL EGGAPI eggIndexWriter_delete_document_serviceclient(HEGGINDEXWRITER hEggIndexWriter_, EGGDID dId)
{
    if (!hEggIndexWriter_)
    {
        return EGG_FALSE;
    }
    
    HEGGINDEXWRITERSERVICECLIENT hEggIndexWriterRm = (HEGGINDEXWRITERSERVICECLIENT)hEggIndexWriter_;
    
    hEggIndexWriterRm->hWriterCache = eggNetPackage_add(hEggIndexWriterRm->hWriterCache,
                                                        &dId, sizeof(dId), EGG_PACKAGE_OPTIMIZE_DELETE);
    
    return EGG_TRUE;
    
}

EBOOL EGGAPI eggIndexWriter_modify_document_serviceclient(HEGGINDEXWRITER hEggIndexWriter_, EGGDID dId, HEGGDOCUMENT hEggDocument)
{
    if (!hEggIndexWriter_)
    {
        return EGG_FALSE;
    }
    
    HEGGINDEXWRITERSERVICECLIENT hEggIndexWriterRm = (HEGGINDEXWRITERSERVICECLIENT)hEggIndexWriter_;
    g_doc_storage = EGG_TRUE;
    //
    HEGGDOCNODE lp_doc_node  = eggDocument_serialization(hEggDocument);
    g_doc_storage = EGG_FALSE;
    
    char* lp_package_buf = (char*)malloc(lp_doc_node->size + sizeof(dId));
    memcpy(lp_package_buf, &dId, sizeof(dId));
    memcpy(lp_package_buf + sizeof(dId), lp_doc_node, lp_doc_node->size);

    hEggIndexWriterRm->hWriterCache = eggNetPackage_add(hEggIndexWriterRm->hWriterCache,
                                                        lp_package_buf, lp_doc_node->size + sizeof(dId), EGG_PACKAGE_OPTIMIZE_MODIFY);
    
    free(lp_doc_node);
    free(lp_package_buf);
    return EGG_TRUE;
    
}
EBOOL EGGAPI eggIndexWriter_incrementmodify_document_serviceclient(HEGGINDEXWRITER hEggIndexWriter_, EGGDID dId, HEGGDOCUMENT hEggDocument)
{
    if (!hEggIndexWriter_)
    {
        return EGG_FALSE;
    }
    
    HEGGINDEXWRITERSERVICECLIENT hEggIndexWriterRm = (HEGGINDEXWRITERSERVICECLIENT)hEggIndexWriter_;
    g_doc_storage = EGG_TRUE;
    //
    HEGGDOCNODE lp_doc_node  = eggDocument_serialization(hEggDocument);
    g_doc_storage = EGG_FALSE;
    
    char* lp_package_buf = (char*)malloc(lp_doc_node->size + sizeof(dId));
    memcpy(lp_package_buf, &dId, sizeof(dId));
    memcpy(lp_package_buf + sizeof(dId), lp_doc_node, lp_doc_node->size);

    hEggIndexWriterRm->hWriterCache = eggNetPackage_add(hEggIndexWriterRm->hWriterCache,
                                                        lp_package_buf, lp_doc_node->size + sizeof(dId), EGG_PACKAGE_OPTIMIZE_ICREMENT);
    
    free(lp_doc_node);
    free(lp_package_buf);
    return EGG_TRUE;
    
}

EBOOL EGGAPI eggIndexWriter_add_field_serviceclient(HEGGINDEXWRITER hEggIndexWriter_, char *fieldName, type_t fieldType, ... /* char *analyzerName */)
{
    if (!hEggIndexWriter_)
    {
        return EGG_FALSE;
    }
    
    HEGGINDEXWRITERSERVICECLIENT hEggIndexWriterRm = (HEGGINDEXWRITERSERVICECLIENT)hEggIndexWriter_;

    int snd_retry = 0;
    int rcv_retry = 0;

retry:
    if (snd_retry >= RETRY_MAX_SND)
    {
        eggPrtLog_error("eggIndexWriterServiceClient", "%s:%d:%s ERR snd_retry >= %d", __FILE__, __LINE__, __func__, RETRY_MAX_SND);
        return EGG_FALSE;
    }

    if (eggServiceClient_change_eggdirpath(hEggIndexWriterRm->hEggHandle,
                                           hEggIndexWriterRm->eggDirPathName) == EGG_FALSE)
    {
        return EGG_FALSE;
    }

    EBOOL retv;
    
    HEGGNETPACKAGE lp_net_package;
    lp_net_package = eggNetPackage_new(EGG_PACKAGE_ADDFIELD);
    lp_net_package = eggNetPackage_add(lp_net_package,
                                       fieldName, strlen(fieldName)+1,
                                       EGG_PACKAGE_FIELDNAME);
    lp_net_package = eggNetPackage_add(lp_net_package,
                                       &fieldType, sizeof(fieldType),
                                       EGG_PACKAGE_FIELDTYPE);
    char *analyzerName = NULL;
    if (fieldType & EGG_OTHER_ANALYZED)
    {
        char *p;
        va_list ap_arg;
        va_start(ap_arg, fieldType);
        p = va_arg(ap_arg, char*);
        if (p && p[0])
        {
            analyzerName = strdup(p);
            assert(analyzerName);
        }
        va_end(ap_arg);
    }
    else
    {
        analyzerName = strdup("");
        assert(analyzerName);
    }
    
    lp_net_package = eggNetPackage_add(lp_net_package,
                                       analyzerName, strlen(analyzerName)+1,
                                       EGG_PACKAGE_ANALYZERNAME);
    free(analyzerName);

    EBOOL r;

    r = hEggIndexWriterRm->hEggHandle->eggRemote_send(hEggIndexWriterRm->hEggHandle,
                                                      lp_net_package,
                                                      eggNetPackage_get_packagesize(lp_net_package),
                                                      0);
    eggNetPackage_delete(lp_net_package);

    if (r == EGG_FALSE)
    {
        snd_retry++;
        eggPrtLog_info("eggIndexWriterServiceClient", "%s:%d:%s send fail retry[%d]", __FILE__, __LINE__, __func__, snd_retry);
        goto retry;
    }
    
    snd_retry = 0;

    EBOOL* p_ret = EGG_NULL;
    EBOOL ret = EGG_FALSE;
    EBOOL *pret = 0;
    HEGGNETPACKAGE lp_recv_package = EGG_NULL;
    size32_t n_recv_sz = 0;
    size32_t n_rec_len = 0;
    r = hEggIndexWriterRm->hEggHandle->eggRemote_receive(hEggIndexWriterRm->hEggHandle,
                                                         &lp_recv_package,
                                                         &n_recv_sz,
                                                         0);
    if (r == EGG_FALSE)
    {
        eggNetPackage_delete(lp_recv_package);

        if (rcv_retry < RETRY_MAX_RCV)
        {
            rcv_retry++;
            eggPrtLog_info("eggIndexWriterServiceClient", "%s:%d:%s recieve fail retry[%d]", __FILE__, __LINE__, __func__, rcv_retry);
            goto retry;
        }

        eggPrtLog_error("eggIndexWriterServiceClient", "%s:%d:%s receive fail rcv_retry >= %d", __FILE__, __LINE__, __func__, RETRY_MAX_RCV);
        
        return EGG_FALSE;
    }
    eggNetPackage_fetch(lp_recv_package, 2,
                        &p_ret, &n_rec_len);
    if (p_ret)
    {
        ret = *p_ret;
    }
    eggNetPackage_delete(lp_recv_package);
    
    return ret;


}

EBOOL EGGAPI eggIndexWriter_modify_field_serviceclient(HEGGINDEXWRITER hEggIndexWriter_, char *oldFieldName, char *fieldName, type_t fieldType, ... /* char *analyzerName */)
{
    if (!hEggIndexWriter_)
    {
        return EGG_FALSE;
    }
    
    HEGGINDEXWRITERSERVICECLIENT hEggIndexWriterRm = (HEGGINDEXWRITERSERVICECLIENT)hEggIndexWriter_;


    int snd_retry = 0;
    int rcv_retry = 0;

retry:
    if (snd_retry >= RETRY_MAX_SND)
    {
        eggPrtLog_error("eggIndexWriterServiceClient", "%s:%d:%s ERR snd_retry >= %d", __FILE__, __LINE__, __func__, RETRY_MAX_SND);
        return EGG_FALSE;
    }

    if (eggServiceClient_change_eggdirpath(hEggIndexWriterRm->hEggHandle,
                                           hEggIndexWriterRm->eggDirPathName) == EGG_FALSE)
    {
        return EGG_FALSE;
    }
    
    EBOOL retv;

    HEGGNETPACKAGE lp_net_package;
    lp_net_package = eggNetPackage_new(EGG_PACKAGE_MODIFYFIELD);
    lp_net_package = eggNetPackage_add(lp_net_package,
                                       oldFieldName, strlen(oldFieldName)+1,
                                       EGG_PACKAGE_FIELDNAME);
    lp_net_package = eggNetPackage_add(lp_net_package,
                                       fieldName, strlen(fieldName)+1,
                                       EGG_PACKAGE_FIELDNAME);
    lp_net_package = eggNetPackage_add(lp_net_package,
                                       &fieldType, sizeof(fieldType),
                                       EGG_PACKAGE_FIELDTYPE);
    char *analyzerName = NULL;
    if (fieldType & EGG_OTHER_ANALYZED)
    {
        char *p;
        va_list ap_arg;
        va_start(ap_arg, fieldType);
        p = va_arg(ap_arg, char*);
        if (p && p[0])
        {
            analyzerName = strdup(p);
            assert(analyzerName);
        }
        va_end(ap_arg);
    }
    else
    {
        analyzerName = strdup("");
        assert(analyzerName);
    }
    
    lp_net_package = eggNetPackage_add(lp_net_package,
                                       analyzerName, strlen(analyzerName)+1,
                                       EGG_PACKAGE_ANALYZERNAME);
    free(analyzerName);

    EBOOL r;
    r = hEggIndexWriterRm->hEggHandle->eggRemote_send(hEggIndexWriterRm->hEggHandle,
                                                      lp_net_package,
                                                      eggNetPackage_get_packagesize(lp_net_package),
                                                      0);
    eggNetPackage_delete(lp_net_package);
    if (r == EGG_FALSE)
    {
        snd_retry++;
        eggPrtLog_info("eggIndexWriterServiceClient", "%s:%d:%s send fail retry[%d]", __FILE__, __LINE__, __func__, snd_retry);
        goto retry;
    }

    snd_retry = 0;

    EBOOL* p_ret = EGG_NULL;
    EBOOL ret = EGG_FALSE;
    EBOOL *pret = 0;
    HEGGNETPACKAGE lp_recv_package = EGG_NULL;
    size32_t n_recv_sz = 0;
    size32_t n_rec_len = 0;
    r = hEggIndexWriterRm->hEggHandle->eggRemote_receive(hEggIndexWriterRm->hEggHandle,
                                                         &lp_recv_package,
                                                         &n_recv_sz,
                                                         0);
    if (r == EGG_FALSE)
    {
        eggNetPackage_delete(lp_recv_package);

        if (rcv_retry < RETRY_MAX_RCV)
        {
            rcv_retry++;
            eggPrtLog_info("eggIndexWriterServiceClient", "%s:%d:%s recieve fail retry[%d]", __FILE__, __LINE__, __func__, rcv_retry);
            goto retry;
        }


        eggPrtLog_error("eggIndexWriterServiceClient", "%s:%d:%s receive fail rcv_retry >= %d", __FILE__, __LINE__, __func__, RETRY_MAX_RCV);
        return EGG_FALSE;
    }
    eggNetPackage_fetch(lp_recv_package, 2,
                        &p_ret, &n_rec_len);
    if (p_ret)
    {
        ret = *p_ret;
    }
    eggNetPackage_delete(lp_recv_package);
    
    return ret;
}
EBOOL EGGAPI eggIndexWriter_delete_field_serviceclient(HEGGINDEXWRITER hEggIndexWriter_, char *fieldName)
{
    if (!hEggIndexWriter_)
    {
        return EGG_FALSE;
    }
    
    HEGGINDEXWRITERSERVICECLIENT hEggIndexWriterRm = (HEGGINDEXWRITERSERVICECLIENT)hEggIndexWriter_;


    int snd_retry = 0;
    int rcv_retry = 0;

retry:
    if (snd_retry >= RETRY_MAX_SND)
    {
        eggPrtLog_error("eggIndexWriterServiceClient", "%s:%d:%s ERR snd_retry >= %d", __FILE__, __LINE__, __func__, RETRY_MAX_SND);
        return EGG_FALSE;
    }
    
    if (eggServiceClient_change_eggdirpath(hEggIndexWriterRm->hEggHandle,
                                           hEggIndexWriterRm->eggDirPathName) == EGG_FALSE)
    {
        return EGG_FALSE;
    }
    
    EBOOL retv;

    HEGGNETPACKAGE lp_net_package;
    lp_net_package = eggNetPackage_new(EGG_PACKAGE_DELETEFIELD);
    lp_net_package = eggNetPackage_add(lp_net_package,
                                       fieldName, strlen(fieldName)+1,
                                       EGG_PACKAGE_FIELDNAME);

    EBOOL r;
    r = hEggIndexWriterRm->hEggHandle->eggRemote_send(hEggIndexWriterRm->hEggHandle,
                                                      lp_net_package,
                                                      eggNetPackage_get_packagesize(lp_net_package),
                                                      0);
    eggNetPackage_delete(lp_net_package);
    if (r == EGG_FALSE)
    {
        snd_retry++;
        eggPrtLog_info("eggIndexWriterServiceClient", "%s:%d:%s send fail retry[%d]", __FILE__, __LINE__, __func__, snd_retry);
        goto retry;
    }
    
    snd_retry = 0;

    EBOOL* p_ret = EGG_NULL;
    EBOOL ret = EGG_FALSE;
    EBOOL *pret = 0;
    HEGGNETPACKAGE lp_recv_package = EGG_NULL;
    size32_t n_recv_sz = 0;
    size32_t n_rec_len = 0;
    r = hEggIndexWriterRm->hEggHandle->eggRemote_receive(hEggIndexWriterRm->hEggHandle,
                                                         &lp_recv_package,
                                                         &n_recv_sz,
                                                         0);
    if (r == EGG_FALSE)
    {
        eggNetPackage_delete(lp_recv_package);

        if (rcv_retry < RETRY_MAX_RCV)
        {
            rcv_retry++;
            eggPrtLog_info("eggIndexWriterServiceClient", "%s:%d:%s recieve fail retry[%d]", __FILE__, __LINE__, __func__, rcv_retry);
            goto retry;

        }

        eggPrtLog_error("eggIndexWriterServiceClient", "%s:%d:%s recieve fail rcv_retry >= %d", __FILE__, __LINE__, __func__, RETRY_MAX_RCV);
        return EGG_FALSE;
    }

    eggNetPackage_fetch(lp_recv_package, 2,
                        &p_ret, &n_rec_len);
    if (p_ret)
    {
        ret = *p_ret;
    }
    eggNetPackage_delete(lp_recv_package);
    
    return ret;


}    
