#ifndef EGGSERVICECLIENTCONFIG_H_
#define EGGSERVICECLIENTCONFIG_H_
#include <egg3/EggDef.h>

typedef struct eggServiceServerConfig eggServiceServerConfig;
typedef struct eggServiceServerConfig *HEGGSERVICESERVERCONFIG;

struct eggServiceServerConfig {
    char *socketfile;
    unsigned short port;
    char *ip;
    char *logfile;
    int  loglevel;
};

HEGGSERVICESERVERCONFIG eggServiceServerConfig_new(char *configfile);

PUBLIC EBOOL eggServiceServerConfig_delete(HEGGSERVICESERVERCONFIG hEggServiceServerConfig);
#endif
