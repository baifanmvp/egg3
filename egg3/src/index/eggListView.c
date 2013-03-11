#include <math.h>
#include <assert.h>
#include "eggListView.h"
#include "../log/eggPrtLog.h"

#define PRINT_LISTINF(off,p) \
    printf("[%s]off[%llu] aSz[%hu]nodeSz[%hu]nodeCnt[%u]"               \
           "blkCnt[%u]curCnt[%u]tmpOff[%u]headOff[%llu]ownOff[%llu]\n", \
           __func__, off, (p)->aSz, (p)->nodeSz,                        \
           (p)->nodeCnt, (p)->blkCnt, (p)->curCnt,                      \
           (p)->tmpOff, (p)->headOff, (p)->ownOff); fflush(NULL)

HEGGLISTVIEW eggListView_new(HVIEWSTREAM hViewStream)
{
    HEGGLISTVIEW lp_list_view = (HEGGLISTVIEW)malloc(sizeof(EGGLISTVIEW));
    lp_list_view->hViewStream = hViewStream;
    lp_list_view->hInfo = EGG_NULL;
    return lp_list_view;
}

EBOOL eggListView_delete(HEGGLISTVIEW hEggListView)
{
    if(EGGLISTVIEW_IS_INVALID(hEggListView))
    {
        return EGG_FALSE;
    }
    if(hEggListView->hInfo)
    {
        free(hEggListView->hInfo);
    }
    free(hEggListView);
    return EGG_TRUE;
}

EBOOL eggListView_load_info(HEGGLISTVIEW hEggListView, offset64_t infoOff)
{
    if(EGGLISTVIEW_IS_INVALID(hEggListView))
    {
        return EGG_FALSE;
    }
    
    
    EGGLISTINF st_list_info = {0};
    HVIEWSTREAM lp_view_stream = hEggListView->hViewStream;
    
    ViewStream_read_nolock(lp_view_stream, &st_list_info, sizeof(EGGLISTINF), infoOff);
    if(infoOff != st_list_info.ownOff)
    {
        
//        PRINT_LISTINF(infoOff, &st_list_info);
        
        assert(0);
    }
    
    if(st_list_info.aSz != sizeof(EGGLISTINF))
    {
        hEggListView->hInfo = realloc(hEggListView->hInfo, st_list_info.aSz);
        ViewStream_read_nolock(lp_view_stream, hEggListView->hInfo, st_list_info.aSz, infoOff);
    }
    else
    {
        hEggListView->hInfo = realloc(hEggListView->hInfo, st_list_info.aSz);
        
        memcpy(hEggListView->hInfo, &st_list_info, st_list_info.aSz);
    }
    return EGG_TRUE;
}




EBOOL eggListView_update_info(HEGGLISTVIEW hEggListView)
    
{
    if(EGGLISTVIEW_IS_INVALID(hEggListView))
    {
        return EGG_FALSE;
    }
    
    ViewStream_update_nolock(hEggListView->hViewStream,
                             hEggListView->hInfo, hEggListView->hInfo->aSz,
                             hEggListView->hInfo->ownOff);
    return EGG_TRUE;
}

offset64_t eggListView_reg_info(HEGGLISTVIEW hEggListView, HEGGLISTINF hInfo)
{
    if(EGGLISTVIEW_IS_INVALID(hEggListView))
    {
        return EGG_FALSE;
    }
    hEggListView->hInfo = realloc(hEggListView->hInfo, hInfo->aSz);
    
    memcpy(hEggListView->hInfo, hInfo, hInfo->aSz);
    
    hEggListView->hInfo->ownOff = ViewStream_write(hEggListView->hViewStream,
                                                   hInfo, hInfo->aSz);
    
    ViewStream_update_nolock(hEggListView->hViewStream,
                             &hEggListView->hInfo->ownOff, sizeof(hEggListView->hInfo->ownOff),
                             hEggListView->hInfo->ownOff + struct_offset(EGGLISTINF, ownOff));
    
    return EGG_TRUE;
}

EBOOL eggListView_insert(HEGGLISTVIEW hEggListView, epointer pNodes, size32_t ndSz)
{
    if(EGGLISTVIEW_IS_INVALID(hEggListView))
    {
        return EGG_FALSE;
    }
    HVIEWSTREAM lp_view_stream = hEggListView->hViewStream;
    HEGGLISTINF lp_list_info = hEggListView->hInfo;
    offset64_t n_block_off = lp_list_info->headOff;
    
    EGGLISTBLOCK st_list_block = {0};
    char* lp_cur_node = pNodes;
    size32_t n_cur_sz = ndSz;
    
    
    if(n_block_off)
    {
        
        ViewStream_read(lp_view_stream, &st_list_block, sizeof(st_list_block), n_block_off);

//        printf("[%u][%u][%u] \n", st_list_block.aCnt, st_list_block.uCnt, st_list_block.next );

        size32_t n_unused_sz = (st_list_block.aCnt - st_list_block.uCnt) * lp_list_info->nodeSz;
        offset64_t n_tmp_off =  n_block_off + sizeof(st_list_block) + st_list_block.uCnt * lp_list_info->nodeSz;
            
        if(n_unused_sz >= n_cur_sz)
        {
            ViewStream_update(lp_view_stream, lp_cur_node, n_cur_sz, n_tmp_off);
            st_list_block.uCnt += n_cur_sz / lp_list_info->nodeSz;
            n_cur_sz = 0;
            ViewStream_update(lp_view_stream, &st_list_block, sizeof(st_list_block), n_block_off);
        }
        else
        {
            
            count_t n_real_cnt = st_list_block.uCnt + n_cur_sz / lp_list_info->nodeSz;
            count_t n_pow2_cnt = 0;
            
            count_t n_tmp_cnt = n_real_cnt;
            POWER_OF_TWO(n_pow2_cnt, n_tmp_cnt);
            
            
            if(n_pow2_cnt < EGGBLOCK_LIMIT_COUNT)
            {
                
                size32_t n_total_sz = n_pow2_cnt * lp_list_info->nodeSz + sizeof(st_list_block);
                size32_t n_read_sz = st_list_block.uCnt * lp_list_info->nodeSz;
                char* lp_total_buf = (char*)malloc(n_total_sz);
                memset(lp_total_buf, 0, n_total_sz);

                //read old nodes
                ViewStream_read(lp_view_stream, lp_total_buf + sizeof(st_list_block),
                                n_read_sz, n_block_off + sizeof(EGGLISTBLOCK));

                //copy new nodes
                memcpy(lp_total_buf + sizeof(st_list_block) + n_read_sz, pNodes, ndSz);

                //free old block
                ViewStream_free_area(lp_view_stream, n_block_off,
                                    sizeof(EGGLISTBLOCK) + st_list_block.aCnt * lp_list_info->nodeSz);

                //update  block info
                st_list_block.aCnt = n_pow2_cnt;
                st_list_block.uCnt = n_real_cnt;

                memcpy((lp_total_buf), &st_list_block, sizeof(st_list_block));
                
                lp_list_info->headOff = ViewStream_write(lp_view_stream, lp_total_buf, n_total_sz);
                free(lp_total_buf);
                n_cur_sz = 0;
            }
            else
            {
                ViewStream_update(lp_view_stream, lp_cur_node, n_unused_sz, n_tmp_off);
                st_list_block.uCnt = st_list_block.aCnt;
                lp_cur_node += n_unused_sz;
                n_cur_sz -= n_unused_sz;
                ViewStream_update(lp_view_stream, &st_list_block, sizeof(st_list_block), n_block_off);

            }
        }
        
     
    }


    while(n_cur_sz)
    {
        
        
        count_t n_real_cnt = n_cur_sz / lp_list_info->nodeSz;
        count_t n_total_cnt = 0;
        size32_t n_write_cnt = 0;
        
        size32_t n_write_sz = 0;
        size32_t n_total_sz = 0;

        char* lp_total_buf = EGG_NULL;

        count_t n_tmp_cnt = n_real_cnt;
        POWER_OF_TWO(n_total_cnt, n_tmp_cnt);
        n_total_cnt = (n_total_cnt < EGGBLOCK_LIMIT_COUNT ? n_total_cnt : EGGBLOCK_LIMIT_COUNT) ;

        n_total_sz = sizeof(st_list_block) + n_total_cnt * lp_list_info->nodeSz;

        n_write_cnt = n_total_cnt >= n_real_cnt ?n_real_cnt : n_total_cnt;
        n_write_sz = n_write_cnt * lp_list_info->nodeSz;
        
        st_list_block.aCnt = n_total_cnt;
        st_list_block.uCnt = n_write_cnt;
        st_list_block.next = lp_list_info->headOff;

        lp_total_buf = (char*)malloc(n_total_sz);
        memset(lp_total_buf, 0, n_total_sz);

        memcpy(lp_total_buf, &st_list_block, sizeof(st_list_block));
        memcpy((HEGGLISTBLOCK)(lp_total_buf) + 1, lp_cur_node, n_write_sz);
        
        lp_list_info->headOff = ViewStream_write(lp_view_stream, lp_total_buf, n_total_sz);
        lp_list_info->blkCnt ++;
        
        
        lp_cur_node += n_write_sz;
        n_cur_sz -= n_write_sz;
        free(lp_total_buf);
    }
    lp_list_info->nodeCnt += ndSz / lp_list_info->nodeSz;
    lp_list_info->curCnt += ndSz / lp_list_info->nodeSz;

    ViewStream_update_nolock(lp_view_stream, lp_list_info, sizeof(EGGLISTINF), hEggListView->hInfo->ownOff);



    
    return EGG_TRUE;
}


EBOOL eggListView_rewrite(HEGGLISTVIEW hEggListView, epointer pNodes, size32_t ndSz)
{
    if(EGGLISTVIEW_IS_INVALID(hEggListView))
    {
        return EGG_FALSE;
    }
    

    HVIEWSTREAM lp_view_stream = hEggListView->hViewStream;
    HEGGLISTINF lp_list_info = hEggListView->hInfo;
    offset64_t n_block_off = lp_list_info->headOff;
    
    offset64_t iter_block_off = lp_list_info->headOff;
    
    EGGLISTBLOCK st_list_block = {0};
    char* lp_cur_node = pNodes;
    size32_t n_cur_sz = ndSz;
    int n_ndSz_tmp = 0;
    
    if( (n_ndSz_tmp = lp_list_info->nodeCnt * lp_list_info->nodeSz - ndSz) < 0)
    {
        return EGG_FALSE;
    }
    
    st_list_block.next = lp_list_info->headOff;
    
    if(!n_cur_sz)
    {
        lp_list_info->headOff = 0;
    }
    else
    {
        while(n_cur_sz)
        {
            n_block_off = st_list_block.next;
            ViewStream_read(lp_view_stream, &st_list_block, sizeof(st_list_block), n_block_off);
        
            if(n_cur_sz < st_list_block.uCnt * lp_list_info->nodeSz)
            {

                st_list_block.uCnt = n_cur_sz / lp_list_info->nodeSz;
            
                ViewStream_update(lp_view_stream, &st_list_block, sizeof(st_list_block), n_block_off);
            
            }
        
            ViewStream_update(lp_view_stream, lp_cur_node,
                              st_list_block.uCnt * lp_list_info->nodeSz,
                              sizeof(st_list_block) + n_block_off);
            
            lp_cur_node += st_list_block.uCnt * lp_list_info->nodeSz;
            n_cur_sz -= st_list_block.uCnt * lp_list_info->nodeSz;
        }
        
        /*
          截断后面的链表
         */
        iter_block_off = st_list_block.next;
        st_list_block.next = 0;
        ViewStream_update(lp_view_stream, &st_list_block, sizeof(st_list_block), n_block_off);
        
    }

    lp_list_info->nodeCnt = ndSz / lp_list_info->nodeSz;
    lp_list_info->curCnt = 0;
    ViewStream_update_nolock(lp_view_stream, lp_list_info, sizeof(EGGLISTINF), hEggListView->hInfo->ownOff);

//    PRINT_LISTINF(hEggListView->hInfo->ownOff, hEggListView->hInfo);
    
    
    /*
      回收后面没用到的块
     */
    
    while(iter_block_off)
    {
        ViewStream_read(lp_view_stream, &st_list_block, sizeof(st_list_block), iter_block_off);
//        printf("listview ViewStream_free_area\n");
        eggPrtLog_info("eggListView", "listview ViewStream_free_area\n");
        ViewStream_free_area(lp_view_stream, iter_block_off,
                            sizeof(EGGLISTBLOCK) + st_list_block.aCnt * lp_list_info->nodeSz);
        
        iter_block_off = st_list_block.next;
    }
    
    return EGG_TRUE;
}



EBOOL eggListView_fetch(HEGGLISTVIEW hEggListView, epointer* ppNodes, size32_t* pNdSz)
{
    if(EGGLISTVIEW_IS_INVALID(hEggListView))
    {
        return EGG_FALSE;
    }
    HEGGLISTINF lp_list_info = hEggListView->hInfo;
    
    if(*pNdSz)
    {
        *pNdSz = *pNdSz < lp_list_info->nodeCnt * lp_list_info->nodeSz ? *pNdSz : lp_list_info->nodeCnt * lp_list_info->nodeSz;
    }
    else
    {
        *pNdSz = lp_list_info->nodeCnt * lp_list_info->nodeSz;
    }

    char* lp_str_buf = (char*)malloc(*pNdSz);
    *ppNodes = lp_str_buf;
    
    size32_t n_str_sz = (size32_t)*pNdSz;
    offset64_t n_iter_off = lp_list_info->headOff;
    EGGLISTBLOCK st_list_block = {0};
    HVIEWSTREAM lp_view_stream = hEggListView->hViewStream;
    
    while(n_str_sz)
    {
        ViewStream_read(lp_view_stream, &st_list_block, sizeof(st_list_block), n_iter_off);
        
        int n_len_read = (int)(n_str_sz - st_list_block.uCnt * lp_list_info->nodeSz);    
        if(n_len_read < 0)
        {
            n_len_read = n_str_sz;
            n_str_sz = 0;
        }
        else
        {
            n_len_read = st_list_block.uCnt * lp_list_info->nodeSz;
            n_str_sz -= n_len_read;
        }
        
        ViewStream_read(lp_view_stream, lp_str_buf, n_len_read, n_iter_off + sizeof(EGGLISTBLOCK));
        lp_str_buf += n_len_read;
        n_iter_off = st_list_block.next;
    }

    return EGG_TRUE;
}
