#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>


#include "eggIndexWriterRemote.h"
#include "eggIndexReaderRemote.h"
#include "../net/eggNetPackage.h"
#include "../eggHttp.h"
#include "../log/eggPrtLog.h"
#include <assert.h>
extern EBOOL g_doc_storage;
struct eggIndexWriterRemote
{
    HEGGHANDLE hEggHandle;      /* must be first */
    HEGGNETPACKAGE hWriterCache;
    char *analyzerName;
};
extern pthread_mutex_t counter_mutex = PTHREAD_MUTEX_INITIALIZER;



HEGGINDEXWRITER EGGAPI eggIndexWriter_open_remote(void *hEggHandle_, char *analyzerName)
{
    HEGGHANDLE hEggHandle = (HEGGHANDLE)hEggHandle_;
    HEGGINDEXWRITERREMOTE hEggIndexWriterRm = (HEGGINDEXWRITERREMOTE)calloc(1, sizeof(EGGINDEXWRITERREMOTE));
    
    assert(hEggIndexWriterRm);
    
    hEggIndexWriterRm->hEggHandle = hEggHandle->eggHandle_dup(hEggHandle);//hEggHandle;
    
    hEggIndexWriterRm->hWriterCache = eggNetPackage_new(EGG_PACKAGE_OPTIMIZE);
    

    return (HEGGINDEXWRITER)hEggIndexWriterRm;
}

EBOOL EGGAPI eggIndexWriter_close_remote(HEGGINDEXWRITER hEggIndexWriter_)
{
    if (!hEggIndexWriter_)
    {
        return EGG_FALSE;
    }
    HEGGINDEXWRITERREMOTE hEggIndexWriterRm = (HEGGINDEXWRITERREMOTE)hEggIndexWriter_;
    hEggIndexWriterRm->hEggHandle->eggHandle_delete(hEggIndexWriterRm->hEggHandle);
    hEggIndexWriterRm->hEggHandle = 0;
    
    if(hEggIndexWriterRm->hWriterCache)
    {
        eggNetPackage_delete(hEggIndexWriterRm->hWriterCache);
        hEggIndexWriterRm->hWriterCache = EGG_NULL;
    }

    free(hEggIndexWriterRm->analyzerName);
    hEggIndexWriterRm->analyzerName = 0;        

    free(hEggIndexWriterRm);
    return EGG_TRUE;
}

EBOOL EGGAPI eggIndexWriter_set_analyzer_remote(HEGGINDEXWRITER hEggIndexWriter_, char *analyzerName)
{
    if (!hEggIndexWriter_)
    {
        return EGG_FALSE;
    }
    HEGGINDEXWRITERREMOTE hEggIndexWriterRm = (HEGGINDEXWRITERREMOTE)hEggIndexWriter_;
    
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

EBOOL EGGAPI eggIndexWriter_add_document_remote(HEGGINDEXWRITER hEggIndexWriter_, HEGGDOCUMENT hEggDocument)
{
    if (!hEggIndexWriter_)
    {
        return EGG_FALSE;
    }
    
    HEGGINDEXWRITERREMOTE hEggIndexWriterRm = (HEGGINDEXWRITERREMOTE)hEggIndexWriter_;
    g_doc_storage = EGG_TRUE;
    //
    HEGGDOCNODE lp_doc_node  = eggDocument_serialization(hEggDocument);
    g_doc_storage = EGG_FALSE;
    
    hEggIndexWriterRm->hWriterCache = eggNetPackage_add(hEggIndexWriterRm->hWriterCache,
                                                                  lp_doc_node, lp_doc_node->size, EGG_PACKAGE_OPTIMIZE_ADD);
    free(lp_doc_node);
    
    return EGG_TRUE;
}

EBOOL EGGAPI eggIndexWriter_optimize_remote(HEGGINDEXWRITER hEggIndexWriter_)
{
    if (!hEggIndexWriter_)
    {
        return EGG_FALSE;
    }
    HEGGINDEXWRITERREMOTE hEggIndexWriterRm = (HEGGINDEXWRITERREMOTE)hEggIndexWriter_;
    
    struct eggHttp *hEggHttp = hEggIndexWriterRm->hEggHandle;
    struct timeval vstart, vend;
    gettimeofday(&vstart, 0);
    
     HEGGNETPACKAGE lp_send_package = hEggIndexWriterRm->hWriterCache;
     if(!lp_send_package->eSize)
     {
         return EGG_TRUE;
     }
     
     int n_send = eggNetPackage_get_packagesize(lp_send_package);

     //  printf("eggHttp url [%s]\n", eggHttp_get_name((HEGGHTTP)hEggIndexWriterRm->hEggHandle));

     EBOOL r;

     r = hEggIndexWriterRm->hEggHandle->eggRemote_send(hEggIndexWriterRm->hEggHandle,
                                                       lp_send_package,
                                                       eggNetPackage_get_packagesize(lp_send_package),
                                                       0);
    eggNetPackage_delete(lp_send_package);
    hEggIndexWriterRm->hWriterCache = EGG_NULL;
    if (r == EGG_FALSE)
    {
        hEggIndexWriterRm->hWriterCache = eggNetPackage_new(EGG_PACKAGE_OPTIMIZE);
        return EGG_FALSE;
    }
    
    EBOOL* p_ret = EGG_NULL;
    EBOOL ret = EGG_FALSE;
    EBOOL *pret = 0;
    HEGGNETPACKAGE lp_recv_package = EGG_NULL;
    size32_t n_recv_sz = 0;
    size32_t n_rec_len;
    r = hEggIndexWriterRm->hEggHandle->eggRemote_receive(hEggIndexWriterRm->hEggHandle,
                                                         &lp_recv_package,
                                                         &n_recv_sz,
                                                         0);
    if (r == EGG_FALSE)
    {
        eggNetPackage_delete(lp_recv_package);
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
    
    
    eggNetPackage_delete(hEggIndexWriterRm->hWriterCache);
    hEggIndexWriterRm->hWriterCache = eggNetPackage_new(EGG_PACKAGE_OPTIMIZE);

    gettimeofday(&vend, 0);
//    fprintf(stderr, "n_send %u, url : [%s],  time %f \n", n_send, (char*)eggHttp_get_name(hEggHttp), (double)(vend.tv_sec - vstart.tv_sec) + (double)(vend.tv_usec - vstart.tv_usec)/1000000 );
    eggPrtLog_info("eggIndexWriterRemote", "n_send %u, url : [%s],  time %f \n", n_send, (char*)eggHttp_get_name(hEggHttp), (double)(vend.tv_sec - vstart.tv_sec) + (double)(vend.tv_usec - vstart.tv_usec)/1000000 );

    return ret;
}

HEGGINDEXREADER EGGAPI eggIndexWriter_init_reader_remote(HEGGINDEXWRITER hEggIndexWriter_)
{
    HEGGINDEXWRITERREMOTE hEggIndexWriterRm = (HEGGINDEXWRITERREMOTE)hEggIndexWriter_;
    
    HEGGINDEXREADER hEggIndexReaderRm  = eggIndexReader_alloc_remote();
    eggIndexReader_set_handle_remote(hEggIndexReaderRm, hEggIndexWriterRm->hEggHandle);
    return hEggIndexReaderRm;
}


EBOOL EGGAPI eggIndexWriter_reindex_document_remote(HEGGINDEXWRITER hEggIndexWriter_, HEGGDOCUMENT hEggDocument, EGGDID dId)
{
    return EGG_TRUE;
}

EBOOL EGGAPI eggIndexWriter_delete_document_remote(HEGGINDEXWRITER hEggIndexWriter_, EGGDID dId)
{
    if (!hEggIndexWriter_)
    {
        return EGG_FALSE;
    }
    
    HEGGINDEXWRITERREMOTE hEggIndexWriterRm = (HEGGINDEXWRITERREMOTE)hEggIndexWriter_;
    
    hEggIndexWriterRm->hWriterCache = eggNetPackage_add(hEggIndexWriterRm->hWriterCache,
                                                        &dId, sizeof(dId), EGG_PACKAGE_OPTIMIZE_DELETE);
    
    return EGG_TRUE;
    
}

EBOOL EGGAPI eggIndexWriter_modify_document_remote(HEGGINDEXWRITER hEggIndexWriter_, EGGDID dId, HEGGDOCUMENT hEggDocument)
{
    if (!hEggIndexWriter_)
    {
        return EGG_FALSE;
    }
    
    HEGGINDEXWRITERREMOTE hEggIndexWriterRm = (HEGGINDEXWRITERREMOTE)hEggIndexWriter_;
    g_doc_storage = EGG_TRUE;
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
EBOOL EGGAPI eggIndexWriter_incrementmodify_document_remote(HEGGINDEXWRITER hEggIndexWriter_, EGGDID dId, HEGGDOCUMENT hEggDocument)
{
    if (!hEggIndexWriter_)
    {
        return EGG_FALSE;
    }
    
    HEGGINDEXWRITERREMOTE hEggIndexWriterRm = (HEGGINDEXWRITERREMOTE)hEggIndexWriter_;
    g_doc_storage = EGG_TRUE;
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

EBOOL EGGAPI eggIndexWriter_add_field_remote(HEGGINDEXWRITER hEggIndexWriter_, char *fieldName, type_t fieldType, ... /* char *analyzerName */)
{
    if (!hEggIndexWriter_)
    {
        return EGG_FALSE;
    }
    
    HEGGINDEXWRITERREMOTE hEggIndexWriterRm = (HEGGINDEXWRITERREMOTE)hEggIndexWriter_;
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
        return EGG_FALSE;
    }
    
    EBOOL* p_ret = EGG_NULL;
    EBOOL ret = EGG_FALSE;
    EBOOL *pret = 0;
    HEGGNETPACKAGE lp_recv_package = EGG_NULL;
    size32_t n_recv_sz = 0;
    size32_t n_rec_len;
    r = hEggIndexWriterRm->hEggHandle->eggRemote_receive(hEggIndexWriterRm->hEggHandle,
                                                         &lp_recv_package,
                                                         &n_recv_sz,
                                                         0);
    if (r == EGG_FALSE)
    {
        eggNetPackage_delete(lp_recv_package);        
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

EBOOL EGGAPI eggIndexWriter_modify_field_remote(HEGGINDEXWRITER hEggIndexWriter_, char *oldFieldName, char *fieldName, type_t fieldType, ... /* char *analyzerName */)
{
    if (!hEggIndexWriter_)
    {
        return EGG_FALSE;
    }
    
    HEGGINDEXWRITERREMOTE hEggIndexWriterRm = (HEGGINDEXWRITERREMOTE)hEggIndexWriter_;
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
        return EGG_FALSE;
    }

    EBOOL* p_ret = EGG_NULL;
    EBOOL ret = EGG_FALSE;
    EBOOL *pret = 0;
    HEGGNETPACKAGE lp_recv_package = EGG_NULL;
    size32_t n_recv_sz = 0;
    size32_t n_rec_len;
    r = hEggIndexWriterRm->hEggHandle->eggRemote_receive(hEggIndexWriterRm->hEggHandle,
                                                         &lp_recv_package,
                                                         &n_recv_sz,
                                                         0);
    if (r == EGG_FALSE)
    {
        eggNetPackage_delete(lp_recv_package);
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
EBOOL EGGAPI eggIndexWriter_delete_field_remote(HEGGINDEXWRITER hEggIndexWriter_, char *fieldName)
{
    if (!hEggIndexWriter_)
    {
        return EGG_FALSE;
    }
    
    HEGGINDEXWRITERREMOTE hEggIndexWriterRm = (HEGGINDEXWRITERREMOTE)hEggIndexWriter_;
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
        return EGG_FALSE;
    }

    EBOOL* p_ret = EGG_NULL;
    EBOOL ret = EGG_FALSE;
    EBOOL *pret = 0;
    HEGGNETPACKAGE lp_recv_package = EGG_NULL;
    size32_t n_recv_sz = 0;
    size32_t n_rec_len;
    r = hEggIndexWriterRm->hEggHandle->eggRemote_receive(hEggIndexWriterRm->hEggHandle,
                                                         &lp_recv_package,
                                                         &n_recv_sz,
                                                         0);
    if (r == EGG_FALSE)
    {
        eggNetPackage_delete(lp_recv_package);
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
