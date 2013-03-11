#ifndef _EGG_IDVIEW_TEST
#define _EGG_IDVIEW_TEST

#include "index/eggIdView.h"
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
class CeggIdViewTest: public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CeggIdViewTest);
    //    CPPUNIT_TEST(testAdd);
    CPPUNIT_TEST(testIntegration);
    CPPUNIT_TEST_SUITE_END();
public:
    /*!
      \fn CeggIdViewTest(void)
      \brief 构造函数
    */
    CeggIdViewTest(void);

    /*!
      \fn ~CeggIdViewTest(void)
      \brief 析构函数
    */
    ~CeggIdViewTest(void);

    void setUp(void);

    void tearDown(void);

    /*!
      \fn void testAdd(void)
      \brief 测试eggBTree add
    */
    void testIntegration(void);


    
private:
};
#endif
