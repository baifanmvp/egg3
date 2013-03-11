#include "eggIdView.h"
#define EGG_IDINTEGRATION_LIMIT (100000)

PUBLIC HEGGIDVIEW eggIdView_new(HEGGFILE hEggFile)
{
    HEGGIDVIEW hEggIdView = (HEGGIDVIEW)malloc(sizeof(EGGIDVIEW));
    
    if (EGGIDVIEW_IS_INVALID(hEggIdView))
    {
        return EGG_NULL;
    }
    
    
    hEggIdView->hViewStream = ViewStream_new(hEggFile);
    hEggIdView->hEggListView = eggListView_new(hEggIdView->hViewStream);
    
    return hEggIdView;
}

PUBLIC EBOOL eggIdView_delete(HEGGIDVIEW hEggIdView)
{
    if (EGGIDVIEW_IS_INVALID(hEggIdView))
    {
        return EGG_FALSE;
    }
    
    eggListView_delete(hEggIdView->hEggListView);
    ViewStream_delete(hEggIdView->hViewStream);
    
    free(hEggIdView);
    
    return EGG_TRUE;
    
}

PUBLIC EBOOL eggIdView_load(HEGGIDVIEW hEggIdView, offset64_t nInfoOff)
{
    if (EGGIDVIEW_IS_INVALID(hEggIdView))
    {
        return EGG_FALSE;
    }
    
    return eggListView_load_info(hEggIdView->hEggListView, nInfoOff);
}

PUBLIC EBOOL eggIdView_reg(HEGGIDVIEW hEggIdView, HEGGLISTINF hInfo)
{
    if (EGGIDVIEW_IS_INVALID(hEggIdView))
    {
        return EGG_FALSE;
    }
    
    return eggListView_reg_info(hEggIdView->hEggListView, hInfo);
}


PUBLIC EBOOL eggIdView_add(HEGGIDVIEW hEggIdView, HEGGIDNODE hEggIdNodes, count_t nIdCnt)
{
    if (EGGIDVIEW_IS_INVALID(hEggIdView))
    {
        return EGG_FALSE;
    }
    
    eggIdNode_set_timestamp(hEggIdNodes, nIdCnt, hEggIdView->hEggListView->hInfo->curCnt + EGG_IDTIMESTAMP_BASEVAL);
    
    eggListView_insert(hEggIdView->hEggListView, hEggIdNodes, nIdCnt * sizeof(EGGIDNODE));
    
    if(hEggIdView->hEggListView->hInfo->curCnt > EGG_IDINTEGRATION_LIMIT)
    {
        printf("eggIdView_integration\n");
//        getchar();
        eggIdView_integration(hEggIdView);
     
    }
    return EGG_TRUE;
        
    
}

PUBLIC EBOOL eggIdView_find(HEGGIDVIEW hEggIdView, offset64_t nListOff, HEGGIDNODE* hEggIdNodes, count_t *lpCount)
{
    if (EGGIDVIEW_IS_INVALID(hEggIdView))
    {
        return EGG_FALSE;
    }
    if(nListOff < 65536)
    {
        return EGG_FALSE;
    }
    size32_t nNodeSz = (*lpCount)*sizeof(EGGIDNODE);

    eggListView_load_info(hEggIdView->hEggListView, nListOff);
    
    eggListView_fetch(hEggIdView->hEggListView, hEggIdNodes, &nNodeSz);

    *lpCount = nNodeSz/sizeof(EGGIDNODE);
    
//    Uti_sedgesort (*hEggIdNodes, *lpCount, sizeof(EGGIDNODE), eggIdNode_cmp_id);
    
    return EGG_TRUE;
}

PUBLIC EBOOL eggIdView_integration(HEGGIDVIEW hEggIdView)
{
    HEGGIDNODE lp_idnodes = EGG_NULL;
    size32_t n_nodes_sz = 0;
    
    eggListView_fetch(hEggIdView->hEggListView, &lp_idnodes, &n_nodes_sz);

    count_t n_nodes_cnt = n_nodes_sz/sizeof(EGGIDNODE);
    if(n_nodes_cnt)
    {
        Uti_sedgesort (lp_idnodes, n_nodes_cnt, sizeof(EGGIDNODE), eggIdNode_cmp_id3);
        
        eggIdNode_filter_repeat(lp_idnodes, &n_nodes_cnt);
        
        eggIdNode_reset_timestamp(lp_idnodes, n_nodes_cnt);
        
        Uti_sedgesort (lp_idnodes, n_nodes_cnt, sizeof(EGGIDNODE), eggIdNode_cmp_weight);
        
        eggListView_rewrite(hEggIdView->hEggListView, lp_idnodes, n_nodes_cnt * sizeof(EGGIDNODE));
        
        free(lp_idnodes);
    }
    return EGG_TRUE;
}


PUBLIC EBOOL eggIdView_update_info(HEGGIDVIEW hEggIdView)
{
    if(EGGIDVIEW_IS_INVALID(hEggIdView))
    {
        return EGG_FALSE;
    }
    
    eggListView_update_info(hEggIdView->hEggListView);
    
    return EGG_TRUE;
}

EBOOL eggIdView_set_actinfo(HEGGIDVIEW hEggIdView, ActInfo *hActInfo)
{
    if(EGGIDVIEW_IS_INVALID(hEggIdView))
    {
        return EGG_FALSE;
    }
    
    EBOOL retv;
    retv  = ViewStream_set_actinfo(hEggIdView->hViewStream, hActInfo);
    return retv;
}

EBOOL eggIdView_clean_actinfo(HEGGIDVIEW hEggIdView, ActInfo *hActInfo)
{
    if(EGGIDVIEW_IS_INVALID(hEggIdView))
    {
        return EGG_FALSE;
    }
    
    EBOOL retv;
    retv  = ViewStream_clean_actinfo(hEggIdView->hViewStream, hActInfo);
    return retv;
    
}
