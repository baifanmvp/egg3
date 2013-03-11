#include "../eggTopCollector.h"
#include "../uti/Utility.h"
#include "../eggScoreDoc.h"
#include "../eggSearchIter.h"
#include "../index/eggFieldView.h"
#include "../index/eggFieldWeight.h"
#include "../interface/eggIndexReaderLocal.h"
#include "../log/eggPrtLog.h"

#include <assert.h>
#include <math.h>
#include <pthread.h>
extern pthread_mutex_t counter_mutex;
typedef struct eggOrderBy EGGORDERBY;
typedef struct eggOrderBy *HEGGORDERBY;

typedef struct eggIdRangeOrder
{
    size16_t fdid;
    count_t cntRange;
    HEGGWRESULT idRange;
} EGGIDRANGEORDER;
PRIVATE EBOOL eggIdRangeOrder_delete(EGGIDRANGEORDER *hIdRangeOrder, count_t cntIdRangeOrder);

struct eggTopCollector
{
    count_t cntTotal;
    count_t cntHits;
    HEGGSCOREDOC docs;
    did_t* idRange;
    count_t cntRange;
    HEGGIDNODE hIdNodes;
    count_t cntId;
    HEGGFIELDKEY hFieldKey;
    HEGGORDERBY hOrderBy;
    count_t cntOrderBy;
    int filterWeight_start;
    int filterWeight_end;
    type_t sortType;
    EGGIDRANGEORDER* hIdRangeOrder;
    count_t cntIdRangeOrder;
};

struct eggOrderBy
{
    char *fieldName;
    EBOOL isAsc;
};

#define EGGTOPCTORCHUNK_DOCS(hTopctorChunk) (hTopctorChunk + 1)
#define EGGTOPCTORCHUNK_IDRANGE(hTopctorChunk) ((char*)(hTopctorChunk + 1) + hTopctorChunk->cntHits * sizeof(EGGSCOREDOC))
#define EGGTOPCTORCHUNK_IDNODES(hTopctorChunk) ((char*)(hTopctorChunk + 1) + hTopctorChunk->cntHits * sizeof(EGGSCOREDOC) + hTopctorChunk->cntRange * sizeof(did_t))
#define EGGTOPCTORCHUNK_FIELDKEY(hTopctorChunk) ((char*)(hTopctorChunk + 1) + hTopctorChunk->cntHits * sizeof(EGGSCOREDOC) + hTopctorChunk->cntRange * sizeof(did_t) + hTopctorChunk->cntId * sizeof(EGGIDNODE))
#define EGGTOPCTORCHUNK_ORDERBY(hTopctorChunk) ((char*)(hTopctorChunk + 1) + hTopctorChunk->cntHits * sizeof(EGGSCOREDOC) + hTopctorChunk->cntRange * sizeof(did_t) + hTopctorChunk->cntId * sizeof(EGGIDNODE) + hTopctorChunk->szFieldKey)

PRIVATE EBOOL eggTopCollector_normalized_weight(HEGGTOPCOLLECTOR hTopCollector);

PRIVATE EBOOL eggTopCollector_normalized_score(HEGGTOPCOLLECTOR hTopCollector);

PRIVATE EBOOL eggTopCollector_normalized_not_score(HEGGTOPCOLLECTOR hTopCollector);

//PRIVATE EBOOL eggTopCollector_normalized_orderby(HEGGTOPCOLLECTOR hTopCollector);


PRIVATE EBOOL eggTopCollector_scoreLinear(HEGGTOPCOLLECTOR hTopCollector);


PRIVATE char *eggOrderBy_serialise(HEGGORDERBY hOrderBy,
                                   count_t cntOrderBy,
                                   size32_t *szOrderBy);
PRIVATE HEGGORDERBY eggOrderBy_unserialise(char *buf, size32_t size,
                                           count_t *cntOrderBy);
PRIVATE HEGGORDERBY eggOrderBy_dup(HEGGORDERBY hOrderBy, count_t cntOrderBy, count_t *nOrderBy);
PRIVATE EBOOL eggOrderBy_delete(HEGGORDERBY hOrderBy, count_t cnt);


PRIVATE EBOOL eggIdRangeOrder_delete(EGGIDRANGEORDER *hIdRangeOrder, count_t cntIdRangeOrder)
{
    if (cntIdRangeOrder == 0 || !hIdRangeOrder)
    {
        return EGG_FALSE;
    }
    count_t i;
    for (i = 0; i < cntIdRangeOrder; i++)
    {
        free(hIdRangeOrder[i].idRange);
    }
    free(hIdRangeOrder);
    return EGG_TRUE;
}

HEGGTOPCOLLECTOR EGGAPI eggTopCollector_new(count_t nNumHits)
{
    HEGGTOPCOLLECTOR lp_top_collector = (HEGGTOPCOLLECTOR)calloc(1, sizeof(EGGTOPCOLLECTOR));
    if (EGGTOPCOLLECTOR_IS_INVALID(lp_top_collector))
    {
        return EGG_NULL;
    }
    
    lp_top_collector->cntTotal = 0;
    lp_top_collector->cntHits = nNumHits;
    lp_top_collector->docs = EGG_NULL;

    lp_top_collector->idRange = EGG_NULL;
    lp_top_collector->cntRange = 0;
    lp_top_collector->sortType = EGG_TOPSORT_NOT;
    return lp_top_collector;
}

EBOOL EGGAPI eggTopCollector_delete(HEGGTOPCOLLECTOR hTopCollector)
{
    if (!EGGTOPCOLLECTOR_IS_INVALID(hTopCollector))
    {

        if (hTopCollector->cntHits)
        {
            EGGSCOREDOC_DELETE(hTopCollector->docs);
            hTopCollector->docs = EGG_NULL;
    
        }

        /* idnode版本去重时,会将cntId直接置0 */
        //if(hTopCollector->cntId)
        {
            free(hTopCollector->hIdNodes);
            hTopCollector->hIdNodes = EGG_NULL;
        }

        if(hTopCollector->hFieldKey)
        {
            eggFieldKey_del(hTopCollector->hFieldKey);
            hTopCollector->hFieldKey = EGG_NULL;
        }

        if(hTopCollector->cntRange)
        {
            free(hTopCollector->idRange);
            hTopCollector->idRange = EGG_NULL;
        }

        if(hTopCollector->cntOrderBy)
        {
            eggOrderBy_delete(hTopCollector->hOrderBy,
                              hTopCollector->cntOrderBy);
            hTopCollector->hOrderBy = EGG_NULL;
        }
        
        free(hTopCollector);
        return EGG_TRUE;
    }

    return EGG_FALSE;
}

HEGGTOPCOLLECTOR EGGAPI eggTopCollector_dup(HEGGTOPCOLLECTOR hTopCollector)
{
    if (EGGTOPCOLLECTOR_IS_INVALID(hTopCollector))
    {
        return EGG_NULL;
    }
    
    HEGGTOPCOLLECTOR lp_top_collector = (HEGGTOPCOLLECTOR)calloc(1, sizeof(EGGTOPCOLLECTOR));
    lp_top_collector->cntTotal = hTopCollector->cntTotal;
    lp_top_collector->cntHits = hTopCollector->cntHits;
    lp_top_collector->cntRange = hTopCollector->cntRange;
    lp_top_collector->cntId = hTopCollector->cntId;
    lp_top_collector->sortType = hTopCollector->sortType;
    
    lp_top_collector->hFieldKey = eggFieldKey_dup(hTopCollector->hFieldKey);

    if(lp_top_collector->cntHits && hTopCollector->docs)
    {
        lp_top_collector->docs = (HEGGSCOREDOC) malloc(sizeof(EGGSCOREDOC)* lp_top_collector->cntHits);
        memcpy(lp_top_collector->docs, hTopCollector->docs, sizeof(EGGSCOREDOC)* lp_top_collector->cntHits);
    }
    
    if(lp_top_collector->cntRange && hTopCollector->idRange)
    {
        lp_top_collector->idRange = (did_t*) malloc(sizeof(did_t)* lp_top_collector->cntRange);
        memcpy(lp_top_collector->idRange, hTopCollector->idRange, sizeof(did_t)* lp_top_collector->cntRange);
    }

    if(lp_top_collector->cntId && hTopCollector->hIdNodes)
    {
        lp_top_collector->hIdNodes = (HEGGIDNODE) malloc(sizeof(EGGIDNODE)* lp_top_collector->cntId);
        memcpy(lp_top_collector->hIdNodes, hTopCollector->hIdNodes, sizeof(EGGIDNODE)* lp_top_collector->cntId);
    }

    lp_top_collector->hOrderBy = eggOrderBy_dup(hTopCollector->hOrderBy,
                                                hTopCollector->cntOrderBy,
                                                &lp_top_collector->cntOrderBy);
    
    
    return lp_top_collector;
}

count_t EGGAPI eggTopCollector_total(HEGGTOPCOLLECTOR hTopCollector)
{
    if (EGGTOPCOLLECTOR_IS_INVALID(hTopCollector))
    {
        return 0;
    }

    return hTopCollector->cntTotal;
}

count_t EGGAPI eggTopCollector_total_hits(HEGGTOPCOLLECTOR hTopCollector)
{
    if (EGGTOPCOLLECTOR_IS_INVALID(hTopCollector))
    {
        return 0;
    }

    return hTopCollector->cntHits;
}

HEGGSCOREDOC EGGAPI eggTopCollector_top_docs(HEGGTOPCOLLECTOR hTopCollector)
{
    if (EGGTOPCOLLECTOR_IS_INVALID(hTopCollector))
    {
        return EGG_NULL;
    }

    return hTopCollector->docs;
}

EBOOL EGGAPI eggTopCollector_override_docs(HEGGTOPCOLLECTOR hTopCollector, HEGGSCOREDOC hScoreDoc, count_t nScoreDoc)
{
    if (EGGTOPCOLLECTOR_IS_INVALID(hTopCollector))
    {
        return EGG_FALSE;
    }
    free(hTopCollector->docs);
    hTopCollector->cntHits = nScoreDoc;
    hTopCollector->docs = hScoreDoc;
    return EGG_TRUE;
}

EBOOL EGGAPI eggTopCollector_truncate_docs(HEGGTOPCOLLECTOR hTopCollector, count_t nScoreDoc)
{
    if (EGGTOPCOLLECTOR_IS_INVALID(hTopCollector))
    {
        return EGG_FALSE;
    }
    if (nScoreDoc > 0 && nScoreDoc < hTopCollector->cntHits)
    {
        hTopCollector->cntHits = nScoreDoc;
    }
    return EGG_TRUE;
}

EBOOL EGGAPI eggTopCollector_clean_docs(HEGGTOPCOLLECTOR hTopCollector)
{
    if (EGGTOPCOLLECTOR_IS_INVALID(hTopCollector))
    {
        return EGG_FALSE;
    }
    
    EGGSCOREDOC_DELETE(hTopCollector->docs);
    hTopCollector->docs = EGG_NULL;
    hTopCollector->cntHits = 0;
    
    return EGG_TRUE;
}

PUBLIC EBOOL eggTopCollector_set_orderby(HEGGTOPCOLLECTOR hTopCollector,
                                         size16_t cnt,
                                         char *fieldName,
                                         EBOOL isAsc, ...)
{
    if (cnt == 0)
    {
        return EGG_FALSE;
    }
    
    hTopCollector->cntOrderBy = cnt;
    hTopCollector->hOrderBy = malloc(hTopCollector->cntOrderBy
                                     * sizeof(EGGORDERBY));
    assert(hTopCollector->hOrderBy);
    index_t idx = 0;
    va_list ap_arg;
    va_start(ap_arg, isAsc);
    
    do
    {
        hTopCollector->hOrderBy[idx].fieldName = strdup(fieldName);
        assert(hTopCollector->hOrderBy[idx].fieldName);
        hTopCollector->hOrderBy[idx].isAsc = isAsc;
        
        fieldName = va_arg(ap_arg, char*);
        isAsc = (EBOOL)va_arg(ap_arg, int);
        idx++;
        
    } while (idx < hTopCollector->cntOrderBy);

    va_end(ap_arg);

    hTopCollector->sortType = EGG_TOPSORT_ORDERBY;
    
    return EGG_TRUE;
}

PUBLIC EBOOL eggTopCollector_clean_orderby(HEGGTOPCOLLECTOR hTopCollector)
{
    if (!hTopCollector)
    {
        return EGG_FALSE;
    }

    if(hTopCollector->cntOrderBy)
    {
        eggOrderBy_delete(hTopCollector->hOrderBy,
                          hTopCollector->cntOrderBy);
        
        hTopCollector->cntOrderBy = 0;
        hTopCollector->hOrderBy = EGG_NULL;
    }
    
    return EGG_TRUE;
}


PRIVATE EBOOL eggOrderBy_delete(HEGGORDERBY hOrderBy, count_t cnt)
{
    count_t idx;
    for (idx = 0; idx < cnt; idx++)
    {
        free(hOrderBy[idx].fieldName);
    }
    free(hOrderBy);
    return EGG_TRUE;
}

PUBLIC EBOOL eggTopCollector_if_orderby(HEGGTOPCOLLECTOR hTopCollector)
{
    if (hTopCollector->hOrderBy)
    {
        return EGG_TRUE;
    }
    return EGG_FALSE;
}

PUBLIC EBOOL eggTopCollector_minus(HEGGTOPCOLLECTOR hTopCollector,
                                   HEGGTOPCOLLECTOR hTopCollector2)
{
    HEGGIDNODE lp_id_nodes = hTopCollector->hIdNodes;
    
    if(!lp_id_nodes)
    {
        hTopCollector->cntHits = 0;
        return EGG_FALSE;
    }

    free(hTopCollector->docs);
    hTopCollector->docs = EGG_NULL;
    free(hTopCollector->idRange);
    hTopCollector->idRange = EGG_NULL;
    hTopCollector->cntRange = 0;  
    
    UTIVECTOR* lp_vector = Uti_vector_create(sizeof(EGGIDNODE));
    index_t n_ids_idx = 0;
    index_t n_ids_idx2 = 0;
    HEGGIDNODE lp_id_nodes2 = hTopCollector2->hIdNodes;    

    while(n_ids_idx != hTopCollector->cntId
          && n_ids_idx2 != hTopCollector2->cntId)
    {
        
        if (lp_id_nodes[n_ids_idx].id < lp_id_nodes2[n_ids_idx2].id)
        {
            Uti_vector_push(lp_vector, lp_id_nodes+n_ids_idx, 1);
            n_ids_idx++;
        }
        else if (lp_id_nodes[n_ids_idx].id > lp_id_nodes2[n_ids_idx2].id)
        {
            n_ids_idx2++;
        }
        else
        {
            n_ids_idx++;
        }
    }
    Uti_vector_push(lp_vector, lp_id_nodes + n_ids_idx,
                    hTopCollector->cntId - n_ids_idx);
    hTopCollector->cntId = Uti_vector_count(lp_vector);    
    hTopCollector->hIdNodes = Uti_vector_data(lp_vector);
    Uti_vector_destroy(lp_vector, EGG_FALSE);

    return EGG_TRUE;    
}

PUBLIC EBOOL eggTopCollector_add_idrange(HEGGTOPCOLLECTOR hTopCollector, fdid_t fdid, HEGGWRESULT hWeightResult, count_t nWeightResult)
{
    if (EGGTOPCOLLECTOR_IS_INVALID(hTopCollector))
    {
        return EGG_FALSE;
    }

    if (nWeightResult == 0)
    {
        return EGG_TRUE;
    }
    
    int i;
    for (i = 0; i < hTopCollector->cntIdRangeOrder; i++)
    {
        if (hTopCollector->hIdRangeOrder[i].fdid == fdid)
        {
            /* 当eggQuery中有相同域的range查询，取结果多的
             */
            if (hTopCollector->hIdRangeOrder[i].cntRange < nWeightResult)
            {
                hTopCollector->hIdRangeOrder[i].cntRange = nWeightResult;
                free(hTopCollector->hIdRangeOrder[i].idRange);
                hTopCollector->hIdRangeOrder[i].idRange = hWeightResult;
            }
            return EGG_TRUE;
        }
    }

    ++hTopCollector->cntIdRangeOrder;
    hTopCollector->hIdRangeOrder = realloc(hTopCollector->hIdRangeOrder,
                                          sizeof(EGGIDRANGEORDER)*hTopCollector->cntIdRangeOrder);
    assert(hTopCollector->hIdRangeOrder);
    EGGIDRANGEORDER *p_idRangeOrder = &hTopCollector->hIdRangeOrder[hTopCollector->cntIdRangeOrder-1];
    assert(p_idRangeOrder);
    p_idRangeOrder->fdid = fdid;
    p_idRangeOrder->cntRange = nWeightResult;
    p_idRangeOrder->idRange = hWeightResult;
    
    return EGG_TRUE;
}

PUBLIC EBOOL eggTopCollector_clean_idrange(HEGGTOPCOLLECTOR hTopCollector)
{
    if (hTopCollector->cntIdRangeOrder == 0 || !hTopCollector->hIdRangeOrder)
    {
        return EGG_FALSE;
    }
    eggIdRangeOrder_delete(hTopCollector->hIdRangeOrder, hTopCollector->cntIdRangeOrder);
    hTopCollector->cntIdRangeOrder = 0;
    hTopCollector->hIdRangeOrder = NULL;
    return EGG_TRUE;
}

PUBLIC EBOOL eggTopCollector_add_idnodes(HEGGTOPCOLLECTOR hTopCollector, HEGGIDNODE hIdNodes, count_t nIdCnt)
{
    if (EGGTOPCOLLECTOR_IS_INVALID(hTopCollector))
    {
        return EGG_FALSE;
    }
    hTopCollector->hIdNodes = hIdNodes;
    hTopCollector->cntId = nIdCnt;
    
    
    return EGG_TRUE;
}

PUBLIC EBOOL eggTopCollector_clean_idnodes(HEGGTOPCOLLECTOR hTopCollector)
{
    if (EGGTOPCOLLECTOR_IS_INVALID(hTopCollector))
    {
        return EGG_FALSE;
    }

    free(hTopCollector->hIdNodes);
    hTopCollector->hIdNodes = EGG_NULL;
    hTopCollector->cntId = 0;
    
    return EGG_TRUE;
}

PUBLIC EBOOL eggTopCollector_add_fieldkey(HEGGTOPCOLLECTOR hTopCollector, HEGGFIELDKEY hEggFieldKey)
{
    if (EGGTOPCOLLECTOR_IS_INVALID(hTopCollector))
    {
        return EGG_FALSE;
    }
    hTopCollector->hFieldKey = hEggFieldKey;
    
    return EGG_TRUE;
}


PUBLIC EBOOL eggTopCollector_clean_fieldkey(HEGGTOPCOLLECTOR hTopCollector)
{
    if (EGGTOPCOLLECTOR_IS_INVALID(hTopCollector))
    {
        return EGG_FALSE;
    }
    
    if(hTopCollector->hFieldKey)
    {
        eggFieldKey_del(hTopCollector->hFieldKey);
        hTopCollector->hFieldKey = EGG_NULL;
    }
    
    return EGG_TRUE;
}

PUBLIC EBOOL eggTopCollector_set_cnthits(HEGGTOPCOLLECTOR hTopCollector, count_t cntHits)
{
    if (EGGTOPCOLLECTOR_IS_INVALID(hTopCollector))
    {
        return EGG_FALSE;
    }
    hTopCollector->cntHits = cntHits;

    return EGG_TRUE;
}

PUBLIC EBOOL eggTopCollector_set_sorttype(HEGGTOPCOLLECTOR hTopCollector, type_t sortType)
{
    if (EGGTOPCOLLECTOR_IS_INVALID(hTopCollector))
    {
        return EGG_FALSE;
    }
    hTopCollector->sortType = sortType;

    return EGG_TRUE;
}

PUBLIC EBOOL eggTopCollector_get_sorttype(HEGGTOPCOLLECTOR hTopCollector)
{
    if (EGGTOPCOLLECTOR_IS_INVALID(hTopCollector))
    {
        return EGG_FALSE;
    }
    return hTopCollector->sortType;
}

PUBLIC EBOOL eggTopCollector_get_keyposition2(HEGGTOPCOLLECTOR hTopCollector,
                                             did_t docId, char *fieldName,
                                             char ***keys, size16_t **keysSz,
                                             int ***position, count_t *cnt)
{
    HEGGIDNODE lp_id_nodes = hTopCollector->hIdNodes;
    
    if(!lp_id_nodes || !fieldName || !fieldName[0] || !hTopCollector->hFieldKey)
    {
        if (keys)
            *keys = NULL;
        *keysSz = NULL;
        if (position)
            *position = 0;
        *cnt = 0;
        return EGG_FALSE;
    }

    index_t lower = 0;
    index_t upper = hTopCollector->cntId;
    while (lower < upper)
    {
        index_t i = lower + ((upper - lower) >> 1);
        if (lp_id_nodes[i].id == docId)
        {
            lower = i;
            break;
        }
        else if (lp_id_nodes[i].id > docId)
        {
            lower = i + 1;
        }
        else
        {
            upper = i - 1;
        }
    }

    if (lp_id_nodes[lower].id != docId)
    {
        if (keys)
            *keys = NULL;
        *keysSz = NULL;
        if (position)
            *position = 0;
        *cnt = 0;
        return EGG_FALSE;
    }

    while (lower > 0 && lp_id_nodes[lower - 1].id == docId)
        lower--;
    
    HEGGFIELDKEY hFieldKey = hTopCollector->hFieldKey;
    *cnt = 0;
    if (!hFieldKey)
    {
        if (keys)
            *keys = NULL;
        *keysSz = NULL;
        if (position)
            *position = 0;
        *cnt = 0;
        return EGG_FALSE;
    }
    while(hFieldKey)
    {
        if (strcmp(hFieldKey->fieldName, fieldName) == 0)
            ++*cnt;
        hFieldKey = hFieldKey->next;
    }
    if (keys)
    {
        *keys = (char **)malloc(*cnt * sizeof(char *));
        assert(*keys);
    }
    *keysSz = (size16_t *)malloc(*cnt * sizeof(size16_t));
    assert(*keysSz);
    if (position)
    {
        *position = (int **)malloc(*cnt * sizeof(int *));
        assert(*position);
    }
    int i = 0, j = 0;
    hFieldKey = hTopCollector->hFieldKey;
    while (hFieldKey)
    {
        if (strcmp(hFieldKey->fieldName, fieldName) == 0)
        {
            if (position)
            {
                (*position)[j] = (int *)malloc((EGG_POS_COUNT + 1) *sizeof(int));
                assert((*position)[j]);
                int pi = 0;
                while (pi < EGG_POS_COUNT)
                {
                    (*position)[j][pi] = (int)*(size16_t *)(lp_id_nodes[lower + i].pos + pi * sizeof(size16_t));
                    pi++;
                }
                (*position)[j][EGG_POS_COUNT] = 0;
            }
            (*keysSz)[j] = hFieldKey->keySz;
            if (keys)
            {
                (*keys)[j] = malloc(hFieldKey->keySz);
                assert((*keys)[j]);
                memcpy((*keys)[j], hFieldKey->key, hFieldKey->keySz);
            }
            j++;
        }
        i++;
        hFieldKey = hFieldKey->next;
    }
    return EGG_TRUE;
}

PUBLIC EBOOL eggTopCollector_get_keyposition(HEGGTOPCOLLECTOR hTopCollector,
                                             did_t docId, char *fieldName,
                                             char ***keys, size16_t **keysSz,
                                             int ***position, count_t *cnt)
{
    if(!fieldName || !fieldName[0] || !hTopCollector->hFieldKey)
    {
        if (keys)
            *keys = NULL;
        *keysSz = NULL;
        if (position)
            *position = 0;
        *cnt = 0;
        return EGG_FALSE;
    }
    /* omit docId, position*/
    docId = 0 ;
    if (position)
    {
        *position = 0;
    }


    HEGGFIELDKEY hFieldKey = hTopCollector->hFieldKey;
    *cnt = 0;

    while(hFieldKey)
    {
        if (strcmp(hFieldKey->fieldName, fieldName) == 0)
            ++*cnt;
        hFieldKey = hFieldKey->next;
    }
    if (!*cnt)
    {
        if (keys)
        {
            *keys = 0;
        }
        *keysSz = 0;
        return EGG_FALSE;
    }
    
    if (keys)
    {
        *keys = (char **)malloc(*cnt * sizeof(char *));
        assert(*keys);
    }
    *keysSz = (size16_t *)malloc(*cnt * sizeof(size16_t));
    assert(*keysSz);
    int j = 0;
    hFieldKey = hTopCollector->hFieldKey;
    while (hFieldKey)
    {
        if (strcmp(hFieldKey->fieldName, fieldName) == 0)
        {
            (*keysSz)[j] = hFieldKey->keySz;
            if (keys)
            {
                (*keys)[j] = malloc(hFieldKey->keySz);
                assert((*keys)[j]);
                memcpy((*keys)[j], hFieldKey->key, hFieldKey->keySz);
            }
            j++;
        }

        hFieldKey = hFieldKey->next;
    }
    return EGG_TRUE;
}


PUBLIC EBOOL eggTopCollector_normalized(HEGGTOPCOLLECTOR hTopCollector, type_t op)
{
    if (EGGTOPCOLLECTOR_IS_INVALID(hTopCollector))
    {
        return EGG_FALSE;
    }
    if(!hTopCollector->cntId)
    {
        return EGG_FALSE;
    }
    
  
    if(EGG_TOPSORT_SCORE & op)
    {
        return eggTopCollector_normalized_score(hTopCollector);        
    }
    /* else if(EGG_TOPSORT_ORDERBY & op) */
    /* { */
    /*     return eggTopCollector_normalized_orderby(hTopCollector); */
    /* } */
    else if(EGG_TOPSORT_ORDERBY & op)
    {
        return EGG_TRUE;
    }
    else if(EGG_TOPSORT_WEIGHT & op)
    {
        return eggTopCollector_normalized_weight(hTopCollector);
    }
    else if(EGG_TOPSORT_NOT & op)
    {
        return eggTopCollector_normalized_not_score(hTopCollector);
    }
    return EGG_FALSE;
}


PUBLIC EBOOL eggTopCollector_ultimatenormalized(HEGGTOPCOLLECTOR hTopCollector)
{
    if (EGGTOPCOLLECTOR_IS_INVALID(hTopCollector))
    {
        return EGG_FALSE;
    }

    if(!eggTopCollector_normalized(hTopCollector, hTopCollector->sortType))
    {
        return EGG_FALSE;
    }
    if(hTopCollector->idRange)
    {
        free(hTopCollector->idRange);
        hTopCollector->idRange = NULL;
        hTopCollector->cntRange = 0;
    }
    
    if(hTopCollector->cntId)
    {
        free(hTopCollector->hIdNodes);
        hTopCollector->hIdNodes = NULL;
        hTopCollector->cntId = 0;
    }    
    
    
    return EGG_TRUE;
}



PUBLIC EBOOL eggTopCollector_cut_result(HEGGTOPCOLLECTOR hTopCollector, HEGGSEARCHITER hIter)
{
    if (EGGTOPCOLLECTOR_IS_INVALID(hTopCollector))
    {
        return EGG_FALSE;
    }
    

    if(hIter->scoreDocIdx == EGG_PAGEFRONT)
    {
        hIter->scoreDocIdx = hTopCollector->cntHits - 1;
    }
    else if(hIter->scoreDocIdx == EGG_PAGEBACK)
    {
        hIter->scoreDocIdx = 0; 
    }
    
    if(hIter->scoreDocIdx + hIter->iterCnt < 0 )
    {
        hIter->iterCnt += (hIter->scoreDocIdx + 1 > 0? hIter->scoreDocIdx + 1 : 0);
        hIter->scoreDocIdx = EGG_PAGEFRONT;
        hTopCollector->cntHits = 0;
        free(hTopCollector->docs);
        return EGG_TRUE;
    }
    else if(hIter->scoreDocIdx + hIter->iterCnt >= hTopCollector->cntHits)
    {
        hIter->iterCnt -= hTopCollector->cntHits - hIter->scoreDocIdx; 
        hIter->scoreDocIdx = EGG_PAGEBACK;
        hTopCollector->cntHits = 0;
        free(hTopCollector->docs);
        return EGG_TRUE;
    }
    else
    {
        hIter->scoreDocIdx += hIter->iterCnt;
        hIter->iterCnt = 0;
        if((hTopCollector->cntHits - hIter->scoreDocIdx) <= hIter->unitCnt)
        {
            hTopCollector->cntHits = hTopCollector->cntHits - hIter->scoreDocIdx;
            hIter->unitCnt = hIter->unitCnt - hTopCollector->cntHits;
            
            EMemMove(hTopCollector->docs, hTopCollector->docs + hIter->scoreDocIdx,
                     hTopCollector->cntHits * sizeof(EGGSCOREDOC));

            hIter->scoreDocIdx = EGG_PAGEBACK;
        }
        else
        {
            hTopCollector->cntHits = hIter->unitCnt;
            hIter->unitCnt = 0;
            
            EMemMove(hTopCollector->docs, hTopCollector->docs + hIter->scoreDocIdx,
                     hTopCollector->cntHits * sizeof(EGGSCOREDOC));
            
            hIter->scoreDocIdx += hTopCollector->cntHits;

        }
            
    }
    
    
    return EGG_TRUE;
}


PUBLIC HEGGTOPCTORCHUNK eggTopCollector_serialization(HEGGTOPCOLLECTOR hTopCollector)
{
    size32_t n_fieldKey;
    char *p_fieldKey;
    p_fieldKey = eggFieldKey_serialise(hTopCollector->hFieldKey, &n_fieldKey);

    size32_t sz_orderBy = 0;
    char *p_orderBy = NULL;
    p_orderBy = eggOrderBy_serialise(hTopCollector->hOrderBy,
                                     hTopCollector->cntOrderBy,
                                     &sz_orderBy);

    size32_t n_buf_size = sizeof(EGGTOPCTORCHUNK) +
        (hTopCollector->docs ? hTopCollector->cntHits : 0 ) * sizeof(EGGSCOREDOC) +
        hTopCollector->cntId * sizeof(EGGIDNODE) + n_fieldKey + sz_orderBy;

    HEGGTOPCTORCHUNK lp_topctor_chunk = (HEGGTOPCTORCHUNK)malloc(n_buf_size);
    
    lp_topctor_chunk->size = n_buf_size;
    lp_topctor_chunk->cntTotal = hTopCollector->cntTotal;
    lp_topctor_chunk->cntHits = (hTopCollector->docs ? hTopCollector->cntHits : 0 );
    lp_topctor_chunk->cntQuery = hTopCollector->cntHits;

    lp_topctor_chunk->cntRange = hTopCollector->cntRange;
    lp_topctor_chunk->cntId = hTopCollector->cntId;
    lp_topctor_chunk->szFieldKey = n_fieldKey;
    lp_topctor_chunk->szOrderBy = sz_orderBy;
    lp_topctor_chunk->sortType = hTopCollector->sortType;
    
    char* p_buf = (char*)(lp_topctor_chunk + 1);
    
    if(lp_topctor_chunk->cntHits)
        memcpy(EGGTOPCTORCHUNK_DOCS(lp_topctor_chunk), hTopCollector->docs, hTopCollector->cntHits * sizeof(EGGSCOREDOC));

    if(lp_topctor_chunk->cntRange)
        memcpy(EGGTOPCTORCHUNK_IDRANGE(lp_topctor_chunk),
               hTopCollector->idRange, hTopCollector->cntRange * sizeof(did_t));
    

    if(lp_topctor_chunk->cntId)
        memcpy(EGGTOPCTORCHUNK_IDNODES(lp_topctor_chunk),
               hTopCollector->hIdNodes, hTopCollector->cntId * sizeof(EGGIDNODE));

    if (lp_topctor_chunk->szFieldKey)
    {

        memcpy(EGGTOPCTORCHUNK_FIELDKEY(lp_topctor_chunk),
               p_fieldKey, n_fieldKey);
        free(p_fieldKey);
    }

    if (lp_topctor_chunk->szOrderBy)
    {

        memcpy(EGGTOPCTORCHUNK_ORDERBY(lp_topctor_chunk),
               p_orderBy, sz_orderBy);
        free(p_orderBy);
    }


    return lp_topctor_chunk;
}

PUBLIC HEGGTOPCOLLECTOR eggTopCollector_unserialization(HEGGTOPCTORCHUNK hTopCtorChunk)
{
    HEGGTOPCOLLECTOR lp_top_collector = (HEGGTOPCOLLECTOR)malloc(sizeof(EGGTOPCOLLECTOR));
    memset(lp_top_collector, 0, sizeof(EGGTOPCOLLECTOR));
    
    lp_top_collector->cntTotal = hTopCtorChunk->cntTotal;
    lp_top_collector->cntHits = hTopCtorChunk->cntQuery;
    lp_top_collector->cntRange = hTopCtorChunk->cntRange;
    lp_top_collector->cntId = hTopCtorChunk->cntId;
    lp_top_collector->sortType = hTopCtorChunk->sortType;

    if(hTopCtorChunk->cntHits)
    {
        lp_top_collector->docs = (HEGGSCOREDOC)malloc(sizeof(EGGSCOREDOC) * hTopCtorChunk->cntHits);
        memcpy(lp_top_collector->docs, EGGTOPCTORCHUNK_DOCS(hTopCtorChunk), sizeof(EGGSCOREDOC) * hTopCtorChunk->cntHits);
    }

    if(hTopCtorChunk->cntRange)
    {
        lp_top_collector->idRange = (did_t*)malloc(sizeof(did_t) * hTopCtorChunk->cntRange);
        memcpy(lp_top_collector->idRange, EGGTOPCTORCHUNK_IDRANGE(hTopCtorChunk), sizeof(did_t) * hTopCtorChunk->cntRange);
    }


    if(hTopCtorChunk->cntId)
    {
        lp_top_collector->hIdNodes = (HEGGIDNODE)malloc(sizeof(EGGIDNODE) * hTopCtorChunk->cntId);
        memcpy(lp_top_collector->hIdNodes, EGGTOPCTORCHUNK_IDNODES(hTopCtorChunk), sizeof(EGGIDNODE) * hTopCtorChunk->cntId);
    }
    
    if (hTopCtorChunk->szFieldKey)
    {
        lp_top_collector->hFieldKey = eggFieldKey_unserialise(EGGTOPCTORCHUNK_FIELDKEY(hTopCtorChunk));
    }

    if (hTopCtorChunk->szOrderBy)
    {
        lp_top_collector->hOrderBy = eggOrderBy_unserialise(EGGTOPCTORCHUNK_ORDERBY(hTopCtorChunk), hTopCtorChunk->szOrderBy, &lp_top_collector->cntOrderBy);
    }
    
    return lp_top_collector;
}

PRIVATE char *eggOrderBy_serialise(HEGGORDERBY hOrderBy,
                                   count_t cntOrderBy,
                                   size32_t *szOrderBy)
{
    if (!hOrderBy || cntOrderBy == 0)
    {
        *szOrderBy = 0;
        return NULL;
    }
    
    *szOrderBy = sizeof(count_t);
    count_t idx = 0;
    while (idx < cntOrderBy)
    {
        *szOrderBy += strlen(hOrderBy[idx].fieldName)+1 + sizeof(EBOOL);
        idx++;
    }
    char *buf = malloc(*szOrderBy);
    assert(buf);
    idx = 0;
    char *p = buf;
    *(count_t *)p = cntOrderBy;
    p += sizeof(count_t);
    while (idx < cntOrderBy)
    {
        strcpy(p, hOrderBy[idx].fieldName);
        p += strlen(hOrderBy[idx].fieldName) + 1;
        *(EBOOL *)p = hOrderBy[idx].isAsc;
        p += sizeof(EBOOL);
        idx++;
    }
    return buf;
}

PRIVATE HEGGORDERBY eggOrderBy_unserialise(char *buf, size32_t size,
                                           count_t *cntOrderBy)
{
    if (!buf || size < sizeof(count_t))
    {
        *cntOrderBy = 0;
        return NULL;
    }
    char *p = buf;
    *cntOrderBy = *(count_t *)p;
    p += sizeof(count_t);
    HEGGORDERBY hOrderBy = calloc(*cntOrderBy, sizeof(EGGORDERBY));
    assert(hOrderBy);
    count_t idx = 0;
    while (p < buf + size && idx < *cntOrderBy)
    {
        hOrderBy[idx].fieldName = strdup(p);
        assert(hOrderBy[idx].fieldName);
        p += strlen(hOrderBy[idx].fieldName) + 1;
        hOrderBy[idx].isAsc = *(EBOOL*)p;
        p += sizeof(EBOOL);
        idx++;
    }
    if (idx != *cntOrderBy || p > buf + size)
    {
        /* fprintf(stderr, "%s:%d:%s violate (idx %llu == *cntOrderBy %llu) && (p <= buf + size %llu)\n", */
        /*         __FILE__, __LINE__, __func__, (long long unsigned)idx, */
        /*         (long long unsigned)*cntOrderBy, */
        /*         (long long unsigned)size); */
        eggPrtLog_error("eggTopCollector", "%s:%d:%s violate (idx %llu == *cntOrderBy %llu) && (p <= buf + size %llu)\n",
                __FILE__, __LINE__, __func__, (long long unsigned)idx,
                (long long unsigned)*cntOrderBy,
                (long long unsigned)size);

        count_t i;
        for (i = 0; i < idx; i++)
        {
            free(hOrderBy[i].fieldName);
        }

        free(hOrderBy);
        *cntOrderBy = 0;
        return NULL;
    }
    return hOrderBy;
}

PRIVATE HEGGORDERBY eggOrderBy_dup(HEGGORDERBY hOrderBy, count_t cntOrderBy, count_t *nOrderBy)
{
    if (!hOrderBy || !cntOrderBy)
    {
        *nOrderBy = 0;
        return NULL;
    }
    
    *nOrderBy = cntOrderBy;
    HEGGORDERBY pOrderBy = (HEGGORDERBY) calloc(cntOrderBy, sizeof(EGGORDERBY));
    assert(pOrderBy);
    
    count_t ind;
    for (ind = 0; ind != cntOrderBy; ind++)
    {
        pOrderBy[ind].fieldName = strdup(hOrderBy[ind].fieldName);
        assert(pOrderBy[ind].fieldName);
        pOrderBy[ind].isAsc = hOrderBy[ind].isAsc;
    }
    return pOrderBy;
}


PUBLIC HEGGTOPCOLLECTOR eggTopCollector_merge_with_ref(HEGGTOPCOLLECTOR hDestCollector, HEGGTOPCOLLECTOR hSrcCollector)
{
    if (EGGTOPCOLLECTOR_IS_INVALID(hDestCollector))
    {
        return EGG_FALSE;
    }
    
    if (EGGTOPCOLLECTOR_IS_INVALID(hSrcCollector))
    {
        return EGG_FALSE;
    }
    if(hDestCollector->docs) free(hDestCollector->docs);
    if(hDestCollector->idRange) free(hDestCollector->idRange);
    if(hDestCollector->hIdNodes) free(hDestCollector->hIdNodes);
    if(hDestCollector->hFieldKey) eggFieldKey_del(hDestCollector->hFieldKey);
    if(hDestCollector->hOrderBy)
    {
        eggOrderBy_delete(hDestCollector->hOrderBy, hDestCollector->cntOrderBy);
    }
    if(hDestCollector->hIdRangeOrder)
    {
        eggIdRangeOrder_delete(hDestCollector->hIdRangeOrder, hDestCollector->cntIdRangeOrder);
    }

    
    memcpy(hDestCollector, hSrcCollector, sizeof(EGGTOPCOLLECTOR));
    
    free(hSrcCollector);
    
    return hDestCollector;
        
}


PUBLIC EBOOL eggTopCollector_set_chunkid_cluster(HEGGTOPCOLLECTOR hCollector, SPANPOINT chunkId)
{
    if (EGGTOPCOLLECTOR_IS_INVALID(hCollector))
    {
        return EGG_FALSE;
    }
    index_t n_docscore_idx = 0;
    while(n_docscore_idx < hCollector->cntHits)
    {
        hCollector->docs[n_docscore_idx].idDoc.cluster.chunkId = chunkId;
        
        n_docscore_idx++;
    }
    return EGG_TRUE;
}

PUBLIC EBOOL eggTopCollector_merge_with_cluster(HEGGTOPCOLLECTOR lp_top_collector, count_t collectorCnt, HEGGTOPCOLLECTOR* lpTopCollectorSet)
{
    if (!collectorCnt)
    {
        return EGG_FALSE;
    }

    index_t n_collect_idx = 0;
    UTIVECTOR* lp_docs_vector = Uti_vector_create(sizeof(EGGSCOREDOC));
    UTIVECTOR* lp_idRange_vector = Uti_vector_create(sizeof(did_t));
    UTIVECTOR* lp_hIdNodes_vector = Uti_vector_create(sizeof(EGGIDNODE));
    count_t n_doc_total = 0;

    n_doc_total += lp_top_collector->cntTotal;
    Uti_vector_push(lp_docs_vector, lp_top_collector->docs, lp_top_collector->cntHits);
    Uti_vector_push(lp_idRange_vector, lp_top_collector->idRange, lp_top_collector->cntRange);
    Uti_vector_push(lp_hIdNodes_vector, lp_top_collector->hIdNodes, lp_top_collector->cntId);
    
    while (n_collect_idx < collectorCnt)
    {
        HEGGTOPCOLLECTOR lp_collector_tmp = lpTopCollectorSet[n_collect_idx];
        Uti_vector_push(lp_docs_vector, lp_collector_tmp->docs, lp_collector_tmp->cntHits);
        Uti_vector_push(lp_idRange_vector, lp_collector_tmp->idRange, lp_collector_tmp->cntRange);
        Uti_vector_push(lp_hIdNodes_vector, lp_collector_tmp->hIdNodes, lp_collector_tmp->cntId);
        n_doc_total += lp_collector_tmp->cntTotal;
        //if(!n_collect_idx)
        if (!lp_top_collector->hFieldKey)
        {
            lp_top_collector->hFieldKey = eggFieldKey_dup(lp_collector_tmp->hFieldKey);
        }
        
        n_collect_idx++;

    }


    free(lp_top_collector->docs);
    free(lp_top_collector->idRange);
    free(lp_top_collector->hIdNodes);
    
    lp_top_collector->docs = Uti_vector_data(lp_docs_vector);
    lp_top_collector->idRange = Uti_vector_data(lp_idRange_vector);
    lp_top_collector->hIdNodes = Uti_vector_data(lp_hIdNodes_vector);
    lp_top_collector->cntTotal = n_doc_total;
    lp_top_collector->cntHits = Uti_vector_count(lp_docs_vector);
    lp_top_collector->cntRange = Uti_vector_count(lp_idRange_vector);
    lp_top_collector->cntId = Uti_vector_count(lp_hIdNodes_vector);

    Uti_vector_destroy(lp_docs_vector, EGG_FALSE);
    Uti_vector_destroy(lp_idRange_vector, EGG_FALSE);
    Uti_vector_destroy(lp_hIdNodes_vector, EGG_FALSE);


    return EGG_TRUE;
        
}

PUBLIC EBOOL eggTopCollector_set_filterweight(HEGGTOPCOLLECTOR hTopCollector, int weight_start, int weight_end)
{
    if (EGGTOPCOLLECTOR_IS_INVALID(hTopCollector))
    {
        return EGG_FALSE;
    }
    hTopCollector->filterWeight_start = weight_start;
    hTopCollector->filterWeight_end = weight_end;
    
    hTopCollector->sortType = EGG_TOPSORT_WEIGHT;

    return EGG_TRUE;
}


PUBLIC EBOOL EGGAPI  eggTopCollector_filter_weight(HEGGTOPCOLLECTOR hTopCollector, int weight_start, int weight_end)
{
    HEGGSCOREDOC docs = hTopCollector->docs;
    if(!docs)
    {
        hTopCollector->cntHits = 0;
        return EGG_FALSE;
    }
    eggTopCollector_sort_with_weight(hTopCollector);
    
    EGGSCOREDOC st_scoredoc = {};
    st_scoredoc.weight = weight_start;
    index_t start = 0;


    Uti_binary_lookup(hTopCollector->docs, hTopCollector->cntHits - 1,
                      &st_scoredoc, &start, eggScoreDoc_cmp_weight2);
    if (start >= 0 && start < hTopCollector->cntHits
        && docs[start].weight > weight_start)
    {
        start++;
    }
    while (start > 0 && start < hTopCollector->cntHits
	   && docs[start].weight == weight_start)
    {
            start++;
    }

    st_scoredoc.weight = weight_end;
    index_t end = 0;
    Uti_binary_lookup(hTopCollector->docs, hTopCollector->cntHits - 1,
                      &st_scoredoc, &end, eggScoreDoc_cmp_weight2);
    while (end > 0 && end < hTopCollector->cntHits
	   && docs[end].weight == weight_end)
    {
        end--;
    }
    if (end >= 0 && end < hTopCollector->cntHits
        && docs[end].weight > weight_end)
    {
        end++;
    }

    HEGGSCOREDOC new_scoredoc = NULL;
    count_t n_new_scoredoc = 0;
    if (start > end)
    {
        n_new_scoredoc = start - end;
        new_scoredoc = malloc(n_new_scoredoc * sizeof(EGGSCOREDOC));
        assert(new_scoredoc);
        memcpy(new_scoredoc, &docs[end], n_new_scoredoc * sizeof(EGGSCOREDOC));
    }
    free(hTopCollector->docs);
    hTopCollector->docs = new_scoredoc;
    hTopCollector->cntHits = n_new_scoredoc;
    hTopCollector->cntTotal = n_new_scoredoc;
    return EGG_TRUE;
}

EBOOL EGGAPI eggTopCollector_filter_weightstring(HEGGTOPCOLLECTOR hTopCollector, char *weightStart, char *weightEnd)
{
    if (!hTopCollector || !weightStart || !weightStart[0] || !weightEnd || !weightEnd[0])
    {
        return EGG_FALSE;
    }
    
    int weight_start, weight_end;
    char *ptmp;
    char *s , *d;
    ptmp = strdup(weightStart);
    assert(ptmp);
    s = d = ptmp;
    while (*s)
    {
        if (*s == '-')
        {
            s++;
            continue;
        }
        *d++ = *s++;
    }
    *d = '\0';
    weight_start = atoi(ptmp);
    free(ptmp);

    ptmp = strdup(weightEnd);
    assert(ptmp);
    s = d = ptmp;
    while (*s)
    {
        if (*s == '-')
        {
            s++;
            continue;
        }
        *d++ = *s++;
    }
    *d = '\0';
    weight_end = atoi(ptmp);
    free(ptmp);
                
    return eggTopCollector_filter_weight(hTopCollector, weight_start, weight_end);
}


PUBLIC EBOOL eggTopCollector_sort_with_score(HEGGTOPCOLLECTOR hTopCollector)
{
    Uti_sedgesort (hTopCollector->docs, hTopCollector->cntHits, sizeof(EGGSCOREDOC), eggScoreDoc_cmp_score);
    return EGG_TRUE;
}

PUBLIC EBOOL eggTopCollector_sort_with_weight(HEGGTOPCOLLECTOR hTopCollector)
{
    Uti_sedgesort (hTopCollector->docs, hTopCollector->cntHits, sizeof(EGGSCOREDOC), eggScoreDoc_cmp_weight);
    return EGG_TRUE;
}

PRIVATE EBOOL eggTopCollector_normalized_weight(HEGGTOPCOLLECTOR hTopCollector)
{
    HEGGIDNODE lp_id_nodes = hTopCollector->hIdNodes;
    
    if(!lp_id_nodes)
    {
        hTopCollector->cntHits = 0;
        return EGG_FALSE;
    }
    if(hTopCollector->docs)
    {
        free(hTopCollector->docs);
        hTopCollector->docs = EGG_NULL;
    }    


    UTIVECTOR* lp_vector = Uti_vector_create(sizeof(EGGSCOREDOC));
    EGGSCOREDOC st_score_doc;
    index_t n_ids_idx = 0;
    struct timeval tv1, tv2;
    gettimeofday(&tv1, 0);
    while(n_ids_idx != hTopCollector->cntId)
    {
        count_t n_equalId_cnt = 0;
        while((lp_id_nodes)[n_ids_idx].id == (lp_id_nodes)[(n_ids_idx)+(n_equalId_cnt)].id)
        {
            (n_equalId_cnt)++;
            if(n_equalId_cnt + n_ids_idx == hTopCollector->cntId) break;
        }
             
            
        EGGDID_DOCID(&st_score_doc.idDoc) = lp_id_nodes[n_ids_idx].id;
        st_score_doc.weight = (double)lp_id_nodes[n_ids_idx].weight;;    
        Uti_vector_push(lp_vector, &st_score_doc, 1);
        n_ids_idx += n_equalId_cnt;
    }
        gettimeofday(&tv2, 0);
//	printf("score time : %f\n", (double)(tv2.tv_sec - tv1.tv_sec) + (double)(tv2.tv_usec - tv1.tv_usec)/1000000);
    hTopCollector->cntHits = hTopCollector->cntHits && hTopCollector->cntHits < Uti_vector_count(lp_vector) ?
        hTopCollector->cntHits : Uti_vector_count(lp_vector);
    
    hTopCollector->cntTotal = hTopCollector->cntHits;
    if(hTopCollector->docs)
    {
        free(hTopCollector->docs);
        hTopCollector->docs = 0;
    }
    
    hTopCollector->docs = Uti_vector_data(lp_vector);
    
    Uti_sedgesort (hTopCollector->docs, hTopCollector->cntHits, sizeof(EGGSCOREDOC), eggScoreDoc_cmp_weight);
    Uti_vector_destroy(lp_vector, EGG_FALSE);
    hTopCollector->sortType = EGG_TOPSORT_WEIGHT;
    
    if (!(hTopCollector->filterWeight_start == 0 && hTopCollector->filterWeight_end == 0))
    {
        eggTopCollector_filter_weight(hTopCollector, hTopCollector->filterWeight_start, hTopCollector->filterWeight_end);
        hTopCollector->cntTotal = hTopCollector->cntHits;
    }
    return EGG_TRUE;
}

PRIVATE EBOOL eggTopCollector_normalized_score(HEGGTOPCOLLECTOR hTopCollector)
{
    HEGGIDNODE lp_id_nodes = hTopCollector->hIdNodes;
    
    if(!lp_id_nodes)
    {
        hTopCollector->cntHits = 0;
        return EGG_FALSE;
    }

    if(hTopCollector->docs)
    {
        free(hTopCollector->docs);
        hTopCollector->docs = EGG_NULL;
    }    

    UTIVECTOR* lp_vector = Uti_vector_create(sizeof(EGGSCOREDOC));
    EGGSCOREDOC st_score_doc;
    index_t n_ids_idx = 0;

    while(n_ids_idx != hTopCollector->cntId)
    {
        count_t n_equalId_cnt = 0;
        while((lp_id_nodes)[n_ids_idx].id == (lp_id_nodes)[(n_ids_idx)+(n_equalId_cnt)].id)
        {
            (n_equalId_cnt)++;
            if(n_equalId_cnt + n_ids_idx == hTopCollector->cntId) break;
        }
             
            
        EGGDID_DOCID(&st_score_doc.idDoc) = lp_id_nodes[n_ids_idx].id;
        similar_score_document(lp_id_nodes + n_ids_idx, n_equalId_cnt, &st_score_doc);
        st_score_doc.weight = lp_id_nodes[n_ids_idx].weight;
        
        Uti_vector_push(lp_vector, &st_score_doc, 1);
        n_ids_idx += n_equalId_cnt;
    }

    hTopCollector->cntHits = hTopCollector->cntHits && hTopCollector->cntHits < Uti_vector_count(lp_vector) ?
        hTopCollector->cntHits : Uti_vector_count(lp_vector);
    
    hTopCollector->cntTotal = hTopCollector->cntHits;
    
    hTopCollector->docs = Uti_vector_data(lp_vector);
    eggTopCollector_scoreLinear(hTopCollector);    
    Uti_sedgesort (hTopCollector->docs, hTopCollector->cntHits, sizeof(EGGSCOREDOC), eggScoreDoc_cmp_score);
    Uti_vector_destroy(lp_vector, EGG_FALSE);
    hTopCollector->sortType = EGG_TOPSORT_SCORE;

    return EGG_TRUE;
}



PRIVATE EBOOL eggTopCollector_normalized_not_score(HEGGTOPCOLLECTOR hTopCollector)
{
    HEGGIDNODE lp_id_nodes = hTopCollector->hIdNodes;
    
    if(!lp_id_nodes)
    {
        hTopCollector->cntHits = 0;
        return EGG_FALSE;
    }
    if(hTopCollector->docs)
    {
        free(hTopCollector->docs);
        hTopCollector->docs = EGG_NULL;
    }    
    

    UTIVECTOR* lp_vector = Uti_vector_create(sizeof(EGGSCOREDOC));
    EGGSCOREDOC st_score_doc;
    index_t n_ids_idx = 0;

    while(n_ids_idx != hTopCollector->cntId)
    {
        count_t n_equalId_cnt = 0;

        while((lp_id_nodes)[n_ids_idx].id == (lp_id_nodes)[(n_ids_idx)+(n_equalId_cnt)].id)
        {
            (n_equalId_cnt)++;
            if(n_equalId_cnt + n_ids_idx == hTopCollector->cntId) break;
        }
        
        
//        EGGIDNODE_EQUALID_CNT(lp_id_nodes, n_ids_idx, n_equalId_cnt);
            
        EGGDID_DOCID(&st_score_doc.idDoc) = lp_id_nodes[n_ids_idx].id;
            
        Uti_vector_push(lp_vector, &st_score_doc, 1);
        n_ids_idx += n_equalId_cnt;
    }
    
    hTopCollector->cntHits = hTopCollector->cntHits && hTopCollector->cntHits < Uti_vector_count(lp_vector) ?
        hTopCollector->cntHits : Uti_vector_count(lp_vector);
    
    hTopCollector->cntTotal = hTopCollector->cntHits;
    
    hTopCollector->docs = Uti_vector_data(lp_vector);
    
    Uti_vector_destroy(lp_vector, EGG_FALSE);
    hTopCollector->sortType = EGG_TOPSORT_NOT;

    return EGG_TRUE;
}

PRIVATE HEGGWRESULT eggWResult_get_fromIdRangeOrder(EGGIDRANGEORDER *hIdRangeOrder,
                                                    count_t cntIdRangeOrder,
                                                    count_t *p_nWeightResult, fdid_t fid)
{
    HEGGWRESULT hWeightResult = NULL;
    *p_nWeightResult = 0;

    if (cntIdRangeOrder == 0 || !hIdRangeOrder)
    {
        return NULL;
    }
    count_t idx;
    for (idx = 0; idx < cntIdRangeOrder; idx++)
    {
        if (hIdRangeOrder[idx].fdid == fid)
        {
            *p_nWeightResult = hIdRangeOrder[idx].cntRange;
            hWeightResult = malloc(sizeof(*hWeightResult) * *p_nWeightResult);
            assert(hWeightResult);
            memcpy(hWeightResult, hIdRangeOrder[idx].idRange, sizeof(*hWeightResult) * *p_nWeightResult);
            Uti_sedgesort (hWeightResult, *p_nWeightResult, sizeof(EGGWRESULT), eggWResult_cmpid_desc);
        }
    }
    return hWeightResult;
}
PRIVATE EBOOL eggTopCollector_order_throughIdRangeOrder(HEGGTOPCOLLECTOR hTopCollector, fdid_t fid, type_t field_type, int isAsc)
{

    HEGGWRESULT hWeightResult = NULL;
    count_t nWeightResult = 0;
    hWeightResult = eggWResult_get_fromIdRangeOrder(hTopCollector->hIdRangeOrder,
                                                    hTopCollector->cntIdRangeOrder,
                                                    &nWeightResult, fid);
    if (!hWeightResult)
    {
        return EGG_FALSE;
    }

    EBOOL ret = EGG_TRUE;

    index_t n_scoredoc_idx = 0;
    HEGGSCOREDOC hDocs = hTopCollector->docs;
    while (n_scoredoc_idx != hTopCollector->cntHits)
    {
    
        index_t lookup_idx = 0;
        EGGWRESULT st_wr;
        st_wr.id = EGGDID_DOCID(&hDocs[n_scoredoc_idx].idDoc);        
        if (EGG_TRUE == Uti_binary_lookup(hWeightResult,
                                          nWeightResult-1, &st_wr,
                                          &lookup_idx, eggWResult_cmpid2_desc))
        {
            memcpy(hDocs[n_scoredoc_idx].orderBy,
                   hWeightResult[lookup_idx].val,
                   sizeof(hDocs[n_scoredoc_idx].orderBy));
        }
        else
        {
            memset(hDocs[n_scoredoc_idx].orderBy, 0,
                   sizeof(hDocs[n_scoredoc_idx].orderBy));
            /* fprintf(stderr, "%s:%d:%s docid[%llu] does not have weightResult from RangeIndex\n", */
            /*         __FILE__, __LINE__, __func__, (long long unsigned)st_wr.id); */
            eggPrtLog_warn("eggTopCollector", "%s:%d:%s docid[%llu] does not have weightResult from RangeIndex\n",
                    __FILE__, __LINE__, __func__, (long long unsigned)st_wr.id);

            ret = EGG_FALSE;
            goto failed;
        }
        
        hDocs[n_scoredoc_idx].sort_tmp_val = (int)n_scoredoc_idx;
            
        n_scoredoc_idx++;        
    }

    if (isAsc)
    {
        if (field_type & EGG_INDEX_INT32)
        {
            Uti_sedgesort(hTopCollector->docs, hTopCollector->cntHits, sizeof(EGGSCOREDOC), eggScoreDoc_cmp_orderbytmpval_int32_desc);
//                Uti_insort(hTopCollector->docs, hTopCollector->cntHits, sizeof(EGGSCOREDOC), eggScoreDoc_cmp_orderby_int32_desc);
        }
        else if (field_type & EGG_INDEX_INT64)
        {
            Uti_sedgesort(hTopCollector->docs, hTopCollector->cntHits, sizeof(EGGSCOREDOC), eggScoreDoc_cmp_orderbytmpval_int64_desc);
//                Uti_insort(hTopCollector->docs, hTopCollector->cntHits, sizeof(EGGSCOREDOC), eggScoreDoc_cmp_orderby_int64_desc);
        }
        else if (field_type & EGG_INDEX_DOUBLE)
        {
            Uti_sedgesort(hTopCollector->docs, hTopCollector->cntHits, sizeof(EGGSCOREDOC), eggScoreDoc_cmp_orderbytmpval_double_desc);
//                Uti_insort(hTopCollector->docs, hTopCollector->cntHits, sizeof(EGGSCOREDOC), eggScoreDoc_cmp_orderby_double_desc);
        }
    }
    else
    {
        if (field_type & EGG_INDEX_INT32)
        {
            Uti_sedgesort(hTopCollector->docs, hTopCollector->cntHits, sizeof(EGGSCOREDOC), eggScoreDoc_cmp_orderbytmpval_int32_asc);
//                Uti_insort(hTopCollector->docs, hTopCollector->cntHits, sizeof(EGGSCOREDOC), eggScoreDoc_cmp_orderby_int32_asc);
        }
        else if (field_type & EGG_INDEX_INT64)
        {
            Uti_sedgesort(hTopCollector->docs, hTopCollector->cntHits, sizeof(EGGSCOREDOC), eggScoreDoc_cmp_orderbytmpval_int64_asc);
//                Uti_insort(hTopCollector->docs, hTopCollector->cntHits, sizeof(EGGSCOREDOC), eggScoreDoc_cmp_orderby_int64_asc);
        }
        else if (field_type & EGG_INDEX_DOUBLE)
        {
            Uti_sedgesort(hTopCollector->docs, hTopCollector->cntHits, sizeof(EGGSCOREDOC), eggScoreDoc_cmp_orderbytmpval_double_asc);
//                Uti_insort(hTopCollector->docs, hTopCollector->cntHits, sizeof(EGGSCOREDOC), eggScoreDoc_cmp_orderby_double_asc);
        }
    }

failed:
    free(hWeightResult);
        
    return ret;
}

PRIVATE HEGGWRESULT eggWResult_get_fromFieldWeight(HEGGFIELDWEIGHT hFieldWeight,  count_t *p_nWeightResult, fdid_t fid)
{
    HEGGWRESULT hWeightResult = NULL;
    *p_nWeightResult = 0;
    if (fid > 0)
    {
        eggFieldView_xlock(hFieldWeight->hFieldView, fid);
        hWeightResult = eggFieldWeight_get_withfid(hFieldWeight,
                                                   fid,
                                                   p_nWeightResult);
        eggFieldView_unlock(hFieldWeight->hFieldView, fid);
        
    }
    return hWeightResult;
}
PRIVATE EBOOL eggTopCollector_order_throughFieldWeight(HEGGTOPCOLLECTOR hTopCollector,
                                                       HEGGFIELDWEIGHT hFieldWeight, fdid_t fid, type_t field_type, int isAsc)
{
    HEGGWRESULT hWeightResult = NULL;
    count_t nWeightResult = 0;
    hWeightResult = eggWResult_get_fromFieldWeight(hFieldWeight,
                                                   &nWeightResult, fid);
    if (!hWeightResult)
    {
        return EGG_FALSE;
    }

    EBOOL ret = EGG_TRUE;

    index_t n_scoredoc_idx = 0;
    HEGGSCOREDOC hDocs = hTopCollector->docs;
    while (n_scoredoc_idx != hTopCollector->cntHits)
    {
    
        index_t lookup_idx = 0;
        EGGWRESULT st_wr;
        st_wr.id = EGGDID_DOCID(&hDocs[n_scoredoc_idx].idDoc);
        if (EGG_TRUE == Uti_binary_lookup(hWeightResult,
                                          nWeightResult-1, &st_wr,
                                          &lookup_idx, eggWResult_cmpid2_desc))
        {
            memcpy(hDocs[n_scoredoc_idx].orderBy,
                   hWeightResult[lookup_idx].val,
                   sizeof(hDocs[n_scoredoc_idx].orderBy));
        }
        else
        {
            memset(hDocs[n_scoredoc_idx].orderBy, 0,
                   sizeof(hDocs[n_scoredoc_idx].orderBy));
            /* fprintf(stderr, "%s:%d:%s docid[%llu] does not have weightResult from FieldWeight\n", */
            /*         __FILE__, __LINE__, __func__, (long long unsigned)st_wr.id); */
            eggPrtLog_warn("eggTopCollector", "%s:%d:%s docid[%llu] does not have weightResult from FieldWeight\n",
                    __FILE__, __LINE__, __func__, (long long unsigned)st_wr.id);

            ret = EGG_FALSE;
            goto failed;
        }

        hDocs[n_scoredoc_idx].sort_tmp_val = n_scoredoc_idx;
        n_scoredoc_idx++;        
    }

    if (isAsc)
    {
        if (field_type & EGG_INDEX_INT32)
        {
            Uti_sedgesort(hTopCollector->docs, hTopCollector->cntHits, sizeof(EGGSCOREDOC), eggScoreDoc_cmp_orderbytmpval_int32_desc);
//                Uti_insort(hTopCollector->docs, hTopCollector->cntHits, sizeof(EGGSCOREDOC), eggScoreDoc_cmp_orderby_int32_desc);
        }
        else if (field_type & EGG_INDEX_INT64)
        {
            Uti_sedgesort(hTopCollector->docs, hTopCollector->cntHits, sizeof(EGGSCOREDOC), eggScoreDoc_cmp_orderbytmpval_int64_desc);
//                Uti_insort(hTopCollector->docs, hTopCollector->cntHits, sizeof(EGGSCOREDOC), eggScoreDoc_cmp_orderby_int64_desc);
        }
        else if (field_type & EGG_INDEX_DOUBLE)
        {
            Uti_sedgesort(hTopCollector->docs, hTopCollector->cntHits, sizeof(EGGSCOREDOC), eggScoreDoc_cmp_orderbytmpval_double_desc);
//                Uti_insort(hTopCollector->docs, hTopCollector->cntHits, sizeof(EGGSCOREDOC), eggScoreDoc_cmp_orderby_double_desc);
        }
    }
    else
    {
        if (field_type & EGG_INDEX_INT32)
        {
            Uti_sedgesort(hTopCollector->docs, hTopCollector->cntHits, sizeof(EGGSCOREDOC), eggScoreDoc_cmp_orderbytmpval_int32_asc);
//                Uti_insort(hTopCollector->docs, hTopCollector->cntHits, sizeof(EGGSCOREDOC), eggScoreDoc_cmp_orderby_int32_asc);
        }
        else if (field_type & EGG_INDEX_INT64)
        {
            Uti_sedgesort(hTopCollector->docs, hTopCollector->cntHits, sizeof(EGGSCOREDOC), eggScoreDoc_cmp_orderbytmpval_int64_asc);
//                Uti_insort(hTopCollector->docs, hTopCollector->cntHits, sizeof(EGGSCOREDOC), eggScoreDoc_cmp_orderby_int64_asc);
        }
        else if (field_type & EGG_INDEX_DOUBLE)
        {
            Uti_sedgesort(hTopCollector->docs, hTopCollector->cntHits, sizeof(EGGSCOREDOC), eggScoreDoc_cmp_orderbytmpval_double_asc);
//                Uti_insort(hTopCollector->docs, hTopCollector->cntHits, sizeof(EGGSCOREDOC), eggScoreDoc_cmp_orderby_double_asc);
        }
    }

failed:
    free(hWeightResult);
        
    return ret;
}


PRIVATE HEGGWRESULT eggWResult_get_fromRangeIndex(HEGGINDEXREADER hIndexReader_,  count_t *p_nWeightResult, fdid_t fid)
{
    HEGGWRESULT hWeightResult = NULL;
    *p_nWeightResult = 0;
    if (fid > 0)
    {
        HEGGINDEXRANGE lp_index_range = eggIndexReader_query_idsrange_local(hIndexReader_,  fid);
        if(lp_index_range && lp_index_range->cnt)
        {
        index_t n_range_idx = 0;
        hWeightResult = (HEGGWRESULT)malloc(sizeof(EGGWRESULT) * lp_index_range->cnt);
        memcpy(hWeightResult, lp_index_range->dids, sizeof(EGGWRESULT) * lp_index_range->cnt );
        
        *p_nWeightResult = lp_index_range->cnt;
        Uti_sedgesort(hWeightResult, *p_nWeightResult, sizeof(EGGWRESULT), eggWResult_cmpid_desc);
        free(lp_index_range->dids);
        }
        free(lp_index_range);
    }
    return hWeightResult;
}


PRIVATE EBOOL eggTopCollector_order_throughRangeIndex(HEGGTOPCOLLECTOR hTopCollector,
                                                      HEGGINDEXREADER hIndexReader, fdid_t fid, type_t field_type, int isAsc)
{
    HEGGWRESULT hWeightResult = NULL;
    count_t nWeightResult = 0;
    hWeightResult = eggWResult_get_fromRangeIndex(hIndexReader, &nWeightResult, fid);
    if (!hWeightResult)
    {
        return EGG_FALSE;
    }

    EBOOL ret = EGG_TRUE;

    index_t n_scoredoc_idx = 0;
    HEGGSCOREDOC hDocs = hTopCollector->docs;
    while (n_scoredoc_idx != hTopCollector->cntHits)
    {
    
        index_t lookup_idx = 0;
        EGGWRESULT st_wr;
        st_wr.id = EGGDID_DOCID(&hDocs[n_scoredoc_idx].idDoc);        
        if (EGG_TRUE == Uti_binary_lookup(hWeightResult,
                                          nWeightResult-1, &st_wr,
                                          &lookup_idx, eggWResult_cmpid2_desc))
        {
            memcpy(hDocs[n_scoredoc_idx].orderBy,
                   hWeightResult[lookup_idx].val,
                   sizeof(hDocs[n_scoredoc_idx].orderBy));
        }
        else
        {
            memset(hDocs[n_scoredoc_idx].orderBy, 0,
                   sizeof(hDocs[n_scoredoc_idx].orderBy));
            /* fprintf(stderr, "%s:%d:%s docid[%llu] does not have weightResult from RangeIndex\n", */
            /*         __FILE__, __LINE__, __func__, (long long unsigned)st_wr.id); */
            eggPrtLog_warn("eggTopCollector", "%s:%d:%s docid[%llu] does not have weightResult from RangeIndex\n",
                    __FILE__, __LINE__, __func__, (long long unsigned)st_wr.id);
            
            ret = EGG_FALSE;
            goto failed;
        }
        
        hDocs[n_scoredoc_idx].sort_tmp_val = (int)n_scoredoc_idx;
            
        n_scoredoc_idx++;        
    }

    if (isAsc)
    {
        if (field_type & EGG_INDEX_INT32)
        {
            Uti_sedgesort(hTopCollector->docs, hTopCollector->cntHits, sizeof(EGGSCOREDOC), eggScoreDoc_cmp_orderbytmpval_int32_desc);
//                Uti_insort(hTopCollector->docs, hTopCollector->cntHits, sizeof(EGGSCOREDOC), eggScoreDoc_cmp_orderby_int32_desc);
        }
        else if (field_type & EGG_INDEX_INT64)
        {
            Uti_sedgesort(hTopCollector->docs, hTopCollector->cntHits, sizeof(EGGSCOREDOC), eggScoreDoc_cmp_orderbytmpval_int64_desc);
//                Uti_insort(hTopCollector->docs, hTopCollector->cntHits, sizeof(EGGSCOREDOC), eggScoreDoc_cmp_orderby_int64_desc);
        }
        else if (field_type & EGG_INDEX_DOUBLE)
        {
            Uti_sedgesort(hTopCollector->docs, hTopCollector->cntHits, sizeof(EGGSCOREDOC), eggScoreDoc_cmp_orderbytmpval_double_desc);
//                Uti_insort(hTopCollector->docs, hTopCollector->cntHits, sizeof(EGGSCOREDOC), eggScoreDoc_cmp_orderby_double_desc);
        }
        
    }
    else
    {
        if (field_type & EGG_INDEX_INT32)
        {
            Uti_sedgesort(hTopCollector->docs, hTopCollector->cntHits, sizeof(EGGSCOREDOC), eggScoreDoc_cmp_orderbytmpval_int32_asc);
//                Uti_insort(hTopCollector->docs, hTopCollector->cntHits, sizeof(EGGSCOREDOC), eggScoreDoc_cmp_orderby_int32_asc);
        }
        else if (field_type & EGG_INDEX_INT64)
        {
            Uti_sedgesort(hTopCollector->docs, hTopCollector->cntHits, sizeof(EGGSCOREDOC), eggScoreDoc_cmp_orderbytmpval_int64_asc);
//                Uti_insort(hTopCollector->docs, hTopCollector->cntHits, sizeof(EGGSCOREDOC), eggScoreDoc_cmp_orderby_int64_asc);
        }
        else if (field_type & EGG_INDEX_DOUBLE)
        {
            Uti_sedgesort(hTopCollector->docs, hTopCollector->cntHits, sizeof(EGGSCOREDOC), eggScoreDoc_cmp_orderbytmpval_double_asc);
//                Uti_insort(hTopCollector->docs, hTopCollector->cntHits, sizeof(EGGSCOREDOC), eggScoreDoc_cmp_orderby_double_asc);
        }
    }

failed:
    free(hWeightResult);
        
    return ret;
}
PUBLIC EBOOL eggTopCollector_orderby(HEGGTOPCOLLECTOR hTopCollector,
                                     HEGGINDEXREADER hIndexReader)
{
    if (!hTopCollector->hOrderBy || hTopCollector->cntOrderBy == 0)
    {
        /* fprintf(stderr, "%s:%d:%s hTopCollector->hOrderBy == %p or hTopCollector->cntOrderBy %u\n", */
        /*         __FILE__, __LINE__, __func__, */
        /*         hTopCollector->hOrderBy, */
        /*         (unsigned)hTopCollector->cntOrderBy); */
        eggPrtLog_error("eggTopCollector", "%s:%d:%s hTopCollector->hOrderBy == %p or hTopCollector->cntOrderBy %u\n",
                __FILE__, __LINE__, __func__,
                hTopCollector->hOrderBy,
                (unsigned)hTopCollector->cntOrderBy);

        return EGG_FALSE;
    }
    HEGGIDNODE lp_id_nodes = hTopCollector->hIdNodes;
    UTIVECTOR* lp_vector = Uti_vector_create(sizeof(EGGSCOREDOC));
    EGGSCOREDOC st_score_doc = {};
    index_t n_ids_idx = 0;

    HEGGORDERBY hOrderBy;
    HEGGWRESULT hWeightResult = NULL;
    count_t n_weightResult = 0;

    /* 生成hTopCollector->docs */
    while (n_ids_idx != hTopCollector->cntId)
    {
        count_t n_equalId_cnt = 0;
        /* 对id去重 */
        {
            while((lp_id_nodes)[n_ids_idx].id == (lp_id_nodes)[(n_ids_idx)+(n_equalId_cnt)].id)
            {
                (n_equalId_cnt)++;
                if(n_equalId_cnt + n_ids_idx == hTopCollector->cntId) break;
            }
        }
        
        EGGDID_DOCID(&st_score_doc.idDoc) = lp_id_nodes[n_ids_idx].id;
        st_score_doc.sort_tmp_val = n_ids_idx;        
        
        Uti_vector_push(lp_vector, &st_score_doc, 1);

        n_ids_idx += n_equalId_cnt;
    }
    hTopCollector->cntHits = hTopCollector->cntHits && hTopCollector->cntHits < Uti_vector_count(lp_vector) ?
        hTopCollector->cntHits : Uti_vector_count(lp_vector);
    hTopCollector->cntTotal = hTopCollector->cntHits;
    hTopCollector->docs = Uti_vector_data(lp_vector);
    Uti_vector_destroy(lp_vector, EGG_FALSE);
    if(hTopCollector->cntId)
    {
        free(hTopCollector->hIdNodes);
        hTopCollector->hIdNodes = NULL;
        hTopCollector->cntId = 0;
    }    
    hTopCollector->sortType = EGG_TOPSORT_ORDERBY;

    
    HEGGFIELDWEIGHT hFieldWeight = eggIndexReader_get_fieldweight_local(hIndexReader);
    HEGGFIELDWEIGHT hFieldView = eggIndexReader_get_fieldview_local(hIndexReader);
    
    index_t n_orderBy_idx = hTopCollector->cntOrderBy - 1;
    for (; n_orderBy_idx >= 0; n_orderBy_idx--)
    {
        hOrderBy = hTopCollector->hOrderBy + n_orderBy_idx;

        fdid_t fid;
        eggFieldView_find(hFieldView, hOrderBy->fieldName, &fid);
        type_t field_type = eggFieldView_get_type(hFieldView, hOrderBy->fieldName);


        if (eggTopCollector_order_throughIdRangeOrder(hTopCollector, fid, field_type, hOrderBy->isAsc) == EGG_TRUE)
        {
            ;
        }
        else if (field_type & EGG_RANGE_INDEX)
        {
            if (eggTopCollector_order_throughRangeIndex(hTopCollector, hIndexReader, fid, field_type, hOrderBy->isAsc) == EGG_FALSE)
            {
                return EGG_FALSE;
            }
        }
        else
        {
            if (eggTopCollector_order_throughFieldWeight(hTopCollector, hFieldWeight, fid, field_type, hOrderBy->isAsc) == EGG_FALSE)
            {
                return EGG_FALSE;
            }
        }
        
    }

    return EGG_TRUE;
}
/*
PRIVATE EBOOL eggTopCollector_normalized_orderby(HEGGTOPCOLLECTOR hTopCollector)
{
    did_t* lp_id_range = hTopCollector->idRange;
            
    if(!lp_id_range)
    {
        hTopCollector->cntHits = 0;

        return EGG_FALSE;
    }

    HEGGIDNODE lp_id_nodes = hTopCollector->hIdNodes;
    
    UTIVECTOR* lp_vector = Uti_vector_create(sizeof(EGGSCOREDOC));
    EGGSCOREDOC st_score_doc;
    index_t n_ids_idx = 0;

    while(n_ids_idx != hTopCollector->cntRange)
    {
        if (lp_id_nodes)
        {
            EGGIDNODE st_id_node;
            st_id_node.id = hTopCollector->idRange[n_ids_idx];
            index_t n_dest_idx = 0;
            
            if(Uti_binary_lookup(hTopCollector->hIdNodes, hTopCollector->cntId - 1, &st_id_node, &n_dest_idx, eggIdNode_cmp_id2))
            {
                EGGDID_DOCID(&st_score_doc.idDoc) = hTopCollector->idRange[n_ids_idx];
        
                Uti_vector_push(lp_vector, &st_score_doc, 1);
                
            }
        }
        else
        {
            EGGDID_DOCID(&st_score_doc.idDoc) = hTopCollector->idRange[n_ids_idx];
            
            Uti_vector_push(lp_vector, &st_score_doc, 1);
        }
        
        n_ids_idx++;
    }

    hTopCollector->cntHits = hTopCollector->cntHits && hTopCollector->cntHits < Uti_vector_count(lp_vector) ?
        hTopCollector->cntHits : Uti_vector_count(lp_vector);
    
    hTopCollector->cntTotal = hTopCollector->cntHits;
    
    hTopCollector->docs = Uti_vector_data(lp_vector);
    
    Uti_vector_destroy(lp_vector, EGG_FALSE);
    hTopCollector->sortType = EGG_TOPSORT_ORDERBY;

    return EGG_TRUE;
}
*/

EBOOL eggTopCollector_scoreLinear(HEGGTOPCOLLECTOR hTopCollector)
{
    
  HEGGSCOREDOC hScoreDoc = hTopCollector->docs;
  count_t cnt = hTopCollector->cntHits;
  int i = 0;
  double max = -1;
  double min = 99999999;
  while(i < cnt)
    {
      if(hScoreDoc[i].score < min)
        {
          min =  hScoreDoc[i].score;
        }
      if(hScoreDoc[i].score > max)
        {
          max = hScoreDoc[i].score;
        }
      i++;
    }
//  printf("max %f min %f\n", max, min);
  eggPrtLog_info("eggTopCollector", "max %f min %f\n", max, min);

  double race = 1.0;
  if(max < 100) race = 0.3;

  if(max / min < 10)
    {
      i = 0;
      while(i < cnt)
        {
          hScoreDoc[i].score = (log(hScoreDoc[i].score + 1)  ) * 10/(log(max + 1
));                                                                                       hScoreDoc[i].score *= 1000;
          hScoreDoc[i].score *= race;
          i++;
        }
    }
  else
    {
      i = 0;
      while(i < cnt)
        {
          hScoreDoc[i].score = (log(hScoreDoc[i].score + 2) -  log(min + 1) ) * 
10/(log(max + 1) -  log(min + 1));                                              
          if(hScoreDoc[i].score > 10) hScoreDoc[i].score = 10;
          hScoreDoc[i].score *= 1000;
          hScoreDoc[i].score *= race;
          i++;
        }
     
    }
  return EGG_TRUE;
}

