/**
   \file eggTopCollector.h
   \brief 结果集合 (API)
   \ingroup egg
*/

#ifndef _EGG_TOPCOLLECTOR_H_
#define _EGG_TOPCOLLECTOR_H_

#include "./EggDef.h"
#include "./eggScoreDoc.h"
#include "./eggFieldKey.h"
#include "./storage/eggIdNode.h"
#include "./eggSearchIter.h"
#include "./eggIndexReader.h"
#include "./index/eggFieldWeight.h"

E_BEGIN_DECLS

typedef struct eggTopCollector  EGGTOPCOLLECTOR; 
typedef struct eggTopCollector* HEGGTOPCOLLECTOR; 

typedef struct eggTopCtorChunk  EGGTOPCTORCHUNK;
typedef struct eggTopCtorChunk* HEGGTOPCTORCHUNK;
#pragma pack(push)
#pragma pack(4)
struct eggTopCtorChunk
{
    size32_t size;
    count_t cntTotal;
    count_t cntHits;
    count_t cntQuery;
    count_t cntRange;
    count_t cntId;
    size32_t szFieldKey;
    size32_t szOrderBy;
    type_t sortType;
};
#pragma pack(pop)

HEGGTOPCOLLECTOR EGGAPI eggTopCollector_new(count_t nNumHits);

EBOOL EGGAPI eggTopCollector_delete(HEGGTOPCOLLECTOR hTopCollector);

HEGGTOPCOLLECTOR EGGAPI eggTopCollector_dup(HEGGTOPCOLLECTOR hTopCollector);

count_t EGGAPI eggTopCollector_total(HEGGTOPCOLLECTOR hTopCollector);

count_t EGGAPI eggTopCollector_total_hits(HEGGTOPCOLLECTOR hTopCollector);

HEGGSCOREDOC EGGAPI eggTopCollector_top_docs(HEGGTOPCOLLECTOR hTopCollector);

EBOOL EGGAPI eggTopCollector_override_docs(HEGGTOPCOLLECTOR hTopCollector, HEGGSCOREDOC hScoreDoc, count_t nScoreDoc);

EBOOL EGGAPI eggTopCollector_truncate_docs(HEGGTOPCOLLECTOR hTopCollector, count_t nScoreDoc);

EBOOL EGGAPI eggTopCollector_clean_docs(HEGGTOPCOLLECTOR hTopCollector);

PUBLIC EBOOL eggTopCollector_add_idrange(HEGGTOPCOLLECTOR hTopCollector, fdid_t fdid, HEGGWRESULT hWeightResult, count_t nWeightResult);

PUBLIC EBOOL eggTopCollector_clean_idrange(HEGGTOPCOLLECTOR hTopCollector);

PUBLIC EBOOL eggTopCollector_add_fieldkey(HEGGTOPCOLLECTOR hTopCollector, HEGGFIELDKEY hEggFieldKey);

PUBLIC EBOOL eggTopCollector_clean_fieldkey(HEGGTOPCOLLECTOR hTopCollector);

PUBLIC EBOOL eggTopCollector_add_idnodes(HEGGTOPCOLLECTOR hTopCollector, HEGGIDNODE hIdNodes, count_t nIdCnt);

PUBLIC EBOOL eggTopCollector_clean_idnodes(HEGGTOPCOLLECTOR hTopCollector);

PUBLIC EBOOL eggTopCollector_set_cnthits(HEGGTOPCOLLECTOR hTopCollector, count_t cntHits);

PUBLIC EBOOL eggTopCollector_set_sorttype(HEGGTOPCOLLECTOR hTopCollector, type_t sortType);

PUBLIC EBOOL eggTopCollector_get_sorttype(HEGGTOPCOLLECTOR hTopCollector);

PUBLIC EBOOL eggTopCollector_set_orderby(HEGGTOPCOLLECTOR hTopCollector,
                                         size16_t cnt,
                                         char *fieldName,
                                         EBOOL isAsc, ...);


PUBLIC EBOOL eggTopCollector_clean_orderby(HEGGTOPCOLLECTOR hTopCollector);

PUBLIC EBOOL eggTopCollector_orderby(HEGGTOPCOLLECTOR hTopCollector,
                                     HEGGINDEXREADER hIndexReader);

PUBLIC EBOOL eggTopCollector_if_orderby(HEGGTOPCOLLECTOR hTopCollector);
                                     
PUBLIC EBOOL eggTopCollector_minus(HEGGTOPCOLLECTOR hTopCollectorResult,
                                   HEGGTOPCOLLECTOR hTopCollector2);

PUBLIC EGGAPI EBOOL eggTopCollector_get_keyposition(HEGGTOPCOLLECTOR hTopCollector,
                                             did_t docId, char *fieldName,
                                             char ***keys, size16_t **keysSz,
                                             int ***position, count_t *cnt);
PUBLIC EBOOL eggTopCollector_set_chunkid_cluster(HEGGTOPCOLLECTOR hCollector, SPANPOINT chunkId);

PUBLIC EBOOL eggTopCollector_cut_result(HEGGTOPCOLLECTOR hTopCollector, HEGGSEARCHITER hIter);
/*!
  \fn PUBLIC EBOOL eggTopCollector_normalized(HEGGTOPCOLLECTOR , type_t )
  \param op (Ref EggDef.h) EGG_TOPSORT_SCORE ...
  \return
*/
PUBLIC EBOOL EGGAPI  eggTopCollector_normalized(HEGGTOPCOLLECTOR hTopCollector, type_t op);

PUBLIC EBOOL eggTopCollector_ultimatenormalized(HEGGTOPCOLLECTOR hTopCollector);

PUBLIC HEGGTOPCTORCHUNK eggTopCollector_serialization(HEGGTOPCOLLECTOR hTopCollector);

PUBLIC HEGGTOPCOLLECTOR eggTopCollector_unserialization(HEGGTOPCTORCHUNK hTopCtorChunk);

PUBLIC HEGGTOPCOLLECTOR eggTopCollector_merge_with_ref(HEGGTOPCOLLECTOR hDestCollector, HEGGTOPCOLLECTOR hSrcCollector);

PUBLIC EBOOL eggTopCollector_set_filterweight(HEGGTOPCOLLECTOR hTopCollector, int weight_start, int weight_end);

PUBLIC EBOOL EGGAPI  eggTopCollector_filter_weight(HEGGTOPCOLLECTOR hCollector, int weight_start, int weight_end);

PUBLIC EBOOL EGGAPI eggTopCollector_filter_weightstring(HEGGTOPCOLLECTOR hTopCollector, char *weightStart, char *weightEnd);

PUBLIC EBOOL eggTopCollector_sort_with_score(HEGGTOPCOLLECTOR hTopCollector);

PUBLIC EBOOL eggTopCollector_sort_with_weight(HEGGTOPCOLLECTOR hTopCollector);

PUBLIC EBOOL eggTopCollector_merge_with_cluster(HEGGTOPCOLLECTOR hTopCollectorResult, count_t collectorCnt, HEGGTOPCOLLECTOR* lpTopCollectorSet);

#define EGGTOPCOLLECTOR_IS_INVALID(hTopCollector)   \
    ((!hTopCollector) ? EGG_TRUE : EGG_FALSE)





/* for back-compliance */
#define eggTopCollector_ultimateNormalized(hTopCollector) eggTopCollector_ultimatenormalized(hTopCollector) 
#define eggTopCollector_set_sortType(hTopCollector, sortType) eggTopCollector_set_sorttype(hTopCollector, sortType) 
#define eggTopCollector_set_orderBy(hTopCollector, cnt, fieldName, ...) eggTopCollector_set_orderby(hTopCollector, cnt, fieldName, __VA_ARGS__) 
#define eggTopCollector_set_filterWeight(hTopCollector, weight_start, weight_end) eggTopCollector_set_filterweight(hTopCollector, weight_start, weight_end) 
#define eggTopCollector_set_cntHits(hTopCollector, cntHits) eggTopCollector_set_cnthits(hTopCollector, cntHits) 
#define eggTopCollector_set_chunkId_cluster(hCollector, chunkId) eggTopCollector_set_chunkid_cluster(hCollector, chunkId) 
#define eggTopCollector_orderBy(hTopCollector, hIndexReader) eggTopCollector_orderby(hTopCollector, hIndexReader) 
#define eggTopCollector_ifOrderBy(hTopCollector) eggTopCollector_if_orderby(hTopCollector) 
#define eggTopCollector_get_sortType(hTopCollector) eggTopCollector_get_sorttype(hTopCollector) 
#define eggTopCollector_get_keyPosition(hTopCollector, docId, fieldName, keys, keysSz, position, cnt) eggTopCollector_get_keyposition(hTopCollector, docId, fieldName, keys, keysSz, position, cnt) 
#define eggTopCollector_filter_weightString(hTopCollector, weightStart, weightEnd) eggTopCollector_filter_weightstring(hTopCollector, weightStart, weightEnd) 
#define eggTopCollector_cutResult(hTopCollector, hIter) eggTopCollector_cut_result(hTopCollector, hIter) 
#define eggTopCollector_clean_idRange(hTopCollector) eggTopCollector_clean_idrange(hTopCollector) 
#define eggTopCollector_add_idRange(hTopCollector, fdid, hWeightResult, nWeightResult) eggTopCollector_add_idrange(hTopCollector, fdid, hWeightResult, nWeightResult) 
#define eggTopCollector_add_idNodes(hTopCollector, hIdNodes, nIdCnt) eggTopCollector_add_idnodes(hTopCollector, hIdNodes, nIdCnt) 
#define eggTopCollector_add_fieldKey(hTopCollector, hEggFieldKey) eggTopCollector_add_fieldkey(hTopCollector, hEggFieldKey) 

/* for back-compliance end */

E_END_DECLS

#endif //_EGG_TOP_DOCUMENT_COLLECTOR_H_
