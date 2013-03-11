#include "eggWeightTest.h"

#include "eggHttpTest.h"
#include "eggPathTest.h"
#include "eggConfigTest.h"
#include "eggItfTest.h"
#include "eggFileTest.h"
#include "eggViewStreamTest.h"
#include "eggBTreeTest.h"

#include "eggThreadPoolTest.h"

#include "eggIdViewTest.h"
#include "eggIdTableTest.h"
#include <cppunit/TestSuite.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestAssert.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/extensions/HelperMacros.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <sys/time.h>
//#include <walrus/walrus.h>

void eggPathTest()
{
    printf("<--   system bit : %d bit   -->\n", _SYS_BIT);
    
    CPPUNIT_TEST_SUITE_REGISTRATION(CeggPathTest);
	CppUnit::TextUi::TestRunner runner;
	CppUnit::TestFactoryRegistry &registry =
        CppUnit::TestFactoryRegistry::getRegistry();
	runner.addTest(registry.makeTest());
	runner.run();
   
}


void eggtpTest()
{
    printf("<--   system bit : %d bit   -->\n", _SYS_BIT);
    
    CPPUNIT_TEST_SUITE_REGISTRATION(eggThreadPoolTest);
	CppUnit::TextUi::TestRunner runner;
	CppUnit::TestFactoryRegistry &registry =
        CppUnit::TestFactoryRegistry::getRegistry();
	runner.addTest(registry.makeTest());
	runner.run();
   

}


void eggItfTest(void)
{
    printf("<--   system bit : %d bit   -->\n", _SYS_BIT);
    CPPUNIT_TEST_SUITE_REGISTRATION( CeggItfTest);

	CppUnit::TextUi::TestRunner runner;
	CppUnit::TestFactoryRegistry &registry =
        CppUnit::TestFactoryRegistry::getRegistry();
	runner.addTest(registry.makeTest());
	runner.run();

}
void eggIdTableTest(void)
{


    printf("<--   system bit : %d bit   -->\n", _SYS_BIT);
    CPPUNIT_TEST_SUITE_REGISTRATION( CeggIdTableTest);

	CppUnit::TextUi::TestRunner runner;
	CppUnit::TestFactoryRegistry &registry =
        CppUnit::TestFactoryRegistry::getRegistry();
	runner.addTest(registry.makeTest());
	runner.run();


}
void eggNoteTest(void)
{
//   printf("<--   system bit : %d bit   -->\n", _SYS_BIT);
//    CPPUNIT_TEST_SUITE_REGISTRATION(CeggNoteTest);

// 	CppUnit::TextUi::TestRunner runner;
// 	CppUnit::TestFactoryRegistry &registry =
//         CppUnit::TestFactoryRegistry::getRegistry();
// 	runner.addTest(registry.makeTest());
// 	runner.run();
   
}

void eggFileTest(void)
{
  printf("<--   system bit : %d bit   -->\n", _SYS_BIT);
  CPPUNIT_TEST_SUITE_REGISTRATION(CeggFileTest);

   CppUnit::TextUi::TestRunner runner;
   CppUnit::TestFactoryRegistry &registry = CppUnit::TestFactoryRegistry::getRegistry();
   runner.addTest(registry.makeTest());
   runner.run();
   return;
}

void eggViewStreamTest(void)
{
  printf("<--   system bit : %d bit   -->\n", _SYS_BIT);
  CPPUNIT_TEST_SUITE_REGISTRATION(CeggViewStreamTest);

   CppUnit::TextUi::TestRunner runner;
   CppUnit::TestFactoryRegistry &registry = CppUnit::TestFactoryRegistry::getRegistry();
   runner.addTest(registry.makeTest());
   runner.run();
   return;
}








void eggfieldTest(void)
{

    
    return;
}

void eggbtreeTest(void)
{

    CPPUNIT_TEST_SUITE_REGISTRATION(CeggBTreeTest);

    CppUnit::TextUi::TestRunner runner;
    CppUnit::TestFactoryRegistry &registry = CppUnit::TestFactoryRegistry::getRegistry();
    runner.addTest(registry.makeTest());
    runner.run();

    
    return;
}

void eggHttpTest(void)
{

    CPPUNIT_TEST_SUITE_REGISTRATION(CeggHttpTest);

    CppUnit::TextUi::TestRunner runner;
    CppUnit::TestFactoryRegistry &registry = CppUnit::TestFactoryRegistry::getRegistry();
    runner.addTest(registry.makeTest());
    runner.run();

    
    return;
}

void eggIdViewTest(void)
{

    CPPUNIT_TEST_SUITE_REGISTRATION(CeggIdViewTest);

    CppUnit::TextUi::TestRunner runner;
    CppUnit::TestFactoryRegistry &registry = CppUnit::TestFactoryRegistry::getRegistry();
    runner.addTest(registry.makeTest());
    runner.run();

    
    return;
}

void eggConfigTest(void)
{

    CPPUNIT_TEST_SUITE_REGISTRATION(CeggConfigTest);

    CppUnit::TextUi::TestRunner runner;
    CppUnit::TestFactoryRegistry &registry = CppUnit::TestFactoryRegistry::getRegistry();
    runner.addTest(registry.makeTest());
    runner.run();

    
    return;
}


void eggWeightTest(void)
{

    CPPUNIT_TEST_SUITE_REGISTRATION(CeggWeightTest);

    CppUnit::TextUi::TestRunner runner;
    CppUnit::TestFactoryRegistry &registry = CppUnit::TestFactoryRegistry::getRegistry();
    runner.addTest(registry.makeTest());
    runner.run();

    
    return;
}


int main(int argc, char ** argv)
{
    if(argc < 2)
    {
        return -1;
    }
    else
    {
        if( strcmp(argv[1], "fileTest") == 0 )
        {
            eggFileTest();
        }
        if( strcmp(argv[1], "ViewStreamTest") == 0 )
        {
            eggViewStreamTest();
        }
        if (strcmp(argv[1], "ItfTest") == 0)
        {
            eggItfTest();
        }
        if (strcmp(argv[1], "fieldTest") == 0)
        {
            eggfieldTest();
        }
        if (strcmp(argv[1], "btreeTest") == 0)
        {
            eggbtreeTest();
        }
        if (strcmp(argv[1], "RecoveryLogTest") == 0)
        {
//            eggrecoverylogtest();
        }
        if (strcmp(argv[1], "listTest") == 0)
        {
            //          egglistTest();
        }
        if (strcmp(argv[1], "httpTest") == 0)
        {
            eggHttpTest();
        }
        if (strcmp(argv[1], "idViewTest") == 0)
        {
            eggIdViewTest();
        }
        
        if (strcmp(argv[1], "configTest") == 0)
        {
            eggConfigTest();
        }

        if( strcmp(argv[1],"IdTableTest")==0)
        {
            eggIdTableTest();
        }

        if( strcmp(argv[1],"eggtpTest")==0)
        {
            eggtpTest();
        }
        if( strcmp(argv[1],"eggPathTest")==0)
        {
            eggPathTest();
        }
        if( strcmp(argv[1],"eggWeightTest")==0)
        {
            eggWeightTest();
        }
    }
    
    return 0;
}
