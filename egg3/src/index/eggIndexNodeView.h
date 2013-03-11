#ifndef _EGG_INDEX_NODE_VIEW_H
#define _EGG_INDEX_NODE_VIEW_H

#define IDXNODE_IS_LEAF   0
#define IDXNODE_IS_NORMAL 1
#define IDXNODE_IS_ROOT   2

#define IDXNODE_FIND 1
#define IDXNODE_NOT_FIND -1

#define IDXNODE_IS_LOCKED   (1 << 0)
#include "../egglib/emem.h"

#include "eggIndexRecord.h"
E_BEGIN_DECLS

typedef struct eggIndexLeafLst EGGINDEXLEAFLST;
typedef struct eggIndexLeafLst* HEGGINDEXLEAFLST;

#pragma pack(push)
#pragma pack(4)
struct eggIndexLeafLst
{
    offset64_t next;
    offset64_t pre;
};

typedef struct eggIndexNodeInfo EGGINDEXNODEINFO;
typedef struct eggIndexNodeInfo* HEGGINDEXNODEINFO;

struct eggIndexNodeInfo
{
    type_t type;
    size16_t rdSize;
    size16_t rdCnt;
};



typedef struct eggIndexNode EGGINDEXNODE;
typedef struct eggIndexNode* HEGGINDEXNODE;

struct eggIndexNode
{
    type_t ntype;
    size16_t rdCnt;
    size32_t flag;
    offset64_t parent;
};
#pragma pack(pop)

typedef struct eggIndexNodeView EGGINDEXNODEVIEW;
typedef struct eggIndexNodeView* HEGGINDEXNODEVIEW;

struct eggIndexNodeView
{
    offset64_t nodeOff;
    HEGGINDEXNODEINFO reInfo;
    HEGGINDEXNODE hNode;
};


HEGGINDEXNODEVIEW eggIndexNodeView_new(HEGGINDEXNODEINFO hNodeInfo, type_t type);

EBOOL eggIndexNodeView_delete(HEGGINDEXNODEVIEW hNodeView);

HEGGINDEXNODEVIEW eggIndexNodeView_split(HEGGINDEXNODEVIEW hNodeView, HEGGINDEXNODEVIEW hNewNodeView);

EBOOL eggIndexNodeView_insert(HEGGINDEXNODEVIEW hNodeView, HEGGINDEXRECORD hRecord, index_t nRdIdx);

EBOOL eggIndexNodeView_locate(HEGGINDEXNODEVIEW hNodeView, HEGGINDEXRECORD hRecord, index_t* pRdIdx);

EBOOL eggIndexNodeView_remove(HEGGINDEXNODEVIEW hNodeView, index_t index);


offset64_t eggIndexNodeView_get_preoff(HEGGINDEXNODEVIEW hNodeView);
    
EBOOL eggIndexNodeView_set_preoff(HEGGINDEXNODEVIEW hNodeView, offset64_t preOff);

offset64_t eggIndexNodeView_get_nextoff(HEGGINDEXNODEVIEW hNodeView);

EBOOL eggIndexNodeView_set_nextoff(HEGGINDEXNODEVIEW hNodeView, offset64_t nextOff);

EBOOL eggIndexNodeView_get_nodeCnt(HEGGINDEXNODEVIEW hNodeView, offset64_t preOff);

#define EGGINDEXNODEVIEW_IS_INVALID(hNodeView) (!hNodeView? EGG_TRUE:EGG_FALSE)

#define EGGINDEXNODEVIEW_RECORD_INDEX(hNodeView, idx) ( (HEGGINDEXRECORD)((char*)((hNodeView)->hNode + 1) + (idx) * (hNodeView)->reInfo->rdSize) )

#define EGGINDEXNODEVIEW_RDCNT(hNodeView)  ((hNodeView)->hNode->rdCnt)

#define EGGINDEXNODEVIEW_OWNEROFF(hNodeView)  ((hNodeView)->nodeOff)
#define EGGINDEXNODEVIEW_PAREOFF(hNodeView)  ((hNodeView)->hNode->parent)

#define EGGINDEXNODEVIEW_BLOCK(hNodeView)  ((hNodeView)->hNode)



/* for back-compliance */

#define eggIndexNodeView_get_nextOff(hNodeView) eggIndexNodeView_get_nextoff(hNodeView)

/* for back-compliance end */

E_END_DECLS

#endif
