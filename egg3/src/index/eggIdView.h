#ifndef _EGG_IDVIEW_H
#define _EGG_IDVIEW_H
#include <sys/types.h>
#include <unistd.h>
#include "../EggDef.h"
#include "eggListView.h"
#include "../storage/File.h"
#include "../storage/ViewStream.h"
#include "../storage/eggIdNode.h"

typedef struct eggIdView EGGIDVIEW;
typedef struct eggIdView* HEGGIDVIEW;
E_BEGIN_DECLS

/*!
  \struct struct tagIdView
  \brief 文章ID集的结构体
*/
struct eggIdView
{
    HVIEWSTREAM hViewStream;
    HEGGLISTVIEW hEggListView;
};

#define EGGIDVIEW_IS_INVALID(hEggIdView) \
    (!hEggIdView ? EGG_TRUE:EGG_FALSE)


PUBLIC HEGGIDVIEW eggIdView_new(HEGGFILE hEggFile);

PUBLIC EBOOL eggIdView_delete(HEGGIDVIEW hEggIdView);

PUBLIC EBOOL eggIdView_load(HEGGIDVIEW hEggIdView, offset64_t nInfoOff);

PUBLIC EBOOL eggIdView_reg(HEGGIDVIEW hEggIdView, HEGGLISTINF hInfo);

PUBLIC EBOOL eggIdView_add(HEGGIDVIEW hEggIdView, HEGGIDNODE hEggIdNodes, count_t nIdCnt);

PUBLIC EBOOL eggIdView_find(HEGGIDVIEW hEggIdView, offset64_t nListOff, HEGGIDNODE* hEggIdNodes, count_t *lpCount);

PUBLIC EBOOL eggIdView_update_info(HEGGIDVIEW hEggIdView);

PUBLIC EBOOL eggIdView_integration(HEGGIDVIEW hEggIdView);

PUBLIC EBOOL eggIdView_set_actinfo(HEGGIDVIEW hEggIdView, ActInfo *hActInfo);

PUBLIC EBOOL eggIdView_clean_actinfo(HEGGIDVIEW hEggIdView, ActInfo *hActInfo);


E_END_DECLS

#endif
