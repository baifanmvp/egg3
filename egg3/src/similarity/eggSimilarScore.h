#ifndef EGG_SIMILARSCORE_H_
#define EGG_SIMILARSCORE_H_

#include "../EggDef.h"
#include "../eggScoreDoc.h"
#include "../storage/eggIdNode.h"
#include <math.h>

struct eggDocOffsetTable
{
    echar* echKey;
    size16_t off;
};


typedef struct eggDocOffsetTable DOCOFFSETTABLE;
typedef struct eggDocOffsetTable* HDOCOFFSETTABLE;

PUBLIC EBOOL similar_score_document(HEGGIDNODE hIdNodes, count_t nIdCnt, HEGGSCOREDOC hScoreDoc);

PUBLIC score_t similar_score_with_field(HEGGIDNODE hIdNodes, count_t nIdCnt);

PUBLIC EBOOL similar_word_with_continuation(HEGGIDNODE hIdNodes, count_t nIdCnt);

#endif //EGG_SIMILARSCORE_H_
