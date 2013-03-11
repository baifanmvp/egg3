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
eggConfig* eggConfig_load_config(char* cfgPath)
{
    eggConfig* p_config = (eggConfig*)malloc(sizeof(eggConfig));
    p_config->cfgTable = g_hash_table_new(g_str_hash, g_str_equal);
    cfgin = fopen(cfgPath, "r");
    fileeof = 1;
    fileline = 0;
    
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
//            printf("key : [%s]\n", list_key_iter->data);
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
            //printf("key : %s\n", list_key_iter->data);
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
            GList* list_data_iter = g_list_first(list_value_iter->data);
            do {
                free(list_data_iter->data);
            } while ((list_data_iter = g_list_next(list_data_iter)) != 0);

            g_list_free(list_value_iter->data); 
            
        } while ((list_value_iter = g_list_next(list_value_iter)) != 0);

        g_list_free(list_value);
    }
    
    g_hash_table_destroy(cfg->cfgTable);

    free(cfg);
    return 1;
}

int eggConfig_set_cfg(eggConfig* cfg, char* tabName, char* data)
{
    
    char* key = tabName;//strdup(tabName);
    eggCfgVal* val = (eggCfgVal*)g_hash_table_lookup(cfg->cfgTable, key);
    if(!val)
    {
        key = strdup(tabName);
//        printf("strdup : %s\n", key);
    }
    val = g_list_append(val, strdup(data));
    g_hash_table_insert(cfg->cfgTable, key, val);
    
    return 1;
}

eggCfgVal* eggConfig_get_cfg(eggConfig* cfg, char* tabName)
{
    return (eggCfgVal*)g_hash_table_lookup(cfg->cfgTable, tabName);
}

EBOOL eggBaseConfig_build()
{
    g_config_handle = eggConfig_load_config(EGG_CONFIG_PATH);
    
    if(g_config_handle)
        return EGG_TRUE;
    else
        return EGG_FALSE;
    
}




PUBLIC EBOOL eggBaseConfig_destroy()
{
    return eggConfig_delete(g_config_handle);
}


