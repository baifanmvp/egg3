#ifndef _EGG_LISTVIEW_H
#define _EGG_LISTVIEW_H
#include "../EggDef.h"
#include "../storage/ViewStream.h"

#define EGGBLOCK_LIMIT_COUNT (4096*128)
E_BEGIN_DECLS

#pragma pack(push)
#pragma pack(4)

typedef struct eggListBlock EGGLISTBLOCK;
typedef struct eggListBlock* HEGGLISTBLOCK;

struct eggListBlock 
{
    size32_t aCnt;
    size32_t uCnt;
    offset64_t next; 
};


typedef struct eggListInf EGGLISTINF;
typedef struct eggListInf* HEGGLISTINF;

struct eggListInf
{
    size16_t aSz;
    size16_t nodeSz;
    size32_t nodeCnt;
    size32_t blkCnt;
    size32_t curCnt;
    size32_t tmpOff;
    offset64_t headOff;
    offset64_t ownOff;
};


#pragma pack(pop)

typedef struct eggListView EGGLISTVIEW;
typedef struct eggListView* HEGGLISTVIEW;

struct eggListView
{
    HVIEWSTREAM hViewStream;
    HEGGLISTINF hInfo;
};

HEGGLISTVIEW eggListView_new(HVIEWSTREAM hViewStream);

EBOOL eggListView_delete(HEGGLISTVIEW hEggListView);

EBOOL eggListView_load_info(HEGGLISTVIEW hEggListView, offset64_t infoOff);

EBOOL eggListView_update_info(HEGGLISTVIEW hEggListView);

offset64_t eggListView_reg_info(HEGGLISTVIEW hEggListView, HEGGLISTINF hInfo);

EBOOL eggListView_insert(HEGGLISTVIEW hEggListView, epointer pNodes, size32_t ndSz);

EBOOL eggListView_fetch(HEGGLISTVIEW hEggListView, epointer* ppNodes, size32_t* pNdSz);

EBOOL eggListView_rewrite(HEGGLISTVIEW hEggListView, epointer pNodes, size32_t ndSz);

#define EGGLISTVIEW_IS_INVALID(hEggListView) ((!hEggListView)? EGG_TRUE :  EGG_FALSE)
#define EGGLISTVIEW_INFO(hEggListView) (hEggListView->hInfo)

#define POWER_OF_TWO(dest, src)     do{ (dest)=1; while (src) { (src)>>=1; (dest)<<=1; }  }while(0)




E_END_DECLS

#endif
