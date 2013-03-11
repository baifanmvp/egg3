#include <egg3/Egg3.h>
int main()
{
     HEGGHANDLE hHandle = eggPath_open("file:///tmp/");
     
     HEGGINDEXWRITER hIndexWriter = eggIndexWriter_open(hHandle, "");
     
     int cnt = 100;
     index_t i = 0;
     while(i != cnt)
     {
         HEGGDOCUMENT hDocument = eggDocument_new();

         HEGGFIELD hField1 = eggField_new("content", "hello", strlen("hello")+1,
                                          EGG_NOT_ANALYZED | EGG_INDEX_STRING | EGG_STORAGE);
         
         
         eggDocument_add(hDocument, hField1);
         
         
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
