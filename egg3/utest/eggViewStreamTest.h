#ifndef _EGG_VIEWSTREAM_TEST_H_
#define _EGG_VIEWSTREAM_TEST_H_

#include "storage/ViewStream.h"
#include "EggDef.h"

#include <cppunit/extensions/HelperMacros.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <time.h>

#include <sys/time.h>

/*!
  \class Cegg2ViewStreamTest
  \brief 读写流操作测试类
*/
class CeggViewStreamTest: public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CeggViewStreamTest);
    CPPUNIT_TEST(testNewDelete);
    CPPUNIT_TEST(testReadWrite);
    CPPUNIT_TEST(testUpdate);
    CPPUNIT_TEST_SUITE_END();

public:
    /*!
      \fn CeggViewStreamTest(void)
      \brief 构造函数
    */
    CeggViewStreamTest(void);

    /*!
      \fn ~CeggViewStreamTest(void)
      \brief 析构函数
    */
    ~CeggViewStreamTest(void);

    void setUp(void);

    void tearDown(void);

    /*!
      \fn void testNewDelete(void)
      \brief 测试ViewStream的创建和销毁
    */
    void testNewDelete(void);

    /*!
      \fn void testRead(void)
      \brief 测试ViewStream读取
    */
    void testReadWrite(void);


    /*!
      \fn void testUpdate(void)
      \brief 测试ViewStream更新
    */
    void testUpdate(void);

    /*!
      \fn void testUpdateInfo(void)
      \brief 测试ViewStream更新info
    */
    void testUpdateInfo(void);

    /*!
      \fn void testReadInfo(void)
      \brief 测试ViewStream更新info
    */
    void testReadInfo(void);

    
private:
    
    HVIEWSTREAM m_hViewStream;
};


#endif //_EGG_FILE_TEST_H_

