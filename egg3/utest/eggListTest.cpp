#include "egg2ListTest.h"
 #include <time.h>
#include <sys/time.h>

Cegg2ListTest::Cegg2ListTest(void)
{
    
}

Cegg2ListTest::~Cegg2ListTest(void)
{
    
}

void Cegg2ListTest::setUp(void)
{
    
    return ;
}

void Cegg2ListTest::tearDown(void)
{
    return ;
    
}

void Cegg2ListTest::testAdd(void)
{
    printf("\Cegg2ListTest::testAdd \n");
    system("rm ./list/list.idx");
    system("cp ./list/list.idx.bak ./list/list.idx");
    HEGGFILE lp_egg_file = EggFile_open("./list/list.idx");

    HVIEWSTREAM lp_view_stream = ViewStream_new(lp_egg_file);
    HEGGLISTVIEW lp_list_view = eggListView_new(lp_view_stream);
    char key[] = "123456";
    
    HEGGLISTINF hInfo = (HEGGLISTINF)malloc(sizeof(EGGLISTINF) + strlen(key) + 1);
    
    memset(hInfo, 0, sizeof(EGGLISTINF) + strlen(key) + 1);
    hInfo->aSz = sizeof(EGGLISTINF) + strlen(key) + 1;
    hInfo->nodeSz = 8;
    memcpy(hInfo+1, key, strlen(key));
    
    eggListView_regInfo(lp_list_view, hInfo);
    long long index = 0;
    while(index != 2000000)
    {
         eggListView_insert(lp_list_view, &index, 8);
         printf("%lld\n", index);
         index++;
    }
    printf("-----------\n");
    long long *offs;
    size32_t n_node_size;
    eggListView_fetch(lp_list_view, (void**)&offs, &n_node_size);
    count_t cnt = n_node_size / 8;
    printf("cnt ; %d\n", cnt);
    
    while(cnt--)
    {
        printf("offs ; %lld\n", offs[cnt]);
    }
    
    eggListView_delete(lp_list_view);

    ViewStream_delete(lp_view_stream);
//    EggFile_close(lp_egg_file);
    return ;
}


void Cegg2ListTest::testAddbat(void)
{
    printf("\Cegg2ListTest::testAdd \n");
    //system("rm ./list/list.idx");
    // system("cp ./list/list.idx.bak ./list/list.idx");
    HEGGFILE lp_egg_file = EggFile_open("./list/list.idx");

    HVIEWSTREAM lp_view_stream = ViewStream_new(lp_egg_file);
    HEGGLISTVIEW lp_list_view = eggListView_new(lp_view_stream);
    char key[] = "123456";
    
    HEGGLISTINF hInfo = (HEGGLISTINF)malloc(sizeof(EGGLISTINF) + strlen(key) + 1);
    
    memset(hInfo, 0, sizeof(EGGLISTINF) + strlen(key) + 1);
    hInfo->aSz = sizeof(EGGLISTINF) + strlen(key) + 1;
    hInfo->nodeSz = 8;
    memcpy(hInfo+1, key, strlen(key));
    
    eggListView_regInfo(lp_list_view, hInfo);
    long long index = 0;
    long long buf[10000];
    int i = 10000;
    while(i--)
    {
        buf[i] = i;
    }
    while(index != 10)
    {
         eggListView_insert(lp_list_view, buf, 8*10000);
         printf("%lld\n", index);
         index++;
    }
    printf("-----------\n");
    long long *offs;
    size32_t n_node_size;
    eggListView_fetch(lp_list_view, (void**)&offs, &n_node_size);
    count_t cnt = n_node_size / 8;
    printf("cnt ; %d\n", cnt);
    
    while(cnt--)
    {
        printf("offs ; %lld\n", offs[cnt]);
    }
    
    eggListView_delete(lp_list_view);

    ViewStream_delete(lp_view_stream);
//    EggFile_close(lp_egg_file);
    return ;
}


void Cegg2ListTest::testFind(void)
{
    return ;
    
}


