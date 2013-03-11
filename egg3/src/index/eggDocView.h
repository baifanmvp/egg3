#ifndef EGG_DOCVIEW_H_
#define EGG_DOCVIEW_H_

#include "../EggDef.h"
#include "../storage/ViewStream.h"
#include "../storage/eggDocNode.h"
#include "../storage/eggIdTable.h"
E_BEGIN_DECLS

#define EGG_FIRST_DOCID(idx) (sizeof(CLUSTER) + MAP_VIEW_OFFSET + (idx) * CLUSTER_ALLOC_SIZE)
typedef struct eggDocViewInfo EGGDOCVIEWINFO;
typedef struct eggDocViewInfo* HEGGDOCVIEWINFO;

#pragma pack(push)
#pragma pack(4)
struct eggDocViewInfo
{
    count_t addCnt;
};
#pragma pack(pop)

typedef struct eggDocView EGGDOCVIEW;
typedef struct eggDocView* HEGGDOCVIEW;

struct eggDocView
{
    EGGDOCVIEWINFO info;
    HVIEWSTREAM hViewStream;
    HEGGIDTABLE hIdTable;
};

extern HEGGDOCVIEW eggDocView_new(HEGGFILE hEggFile, HEGGIDTABLE hIdTable);

extern EBOOL eggDocView_delete(HEGGDOCVIEW hEggDocView);

extern did_t eggDocView_add(HEGGDOCVIEW hEggDocView, const HEGGDOCNODE hDocNode);

extern EBOOL eggDocView_query(HEGGDOCVIEW hEggDocView, did_t id, HEGGDOCNODE* lphDocNode);

extern EBOOL eggDocView_update(HEGGDOCVIEW hEggDocView, did_t idNum, HEGGDOCNODE hDocNode);

extern EBOOL eggDocView_remove(HEGGDOCVIEW hEggDocView, did_t id);

extern size64_t eggDocView_size(HEGGDOCVIEW hEggDocView);

extern EBOOL eggDocView_update_info(HEGGDOCVIEW hEggDocView);

extern count_t eggDocView_get_doccnt(HEGGDOCVIEW hEggDocView);

extern EBOOL eggDocView_rdlock_id(HEGGDOCVIEW hEggDocView, did_t id);

extern EBOOL eggDocView_wrlock_id(HEGGDOCVIEW hEggDocView, did_t id);

extern EBOOL eggDocView_unlock_id(HEGGDOCVIEW hEggDocView, did_t id);

extern EBOOL eggDocView_clean_actinfo(HEGGDOCVIEW hEggDocView,
                                     ActInfo *hActInfo);
extern EBOOL eggDocView_set_actinfo(HEGGDOCVIEW hEggDocView, ActInfo *hActInfo);


#define EGGDOCVIEW_IS_INVALID(hEggDocView)      \
    (!hEggDocView ? EGG_TRUE : EGG_FALSE)

#define EGGDOCVIEW_INFO(hEggDocView)            \
    (((EGGDOCVIEW*)hEggDocView)->info)



/* for back-compliance */

#define eggDocView_get_docCnt(hEggDocView) eggDocView_get_doccnt(hEggDocView) 

/* for back-compliance end */



E_END_DECLS

#endif
