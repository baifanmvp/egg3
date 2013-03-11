#include "../eggAnalyzer.h"
#include "eggPath.h"
#include "eggIndexReader.h"
#include "eggIndexWriter.h"
#include "eggIndexSearcher.h"
#include "../log/eggPrtLog.h"

#include <dlfcn.h>
#include <pthread.h>
/*
  field1 : typeInfo
  field2 : analyName
  field3 : dictName
  field4 : dictKey
  
  example :
  doc1 "dict" "ImCwsLexAnalyzer" "xuzhou_cws" "111222" 
  doc2 "dict" "ImCwsLexAnalyzer" "shanghai_cws" "111222"

  //add
  eggField_new(..., "xuzhou_cws")
  
  //search
  eggQuery_new_string(..., "xuzhou_cws")

*/
HEGGANALYZERSET g_analyzerSet_handle = EGG_NULL;
extern HEGGCONFIG g_config_handle;

PRIVATE EBOOL eggAnalyzer_add_newAnaly(HEGGANALYZERSET hAnalyzerSet, char* analyName);

PRIVATE EBOOL eggAnalyzerSet_add_analyUnit (HEGGANALYZERSET hAnalyzerSet, char* analyzerName, char* laPath);

PRIVATE char* eggAnalyzerSet_get_realdll(char* laPath);


PUBLIC EBOOL EGGAPI eggAnalyzerSet_build()
{
    if(!POINTER_IS_INVALID(g_analyzerSet_handle))
    {
        return EGG_FALSE;
    }
    
    g_analyzerSet_handle = (HEGGANALYZERSET)malloc(sizeof(EGGANALYZERSET));
    memset(g_analyzerSet_handle, 0, sizeof(EGGANALYZERSET));
    pthread_mutex_init( &g_analyzerSet_handle->mutex, NULL);
    g_analyzerSet_handle->time = time(0);
//    signal(SIGALRM, eggAnalyzerSet_update_dict);
    return EGG_TRUE;
}


PUBLIC EBOOL EGGAPI eggAnalyzerSet_destroy()
{
    if(POINTER_IS_INVALID(g_analyzerSet_handle))
    {
        return EGG_FALSE;
    }
        
    while(g_analyzerSet_handle->cnt --)
    {
        g_analyzerSet_handle->pUnitSet[g_analyzerSet_handle->cnt].fnDelete(g_analyzerSet_handle->pUnitSet[g_analyzerSet_handle->cnt].pHandle);
        free(g_analyzerSet_handle->pUnitSet[g_analyzerSet_handle->cnt].pName);
    }
    
    pthread_mutex_destroy(&g_analyzerSet_handle->mutex);
    
    free(g_analyzerSet_handle);
    g_analyzerSet_handle = EGG_NULL;
    
    return EGG_TRUE;
}

PUBLIC  P_NEW_BLOCK_ITEM eggAnalyzer_get_dictlist(char *analyzerName)
{
    if(!analyzerName)
    {
        return EGG_NULL;
    }
    
    P_NEW_BLOCK_ITEM pBlockItem = NULL;
    
    HEGGTOPCOLLECTOR hTopCollector = eggSySRecorder_get_dict("dict", analyzerName);


    HEGGSCOREDOC lp_score_doc = eggTopCollector_top_docs(hTopCollector);
    count_t cnt =  eggTopCollector_total_hits(hTopCollector);
    HEGGINDEXREADER hIndexReader = eggSySRecorder_alloc_reader();
    while(cnt--)
    {
        HEGGDOCUMENT lp_eggDocument = EGG_NULL;
        eggIndexReader_get_document(hIndexReader, lp_score_doc[cnt].idDoc, &lp_eggDocument);
        char* pDict = EGG_NULL;
        char* pKey = EGG_NULL;
        
	HEGGFIELD hField1 = eggDocument_get_field(lp_eggDocument, EGG_SYS_DICTNAME);
	HEGGFIELD hField2 = eggDocument_get_field(lp_eggDocument, EGG_SYS_DICTKEY);
	size32_t n_len1 = 0;
	size32_t n_len2 = 0;
	pDict = eggField_get_value(hField1, &n_len1);
	pKey = eggField_get_value(hField2, &n_len2);


        
        if(pDict && pKey)
        {
	  char *lp_dict_buf = strndup(pDict, n_len1);
	  char *lp_key_buf = strndup(pKey, n_len2);
//	  printf("[%s], [%s]\n", lp_dict_buf, lp_key_buf);
	  eggPrtLog_info("eggAnalyzer", "[%s], [%s]\n", lp_dict_buf, lp_key_buf);
	  pBlockItem = BlockItemPushWord(pBlockItem, lp_dict_buf, lp_key_buf, "NR", 1000000);
	  free(lp_dict_buf);
	  free(lp_key_buf);
            
        }
	//        free(pDict);
        //free(pKey);
        eggDocument_delete(lp_eggDocument);

    }

    eggTopCollector_delete(hTopCollector);
    eggSySRecorder_free_reader((void**)&hIndexReader);

    return pBlockItem;
}

#ifdef __CYGWIN__ 
extern flag_t g_eggLibLoad_cygwin;
extern pthread_mutex_t counter_mutex;

#endif


PUBLIC HEGGANALYZER EGGAPI eggAnalyzer_get(char *analyzerName)
{
    if(!analyzerName)
    {
        return EGG_NULL;
    }
    
#ifdef __CYGWIN__
    if(g_eggLibLoad_cygwin == EGG_FALSE)
    {
        pthread_mutex_lock(&counter_mutex);
        if(g_eggLibLoad_cygwin == EGG_FALSE)
        {
            eggLibLoad_init();
            g_eggLibLoad_cygwin = EGG_TRUE;
        }
        pthread_mutex_unlock(&counter_mutex);
    }
#endif

    
    int n_analyzertmp_len = strlen(analyzerName);
    if(n_analyzertmp_len > 128-1)
    {
        return EGG_NULL;
    }
    
    char analyzer_buf[128] = {0};

    memcpy(analyzer_buf, analyzerName, n_analyzertmp_len+1);
    analyzerName = analyzer_buf;

    pthread_mutex_lock(&g_analyzerSet_handle->mutex);

    eggAnalyzerSet_update_dict();
    index_t n_idx = g_analyzerSet_handle->cnt;
    int n_analyzer_name_len = strlen(analyzerName);
    
    if(! n_analyzer_name_len)
    {
        pthread_mutex_unlock(&g_analyzerSet_handle->mutex);

        return EGG_NULL;
    }
    
    while(n_idx --)
    {
        if(strlen(g_analyzerSet_handle->pUnitSet[n_idx].pName) == n_analyzer_name_len  &&
           strncmp(g_analyzerSet_handle->pUnitSet[n_idx].pName, analyzerName, n_analyzer_name_len) == 0)
        {
            pthread_mutex_unlock(&g_analyzerSet_handle->mutex);
            return g_analyzerSet_handle->pUnitSet + n_idx;
        }
    }
    
    EBOOL ret = eggAnalyzer_add_newAnaly(g_analyzerSet_handle, analyzerName);

    if(ret)
    {
        pthread_mutex_unlock(&g_analyzerSet_handle->mutex);
        return g_analyzerSet_handle->pUnitSet + (g_analyzerSet_handle->cnt -1);
    }
    else
    {
        pthread_mutex_unlock(&g_analyzerSet_handle->mutex);
        //printf("no find analyzer [file: %s line: %d ]\n", __FILE__, __LINE__);
        eggPrtLog_error("eggAnalyzer", "no find analyzer %s [file: %s line: %d ]\n", analyzerName, __FILE__, __LINE__);
        return EGG_NULL;
    }
}

PRIVATE EBOOL eggAnalyzer_add_newAnaly(HEGGANALYZERSET hAnalyzerSet, char* analyName)
{
    if(!hAnalyzerSet || !analyName)
    {
        return EGG_FALSE;
    }

    eggCfgVal* p_list_val = eggConfig_get_cfg(g_config_handle, "analyzer");

    if(p_list_val == EGG_NULL)
    {
        return EGG_FALSE;
    }
    
    GList* list_value_iter = g_list_first(p_list_val);
    do {
        char* lp_analyname = list_value_iter->data;
        list_value_iter = g_list_next(list_value_iter);
        
        char* lp_dll_path = list_value_iter->data;
        if(strcmp(lp_analyname, analyName) == 0)
        {
            return eggAnalyzerSet_add_analyUnit (hAnalyzerSet, analyName, lp_dll_path);
        }
        
        
    } while ((list_value_iter = g_list_next(list_value_iter)) != 0);
    return EGG_FALSE;
        
}

PRIVATE EBOOL eggAnalyzerSet_add_analyUnit (HEGGANALYZERSET hAnalyzerSet, char* analyzerName, char* laPath)
{

    int n_analyzerName_sz = strlen(analyzerName);
    char* realPath = EGG_NULL;
    if((realPath = eggAnalyzerSet_get_realdll(laPath))  == EGG_NULL)
    {
        return EGG_FALSE;
    }
    
    void* hdll = dlopen(realPath, RTLD_LAZY);
    free(realPath);
    if(!hdll)
    {
        return EGG_FALSE;
    }
    memcpy(analyzerName + n_analyzerName_sz, EGG_FNANALYZER_NEW, strlen(EGG_FNANALYZER_NEW)+1);
    hAnalyzerSet->pUnitSet[hAnalyzerSet->cnt].fnNew = dlsym(hdll, analyzerName);
    if(POINTER_IS_INVALID(hAnalyzerSet->pUnitSet[hAnalyzerSet->cnt].fnNew))
    {
        dlclose(hdll);
        return EGG_FALSE;
    }
    
    memcpy(analyzerName + n_analyzerName_sz, EGG_FNANALYZER_DELETE, strlen(EGG_FNANALYZER_DELETE)+1);
    hAnalyzerSet->pUnitSet[hAnalyzerSet->cnt].fnDelete = dlsym(hdll, analyzerName);
    if(POINTER_IS_INVALID(hAnalyzerSet->pUnitSet[hAnalyzerSet->cnt].fnDelete))
    {
        dlclose(hdll);
        return EGG_FALSE;
    }
    
    memcpy(analyzerName + n_analyzerName_sz, EGG_FNANALYZER_TOKENIZE, strlen(EGG_FNANALYZER_TOKENIZE)+1);
    hAnalyzerSet->pUnitSet[hAnalyzerSet->cnt].fnTokenize = dlsym(hdll, analyzerName);
    if(POINTER_IS_INVALID(hAnalyzerSet->pUnitSet[hAnalyzerSet->cnt].fnTokenize))
    {
        dlclose(hdll);
        return EGG_FALSE;
    }

    memcpy(analyzerName + n_analyzerName_sz, EGG_FNANALYZER_UPDATEWORD, strlen(EGG_FNANALYZER_UPDATEWORD)+1);
    hAnalyzerSet->pUnitSet[hAnalyzerSet->cnt].fnUpdateword = dlsym(hdll, analyzerName);
    if(POINTER_IS_INVALID(hAnalyzerSet->pUnitSet[hAnalyzerSet->cnt].fnUpdateword))
    {
        dlclose(hdll);
        return EGG_FALSE;
    }
    analyzerName[n_analyzerName_sz] = '\0';
    P_NEW_BLOCK_ITEM pBlockItem = eggAnalyzer_get_dictlist(analyzerName);
    hAnalyzerSet->pUnitSet[hAnalyzerSet->cnt].pHandle = hAnalyzerSet->pUnitSet[hAnalyzerSet->cnt].fnNew(pBlockItem);

    hAnalyzerSet->pUnitSet[hAnalyzerSet->cnt].pName = strndup(analyzerName, n_analyzerName_sz);
    
    hAnalyzerSet->cnt++;
    //    dlclose(dllpath);
    BlockItemRelease(pBlockItem);

    return EGG_TRUE;
}


PUBLIC void  eggAnalyzerSet_update_dict()
{
    if(POINTER_IS_INVALID(g_analyzerSet_handle))
    {
        return ;
    }
    if(time(0) - g_analyzerSet_handle->time < EGGANALYDICT_FLUSHTIME)
    {
//        printf("-------------------\n");
        return ;
    }
    index_t idx = g_analyzerSet_handle->cnt;
    while(idx --)
    {
        
        P_NEW_BLOCK_ITEM pBlockItem = eggAnalyzer_get_dictlist(g_analyzerSet_handle->pUnitSet[idx].pName);
        
        ImLexAnalyzer_UpdateWord((ImLexAnalyzer*)g_analyzerSet_handle->pUnitSet[idx].pHandle, pBlockItem);
        
        BlockItemRelease(pBlockItem);

 
    }

    g_analyzerSet_handle->time = time(0);
    return ;
}


PRIVATE char* eggAnalyzerSet_get_realdll(char* laPath)
{
    if (!laPath)
    {
        return EGG_NULL;
    }
    FILE* la_fp = fopen(laPath, "r");
    
    if (!la_fp)
    {
        return EGG_NULL;
    }
    char* ldline = EGG_NULL;
    size_t n_ldline_sz = 0;
    while(getline(&ldline, &n_ldline_sz, la_fp) != -1)
    {
        
        if(strncmp(ldline, "dlname='", 8) == 0)
        {
            char* lp_line_end = strchr(ldline + 8, '\'');
            *lp_line_end = '\0';
            int n_line_end_sz = strlen(ldline + 8);
            
            char* lp_lapath_end = strrchr(laPath, '/');
            int n_lapath_sz = (int)(lp_lapath_end - laPath) + 1;

            char* realLaPath = malloc(n_lapath_sz + n_line_end_sz + 1);
            memcpy(realLaPath, laPath, n_lapath_sz);
            memcpy(realLaPath + n_lapath_sz, ldline + 8, n_line_end_sz);
            realLaPath[n_lapath_sz + n_line_end_sz] = '\0';

            free(ldline);
            fclose(la_fp);
            return realLaPath;
        }
    }
    free(ldline);
    fclose(la_fp);
    return EGG_FALSE;
}



////////////////////////
////////////////////////
////////////////////////
////////////////////////
////////////////////////
