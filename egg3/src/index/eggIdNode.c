#include "eggIdNode.h"

HEGGIDNODE eggIdNode_merge_or(HEGGIDNODE hSrcIdNodes, HEGGIDNODE hDestIdNodes, count_t srcCnt, count_t destCnt, count_t* pResCnt)
{
    *pResCnt = srcCnt + destCnt;
    
    HEGGIDNODE lp_res_idNodes = (HEGGIDNODE)malloc(*pResCnt * sizeof(EGGIDNODE));
    
    memcpy(lp_res_idNodes, hSrcIdNodes, sizeof(EGGIDNODE) * srcCnt);
    memcpy(lp_res_idNodes + srcCnt, hDestIdNodes, sizeof(EGGIDNODE) * destCnt);
    
    uti_sedgesort (lp_res_idNodes, sizeof(EGGIDNODE)*(*pResCnt), sizeof(EGGIDNODE), eggIdNode_cmp_id);
    
    return lp_res_idNodes;
}

HEGGIDNODE eggIdNode_merge_and(HEGGIDNODE hSrcIdNodes, HEGGIDNODE hDestIdNodes, count_t srcCnt, count_t destCnt, count_t* pResCnt)
{
    UTIVECTOR* lp_vector = Uti_vector_create(sizeof(EGGIDNODE));
    index_t n_src_idx = 0;
    index_t n_dest_idx = 0;
    
    while(n_src_idx != srcCnt && n_dest_idx != destCnt)
    {
        if (hSrcIdNodes[n_src_idx]->id > hDestIdNodes[n_dest_idx]->id)
        {
            n_src_idx ++;
        }
        else if (hSrcIdNodes[n_src_idx]->id < hDestIdNodes[n_dest_idx]->id)
        {
            n_dest_idx ++;
        }
        else
        {
            do
            {
                Uti_vector_push(lp_vector, hSrcIdNodes + n_src_idx, 1);
                n_src_idx ++;
            }while(hSrcIdNodes[n_src_idx]->id == hDestIdNodes[n_dest_idx]->id);
            
            Uti_vector_push(lp_vector, hDestIdANodes + n_dest_idx, 1);
            n_dest_idx
            
        }
    }
    HEGGIDNODE lp_res_idNodes = Uti_vector_data(lp_vector);
    *pResCnt = Uti_vector_count(lp_vector);
    
    Uti_vector_destroy(lp_vector, EGG_FALSE);

    return lp_res_idNodes;
}

int eggIdNode_cmp_id(HEGGIDNODE hSrcIdNode, HEGGIDNODE hDestIdNode)
{
    if(hSrcIdNode->id > hDestIdNode->id)
    {
        return 1;
    }
    else if(hSrcIdNode->id < hDestIdNode->id)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}

