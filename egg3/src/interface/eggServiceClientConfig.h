#ifndef EGGSERVICECLIENTCONFIG_H_
#define EGGSERVICECLIENTCONFIG_H_
#include "../EggDef.h"

typedef struct eggServiceClientConfig eggServiceClientConfig;
typedef struct eggServiceClientConfig *HEGGSERVICECLIENTCONFIG;

struct eggServiceClientConfig {
    char *socketfile;
    unsigned short port;
    char *ip;
};

HEGGSERVICECLIENTCONFIG eggServiceClientConfig_new(char *configfile);

HEGGSERVICECLIENTCONFIG eggServiceClientConfig_new_default(void);

PUBLIC EBOOL eggServiceClientConfig_delete(HEGGSERVICECLIENTCONFIG hEggServiceClientConfig);
#endif
