#include <stdio.h>
#include ".././src/index/eggIdView.h"
#include ".././src/index/eggIndexView.h"
#include ".././src/index/eggFieldView.h"
#include "egg3/Egg3.h"
typedef  struct eggIdNode2 EGGIDNODE2;
typedef  struct eggIdNode2* HEGGIDNODE2;
#pragma pack(push)
#pragma pack(4)

struct eggIdNode2
{
    did_t id;
    int weight;
    size16_t keyCnt;
    size16_t mask; //field mask
    char pos[EGG_POS_SPACE];
};
#pragma pack (pop)
int eggIdNode2_cmpWeight(HEGGIDNODE2 hSrcIdNode, HEGGIDNODE2 hDestIdNode)
{
    if(hSrcIdNode->weight > hDestIdNode->weight)
    {
        return 1;
    }
    else if(hSrcIdNode->weight < hDestIdNode->weight)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}

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
    char idxBakPath[1024];
    char idxPath[1024];
    HEGGDIRECTORY lp_dir =  eggDirectory_open(argv[1]);
    HEGGINDEXREADER hEggIndexReader = eggIndexReader_open(lp_dir);
    
    sprintf(fddPath, "%s/egg.fdd", argv[1]);
    sprintf(iddPath, "%s/egg.idd", argv[1]);
    sprintf(idxBakPath, "%s/eggbak.idx", argv[1]);
    sprintf(idxPath, "%s/egg.idx", argv[1]);
    
    HEGGFILE lp_egg_field = EggFile_open(fddPath);
    HEGGFILE lp_egg_id = EggFile_open(iddPath);
    HEGGFILE lp_egg_idx_bak = EggFile_open(idxBakPath);
    HEGGFILE lp_egg_idx = EggFile_open(idxPath);
    
    HEGGFIELDVIEW lp_field_view = eggFieldView_new(lp_egg_field);
    HEGGINDEXVIEW lp_index_view = eggIndexView_new(lp_egg_idx, NULL);
    HEGGIDVIEW lp_id_view = eggIdView_new(lp_egg_id);
    HEGGINDEXVIEW lp_index_bak_view = eggIndexView_new(lp_egg_idx_bak, NULL);

    fdid_t fdid = 0;
    struct eggIndexInfo *lp_index_info;
        
    int n_field = 0;
    int n_key_cnt = 0;
    while ((lp_index_info = eggFieldView_iter(lp_field_view, &fdid)))
    {
        eggIndexView_load_info(lp_index_view, lp_index_info, lp_field_view);
        
        struct eggIndexInfo *lp_index_info_bak = (struct eggIndexInfo*)malloc(sizeof(struct eggIndexInfo));
        memcpy(lp_index_info_bak, lp_index_info, sizeof(struct eggIndexInfo));
        
        lp_index_info_bak->rootOff = 0;
        lp_index_info_bak->leafOff = 0;

        eggIndexView_load_info(lp_index_bak_view, lp_index_info_bak, EGG_NULL);
        
        int cnt = 0;
        offset64_t n_iter_off = EGGINDEXVIEW_LEAFOFF(lp_index_view);
        while (n_iter_off)
        {
            HEGGINDEXNODEVIEW lp_node_view = eggIndexView_load_node(lp_index_view, n_iter_off);
            index_t n_index_iter = 0;
            while(n_index_iter != EGGINDEXNODEVIEW_RDCNT(lp_node_view))
            {
                HEGGINDEXRECORD pRecord = EGGINDEXNODEVIEW_RECORD_INDEX(lp_node_view, n_index_iter);
                printf("KEY[%.*s] KEY CNT : %d\n",
                       EGGINDEXRECORD_KSIZE(pRecord),
                       EGGINDEXRECORD_KEY(pRecord), n_key_cnt++);
                
                eggIndexView_insert(lp_index_bak_view, EGGINDEXRECORD_KEY(pRecord), EGGINDEXRECORD_KSIZE(pRecord), EGGINDEXRECORD_VAL(pRecord), EGGINDEXRECORD_VSIZE(pRecord) );
                cnt++;
                n_index_iter++;
            }
            size16_t n_node_size = sizeof(EGGINDEXNODE) + lp_index_view->hInfo->rdSize * (lp_index_view->hInfo->rdCnt + 1);

            n_iter_off = eggIndexNodeView_get_nextoff(lp_node_view);
            eggIndexNodeView_delete(lp_node_view);
        }
        eggFieldView_release_indexinfo(lp_field_view, lp_index_info_bak->fid, lp_index_info_bak);
//        free(lp_index_info);
        printf("field %d[%d]\n", n_field, cnt);
        printf("------------------------------------\n");
        n_field++;
    }
    return 0;
}
