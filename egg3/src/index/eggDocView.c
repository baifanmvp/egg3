
#include "eggDocView.h"
#include "../log/eggPrtLog.h"

PUBLIC HEGGDOCVIEW eggDocView_new(HEGGFILE hEggFile, HEGGIDTABLE hIdTable)
{
    HEGGDOCVIEW hEggDocView = (HEGGDOCVIEW)malloc(sizeof(EGGDOCVIEW));
    if (!hEggDocView)
    {
        return EGG_NULL;
    }

    hEggDocView->hViewStream = ViewStream_new(hEggFile);
    hEggDocView->info.addCnt = 0;
    hEggDocView->hIdTable = hIdTable;
    
    return hEggDocView;
}

PUBLIC EBOOL eggDocView_delete(HEGGDOCVIEW hEggDocView)
{
    if (EGGDOCVIEW_IS_INVALID(hEggDocView))
    {
        return EGG_FALSE;
    }
    
    ViewStream_delete(hEggDocView->hViewStream);
    
    if(hEggDocView->hIdTable)
    {
        eggIdTable_delete(hEggDocView->hIdTable);   
    }
    
    free(hEggDocView);
    
    return EGG_TRUE;

}

PUBLIC did_t eggDocView_add(HEGGDOCVIEW hEggDocView, HEGGDOCNODE hDocNode)
{
    if (EGGDOCVIEW_IS_INVALID(hEggDocView))
    {
        return EGG_FALSE;
    }
    //ViewStream_startlog(hEggDocView->hViewStream);
    EGGDOCVIEWINFO* lp_info = (EGGDOCVIEWINFO*)(&(EGGDOCVIEW_INFO(hEggDocView)));
    
    offset64_t n_doc_Off = (offset64_t)ViewStream_write(hEggDocView->hViewStream,
                                       hDocNode,
                                       hDocNode->size);
    
    did_t id = eggIdTable_map_id(hEggDocView->hIdTable, n_doc_Off);
    
    lp_info->addCnt++;
    //ViewStream_endlog(hEggDocView->hViewStream);
    return id;

}

PUBLIC EBOOL eggDocView_query(HEGGDOCVIEW hEggDocView, did_t id, HEGGDOCNODE* lphDocNode)
{
    if (EGGDOCVIEW_IS_INVALID(hEggDocView))
    {
        return EGG_FALSE;
    }

    if (lphDocNode == EGG_NULL)
    {
        return EGG_FALSE;
    }
    

    offset64_t n_doc_Off = eggIdTable_get_off(hEggDocView->hIdTable, id);
    
    if(n_doc_Off == TABLE_ID_DELETE)
    {
        return TABLE_ID_DELETE;
    }
    else if(n_doc_Off == TABLE_ID_OVERFLOW)
    {
        return TABLE_ID_OVERFLOW;
    }

    EBOOL eRet = EGG_FALSE;
    EGGDOCNODE st_dn_info = {0};


    eRet = ViewStream_read(hEggDocView->hViewStream,
                           &st_dn_info,
                           sizeof(EGGDOCNODE),
                           (offset64_t)n_doc_Off);
    
    *lphDocNode = (HEGGDOCNODE)malloc(st_dn_info.size);
    
    ViewStream_read(hEggDocView->hViewStream,
                    *lphDocNode,
                    st_dn_info.size,
                    (offset64_t)n_doc_Off);
 
    return EGG_TRUE;
}



PUBLIC EBOOL eggDocView_update(HEGGDOCVIEW hEggDocView, did_t id, HEGGDOCNODE hDocNode)
{
    if (EGGDOCVIEW_IS_INVALID(hEggDocView))
    {
        return EGG_FALSE;
    }

    
    
    EBOOL eRet = EGG_FALSE;
    EGGDOCNODE st_dn_info = {0};
    
    offset64_t n_doc_Off = eggIdTable_get_off(hEggDocView->hIdTable, id);

    if(n_doc_Off == TABLE_ID_DELETE)
    {
        
        return TABLE_ID_DELETE;
    }
    else if(n_doc_Off == TABLE_ID_OVERFLOW)
    {
            
        return TABLE_ID_OVERFLOW;
    }
    
    eRet = ViewStream_read(hEggDocView->hViewStream,
                           &st_dn_info,
                           sizeof(EGGDOCNODE),
                           (offset64_t)n_doc_Off);
    
    offset64_t oldOff = n_doc_Off;
    size32_t oldSz = st_dn_info.size;
    
    //ViewStream_startlog(hEggDocView->hViewStream);
    
    n_doc_Off = (offset64_t)ViewStream_write(hEggDocView->hViewStream,
                                             hDocNode,
                                             hDocNode->size);

    //ViewStream_endlog(hEggDocView->hViewStream);
    
    eggIdTable_update_off(hEggDocView->hIdTable, id, n_doc_Off);
    eggIdTable_unlock_id(hEggDocView->hIdTable, id);
     
//fprintf(stderr, "---------%s: off[%llu]len[%llu]\n", __func__, (long long unsigned)oldOff, (long long unsigned)oldSz);
    eggPrtLog_info("eggDocView", "%s: off[%llu]len[%llu]\n", __func__, (long long unsigned)oldOff, (long long unsigned)oldSz);
     ViewStream_free_area(hEggDocView->hViewStream, oldOff, oldSz);
    
    
    return EGG_TRUE;
}



PUBLIC EBOOL eggDocView_remove(HEGGDOCVIEW hEggDocView, did_t id)
{
    if (EGGDOCVIEW_IS_INVALID(hEggDocView))
    {
        return EGG_FALSE;
    }

    
    
    EBOOL eRet = EGG_FALSE;
    EGGDOCNODE st_dn_info = {0};

    offset64_t n_doc_Off = eggIdTable_get_off(hEggDocView->hIdTable, id);
    
    if(n_doc_Off == TABLE_ID_DELETE)
    {
        return TABLE_ID_DELETE;
    }
    else if(n_doc_Off == TABLE_ID_OVERFLOW)
    {

        return TABLE_ID_OVERFLOW;
    }
    
    eRet = ViewStream_read(hEggDocView->hViewStream,
                           &st_dn_info,
                           sizeof(EGGDOCNODE),
                           (offset64_t)n_doc_Off);
    

    eggIdTable_update_off(hEggDocView->hIdTable, id, TABLE_ID_DELETE);
    
//fprintf(stderr, "---------%s: off[%llu]len[%llu]\n", __func__, (long long unsigned)n_doc_Off, (long long unsigned)st_dn_info.size);
    eggPrtLog_info("eggDocView", "%s: off[%llu]len[%llu]\n", __func__, (long long unsigned)n_doc_Off, (long long unsigned)st_dn_info.size);
    ViewStream_free_area(hEggDocView->hViewStream, n_doc_Off, st_dn_info.size);
    
    return EGG_TRUE;
}



PUBLIC EBOOL eggDocView_update_info(HEGGDOCVIEW hEggDocView)
{
    if (EGGDOCVIEW_IS_INVALID(hEggDocView))
    {
        return EGG_FALSE;
    }

    EGGDOCVIEWINFO st_doc_info;
    //ViewStream_startlog(hEggDocView->hViewStream);
    ViewStream_read(hEggDocView->hViewStream, &st_doc_info, sizeof(st_doc_info), 0);
    
    st_doc_info.addCnt += EGGDOCVIEW_INFO(hEggDocView).addCnt;
    

    ViewStream_update(hEggDocView->hViewStream,
                      &st_doc_info,
                      sizeof(st_doc_info),
                      (offset64_t)0);
    
    EGGDOCVIEW_INFO(hEggDocView).addCnt = 0;
    //ViewStream_endlog(hEggDocView->hViewStream);
    return EGG_TRUE;
}

PUBLIC count_t eggDocView_get_doccnt(HEGGDOCVIEW hEggDocView)
{
    if (EGGDOCVIEW_IS_INVALID(hEggDocView))
    {
        return EGG_FALSE;
    }

#if 0    
    EGGDOCVIEWINFO st_doc_info;

    ViewStream_read(hEggDocView->hViewStream, &st_doc_info, sizeof(st_doc_info), 0);
    return st_doc_info.addCnt;
#else
    return hEggDocView->hIdTable->hIdTableInfo->docCnt;
#endif    
}

PUBLIC size64_t eggDocView_size(HEGGDOCVIEW hEggDocView)
{
    return ViewStream_file_size(hEggDocView->hViewStream);
}

EBOOL eggDocView_rdlock_id(HEGGDOCVIEW hEggDocView, did_t id)
{

    eggIdTable_rdlock_id(hEggDocView->hIdTable, id);

    return EGG_TRUE;
}

EBOOL eggDocView_wrlock_id(HEGGDOCVIEW hEggDocView, did_t id)
{
    
    eggIdTable_wrlock_id(hEggDocView->hIdTable, id);
    
    return EGG_TRUE;
}

EBOOL eggDocView_unlock_id(HEGGDOCVIEW hEggDocView, did_t id)
{
    eggIdTable_unlock_id(hEggDocView->hIdTable, id);
    
    return EGG_TRUE;
}

EBOOL eggDocView_set_actinfo(HEGGDOCVIEW hEggDocView, ActInfo *hActInfo)
{
    if (EGGDOCVIEW_IS_INVALID(hEggDocView))
    {
        return EGG_FALSE;
    }

    EBOOL retv;
    retv = ViewStream_set_actinfo(hEggDocView->hViewStream,
                                 hActInfo);
    return retv;
}

EBOOL eggDocView_clean_actinfo(HEGGDOCVIEW hEggDocView, ActInfo *hActInfo)
{
    if (EGGDOCVIEW_IS_INVALID(hEggDocView))
    {
        return EGG_FALSE;
    }

    EBOOL retv;
    retv = ViewStream_clean_actinfo(hEggDocView->hViewStream,
                                   hActInfo);
    return retv;
}
