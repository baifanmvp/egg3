#include <egg3/Egg3.h>
struct personinfo
{
    char sex;
    int num;
};

struct personinfo persons[] = {{'M', 4}, {'M', 3},  {'M', 1},  {'M', 2},  {'M', 7},  {'M', 5},  {'M', 5},  {'M', 9},  {'M', 4},  {'M', 2},  {'M', 5},  {'M', 4},  {'M', 6},  {'W', 1},  {'W', 2},  {'W', 3},  {'W', 7},  {'M', 5},  {'W', 4},  {'W', 4}};
int main()
{
     HEGGHANDLE hHandle = eggPath_open("file:///tmp/");
     
     HEGGINDEXWRITER hIndexWriter = eggIndexWriter_open(hHandle, "");
     
     int cnt = 20;
     index_t i = 0;
     while(i != cnt)
     {
         HEGGDOCUMENT hDocument = eggDocument_new();

         HEGGFIELD hField1 = eggField_new("sex", (char*)&persons[i].sex, 1,
                                          EGG_NOT_ANALYZED | EGG_INDEX_STRING | EGG_STORAGE);
         
         HEGGFIELD hField2 = eggField_new("num", (void*)&persons[i].num, sizeof(int),
                                          EGG_NOT_ANALYZED | EGG_INDEX_INT32 | EGG_STORAGE);
         
         eggDocument_add(hDocument, hField1);
         eggDocument_add(hDocument, hField2);
         
         eggIndexWriter_add_document(hIndexWriter, hDocument);
                  
         eggDocument_delete(hDocument);
         i++;
     }
     
     if(eggIndexWriter_optimize(hIndexWriter))
     {
         printf("optimize success! \n");
     }
     eggIndexWriter_close(hIndexWriter);
     eggPath_close(hHandle);

     return 0;
}
