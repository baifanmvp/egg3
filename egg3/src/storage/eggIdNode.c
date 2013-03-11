
#include "eggIdNode.h"
#include "../similarity/eggSimilarScore.h"
#include "../uti/Utility.h"
#include <math.h>
#include <float.h>
HEGGIDNODE eggIdNode_merge_or(HEGGIDNODE hSrcIdNodes, HEGGIDNODE hDestIdNodes, count_t srcCnt, count_t destCnt, count_t* pResCnt)
{
    *pResCnt = srcCnt + destCnt;
    if(*pResCnt == 0)
    {
        return EGG_NULL;
    }
    
    HEGGIDNODE lp_res_idNodes = (HEGGIDNODE)malloc(*pResCnt * sizeof(EGGIDNODE));
    
    memcpy(lp_res_idNodes, hSrcIdNodes, sizeof(EGGIDNODE) * srcCnt);
    
    memcpy(lp_res_idNodes + srcCnt, hDestIdNodes, sizeof(EGGIDNODE) * destCnt);
    
    Uti_sedgesort (lp_res_idNodes, (*pResCnt), sizeof(EGGIDNODE), eggIdNode_cmp_id);
    
    return lp_res_idNodes;
}

HEGGIDNODE eggIdNode_merge_and(HEGGIDNODE hSrcIdNodes, HEGGIDNODE hDestIdNodes, count_t srcCnt, count_t destCnt, count_t* pResCnt)
{
    if(srcCnt == 0 || destCnt == 0)
    {
        return EGG_NULL;
    }
    
    UTIVECTOR* lp_vector = Uti_vector_create(sizeof(EGGIDNODE));
    index_t n_src_idx = 0;
    index_t n_dest_idx = 0;
    
    while(n_src_idx != srcCnt && n_dest_idx != destCnt)
    {
        if (hSrcIdNodes[n_src_idx].id > hDestIdNodes[n_dest_idx].id)
        {
            n_src_idx ++;
        }
        else if (hSrcIdNodes[n_src_idx].id < hDestIdNodes[n_dest_idx].id)
        {
            n_dest_idx ++;
        }
        else
        {
            
            did_t id = hSrcIdNodes[n_src_idx].id;

            do
	      {
                Uti_vector_push(lp_vector, hSrcIdNodes + n_src_idx, 1);
                n_src_idx ++;
	      }while(n_src_idx != srcCnt && hSrcIdNodes[n_src_idx].id == hDestIdNodes[n_dest_idx].id);
            
            do
	      {
                Uti_vector_push(lp_vector, hDestIdNodes + n_dest_idx, 1);
                n_dest_idx++;
	      }while(n_dest_idx != destCnt && hDestIdNodes[n_dest_idx].id == id);
        }
    }
    HEGGIDNODE lp_res_idNodes = Uti_vector_data(lp_vector);
    *pResCnt = Uti_vector_count(lp_vector);
    
    Uti_vector_destroy(lp_vector, EGG_FALSE);

    return lp_res_idNodes;
}

HEGGIDNODE eggIdNode_merge_minus(HEGGIDNODE hSrcIdNodes, HEGGIDNODE hDestIdNodes, count_t srcCnt, count_t destCnt, count_t* pResCnt)
{
    if(srcCnt == 0 || destCnt == 0)
    {
        return EGG_NULL;
    }
    
    UTIVECTOR* lp_vector = Uti_vector_create(sizeof(EGGIDNODE));
    index_t n_src_idx = 0;
    index_t n_dest_idx = 0;
    
    while(n_src_idx != srcCnt && n_dest_idx != destCnt)
    {
        if (hSrcIdNodes[n_src_idx].id > hDestIdNodes[n_dest_idx].id)
        {
            Uti_vector_push(lp_vector, hSrcIdNodes + n_src_idx, 1);            
            n_src_idx ++;
        }
        else if (hSrcIdNodes[n_src_idx].id < hDestIdNodes[n_dest_idx].id)
        {
            n_dest_idx ++;
        }
        else
        {
            n_src_idx ++;
            n_dest_idx++;
        }
    }
    Uti_vector_push(lp_vector, hSrcIdNodes + n_src_idx, srcCnt-n_src_idx);
    HEGGIDNODE lp_res_idNodes = Uti_vector_data(lp_vector);
    *pResCnt = Uti_vector_count(lp_vector);
    
    Uti_vector_destroy(lp_vector, EGG_FALSE);

    return lp_res_idNodes;
}

EBOOL eggIdNode_filter_by_score(HEGGIDNODE hIdNodes, count_t* pIdCnt)
{
    if(!hIdNodes || *pIdCnt == 0)
    {
        return EGG_FALSE;
    }
    
    index_t n_ids_idx = 0;
    count_t n_org_cnt = *pIdCnt;
    index_t n_fact_idx = 0;
    
    count_t n_equalId_cnt = 0;
     while((hIdNodes)[0].id == (hIdNodes)[(0)+(n_equalId_cnt)].id)
    {
        (n_equalId_cnt)++;
        if(n_equalId_cnt == n_org_cnt) break;
    }
    if(n_equalId_cnt < 2)
    {
        return EGG_TRUE;
    }
    score_t n_cmp_score = pow(100, n_equalId_cnt - 1) > 0 ? pow(100, n_equalId_cnt-1) : 1e9;
    while(n_ids_idx != n_org_cnt)
    {

        score_t n_score = similar_score_with_field(hIdNodes + n_ids_idx, n_equalId_cnt);
        printf("n_score :%f\n", n_score);
        if(n_score > n_cmp_score)
        {
            if(n_fact_idx != n_ids_idx)
            {
                memcpy(hIdNodes + n_fact_idx, hIdNodes + n_ids_idx, n_equalId_cnt * sizeof(EGGIDNODE));
            }
            n_fact_idx += n_equalId_cnt;
        }
        
        n_ids_idx += n_equalId_cnt;
    }
        
    *pIdCnt = n_fact_idx;
    
    return EGG_TRUE;
}



EBOOL eggIdNode_filter_by_continuation(HEGGIDNODE hIdNodes, count_t* pIdCnt)
{
    if(!hIdNodes || *pIdCnt == 0)
    {
        return EGG_FALSE;
    }
    
    index_t n_ids_idx = 0;
    count_t n_org_cnt = *pIdCnt;
    index_t n_fact_idx = 0;
    
    count_t n_equalId_cnt = 0;
     while((hIdNodes)[0].id == (hIdNodes)[(0)+(n_equalId_cnt)].id)
    {
        (n_equalId_cnt)++;
        if(n_equalId_cnt == n_org_cnt) break;
    }
    if(n_equalId_cnt < 2)
    {
        return EGG_TRUE;
    }
  
    while(n_ids_idx != n_org_cnt)
    {

        EBOOL ret = similar_word_with_continuation(hIdNodes + n_ids_idx, n_equalId_cnt);
        if(ret == EGG_TRUE)
        {
            if(n_fact_idx != n_ids_idx)
            {
                memcpy(hIdNodes + n_fact_idx, hIdNodes + n_ids_idx, n_equalId_cnt * sizeof(EGGIDNODE));
            }
            n_fact_idx += n_equalId_cnt;
        }
        
        n_ids_idx += n_equalId_cnt;
    }
        
    *pIdCnt = n_fact_idx;
    
    return EGG_TRUE;
}


//desc
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

//asc
int eggIdNode_cmp_id3(HEGGIDNODE hSrcIdNode, HEGGIDNODE hDestIdNode)
{
    if(hSrcIdNode->id < hDestIdNode->id)
    {
        return 1;
    }
    else if(hSrcIdNode->id > hDestIdNode->id)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}

int eggIdNode_cmp_id2(HEGGIDNODE hSrcIdNodes, index_t idx, HEGGIDNODE hDestIdNodes)
{
    if((hSrcIdNodes + idx)->id > hDestIdNodes->id)
    {
        return 1;
    }
    else if( (hSrcIdNodes + idx)->id < hDestIdNodes->id)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}

int eggIdNode_cmp_mask(HEGGIDNODE hSrcIdNode, HEGGIDNODE hDestIdNode)
{
    if(hSrcIdNode->mask > hDestIdNode->mask)
    {
        return 1;
    }
    else if(hSrcIdNode->mask < hDestIdNode->mask)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}


int eggIdNode_cmp_weight(HEGGIDNODE hSrcIdNode, HEGGIDNODE hDestIdNode)
{
    if(hSrcIdNode->weight < hDestIdNode->weight)
    {
        return 1;
    }
    else if(hSrcIdNode->weight > hDestIdNode->weight)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}

int eggIdNode_cmp_weight2(HEGGIDNODE hSrcIdNode, index_t idx, HEGGIDNODE hDestIdNode)
{
    if((hSrcIdNode + idx)->weight < hDestIdNode->weight)
    {
        return 1;
    }
    else if((hSrcIdNode + idx)->weight > hDestIdNode->weight)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}

EBOOL eggIdNode_set_timestamp(HEGGIDNODE hIdNodes, count_t idCnt, u32 baseValue)
{
    index_t n_id_idx = 0;
    while(n_id_idx != idCnt)
    {
        hIdNodes[n_id_idx].timeStamp = baseValue + n_id_idx + 1;
        n_id_idx++;
    }
    
    return EGG_TRUE;
}

EBOOL eggIdNode_set_boost(HEGGIDNODE hIdNodes, count_t idCnt, eggBoost_t boost)
{
    index_t n_id_idx = 0;
    while(n_id_idx != idCnt)
    {
        hIdNodes[n_id_idx].boost = boost;
        n_id_idx++;
    }
    
    return EGG_TRUE;
}

/*
    合并 id,取时间戳最大的
    删除flag=EGG_IDNODE_INVALID的id, weight付float的最小值
        
 */
EBOOL eggIdNode_filter_repeat(HEGGIDNODE hIdNodes, count_t* lpIdCnt)
{
    if(!(*lpIdCnt))
    {
        return EGG_FALSE;
    }
    

    count_t n_src_cnt = *lpIdCnt;
    count_t n_dest_cnt = *lpIdCnt;

    index_t i_src = 1;
    index_t i_dest = 1;
    
    while(i_src < n_src_cnt)
    {
        if(eggIdNode_cmp_id(hIdNodes + i_src, hIdNodes + (i_src - 1)) )
        {
            if(hIdNodes[i_dest - 1].flag == EGG_IDNODE_INVALID)
            {
                i_dest--;
            }

            if(i_src != i_dest)
            {

                hIdNodes[i_dest] = hIdNodes[i_src];
            }
            i_dest++;
        }
        else
        {
            if(hIdNodes[i_dest - 1].timeStamp < hIdNodes[i_src].timeStamp)
            {
                hIdNodes[i_dest - 1] = hIdNodes[i_src];
            }
        }
        i_src++;
    }
    
    if(hIdNodes[i_dest - 1].flag == EGG_IDNODE_INVALID)
    {
        i_dest--;
    }
    
    *lpIdCnt = i_dest;
    return EGG_TRUE;
}

EBOOL eggIdNode_reset_timestamp(HEGGIDNODE hIdNodes, count_t idCnt)
{
    while(idCnt--)
    {
        hIdNodes[idCnt].timeStamp = EGG_IDTIMESTAMP_BASEVAL;
    }
    return EGG_TRUE;
}
