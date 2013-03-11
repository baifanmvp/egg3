#ifndef _EGG_LIST_TEST
#define _EGG_LIST_TEST

#include "index/eggListView.h"
#include "EggDef.h"

#include <cppunit/extensions/HelperMacros.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <time.h>

#include <sys/time.h>

/*!
  \class CeggListTest
  \brief list（CeggListTest）测试类
*/
class CeggListTest: public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CeggListTest);
//    CPPUNIT_TEST(testAdd);
    CPPUNIT_TEST(testAddbat);

//    CPPUNIT_TEST(testFind);
    CPPUNIT_TEST_SUITE_END();
public:
    /*!
      \fn CeggListTest(void)
      \brief 构造函数
    */
    CeggListTest(void);

    /*!
      \fn ~CeggListTest(void)
      \brief 析构函数
    */
    ~CeggListTest(void);

    void setUp(void);

    void tearDown(void);

    /*!
      \fn void testAdd(void)
      \brief 测试CeggListTest add
    */
    void testAdd(void);
    void testAddbat(void);
    /*!
      \fn void testFind(void)
      \brief 测试CeggListTest find
    */
    void testFind(void);



    
private:
};
#endif
