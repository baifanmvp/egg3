#ifndef EGG_BAKER_MANAGER_H
#define EGG_BAKER_MANAGER_H

#include <egg3/Egg3.h>

#include <eggRWSBakerCfg.h>
typedef index_t eggBakerId_t;
typedef struct eggRWSBaker  EGGRWSBAKER; 
typedef struct eggRWSBaker* HEGGRWSBAKER;
struct eggRWSBaker
{
    index_t bakIdx;
    HEGGINDEXREADER hReader;
    HEGGINDEXWRITER hWriter;
    HEGGINDEXSEARCHER hSearcher;
    struct eggRWSBaker* next;
    
};





typedef struct eggRWSReqInfo  EGGRWSREQINFO;
typedef struct eggRWSReqInfo* HEGGRWSREQINFO;

struct eggRWSReqInfo
{
    flag_t flag;
    long timestamp;
    pthread_t thrId;
    pthread_cond_t cond;

    struct eggRWSBaker* baker;
    struct eggRWSReqInfo* next;
};




typedef struct eggRWSBakerGroup  EGGRWSBAKERGROUP; 
typedef struct eggRWSBakerGroup* HEGGRWSBAKERGROUP;

struct eggRWSBakerGroup
{
    struct eggRWSBaker *head;
    struct eggRWSBaker *tail;
    long timestamp;
    count_t vCnt;
};

typedef struct eggRWSBakerInfo  EGGRWSBAKERINFO;
typedef struct eggRWSBakerInfo* HEGGRWSBAKERINFO;

struct eggRWSBakerInfo
{
    flag_t status;
    count_t reqCnt;
    long timestamp;
    char* path;
    pthread_t thrId;

    pthread_mutex_t mutex;
    struct eggRWSBaker* baker;
    
};

typedef struct eggRWSBakerTable  EGGRWSBAKERTABLE;
typedef struct eggRWSBakerTable* HEGGRWSBAKERTABLE;

struct eggRWSBakerTable
{
    count_t aCnt;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    struct eggRWSBakerInfo** infos;
    
    index_t validGroupIdx;
    EGGRWSBAKERGROUP bakGroup[2];
};


typedef struct eggRWSReqQuene  EGGRWSREQQUENE;
typedef struct eggRWSReqQuene* HEGGRWSREQQUENE;

struct eggRWSReqQuene
{
    struct eggRWSReqInfo *head;
    struct eggRWSReqInfo *tail;
    count_t count;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
};

typedef struct eggRWSBakerManager  EGGRWSBAKERMANAGER;
typedef struct eggRWSBakerManager* HEGGRWSBAKERMANAGER;

struct eggRWSBakerManager
{
    HEGGRWSBAKERCFG hConfig;
    HEGGRWSBAKERTABLE hBakTab;
    HEGGRWSREQQUENE hReqQueue;
    
    pthread_t counterId;
    flag_t  counterStat;

    
    pthread_t workerId;
    flag_t  workerStat;

    long maxtime;
};



#define EGG_BAKER_BUSYING                      (0x0001) 
#define EGG_BAKER_READING                      (0x0002)

#define EGG_WORKER_EXEING                      (0x0001)
#define EGG_WORKER_STOP                        (0x0000)

#define EGG_COUNTER_EXEING                      (0x0001)
#define EGG_COUNTER_STOP                        (0x0000)



#define EGG_BAKER_SET_BUSY(stat)               ((stat) |= EGG_BAKER_BUSYING)
#define EGG_BAKER_SET_IDLE(stat)               ((stat) &= ~(EGG_BAKER_BUSYING))

#define EGG_BAKER_IS_BUSY(stat)           (((stat) & (EGG_BAKER_BUSYING)))

#define EGG_BAKER_IS_IDLE(stat)           (!((stat) & (EGG_BAKER_BUSYING)))


#define EGG_BAKER_SET_READING(stat)            ((stat) |= EGG_BAKER_READING)
#define EGG_BAKER_SET_WRITEING(stat)           ((stat) &= ~(EGG_BAKER_READING))

#define EGG_BAKER_IS_READING(stat)           (((stat) & (EGG_BAKER_READING)))

#define EGG_BAKER_IS_WRITEING(stat)           (!((stat) & (EGG_BAKER_READING)))


#define EGG_BAKER_RW_SWAP(stat)                ((stat & EGG_BAKER_READING) ? (stat) &= ~(EGG_BAKER_READING) : ((stat) |= EGG_BAKER_READING))


#define eggRWSBakerManager_timestamp(bakManager) ((bakManager)->maxtime)

#define eggRWSBakerInfo_timestamp(bakInfo) ((bakInfo)->timestamp)


#define EGGRWSBAKERINFO_STATUS(bakerInfo) ((bakerInfo)->status)


HEGGRWSREQINFO eggRWSReqInfo_new(pthread_t thrId, flag_t flag);

EBOOL eggRWSReqInfo_delete(HEGGRWSREQINFO hReqInfo);

HEGGRWSBAKERINFO eggRWSBakerInfo_new(char* path, long timestamp);

EBOOL eggRWSBakerInfo_delete(HEGGRWSBAKERINFO hBakerInfo);

HEGGRWSBAKERTABLE eggRWSBakerTable_new(count_t nBakerCnt);

EBOOL eggRWSBakerTable_delete(HEGGRWSBAKERTABLE hRWSBakerTable);

EBOOL eggRWSBakerTable_loadBakerInfo(HEGGRWSBAKERTABLE hRWSBakerTable, HEGGRWSBAKERINFO hRWSBakerInfo);

HEGGRWSBAKERMANAGER eggRWSBakerManager_new(HEGGRWSBAKERCFG hRWSConfig, long timestamp);
 
EBOOL eggRWSBakerManager_delete(HEGGRWSBAKERMANAGER hRWSBakerManager);

HEGGRWSBAKER eggRWSBakerManager_alloc_baker(HEGGRWSBAKERMANAGER hRWSBakerManager, pthread_t thrId);

EBOOL eggRWSBakerManager_free_baker(HEGGRWSBAKERMANAGER hRWSBakerManager, HEGGRWSBAKER hBaker);

EBOOL eggRWSBakerManager_move_baker(HEGGRWSBAKERMANAGER hRWSBakerManager );

EBOOL eggRWSBakerManager_alter_group(HEGGRWSBAKERMANAGER hRWSBakerManager, long timestamp);

EBOOL eggRWSBakerManager_getRespon(HEGGRWSBAKERMANAGER hRWSBakerManager, HEGGRWSREQINFO hRWSReqInfo);

EBOOL eggRWSBakerManager_processReq(HEGGRWSBAKERMANAGER hRWSBakerManager);

EBOOL eggRWSBakerManager_create_worker(HEGGRWSBAKERMANAGER hRWSBakerManager);

EBOOL eggRWSBakerManager_create_counter(HEGGRWSBAKERMANAGER hRWSBakerManager);

EBOOL eggRWSBakerManager_destroy_worker(HEGGRWSBAKERMANAGER hRWSBakerManager);


EBOOL eggRWSBakerManager_wait_worker(HEGGRWSBAKERMANAGER hRWSBakerManager);


EBOOL eggRWSBakerManager_moveBakersByIds(HEGGRWSBAKERMANAGER hRWSBakerManager, eggBakerId_t* ids, count_t nIdCnt);

EBOOL eggRWSBakerManager_waitWBakers(HEGGRWSBAKERMANAGER hRWSBakerManager);

HEGGRWSBAKERGROUP eggRWSBakerManager_getWGroup(HEGGRWSBAKERMANAGER hRWSBakerManager);

HEGGRWSBAKERGROUP eggRWSBakerManager_getRGroup(HEGGRWSBAKERMANAGER hRWSBakerManager);

EBOOL  eggRWSBakerManager_WToR_Group(HEGGRWSBAKERMANAGER hRWSBakerManager);


HEGGRWSBAKERINFO* eggRWSBakerManager_getBakerInfo(HEGGRWSBAKERMANAGER hRWSBakerManager, count_t* pInfoCnt);

/*
  // get baker 1, 2, 3
  eggRWSBakerManager_moveBakersByIds(HEGGRWSBAKERMANAGER hRWSBakerManager, eggBakerId_t* ids, count_t nIdCnt);
  eggRWSBakerManager_waitWBakers(HEGGRWSBAKERMANAGER hRWSBakerManager);
  lp_bakers = eggRWSBakerManager_getWGroup(HEGGRWSBAKERMANAGER hRWSBakerManager);
  //export lp_bakers 1, 2, 3

  eggRWSBakerManager_alter_group(HEGGRWSBAKERMANAGER hRWSBakerManager, long timestamp);
  eggRWSBakerManager_waitWBakers(HEGGRWSBAKERMANAGER hRWSBakerManager);
  lp_bakers = eggRWSBakerManager_getWGroup(HEGGRWSBAKERMANAGER hRWSBakerManager);

  eggRWSBakerManager_WToR_Group(HEGGRWSBAKERMANAGER hRWSBakerManager);
  //export lp_bakers 4, 5, 6
  
  
 */
#endif 
