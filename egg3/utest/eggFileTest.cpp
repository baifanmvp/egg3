#include "eggFileTest.h"



CeggFileTest::CeggFileTest(void)
{
}

CeggFileTest::~CeggFileTest(void)
{
}

void CeggFileTest::setUp(void)
{
    system("mkdir utest_data 2>/dev/null");
    
    unlink("./utest_data/egg2FileTest.txt");
    system("touch ./utest_data/egg2FileTest.txt");

    m_hEggFile = EggFile_open("./utest_data/egg2FileTest.txt");

    CPPUNIT_ASSERT(m_hEggFile != EGG_NULL);
}

void CeggFileTest::tearDown(void)
{
    EBOOL eRet = EggFile_close(m_hEggFile);

    CPPUNIT_ASSERT(eRet == EGG_TRUE);
    system("rm utest_data/egg2FileTest.txt");
}

void CeggFileTest::testOpenClose(void)
{
    HEGGFILE hEggFile = EggFile_open("./utest_data/egg2FileTest.txt");

    CPPUNIT_ASSERT(hEggFile != EGG_NULL);
    
    EBOOL eRet = EggFile_close(hEggFile);

    CPPUNIT_ASSERT(eRet == EGG_TRUE);
    
}

void CeggFileTest::testRead(void)
{
    char rgsz_result[1024] = {0};
    memcpy(rgsz_result, "1234567890-0987643221", strlen("1234567890-0987643221"));
    EggFile_write(m_hEggFile, rgsz_result, sizeof(rgsz_result), 0);
    
    EBOOL eRet = EggFile_read(m_hEggFile, rgsz_result, 1024, 0);
    CPPUNIT_ASSERT(eRet == EGG_TRUE);

}

void CeggFileTest::testWrite(void)
{
    char rgsz_result[1024] = {0};
    memcpy(rgsz_result, "1234567890-0987643221", strlen("1234567890-0987643221"));
    EBOOL eRet = EggFile_write(m_hEggFile, rgsz_result, strlen(rgsz_result), 0);
    
    CPPUNIT_ASSERT(eRet == EGG_TRUE);

}

void CeggFileTest::testUpdate(void)
{
}

void CeggFileTest::testLock(void)
{

}

