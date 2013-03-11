#include <stdio.h>
#include ".././src/index/eggIdView.h"
#include ".././src/index/eggIndexView.h"
#include ".././src/index/eggFieldView.h"
#include "egg3/Egg3.h"

int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        printf("argc error!\n");
        exit(-1);
    }
    if(access(argv[1], F_OK))
    {
        printf("path is error!\n");
        exit(-1);
    }
    char fddPath[1024];
    char iddPath[1024];
    char iddBakPath[1024];
    char idxPath[1024];
    
    sprintf(fddPath, "%s/egg.fdd", argv[1]);
    sprintf(iddPath, "%s/egg.idd", argv[1]);
    sprintf(iddBakPath, "%s/eggbak.idd", argv[1]);
    sprintf(idxPath, "%s/egg.idx", argv[1]);
    
    HEGGFILE lp_egg_field = EggFile_open(fddPath);
    HEGGFILE lp_egg_id = EggFile_open(iddPath);
    HEGGFILE lp_egg_id_bak = EggFile_open(iddBakPath);
    HEGGFILE lp_egg_idx = EggFile_open(idxPath);
    
    HEGGFIELDVIEW lp_field_view = eggFieldView_new(lp_egg_field);
    HEGGINDEXVIEW lp_index_view = eggIndexView_new(lp_egg_idx, NULL);
    HEGGIDVIEW lp_id_view = eggIdView_new(lp_egg_id);
    HEGGIDVIEW lp_id_bak_view = eggIdView_new(lp_egg_id_bak);
    
    fdid_t fdid = 0;
    struct eggIndexInfo *lp_index_info;
    int n_field = 0;

    while ((lp_index_info = eggFieldView_iter(lp_field_view, &fdid)))
    {
        eggIndexView_load_info(lp_index_view, lp_index_info, lp_field_view);
        
        int cnt = 0;
        offset64_t n_iter_off = EGGINDEXVIEW_LEAFOFF(lp_index_view);
        while (n_iter_off)
        {
            HEGGINDEXNODEVIEW lp_node_view = eggIndexView_load_node(lp_index_view, n_iter_off);
            index_t n_index_iter = 0;
            while(n_index_iter != EGGINDEXNODEVIEW_RDCNT(lp_node_view))
            {
                HEGGINDEXRECORD pRecord = EGGINDEXNODEVIEW_RECORD_INDEX(lp_node_view, n_index_iter);
                {
                    struct timeval tv1, tv2;
                    gettimeofday(&tv1, 0);

                    offset64_t n_nodelist_off = 0;
                    n_nodelist_off = *(offset64_t*)EGGINDEXRECORD_VAL(pRecord);
                    eggIdView_load(lp_id_view, n_nodelist_off);
                    
                    printf("KEY[%.*s]\n",
                           EGGINDEXRECORD_KSIZE(pRecord),
                           EGGINDEXRECORD_KEY(pRecord));
                    
/*                    printf("aSz = %hu, nodeCnt = %u, blkCnt = %u\n",
                           lp_id_view->hEggListView->hInfo->aSz,
                           lp_id_view->hEggListView->hInfo->nodeCnt,
                           lp_id_view->hEggListView->hInfo->blkCnt);
                    
                    HEGGIDNODE hEggIdNodes = EGG_NULL;
                    count_t n_id_cnt = 0;
                    
                    eggIdView_find(lp_id_view, n_nodelist_off, &hEggIdNodes, &n_id_cnt);
                    
                    HEGGLISTINF lp_list_info = (HEGGLISTINF)malloc(lp_id_view->hEggListView->hInfo->aSz);
                    
                    if(lp_id_view->hEggListView->hInfo->aSz - sizeof(EGGLISTINF))
                    {
                        memcpy(lp_list_info+1, lp_id_view->hEggListView->hInfo+1,
                               lp_id_view->hEggListView->hInfo->aSz - sizeof(EGGLISTINF));
                    }
                    
                    memset(lp_list_info, 0, sizeof(EGGLISTINF));
                    
                    lp_list_info->aSz = lp_id_view->hEggListView->hInfo->aSz;
                    lp_list_info->nodeSz = lp_id_view->hEggListView->hInfo->nodeSz;
                    
                    eggIdView_reg(lp_id_bak_view, lp_list_info);
                    eggIdView_add(lp_id_bak_view, hEggIdNodes, n_id_cnt);

                    *(offset64_t*)EGGINDEXRECORD_VAL(pRecord) = lp_id_bak_view->hEggListView->hInfo->ownOff;
                    
                    free(hEggIdNodes);
                    free(lp_list_info);
                    
                    gettimeofday(&tv2, 0);
                    printf("every key time: [%f]\n", ((double)tv2.tv_sec - (double)tv1.tv_sec) +  ((double)tv2.tv_usec - (double)tv1.tv_usec)/1000000);
*/  
                }
//                eggIndexRecord_delete(pRecord);
                cnt++;
                n_index_iter++;
            }
            size16_t n_node_size = sizeof(EGGINDEXNODE) + lp_index_view->hInfo->rdSize * (lp_index_view->hInfo->rdCnt + 1);

            ViewStream_update(lp_index_view->hViewStream, lp_node_view->hNode, n_node_size, n_iter_off);
            n_iter_off = eggIndexNodeView_get_nextoff(lp_node_view);
            eggIndexNodeView_delete(lp_node_view);
        }
        printf("field %d[%d]\n", n_field, cnt);
        printf("------------------------------------\n");
        n_field++;
    }
    return 0;
}
