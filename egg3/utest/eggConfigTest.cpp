#include "eggConfigTest.h"
 #include <time.h>
#include <sys/time.h>
extern HEGGCONFIG g_config_handle;

CeggConfigTest::CeggConfigTest(void)
{
    
}

CeggConfigTest::~CeggConfigTest(void)
{
    
}

void CeggConfigTest::setUp(void)
{
    
    return ;
}

void CeggConfigTest::tearDown(void)
{
    return ;
    
}

void CeggConfigTest::testRead(void)
{
    eggAnalyzer_get("ImCyLexAnalyzer");
    eggAnalyzer_get("ImCnLexAnalyzer");
    eggAnalyzer_get("ImCwsLexAnalyzer");
    eggAnalyzer_get("ImCwsLexAnalyzer");
    eggAnalyzer_get("ImCwsLexAnalyzer");
    eggAnalyzer_get("ImCwsLexAnalyzer");
    eggAnalyzer_get("ImCwsLexAnalyzer");
    return ;
}
