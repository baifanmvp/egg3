#ifndef _EGG_INDEXVIEW_H
#define _EGG_INDEXVIEW_H
#include "../EggDef.h"
#include "../uti/Utility.h"
#include "../storage/ViewStream.h"
#include "./eggIndexNodeView.h"
#include "../storage/eggRecoveryLog.h"
#include "../storage/ViewStream.h"
#include "./eggFieldView.h"
E_BEGIN_DECLS

typedef struct eggRangeId EGGRANGEID;
typedef struct eggRangeId* HEGGRANGEID;
struct eggRangeId
{
    did_t did;
    char val[8];
};

typedef struct eggIndexRange EGGINDEXRANGE;
typedef struct eggIndexRange* HEGGINDEXRANGE;
struct eggIndexRange
{
    size32_t cnt;
    HEGGRANGEID dids;
};



#pragma pack(push)
#pragma pack(4)
typedef struct eggIndexInfo EGGINDEXINFO;
typedef struct eggIndexInfo* HEGGINDEXINFO;

struct eggIndexInfo
{
    //define ascii sort 0x0000 00001
    //define num sort 0x0000 00002
    //define asc 0x0000 00004
    //define dest 0x0000 00008
    type_t type;
    size16_t rdSize;
    size16_t rdCnt;
    offset64_t rootOff;
    offset64_t leafOff;
    fdid_t fid;
};
#pragma pack(pop)

typedef struct eggIndexView EGGINDEXVIEW;
typedef struct eggIndexView* HEGGINDEXVIEW;

struct eggIndexView
{
    HVIEWSTREAM hViewStream;
    HEGGINDEXINFO hInfo;
    HEGGFIELDVIEW hFieldView;
    pthread_mutex_t mutex;
};

HEGGINDEXVIEW eggIndexView_new(HEGGFILE hEggFile, HEGGINDEXINFO hInfo);

EBOOL eggIndexView_delete(HEGGINDEXVIEW hEggIndexView);

PUBLIC EBOOL eggIndexView_load_info(HEGGINDEXVIEW hEggIndexView, HEGGINDEXINFO hInfo, HEGGFIELDVIEW hFieldView);

EBOOL eggIndexView_insert(HEGGINDEXVIEW hEggIndexView, void* key, size32_t kSz, void* val, size32_t vSz);

HEGGINDEXRECORD eggIndexView_fetch(HEGGINDEXVIEW hEggIndexView, void* key, size32_t kSz, void* val, size32_t vSz);

PUBLIC HEGGINDEXRECORD eggIndexView_fetch_withfid(HEGGINDEXVIEW hEggIndexView, fdid_t fid, void* key, size32_t kSz, void* val, size32_t vSz);

PUBLIC HEGGINDEXRANGE eggIndexView_fetch_docid_withfid(HEGGINDEXVIEW hEggIndexView, fdid_t fid, void* key, size32_t kSz, void* val, size32_t vSz);

PUBLIC HEGGINDEXRANGE eggIndexView_range_withfid(HEGGINDEXVIEW hEggIndexView, fdid_t fid, void* startKey, size32_t startSz, void* endKey, size32_t endSz);

PUBLIC EBOOL eggIndexView_insert_pos(HEGGINDEXVIEW hEggIndexView, void* key, size16_t kSz, void* val, size16_t vSz, offset64_t ndPos, index_t rdIdx);

PUBLIC HEGGINDEXRECORD eggIndexView_locate(HEGGINDEXVIEW hEggIndexView, void* key, size32_t kSz, void* val, size32_t vSz, offset64_t* pNdPos, index_t* pRdIdx);

PUBLIC HEGGINDEXRANGE eggIndexView_fetch_docid(HEGGINDEXVIEW hEggIndexView, void* key, size32_t kSz);


PUBLIC HEGGINDEXRANGE eggIndexView_range(HEGGINDEXVIEW hEggIndexView, void* startKey, size32_t startSz, void* endKey, size32_t endSz);

offset64_t eggIndexView_get_leafOff(HEGGINDEXVIEW hEggIndexView);

HEGGINDEXNODEVIEW eggIndexView_load_node(HEGGINDEXVIEW hEggIndexView, offset64_t nodeOff);

PUBLIC EBOOL eggIndexView_itercheck(HEGGINDEXVIEW hEggIndexView, offset64_t nIterOff, HEGGINDEXRECORD* lplp_key_rd);

PUBLIC EBOOL eggIndexView_update_record(HEGGINDEXVIEW hEggIndexView, HEGGINDEXRECORD hRecord, offset64_t ndPos, index_t rdIdx);
 
PUBLIC EBOOL eggIndexView_set_actinfo(HEGGINDEXVIEW hEggIndexView,
                                     ActInfo *hActInfo);
PUBLIC EBOOL eggIndexView_clean_actinfo(HEGGINDEXVIEW hEggIndexView,
                                       ActInfo *hActInfo);
PUBLIC EBOOL eggIndexView_update_recordvalue(HEGGINDEXVIEW hEggIndexView, offset64_t ndPos, index_t rdIdx, void* val, size32_t vSz);

PUBLIC HEGGINDEXRANGE eggIndexView_export_ids(HEGGINDEXVIEW hEggIndexView);

#define EGGINDEXVIEW_IS_INVALID(hEggIndexView) (!hEggIndexView ? EGG_TRUE : EGG_FALSE)

#define EGGINDEXVIEW_LEAFOFF(hEggIndexView)  ((hEggIndexView)->hInfo->leafOff)

#define EGGINDEXVIEW_FIELDVIEW(hEggIndexView)  ((hEggIndexView)->hFieldView)

#define EGGINDEXVIEW_RECORD_OVERFLOW(hEggIndexView, kSz, vSz)  ((hEggIndexView)->hInfo->rdSize < (kSz) + (vSz) + sizeof(EGGINDEXRECORD))

/* for back-compliance */

#define eggIndexView_iterCheck(hEggIndexView, nIterOff, lplp_key_rd) eggIndexView_itercheck(hEggIndexView, nIterOff, lplp_key_rd)

/* for back-compliance end */

E_END_DECLS

#endif
