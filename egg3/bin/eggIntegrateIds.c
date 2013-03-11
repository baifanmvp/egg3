#include <stdio.h>

#include <egg3/index/eggIdView.h>
#include <egg3/index/eggIndexView.h>
#include <egg3/index/eggFieldView.h>
#include <egg3/Egg3.h>

int main(int argc, char* argv[])
{
    if(argc < 2)
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
    char idxPath[1024];
    count_t n_field_cnt = argc - 2;
    
    sprintf(fddPath, "%s/egg.fdd", argv[1]);
    sprintf(iddPath, "%s/egg.idd", argv[1]);
    sprintf(idxPath, "%s/egg.idx", argv[1]);
    
    HEGGFILE lp_egg_field = EggFile_open(fddPath);
    HEGGFILE lp_egg_id = EggFile_open(iddPath);
    HEGGFILE lp_egg_idx = EggFile_open(idxPath);
    HEGGFIELDVIEW lp_field_view = eggFieldView_new(lp_egg_field);
    HEGGINDEXVIEW lp_index_view = eggIndexView_new(lp_egg_idx, NULL);
    HEGGIDVIEW lp_id_view = eggIdView_new(lp_egg_id);
    
    fdid_t fdid = 0;
    struct eggIndexInfo *lp_index_info;
    int n_field = 0;
    count_t aNodeCnt = 0;
    
    count_t useNodeCnt = 0;
    count_t bigBlkCnt = 0;
    count_t aBlkCnt = 0;
    count_t oneNodeCnt = 0;
    
    while ((lp_index_info = eggFieldView_iter(lp_field_view, &fdid)))
    {
        index_t n_field_idx = 0;
        while(n_field_idx != n_field_cnt)
        {
            fdid_t fdidtmp = 0;
            eggFieldView_find(lp_field_view, argv[n_field_idx+2], &fdidtmp);
            
            if(fdidtmp == lp_index_info->fid)break;
            
            n_field_idx++;
        }
        if(!(n_field_idx == 0 || n_field_idx != n_field_cnt))
        {
            continue;
        }
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

                    HEGGIDNODE hIdNodes;
                    count_t cnt = 0;
                    char key[1000]= {0};
                    memcpy(key, EGGINDEXRECORD_KEY(pRecord), EGGINDEXRECORD_KSIZE(pRecord));
		    //                    printf("[%lld] [%llu] [%s] \n", fdid, *(offset64_t*)EGGINDEXRECORD_VAL(pRecord), key);
                    printf("[%s] \n", key);
                    eggIdView_load(lp_id_view, *(offset64_t*)EGGINDEXRECORD_VAL(pRecord));
                    printf("%d\n", lp_id_view->hEggListView->hInfo->nodeCnt);
                    
//                    eggIdView_find(lp_id_view, *(offset64_t*)EGGINDEXRECORD_VAL(pRecord), &hIdNodes, &cnt);
                    //                  printf("idnum : %d\n", cnt);
                    
                    free(hIdNodes);
                }
//                eggIndexRecord_delete(pRecord);
                cnt++;
                n_index_iter++;
                
            }
            n_iter_off = eggIndexNodeView_get_nextoff(lp_node_view);
            eggIndexNodeView_delete(lp_node_view);
        }
        printf("field %d[%d]\n", n_field, cnt);
        printf("------------------------------------\n");
        n_field++;
    }
    printf("oneNodeCnt : %d\n", oneNodeCnt);
    printf("useNodeCnt : %d\n", useNodeCnt);
    printf("aNodeCnt : %d\n", aNodeCnt);
    printf("bigBlkCnt : %d\n", bigBlkCnt);
    printf("aBlkCnt : %d\n", aBlkCnt);
    return 0;
}
