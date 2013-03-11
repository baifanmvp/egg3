#ifndef EGG_RWS_BAKER_CFG_H
#define EGG_RWS_BAKER_CFG_H
#include "egg3/Egg3.h"
#include "egg3/conf/eggConfig.h"

#define EGG_BAKERCFG_SYNTAXERR               0
#define EGG_BAKERCFG_NOR                     1
#define EGG_BAKERCFG_SPACELINE               2
#define EGG_BAKERCFG_NOTE                    3
#define EGG_BAKERCFG_PNULL                   4
#define EGG_BAKERCFG_ADDRERR                 5
#define EGG_BAKERCFG_QUEUEDEPERR             6
#define EGG_BAKERCFG_INVALID                 7

#define EGG_BAKERCFG_TYPE_BAKERADDR          20
#define EGG_BAKERCFG_TYPE_QUEUEDEP           21
#define EGG_BAKERCFG_TYPE_WORKDIR            22
#define EGG_BAKERCFG_TYPE_IP                 23
#define EGG_BAKERCFG_TYPE_PORT               24
#define EGG_BAKERCFG_TYPE_MEMSERVEREXENAME   25
#define EGG_BAKERCFG_TYPE_LOGFILE            26
#define EGG_BAKERCFG_TYPE_DOCUMENTEXPORTEXENAME   27
#define EGG_BAKERCFG_TYPE_COUNTER                 28
#define EGG_BAKERCFG_TYPE_MEMSERVERAGE            29
#define EGG_BAKERCFG_TYPE_NOWAITCLEANUPMEMSERVER  30
#define EGG_BAKERCFG_TYPE_CONNECTTHREADNUM        31
#define EGG_BAKERCFG_TYPE_NUMMEMSERVERMAX         32

#define EGG_BAKERCFG_MAX_QUEUEDEP            1024


#define EGG_BAKERCFG_TAB_IP                           ":ip"
#define EGG_BAKERCFG_TAB_PORT                         ":port"
#define EGG_BAKERCFG_TAB_CONNECTTHREADNUM             ":connectthreadnum"
#define EGG_BAKERCFG_TAB_LOGFILE                      ":logfile"
#define EGG_BAKERCFG_TAB_WORKDIR                      ":workdir"
#define EGG_BAKERCFG_TAB_MEMSERVEREXENAME             ":memserverexename"
#define EGG_BAKERCFG_TAB_MEMSERVERAGE                 ":memserverage"
#define EGG_BAKERCFG_TAB_DOCEXPORTEXENAME             ":docexportexename"
#define EGG_BAKERCFG_TAB_COUNTER                      ":counter"
#define EGG_BAKERCFG_TAB_BAKERADDR                    ":eggpath"
#define EGG_BAKERCFG_TAB_NUMMEMSERVERMAX              ":nummemservermax"
#define EGG_BAKERCFG_TAB_NOWAITCLEANUPMEMSERVER       ":nowaitcleanupmemserver"


typedef  struct eggRWSBakerCfg   EGGRWSBAKERCFG;
typedef  struct eggRWSBakerCfg*  HEGGRWSBAKERCFG;

struct eggRWSBakerCfg
{
    char* configPath;
    HEGGCONFIG hCfg;
    char* ip;
    int port;
    int connectThreadNum;
    char* workDir;
    char* memServerExeName;
    int memServerAge_minutes;
    int numMemServerMax;    
    char* documentExportExeName;
    EBOOL noWaitCleanUpMemServer;
    char *logFile;
    char** bakerNames;
    count_t bakerCnt;
    EBOOL counterStatus;
};
HEGGRWSBAKERCFG eggRWSBakerCfg_new(char* path);

EBOOL eggRWSBakerCfg_loadFile(HEGGRWSBAKERCFG hConfig, char* RWSName);

EBOOL eggRWSBakerCfg_delete(HEGGRWSBAKERCFG hConfig);

PUBLIC char** eggRWSBakerCfg_get_bakerNames(HEGGRWSBAKERCFG hConfig, count_t* pCntName);

#endif
