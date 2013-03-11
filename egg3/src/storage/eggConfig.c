#include <stdio.h>
#include <stdlib.h>
#include "eggConfig.h"
#include "eggCfg.bison.h"
extern FILE* cfgin;
extern int cfglineno;
extern int fileeof;
extern int fileline;
extern int cfgparse(void*);

HEGGCONFIG g_config_handle = EGG_NULL;
eggConfig* eggConfig_loadConfig(char* cfgPath)
{
    eggConfig* p_config = (eggConfig*)malloc(sizeof(eggConfig));
    p_config->cfgTable = g_hash_table_new(g_str_hash, g_str_equal);
    cfgin = fopen(cfgPath, "r");
    fileeof = 1;
  
    while(fileeof == 1)
    { 
        cfgparse(p_config);          
    }
    
    fclose(cfgin);
    return p_config;
}
int eggConfig_all_key(eggConfig* cfg)
{
    GList* list_key = g_hash_table_get_keys(cfg->cfgTable);
    if (list_key != 0)
    {
        GList* list_key_iter = g_list_first(list_key);
        int nkeycnt = 0;
        do {
            printf("key : [%s]\n", list_key_iter->data);
            nkeycnt++;
        } while ((list_key_iter = g_list_next(list_key_iter)) != 0);
    }
    return 1;
}



int eggConfig_delete(eggConfig* cfg)
{


    GList* list_key = g_hash_table_get_keys(cfg->cfgTable);
    GList* list_value = g_hash_table_get_values(cfg->cfgTable);
    
    if (list_key != 0)
    {
        GList* list_key_iter = g_list_first(list_key);
        int nkeycnt = 0;
        do {
            free(list_key_iter->data);
            nkeycnt++;
        } while ((list_key_iter = g_list_next(list_key_iter)) != 0);
        //printf("KEY CNT : %d\n", nkeycnt);       
        g_list_free(list_key);
    }

    if (list_value != 0)
    {
        GList* list_value_iter = g_list_first(list_value);
        do {
            g_list_free_full(list_value_iter->data, free); 
            
        } while ((list_value_iter = g_list_next(list_value_iter)) != 0);

        g_list_free(list_value);
    }
    
    g_hash_table_destroy(cfg->cfgTable);

    
    return 1;
}

int eggConfig_setCfg(eggConfig* cfg, char* tabName, char* data)
{
    char* key = strdup(tabName);
    eggCfgVal* val = (eggCfgVal*)g_hash_table_lookup(cfg->cfgTable, key);
    val = g_list_append(val, strdup(data));
    g_hash_table_insert(cfg->cfgTable, key, val);

    return 1;
}

eggCfgVal* eggConfig_getCfg(eggConfig* cfg, char* tabName)
{
    return (eggCfgVal*)g_hash_table_lookup(cfg->cfgTable, tabName);
}

EBOOL eggBaseConfig_build()
{
    g_config_handle = eggConfig_loadConfig("./egg.cfg");
    g_config_handle ?  return EGG_TRUE; : return EGG_FALSE; 
}




PUBLIC EBOOL eggBaseConfig_destroy()
{
    return eggConfig_delete(g_config_handle);
}


