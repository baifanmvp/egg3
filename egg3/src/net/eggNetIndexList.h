
#ifndef EGG_NETINDEXLIST_H_
#define EGG_NETINDEXLIST_H_
#include "../eggIndexReader.h"
#include "../eggIndexWriter.h"
#include "../eggIndexSearcher.h"
#include "../eggDirectory.h"
#include "../eggAnalyzer.h"
#include "./eggNetHttp.h"
#include "./eggNetPackage.h"
#include "../EggDef.h"
#include "../eggPath.h"

#include <pthread.h>
#include <signal.h>

typedef struct eggNetIndexNode EGGNETINDEXNODE;
typedef struct eggNetIndexNode* HEGGNETINDEXNODE;
struct eggNetIndexNode
{
    HEGGINDEXSEARCHER hSearcher;
    HEGGINDEXREADER hReader;
    HEGGINDEXWRITER hWriter;
    HEGGHANDLE hHandle;
    
    char* analyzerName;
    
    HEGGNETINDEXNODE next;
};

typedef struct eggNetIndexList EGGNETINDEXLIST;
typedef struct eggNetIndexList* HEGGNETINDEXLIST;

struct eggNetIndexList
{
    HEGGNETINDEXNODE head;
    count_t cnt;
    pthread_mutex_t mutex;

};


HEGGNETINDEXLIST eggNetIndexList_new();

EBOOL eggNetIndexList_delete(HEGGNETINDEXLIST hNetIndexList);

HEGGNETINDEXNODE eggNetIndexList_create_indexhandle(HEGGNETINDEXLIST hNetIndexList, const char* path, const char* analyName);

HEGGNETINDEXNODE eggNetIndexList_find_indexhandle(HEGGNETINDEXLIST hNetIndexList, const char* path);



EBOOL eggNetIndexList_optimize(HEGGNETINDEXLIST hNetIndexList);


#endif
