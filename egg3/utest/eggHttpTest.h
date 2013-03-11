#ifndef _EGG_HTTP_TEST
#define _EGG_HTTP_TEST
#include "net/eggNetHttp.h"
#include "net/eggNetPackage.h"

#include <cppunit/extensions/HelperMacros.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <time.h>

#include <sys/time.h>

class CeggHttpTest: public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CeggHttpTest);
    CPPUNIT_TEST(testConnect);
    CPPUNIT_TEST_SUITE_END();
public:
    /*!
      \fn CeggHttpTest(void)
      \brief 构造函数
    */
    CeggHttpTest(void);

    ~CeggHttpTest(void);

    void setUp(void);

    void tearDown(void);

    void testConnect(void);


    
private:
};
#endif
