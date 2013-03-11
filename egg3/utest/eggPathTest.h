#ifndef _EGG_PATH_TEST
#define _EGG_PATH_TEST
#include "eggPath.h"
#include <cppunit/extensions/HelperMacros.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

class CeggPathTest: public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CeggPathTest);
    CPPUNIT_TEST(testPath);
    CPPUNIT_TEST_SUITE_END();
public:
    /*!
      \fn CeggHttpTest(void)
      \brief 构造函数
    */
    CeggPathTest(void);

    ~CeggPathTest(void);

    void setUp(void);

    void tearDown(void);

    void testPath(void);


    
private:
};
#endif
