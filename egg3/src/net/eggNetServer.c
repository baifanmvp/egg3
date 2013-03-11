#include "eggNetServer.h"
#include "../eggSearchIter.h"
#include "../log/eggPrtLog.h"
#include <assert.h>
#include <pthread.h>
HEGGNETINDEXLIST g_eggNet_list = EGG_NULL;
pthread_mutex_t g_eggNet_list_mutex = PTHREAD_MUTEX_INITIALIZER;
extern pthread_mutex_t counter_mutex;

PRIVATE HEGGNETPACKAGE eggNetServer_loadEgg(HEGGNETSERVER hNetServer, HEGGNETPACKAGE hNetPackage);

PRIVATE HEGGNETPACKAGE eggNetServer_optimize(HEGGNETSERVER hNetServer, HEGGNETPACKAGE hNetPackage);

PRIVATE HEGGNETPACKAGE eggNetServer_query_documents(HEGGNETSERVER hNetServer, HEGGNETPACKAGE hNetPackage);

PRIVATE HEGGNETPACKAGE eggNetServer_query_count_documents(HEGGNETSERVER hNetServer, HEGGNETPACKAGE hNetPackage);

PRIVATE HEGGNETPACKAGE eggNetServer_query_documents_with_sort(HEGGNETSERVER hNetServer, HEGGNETPACKAGE hNetPackage);

PRIVATE HEGGNETPACKAGE eggNetServer_filter_documents(HEGGNETSERVER hNetServer, HEGGNETPACKAGE hNetPackage);

PRIVATE HEGGNETPACKAGE eggNetServer_get_document(HEGGNETSERVER hNetServer, HEGGNETPACKAGE hNetPackage);

PRIVATE HEGGNETPACKAGE eggNetServer_export_document(HEGGNETSERVER hNetServer, HEGGNETPACKAGE hNetPackage);

PRIVATE HEGGNETPACKAGE eggNetServer_get_documentSet(HEGGNETSERVER hNetServer, HEGGNETPACKAGE hNetPackage);

PRIVATE HEGGNETPACKAGE eggNetServer_query_documents_with_iter(HEGGNETSERVER hNetServer, HEGGNETPACKAGE hNetPackage);

PRIVATE HEGGNETPACKAGE eggNetServer_get_docTotalCnt(HEGGNETSERVER hNetServer, HEGGNETPACKAGE hNetPackage);

PRIVATE HEGGNETPACKAGE eggNetServer_get_fieldNameInfo(HEGGNETSERVER hNetServer, HEGGNETPACKAGE hNetPackage);

PRIVATE HEGGNETPACKAGE eggNetServer_get_singleFieldNameInfo(HEGGNETSERVER hNetServer, HEGGNETPACKAGE hNetPackage);

PRIVATE HEGGNETPACKAGE eggNetServer_add_field(HEGGNETSERVER hNetServer, HEGGNETPACKAGE hNetPackage);

PRIVATE HEGGNETPACKAGE eggNetServer_modify_field(HEGGNETSERVER hNetServer, HEGGNETPACKAGE hNetPackage);

PRIVATE HEGGNETPACKAGE eggNetServer_delete_field(HEGGNETSERVER hNetServer, HEGGNETPACKAGE hNetPackage);

HEGGNETINDEXLIST eggNetServer_findIndexHandle(const char* path);

HEGGNETSERVER eggNetServer_new (HEGGNETSTREAM hInStream, HEGGNETSTREAM hOutStream, eggFnServerSend fnSend, eggFnServerRecv fnRecv)
{
    HEGGNETSERVER lp_net_server = (HEGGNETSERVER)malloc(sizeof(EGGNETSERVER));
    
    lp_net_server->hInStream = hInStream;
    lp_net_server->hOutStream = hOutStream;

    lp_net_server->fnSend = fnSend;
    lp_net_server->fnRecv = fnRecv;

    lp_net_server->head = EGG_NULL;
    
    lp_net_server->hSearcher = EGG_NULL;
    lp_net_server->hReader = EGG_NULL;
    lp_net_server->hWriter = EGG_NULL;
    
    
    return lp_net_server;
}


EBOOL eggNetServer_delete (HEGGNETSERVER hNetServer)
{
    if(POINTER_IS_INVALID(hNetServer))
    {
        return EGG_FALSE;
    }
    
    free(hNetServer);
    return EGG_TRUE;
}

EBOOL eggNetServer_recv(HEGGNETSERVER hNetServer, epointer ePointer, size32_t size, int flags)
{
    if(POINTER_IS_INVALID(hNetServer))
    {
        return EGG_FALSE;
    }
    return hNetServer->fnRecv( hNetServer, ePointer, size, flags);
    
}
    
EBOOL eggNetServer_send(HEGGNETSERVER hNetServer, epointer ePointer, size32_t size, int flags)
{
    if(POINTER_IS_INVALID(hNetServer))
    {
        return EGG_FALSE;
    }
    return hNetServer->fnSend( hNetServer, ePointer, size, flags);
}

EBOOL eggNetServer_recv_cgi(HEGGNETSERVER hNetServer, epointer ePointer, size32_t size, int flags)
{
    if(POINTER_IS_INVALID(hNetServer))
    {
        return EGG_FALSE;
    }
    
    char* lp_str = (char*)ePointer;
//    FCGI_stdin->stdio_stream = stdin;
    while(size)
    {
        
        int n_recv = fread(lp_str, 1, size, stdin);
        
        if(n_recv == -1)
        {
            return EGG_FALSE;
        }
        else
        {
            size -= n_recv;
            lp_str += n_recv;
        }
    }

    
    return EGG_TRUE;
}



EBOOL eggNetServer_send_cgi(HEGGNETSERVER hNetServer, epointer ePointer, size32_t size, int flags)
{
    if(POINTER_IS_INVALID(hNetServer))
    {
        return EGG_FALSE;
    }
    
    char* lp_str = (char*)ePointer;

    while(size)
    {
        
        int n_send = fwrite(lp_str,  1, size, stdout);
        
        if(n_send == -1)
        {
            return EGG_FALSE;
        }
        else
        {
            size -= n_send;
            lp_str += n_send;
        }
    }

    fflush(hNetServer->hOutStream);

    return EGG_TRUE;
}

EBOOL eggNetServer_destory_egg(HEGGNETSERVER hNetServer)
{
    if(POINTER_IS_INVALID(hNetServer))
    {
        return EGG_FALSE;
    }
    eggIndexSearcher_delete(hNetServer->hSearcher);
    eggIndexWriter_close(hNetServer->hWriter);
    eggIndexReader_close(hNetServer->hReader);
  
//    eggDirectory_close((HEGGDIRECTORY)(hNetServer->hHandle));
    return EGG_TRUE;
}

HEGGNETPACKAGE eggNetServer_processing(HEGGNETSERVER hNetServer, HEGGNETPACKAGE hNetPackage)
{
    HEGGNETPACKAGE lp_res_package = EGG_NULL;
    EBOOL ret;
    if(POINTER_IS_INVALID(hNetServer) || POINTER_IS_INVALID(hNetPackage))
    {
        ret = EGG_NET_IVDHANDLE;
        lp_res_package = eggNetPackage_new(0);
        lp_res_package = eggNetPackage_add(lp_res_package, &ret, sizeof(ret), EGG_PACKAGE_RET);
        return lp_res_package;
    }

    
    switch(hNetPackage->op)
    {
        
    case EGG_PACKAGE_LOADEGG:
        //printf("load egg-------------\n");
        eggPrtLog_info("eggNetServer", "load egg\n");
        lp_res_package = eggNetServer_loadEgg(hNetServer, hNetPackage);break;
        
    case EGG_PACKAGE_OPTIMIZE:
        //printf("optimize-------------\n");
        eggPrtLog_info("eggNetServer", "optimize\n");
        lp_res_package = eggNetServer_optimize(hNetServer, hNetPackage);break;
        
    case EGG_PACKAGE_SEARCH:
        lp_res_package = eggNetServer_query_documents(hNetServer, hNetPackage);break;
        
    case EGG_PACKAGE_SEARCH_SORT:
        
        lp_res_package = eggNetServer_query_documents_with_sort(hNetServer, hNetPackage);break;

    case EGG_PACKAGE_SEARCHFILTER:
        lp_res_package = eggNetServer_filter_documents(hNetServer, hNetPackage);break;
        
    case EGG_PACKAGE_SEARCH_ITER:
        lp_res_package = eggNetServer_query_documents_with_iter(hNetServer, hNetPackage);break;

    case EGG_PACKAGE_SEARCH_COUNT:
        lp_res_package = eggNetServer_query_count_documents(hNetServer, hNetPackage);break;
        
    case EGG_PACKAGE_GETDOC:
        lp_res_package = eggNetServer_get_document(hNetServer, hNetPackage);break;
        
    case EGG_PACKAGE_GETDOCSET:
        lp_res_package = eggNetServer_get_documentSet(hNetServer, hNetPackage);break;
        
    case EGG_PACKAGE_EXPORTDOC:
        lp_res_package = eggNetServer_export_document(hNetServer, hNetPackage);break;

    case EGG_PACKAGE_GETDOCTOTALCNT:
        lp_res_package = eggNetServer_get_docTotalCnt(hNetServer, hNetPackage);break;
        
    case EGG_PACKAGE_GETFIELDNAMEINFO:
        lp_res_package = eggNetServer_get_fieldNameInfo(hNetServer, hNetPackage);break;
        
    case EGG_PACKAGE_GETSINGLEFIELDNAMEINFO:
        lp_res_package = eggNetServer_get_singleFieldNameInfo(hNetServer, hNetPackage);break;
        
    case EGG_PACKAGE_ADDFIELD:
        lp_res_package = eggNetServer_add_field(hNetServer, hNetPackage);break;

    case EGG_PACKAGE_MODIFYFIELD:
        lp_res_package = eggNetServer_modify_field(hNetServer, hNetPackage);break;
        
    case EGG_PACKAGE_DELETEFIELD:
        lp_res_package = eggNetServer_delete_field(hNetServer, hNetPackage);break;

    default:
        ret = EGG_NET_IVDOP;
        lp_res_package = eggNetPackage_new(hNetPackage->op);
        lp_res_package = eggNetPackage_add(lp_res_package, &ret, sizeof(ret), EGG_PACKAGE_RET);
    }

    return lp_res_package;
}

EBOOL eggNetServer_set_indexhandle(HEGGNETSERVER hNetServer, char *pEggName, char *pAnalyzerName)
{
    if(POINTER_IS_INVALID(hNetServer))
    {
        return EGG_FALSE;
    }
    
    if(!pAnalyzerName)
    {
        pAnalyzerName = ANALYZER_CWSLEX;
    }
    else if (strcmp(pAnalyzerName, ANALYZER_CNLEX) == 0)
    {
        pAnalyzerName = ANALYZER_CNLEX;
    }
    else if (strcmp(pAnalyzerName, ANALYZER_CYLEX) == 0)
    {
        pAnalyzerName = ANALYZER_CYLEX;
    }
    else
    {
        pAnalyzerName = ANALYZER_CWSLEX;
    }
    /* pthread_mutex_lock(&counter_mutex);     */
    /* HEGGNETINDEXNODE lp_res_node = eggNetIndexList_find_indexhandle(g_eggNet_list, pEggName); */
    /* if(lp_res_node) */
    /* { */
    /*     eggIndexWriter_set_analyzer(lp_res_node->hWriter, pAnalyzerName); */
    /* } */
    /* else */
    /* { */
    /*     lp_res_node = eggNetIndexList_create_indexhandle(g_eggNet_list, pEggName, pAnalyzerName); */
        
     
    /* } */
    /* pthread_mutex_unlock(&counter_mutex); */
    HEGGDIRECTORY lp_directory = eggDirectory_open(pEggName);
    if(!lp_directory) return EGG_NULL;
    
    HEGGINDEXWRITER lp_writer = eggIndexWriter_open(lp_directory, pAnalyzerName);
    if(!lp_writer) return EGG_NULL;
    
    HEGGINDEXREADER lp_reader = eggIndexWriter_init_reader(lp_writer);
    if(!lp_reader) return EGG_NULL;
    
    HEGGINDEXSEARCHER lp_searcher = eggIndexSearcher_new(lp_reader);
    if(!lp_searcher) return EGG_NULL;
    

    hNetServer->hSearcher = lp_searcher;
    hNetServer->hReader = lp_reader;
    hNetServer->hWriter = lp_writer;

    /* hNetServer->hSearcher = lp_res_node->hSearcher; */
    /* hNetServer->hReader = lp_res_node->hReader; */
    /* hNetServer->hWriter = lp_res_node->hWriter; */
    return EGG_TRUE;
}

PRIVATE HEGGNETPACKAGE eggNetServer_loadEgg(HEGGNETSERVER hNetServer, HEGGNETPACKAGE hNetPackage)
{
    HEGGNETPACKAGE lp_res_package = eggNetPackage_new(EGG_PACKAGE_LOADEGG);
    EBOOL ret = EGG_TRUE;
    if(POINTER_IS_INVALID(hNetServer) || POINTER_IS_INVALID(hNetPackage))
    {
        ret = EGG_NET_IVDHANDLE;
        lp_res_package = eggNetPackage_add(lp_res_package, &ret, sizeof(ret), EGG_PACKAGE_RET);
        return lp_res_package;
    }
    int n_path_len = 0;
    char* lp_path_sign = EGG_NULL;
    eggNetPackage_fetch(hNetPackage, 2, &lp_path_sign, &n_path_len);


    HEGGDIRECTORY lp_directory = eggDirectory_open(lp_path_sign);
    if(!lp_directory) ret = EGG_PATH_ERROR;
    
    hNetServer->hWriter = eggIndexWriter_open(lp_directory, "");
    if(!hNetServer->hWriter) ret = EGG_PATH_ERROR;
    
    hNetServer->hReader = eggIndexReader_open(lp_directory);
    if(!hNetServer->hReader) ret = EGG_PATH_ERROR;
        
    hNetServer->hSearcher = eggIndexSearcher_new(hNetServer->hReader);
    if(!hNetServer->hSearcher) ret = EGG_PATH_ERROR;

    eggDirectory_close(lp_directory);


//    eggNetServer_set_indexhandle(hNetServer, lp_path_sign, "");
    
    lp_res_package = eggNetPackage_add(lp_res_package, &ret, sizeof(ret), EGG_PACKAGE_RET);
    
    return lp_res_package;

}


PRIVATE HEGGNETPACKAGE eggNetServer_optimize(HEGGNETSERVER hNetServer, HEGGNETPACKAGE hNetPackage)
{
    HEGGNETPACKAGE lp_res_package = eggNetPackage_new(EGG_PACKAGE_OPTIMIZE);
    EBOOL ret;
    
    if(POINTER_IS_INVALID(hNetServer) || POINTER_IS_INVALID(hNetPackage))
    {
        ret = EGG_NET_IVDHANDLE;
        lp_res_package = eggNetPackage_add(lp_res_package, &ret, sizeof(ret), EGG_PACKAGE_RET);
        return lp_res_package;
    }
 
    
    size32_t n_iter_sz = 0;
    char* lp_data_str = (char*)(hNetPackage + 1);
    struct timeval tvstart,tvend;
    gettimeofday(&tvstart, 0);
    count_t a_cnt = 0;
    
    
    while(n_iter_sz != hNetPackage->eSize)
    {
        HEGGNETUNITPACKAGE lp_unit_package = (HEGGNETUNITPACKAGE)(lp_data_str + n_iter_sz);
        if(lp_unit_package->ty == EGG_PACKAGE_OPTIMIZE_ADD)
        {
            HEGGDOCNODE lp_doc_node = (HEGGDOCNODE)malloc(lp_unit_package->size);
            memcpy(lp_doc_node, lp_unit_package + 1, lp_unit_package->size);
            HEGGDOCUMENT lp_document = eggDocument_unserialization(lp_doc_node);
            
            eggIndexWriter_add_document(hNetServer->hWriter, lp_document);

            eggDocument_delete(lp_document);
	    // printf("count doc : %d\n", ++a_cnt);
        }
        else if(lp_unit_package->ty == EGG_PACKAGE_OPTIMIZE_DELETE)
        {
            EGGDID dId = {0};
            memcpy(&dId, lp_unit_package + 1, lp_unit_package->size);
            eggIndexWriter_delete_document(hNetServer->hWriter, dId );
            
        }
        else if(lp_unit_package->ty == EGG_PACKAGE_OPTIMIZE_MODIFY)   //modify
        {
            EGGDID dId = {0};
            memcpy(&dId, lp_unit_package + 1, sizeof(dId));

            HEGGDOCNODE lp_doc_node = (HEGGDOCNODE)malloc(lp_unit_package->size - sizeof(dId));
            memcpy(lp_doc_node, (char*)(lp_unit_package + 1) + sizeof(dId), lp_unit_package->size - sizeof(dId));
            HEGGDOCUMENT lp_document = eggDocument_unserialization(lp_doc_node);
            
            eggIndexWriter_modify_document(hNetServer->hWriter, dId, lp_document);
            
            //eggDocument_delete(lp_document);
        }
        else
        {
            EGGDID dId = {0};
            memcpy(&dId, lp_unit_package + 1, sizeof(dId));
            
            HEGGDOCNODE lp_doc_node = (HEGGDOCNODE)malloc(lp_unit_package->size - sizeof(dId));
            memcpy(lp_doc_node, (char*)(lp_unit_package + 1) + sizeof(dId), lp_unit_package->size - sizeof(dId));
            HEGGDOCUMENT lp_document = eggDocument_unserialization(lp_doc_node);
            
            eggIndexWriter_incrementmodify_document(hNetServer->hWriter, dId, lp_document);
            
//            eggDocument_delete(lp_document);
        }
        
        n_iter_sz += sizeof(EGGNETUNITPACKAGE) + lp_unit_package->size;
    }
    gettimeofday(&tvend, 0);

//    fprintf(stderr, "ADD doc time %f\n" , (tvend.tv_sec - tvstart.tv_sec) + (double)(tvend.tv_usec - tvstart.tv_usec)/1000000 );
    eggPrtLog_info("eggNetServer", "ADD doc time %f\n" , (tvend.tv_sec - tvstart.tv_sec) + (double)(tvend.tv_usec - tvstart.tv_usec)/1000000 );
    
    gettimeofday(&tvstart, 0);
    ret = eggIndexWriter_optimize(hNetServer->hWriter);
    gettimeofday(&tvend, 0);

//    fprintf(stderr, "optimize doc time %f\n", (tvend.tv_sec - tvstart.tv_sec) + (double)(tvend.tv_usec - tvstart.tv_usec)/1000000 );
    eggPrtLog_info("eggNetServer", "optimize doc time %f\n", (tvend.tv_sec - tvstart.tv_sec) + (double)(tvend.tv_usec - tvstart.tv_usec)/1000000 );

    
    lp_res_package = eggNetPackage_add(lp_res_package, &ret, sizeof(ret), EGG_PACKAGE_RET);

    return lp_res_package;
}

PRIVATE HEGGNETPACKAGE eggNetServer_query_documents(HEGGNETSERVER hNetServer, HEGGNETPACKAGE hNetPackage)
{
    HEGGNETPACKAGE lp_res_package = eggNetPackage_new(EGG_PACKAGE_SEARCH);
    EBOOL ret;
    
    if(POINTER_IS_INVALID(hNetServer) || POINTER_IS_INVALID(hNetPackage))
    {
        ret = EGG_NET_IVDHANDLE;
        lp_res_package = eggNetPackage_add(lp_res_package, &ret, sizeof(ret), EGG_PACKAGE_RET);
        return lp_res_package;
    }
 
    
    char* lp_topCollector_str = EGG_NULL;
    size32_t n_topCollector_sz = 0;
    char* lp_query_str = EGG_NULL;
    size32_t n_query_sz = 0;
    
    eggNetPackage_fetch(hNetPackage, 4, &lp_topCollector_str, &n_topCollector_sz, &lp_query_str, &n_query_sz);

    HEGGTOPCOLLECTOR lp_topcollector = eggTopCollector_unserialization((HEGGTOPCTORCHUNK)lp_topCollector_str);
    HEGGQUERY lp_query = eggQuery_unserialise(lp_query_str, n_query_sz);
        
    ret = eggIndexSearcher_search_with_query(hNetServer->hSearcher, lp_topcollector, lp_query);
    
    lp_res_package = eggNetPackage_add(lp_res_package, &ret, sizeof(ret), EGG_PACKAGE_RET);
    
    if(ret)
    {
        HEGGTOPCTORCHUNK lp_topctor_chunk = eggTopCollector_serialization(lp_topcollector);
        lp_res_package = eggNetPackage_add(lp_res_package, lp_topctor_chunk, lp_topctor_chunk->size, EGG_PACKAGE_TOPCOLLECTOR);
        free(lp_topctor_chunk);
    }
    
    eggQuery_delete(lp_query);
    eggTopCollector_delete(lp_topcollector);

    return lp_res_package;

}


PRIVATE HEGGNETPACKAGE eggNetServer_query_count_documents(HEGGNETSERVER hNetServer, HEGGNETPACKAGE hNetPackage)
{
    HEGGNETPACKAGE lp_res_package = eggNetPackage_new(EGG_PACKAGE_SEARCH_COUNT);
    EBOOL ret;
    
    if(POINTER_IS_INVALID(hNetServer) || POINTER_IS_INVALID(hNetPackage))
    {
        ret = EGG_NET_IVDHANDLE;
        lp_res_package = eggNetPackage_add(lp_res_package, &ret, sizeof(ret), EGG_PACKAGE_RET);
        return lp_res_package;
    }
 
    
    char* lp_topCollector_str = EGG_NULL;
    size32_t n_topCollector_sz = 0;
    char* lp_query_str = EGG_NULL;
    size32_t n_query_sz = 0;
    
    eggNetPackage_fetch(hNetPackage, 4, &lp_topCollector_str, &n_topCollector_sz, &lp_query_str, &n_query_sz);

    HEGGTOPCOLLECTOR lp_topcollector = eggTopCollector_unserialization((HEGGTOPCTORCHUNK)lp_topCollector_str);
    HEGGQUERY lp_query = eggQuery_unserialise(lp_query_str, n_query_sz);
        
    ret = eggIndexSearcher_count_with_query(hNetServer->hSearcher, lp_topcollector, lp_query);
    
    lp_res_package = eggNetPackage_add(lp_res_package, &ret, sizeof(ret), EGG_PACKAGE_RET);
    
    if(ret)
    {
        HEGGTOPCTORCHUNK lp_topctor_chunk = eggTopCollector_serialization(lp_topcollector);
        lp_res_package = eggNetPackage_add(lp_res_package, lp_topctor_chunk, lp_topctor_chunk->size, EGG_PACKAGE_TOPCOLLECTOR);
        free(lp_topctor_chunk);
    }
    
    eggQuery_delete(lp_query);
    eggTopCollector_delete(lp_topcollector);

    return lp_res_package;

}



PRIVATE HEGGNETPACKAGE eggNetServer_query_documents_with_sort(HEGGNETSERVER hNetServer, HEGGNETPACKAGE hNetPackage)
{
    HEGGNETPACKAGE lp_res_package = eggNetPackage_new(EGG_PACKAGE_SEARCH);
    EBOOL ret;
    struct timeval vstart, vend;
    gettimeofday(&vstart, 0);
    
    if(POINTER_IS_INVALID(hNetServer) || POINTER_IS_INVALID(hNetPackage))
    {
        ret = EGG_NET_IVDHANDLE;
        lp_res_package = eggNetPackage_add(lp_res_package, &ret, sizeof(ret), EGG_PACKAGE_RET);
        return lp_res_package;
    }
 
    
    char* lp_topCollector_str = EGG_NULL;
    size32_t n_topCollector_sz = 0;
    char* lp_query_str = EGG_NULL;
    size32_t n_query_sz = 0;
    
    eggNetPackage_fetch(hNetPackage, 4, &lp_topCollector_str, &n_topCollector_sz, &lp_query_str, &n_query_sz);

    HEGGTOPCOLLECTOR lp_topcollector = eggTopCollector_unserialization((HEGGTOPCTORCHUNK)lp_topCollector_str);
    HEGGQUERY lp_query = eggQuery_unserialise(lp_query_str, n_query_sz);
    
    gettimeofday(&vend, 0);
//    fprintf(stderr, "%f[%d] query before time : %f \n", vstart.tv_sec+vstart.tv_usec/1000000., getpid(), (double)(vend.tv_sec - vstart.tv_sec) + (double)(vend.tv_usec - vstart.tv_usec)/1000000);
    eggPrtLog_info("eggNetServer", "%f[%d] query before time : %f \n", vstart.tv_sec+vstart.tv_usec/1000000., getpid(), (double)(vend.tv_sec - vstart.tv_sec) + (double)(vend.tv_usec - vstart.tv_usec)/1000000);
    
      
    gettimeofday(&vstart, 0);

    ret = eggIndexSearcher_search_with_query(hNetServer->hSearcher, lp_topcollector, lp_query);
    gettimeofday(&vend, 0);
//    fprintf(stderr, "%f[%d] query ing time : %f \n", vstart.tv_sec+vstart.tv_usec/1000000., getpid(), (double)(vend.tv_sec - vstart.tv_sec) + (double)(vend.tv_usec - vstart.tv_usec)/1000000);
    eggPrtLog_info("eggNetServer", "%f[%d] query ing time : %f \n", vstart.tv_sec+vstart.tv_usec/1000000., getpid(), (double)(vend.tv_sec - vstart.tv_sec) + (double)(vend.tv_usec - vstart.tv_usec)/1000000);

    gettimeofday(&vstart, 0);
    
    lp_res_package = eggNetPackage_add(lp_res_package, &ret, sizeof(ret), EGG_PACKAGE_RET);
    
    if(ret)
    {
        
        HEGGTOPCTORCHUNK lp_topctor_chunk = eggTopCollector_serialization(lp_topcollector);
        lp_res_package = eggNetPackage_add(lp_res_package, lp_topctor_chunk, lp_topctor_chunk->size, EGG_PACKAGE_TOPCOLLECTOR);
        free(lp_topctor_chunk);
        //printf("ret ------------------ is  TRUE\n");
        eggPrtLog_info("eggNetServer", "ret is TRUE\n");
    }
    
    eggQuery_delete(lp_query);
    eggTopCollector_delete(lp_topcollector);
    gettimeofday(&vend, 0);
//    fprintf(stderr,"%f[%d] query after time : %f \n", vstart.tv_sec+vstart.tv_usec/1000000., getpid(), (double)(vend.tv_sec - vstart.tv_sec) + (double)(vend.tv_usec - vstart.tv_usec)/1000000);
    eggPrtLog_info("eggNetServer","%f[%d] query after time : %f \n", vstart.tv_sec+vstart.tv_usec/1000000., getpid(), (double)(vend.tv_sec - vstart.tv_sec) + (double)(vend.tv_usec - vstart.tv_usec)/1000000);

    return lp_res_package;

}


PRIVATE HEGGNETPACKAGE eggNetServer_query_documents_with_iter(HEGGNETSERVER hNetServer, HEGGNETPACKAGE hNetPackage)
{
    HEGGNETPACKAGE lp_res_package = eggNetPackage_new(EGG_PACKAGE_SEARCH);
    EBOOL ret;
    struct timeval vstart, vend;
    gettimeofday(&vstart, 0);
    
    if(POINTER_IS_INVALID(hNetServer) || POINTER_IS_INVALID(hNetPackage))
    {
        ret = EGG_NET_IVDHANDLE;
        lp_res_package = eggNetPackage_add(lp_res_package, &ret, sizeof(ret), EGG_PACKAGE_RET);
        return lp_res_package;
    }
 
    
    char* lp_topCollector_str = EGG_NULL;
    size32_t n_topCollector_sz = 0;
    char* lp_query_str = EGG_NULL;
    size32_t n_query_sz = 0;
    HEGGSEARCHITER lp_iter = EGG_NULL;
    size32_t n_iter_sz = 0;
    
    eggNetPackage_fetch(hNetPackage, 6, &lp_iter, &n_iter_sz, &lp_topCollector_str, &n_topCollector_sz,
                        &lp_query_str, &n_query_sz);

    HEGGTOPCOLLECTOR lp_topcollector = eggTopCollector_unserialization((HEGGTOPCTORCHUNK)lp_topCollector_str);
    HEGGQUERY lp_query = eggQuery_unserialise(lp_query_str, n_query_sz);
    gettimeofday(&vend, 0);
//    fprintf(stderr, "%f[%d] query before time : %f \n", vstart.tv_sec+vstart.tv_usec/1000000., getpid(), (double)(vend.tv_sec - vstart.tv_sec) + (double)(vend.tv_usec - vstart.tv_usec)/1000000);
    eggPrtLog_info("eggNetServer", "%f[%d] query before time : %f \n", vstart.tv_sec+vstart.tv_usec/1000000., getpid(), (double)(vend.tv_sec - vstart.tv_sec) + (double)(vend.tv_usec - vstart.tv_usec)/1000000);
  
      
    gettimeofday(&vstart, 0);
    ret = eggIndexSearcher_search_with_queryiter(hNetServer->hSearcher, lp_topcollector, lp_query, lp_iter);
    
    gettimeofday(&vend, 0);
//    fprintf(stderr, "%f[%d] query ing time : %f \n", vstart.tv_sec+vstart.tv_usec/1000000., getpid(), (double)(vend.tv_sec - vstart.tv_sec) + (double)(vend.tv_usec - vstart.tv_usec)/1000000);
    eggPrtLog_info("eggNetServer", "%f[%d] query ing time : %f \n", vstart.tv_sec+vstart.tv_usec/1000000., getpid(), (double)(vend.tv_sec - vstart.tv_sec) + (double)(vend.tv_usec - vstart.tv_usec)/1000000);

    gettimeofday(&vstart, 0);
    
    lp_res_package = eggNetPackage_add(lp_res_package, &ret, sizeof(ret), EGG_PACKAGE_RET);
    

    
    
    lp_res_package = eggNetPackage_add(lp_res_package, lp_iter, n_iter_sz, EGG_PACKAGE_ITER);
//    eggSearchIter_delete(lp_iter);
    
    HEGGTOPCTORCHUNK lp_topctor_chunk = eggTopCollector_serialization(lp_topcollector);
    lp_res_package = eggNetPackage_add(lp_res_package, lp_topctor_chunk, lp_topctor_chunk->size, EGG_PACKAGE_TOPCOLLECTOR);
    free(lp_topctor_chunk);
    
    eggQuery_delete(lp_query);
    eggTopCollector_delete(lp_topcollector);
    gettimeofday(&vend, 0);
//    fprintf(stderr,"%f[%d] query after time : %f \n", vstart.tv_sec+vstart.tv_usec/1000000., getpid(), (double)(vend.tv_sec - vstart.tv_sec) + (double)(vend.tv_usec - vstart.tv_usec)/1000000);
    eggPrtLog_info("eggNetServer","%f[%d] query after time : %f \n", vstart.tv_sec+vstart.tv_usec/1000000., getpid(), (double)(vend.tv_sec - vstart.tv_sec) + (double)(vend.tv_usec - vstart.tv_usec)/1000000);

    return lp_res_package;

}


PRIVATE HEGGNETPACKAGE eggNetServer_filter_documents(HEGGNETSERVER hNetServer, HEGGNETPACKAGE hNetPackage)
{
    HEGGNETPACKAGE lp_res_package = eggNetPackage_new(EGG_PACKAGE_SEARCHFILTER);
    EBOOL ret;
    
    if(POINTER_IS_INVALID(hNetServer) || POINTER_IS_INVALID(hNetPackage))
    {
        ret = EGG_NET_IVDHANDLE;
        lp_res_package = eggNetPackage_add(lp_res_package, &ret, sizeof(ret), EGG_PACKAGE_RET);
        return lp_res_package;
    }
 
    
    char* lp_topCollector_str = EGG_NULL;
    size32_t n_topCollector_sz = 0;
    char* lp_query_str = EGG_NULL;
    size32_t n_query_sz = 0;
    char iforderbyit = 0;
    size32_t iforderbyit_sz = 0;
    
    eggNetPackage_fetch(hNetPackage, 6, &lp_topCollector_str, &n_topCollector_sz, &lp_query_str, &n_query_sz, &iforderbyit, &iforderbyit_sz);

    HEGGTOPCOLLECTOR lp_topcollector = eggTopCollector_unserialization((HEGGTOPCTORCHUNK)lp_topCollector_str);
    HEGGQUERY lp_query = eggQuery_unserialise(lp_query_str, n_query_sz);
        
    ret = eggIndexSearcher_filter(hNetServer->hSearcher, lp_topcollector, lp_query, iforderbyit);
    
    lp_res_package = eggNetPackage_add(lp_res_package, &ret, sizeof(ret), EGG_PACKAGE_RET);
    
    if(ret)
    {
        HEGGTOPCTORCHUNK lp_topctor_chunk = eggTopCollector_serialization(lp_topcollector);
        lp_res_package = eggNetPackage_add(lp_res_package, lp_topctor_chunk, lp_topctor_chunk->size, EGG_PACKAGE_TOPCOLLECTOR);
        free(lp_topctor_chunk);
    }
    
    eggQuery_delete(lp_query);
    eggTopCollector_delete(lp_topcollector);

    return lp_res_package;

}

PRIVATE HEGGNETPACKAGE eggNetServer_get_document(HEGGNETSERVER hNetServer, HEGGNETPACKAGE hNetPackage)
{
    
    HEGGNETPACKAGE lp_res_package = eggNetPackage_new(EGG_PACKAGE_GETDOC);
    EBOOL ret;
    
    if(POINTER_IS_INVALID(hNetServer) || POINTER_IS_INVALID(hNetPackage))
    {
        ret = EGG_NET_IVDHANDLE;
        lp_res_package = eggNetPackage_add(lp_res_package, &ret, sizeof(ret), EGG_PACKAGE_RET);
        return lp_res_package;
    }
    
    HEGGNETUNITPACKAGE lp_unit_package = (HEGGNETUNITPACKAGE)(hNetPackage + 1);
    EGGDID n_doc_id = *(EGGDID*)(lp_unit_package + 1);
    HEGGDOCUMENT p_document = EGG_NULL;
    
    
    ret = eggIndexReader_get_document(hNetServer->hReader, n_doc_id, &p_document);
    lp_res_package = eggNetPackage_add(lp_res_package, &ret, sizeof(ret), EGG_PACKAGE_RET);
    
    if(p_document)
    {
        HEGGDOCNODE lp_doc_node = eggDocument_serialization(p_document);
        
        lp_res_package = eggNetPackage_add(lp_res_package, lp_doc_node, lp_doc_node->size, EGG_PACKAGE_DOC);
        
        free(lp_doc_node);
        eggDocument_delete(p_document);
    }

    return lp_res_package;
}

PRIVATE HEGGNETPACKAGE eggNetServer_get_documentSet(HEGGNETSERVER hNetServer, HEGGNETPACKAGE hNetPackage)
{
    
    HEGGNETPACKAGE lp_res_package = eggNetPackage_new(EGG_PACKAGE_GETDOCSET);
    EBOOL ret;
    
    if(POINTER_IS_INVALID(hNetServer) || POINTER_IS_INVALID(hNetPackage))
    {
        ret = EGG_NET_IVDHANDLE;
        lp_res_package = eggNetPackage_add(lp_res_package, &ret, sizeof(ret), EGG_PACKAGE_RET);
        return lp_res_package;
    }
    
    HEGGNETUNITPACKAGE lp_unit_package = (HEGGNETUNITPACKAGE)(hNetPackage + 1);
    char* lp_net_res_org = (char*)malloc(lp_unit_package->size);
    char* lp_net_res = lp_net_res_org;
    memcpy(lp_net_res_org, lp_unit_package + 1, lp_unit_package->size);
    
    count_t n_scoreDoc_cnt = *((count_t*)(lp_net_res));
    lp_net_res += sizeof(count_t);
    HEGGSCOREDOC lp_score_doc = lp_net_res;

    ret = EGG_TRUE;
    lp_res_package = eggNetPackage_add(lp_res_package, &ret, sizeof(ret), EGG_PACKAGE_RET);

    index_t n_scoreDoc_idx = 0;
    while(n_scoreDoc_idx < n_scoreDoc_cnt)
    {
        HEGGDOCUMENT p_document = EGG_NULL;
        HEGGDOCNODE lp_doc_node = EGG_NULL;
        
        ret = eggIndexReader_get_document(hNetServer->hReader, EGGSCOREDOC_ID_I(lp_score_doc, n_scoreDoc_idx), &p_document);
        
        if(ret)
        {
            lp_doc_node = eggDocument_serialization(p_document);
            lp_res_package = eggNetPackage_add(lp_res_package, lp_doc_node, lp_doc_node->size, EGG_PACKAGE_DOC);
            free(lp_doc_node);
            eggDocument_delete(p_document);
        }
        else
        {
            lp_res_package = eggNetPackage_add(lp_res_package, lp_doc_node, lp_doc_node->size, EGG_PACKAGE_DOC);
        }
        n_scoreDoc_idx++;
    }
    
    free(lp_net_res_org);

    return lp_res_package;
}



PRIVATE HEGGNETPACKAGE eggNetServer_export_document(HEGGNETSERVER hNetServer, HEGGNETPACKAGE hNetPackage)
{
    
    HEGGNETPACKAGE lp_res_package = eggNetPackage_new(EGG_PACKAGE_EXPORTDOC);
    EBOOL ret;
    
    if(POINTER_IS_INVALID(hNetServer) || POINTER_IS_INVALID(hNetPackage))
    {
        ret = EGG_NET_IVDHANDLE;
        lp_res_package = eggNetPackage_add(lp_res_package, &ret, sizeof(ret), EGG_PACKAGE_RET);
        return lp_res_package;
    }
    
    HEGGNETUNITPACKAGE lp_unit_package = (HEGGNETUNITPACKAGE)(hNetPackage + 1);
    did_t n_doc_id = *(did_t*)(lp_unit_package + 1);
    
    HEGGDOCUMENT p_document = eggIndexReader_export_document(hNetServer->hReader, &n_doc_id);
    

    
    if(p_document)
    {
        ret = EGG_TRUE;
        lp_res_package = eggNetPackage_add(lp_res_package, &ret, sizeof(ret), EGG_PACKAGE_RET);
        lp_res_package = eggNetPackage_add(lp_res_package, &n_doc_id, sizeof(n_doc_id), EGG_PACKAGE_ID);
        
        HEGGDOCNODE lp_doc_node = eggDocument_serialization(p_document);
        
        lp_res_package = eggNetPackage_add(lp_res_package, lp_doc_node, lp_doc_node->size, EGG_PACKAGE_DOC);
        
        free(lp_doc_node);
        eggDocument_delete(p_document);

    }
    else
    {
        ret = EGG_FALSE;
        lp_res_package = eggNetPackage_add(lp_res_package, &ret, sizeof(ret), EGG_PACKAGE_RET);
        
    }
    return lp_res_package;

}

PRIVATE HEGGNETPACKAGE eggNetServer_get_docTotalCnt(HEGGNETSERVER hNetServer, HEGGNETPACKAGE hNetPackage)
{
    
    HEGGNETPACKAGE lp_res_package = eggNetPackage_new(EGG_PACKAGE_GETDOCTOTALCNT);
    EBOOL ret;    
    if(POINTER_IS_INVALID(hNetServer) || POINTER_IS_INVALID(hNetPackage))
    {
        ret = EGG_NET_IVDHANDLE;
        lp_res_package = eggNetPackage_add(lp_res_package, &ret, sizeof(ret), EGG_PACKAGE_RET);
        return lp_res_package;
    }
    ret = EGG_TRUE;
    lp_res_package = eggNetPackage_add(lp_res_package, &ret, sizeof(ret), EGG_PACKAGE_RET);
    count_t count = 0;
    count = eggIndexReader_get_doctotalcnt(hNetServer->hReader);
    lp_res_package = eggNetPackage_add(lp_res_package, &count, sizeof(count), EGG_PACKAGE_COUNT);

    
    return lp_res_package;
    
}

PRIVATE HEGGNETPACKAGE eggNetServer_get_fieldNameInfo(HEGGNETSERVER hNetServer, HEGGNETPACKAGE hNetPackage)
{
    
    HEGGNETPACKAGE lp_res_package = eggNetPackage_new(EGG_PACKAGE_GETFIELDNAMEINFO);
    EBOOL ret;    
    if(POINTER_IS_INVALID(hNetServer) || POINTER_IS_INVALID(hNetPackage))
    {
        ret = EGG_NET_IVDHANDLE;
        lp_res_package = eggNetPackage_add(lp_res_package, &ret, sizeof(ret), EGG_PACKAGE_RET);
        return lp_res_package;
    }

    HEGGFIELDNAMEINFO hFieldNameInfo;
    count_t cntFieldNameInfo;
    ret = eggIndexReader_get_fieldnameinfo(hNetServer->hReader, &hFieldNameInfo, &cntFieldNameInfo);
    
    if(ret == EGG_TRUE)
    {
        lp_res_package = eggNetPackage_add(lp_res_package, &ret, sizeof(ret), EGG_PACKAGE_RET);

        char *pNameInfo = NULL;
        size32_t szNameInfo = 0;
        pNameInfo = eggFieldView_serialise_fieldnameinfo(hFieldNameInfo,
                                                         cntFieldNameInfo,
                                                         &szNameInfo);
        
        lp_res_package = eggNetPackage_add(lp_res_package, pNameInfo, szNameInfo, EGG_PACKAGE_FIELDNAMEINFO);
        
        free(pNameInfo);
        eggFieldView_delete_fieldnameinfo(hFieldNameInfo, cntFieldNameInfo);

    }
    else
    {
        lp_res_package = eggNetPackage_add(lp_res_package, &ret, sizeof(ret), EGG_PACKAGE_RET);
        
    }
    
    return lp_res_package;
}

PRIVATE HEGGNETPACKAGE eggNetServer_get_singleFieldNameInfo(HEGGNETSERVER hNetServer, HEGGNETPACKAGE hNetPackage)
{
    
    HEGGNETPACKAGE lp_res_package = eggNetPackage_new(EGG_PACKAGE_GETFIELDNAMEINFO);
    EBOOL ret;    
    if(POINTER_IS_INVALID(hNetServer) || POINTER_IS_INVALID(hNetPackage))
    {
        ret = EGG_NET_IVDHANDLE;
        lp_res_package = eggNetPackage_add(lp_res_package, &ret, sizeof(ret), EGG_PACKAGE_RET);
        return lp_res_package;
    }

    char *fieldName = NULL;
    size32_t n_fieldName_sz = 0;
    eggNetPackage_fetch(hNetPackage, 2, &fieldName, &n_fieldName_sz);
    
    HEGGFIELDNAMEINFO hFieldNameInfo;
    ret = eggIndexReader_get_singlefieldnameinfo(hNetServer->hReader, fieldName, &hFieldNameInfo);
    
    if(ret == EGG_TRUE)
    {
        lp_res_package = eggNetPackage_add(lp_res_package, &ret, sizeof(ret), EGG_PACKAGE_RET);

        char *pNameInfo = NULL;
        size32_t szNameInfo = 0;
        pNameInfo = eggFieldView_serialise_fieldnameinfo(hFieldNameInfo,
                                                         1,
                                                         &szNameInfo);
        
        lp_res_package = eggNetPackage_add(lp_res_package, pNameInfo, szNameInfo, EGG_PACKAGE_FIELDNAMEINFO);
        
        free(pNameInfo);
        eggFieldView_delete_fieldnameinfo(hFieldNameInfo, 1);

    }
    else
    {
        lp_res_package = eggNetPackage_add(lp_res_package, &ret, sizeof(ret), EGG_PACKAGE_RET);
        
    }
    return lp_res_package;
}


PRIVATE HEGGNETPACKAGE eggNetServer_add_field(HEGGNETSERVER hNetServer, HEGGNETPACKAGE hNetPackage)
{
    HEGGNETPACKAGE lp_res_package = eggNetPackage_new(EGG_PACKAGE_GETFIELDNAMEINFO);
    EBOOL ret;    
    if(POINTER_IS_INVALID(hNetServer) || POINTER_IS_INVALID(hNetPackage))
    {
        ret = EGG_NET_IVDHANDLE;
        lp_res_package = eggNetPackage_add(lp_res_package, &ret, sizeof(ret), EGG_PACKAGE_RET);
        return lp_res_package;
    }

    char *fieldName = NULL;
    size32_t n_fieldName_sz = 0;
    type_t *p_fieldType = 0;
    size32_t n_fieldType_sz = 0;
    char *analyzerName = NULL;
    size32_t n_analyzerName_sz = 0;
    eggNetPackage_fetch(hNetPackage, 6, &fieldName, &n_fieldName_sz,
                        &p_fieldType, &n_fieldType_sz,
                        &analyzerName, &n_analyzerName_sz);
    if (*p_fieldType & EGG_OTHER_ANALYZED)
    {
        ret = eggIndexWriter_add_field(hNetServer->hWriter, fieldName, *p_fieldType, analyzerName);
    }
    else
    {
        ret = eggIndexWriter_add_field(hNetServer->hWriter, fieldName, *p_fieldType);
    }
    
    lp_res_package = eggNetPackage_add(lp_res_package, &ret, sizeof(ret), EGG_PACKAGE_RET);
    

    return lp_res_package;
}

PRIVATE HEGGNETPACKAGE eggNetServer_modify_field(HEGGNETSERVER hNetServer, HEGGNETPACKAGE hNetPackage)
{
    HEGGNETPACKAGE lp_res_package = eggNetPackage_new(EGG_PACKAGE_GETFIELDNAMEINFO);
    EBOOL ret;    
    if(POINTER_IS_INVALID(hNetServer) || POINTER_IS_INVALID(hNetPackage))
    {
        ret = EGG_NET_IVDHANDLE;
        lp_res_package = eggNetPackage_add(lp_res_package, &ret, sizeof(ret), EGG_PACKAGE_RET);
        return lp_res_package;
    }

    char *oldFieldName = NULL;
    size32_t n_oldFieldName_sz = 0;
    char *fieldName = NULL;
    size32_t n_fieldName_sz = 0;
    type_t *p_fieldType = 0;
    size32_t n_fieldType_sz = 0;
    char *analyzerName = NULL;
    size32_t n_analyzerName_sz = 0;
    eggNetPackage_fetch(hNetPackage, 6, &oldFieldName, &n_oldFieldName_sz,
                        &fieldName, &n_fieldName_sz,
                        &p_fieldType, &n_fieldType_sz,
                        &analyzerName, &n_analyzerName_sz);
    if (*p_fieldType & EGG_OTHER_ANALYZED)
    {
        ret = eggIndexWriter_modify_field(hNetServer->hWriter, oldFieldName,
                                          fieldName, *p_fieldType, analyzerName);
    }
    else
    {
        ret = eggIndexWriter_modify_field(hNetServer->hWriter, oldFieldName,
                                          fieldName, *p_fieldType);
    }
    
    lp_res_package = eggNetPackage_add(lp_res_package, &ret, sizeof(ret), EGG_PACKAGE_RET);
    
    return lp_res_package;
}

PRIVATE HEGGNETPACKAGE eggNetServer_delete_field(HEGGNETSERVER hNetServer, HEGGNETPACKAGE hNetPackage)
{
    HEGGNETPACKAGE lp_res_package = eggNetPackage_new(EGG_PACKAGE_GETFIELDNAMEINFO);
    EBOOL ret;    
    if(POINTER_IS_INVALID(hNetServer) || POINTER_IS_INVALID(hNetPackage))
    {
        ret = EGG_NET_IVDHANDLE;
        lp_res_package = eggNetPackage_add(lp_res_package, &ret, sizeof(ret), EGG_PACKAGE_RET);
        return lp_res_package;
    }

    char *fieldName = NULL;
    size32_t n_fieldName_sz = 0;
    eggNetPackage_fetch(hNetPackage, 2, &fieldName, &n_fieldName_sz);
    
    ret = eggIndexWriter_delete_field(hNetServer->hWriter, fieldName);
    
    lp_res_package = eggNetPackage_add(lp_res_package, &ret, sizeof(ret), EGG_PACKAGE_RET);
    
    return lp_res_package;    
}

