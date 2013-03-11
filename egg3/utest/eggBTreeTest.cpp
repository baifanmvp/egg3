#include "eggBTreeTest.h"
 #include <time.h>
#include <sys/time.h>

CeggBTreeTest::CeggBTreeTest(void)
{
    
}

CeggBTreeTest::~CeggBTreeTest(void)
{
    
}

void CeggBTreeTest::setUp(void)
{
    system(" mkdir utest_data/; rm utest_data/btree.idx; dd if=/dev/zero of=./utest_data/btree.idx bs=65536 count=1");
    return ;
}

void CeggBTreeTest::tearDown(void)
{
    system(" rm utest_data/btree.idx ");
    return ;
    
}



void CeggBTreeTest::testAdd(void)
{
    printf("\nCeggBTreeTest::testAdd \n");
     // system("rm ./btree/btree.idx");
     // system("cp ./btree/btree.idx.bak ./btree/btree.idx");
    HEGGFILE lp_egg_file = EggFile_open("./utest_data/btree.idx");

    
    EGGINDEXINFO st_index_info = {0};
    st_index_info.rdSize = 112;
    st_index_info.rdCnt = 36;
    st_index_info.type = EGG_INDEX_STRING | BTREE_RECORD_REPEAT;
    st_index_info.rootOff = 0;
    st_index_info.leafOff = 0;
    srand(time(0));
    HEGGINDEXVIEW lp_index_view = eggIndexView_new(lp_egg_file, &st_index_info);
    count_t index = 0;
    count_t count = 0;
    char key[64];
//    printf("count:");
//    scanf("%d", &count);
    struct timeval tstart, tend;
    gettimeofday(&tstart, 0);
    FILE* fp_file = fopen("kfc.txt", "r+");
    char *buf = EGG_NULL;
    size_t fileSize = 0;
    offset64_t off;
    size32_t random = 0;
    while (getline(&buf, &fileSize, fp_file) != -1 && count < 10000)
    {
        int tmp = index;
//         sprintf(key, "%d", tmp);
         //  eggBTree_insert(lp_egg_btree, key, strlen(key)+1, key, strlen(key) );
        random = rand()%100;
        off = rand()%10000;//100-count;//rand()%10000;
        struct eggRecordDocId docId = {0};
        docId.did = off;
        docId.flag = 1;
        char buf[32];
        sprintf(buf, "%u", random);
        eggIndexView_insert(lp_index_view, buf, strlen(buf), &docId, sizeof(docId) );
//        eggIndexView_insert(lp_index_view, &random, sizeof(random), &docId, sizeof(docId) );
        
        count++;
//        if(index == count)break;
    }
    gettimeofday(&tend, 0);
    printf("time: %f\n", (double)(tend.tv_sec - tstart.tv_sec) + (double)(tend.tv_usec - tstart.tv_usec)/1000000);
    HEGGINDEXRECORD pRecord = 0;
///    return ;
    //   eggIndexView_itercheck(lp_index_view, lp_index_view->hInfo->rootOff, &pRecord);

    printf("-----------------------\n");
    printf("-----------------------\n");
    printf("-----------------------\n");
    printf("-----------------------\n");
    
    offset64_t n_iter_off = EGGINDEXVIEW_LEAFOFF(lp_index_view);
    while(n_iter_off)
    {
        HEGGINDEXNODEVIEW lp_node_view = eggIndexView_load_node(lp_index_view, n_iter_off);
        index_t n_index_iter = 0;
        
        while(n_index_iter != EGGINDEXNODEVIEW_RDCNT(lp_node_view))
        {
            HEGGINDEXRECORD pRecord = EGGINDEXNODEVIEW_RECORD_INDEX(lp_node_view, n_index_iter);
            //     printf("------key [%s]\n", EGGINDEXRECORD_KEY(pRecord));
            printf("key [%s], [%llu]\n", EGGINDEXRECORD_KEY(pRecord), ((struct eggRecordDocId*)EGGINDEXRECORD_VAL(pRecord))->did );
//            printf("[%llu]\n",  ((struct eggRecordDocId*)EGGINDEXRECORD_VAL(pRecord))->did );

            n_index_iter++;
        }
        
        n_iter_off = eggIndexNodeView_get_nextoff(lp_node_view);
        eggIndexNodeView_delete(lp_node_view);
    }
    
//////////////////////////////////////////////////////////////////////////////////////
//  char buf[32];
    printf("********************************\n");
    count = 0;
    while(count < 100)
    {
        char start[32];
        char end[32];
        scanf("%s", start);
        scanf("%s", end);
        sprintf(buf, "%d", count);
        HEGGINDEXRANGE p_range = eggIndexView_range(lp_index_view, start, strlen(start), end, strlen(end));
        while(index < p_range->cnt)
        {
            printf("did : %llu\n ", p_range->dids[index].did);
//            printf("[%llu]\n", p_range->dids[index].did);
            index++;
        }
        index =0;
        count++;

        /*
        scanf("%s", buf);
        sprintf(buf, "%d", count);
        HEGGINDEXRANGE p_range = eggIndexView_fetch_docId(lp_index_view, (void*)buf, strlen(buf));
        //      HEGGINDEXRANGE p_range = eggIndexView_fetch_docId(lp_index_view, &count, sizeof(count));
        while(index < p_range->cnt)
        {
            printf("did : %llu\n ", p_range->dids[index].did);
//            printf("[%llu]\n", p_range->dids[index].did);
            index++;
        }
        index =0;
        count++;*/
    }
    // eggIndexView_delete(lp_index_view);
//    EggFile_close(lp_egg_file);
    return ;
}

void CeggBTreeTest::testFind(void)
{
    printf("\nCeggBTreeTest::testFind \n");
//     system("rm ./btree/btree.idx");
    //   system("cp ./btree/btree.idx.bak ./btree/btree.idx");
    HEGGFILE lp_egg_field = EggFile_open("/home/bf/Dev/egg_bak/egg.fdd");
    HEGGFILE lp_egg_idx = EggFile_open("/home/bf/Dev/egg_bak/egg.idx");
    HEGGFIELDVIEW lp_field_view = eggFieldView_new(lp_egg_field);

    fdid_t fdid = 0;
    char fieldname[100];
    printf("field name : ");
    scanf("%s", fieldname);
    eggFieldView_find(lp_field_view, fieldname, &fdid);

    struct eggIndexInfo* lp_index_info = eggFieldView_get_indexinfo(lp_field_view, fdid);

    
    EGGINDEXINFO st_index_info = *lp_index_info;

    HEGGINDEXVIEW lp_index_view = eggIndexView_new(lp_egg_idx, &st_index_info);
    struct timeval tstart, tend;

    HEGGINDEXRECORD pRecord;
    
    eggIndexView_itercheck(lp_index_view, lp_index_view->hInfo->rootOff, &pRecord);

    return ;
    
}


