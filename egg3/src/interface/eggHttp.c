#include "../eggHttp.h"
#include "../eggHandle.h"
#include "../net/eggNetHttp.h"
#include "../net/eggNetPackage.h"
#include "../EggDef.h"
#include "eggIndexReaderRemote.h"
#include "eggIndexWriterRemote.h"
#include "eggIndexSearcherRemote.h"
#include <assert.h>

struct eggHttp
{
    EGGHANDLE eggHandle;
    char *url;
    HEGGNETHTTP hEggNetHttp;
    HEGGNETPACKAGE hWriterCache;
};

#define EGGHTTP_IS_INVALID(hEggHttp)  \
    (!hEggHttp ? EGG_TRUE : EGG_FALSE)

HEGGHTTP eggHttp_dup(HEGGHTTP hEggHttp);
EBOOL EGGAPI eggHttp_delete(HEGGHTTP hEggHttp);
EBOOL eggHttp_send(HEGGHTTP hEggHttp, char *buf, int size, int flags);
EBOOL eggHttp_receive(HEGGHTTP hEggHttp, char **buf, int *size, int flags);
EBOOL eggHttp_serve();


HEGGHTTP EGGAPI eggHttp_open(const char *url)
{
    if (!url || !url[0])
    {
        return NULL;
    }
    struct eggHttp *hEggHttp;
    hEggHttp = (struct eggHttp*)calloc(1, sizeof(struct eggHttp));
    assert(hEggHttp);
    hEggHttp->url = strdup(url);
    assert(hEggHttp->url);

    hEggHttp->eggHandle.eggHandle_dup = eggHttp_dup;
    hEggHttp->eggHandle.eggHandle_delete = eggHttp_delete;
    hEggHttp->eggHandle.eggHandle_getName = eggHttp_get_name;
    hEggHttp->eggHandle.eggIndexReader_open = eggIndexReader_open_remote;
    hEggHttp->eggHandle.eggIndexReader_close = eggIndexReader_close_remote;
    hEggHttp->eggHandle.eggIndexReader_get_document = eggIndexReader_get_document_remote;
    hEggHttp->eggHandle.eggIndexReader_get_documentset = eggIndexReader_get_documentset_remote;
    hEggHttp->eggHandle.eggIndexReader_export_document = eggIndexReader_export_document_remote;
    hEggHttp->eggHandle.eggIndexReader_get_doctotalcnt = eggIndexReader_get_doctotalcnt_remote;
    hEggHttp->eggHandle.eggIndexReader_get_fieldnameinfo = eggIndexReader_get_fieldnameinfo_remote;
    hEggHttp->eggHandle.eggIndexReader_get_singlefieldnameinfo = eggIndexReader_get_singlefieldnameinfo_remote;
    hEggHttp->eggHandle.eggIndexReader_free = eggIndexReader_free_remote;
    
    hEggHttp->eggHandle.eggIndexSearcher_new = eggIndexSearcher_new_remote;
    hEggHttp->eggHandle.eggIndexSearcher_delete = eggIndexSearcher_delete_remote;
    hEggHttp->eggHandle.eggIndexSearcher_get_queryiter = eggIndexSearcher_get_queryiter_remote;
    hEggHttp->eggHandle.eggIndexSearcher_search_with_query = eggIndexSearcher_search_with_query_remote;
    hEggHttp->eggHandle.eggIndexSearcher_count_with_query = eggIndexSearcher_count_with_query_remote;
    hEggHttp->eggHandle.eggIndexSearcher_search_with_queryiter = eggIndexSearcher_search_with_queryiter_remote;
    hEggHttp->eggHandle.eggIndexSearcher_filter = eggIndexSearcher_filter_remote;
    
    hEggHttp->eggHandle.eggIndexWriter_open = eggIndexWriter_open_remote;
    hEggHttp->eggHandle.eggIndexWriter_close = eggIndexWriter_close_remote;
    hEggHttp->eggHandle.eggIndexWriter_set_analyzer = eggIndexWriter_set_analyzer_remote;
    hEggHttp->eggHandle.eggIndexWriter_add_document = eggIndexWriter_add_document_remote;
    hEggHttp->eggHandle.eggIndexWriter_optimize = eggIndexWriter_optimize_remote;
    hEggHttp->eggHandle.eggIndexWriter_init_reader = eggIndexWriter_init_reader_remote;
    hEggHttp->eggHandle.eggRemote_send = eggHttp_send;
    hEggHttp->eggHandle.eggRemote_receive = eggHttp_receive;
    hEggHttp->eggHandle.eggIndexWriter_reindex_document = eggIndexWriter_reindex_document_remote;
    hEggHttp->eggHandle.eggIndexWriter_modify_document = eggIndexWriter_modify_document_remote;
    hEggHttp->eggHandle.eggIndexWriter_incrementmodify_document = eggIndexWriter_incrementmodify_document_remote;
    hEggHttp->eggHandle.eggIndexWriter_delete_document = eggIndexWriter_delete_document_remote;
    hEggHttp->eggHandle.eggIndexWriter_add_field = eggIndexWriter_add_field_remote;
    hEggHttp->eggHandle.eggIndexWriter_modify_field = eggIndexWriter_modify_field_remote;
    hEggHttp->eggHandle.eggIndexWriter_delete_field = eggIndexWriter_delete_field_remote;

    hEggHttp->eggHandle.eggHandle_close = eggHttp_close;

    hEggHttp->hEggNetHttp = eggNetHttp_new(hEggHttp->url, "POST");

    return hEggHttp;
}

HEGGHTTP eggHttp_dup(HEGGHTTP hEggHttp)
{
    if (!hEggHttp)
    {
        return NULL;
    }
    
    return eggHttp_open(hEggHttp->url);
}

EBOOL EGGAPI eggHttp_delete(HEGGHTTP hEggHttp)
{
    if (EGGHTTP_IS_INVALID(hEggHttp))
    {
        return EGG_FALSE;
    }

    free(hEggHttp->url);
    eggNetHttp_delete(hEggHttp->hEggNetHttp);
    free(hEggHttp);
    return EGG_TRUE;
}

EBOOL EGGAPI eggHttp_close(HEGGHTTP hEggHttp)
{
    int ret = eggHttp_delete(hEggHttp);
    return ret;
}

char* EGGAPI eggHttp_get_name(HEGGHTTP hEggHttp)
{
    if (!hEggHttp)
    {
        return NULL;
    }
    return hEggHttp->url;
}

EBOOL eggHttp_send(HEGGHTTP hEggHttp, char *buf, int size, int flags)
{
//    printf("----------eggHttp_send start!----------\n");
    eggNetHttp_restart_network(hEggHttp->hEggNetHttp);
    eggNetHttp_connect(hEggHttp->hEggNetHttp);
    while(!eggNetHttp_send(hEggHttp->hEggNetHttp, buf, size, flags) )
    {
        if(errno == EPIPE)
        {
            sleep(1);
            eggNetHttp_restart_network(hEggHttp->hEggNetHttp);
            eggNetHttp_connect(hEggHttp->hEggNetHttp);
        }
        else
        {
            return EGG_FALSE;
        }
    }
    //  printf("----------eggHttp_send over----------!\n");

    return EGG_TRUE;
}

EBOOL eggHttp_receive(HEGGHTTP hEggHttp, char **ppbuf, int *psize, int flags)
{
    int ret = eggNetHttp_recv(hEggHttp->hEggNetHttp, ppbuf, psize, flags);
    return ret;
}
/*
EBOOL eggHttp_serve()
{
    return EGG_FALSE;
}
*/
