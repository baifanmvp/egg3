#include "eggRWSBakerCfg.h"
#include <arpa/inet.h>
#include <ctype.h>
#include <assert.h>

    
PRIVATE char* eggRWSBakerCfg_parse(HEGGRWSBAKERCFG hConfig, char* configline);

PUBLIC HEGGRWSBAKERCFG eggRWSBakerCfg_new(char* path)
{
    if(POINTER_IS_INVALID(path))
    {
        return EGG_NULL;
    }

    HEGGRWSBAKERCFG lp_config = (HEGGRWSBAKERCFG) malloc(sizeof(EGGRWSBAKERCFG));
    
    memset(lp_config, 0, sizeof(EGGRWSBAKERCFG));
    
    lp_config->hCfg = eggConfig_load_config(path);
    if (!lp_config->hCfg)
    {
        fprintf(stderr, "eggRWSBakerCfg_new loadfile error : %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    lp_config->configPath = strdup(path);
    lp_config->bakerNames = (char**)malloc(sizeof(char*)* 1024 );
    return lp_config;
}

PUBLIC EBOOL eggRWSBakerCfg_loadFile(HEGGRWSBAKERCFG hConfig, char* RWSName)
{
    if(POINTER_IS_INVALID(hConfig))
    {
        return EGG_FALSE;
    }
    char rws_name[128] = {0};
    eggCfgVal* lp_db_list = eggConfig_get_cfg(hConfig->hCfg, "dbName");
    if(!lp_db_list)
    {
        return EGG_FALSE;
    }
    
    GList* db_list_iter = g_list_first(lp_db_list);
    do
    {
        if(RWSName == EGG_NULL || 0 == strcmp(db_list_iter->data, RWSName))
        {
            memcpy(rws_name, db_list_iter->data, strlen(db_list_iter->data));
            break;
        }
    } while ((db_list_iter = g_list_next(db_list_iter)) != 0);
    
    if(rws_name[0] == 0)
    {
        return EGG_FALSE;
    }
    
    int n_name_len = strlen(rws_name);
    eggCfgVal* lp_val_tmp = EGG_NULL;

    //ip
    memcpy(rws_name + n_name_len, EGG_BAKERCFG_TAB_IP, strlen(EGG_BAKERCFG_TAB_IP) + 1);
    lp_val_tmp = eggConfig_get_cfg(hConfig->hCfg, rws_name);
    if(lp_val_tmp)
        hConfig->ip = strdup(g_list_first(lp_val_tmp)->data);
    
    //port
    memcpy(rws_name + n_name_len, EGG_BAKERCFG_TAB_PORT, strlen(EGG_BAKERCFG_TAB_PORT) + 1);
    lp_val_tmp = eggConfig_get_cfg(hConfig->hCfg, rws_name);
    if(lp_val_tmp)
        hConfig->port = atoi(g_list_first(lp_val_tmp)->data);
    
    //connectThreadNum
    memcpy(rws_name + n_name_len, EGG_BAKERCFG_TAB_CONNECTTHREADNUM, strlen(EGG_BAKERCFG_TAB_CONNECTTHREADNUM) + 1);
    lp_val_tmp = eggConfig_get_cfg(hConfig->hCfg, rws_name);
    if(lp_val_tmp)
        hConfig->connectThreadNum = atoi(g_list_first(lp_val_tmp)->data);

    //workDir
    memcpy(rws_name + n_name_len, EGG_BAKERCFG_TAB_WORKDIR, strlen(EGG_BAKERCFG_TAB_WORKDIR) + 1);
    lp_val_tmp = eggConfig_get_cfg(hConfig->hCfg, rws_name);
    if(lp_val_tmp)
        hConfig->workDir = strdup(g_list_first(lp_val_tmp)->data);

    //memServerExeName
    memcpy(rws_name + n_name_len, EGG_BAKERCFG_TAB_MEMSERVEREXENAME, strlen(EGG_BAKERCFG_TAB_MEMSERVEREXENAME) + 1);
    lp_val_tmp = eggConfig_get_cfg(hConfig->hCfg, rws_name);
    if(lp_val_tmp)
        hConfig->memServerExeName = strdup(g_list_first(lp_val_tmp)->data);

    //memServerAge_minutes
    memcpy(rws_name + n_name_len, EGG_BAKERCFG_TAB_MEMSERVERAGE, strlen(EGG_BAKERCFG_TAB_MEMSERVERAGE) + 1);
    lp_val_tmp = eggConfig_get_cfg(hConfig->hCfg, rws_name);
    if(lp_val_tmp)
    {

        char *p;
        long minutes;
        minutes = strtol(g_list_first(lp_val_tmp)->data, &p, 10);
        while (isspace(*p)) p++;
        
        if (*p == 'H' || *p == 'h')
        {
            minutes *= 60;
        }
        else if (*p == 'D' || *p == 'd')
        {
            minutes *= 60 * 24;
        }
        else if (isalpha(*p))
        {
            fprintf(stderr, "unknown config file setting, memserverage %c\n", *p);
        }
            
            
        hConfig->memServerAge_minutes = minutes;
    }
        


    //numMemServerMax
    memcpy(rws_name + n_name_len, EGG_BAKERCFG_TAB_NUMMEMSERVERMAX, strlen(EGG_BAKERCFG_TAB_NUMMEMSERVERMAX) + 1);
    lp_val_tmp = eggConfig_get_cfg(hConfig->hCfg, rws_name);
    if(lp_val_tmp)
        hConfig->numMemServerMax = atoi(g_list_first(lp_val_tmp)->data);

    hConfig->numMemServerMax = hConfig->numMemServerMax < 0 ? 0:hConfig->numMemServerMax ;

    //documentExportExeName
    memcpy(rws_name + n_name_len, EGG_BAKERCFG_TAB_DOCEXPORTEXENAME, strlen(EGG_BAKERCFG_TAB_DOCEXPORTEXENAME) + 1);
    lp_val_tmp = eggConfig_get_cfg(hConfig->hCfg, rws_name);
    if(lp_val_tmp)
        hConfig->documentExportExeName = strdup(g_list_first(lp_val_tmp)->data);

    //noWaitCleanUpMemServer
    memcpy(rws_name + n_name_len,  EGG_BAKERCFG_TAB_NOWAITCLEANUPMEMSERVER, strlen(EGG_BAKERCFG_TAB_NOWAITCLEANUPMEMSERVER) + 1);
    lp_val_tmp = eggConfig_get_cfg(hConfig->hCfg, rws_name);
    if(lp_val_tmp)
    {
        if(strcmp(g_list_first(lp_val_tmp)->data, "yes") == 0)
        {
            hConfig->noWaitCleanUpMemServer = EGG_TRUE;
        }
        else
        {
            hConfig->noWaitCleanUpMemServer = EGG_FALSE;
        }
    }

    //counterStatus
    memcpy(rws_name + n_name_len,  EGG_BAKERCFG_TAB_COUNTER, strlen(EGG_BAKERCFG_TAB_COUNTER) + 1);
    lp_val_tmp = eggConfig_get_cfg(hConfig->hCfg, rws_name);
    if(lp_val_tmp)
    {
        if(strcmp(g_list_first(lp_val_tmp)->data, "yes") == 0)
        {
            hConfig->counterStatus = EGG_TRUE;
        }
        else
        {
            hConfig->counterStatus = EGG_FALSE;
        }
    }
    
    //logFile
    memcpy(rws_name + n_name_len, EGG_BAKERCFG_TAB_LOGFILE, strlen(EGG_BAKERCFG_TAB_LOGFILE) + 1);
    lp_val_tmp = eggConfig_get_cfg(hConfig->hCfg, rws_name);
    if(lp_val_tmp)
        hConfig->logFile = strdup(g_list_first(lp_val_tmp)->data);

    //bakeraddr
    memcpy(rws_name + n_name_len, EGG_BAKERCFG_TAB_BAKERADDR, strlen(EGG_BAKERCFG_TAB_BAKERADDR) + 1);
    lp_val_tmp = eggConfig_get_cfg(hConfig->hCfg, rws_name);
    
    if(lp_val_tmp)
    {
        lp_val_tmp = g_list_first(lp_val_tmp);
        do
        {
            hConfig->bakerNames[hConfig->bakerCnt++] = strdup(lp_val_tmp->data);
        } while ((lp_val_tmp = g_list_next(lp_val_tmp)) != 0);
    
    }

    return EGG_TRUE;

}
PUBLIC char** eggRWSBakerCfg_get_bakerNames(HEGGRWSBAKERCFG hConfig, count_t* pCntName)
{
    if(POINTER_IS_INVALID(hConfig))
    {
        return EGG_NULL;
    }
    if(hConfig->bakerCnt == 0)
    {
        return EGG_NULL;
    }
    
    *pCntName = hConfig->bakerCnt;
    return hConfig->bakerNames;
}

PUBLIC EBOOL eggRWSBakerCfg_delete(HEGGRWSBAKERCFG hConfig)
{
    if(POINTER_IS_INVALID(hConfig))
    {
        return EGG_FALSE;
    }
    free(hConfig->configPath);
    while(hConfig->bakerCnt--)
    {
        free(hConfig->bakerNames[hConfig->bakerCnt]);
    }
    eggConfig_delete(hConfig->hCfg);
    free(hConfig->bakerNames);
    free(hConfig->workDir);
    free(hConfig->memServerExeName);
    free(hConfig->documentExportExeName);
    free(hConfig->logFile);
    free(hConfig->ip);
    free(hConfig);

    return EGG_TRUE;
}

