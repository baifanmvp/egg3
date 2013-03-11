#ifndef _EGG_IDNODE_H
#define _EGG_IDNODE_H

#include "../EggDef.h"
#include "../uti/Utility.h"
#define EGG_POS_COUNT (5)
#define EGG_POS_SPACE (EGG_POS_COUNT * sizeof(size16_t))

#define EGG_IDNODE_VALID (1)
#define EGG_IDNODE_INVALID (0)

#define EGG_IDTIMESTAMP_BASEVAL (64)

typedef u8 eggBoost_t;

typedef  struct eggIdNode EGGIDNODE;
typedef  struct eggIdNode* HEGGIDNODE;

#pragma pack(push)
#pragma pack(4)
struct eggIdNode
{
    did_t id;
    int weight;
    size16_t keyCnt;
    size16_t mask; //field mask
    u32 timeStamp;
    u8 flag;
    eggBoost_t boost;
    u8 reservd[2];
    char pos[EGG_POS_SPACE];
};
#pragma pack(pop)

#define EGGIDNODE_ID(hEggIdNode) ((hEggIdNode)->id)
#define EGGIDNODE_POS(hEggIdNode) ((hEggIdNode)->pos)
#define EGGIDNODE_WEIGHT(hEggIdNode) ((hEggIdNode)->weight)
#define EGGIDNODE_TIMESTAMP(hEggIdNode) ((hEggIdNode)->timeStamp)
#define EGGIDNODE_BOOST(hEggIdNode) ((hEggIdNode)->boost)

#define EGGIDNODE_EQUALID_CNT(hIdNode, idx, cnt)            \
    while((hIdNode)[idx].id == (hIdNode)[(idx)+(cnt)].id)   \
    {                                                       \
        (cnt)++;                                            \
    }

#define EGGIDNODE_EQUALMASK_CNT(hIdNode, idx, cnt, limitCnt)             \
    while((hIdNode)[idx].mask == (hIdNode)[(idx)+(cnt)].mask)   \
    {                                                       \
        (cnt)++;                                            \
        if(limitCnt == cnt)                                 \
        {                                                   \
            break;                                          \
        }                                                   \
    }

typedef  struct eggIdNodePos EGGIDNODEPOS;
typedef  struct eggIdNodePos* HEGGIDNODEPOS;

struct eggIdNodePos
{
    size16_t cnt;
};

#define EGGIDNODEPOS_CNT(hEggIdNodePos) ((hEggIdNodePos)->cnt)
#define EGGIDNODEPOS_DAT(hEggIdNodePos) ((hEggIdNodePos) + 1)

HEGGIDNODE eggIdNode_merge_or(HEGGIDNODE hSrcIdNodes, HEGGIDNODE hDestIdNodes, count_t srcCnt, count_t destCnt, count_t* pResCnt);

HEGGIDNODE eggIdNode_merge_and(HEGGIDNODE hSrcIdNodes, HEGGIDNODE hDestIdNodes, count_t srcCnt, count_t destCnt, count_t* pResCnt);

HEGGIDNODE eggIdNode_merge_minus(HEGGIDNODE hSrcIdNodes, HEGGIDNODE hDestIdNodes, count_t srcCnt, count_t destCnt, count_t* pResCnt);

int eggIdNode_cmp_id(HEGGIDNODE hSrcIdNodes, HEGGIDNODE hDestIdNodes);

int eggIdNode_cmp_id2(HEGGIDNODE hSrcIdNodes, index_t idx, HEGGIDNODE hDestIdNodes);

int eggIdNode_cmp_id3(HEGGIDNODE hSrcIdNodes, HEGGIDNODE hDestIdNodes);

int eggIdNode_cmp_mask(HEGGIDNODE hSrcIdNodes, HEGGIDNODE hDestIdNodes);
int eggIdNode_cmp_weight(HEGGIDNODE hSrcIdNode, HEGGIDNODE hDestIdNode);
int eggIdNode_cmp_weight2(HEGGIDNODE hSrcIdNode, index_t idx, HEGGIDNODE hDestIdNode);

EBOOL eggIdNode_filter_by_score(HEGGIDNODE hIdNodes, count_t* pIdCnt);

EBOOL eggIdNode_filter_by_continuation(HEGGIDNODE hIdNodes, count_t* pIdCnt);

EBOOL eggIdNode_set_timestamp(HEGGIDNODE hIdNodes, count_t idCnt, u32 baseValue);

EBOOL eggIdNode_set_boost(HEGGIDNODE hIdNodes, count_t idCnt, eggBoost_t boost);

EBOOL eggIdNode_filter_repeat(HEGGIDNODE hIdNodes, count_t* lpIdCnt);

EBOOL eggIdNode_reset_timestamp(HEGGIDNODE hIdNodes, count_t idCnt);
#endif

