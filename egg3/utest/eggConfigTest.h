#ifndef _EGG_CONFIG_TEST
#define _EGG_CONFIG_TEST

#include "conf/eggConfig.h"
#include "eggAnalyzer.h"
#include "EggDef.h"

#include <cppunit/extensions/HelperMacros.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <time.h>

#include <sys/time.h>

/*!
  \class CeggConfigTest
  \brief（eggConfig）测试类
*/
class CeggConfigTest: public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CeggConfigTest);
    //    CPPUNIT_TEST(testAdd);
    CPPUNIT_TEST(testRead);
    CPPUNIT_TEST_SUITE_END();
public:
    /*!
      \fn CeggConfigTest(void)
      \brief 构造函数
    */
    CeggConfigTest(void);

    /*!
      \fn ~CeggConfigTest(void)
      \brief 析构函数
    */
    ~CeggConfigTest(void);

    void setUp(void);

    void tearDown(void);


    /*!
      \fn void testRead(void)
      \brief 测试eggConfig find
    */
    void testRead(void);



    
private:
    
};
#endif
