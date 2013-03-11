#ifndef _EGG_RECOVERYLOG_TEST
#define _EGG_RECOVERYLOG_TEST

#include "index/eggIndexView.h"
#include "storage/eggRecoveryLog.h"
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
  \class CeggRecoveryLogTest
  \brief b+ tree（eggRecoveryLog）测试类
*/
class CeggRecoveryLogTest: public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CeggRecoveryLogTest);
    CPPUNIT_TEST(testAdd);
//    CPPUNIT_TEST(testFind);
    CPPUNIT_TEST_SUITE_END();
public:
    /*!
      \fn CeggRecoveryLogTest(void)
      \brief 构造函数
    */
    CeggRecoveryLogTest(void);

    /*!
      \fn ~CeggRecoveryLogTest(void)
      \brief 析构函数
    */
    ~CeggRecoveryLogTest(void);

    void setUp(void);

    void tearDown(void);

    /*!
      \fn void testAdd(void)
      \brief 测试eggRecoveryLog add
    */
    void testAdd(void);

    /*!
      \fn void testFind(void)
      \brief 测试eggRecoveryLog find
    */
    void testFind(void);



    
private:
};
#endif
