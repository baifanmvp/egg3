#ifndef _EGG_BTREE_TEST
#define _EGG_BTREE_TEST

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
class CeggBTreeTest: public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CeggBTreeTest);
        CPPUNIT_TEST(testAdd);
//    CPPUNIT_TEST(testFind);
    CPPUNIT_TEST_SUITE_END();
public:
    /*!
      \fn CeggBTreeTest(void)
      \brief 构造函数
    */
    CeggBTreeTest(void);

    /*!
      \fn ~CeggBTreeTest(void)
      \brief 析构函数
    */
    ~CeggBTreeTest(void);

    void setUp(void);

    void tearDown(void);

    /*!
      \fn void testAdd(void)
      \brief 测试eggBTree add
    */
    void testAdd(void);

    /*!
      \fn void testFind(void)
      \brief 测试eggBTree find
    */
    void testFind(void);



    
private:
};
#endif

