#ifndef EGG_FIELD_WEIGHT_H_
#define EGG_FIELD_WEIGHT_H_
#include "../EggDef.h"
#include "../eggField.h"
#include "eggFieldView.h"
#include "../storage/ViewStream.h"
E_BEGIN_DECLS

#define WEIGHT_CACHEBLOCK_LIMIT 16

#define WEIGHT_NODECOUNT_LIMIT 500000

#define WEIGHT_NODE_VALID 1
#define WEIGHT_NODE_INVALID 0

#pragma pack(push)
#pragma pack(4)

/* typedef struct eggWeightNode EGGWEIGHTNODE; */
/* typedef struct eggWeightNode* HEGGWEIGHTNODE; */

/* struct eggWeightNode */
/* { */
/*     double value; */
/* }; */

typedef  struct eggWResult EGGWRESULT;
typedef  struct eggWResult* HEGGWRESULT;

struct eggWResult
{
    did_t id;
    char val[8];
};


typedef  struct eggWNode EGGWNODE;
typedef  struct eggWNode* HEGGWNODE;

struct eggWNode
{
    flag_t flag;
    char val[8];
};


typedef struct eggWeightBlock EGGWEIGHTBLOCK;
typedef struct eggWeightBlock* HEGGWEIGHTBLOCK;

struct eggWeightBlock
{
    size64_t aCnt;
    size64_t eCnt;
    
    size64_t maxId;
    fdid_t fid;
    type_t type;
};

typedef struct eggWeightCache EGGWEIGHTCACHE;
typedef struct eggWeightCache* HEGGWEIGHTCACHE;

struct eggWeightCache
{
    HEGGWEIGHTBLOCK cacheBlock[WEIGHT_CACHEBLOCK_LIMIT];
    count_t eCnt;
};

typedef struct eggFieldWeight EGGFIELDWEIGHT;
typedef struct eggFieldWeight* HEGGFIELDWEIGHT;

struct eggFieldWeight
{
    HVIEWSTREAM hViewStream;
    HEGGWEIGHTCACHE hCache;
    HEGGFIELDVIEW hFieldView;
    pthread_mutex_t mutex;
};

#define EGGFIELDWEIGHT_IS_INVALID(hFieldWeight) \
    (!hFieldWeight ? EGG_TRUE:EGG_FALSE)


#pragma pack(pop)

HEGGFIELDWEIGHT eggFieldWeight_new(HEGGFILE hEggFile, HEGGFIELDVIEW fieldView);

EBOOL eggFieldWeight_delete(HEGGFIELDWEIGHT hFieldWeight);

EBOOL eggFieldWeight_add(HEGGFIELDWEIGHT hFieldWeight, HEGGFIELD hField, did_t id);

EBOOL eggFieldWeight_remove(HEGGFIELDWEIGHT hFieldWeight, HEGGFIELD hField, did_t id);

HEGGWRESULT eggFieldWeight_get_withname(HEGGFIELDWEIGHT hFieldWeight, char* fieldName, count_t* lpCount);

HEGGWRESULT eggFieldWeight_get_withfid(HEGGFIELDWEIGHT hFieldWeight, fdid_t fid, count_t* lpCount);

HEGGWRESULT eggFieldWeight_get_withfid(HEGGFIELDWEIGHT hFieldWeight, fdid_t fid, count_t* lpCount);

type_t eggFieldWeight_type_withname(HEGGFIELDWEIGHT hFieldWeight, char* fieldName);

EBOOL eggFieldWeight_clean_cache(HEGGFIELDWEIGHT hFieldWeight);


#define WEIGHTNODE_OFFSET(base, idx) ((base) + sizeof(EGGWEIGHTBLOCK) + (idx) * sizeof(EGGWNODE))

#define WEIGHTNODE_FULL(hBlock, id) ((hBlock)->aCnt < id)

int eggWResult_cmpval_int32_asc(HEGGWRESULT src, HEGGWRESULT des);
int eggWResult_cmpval_int32_desc(HEGGWRESULT src, HEGGWRESULT des);
int eggWResult_cmpval_int64_asc(HEGGWRESULT src, HEGGWRESULT des);
int eggWResult_cmpval_int64_desc(HEGGWRESULT src, HEGGWRESULT des);
int eggWResult_cmpval_double_asc(HEGGWRESULT src, HEGGWRESULT des);
int eggWResult_cmpval_double_desc(HEGGWRESULT src, HEGGWRESULT des);
int eggWResult_cmpid_desc(HEGGWRESULT src, HEGGWRESULT des);
int eggWResult_cmpid2_desc(HEGGWRESULT src, index_t idx, HEGGWRESULT des);

EBOOL eggFieldWeight_set_actinfo(HEGGFIELDWEIGHT hFieldWeight, ActInfo *hActInfo);
EBOOL eggFieldWeight_clean_actinfo(HEGGFIELDWEIGHT hFieldWeight, ActInfo *hActInfo);



/* for back-compliance */

#define eggFieldWeight_get_withName(hFieldWeight, fieldName, lpCount) eggFieldWeight_get_withname(hFieldWeight, fieldName, lpCount)

/* for back-compliance end */

E_END_DECLS

#endif
