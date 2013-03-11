#ifndef _EGG_THREADPOOL_TEST
#define _EGG_THREADPOOL_TEST

#include "index/eggIndexView.h"
#include "index/eggFieldView.h"
#include "EggDef.h"

#include <cppunit/extensions/HelperMacros.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <time.h>

#include <sys/time.h>

/*!
  \class CeggBTreeTest
  \brief b+ tree（eggBTree）测试类
*/
class eggThreadPoolTest:public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(eggThreadPoolTest);
    //    CPPUNIT_TEST(testAdd);
    CPPUNIT_TEST(eggThreadPool11_add_work);
    CPPUNIT_TEST_SUITE_END();
    public:

    eggThreadPoolTest(void);
    ~eggThreadPoolTest(void);

    void setUp(void);

    void tearDown(void);
                
    void eggThreadPool11_add_work(void);
    
private:

};
#endif

