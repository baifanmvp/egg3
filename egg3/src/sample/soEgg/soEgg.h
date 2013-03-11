/**
   \file soEgg.h
   \brief
*/
#ifndef _SAMPLE_SO_EGG_H_
#define _SAMPLE_SO_EGG_H_
#include <walrus/walrus.h>
#include "./soID.h"


W_BEGIN_DECLS

typedef struct _soEgg soEgg;

struct _soEgg
{
    path_t* m_pDir;

    HINDEXWRITE m_hIndexWriter;
    HFIELDINDEX m_hFieldIndex;
    
};


/*!
  \fn GLOBAL soEgg* soEgg_new(const path_t*)
  \brief
  \param pMainDir
  \return
*/
GLOBAL soEgg* soEgg_new(const path_t* pMainDir);

GLOBAL WBOOL soEgg_delete(soEgg* pSoEgg);

GLOBAL WBOOL soEgg_deploy(soEgg* pSoEgg);

GLOBAL soBuildID soEgg_add_build(soEgg* pSoEgg, soBuild* pSoBuild);

GLOBAL HTOPCOLLECTOR soEgg_query_build_ids(soEgg* pSoEgg, const HMULTIQUERY hMultiQuery);

GLOBAL HTOPCOLLECTOR soEgg_query_by_range(soEgg* pSoEgg,
                                          const field_t* pFieldName,
                                          FieldRangeFilter pFieldRangeFilter);


#define soEgg_is_object(pSoEgg) \
    ((pSoEgg)?W_TRUE:W_FALSE)


#define SOEGG_FREE(obj) \
W_STMT_BEGIN \
if (obj != W_NULL) \
   {free(obj); obj = W_NULL;}  \
W_STMT_END




W_END_DECLS


#endif //_SAMPLE_SO_EGG_H_
