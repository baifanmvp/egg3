#ifndef _EGG_INDEX_RECORD_H
#define _EGG_INDEX_RECORD_H


#include "../EggDef.h"
#include "../uti/eggUtiMd5.h"
E_BEGIN_DECLS

typedef struct eggIndexRdPos  EGGINDEXRDPOS;
typedef struct eggIndexRdPos* HEGGINDEXRDPOS;

struct eggIndexRdPos
{
    offset64_t ndPos;
    index_t rdIdx;
};


typedef struct eggIndexRecord  EGGINDEXRECORD;
typedef struct eggIndexRecord* HEGGINDEXRECORD;
typedef int (*RDCMP) (HEGGINDEXRECORD pSrcNode, HEGGINDEXRECORD pDestNode);


#pragma pack(push)
#pragma pack(4)
struct eggIndexRecord
{
    offset64_t childOff;
    offset64_t hostOff;
    
    size16_t kSz;
    size16_t vSz;
};
#pragma pack(pop)

typedef struct eggRecordDocId  EGGRECORDDOCID;
typedef struct eggRecordDocId* HEGGRECORDDOCID;

#pragma pack(push)
#pragma pack(4)
struct eggRecordDocId
{
    u8 flag;
    offset64_t did;
};
#pragma pack(pop)
#define RECORDDOCID_VALID ((flag_t)(1))
#define RECORDDOCID_NOVALID ((flag_t)(0))

#define RECORDDOCID_MIN ((offset64_t)(0))
#define RECORDDOCID_MAX ((offset64_t)(-1))

HEGGINDEXRECORD eggIndexRecord_new(size16_t aSz, void* pKey, size32_t kSz, void* pVal, size32_t vSz);

EBOOL eggIndexRecord_delete(HEGGINDEXRECORD hEggIndexRecord);

RDCMP eggIndexNode_get_fncmp(type_t type);

#define EGGINDEXRECORD_KEY(pRecord) ((HEGGINDEXRECORD)(pRecord) + 1)

#define EGGINDEXRECORD_KSIZE(pRecord) (( (HEGGINDEXRECORD)(pRecord) )->kSz)

#define EGGINDEXRECORD_VAL(pRecord) ( (char*)(pRecord) + sizeof(EGGINDEXRECORD) + (pRecord)->kSz  )



#define EGGINDEXRECORD_VSIZE(pRecord) (( (HEGGINDEXRECORD)(pRecord) )->vSz)

#define EGGINDEXRECORD_IS_INVALID(pRecord) (!(pRecord)? EGG_TRUE : EGG_FALSE)

//#define EGGINDEXRECORD_IS_(pRecord) (!(pRecord)? EGG_TRUE : EGG_FALSE)

#define  BTREE_RECORD_UNIQUE                        (EGG_NORMAL_INDEX)
#define  BTREE_RECORD_REPEAT                        (EGG_RANGE_INDEX)

E_END_DECLS

#endif
