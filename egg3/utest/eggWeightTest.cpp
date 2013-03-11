#include "./eggWeightTest.h"
#include "./eggDirectory.h"
#include "./eggHttp.h"
#include "./eggCluster.h"
#include "./eggPath.h"
#include "./eggIndexReader.h"
#include "./eggIndexWriter.h"
#include "./eggQuery.h"
#include "./eggScoreDoc.h"
#include "./eggTopCollector.h"
#include "./eggIndexSearcher.h"


#include <stdio.h>
#include <time.h>
#include <sys/time.h>

CeggWeightTest::CeggWeightTest(void)
{
    return ;
}

CeggWeightTest::~CeggWeightTest(void)
{
    return ;
}

void CeggWeightTest::setUp(void)
{
    system("mkdir utest_data 2>/dev/null");
    return ;
}

void CeggWeightTest::tearDown(void)
{
    return ;
}

void CeggWeightTest::addWithName()
{
    unlink("utest_data/weight.test");
    unlink("utest_data/field.test");
    HEGGFILE hWFile =  EggFile_open("utest_data/weight.test");
    HEGGFILE hFFile =  EggFile_open("utest_data/field.test");
    
    epointer lp_init_tmp = (epointer)malloc(MAP_VIEW_OFFSET);
    memset(lp_init_tmp, 0, MAP_VIEW_OFFSET);

    EggFile_write(hWFile, lp_init_tmp, MAP_VIEW_OFFSET, 0);


    
    HEGGFIELDVIEW lp_field_view = eggFieldView_new(hFFile);

    fdid_t fid = eggFieldView_register(lp_field_view, "price", EGG_INDEX_INT64);

    HEGGFIELDWEIGHT lp_field_weight = eggFieldWeight_new(hWFile, lp_field_view);
    count_t cnt = 10;
    index_t idx = 0;
    srand(time(0));
    while(idx != cnt )
    {
        long long val = rand()%10;
        HEGGFIELD hField =  eggField_new("price",
                                         (char*)&val, sizeof(val),
                                         EGG_NOT_ANALYZED | EGG_INDEX_INT64 | EGG_STORAGE);

        eggFieldWeight_add(lp_field_weight, hField, (idx+1) * 10);    
        eggField_delete(hField);
        idx++;
    }
    return ;
}

void CeggWeightTest::getWithName()
{
    HEGGFILE hWFile =  EggFile_open("utest_data/weight.test");
    HEGGFILE hFFile =  EggFile_open("utest_data/field.test");

    HEGGFIELDVIEW lp_field_view = eggFieldView_new(hFFile);

    fdid_t fid = eggFieldView_register(lp_field_view, "price", EGG_INDEX_INT64);

    HEGGFIELDWEIGHT lp_field_weight = eggFieldWeight_new(hWFile, lp_field_view);
    count_t n_rec_cnt = 0;
    
    HEGGWRESULT lp_res = eggFieldWeight_get_withname(lp_field_weight, "price", &n_rec_cnt);
    index_t idx = 0;
    
    while(idx != n_rec_cnt)
    {
        printf("idx : [%llu] weight : [%llu]\n", lp_res[idx].id, *(long long *)(lp_res[idx].val));
        idx++;
    }
    printf("n_rec_cnt : %d\n", n_rec_cnt);
    return ;
}

void CeggWeightTest::deleteWithName()
{
    HEGGFILE hWFile =  EggFile_open("utest_data/weight.test");
    HEGGFILE hFFile =  EggFile_open("utest_data/field.test");

    HEGGFIELDVIEW lp_field_view = eggFieldView_new(hFFile);


    HEGGFIELDWEIGHT lp_field_weight = eggFieldWeight_new(hWFile, lp_field_view);
    count_t n_rec_cnt = 0;
    did_t id = 0;
//    printf("id : ");
//    scanf("%llu", &id);
    id = 1;
    offset64_t val = 0;;
    HEGGFIELD hField =  eggField_new("price",
                                     (char*)&val, sizeof(val),
                                     EGG_NOT_ANALYZED | EGG_INDEX_INT64 | EGG_STORAGE);
    
    EBOOL ret = eggFieldWeight_remove(lp_field_weight, hField, id);
    if(ret)
    {
        printf("delete OK [%llu]\n", id);
    }
    else
    {
        printf("delete false [%llu]\n", id);
    }
    
    return ;
}
