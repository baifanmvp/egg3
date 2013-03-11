#ifndef EGG_CLUSTERCOMMON_H_
#define EGG_CLUSTERCOMMON_H_
#include "../EggDef.h"
#include "../net/eggSpanUnit.h"

typedef void* HEGGMASTERHAND;


typedef void* HEGGCHUNKOBJ;

typedef struct eggChunkHand  EGGCHUNKHAND;
typedef struct eggChunkHand* HEGGCHUNKHAND;

struct eggChunkHand
{
    HEGGCHUNKOBJ hChunkObj;
    SPANPOINT start;
    SPANPOINT end;
};
#define EGG_SPANPOINT_NOT_FIND -1

PUBLIC index_t eggChunkHand_find_spanpoint(HEGGCHUNKHAND hHandSet, count_t nHandCnt, SPANPOINT curPoint);
#endif
