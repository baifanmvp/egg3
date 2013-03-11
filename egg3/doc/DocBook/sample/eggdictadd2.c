#include <egg3/Egg3.h>
int main()
{
     HEGGHANDLE hHandle = eggPath_open("file:///egg/");
     
     HEGGINDEXWRITER hIndexWriter = eggIndexWriter_open(hHandle, "");
     
     {    
         HEGGDOCUMENT hDocument = eggDocument_new();

         HEGGFIELD hField1 = eggField_new("content", "一次性交易", strlen("一次性交易"),EGG_CWS_ANALYZED | EGG_INDEX_STRING | EGG_STORAGE);
         eggField_set_dictname(hField1, "cwstest");
         eggDocument_add(hDocument, hField1);
         
         if(eggIndexWriter_add_document(hIndexWriter, hDocument))
         {
             printf("Doc1：(add dictword!) ｛ content: “一次性交易”｝   add OK \n");

         }
         
         eggDocument_delete(hDocument);
     }
     
     if(eggIndexWriter_optimize(hIndexWriter))
     {
         printf("optimize success! \n");
     }
     eggIndexWriter_close(hIndexWriter);
     eggPath_close(hHandle);

     return 0;
}
