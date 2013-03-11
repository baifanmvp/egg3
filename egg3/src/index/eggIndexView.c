#include "eggIndexView.h"
#include "../log/eggPrtLog.h"


PRIVATE offset64_t eggIndexView_split(HEGGINDEXVIEW hEggIndexView, HEGGINDEXNODEVIEW hNodeViewSrc);

PRIVATE EBOOL eggIndexView_update_childNode_info(HEGGINDEXVIEW hEggIndexView, HEGGINDEXNODEVIEW hNodeView);

PRIVATE offset64_t eggIndexView_create_rootNode(HEGGINDEXVIEW hEggIndexView);

PRIVATE HEGGINDEXRECORD eggIndexView_relocate(HEGGINDEXVIEW hEggIndexView, void* key, size32_t kSz, void* val, size32_t vSz, offset64_t* pNdPos, index_t* pRdIdx);

PRIVATE EBOOL eggIndexView_set_rangeId(HEGGINDEXVIEW hEggIndexView, HEGGRANGEID hRangeId, HEGGINDEXRECORD hRecord, HEGGINDEXRECORD hLastRecord);

PUBLIC HEGGINDEXVIEW eggIndexView_new(HEGGFILE hEggFile, HEGGINDEXINFO hInfo)

{
    if(!hEggFile)
    {
        return EGG_NULL;
    }

    HEGGINDEXVIEW lp_index_view = (HEGGINDEXVIEW)malloc(sizeof(EGGINDEXVIEW));
    
    lp_index_view->hInfo = hInfo;

    lp_index_view->hFieldView = EGG_NULL;
    
    lp_index_view->hViewStream = ViewStream_new(hEggFile);

    pthread_mutex_init( &lp_index_view->mutex, NULL);

    return lp_index_view;
}

PUBLIC EBOOL eggIndexView_delete(HEGGINDEXVIEW hEggIndexView)
{
    if(EGGINDEXVIEW_IS_INVALID(hEggIndexView))
    {
        return EGG_FALSE;
    }
    if(hEggIndexView->hInfo)
    {
        free(hEggIndexView->hInfo);
        hEggIndexView->hInfo = 0;
    }
    pthread_mutex_destroy( &hEggIndexView->mutex);

    ViewStream_delete(hEggIndexView->hViewStream);
    hEggIndexView->hViewStream = 0;
    free(hEggIndexView);


    return EGG_TRUE;
}

PUBLIC EBOOL eggIndexView_load_info(HEGGINDEXVIEW hEggIndexView, HEGGINDEXINFO hInfo,HEGGFIELDVIEW hFieldView)
{
    if(! hEggIndexView)
    {
        return EGG_FALSE;
    }
    if(hEggIndexView->hInfo)
    {
        free(hEggIndexView->hInfo);
        hEggIndexView->hInfo = 0;
    }
    hEggIndexView->hInfo = hInfo;
    hEggIndexView->hFieldView = hFieldView;


    return EGG_TRUE;
}


PUBLIC EBOOL eggIndexView_insert(HEGGINDEXVIEW hEggIndexView, void* key, size32_t kSz, void* val, size32_t vSz)
{
    if(EGGINDEXVIEW_IS_INVALID(hEggIndexView))
    {
        return EGG_FALSE;
    }

//    ViewStream_startlog(hEggIndexView->hViewStream);

        


    offset64_t ndPos;
    index_t rdIdx;
    // printf("--------------%.*s\n", kSz, key);
    HEGGINDEXRECORD lp_record = eggIndexView_locate(hEggIndexView, key, kSz, val, vSz, &ndPos, &rdIdx);
    if(lp_record)
    {
        if(hEggIndexView->hInfo->type & BTREE_RECORD_REPEAT)
        {
            HEGGRECORDDOCID p_record_id = EGGINDEXRECORD_VAL(lp_record);
            eggIndexView_update_recordvalue(hEggIndexView, ndPos, rdIdx, val, vSz);
            
            free(lp_record);
            return EGG_TRUE;
        }
        
        free(lp_record);
//        ViewStream_endlog(hEggIndexView->hViewStream);        
        return EGG_FALSE;
    }
    else
    {
        eggIndexView_insert_pos(hEggIndexView, key, kSz, val, vSz, ndPos, rdIdx);
//        ViewStream_endlog(hEggIndexView->hViewStream);
        return EGG_TRUE;
    }
}

PUBLIC HEGGINDEXRECORD eggIndexView_fetch(HEGGINDEXVIEW hEggIndexView, void* key, size32_t kSz, void* val, size32_t vSz)
{
    if(EGGINDEXVIEW_IS_INVALID(hEggIndexView))
    {
        return EGG_FALSE;
    }
    offset64_t ndPos;
    index_t rdIdx;
    HEGGINDEXRECORD lp_record = eggIndexView_locate(hEggIndexView, key, kSz, val, vSz, &ndPos, &rdIdx);
    return lp_record;
}

PUBLIC HEGGINDEXRECORD eggIndexView_fetch_withfid(HEGGINDEXVIEW hEggIndexView, fdid_t fid, void* key, size32_t kSz, void* val, size32_t vSz)
{
    HEGGFIELDVIEW hFieldView = EGGINDEXVIEW_FIELDVIEW(hEggIndexView);
    
    pthread_mutex_lock(&hEggIndexView->mutex);
    if(hEggIndexView->hInfo)
    {
        free(hEggIndexView->hInfo);
    }
    hEggIndexView->hInfo = eggFieldView_get_indexinfo(hFieldView, fid);
    HEGGINDEXRECORD lp_record_ret = eggIndexView_fetch(hEggIndexView, key, kSz,val, vSz);
    
    pthread_mutex_unlock(&hEggIndexView->mutex);
    
    return lp_record_ret;
                                                                                    
}


PUBLIC HEGGINDEXRANGE eggIndexView_fetch_docid_withfid(HEGGINDEXVIEW hEggIndexView, fdid_t fid, void* key, size32_t kSz, void* val, size32_t vSz)
{
    HEGGFIELDVIEW hFieldView = EGGINDEXVIEW_FIELDVIEW(hEggIndexView);
    
    pthread_mutex_lock(&hEggIndexView->mutex);
    if(hEggIndexView->hInfo)
    {
        free(hEggIndexView->hInfo);
    }
    hEggIndexView->hInfo = eggFieldView_get_indexinfo(hFieldView, fid);
    HEGGINDEXRANGE lp_rangeids_ret = eggIndexView_fetch_docid(hEggIndexView, key, kSz);
    
    pthread_mutex_unlock(&hEggIndexView->mutex);
    
    return lp_rangeids_ret;
                                                                                    
}
PUBLIC HEGGINDEXRANGE eggIndexView_range_withfid(HEGGINDEXVIEW hEggIndexView, fdid_t fid, void* startKey, size32_t startSz, void* endKey, size32_t endSz)
{
    HEGGFIELDVIEW hFieldView = EGGINDEXVIEW_FIELDVIEW(hEggIndexView);
    
    pthread_mutex_lock(&hEggIndexView->mutex);
    if(hEggIndexView->hInfo)
    {
        free(hEggIndexView->hInfo);
    }
    hEggIndexView->hInfo = eggFieldView_get_indexinfo(hFieldView, fid);
    
    HEGGINDEXRANGE lp_rangeids_ret = eggIndexView_range(hEggIndexView, startKey, startSz, endKey, endSz);
    
    
    pthread_mutex_unlock(&hEggIndexView->mutex);
    
    return lp_rangeids_ret;
                                                                                    
}


PUBLIC HEGGINDEXNODEVIEW eggIndexView_load_node(HEGGINDEXVIEW hEggIndexView, offset64_t nodeOff)
{
    if(EGGINDEXVIEW_IS_INVALID(hEggIndexView))
    {
        return EGG_NULL;
    }
    HEGGINDEXINFO lp_info = hEggIndexView->hInfo;
    HEGGINDEXNODEVIEW lp_node_view = eggIndexNodeView_new((HEGGINDEXNODEINFO)(hEggIndexView->hInfo), IDXNODE_IS_NORMAL);
    size16_t n_node_size = sizeof(EGGINDEXNODE) + lp_info->rdSize * (lp_info->rdCnt + 1);
    
    ViewStream_read(hEggIndexView->hViewStream, lp_node_view->hNode, n_node_size, nodeOff);
    
    return lp_node_view;
}

PUBLIC EBOOL eggIndexView_insert_pos(HEGGINDEXVIEW hEggIndexView, void* key, size16_t kSz, void* val, size16_t vSz, offset64_t ndPos, index_t rdIdx)
{
    if(EGGINDEXVIEW_IS_INVALID(hEggIndexView))
    {
        return EGG_NULL;
    }
//    ViewStream_startlog(hEggIndexView->hViewStream);
    
    HEGGINDEXINFO lp_info = hEggIndexView->hInfo;
    HEGGINDEXRECORD lp_record = eggIndexRecord_new(lp_info->rdSize, key, kSz, val, vSz);
    HVIEWSTREAM lp_view_stream = hEggIndexView->hViewStream;    
    size16_t n_node_size = sizeof(EGGINDEXNODE) + lp_info->rdSize * (lp_info->rdCnt + 1);
    
    if(!lp_info->rootOff)
    {
        lp_info->rootOff = eggIndexView_create_rootNode(hEggIndexView);
        lp_info->leafOff = lp_info->rootOff;
        ndPos = lp_info->rootOff;
        rdIdx = 0;
    }
    

    HEGGINDEXNODEVIEW lp_node_iter_view = eggIndexNodeView_new(lp_info, IDXNODE_IS_NORMAL);
    ViewStream_read(lp_view_stream, lp_node_iter_view->hNode, n_node_size, ndPos );
    lp_node_iter_view->nodeOff = ndPos;
    
    eggIndexNodeView_insert(lp_node_iter_view, lp_record, rdIdx);
    
    if(EGGINDEXNODEVIEW_RDCNT(lp_node_iter_view) == lp_info->rdCnt)
    {
        offset64_t node_iter_off = eggIndexView_split(hEggIndexView, lp_node_iter_view);
        lp_info->rootOff = node_iter_off? node_iter_off:lp_info->rootOff;
    }
    else
    {
        ViewStream_update(lp_view_stream, lp_node_iter_view->hNode,
                          n_node_size, EGGINDEXNODEVIEW_OWNEROFF(lp_node_iter_view));
    }
    eggIndexRecord_delete(lp_record);
    eggIndexNodeView_delete(lp_node_iter_view);
//    ViewStream_endlog(hEggIndexView->hViewStream);
    
    return EGG_TRUE;
}

PUBLIC HEGGINDEXRECORD eggIndexView_locate(HEGGINDEXVIEW hEggIndexView, void* key, size32_t kSz, void* val, size32_t vSz, offset64_t* pNdPos, index_t* pRdIdx)
{
    if(EGGINDEXVIEW_IS_INVALID(hEggIndexView))
    {
        return EGG_NULL;
    }
    
    HEGGINDEXINFO lp_info = hEggIndexView->hInfo;
    offset64_t node_iter_off = lp_info->rootOff;
    HVIEWSTREAM lp_view_stream = hEggIndexView->hViewStream;
    
    size16_t n_node_size = sizeof(EGGINDEXNODE) + lp_info->rdSize * (lp_info->rdCnt + 1);

    HEGGINDEXNODEVIEW lp_node_iter_view = eggIndexNodeView_new((HEGGINDEXNODEINFO)lp_info, IDXNODE_IS_NORMAL);
    index_t n_idx_iter = 0;
    HEGGINDEXRECORD lp_record = eggIndexRecord_new(hEggIndexView->hInfo->rdSize, key, kSz, val, vSz);
    
    if((lp_info->type & EGG_RANGE_INDEX) && (!val || vSz != sizeof(EGGRECORDDOCID) ))
    {
        //printf("btree is range index, val and vSz can't error!\n");
        eggPrtLog_error("eggIndexView", "btree is range index, val and vSz can't error!\n");
        return EGG_NULL;
    }
    //start node
    while(node_iter_off)
    {
        ViewStream_read(lp_view_stream, lp_node_iter_view->hNode, n_node_size, node_iter_off);
        
        if(IDXNODE_IS_LOCKED & lp_node_iter_view->hNode->flag)
        {
            //  if the node is locked
            eggIndexNodeView_delete(lp_node_iter_view);
            eggIndexRecord_delete(lp_record);
            //printf("if the node is locked\n");
            eggPrtLog_info("eggIndexView", "if the node is locked\n");
            return eggIndexView_relocate(hEggIndexView, key, kSz, val, vSz, pNdPos, pRdIdx);
        }
        
        if(eggIndexNodeView_locate(lp_node_iter_view, lp_record, &n_idx_iter) == IDXNODE_FIND)
        {
            memcpy(lp_record, EGGINDEXNODEVIEW_RECORD_INDEX(lp_node_iter_view, n_idx_iter), lp_info->rdSize);
            
            if(lp_node_iter_view->hNode->ntype == IDXNODE_IS_LEAF)
            {
                *pNdPos = node_iter_off;
                *pRdIdx = n_idx_iter;
            }
            else
            {
                *pNdPos = lp_record->hostOff;
                *pRdIdx = 0;
                
                ViewStream_read(lp_view_stream, lp_node_iter_view->hNode, n_node_size, lp_record->hostOff);
                
                memcpy(lp_record, EGGINDEXNODEVIEW_RECORD_INDEX(lp_node_iter_view, *pRdIdx), lp_info->rdSize);
            }
            eggIndexNodeView_delete(lp_node_iter_view);
            return lp_record;
        }
        else
        {
            HEGGINDEXRECORD lp_record_tmp = EGGINDEXNODEVIEW_RECORD_INDEX(lp_node_iter_view, n_idx_iter);        
            *pNdPos = node_iter_off;
            *pRdIdx = n_idx_iter;
            node_iter_off = lp_record_tmp->childOff;
        }
    }
    
    eggIndexNodeView_delete(lp_node_iter_view);
    eggIndexRecord_delete(lp_record);
    return EGG_NULL;
}

PUBLIC HEGGINDEXRANGE eggIndexView_fetch_docid(HEGGINDEXVIEW hEggIndexView, void* key, size32_t kSz)
{
    if(EGGINDEXVIEW_IS_INVALID(hEggIndexView))
    {
        return EGG_NULL;
    }
    HEGGINDEXINFO lp_info = hEggIndexView->hInfo;
    if(!(lp_info->type & BTREE_RECORD_REPEAT))
    {
        return EGG_NULL;
    }
    
    offset64_t n_rd_off = 0;
    index_t    n_rd_idx = 0;
    struct eggRecordDocId st_docId = {0};
    RDCMP fnRdCmp = eggIndexNode_get_fncmp(lp_info->type);

    st_docId.did = RECORDDOCID_MAX;
    HEGGINDEXRECORD lp_record = eggIndexView_locate(hEggIndexView, key, kSz, &st_docId, sizeof(st_docId), &n_rd_off, &n_rd_idx);
    
    void* lp_src_key = key;
    size32_t n_src_ksz = kSz;
    char md5_key[UTI_MD5_DIGEST_LENGTH];

    //MD5 change
    if((size32_t)lp_info->rdSize < sizeof(EGGINDEXRECORD) + kSz + sizeof(EGGRECORDDOCID))
    {
//        kSz = MD5_DIGEST_LENGTH;
        eggUtiMD5(key, kSz, md5_key);
        n_src_ksz = UTI_MD5_DIGEST_LENGTH;
        lp_src_key = md5_key;
    }
    

    HEGGINDEXRANGE lp_index_range = (HEGGINDEXRANGE)malloc(sizeof(EGGINDEXRANGE));
    UTIVECTOR* lp_uti_vector = Uti_vector_create(sizeof(EGGRANGEID));
    EGGRANGEID st_range_id = {0};
    while(n_rd_off)
    {
        HEGGINDEXNODEVIEW lp_indexnode_view = eggIndexView_load_node(hEggIndexView, n_rd_off);
        
        while(EGGINDEXNODEVIEW_RDCNT(lp_indexnode_view) != n_rd_idx)
        {
            HEGGINDEXRECORD lp_record_tmp = EGGINDEXNODEVIEW_RECORD_INDEX(lp_indexnode_view, n_rd_idx);
            HEGGRECORDDOCID lp_record_id = EGGINDEXRECORD_VAL(lp_record_tmp);
            
            void* lp_key_tmp = EGGINDEXRECORD_KEY(lp_record_tmp);
            size32_t n_key_sz = EGGINDEXRECORD_KSIZE(lp_record_tmp);
            
            if(n_key_sz != n_src_ksz || memcmp(lp_key_tmp, lp_src_key, n_src_ksz) != 0)
            {
                eggIndexNodeView_delete(lp_indexnode_view);
                goto fetchId_end;
            }
            else if (lp_record_id->flag == RECORDDOCID_VALID)
            {
                st_range_id.did = lp_record_id->did;
                Uti_vector_push(lp_uti_vector, &st_range_id, 1);
            }
            n_rd_idx++;
        }
        n_rd_off = eggIndexNodeView_get_nextoff(lp_indexnode_view);
        eggIndexNodeView_delete(lp_indexnode_view);
        n_rd_idx = 0;
    }
    
fetchId_end:
    lp_index_range->cnt = Uti_vector_count(lp_uti_vector);
    lp_index_range->dids = Uti_vector_data(lp_uti_vector);
    
    Uti_vector_destroy(lp_uti_vector, 0);
    
    return lp_index_range;

    
}


PUBLIC HEGGINDEXRANGE eggIndexView_range(HEGGINDEXVIEW hEggIndexView, void* startKey, size32_t startSz, void* endKey, size32_t endSz)
{
    if(EGGINDEXVIEW_IS_INVALID(hEggIndexView))
    {
        return EGG_NULL;
    }
    HEGGINDEXINFO lp_info = hEggIndexView->hInfo;
    if(!(lp_info->type & BTREE_RECORD_REPEAT))
    {
        
        return EGG_NULL;
    }

    HEGGINDEXRECORD lp_start_rd = eggIndexRecord_new(lp_info->rdSize, startKey, startSz, EGG_NULL, 0);
    HEGGINDEXRECORD lp_end_rd = eggIndexRecord_new(lp_info->rdSize, endKey, endSz, EGG_NULL, 0);
    
    RDCMP fnRdCmp = eggIndexNode_get_fncmp(lp_info->type);
    
    if(fnRdCmp(lp_start_rd, lp_end_rd) > 0)
    {
        eggIndexRecord_delete(lp_start_rd);
        eggIndexRecord_delete(lp_end_rd);
        return EGG_NULL;
    }
    eggIndexRecord_delete(lp_start_rd);
    eggIndexRecord_delete(lp_end_rd);

    HEGGINDEXRANGE lp_index_range = (HEGGINDEXRANGE)malloc(sizeof(EGGINDEXRANGE));
    UTIVECTOR* lp_uti_vector = Uti_vector_create(sizeof(EGGRANGEID));
    
    offset64_t n_start_rdOff;
    offset64_t n_end_rdOff;
    index_t    n_start_rdIdx;
    index_t    n_end_rdIdx;
    struct eggRecordDocId st_docId = {0};

    st_docId.did = RECORDDOCID_MAX;
    HEGGINDEXRECORD lp_start_record = eggIndexView_locate(hEggIndexView, startKey, startSz, &st_docId, sizeof(st_docId), &n_start_rdOff, &n_start_rdIdx);
    
    st_docId.did = RECORDDOCID_MIN;
    HEGGINDEXRECORD lp_end_record = EGG_NULL;
    if(!(lp_end_record = eggIndexView_locate(hEggIndexView, endKey, endSz, &st_docId, sizeof(st_docId),  &n_end_rdOff, &n_end_rdIdx)))
    {
        n_end_rdIdx--;
    }
    
    if (lp_start_record)
    {
        eggIndexRecord_delete(lp_start_record);
    }
    if (lp_end_record)
    {
        eggIndexRecord_delete(lp_end_record);
    }

    EGGRANGEID st_range_id = {0};
    HEGGINDEXRECORD lp_last_record = EGG_NULL;
    
    while(n_start_rdOff != n_end_rdOff)
    {
        HEGGINDEXNODEVIEW lp_indexnode_view = eggIndexView_load_node(hEggIndexView, n_start_rdOff);
        
        while(EGGINDEXNODEVIEW_RDCNT(lp_indexnode_view) != n_start_rdIdx)
        {
            HEGGINDEXRECORD lp_record = EGGINDEXNODEVIEW_RECORD_INDEX(lp_indexnode_view, n_start_rdIdx);

            if(eggIndexView_set_rangeId(hEggIndexView, &st_range_id, lp_record, lp_last_record) == EGG_TRUE)
            {
                Uti_vector_push(lp_uti_vector, &st_range_id, 1);
            }
            lp_last_record = lp_record;
            
            n_start_rdIdx++;
        }
        n_start_rdOff = eggIndexNodeView_get_nextoff(lp_indexnode_view);
        eggIndexNodeView_delete(lp_indexnode_view);
        n_start_rdIdx = 0;
    }
    
    HEGGINDEXNODEVIEW lp_indexnode_view = eggIndexView_load_node(hEggIndexView, n_start_rdOff);    
    while(n_start_rdIdx <= n_end_rdIdx)
    {
        HEGGINDEXRECORD lp_record = EGGINDEXNODEVIEW_RECORD_INDEX(lp_indexnode_view, n_start_rdIdx);
        if(eggIndexView_set_rangeId(hEggIndexView, &st_range_id, lp_record, lp_last_record) == EGG_TRUE)
        {
            Uti_vector_push(lp_uti_vector, &st_range_id, 1);
        }
        lp_last_record = lp_record;

        n_start_rdIdx++;
    }
    eggIndexNodeView_delete(lp_indexnode_view);

    lp_index_range->cnt = Uti_vector_count(lp_uti_vector);
    lp_index_range->dids = Uti_vector_data(lp_uti_vector);
    
    Uti_vector_destroy(lp_uti_vector, 0);
    
    return lp_index_range;
    
}
PUBLIC EBOOL eggIndexView_update_record(HEGGINDEXVIEW hEggIndexView, HEGGINDEXRECORD hRecord, offset64_t ndPos, index_t rdIdx)
{
    if(EGGINDEXVIEW_IS_INVALID(hEggIndexView))
    {
        return EGG_NULL;
    }
//    ViewStream_startlog(hEggIndexView->hViewStream);
    
    HEGGINDEXINFO lp_info = hEggIndexView->hInfo;
    HVIEWSTREAM lp_view_stream = hEggIndexView->hViewStream;    
    
    offset64_t n_record_off = (ndPos) + sizeof(EGGINDEXNODE) + (offset64_t)((rdIdx)*lp_info->rdSize);

    ViewStream_update(lp_view_stream, hRecord, lp_info->rdSize, n_record_off);
//    ViewStream_endlog(hEggIndexView->hViewStream);
    
    return EGG_TRUE;
    
}

PUBLIC EBOOL eggIndexView_update_recordvalue(HEGGINDEXVIEW hEggIndexView, offset64_t ndPos, index_t rdIdx, void* val, size32_t vSz)
{
    if(EGGINDEXVIEW_IS_INVALID(hEggIndexView))
    {
        return EGG_NULL;
    }
//    ViewStream_startlog(hEggIndexView->hViewStream);
    HEGGINDEXINFO lp_info = hEggIndexView->hInfo;
    HVIEWSTREAM lp_view_stream = hEggIndexView->hViewStream;    
    
    offset64_t n_record_off = (ndPos) + sizeof(EGGINDEXNODE) + (offset64_t)((rdIdx)*lp_info->rdSize);

    HEGGINDEXRECORD lp_record = (HEGGINDEXRECORD)malloc(lp_info->rdSize);
    ViewStream_read(lp_view_stream, lp_record, lp_info->rdSize, n_record_off );

    if(vSz != (size32_t)EGGINDEXRECORD_VSIZE(lp_record))
    {
        free(lp_record);
        return EGG_NULL;
    }
    memcpy(EGGINDEXRECORD_VAL(lp_record), val, vSz);

    ViewStream_update(lp_view_stream, lp_record, lp_info->rdSize, n_record_off );
    free(lp_record);
//    ViewStream_endlog(hEggIndexView->hViewStream);
    
    return EGG_TRUE;
    
}
PUBLIC HEGGINDEXRANGE eggIndexView_export_ids(HEGGINDEXVIEW hEggIndexView)
{
    if(!(hEggIndexView->hInfo->type & BTREE_RECORD_REPEAT))
    {
        return EGG_NULL;
    }
    
    HEGGINDEXRANGE lp_ids_range = (HEGGINDEXRANGE)malloc(sizeof(EGGINDEXRANGE));
    UTIVECTOR* lp_ids_vector = Uti_vector_create(sizeof(EGGRANGEID));

    offset64_t n_iter_off = hEggIndexView->hInfo->leafOff;
    EGGRANGEID st_range_id = {0};
    HEGGINDEXRECORD lp_last_record = EGG_NULL;
    while(n_iter_off)
    {
        HEGGINDEXNODEVIEW lp_indexnode_view = eggIndexView_load_node(hEggIndexView, n_iter_off);
        
        index_t n_rd_idx = 0;
        
        while(n_rd_idx < EGGINDEXNODEVIEW_RDCNT(lp_indexnode_view))
        {
            HEGGINDEXRECORD lp_record = EGGINDEXNODEVIEW_RECORD_INDEX(lp_indexnode_view, n_rd_idx);
            
            if(eggIndexView_set_rangeId(hEggIndexView, &st_range_id, lp_record, lp_last_record) == EGG_TRUE)
            {
                Uti_vector_push(lp_ids_vector, &st_range_id, 1);
            }
            lp_last_record = lp_record;
            n_rd_idx++;
        }
        
        n_iter_off = eggIndexNodeView_get_nextoff(lp_indexnode_view);
        eggIndexNodeView_delete(lp_indexnode_view);
        
    }
    lp_ids_range->cnt = Uti_vector_count(lp_ids_vector);
    lp_ids_range->dids = Uti_vector_data(lp_ids_vector);
    
    Uti_vector_destroy(lp_ids_vector, EGG_FALSE);
    return lp_ids_range;
}


PUBLIC EBOOL eggIndexView_itercheck(HEGGINDEXVIEW hEggIndexView, offset64_t nIterOff, HEGGINDEXRECORD* lplp_key_rd)
{
  if(EGGINDEXVIEW_IS_INVALID(hEggIndexView))
    {
      return EGG_FALSE;
    }

  HEGGINDEXINFO lp_info = hEggIndexView->hInfo;
    
  HEGGINDEXNODEVIEW lp_iter_node = eggIndexNodeView_new(lp_info, 0);
  size16_t n_node_size = sizeof(EGGINDEXNODE) + lp_info->rdSize * (lp_info->rdCnt + 1);
    
  ViewStream_read(hEggIndexView->hViewStream, lp_iter_node->hNode, n_node_size, nIterOff);
    
  index_t n_rd_idx = 0;
  count_t n_rd_cnt = EGGINDEXNODEVIEW_RDCNT(lp_iter_node);
    
  while(n_rd_idx <= n_rd_cnt)
    {
      HEGGINDEXRECORD lp_iter_rd = EGGINDEXNODEVIEW_RECORD_INDEX(lp_iter_node, n_rd_idx);
      if(lp_iter_rd->childOff)
        {
//             printf("lp_iter_rd->childOff : %llu\n", lp_iter_rd->childOff);
            eggPrtLog_info("eggIndexView", "lp_iter_rd->childOff : %llu\n", lp_iter_rd->childOff);
             eggIndexView_itercheck(hEggIndexView, lp_iter_rd->childOff, lplp_key_rd);
        }
      if(n_rd_idx != n_rd_cnt)
        {
	  char* lp_buf = (char*)malloc(EGGINDEXRECORD_KSIZE(lp_iter_rd) + 1);
	  memcpy(lp_buf,  EGGINDEXRECORD_KEY(lp_iter_rd), EGGINDEXRECORD_KSIZE(lp_iter_rd));
	  lp_buf[EGGINDEXRECORD_KSIZE(lp_iter_rd)] = '\0';
	  //printf("key : [%s]\n", lp_buf);
      eggPrtLog_info("eggIndexView", "key : [%s]\n", lp_buf);


	  if(!*lplp_key_rd)
            {
	      *lplp_key_rd = (HEGGINDEXRECORD)malloc(lp_info->rdSize);
	      memcpy(*lplp_key_rd, lp_iter_rd, lp_info->rdSize);
            }
	  else
            {
	      RDCMP fnRdCmp = eggIndexNode_get_fncmp(lp_info->type);
            
	      int n_cmp_ret = fnRdCmp(lp_iter_rd, (HEGGINDEXRECORD)(*lplp_key_rd));
        
          if( n_cmp_ret == 0 && (lp_info->type & BTREE_RECORD_REPEAT))
          {
              n_cmp_ret = eggIndexRecord_cmp_with_id(lp_iter_rd, (HEGGINDEXRECORD)(*lplp_key_rd));
          }
            
	      if(n_cmp_ret < 0 )
                {
		  /* HEGGFILE lp_egg_id = EggFile_open("/home/bf/k2d/egg.idd"); */
		  /* HEGGIDVIEW lp_id_view = eggIdView_new(lp_egg_id); */

		  /* offset64_t n_nodelist_off = *(offset64_t*)EGGINDEXRECORD_VAL(lp_iter_rd); */
		  /* eggIdView_load(lp_id_view, n_nodelist_off); */
		  /* printf("n_nodelist_off %llu lp_id_view->hEggListView->hInfo->aSz %u\n", n_nodelist_off, lp_id_view->hEggListView->hInfo->aSz); */
//		  perror("btree error\n");
                    eggPrtLog_error("eggIndexView", "btree error\n");
		  getchar();
		  exit(-1);
                }
	      memcpy(*lplp_key_rd, lp_iter_rd, lp_info->rdSize);

            }

            
	  free(lp_buf);
        }
      n_rd_idx++;
    }
    
  eggIndexNodeView_delete(lp_iter_node);
    
  return EGG_TRUE;
}

PRIVATE HEGGINDEXRECORD eggIndexView_relocate(HEGGINDEXVIEW hEggIndexView, void* key, size32_t kSz, void* val, size32_t vSz, offset64_t* pNdPos, index_t* pRdIdx)
{
    HEGGINDEXINFO lp_info;
    if(hEggIndexView->hFieldView)
        eggFieldView_slock(hEggIndexView->hFieldView, hEggIndexView->hInfo->fid);
    
    lp_info = eggFieldView_get_indexinfo(hEggIndexView->hFieldView, hEggIndexView->hInfo->fid);
    eggIndexView_load_info(hEggIndexView, lp_info, hEggIndexView->hFieldView);
    
    if(hEggIndexView->hFieldView)
        eggFieldView_unlock(hEggIndexView->hFieldView, lp_info->fid);
    
    return eggIndexView_locate(hEggIndexView, key, kSz, val, vSz, pNdPos, pRdIdx);
}
PRIVATE offset64_t eggIndexView_create_rootNode(HEGGINDEXVIEW hEggIndexView)
{
    HEGGINDEXINFO lp_info = hEggIndexView->hInfo;
    HVIEWSTREAM lp_view_stream = hEggIndexView->hViewStream;
    
    size16_t n_node_size = sizeof(EGGINDEXNODE) + lp_info->rdSize * (lp_info->rdCnt + 1);
    
    HEGGINDEXNODEVIEW lp_nodeview_root = eggIndexNodeView_new((HEGGINDEXNODEINFO)lp_info, IDXNODE_IS_LEAF);
    
    offset64_t n_root_off = ViewStream_write(lp_view_stream, lp_nodeview_root->hNode, n_node_size);

    eggIndexNodeView_delete(lp_nodeview_root);

    return n_root_off;
}


PRIVATE EBOOL eggIndexView_update_childNode_info(HEGGINDEXVIEW hEggIndexView, HEGGINDEXNODEVIEW hNodeView)
{
    if(EGGINDEXVIEW_IS_INVALID(hEggIndexView))
    {
        return EGG_FALSE;
    }
    
    if(EGGINDEXNODEVIEW_IS_INVALID(hNodeView))
    {
        return EGG_FALSE;
    }

    HEGGINDEXINFO lp_info = hEggIndexView->hInfo;
    HVIEWSTREAM lp_view_stream = hEggIndexView->hViewStream;
    
    char* lp_child_records = (char*)(hNodeView->hNode + 1);
    count_t n_record_count = hNodeView->hNode->rdCnt;
    EGGINDEXNODE st_node_info;
    
    do
    {        
        HEGGINDEXRECORD lp_record = (HEGGINDEXRECORD)(lp_child_records + lp_info->rdSize * n_record_count);
        
        ViewStream_read(lp_view_stream, &st_node_info, sizeof(st_node_info), lp_record->childOff);
        st_node_info.parent = EGGINDEXNODEVIEW_OWNEROFF(hNodeView);
        ViewStream_update(lp_view_stream, &st_node_info, sizeof(st_node_info), lp_record->childOff);
        
    }while(n_record_count--);

    return EGG_TRUE;
}

PRIVATE offset64_t eggIndexView_split(HEGGINDEXVIEW hEggIndexView, HEGGINDEXNODEVIEW hNodeViewSrc)
{
    if(EGGINDEXVIEW_IS_INVALID(hEggIndexView))
    {
        return EGG_FALSE;
    }
    
    //EGGINDEXNODEVIEW_OWNEROFF(hNodeViewIter)
    offset64_t n_locks_node[128] = {0};
    count_t n_locks_cnt = 0;
    HEGGINDEXINFO lp_info = hEggIndexView->hInfo;
    HVIEWSTREAM lp_view_stream = hEggIndexView->hViewStream;
    
    size16_t n_node_size = sizeof(EGGINDEXNODE) + lp_info->rdSize * (lp_info->rdCnt + 1);
    HEGGINDEXRECORD lp_record_ist = (HEGGINDEXRECORD)malloc(lp_info->rdSize);
    
    index_t n_idx_ist = 0;
    offset64_t n_root_off = 0;
    
    HEGGINDEXNODEVIEW lp_nodeview_iter = hNodeViewSrc;
    HEGGINDEXNODEVIEW lp_nodeview_parent = eggIndexNodeView_new( (HEGGINDEXNODEINFO)(hEggIndexView->hInfo), IDXNODE_IS_NORMAL);
    HEGGINDEXNODEVIEW lp_nodeview_new = eggIndexNodeView_new((HEGGINDEXNODEINFO)(hEggIndexView->hInfo), hNodeViewSrc->hNode->ntype);
    
    while(EGGINDEXNODEVIEW_RDCNT(lp_nodeview_iter) == lp_info->rdCnt)
    {
        //CREATE PARENT NODE
        if(EGGINDEXNODEVIEW_PAREOFF(lp_nodeview_iter) == EGG_NULL)
        {
            HEGGINDEXNODEVIEW lp_nodeview_tmp = eggIndexNodeView_new((HEGGINDEXNODEINFO)(hEggIndexView->hInfo) , IDXNODE_IS_ROOT);
            
            EGGINDEXNODEVIEW_PAREOFF(lp_nodeview_iter) = ViewStream_write(lp_view_stream,
                                                                      lp_nodeview_tmp->hNode, n_node_size);
            
            n_root_off = EGGINDEXNODEVIEW_PAREOFF(lp_nodeview_iter);
            
            eggIndexNodeView_delete(lp_nodeview_tmp);
        }

        //NEW NODE 
        eggIndexNodeView_split(lp_nodeview_iter, lp_nodeview_new);
        memcpy(lp_record_ist, EGGINDEXNODEVIEW_RECORD_INDEX(lp_nodeview_new, 0), lp_info->rdSize);
        
        EGGINDEXNODEVIEW_OWNEROFF(lp_nodeview_new) = ViewStream_write(lp_view_stream,
                                                                      lp_nodeview_new->hNode, n_node_size);
        
        if(lp_nodeview_new->hNode->ntype != IDXNODE_IS_LEAF)
        {
            eggIndexNodeView_remove(lp_nodeview_new, 0);
            
            // UPDATE CHILD NODE INFO (NOT LEAF)
            eggIndexView_update_childNode_info(hEggIndexView, lp_nodeview_new);
        }
        else
        {
            HEGGINDEXLEAFLST lp_left_lst = (HEGGINDEXLEAFLST)(EGGINDEXNODEVIEW_RECORD_INDEX(lp_nodeview_iter, lp_info->rdCnt) + 1);
            HEGGINDEXLEAFLST lp_right_lst = (HEGGINDEXLEAFLST)(EGGINDEXNODEVIEW_RECORD_INDEX(lp_nodeview_new, lp_info->rdCnt) + 1);
            
            lp_right_lst->next = lp_left_lst->next;
            lp_left_lst->next = EGGINDEXNODEVIEW_OWNEROFF(lp_nodeview_new);
        }
        
        // PARENT NODE    
        
        ViewStream_read(lp_view_stream, lp_nodeview_parent->hNode, n_node_size, EGGINDEXNODEVIEW_PAREOFF(lp_nodeview_iter) );
        lp_nodeview_parent->nodeOff = EGGINDEXNODEVIEW_PAREOFF(lp_nodeview_iter);
        
        eggIndexNodeView_locate(lp_nodeview_parent, lp_record_ist, &n_idx_ist);
        eggIndexNodeView_insert(lp_nodeview_parent, lp_record_ist, n_idx_ist);
        
        // INSERT RECORD
        HEGGINDEXRECORD lp_record_pare = (HEGGINDEXRECORD)EGGINDEXNODEVIEW_RECORD_INDEX(lp_nodeview_parent, n_idx_ist);
        
        lp_record_pare->childOff = EGGINDEXNODEVIEW_OWNEROFF(lp_nodeview_iter);
        if(lp_nodeview_new->hNode->ntype == IDXNODE_IS_LEAF)
        {
            lp_record_pare->hostOff = EGGINDEXNODEVIEW_OWNEROFF(lp_nodeview_new);
        }
        
        lp_record_pare = EGGINDEXNODEVIEW_RECORD_INDEX(lp_nodeview_parent, n_idx_ist + 1);
        lp_record_pare->childOff = EGGINDEXNODEVIEW_OWNEROFF(lp_nodeview_new);
        
        //UPDATE NODE TO FILE
        lp_nodeview_iter->hNode->flag |= IDXNODE_IS_LOCKED;
        lp_nodeview_parent->hNode->flag |= IDXNODE_IS_LOCKED;
        n_locks_node[n_locks_cnt++] = EGGINDEXNODEVIEW_OWNEROFF(lp_nodeview_iter);
        n_locks_node[n_locks_cnt++] = EGGINDEXNODEVIEW_OWNEROFF(lp_nodeview_parent);
        ///////////
        ViewStream_update(lp_view_stream, lp_nodeview_iter->hNode, n_node_size, EGGINDEXNODEVIEW_OWNEROFF(lp_nodeview_iter));
        ViewStream_update(lp_view_stream, lp_nodeview_parent->hNode, n_node_size, EGGINDEXNODEVIEW_OWNEROFF(lp_nodeview_parent));
//        printf("----spliit --------\n");
//        sleep(1);
        ViewStream_update(lp_view_stream, lp_nodeview_new->hNode, n_node_size, EGGINDEXNODEVIEW_OWNEROFF(lp_nodeview_new));
        
        //RECURSIVE NODE
        memcpy(EGGINDEXNODEVIEW_BLOCK(lp_nodeview_iter),
               EGGINDEXNODEVIEW_BLOCK(lp_nodeview_parent), n_node_size);
        lp_nodeview_iter->nodeOff = lp_nodeview_parent->nodeOff;
    }
    
    eggIndexNodeView_delete(lp_nodeview_parent);
    eggIndexNodeView_delete(lp_nodeview_new);
    free(lp_record_ist);
    /////unlock node//////
    while(n_locks_cnt--)
    {
        size32_t flag = 0;
        ViewStream_update(lp_view_stream, &flag, sizeof(size32_t),
                          n_locks_node[n_locks_cnt] + (offset64_t)STRUCT_POS(EGGINDEXNODE, flag));
    }
    return n_root_off;
}

EBOOL eggIndexView_clean_actinfo(HEGGINDEXVIEW hEggIndexView, ActInfo *hActInfo)
{
    if(EGGINDEXVIEW_IS_INVALID(hEggIndexView))
    {
        return EGG_FALSE;
    }

    EBOOL retv;
    retv = ViewStream_clean_actinfo(hEggIndexView->hViewStream, hActInfo);
    return retv;
}
EBOOL eggIndexView_set_actinfo(HEGGINDEXVIEW hEggIndexView, ActInfo *hActInfo)
{
    if(EGGINDEXVIEW_IS_INVALID(hEggIndexView))
    {
        return EGG_FALSE;
    }

    EBOOL retv;
    retv = ViewStream_set_actinfo(hEggIndexView->hViewStream, hActInfo);
    return retv;
}
EBOOL eggIndexView_set_rangeId(HEGGINDEXVIEW hEggIndexView, HEGGRANGEID hRangeId, HEGGINDEXRECORD hRecord, HEGGINDEXRECORD hLastRecord)
{
    HEGGINDEXINFO lp_info = hEggIndexView->hInfo;
    
    HEGGRECORDDOCID lp_record_id = (HEGGRECORDDOCID)EGGINDEXRECORD_VAL(hRecord);
    if (lp_record_id->flag != RECORDDOCID_VALID)
    {
        return EGG_FALSE;
    }

    hRangeId->did = lp_record_id->did;
    char* lp_key_rd = EGGINDEXRECORD_KEY(hRecord);
    size32_t n_key_rd = EGGINDEXRECORD_KSIZE(hRecord);
    if(lp_info->type & EGG_INDEX_STRING)
    {
        if(hLastRecord)
        {
            if(EGGINDEXRECORD_KSIZE(hRecord) !=
               EGGINDEXRECORD_KSIZE(hLastRecord) ||
               memcmp(hRecord, hLastRecord, n_key_rd) == 0)
            {
                (*(int32_t*)(hRangeId->val))++;
            }
        }
        else
        {
            *(int32_t*)(hRangeId->val) = 0;
        }
    }
    else if(lp_info->type & EGG_INDEX_INT32)
        *(int32_t*)(hRangeId->val) = *(int32_t*)(lp_key_rd);
    else if(lp_info->type & EGG_INDEX_INT64)
        *(int64_t*)(hRangeId->val) = *(int64_t*)(lp_key_rd);
    else if(lp_info->type & EGG_INDEX_DOUBLE)
        *(double*)(hRangeId->val) = *(double*)(lp_key_rd);
    
    return EGG_TRUE;

}
