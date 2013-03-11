#ifndef _EGG_IDTABLE_TEST_H_
#define _EGG_IDTABLE_TEST_H_


#include <cppunit/extensions/HelperMacros.h>

class CeggIdTableTest: public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CeggIdTableTest);
    CPPUNIT_TEST(testIdTable);
    
//     CPPUNIT_TEST(testIndexAdd);
//   CPPUNIT_TEST(testIndexSearch);
    CPPUNIT_TEST_SUITE_END();
 public:
    CeggIdTableTest(void);
    
    ~CeggIdTableTest(void);

    void setUp(void);
    void tearDown(void);
    
    void testIdTable(void);
private:
    
};
#endif
