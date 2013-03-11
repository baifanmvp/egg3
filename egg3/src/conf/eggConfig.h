#ifndef _EGG_CONFIG_H
#define _EGG_CONFIG_H

#include <stdio.h>
#include <glib.h>
#include <string.h>
#include "../EggDef.h"

typedef GList eggCfgVal;
    
typedef struct _eggConfig eggConfig;
typedef struct _eggConfig* HEGGCONFIG;
struct _eggConfig
{
    GHashTable* cfgTable;
    
};

eggConfig* eggConfig_load_config(char* cfgPath);

int eggConfig_delete(eggConfig* cfg);

int eggConfig_set_cfg(eggConfig* cfg, char* tabName, char* data);

eggCfgVal* eggConfig_get_cfg(eggConfig* cfg, char* tabName);



EBOOL eggBaseConfig_build();




PUBLIC EBOOL eggBaseConfig_destroy();


extern HEGGCONFIG g_config_handle;

#endif
