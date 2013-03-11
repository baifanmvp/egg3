#include "eggViewStreamTest.h"



CeggViewStreamTest::CeggViewStreamTest(void)
{
}

CeggViewStreamTest::~CeggViewStreamTest(void)
{
}

void CeggViewStreamTest::setUp(void)
{
    system("mkdir utest_data 2>/dev/null");
    system("rm ./utest_data/egg2ViewStreamTest.txt 2>/dev/null; touch ./utest_data/egg2ViewStreamTest.txt");
    
    HEGGFILE hEggFile = EggFile_open("./utest_data/egg2ViewStreamTest.txt");
    m_hViewStream = 0;
    m_hViewStream =  ViewStream_new(hEggFile);
    CPPUNIT_ASSERT(m_hViewStream != EGG_NULL);

}

void CeggViewStreamTest::tearDown(void)
{
    EBOOL eRet = ViewStream_delete(m_hViewStream);

    CPPUNIT_ASSERT(eRet == EGG_TRUE);
    
    system("rm ./utest_data/egg2ViewStreamTest.txt 2>/dev/null");
}

void CeggViewStreamTest::testNewDelete(void)
{
    HEGGFILE hEggFile = EggFile_open("./utest_data/egg2ViewStreamTest.txt");

    HVIEWSTREAM hViewStream =  ViewStream_new(hEggFile);
    CPPUNIT_ASSERT(hViewStream != EGG_NULL);
    
    EBOOL eRet = ViewStream_delete(hViewStream);

    CPPUNIT_ASSERT(eRet == EGG_TRUE);
    
}


void CeggViewStreamTest::testReadWrite(void)
{
    char lpWrite[] = "A customer at the opening day on Saturday, surnamed Lin, spotted a group of creepy looking men at the bottom of the spiral staircase in the Pudong New Area store's glass cylindrical construction looking upwards attentively. At first she thought they were admiring the high-tech store's modern design.";
    char lpRead[1000] = {0};
    printf("%d\n", strlen(lpWrite));
    offset64_t nOffset = ViewStream_write(m_hViewStream,  lpWrite, strlen( lpWrite));
    ViewStream_read(m_hViewStream,  lpRead, strlen( lpWrite), nOffset);
    
    CPPUNIT_ASSERT(strncmp(lpWrite, lpRead, strlen( lpWrite) )  == 0);
    //printf("%s\n", lpRead);
}

void CeggViewStreamTest::testUpdate(void)
{
    char lpWrite[] = "A customer at the opening day on Saturday, surnamed Lin, spotted a group of creepy looking men at the bottom of the spiral staircase in the Pudong New Area store's glass cylindrical construction looking upwards attentively. At first she thought they were admiring the high-tech store's modern design.";
    char lpUpdate[] = "baifanmvp";
    char lpRead[1000] = {0};
    //printf("%d\n", strlen(lpWrite));
    
    offset64_t nOffset = ViewStream_write(m_hViewStream,  lpWrite, strlen( lpWrite));
    ViewStream_update(m_hViewStream,  lpUpdate, strlen(lpUpdate), nOffset);
    ViewStream_read(m_hViewStream,  lpRead, strlen( lpUpdate), nOffset);
    
    CPPUNIT_ASSERT(strncmp(lpUpdate, lpRead, strlen( lpUpdate ) ) == 0);
    //printf("%s\n", lpRead);
}

void CeggViewStreamTest::testUpdateInfo(void)
{

}

void CeggViewStreamTest::testReadInfo(void)
{

}

