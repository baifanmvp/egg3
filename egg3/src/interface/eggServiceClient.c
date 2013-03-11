#include "../eggServiceClient.h"
#include "../eggHandle.h"
#include "eggServiceClientConfig.h"
#include "../net/eggNetServiceClient.h"
#include "../net/eggNetPackage.h"
#include "../EggDef.h"
#include "eggIndexReaderServiceClient.h"
#include "eggIndexWriterServiceClient.h"
#include "eggIndexSearcherServiceClient.h"
#include "../log/eggPrtLog.h"
#include <assert.h>

struct eggServiceClient
{
    EGGHANDLE eggHandle;
    char *name;
    char *configFile;
    char *eggDirPathName;
    HEGGSERVICECLIENTCONFIG hEggServiceClientConfig;
    HEGGNETSERVICECLIENT hEggNetServiceClient;
    int flag;
};
#define FLAG_CONNECT_OK 1
#define flag_set_unconnected(flag) (flag = flag & ~FLAG_CONNECT_OK)
#define flag_set_connected(flag) (flag = flag | FLAG_CONNECT_OK)
#define flag_is_connected(flag) (flag & FLAG_CONNECT_OK)

#define EGGSERVICECLIENT_IS_INVALID(hEggServiceClient)  \
    (!hEggServiceClient ? EGG_TRUE : EGG_FALSE)

HEGGSERVICECLIENT eggServiceClient_dup(HEGGSERVICECLIENT hEggServiceClient);
EBOOL EGGAPI eggServiceClient_delete(HEGGSERVICECLIENT hEggServiceClient);
EBOOL eggServiceClient_send(HEGGSERVICECLIENT hEggServiceClient, char *buf, int size, int flags);
EBOOL eggServiceClient_receive(HEGGSERVICECLIENT hEggServiceClient, char **ppbuf, int *psize, int flags);


HEGGSERVICECLIENT EGGAPI eggServiceClient_open(const char *name)
{
    if (!name || !name[0])
    {
        return NULL;
    }

    struct eggServiceClient *hEggServiceClient = NULL;
    
    char *p = name;
    char *q;
    if (p[0] == ':'
          && p[1] == ':'
          && strchr(p+2, ':'))
    {                           /* config file */

        hEggServiceClient = (struct eggServiceClient*)calloc(1, sizeof(struct eggServiceClient));
        assert(hEggServiceClient);
        p = name;
        HEGGSERVICECLIENTCONFIG hEggServiceClientConfig;
        p +=2;
        q = strchr(p, ':');
        char *configFile;
        configFile = malloc(q-p+1);
        assert(configFile);
        configFile[q-p] = '\0';
        strncpy(configFile, p, q-p);
        char *p_eggdirpathname = NULL;
        if (q[1])
        {
            p_eggdirpathname = q+1;
        }
        else
        {
            p_eggdirpathname = name + strlen(name);
        }
    
        if (!(hEggServiceClientConfig = eggServiceClientConfig_new(configFile)))
        {
            free(configFile);
            free(hEggServiceClient);
            return NULL;
        }
        
        hEggServiceClient->configFile = configFile;
        hEggServiceClient->name = strdup(name);
        assert(hEggServiceClient->name);
        hEggServiceClient->eggDirPathName = p_eggdirpathname - name + hEggServiceClient->name;
        hEggServiceClient->hEggServiceClientConfig = hEggServiceClientConfig;
        
        
    }
    else if (p[0] == '/'
             && (p = strchr(p, ':'))
             && (q=p+1)[0] == '/')
    {                           /* unix sock file */
        hEggServiceClient = (struct eggServiceClient*)calloc(1, sizeof(struct eggServiceClient));
        assert(hEggServiceClient);
        HEGGSERVICECLIENTCONFIG hEggServiceClientConfig;
        
        if (!(hEggServiceClientConfig = eggServiceClientConfig_new_default()))
        {
            free(hEggServiceClient);
            return NULL;
        }
        hEggServiceClientConfig->socketfile = strndup(name, p - name);
        assert(hEggServiceClientConfig->socketfile);
        
        hEggServiceClient->name = strdup(name);
        assert(hEggServiceClient->name);
        hEggServiceClient->eggDirPathName = q - name + hEggServiceClient->name;
        
        hEggServiceClient->hEggServiceClientConfig = hEggServiceClientConfig;
        
    }
    else if ((p = strchr(p, ':')) &&
               (q = strchr(p+1, '/')))
    {                           /* tcp port */
        hEggServiceClient = (struct eggServiceClient*)calloc(1, sizeof(struct eggServiceClient));
        assert(hEggServiceClient);
        HEGGSERVICECLIENTCONFIG hEggServiceClientConfig;
        
        if (!(hEggServiceClientConfig = eggServiceClientConfig_new_default()))
        {
            free(hEggServiceClient);
            return NULL;
        }
        hEggServiceClientConfig->ip = strndup(name, p - name);
        assert(hEggServiceClientConfig->ip);
        hEggServiceClientConfig->port = (unsigned short)strtol(p+1, NULL, 10);
        
        hEggServiceClient->name = strdup(name);
        assert(hEggServiceClient->name);
        hEggServiceClient->eggDirPathName = q - name + hEggServiceClient->name;
        
        hEggServiceClient->hEggServiceClientConfig = hEggServiceClientConfig;
        
    }
    else
    {
        /* fprintf(stderr, */
        /*         "%s:%d:%s ERR please use '::configFile:eggDirPathName'\n" */
        /*         " Or use '/unixsockfile:/tmp/eggdata/'\n" */
        /*         " Or use 'ip:port/tmp/eggdata/'\n\n", */
        /*         __FILE__, __LINE__, __func__); */
        eggPrtLog_error("eggServiceClient",
                "%s:%d:%s ERR please use '::configFile:eggDirPathName'\n"
                " Or use '/unixsockfile:/tmp/eggdata/'\n"
                " Or use 'ip:port/tmp/eggdata/'\n\n",
                __FILE__, __LINE__, __func__);
        
        
        return NULL;
    }

    hEggServiceClient->eggHandle.eggHandle_dup = eggServiceClient_dup;
    hEggServiceClient->eggHandle.eggHandle_delete = eggServiceClient_delete;
    hEggServiceClient->eggHandle.eggHandle_getName = eggServiceClient_get_name;
    hEggServiceClient->eggHandle.eggIndexReader_open = eggIndexReader_open_serviceclient;
    hEggServiceClient->eggHandle.eggIndexReader_close = eggIndexReader_close_serviceclient;
    hEggServiceClient->eggHandle.eggIndexReader_get_document = eggIndexReader_get_document_serviceclient;
    hEggServiceClient->eggHandle.eggIndexReader_get_documentset = eggIndexReader_get_documentset_serviceclient;
    hEggServiceClient->eggHandle.eggIndexReader_export_document = eggIndexReader_export_document_serviceclient;
    hEggServiceClient->eggHandle.eggIndexReader_get_doctotalcnt = eggIndexReader_get_doctotalcnt_serviceclient;
    hEggServiceClient->eggHandle.eggIndexReader_get_fieldnameinfo = eggIndexReader_get_fieldnameinfo_serviceclient;
    hEggServiceClient->eggHandle.eggIndexReader_get_singlefieldnameinfo = eggIndexReader_get_singlefieldnameinfo_serviceclient;
    hEggServiceClient->eggHandle.eggIndexReader_free = eggIndexReader_free_serviceclient;
    
    hEggServiceClient->eggHandle.eggIndexSearcher_new = eggIndexSearcher_new_serviceclient;
    hEggServiceClient->eggHandle.eggIndexSearcher_delete = eggIndexSearcher_delete_serviceclient;
    hEggServiceClient->eggHandle.eggIndexSearcher_get_queryiter = eggIndexSearcher_get_queryiter_serviceclient;
    hEggServiceClient->eggHandle.eggIndexSearcher_search_with_query = eggIndexSearcher_search_with_query_serviceclient;
    hEggServiceClient->eggHandle.eggIndexSearcher_count_with_query = eggIndexSearcher_count_with_query_serviceclient;    
    hEggServiceClient->eggHandle.eggIndexSearcher_search_with_queryiter = eggIndexSearcher_search_with_queryiter_serviceclient;
    hEggServiceClient->eggHandle.eggIndexSearcher_filter = eggIndexSearcher_filter_serviceclient;
    
    hEggServiceClient->eggHandle.eggIndexWriter_open = eggIndexWriter_open_serviceclient;
    hEggServiceClient->eggHandle.eggIndexWriter_close = eggIndexWriter_close_serviceclient;
    hEggServiceClient->eggHandle.eggIndexWriter_set_analyzer = eggIndexWriter_set_analyzer_serviceclient;
    hEggServiceClient->eggHandle.eggIndexWriter_add_document = eggIndexWriter_add_document_serviceclient;
    hEggServiceClient->eggHandle.eggIndexWriter_optimize = eggIndexWriter_optimize_serviceclient;
    hEggServiceClient->eggHandle.eggIndexWriter_init_reader = eggIndexWriter_init_reader_serviceclient;
    hEggServiceClient->eggHandle.eggRemote_send = eggServiceClient_send;
    hEggServiceClient->eggHandle.eggRemote_receive = eggServiceClient_receive;
    hEggServiceClient->eggHandle.eggIndexWriter_reindex_document = eggIndexWriter_reindex_document_serviceclient;
    hEggServiceClient->eggHandle.eggIndexWriter_modify_document = eggIndexWriter_modify_document_serviceclient;
    hEggServiceClient->eggHandle.eggIndexWriter_incrementmodify_document = eggIndexWriter_incrementmodify_document_serviceclient;
    hEggServiceClient->eggHandle.eggIndexWriter_delete_document = eggIndexWriter_delete_document_serviceclient;
    hEggServiceClient->eggHandle.eggIndexWriter_add_field = eggIndexWriter_add_field_serviceclient;
    hEggServiceClient->eggHandle.eggIndexWriter_modify_field = eggIndexWriter_modify_field_serviceclient;
    hEggServiceClient->eggHandle.eggIndexWriter_delete_field = eggIndexWriter_delete_field_serviceclient;    

    hEggServiceClient->eggHandle.eggHandle_close = eggServiceClient_close;

    hEggServiceClient->hEggNetServiceClient = eggNetServiceClient_new(hEggServiceClient->hEggServiceClientConfig);

    flag_set_unconnected(hEggServiceClient->flag);
    
    return hEggServiceClient;
}

HEGGSERVICECLIENT eggServiceClient_dup(HEGGSERVICECLIENT hEggServiceClient)
{
    if (!hEggServiceClient)
    {
        return NULL;
    }
    
    return eggServiceClient_open(hEggServiceClient->name);
}

EBOOL EGGAPI eggServiceClient_delete(HEGGSERVICECLIENT hEggServiceClient)
{
    if (EGGSERVICECLIENT_IS_INVALID(hEggServiceClient))
    {
        return EGG_FALSE;
    }

    free(hEggServiceClient->configFile);
    free(hEggServiceClient->name);
    eggServiceClientConfig_delete(hEggServiceClient->hEggServiceClientConfig);
    eggNetServiceClient_delete(hEggServiceClient->hEggNetServiceClient);
    free(hEggServiceClient);
    return EGG_TRUE;
}

EBOOL EGGAPI eggServiceClient_close(HEGGSERVICECLIENT hEggServiceClient)
{
    int ret = eggServiceClient_delete(hEggServiceClient);
    return ret;
}

char* EGGAPI eggServiceClient_get_name(HEGGSERVICECLIENT hEggServiceClient)
{
    if (!hEggServiceClient)
    {
        return NULL;
    }
    return hEggServiceClient->name;
}

char* EGGAPI eggServiceClient_get_eggdirpathname(HEGGSERVICECLIENT hEggServiceClient)
{
    if (!hEggServiceClient)
    {
        return NULL;
    }
    return hEggServiceClient->eggDirPathName;
}

char* EGGAPI eggServiceClient_get_configfile(HEGGSERVICECLIENT hEggServiceClient)
{
    if (!hEggServiceClient)
    {
        return NULL;
    }
    return hEggServiceClient->configFile;
}

EBOOL eggServiceClient_send(HEGGSERVICECLIENT hEggServiceClient, char *buf, int size, int flags)
{
    if (!hEggServiceClient || !hEggServiceClient->hEggNetServiceClient)
    {
        return EGG_FALSE;
    }

    if (!flag_is_connected(hEggServiceClient->flag))
    {
        eggPrtLog_error("eggServiceClient", "%s:%d:%s ERR eggNetServiceClient_send eggServiceClient unconnected", __FILE__, __LINE__, __func__);
        
        return EGG_FALSE;
    }
    
    HEGGNETSERVICECLIENT hEggNetServiceClient = hEggServiceClient->hEggNetServiceClient;
    
    EBOOL retv;
    retv = eggNetServiceClient_send(hEggNetServiceClient, buf, size, flags);
    if (retv == EGG_FALSE)
    {
        flag_set_unconnected(hEggServiceClient->flag);
    }
    
    return retv;
}

EBOOL eggServiceClient_receive(HEGGSERVICECLIENT hEggServiceClient, char **ppbuf, int *psize, int flags)
{
    if (!hEggServiceClient || !hEggServiceClient->hEggNetServiceClient)
    {
        return EGG_FALSE;
    }

    if (!flag_is_connected(hEggServiceClient->flag))
    {
        eggPrtLog_error("eggServiceClient", "%s:%d:%s ERR eggNetServiceClient_receive eggServiceClient unconnected", __FILE__, __LINE__, __func__);
        
        return EGG_FALSE;
    }
    
    HEGGNETSERVICECLIENT hEggNetServiceClient = hEggServiceClient->hEggNetServiceClient;
    
    EBOOL retv;
    retv = eggNetServiceClient_recv(hEggNetServiceClient, ppbuf, psize, flags);
    if (retv == EGG_FALSE)
    {
        flag_set_unconnected(hEggServiceClient->flag);
    }
    
    return retv;
}


#define RETRY_MAX_SND 3
#define RETRY_MAX_RCV 3

PUBLIC EBOOL eggServiceClient_change_eggdirpath(HEGGSERVICECLIENT hEggServiceClient, char *eggDirPath)
{
    if (!hEggServiceClient || !hEggServiceClient->hEggNetServiceClient || !eggDirPath || !eggDirPath[0])
    {
        return EGG_FALSE;
    }

    if (strcmp(eggDirPath, hEggServiceClient->eggDirPathName) != 0)
    {
        if (strlen(hEggServiceClient->eggDirPathName) >= strlen(eggDirPath))
        {
            strcpy(hEggServiceClient->eggDirPathName, eggDirPath);
        }
        else
        {
            int n = strlen(eggDirPath)
                + hEggServiceClient->eggDirPathName - hEggServiceClient->name + 1;
            char *p = malloc(n);
            assert(p);
            strncpy(p, hEggServiceClient->name,
                    hEggServiceClient->eggDirPathName - hEggServiceClient->name);
            strcpy(hEggServiceClient->eggDirPathName - hEggServiceClient->name + p,
                   eggDirPath);
            free(hEggServiceClient->name);
            hEggServiceClient->name = p;
        }
        flag_set_unconnected(hEggServiceClient->flag);
    }
    if (flag_is_connected(hEggServiceClient->flag))
    {
        return EGG_TRUE;
    }

    HEGGNETSERVICECLIENT hEggNetServiceClient = hEggServiceClient->hEggNetServiceClient;

    int snd_retry = 0;
    int rcv_retry = 0;    
    EBOOL r = EGG_FALSE;

retry:
    if (snd_retry >= RETRY_MAX_SND)
    {
        eggPrtLog_error("eggServiceClient", "%s:%d:%s ERR send fail snd_retry >= %d\n", __FILE__, __LINE__, __func__, RETRY_MAX_SND);
        
        return EGG_FALSE;
    }
    
    if (eggNetServiceClient_restart_network(hEggNetServiceClient) == EGG_FALSE)
    {
        eggPrtLog_error("eggServiceClient", "%s:%d:%s ERR eggNetServiceClient_restart_network == EGG_FALSE\n", __FILE__, __LINE__, __func__);

        return EGG_FALSE;
    }
    
    HEGGNETPACKAGE hNetPackage = eggNetPackage_new(EGG_PACKAGE_LOADEGG);
    eggNetPackage_add(hNetPackage, eggDirPath, strlen(eggDirPath)+1, EGG_PACKAGE_PATH);
    r = eggNetServiceClient_send(hEggNetServiceClient, hNetPackage, eggNetPackage_get_packagesize(hNetPackage), 0);
    eggNetPackage_delete(hNetPackage);
    
    if (r == EGG_FALSE)
    {
        snd_retry++;
        
        eggPrtLog_error("eggServiceClient", "%s:%d:%s ERR eggServiceClient_send[%s] retry[%d]\n",
                        __FILE__, __LINE__, __func__, eggDirPath, snd_retry);
        goto retry;
    }
    
    snd_retry = 0;
    
    HEGGNETPACKAGE lp_res_package = NULL;
    int n_res_package = 0;
    r = eggNetServiceClient_recv(hEggNetServiceClient, &lp_res_package, &n_res_package, 0);
    
    if (r == EGG_FALSE)
    {
        eggNetPackage_delete(lp_res_package);

        if (rcv_retry < RETRY_MAX_RCV)
        {
            rcv_retry++;
            eggPrtLog_info("eggIndexReaderServiceClient", "%s:%d:%s recieve fail retry[%d]", __FILE__, __LINE__, __func__, rcv_retry);
            goto retry;
        }

        eggPrtLog_error("eggServiceClient", "%s:%d:%s ERR eggServiceClient_receive [%s] receive fail rcv_retry >= %d\n", __FILE__, __LINE__, __func__, eggDirPath, RETRY_MAX_RCV);

        return EGG_FALSE;
    }
    
    EBOOL ret;
    EBOOL *p_ret;
    size32_t n_ret;
    eggNetPackage_fetch(lp_res_package, 2, &p_ret, &n_ret);
    ret = *p_ret;
    eggNetPackage_delete(lp_res_package);

    if(ret == EGG_PATH_ERROR)
    {
        /* fprintf(stderr, "%s:%d:%s ERR  %s is error\n", */
        /*         __FILE__, __LINE__, __func__, eggDirPath); */
        eggPrtLog_error("eggServiceClient", "%s:%d:%s ERR  %s is error\n",
                        __FILE__, __LINE__, __func__, eggDirPath);
        ret = EGG_FALSE;
    }

    if (ret == EGG_TRUE)
    {
        flag_set_connected(hEggServiceClient->flag);
    }
    
    return ret;
}
