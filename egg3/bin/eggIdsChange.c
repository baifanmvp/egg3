#include <stdio.h>
#include <egg3/index/eggIdView.h>
#include <egg3/index/eggIndexView.h>
#include <egg3/index/eggFieldView.h>
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
#pragma pack(pop)
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

int date_to_sec(char* date)
{
  struct tm tm1= {0};
  tm1.tm_year=atoi(date)-1900;
  tm1.tm_mon=atoi(strchr(date, '-')+1)-1;
  tm1.tm_mday=atoi(strrchr(date, '-')+1);
  return mktime(&tm1);
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
    char iddBakPath[1024];
    char idxPath[1024];
    HEGGDIRECTORY lp_dir =  eggDirectory_open(argv[1]);
    HEGGINDEXREADER hEggIndexReader = eggIndexReader_open(lp_dir);
    
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

    //GHashTable* phash = g_hash_table_new(g_int64_hash, g_int64_equal);
    fdid_t fdid = 0;
    struct eggIndexInfo *lp_index_info;
    int n_field = 0;
    int n_key_cnt = 0;
    while ((lp_index_info = eggFieldView_iter(lp_field_view, &fdid)))
    {
        eggIndexView_load_info(lp_index_view, lp_index_info, lp_field_view);
        
        int cnt = 0;
        offset64_t n_iter_off = EGGINDEXVIEW_LEAFOFF(lp_index_view);
        while (n_iter_off)
        {
        }
        
    }
    return 0;
}
