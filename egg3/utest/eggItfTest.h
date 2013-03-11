#ifndef _EGG_ITF_TEST_H_
#define _EGG_ITF_TEST_H_


#include <cppunit/extensions/HelperMacros.h>


class CeggItfTest: public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CeggItfTest);
    CPPUNIT_TEST(testAddandSearch);
    
//     CPPUNIT_TEST(testIndexAdd);
//   CPPUNIT_TEST(testIndexSearch);
    CPPUNIT_TEST_SUITE_END();
    
public:
    CeggItfTest(void);
    
    ~CeggItfTest(void);

    void setUp(void);
    void tearDown(void);
    
    void testDown(void);

    void testIndexAdd(char*);

    void testIndexSearch(char*);
    void testIndexSearchIter(char*);
    void testReIndex(char* );
    void testExportDoc(char*);
    void testDeleteDoc(char*);
    void testModifyDoc(char*);
    void testAddandSearch();
    void testIndexUpdate(char*);
    void testIndexMultiSearch(char*);
private:
    
};


#endif //_EGG_ITF_ADD_TEST_H_
