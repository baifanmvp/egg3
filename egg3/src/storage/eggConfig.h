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

eggConfig* eggConfig_loadConfig(char* cfgPath);

int eggConfig_delete(eggConfig* cfg);

int eggConfig_setCfg(eggConfig* cfg, char* tabName, char* data);

eggCfgVal* eggConfig_getCfg(eggConfig* cfg, char* tabName);



EBOOL eggBaseConfig_build();




PUBLIC EBOOL eggBaseConfig_destroy();

#endif
