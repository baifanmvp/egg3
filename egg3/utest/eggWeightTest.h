#ifndef _EGG_WEIGHT_TEST
#define _EGG_WEIGHT_TEST

#include "EggDef.h"
#include "index/eggFieldWeight.h"
#include "index/eggFieldView.h"

#include <cppunit/extensions/HelperMacros.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <time.h>

#include <sys/time.h>

/*!
  \class Cegg2WeightTest
  \brief eggFieldWeight 测试类
*/
class CeggWeightTest: public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CeggWeightTest);
    CPPUNIT_TEST(addWithName);
    CPPUNIT_TEST(deleteWithName);
 
    CPPUNIT_TEST(getWithName);
//    CPPUNIT_TEST(testFind);
    CPPUNIT_TEST_SUITE_END();
public:
    /*!
      \fn CeggWeightTest(void)
      \brief 构造函数
    */
    CeggWeightTest(void);

    /*!
      \fn ~CeggWeightTest(void)
      \brief 析构函数
    */
    ~CeggWeightTest(void);

    void setUp(void);

    void tearDown(void);

    /*!
      \fn void addWithName(void)
      \brief 测试eggFieldWeight add
    */
    void addWithName(void);

    /*!
      \fn void testFind(void)
      \brief 测试eggBTree find
    */
    void getWithName(void);
    
    void deleteWithName(void);

    
private:
};
#endif

