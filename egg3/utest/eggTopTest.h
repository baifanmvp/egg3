#ifndef _EGG_TOP_TEST
#define _EGG_TOP_TEST

#include "TopCollector.h"
#include "InterfaceDef.h"
#include "EggDef.h"

#include <cppunit/extensions/HelperMacros.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <time.h>

#include <sys/time.h>

/*!
  \class CeggTopTest
  \brief scoreDoc集（TopCollector）测试类
*/
class CeggTopTest: public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CeggTopTest);
    CPPUNIT_TEST(testMerge);
    CPPUNIT_TEST_SUITE_END();
public:
        /*!
      \fn CeggTopTest(void)
      \brief construct fn
    */
    CeggTopTest(void);

    /*!
      \fn ~CeggTopTest(void)
      \brief destruct fn
    */
    ~CeggTopTest(void);

    void setUp(void);

    void tearDown(void);

    /*!
      \fn void testMerge(void)
      \brief test the time of merge scoreDocs
    */
    void testMerge(void);

    
private:
    TOPCOLLECTOR* m_hTopCollector;
};
#endif
