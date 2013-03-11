#include "eggPathTest.h"
 #include <time.h>
#include <sys/time.h>

CeggPathTest::CeggPathTest(void)
{
    
}

CeggPathTest::~CeggPathTest(void)
{
    
}

void CeggPathTest::setUp(void)
{
    
    return ;
}

void CeggPathTest::tearDown(void)
{
    return ;
    
}
void CeggPathTest::testPath(void)
{

    eggPath_open("/usr/local/fjdk");
    eggPath_open("192.168.1.3:433/cgi-bin/egg.fcgi?EGG_DIR_PATH=/usr/local/fjdk");
    eggPath_open("192.168.1.1:243343:/usr/local/fjdk");

    printf("hello world\n");
    return ;
}
