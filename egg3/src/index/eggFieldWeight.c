#include "eggFieldWeight.h"
#include "../log/eggPrtLog.h"
#include <stdint.h>

extern pthread_mutex_t counter_mutex;

EBOOL eggFieldWeight_block_init(HEGGFIELDWEIGHT hFieldWeight, HEGGFIELD hField, did_t id);

EBOOL eggFieldWeight_rewrite_block(HEGGFIELDWEIGHT hFieldWeight, HEGGFIELD hField, did_t id, fweight_t* pBaseOff, HEGGWEIGHTBLOCK hWeightBlock);
HEGGFIELDWEIGHT eggFieldWeight_new(HEGGFILE hEggFile, HEGGFIELDVIEW hFieldView)
{
    if (POINTER_IS_INVALID(hEggFile) || POINTER_IS_INVALID(hFieldView))
    {
        return EGG_NULL;
    }
    
    HEGGFIELDWEIGHT lp_field_weight = (HEGGFIELDWEIGHT)malloc(sizeof(EGGFIELDWEIGHT));
    
    lp_field_weight->hViewStream = ViewStream_new(hEggFile);
    lp_field_weight->hFieldView = hFieldView;
    
    lp_field_weight->hCache = (HEGGWEIGHTCACHE)malloc(sizeof(EGGWEIGHTCACHE));
    memset(lp_field_weight->hCache, 0, sizeof(EGGWEIGHTCACHE));
    pthread_mutex_init( &lp_field_weight->mutex, NULL);
    
    return lp_field_weight;
}

EBOOL eggFieldWeight_delete(HEGGFIELDWEIGHT hFieldWeight)
{
    if (POINTER_IS_INVALID(hFieldWeight))
    {
        return EGG_FALSE;
    }
    while(hFieldWeight->hCache->eCnt)
    {
        hFieldWeight->hCache->eCnt--;
            
        free(hFieldWeight->hCache->cacheBlock[hFieldWeight->hCache->eCnt]);
    }
    ViewStream_delete(hFieldWeight->hViewStream);
    pthread_mutex_destroy(&hFieldWeight->mutex);

    free(hFieldWeight->hCache);
    free(hFieldWeight);
    
    return EGG_TRUE;
}



EBOOL eggFieldWeight_add(HEGGFIELDWEIGHT hFieldWeight, HEGGFIELD hField, did_t id)
{
    if (POINTER_IS_INVALID(hFieldWeight))
    {
        return EGG_FALSE;
    }
    if (id == 0)
    {
        return EGG_TRUE;
    }

    EGGWEIGHTBLOCK st_weight_block = {0};
    
    char* lp_field_name = eggField_get_name(hField);
    type_t type = eggField_get_type(hField);
    size32_t n_val_len = 0;
    char* lp_field_val = eggField_get_value(hField, &n_val_len);
    if(n_val_len > 8)
    {
        //printf("eggField value len over 8 byte!\n");
        eggPrtLog_error("eggFieldWeight", "eggField value len over 8 byte!\n");
        return EGG_FALSE;
    }
    //printf("add id : %llu [%d] \n", id, *(int*)lp_field_val);

    pthread_mutex_lock(&hFieldWeight->mutex);
    
    fweight_t n_base_off = eggFieldView_get_fieldweight(hFieldWeight->hFieldView, lp_field_name);
    
    if(!n_base_off)
    {
        //printf("----------eggFieldWeight_block_init---------- %llu , %s\n", n_base_off, lp_field_name);
        EBOOL ret = eggFieldWeight_block_init(hFieldWeight, hField, id);
        pthread_mutex_unlock(&hFieldWeight->mutex);
        return ret;
    }
    
    ViewStream_read_nolock(hFieldWeight->hViewStream, &st_weight_block, sizeof(st_weight_block), n_base_off);
    
    
    if(WEIGHTNODE_FULL(&st_weight_block, id))
    {
        // printf("eggFieldWeight_rewrite_block [%llu] [%llu]  \n", st_weight_block.aCnt, id);
        eggFieldWeight_rewrite_block(hFieldWeight, hField, id, &n_base_off, &st_weight_block);
    }
    
    offset64_t n_node_off = WEIGHTNODE_OFFSET(n_base_off, id - 1);

    
    EGGWNODE st_wnode = {0};
    ViewStream_read_nolock(hFieldWeight->hViewStream, &st_wnode, sizeof(st_wnode), n_node_off);
    if(st_wnode.flag == WEIGHT_NODE_INVALID)
    {
        st_weight_block.eCnt++;    
    }
    
    st_wnode.flag = WEIGHT_NODE_VALID;
    memcpy(st_wnode.val, lp_field_val, n_val_len);
    ViewStream_update_nolock(hFieldWeight->hViewStream, &st_wnode, sizeof(EGGWNODE), n_node_off);
    
    st_weight_block.maxId = st_weight_block.maxId < id ? id : st_weight_block.maxId;
    ViewStream_update_nolock(hFieldWeight->hViewStream, &st_weight_block, sizeof(EGGWEIGHTBLOCK), n_base_off);
    
    pthread_mutex_unlock(&hFieldWeight->mutex);
    
    return EGG_TRUE;
}
EBOOL eggFieldWeight_remove(HEGGFIELDWEIGHT hFieldWeight, HEGGFIELD hField, did_t id)
{
    if (POINTER_IS_INVALID(hFieldWeight))
    {
        return EGG_FALSE;
    }
    if (id == 0)
    {
        return EGG_TRUE;
    }
    
    EGGWEIGHTBLOCK st_weight_block = {0};
    
    char* lp_field_name = eggField_get_name(hField);
    pthread_mutex_lock(&hFieldWeight->mutex);
    

    fweight_t n_base_off = eggFieldView_get_fieldweight(hFieldWeight->hFieldView, lp_field_name);
    
    if(!n_base_off)
    {
        //printf("n_base_off == NULL\n");
        eggPrtLog_error("eggFieldWeight", "n_base_off == NULL\n");
        pthread_mutex_unlock(&hFieldWeight->mutex);

        return EGG_FALSE;
    }
    
    ViewStream_read_nolock(hFieldWeight->hViewStream, &st_weight_block, sizeof(st_weight_block), n_base_off);
    
    if(WEIGHTNODE_FULL(&st_weight_block, id))
    {
        //printf("WEIGHTNODE_FULL(&st_weight_block, id)\n");
        eggPrtLog_error("eggFieldWeight", "WEIGHTNODE_FULL(&st_weight_block, id)\n");
        pthread_mutex_unlock(&hFieldWeight->mutex);
        return EGG_FALSE;
    }

    offset64_t n_node_off = WEIGHTNODE_OFFSET(n_base_off, id - 1);
    EGGWNODE st_wnode = {0};
    
    ViewStream_read_nolock(hFieldWeight->hViewStream, &st_wnode, sizeof(st_wnode), n_node_off);
    if(st_wnode.flag == WEIGHT_NODE_VALID)
    {
        st_weight_block.eCnt--;
        st_wnode.flag = WEIGHT_NODE_INVALID;
        ViewStream_update_nolock(hFieldWeight->hViewStream, &st_wnode, sizeof(EGGWNODE), n_node_off);
        
        ViewStream_update_nolock(hFieldWeight->hViewStream, &st_weight_block, sizeof(EGGWEIGHTBLOCK), n_base_off);
        pthread_mutex_unlock(&hFieldWeight->mutex);

        return EGG_TRUE;
    }
    else
    {
        pthread_mutex_unlock(&hFieldWeight->mutex);

        return EGG_FALSE;
    }
    
    
    
}

type_t eggFieldWeight_type_withname(HEGGFIELDWEIGHT hFieldWeight, char* fieldName)
{
    if (POINTER_IS_INVALID(hFieldWeight))
    {
        return EGG_FALSE;
    }

    return eggFieldView_get_type(hFieldWeight->hFieldView, fieldName);

}
HEGGWRESULT eggFieldWeight_get_withfid(HEGGFIELDWEIGHT hFieldWeight, fdid_t fid, count_t* lpCount)
{
    HEGGFIELDNAMEINFO lp_field_info = eggFieldView_get_singlefieldnameinfo_byfid(hFieldWeight->hFieldView, fid);
    HEGGWRESULT lp_wresult = eggFieldWeight_get_withname(hFieldWeight, lp_field_info->name, lpCount);
    
    eggFieldView_delete_fieldnameinfo(lp_field_info, 1);
    
    return lp_wresult;
}


HEGGWRESULT eggFieldWeight_get_withname(HEGGFIELDWEIGHT hFieldWeight, char* fieldName, count_t* lpCount)
{
    if (POINTER_IS_INVALID(hFieldWeight))
    {
        return EGG_NULL;
    }

    pthread_mutex_lock(&hFieldWeight->mutex);
    
    fdid_t fid = 0;
    eggFieldView_find(hFieldWeight->hFieldView, fieldName, &fid);
    index_t n_idx = 0;
    
    //find result in cache
    while(n_idx < hFieldWeight->hCache->eCnt)
    {
        if(hFieldWeight->hCache->cacheBlock[n_idx]->fid == fid)
        {
            *lpCount = hFieldWeight->hCache->cacheBlock[n_idx]->eCnt;
            HEGGWRESULT lp_res_back = (HEGGWRESULT)malloc(sizeof(EGGWRESULT) * (*lpCount) );
            memcpy(lp_res_back, hFieldWeight->hCache->cacheBlock[n_idx] + 1,  sizeof(EGGWRESULT) * (*lpCount));
            pthread_mutex_unlock(&hFieldWeight->mutex);
            return (HEGGWRESULT)(lp_res_back);
        }
        n_idx++;
    }


    fweight_t n_base_off = eggFieldView_get_fieldweight(hFieldWeight->hFieldView, fieldName);

    EGGWEIGHTBLOCK st_weight_block = {0};
    
    ViewStream_read_nolock(hFieldWeight->hViewStream, &st_weight_block, sizeof(st_weight_block), n_base_off);
    if(!st_weight_block.eCnt)
    {
        *lpCount = 0;
        pthread_mutex_unlock(&hFieldWeight->mutex);        
        return EGG_NULL;
    }

    //////////////////////////////////////
    int n_nodes_len = sizeof(EGGWEIGHTBLOCK) + sizeof(EGGWNODE) * (st_weight_block.maxId);
    
    HEGGWEIGHTBLOCK lp_nodes_buf = (HEGGWEIGHTBLOCK)malloc(n_nodes_len);
    HEGGWNODE lp_wnodes = (HEGGWNODE)(lp_nodes_buf + 1);
    memcpy(lp_nodes_buf, &st_weight_block, sizeof(EGGWEIGHTBLOCK) );

    ViewStream_read_nolock(hFieldWeight->hViewStream, (lp_nodes_buf) + 1, n_nodes_len - sizeof(st_weight_block), n_base_off + sizeof(st_weight_block));
    
    //////////////////////////////////////
    int n_res_len = sizeof(EGGWEIGHTBLOCK) + sizeof(EGGWRESULT) * (st_weight_block.eCnt);
    HEGGWEIGHTBLOCK lp_res_buf = (HEGGWEIGHTBLOCK)malloc(n_res_len);
    HEGGWRESULT lp_wres = (HEGGWRESULT)(lp_res_buf + 1);
    memcpy(lp_res_buf, &st_weight_block, sizeof(EGGWEIGHTBLOCK) );
    index_t n_res_idx = 0;
    index_t n_nodes_idx = 0;
    while(n_nodes_idx != st_weight_block.maxId)
    {
        if(lp_wnodes[n_nodes_idx].flag == WEIGHT_NODE_VALID)
        {
            lp_wres[n_res_idx].id = n_nodes_idx + 1;
            memcpy(lp_wres[n_res_idx].val, lp_wnodes[n_nodes_idx].val, 8);
            n_res_idx++;
            if(n_res_idx == st_weight_block.eCnt)break;
        }
        n_nodes_idx++;
    }
/*    if(st_weight_block.maxId != lp_wres[st_weight_block.eCnt - 1].id)
    {
        st_weight_block.maxId = lp_wres[st_weight_block.eCnt - 1].id;
        ViewStream_update_nolock(hFieldWeight->hViewStream, &st_weight_block, sizeof(EGGWEIGHTBLOCK), n_base_off);
        }*/
    //////////////////////////////////////
    free(lp_nodes_buf);
    hFieldWeight->hCache->cacheBlock[n_idx] = lp_res_buf;
    hFieldWeight->hCache->eCnt ++;
    
    *lpCount = st_weight_block.eCnt;
    
    HEGGWRESULT lp_res_back = (HEGGWRESULT)malloc(sizeof(EGGWRESULT) * (*lpCount) );
    memcpy(lp_res_back, lp_res_buf + 1,  sizeof(EGGWRESULT) * (*lpCount));
    
    pthread_mutex_unlock(&hFieldWeight->mutex);
    
    return (HEGGWRESULT)(lp_res_back);
    
}

EBOOL eggFieldWeight_clean_cache(HEGGFIELDWEIGHT hFieldWeight)
{
    if (POINTER_IS_INVALID(hFieldWeight))
    {
        return EGG_FALSE;
    }

    pthread_mutex_lock(&hFieldWeight->mutex);    
    while(hFieldWeight->hCache->eCnt)
    {
        hFieldWeight->hCache->eCnt--;
            
        free(hFieldWeight->hCache->cacheBlock[hFieldWeight->hCache->eCnt]);
    }
    pthread_mutex_unlock(&hFieldWeight->mutex);    
    return EGG_TRUE;
}

EBOOL eggFieldWeight_block_init(HEGGFIELDWEIGHT hFieldWeight, HEGGFIELD hField, did_t id)
{
    //printf("----------eggFieldWeight_block_init---------- \n");
    eggPrtLog_info("eggFieldWeight", "eggFieldWeight_block_init\n");
    char* lp_field_name = eggField_get_name(hField);
    type_t type = eggField_get_type(hField);
    size32_t n_val_len = 0;
    char* lp_field_val = eggField_get_value(hField, &n_val_len);
    count_t n_max_cnt = ((id / WEIGHT_NODECOUNT_LIMIT) + 1) * WEIGHT_NODECOUNT_LIMIT;
    
    int n_buf_len = sizeof(EGGWEIGHTBLOCK) + sizeof(EGGWNODE) * (n_max_cnt);
    HEGGWEIGHTBLOCK lp_block_buf = (HEGGWEIGHTBLOCK)malloc(n_buf_len);
    memset(lp_block_buf, 0, n_buf_len);
    
    lp_block_buf->eCnt = 1;
    lp_block_buf->maxId = id;
    lp_block_buf->aCnt = n_max_cnt;
    lp_block_buf->type = type;
    

    if(eggFieldView_find(hFieldWeight->hFieldView, lp_field_name, &lp_block_buf->fid) != EGG_TRUE)
    {
        free(lp_block_buf);

        return EGG_FALSE;
    }

    EGGWNODE st_wnode = {0};
    st_wnode.flag = WEIGHT_NODE_VALID;
    
    memcpy(st_wnode.val, lp_field_val, n_val_len);

    memcpy((char*)(lp_block_buf + 1) + ((int)id - 1) * sizeof(EGGWNODE), &st_wnode, sizeof(EGGWNODE));
    
    offset64_t n_base_off = ViewStream_write(hFieldWeight->hViewStream, lp_block_buf, n_buf_len);


    eggFieldView_set_fieldweight(hFieldWeight->hFieldView, lp_field_name, n_base_off);

    free(lp_block_buf);
    return EGG_TRUE;
    
}

EBOOL eggFieldWeight_rewrite_block(HEGGFIELDWEIGHT hFieldWeight, HEGGFIELD hField, did_t id, fweight_t* pBaseOff, HEGGWEIGHTBLOCK hWeightBlock)
{
    char* lp_field_name = eggField_get_name(hField);
    type_t type = eggField_get_type(hField);
    size32_t n_val_len = 0;
    char* lp_field_val = eggField_get_value(hField, &n_val_len);
    
    count_t n_max_cnt = ((id / WEIGHT_NODECOUNT_LIMIT) + 1) * WEIGHT_NODECOUNT_LIMIT;
    
    int n_buf_len = sizeof(EGGWEIGHTBLOCK) + sizeof(EGGWNODE) * (n_max_cnt);

    HEGGWEIGHTBLOCK lp_block_buf = (HEGGWEIGHTBLOCK)malloc(n_buf_len);
        
    memset(lp_block_buf, 0, n_buf_len);

    memcpy(lp_block_buf, hWeightBlock, sizeof(EGGWEIGHTBLOCK));
        
    ViewStream_read_nolock(hFieldWeight->hViewStream, (lp_block_buf) + 1,
                           sizeof(EGGWNODE) * hWeightBlock->aCnt, *pBaseOff + sizeof(EGGWEIGHTBLOCK));

    ViewStream_free_area(hFieldWeight->hViewStream, *pBaseOff,
                        sizeof(EGGWEIGHTBLOCK) + n_val_len * hWeightBlock->aCnt );
        
    lp_block_buf->aCnt =  n_max_cnt;
    hWeightBlock->aCnt =  n_max_cnt;
    *pBaseOff = ViewStream_write(hFieldWeight->hViewStream, lp_block_buf, n_buf_len);


    eggFieldView_set_fieldweight(hFieldWeight->hFieldView, lp_field_name, *pBaseOff);

    free(lp_block_buf);

    return EGG_TRUE;
    
}

int eggWResult_cmpval_int32_asc(HEGGWRESULT src, HEGGWRESULT des)
{
    if (*(int32_t *)src->val > *(int32_t*)des->val)
        return 1;
    else if (*(int32_t *)src->val < *(int32_t*)des->val)
        return -1;
    else
        return 0;
}

int eggWResult_cmpval_int32_desc(HEGGWRESULT src, HEGGWRESULT des)
{
    if (*(int32_t *)src->val < *(int32_t*)des->val)
        return 1;
    else if (*(int32_t *)src->val > *(int32_t*)des->val)
        return -1;
    else
        return 0;
}

int eggWResult_cmpval_int64_asc(HEGGWRESULT src, HEGGWRESULT des)
{
    if (*(int64_t *)src->val > *(int64_t*)des->val)
        return 1;
    else if (*(int64_t *)src->val < *(int64_t*)des->val)
        return -1;
    else
        return 0;
    
}

int eggWResult_cmpval_int64_desc(HEGGWRESULT src, HEGGWRESULT des)
{
    if (*(int64_t *)src->val < *(int64_t*)des->val)
        return 1;
    else if (*(int64_t *)src->val > *(int64_t*)des->val)
        return -1;
    else
        return 0;
    
}

int eggWResult_cmpval_double_asc(HEGGWRESULT src, HEGGWRESULT des)
{
    if (*(double *)src->val > *(double*)des->val)
        return 1;
    else if (*(double *)src->val < *(double*)des->val)
        return -1;
    else
        return 0;
    
}

int eggWResult_cmpval_double_desc(HEGGWRESULT src, HEGGWRESULT des)
{
    if (*(double *)src->val < *(double*)des->val)
        return 1;
    else if (*(double *)src->val > *(double*)des->val)
        return -1;
    else
        return 0;
    
}

int eggWResult_cmpid_desc(HEGGWRESULT src, HEGGWRESULT des)
{
    if (src->id < des->id)
        return 1;
    else if (src->id > des->id)
        return -1;
    else
        return 0;
    
}

int eggWResult_cmpid2_desc(HEGGWRESULT src, index_t idx, HEGGWRESULT des)
{
    if ((src+idx)->id < des->id)
        return 1;
    else if ((src+idx)->id > des->id)
        return -1;
    else
        return 0;
    
}

EBOOL eggFieldWeight_set_actinfo(HEGGFIELDWEIGHT hFieldWeight, ActInfo *hActInfo)
{
    if(EGGFIELDWEIGHT_IS_INVALID(hFieldWeight))
    {
        return EGG_FALSE;
    }
    
    EBOOL retv;
    retv  = ViewStream_set_actinfo(hFieldWeight->hViewStream, hActInfo);
    return retv;
}

EBOOL eggFieldWeight_clean_actinfo(HEGGFIELDWEIGHT hFieldWeight, ActInfo *hActInfo)
{
    if(EGGFIELDWEIGHT_IS_INVALID(hFieldWeight))
    {
        return EGG_FALSE;
    }
    
    EBOOL retv;
    retv  = ViewStream_clean_actinfo(hFieldWeight->hViewStream, hActInfo);
    return retv;
    
}
