#include <Egg3.h>
extern "C"
{
#include "../src/storage/eggIdTable.h"
}
#include "eggIdTableTest.h"

CeggIdTableTest::CeggIdTableTest(void)
{
}

CeggIdTableTest::~CeggIdTableTest(void)
{
}

void CeggIdTableTest::setUp(void)
{
}

void CeggIdTableTest::tearDown(void)
{

}
void CeggIdTableTest::testIdTable(void)
{
   did_t i1,i2,i3,i4,i5;
   did_t f1,f2,f3,f4,f5;
   HEGGFILE hEggFile=EggFile_open("./lee");
   HEGGIDTABLE hIdTable=eggIdTable_new(hEggFile);
   i1=eggIdTable_map_id(hIdTable,250);
   i2=eggIdTable_map_id(hIdTable,150);
   i3=eggIdTable_map_id(hIdTable,450);
   i4=eggIdTable_map_id(hIdTable,550);
   i5=eggIdTable_map_id(hIdTable,350);
   printf("the index is:%ld\t",i1);
   printf("%ld\t",i2);
   printf("%ld\t",i3);
   printf("%ld\t",i4);
   printf("%ld\n",i5);
   f1=eggIdTable_get_off(hIdTable,i1);
   f2=eggIdTable_get_off(hIdTable,i2);
   f3=eggIdTable_get_off(hIdTable,i3);
   f4=eggIdTable_get_off(hIdTable,i4);
   f5=eggIdTable_get_off(hIdTable,i5);
   printf("the offset is:%ld\t",f1);
   printf("%ld\t",f2);
   printf("%ld\t",f3);
   printf("%ld\t",f4);
   printf("%ld\n",f5);

   EggFile_close(hEggFile);
}
