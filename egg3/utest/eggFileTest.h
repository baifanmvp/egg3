#ifndef _EGG_FILE_TEST_H_
#define _EGG_FILE_TEST_H_

#include "storage/File.h"
#include "EggDef.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <time.h>

#include <sys/time.h>
#define rpl_malloc malloc
#include <cppunit/extensions/HelperMacros.h>

/*!
  \class CeggFileTest
  \brief 文件操作测试类
*/
class CeggFileTest: public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CeggFileTest);
    CPPUNIT_TEST(testOpenClose);
    CPPUNIT_TEST(testRead);
    CPPUNIT_TEST(testWrite);
//    CPPUNIT_TEST();
    //  CPPUNIT_TEST(testNoteExportData);
    CPPUNIT_TEST_SUITE_END();

public:
    /*!
      \fn CeggFileTest(void)
      \brief 构造函数
    */
    CeggFileTest(void);

    /*!
      \fn ~CeggFileTest(void)
      \brief 析构函数
    */
    ~CeggFileTest(void);

    void setUp(void);

    void tearDown(void);

    /*!
      \fn void testOpenClose(void)
      \brief 测试文件打开和关闭
    */
    void testOpenClose(void);

    /*!
      \fn void testRead(void)
      \brief 测试文件读取
    */
    void testRead(void);

    /*!
      \fn void testWrite(void)
      \brief 测试文件写入
    */
    void testWrite(void);

    /*!
      \fn void testUpdate(void)
      \brief
    */
    void testUpdate(void);

    /*!
      \fn void testLock(void)
      \brief
    */
    void testLock(void);
    
private:

    HEGGFILE m_hEggFile;
};


#endif //_EGG_FILE_TEST_H_

