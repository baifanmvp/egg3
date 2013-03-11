/**
   \file eggDid.h
   \brief document ID
   \ingroup egg
*/
#ifndef EGG_DID_H_
#define EGG_DID_H_
#include "./EggDef.h"
#include "./net/eggSpanUnit.h"

E_BEGIN_DECLS


/*!
  \typedef struct tagScoreDoc SCOREDOC
  \brief  打分文章ID
*/
typedef union eggDid EGGDID;
typedef union eggDid* HEGGDID;
#pragma pack(push)
#pragma pack(4)

union eggDid
{
    struct 
    {
        did_t docId;
    };

    struct eggDidLocal
    {
        did_t docId;
    }local;
    
    struct eggDidCluster
    {
        did_t docId;
        SPANPOINT chunkId;
    }cluster;
    
};
#pragma pack(pop)

#define EGGDID_DOCID(hDid) (((HEGGDID)(hDid))->docId)

#define EGGDID_CHUNKID(hDid) (((HEGGDID)(hDid))->cluster.chunkId)

E_END_DECLS

#endif //_EGG_SCORE_DOCUMENT_H_
