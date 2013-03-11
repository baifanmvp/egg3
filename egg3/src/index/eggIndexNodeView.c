#include "eggIndexNodeView.h"

HEGGINDEXNODEVIEW eggIndexNodeView_new(HEGGINDEXNODEINFO hNodeInfo, type_t type)
{
    HEGGINDEXNODEVIEW lp_node_view = (HEGGINDEXNODEVIEW)malloc(sizeof(EGGINDEXNODEVIEW));
    size16_t n_node_size = sizeof(EGGINDEXNODE) + hNodeInfo->rdSize * (hNodeInfo->rdCnt + 1);
    
    lp_node_view->reInfo = hNodeInfo;
    lp_node_view->hNode = (HEGGINDEXNODE)malloc(n_node_size);
    lp_node_view->hNode->flag = 0;
    
    memset(lp_node_view->hNode, 0, n_node_size);
    lp_node_view->hNode->ntype = type;
    
    return lp_node_view;
}

EBOOL eggIndexNodeView_delete(HEGGINDEXNODEVIEW hNodeView)
{
    if(EGGINDEXNODEVIEW_IS_INVALID(hNodeView))
    {
        return EGG_FALSE;
    }

    free(hNodeView->hNode);
    free(hNodeView);
    hNodeView = EGG_NULL;
    
    return EGG_TRUE;
}


HEGGINDEXNODEVIEW eggIndexNodeView_split(HEGGINDEXNODEVIEW hNodeView, HEGGINDEXNODEVIEW hNewNodeView)
{
    if(EGGINDEXNODEVIEW_IS_INVALID(hNodeView))
    {
        return EGG_FALSE;
    }

    HEGGINDEXNODEINFO lp_node_info = hNodeView->reInfo;
    HEGGINDEXNODEVIEW lp_new_node_view = hNewNodeView;
    
    char* lp_new_records = (char*)(lp_new_node_view->hNode + 1);
    
    HEGGINDEXNODE lp_split_node = hNodeView->hNode;
    char* lp_split_records = (char*)(hNodeView->hNode + 1);
    
    size16_t n_tot_rdCnt = lp_split_node->rdCnt;
    index_t split_idx = lp_split_node->rdCnt / 2;
    lp_split_node->rdCnt = lp_split_node->rdCnt / 2;
    lp_split_records += lp_node_info->rdSize * split_idx;
    
    HEGGINDEXNODE lp_new_node = lp_new_node_view->hNode;  
    lp_new_node->rdCnt = n_tot_rdCnt - lp_split_node->rdCnt;

    memcpy(lp_new_records, lp_split_records, lp_node_info->rdSize * lp_new_node->rdCnt + sizeof(EGGINDEXRECORD));
    
    lp_new_node->parent = lp_split_node->parent;
    lp_new_node->ntype = lp_split_node->ntype;
    return lp_new_node_view;
}


EBOOL eggIndexNodeView_locate(HEGGINDEXNODEVIEW hNodeView, HEGGINDEXRECORD hRecord, index_t* pRdIdx)
{
    if(EGGINDEXNODEVIEW_IS_INVALID(hNodeView))
    {
        return EGG_FALSE;
    }
    
    HEGGINDEXNODEINFO lp_node_info = hNodeView->reInfo;
    HEGGINDEXNODE lp_node = hNodeView->hNode;
    
    char* lp_records = (char*)(lp_node + 1);
    
    size16_t n_record_size = lp_node_info->rdSize;
    size16_t n_record_cnt = lp_node->rdCnt;
    
    *pRdIdx  = 0;
    
    RDCMP fnRdCmp = eggIndexNode_get_fncmp(lp_node_info->type);
    
    while((*pRdIdx) != n_record_cnt)
    {
        int n_cmp_ret = fnRdCmp((HEGGINDEXRECORD)(lp_records + (*pRdIdx) * n_record_size), hRecord);
        
        if( n_cmp_ret == 0 && (lp_node_info->type & BTREE_RECORD_REPEAT))
        {
            n_cmp_ret = eggIndexRecord_cmp_with_id((HEGGINDEXRECORD)(lp_records + (*pRdIdx) * n_record_size), hRecord);
        }
        
        if(n_cmp_ret > 0)
        {
            return IDXNODE_NOT_FIND;
        }
        else if(n_cmp_ret == 0)
        {
            return IDXNODE_FIND;            
        }
        
        ++(*pRdIdx);
    }
    
    return IDXNODE_NOT_FIND;
    
}
EBOOL eggIndexNodeView_remove(HEGGINDEXNODEVIEW hNodeView, index_t nRdIdx)
{
    if(EGGINDEXNODEVIEW_IS_INVALID(hNodeView))
    {
        return EGG_FALSE;
    }
    
    HEGGINDEXNODEINFO lp_node_info = hNodeView->reInfo;
    HEGGINDEXNODE lp_node = hNodeView->hNode;
    
    char* lp_records = (char*)(lp_node + 1);
    
    size16_t n_record_size = lp_node_info->rdSize;
    size16_t n_record_cnt = lp_node->rdCnt;
    
    if(n_record_cnt <= 1 || n_record_cnt <=  nRdIdx || nRdIdx < 0)
    {
        return EGG_FALSE;
    }

    EMemMove(lp_records + nRdIdx * n_record_size,
             lp_records + (nRdIdx + 1) * n_record_size ,
             (n_record_cnt + 1 - nRdIdx) * n_record_size + sizeof(EGGINDEXRECORD));
    
    lp_node->rdCnt--;
    return EGG_TRUE;

}

EBOOL eggIndexNodeView_insert(HEGGINDEXNODEVIEW hNodeView, HEGGINDEXRECORD hRecord, index_t nRdIdx)
{
    if(EGGINDEXNODEVIEW_IS_INVALID(hNodeView))
    {
        return EGG_FALSE;
    }
    
    HEGGINDEXNODEINFO lp_node_info = hNodeView->reInfo;
    HEGGINDEXNODE lp_node = hNodeView->hNode;
    
    char* lp_records = (char*)(lp_node + 1);
    
    size16_t n_record_size = lp_node_info->rdSize;
    size16_t n_record_cnt = lp_node->rdCnt;
    
    if(n_record_cnt == lp_node_info->rdCnt)
    {
        return EGG_FALSE;
    }
    
    if(n_record_cnt - nRdIdx)
    {
        EMemMove(lp_records + (nRdIdx + 1) * n_record_size,
                 lp_records + nRdIdx * n_record_size ,
                 (n_record_cnt  - nRdIdx) * n_record_size + sizeof(EGGINDEXRECORD));
    }

    memcpy(lp_records + nRdIdx * n_record_size,
           hRecord, n_record_size);
    
    lp_node->rdCnt++;
    
    return EGG_TRUE;
}

offset64_t eggIndexNodeView_get_preoff(HEGGINDEXNODEVIEW hNodeView)
{
    if(EGGINDEXNODEVIEW_IS_INVALID(hNodeView))
    {
        return EGG_FALSE;
    }
    
    HEGGINDEXNODEINFO lp_node_info = hNodeView->reInfo;
    HEGGINDEXNODE lp_node = hNodeView->hNode;
    
    if(lp_node->ntype != IDXNODE_IS_LEAF)
    {
        return EGG_FALSE;        
    }
    HEGGINDEXRECORD lp_record_last = (HEGGINDEXRECORD)((char*)(lp_node + 1) + lp_node_info->rdSize * lp_node_info->rdCnt); 
    HEGGINDEXLEAFLST lp_node_lst = (HEGGINDEXLEAFLST)(lp_record_last + 1);
    return lp_node_lst->pre;
}

EBOOL eggIndexNodeView_set_preoff(HEGGINDEXNODEVIEW hNodeView, offset64_t preOff)
{
    if(EGGINDEXNODEVIEW_IS_INVALID(hNodeView))
    {
        return EGG_FALSE;
    }
    
    HEGGINDEXNODEINFO lp_node_info = hNodeView->reInfo;
    HEGGINDEXNODE lp_node = hNodeView->hNode;
    
    if(lp_node->ntype != IDXNODE_IS_LEAF)
    {
        return EGG_FALSE;        
    }
    HEGGINDEXRECORD lp_record_last = (HEGGINDEXRECORD)((char*)(lp_node + 1) + lp_node_info->rdSize * lp_node_info->rdCnt); 
    HEGGINDEXLEAFLST lp_node_lst = (HEGGINDEXLEAFLST)(lp_record_last + 1);
    
    lp_node_lst->pre = preOff;

    return EGG_TRUE;
}

EBOOL eggIndexNodeView_set_nextoff(HEGGINDEXNODEVIEW hNodeView, offset64_t nextOff)
{
    if(EGGINDEXNODEVIEW_IS_INVALID(hNodeView))
    {
        return EGG_FALSE;
    }
    
    HEGGINDEXNODEINFO lp_node_info = hNodeView->reInfo;
    HEGGINDEXNODE lp_node = hNodeView->hNode;
    
    if(lp_node->ntype != IDXNODE_IS_LEAF)
    {
        return EGG_FALSE;        
    }
    HEGGINDEXRECORD lp_record_last = (HEGGINDEXRECORD)((char*)(lp_node + 1) + lp_node_info->rdSize * lp_node_info->rdCnt); 
    HEGGINDEXLEAFLST lp_node_lst = (HEGGINDEXLEAFLST)(lp_record_last + 1);
    
    lp_node_lst->next = nextOff;

    return EGG_TRUE;

}


offset64_t eggIndexNodeView_get_nextoff(HEGGINDEXNODEVIEW hNodeView)
{
    if(EGGINDEXNODEVIEW_IS_INVALID(hNodeView))
    {
        return EGG_FALSE;
    }
    
    HEGGINDEXNODEINFO lp_node_info = hNodeView->reInfo;
    HEGGINDEXNODE lp_node = hNodeView->hNode;
    
    if(lp_node->ntype != IDXNODE_IS_LEAF)
    {
        return EGG_FALSE;        
    }
    HEGGINDEXRECORD lp_record_last = (HEGGINDEXRECORD)((char*)(lp_node + 1) + lp_node_info->rdSize * lp_node_info->rdCnt); 
    HEGGINDEXLEAFLST lp_node_lst = (HEGGINDEXLEAFLST)(lp_record_last + 1);
    
    return lp_node_lst->next;
}

HEGGINDEXLEAFLST eggIndexNodeView_get_lstinfo(HEGGINDEXNODEVIEW hNodeView)
{
    if(EGGINDEXNODEVIEW_IS_INVALID(hNodeView))
    {
        return EGG_NULL;
    }
    
    HEGGINDEXNODEINFO lp_node_info = hNodeView->reInfo;
    HEGGINDEXNODE lp_node = hNodeView->hNode;
    
    if(lp_node->ntype != IDXNODE_IS_LEAF)
    {
        return EGG_NULL;        
    }
    HEGGINDEXRECORD lp_record_last = (HEGGINDEXRECORD)((char*)(lp_node + 1) + lp_node_info->rdSize * lp_node_info->rdCnt); 
    HEGGINDEXLEAFLST lp_node_lst = (HEGGINDEXLEAFLST)(lp_record_last + 1);
    
    return lp_node_lst;
}

