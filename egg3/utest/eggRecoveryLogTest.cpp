#include "egg2RecoveryLogTest.h"
 #include <time.h>
#include <sys/time.h>
extern "C" int eggRecoveryLog_recover();

Cegg2RecoveryLogTest::Cegg2RecoveryLogTest(void)
{
    
}

Cegg2RecoveryLogTest::~Cegg2RecoveryLogTest(void)
{
    
}

void Cegg2RecoveryLogTest::setUp(void)
{
    
    return ;
}

void Cegg2RecoveryLogTest::tearDown(void)
{
    return ;
    
}

void Cegg2RecoveryLogTest::testAdd(void)
{
    HEGGFILE lp_egg_file = EggFile_open("./btree/RecoveryLog.idx");

    HVIEWSTREAM lp_view_stream = ViewStream_new(lp_egg_file);
    
    EGGINDEXINFO st_index_info = {0};
    st_index_info.rdSize = 192;
    st_index_info.rdCnt = 32;
    st_index_info.type = INDEXTYPE_STRING;

    HEGGINDEXVIEW lp_index_view = eggIndexView_new(lp_view_stream, &st_index_info);
    time_t timeval;
    char *val;
    char key[64];
    sprintf(key, "www.baidu.com");
    
    HEGGINDEXRECORD hIndexRecord;
    
    time(&timeval);
    val = "00";
    printf("Insert[%s][%s]\n", key, val);
    eggIndexView_insert(lp_index_view, key, strlen(key), val, strlen(val));
    
    key[0] = '2';
    eggIndexView_insertFalt(lp_index_view, key, strlen(key), val, strlen(val));
    eggRecoveryLog_recover();
    exit(0);    
    
    hIndexRecord = eggIndexView_fetch(lp_index_view, key, strlen(key));
    if (hIndexRecord)
    {
    printf("Fetch[%.*s][%.*s]\n",
           EGGINDEXRECORD_KSIZE(hIndexRecord),
           EGGINDEXRECORD_KEY(hIndexRecord),
           EGGINDEXRECORD_VSIZE(hIndexRecord),
           EGGINDEXRECORD_VAL(hIndexRecord));
    }
    else
    {
        printf("falt%p\n", hIndexRecord);
    }

    
    
    eggIndexView_delete(lp_index_view);
    ViewStream_delete(lp_view_stream);
//    EggFile_close(lp_egg_file);
    return ;
}

void Cegg2RecoveryLogTest::testFind(void)
{
    return ;
    
}


