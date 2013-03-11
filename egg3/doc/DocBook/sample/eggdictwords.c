#include <egg3/Egg3.h>
int main()
{
     HEGGHANDLE hHandle = eggPath_open("file:///%%%/var/lib/egg3/sysdata/");
     
     HEGGINDEXWRITER hIndexWriter = eggIndexWriter_open(hHandle, "");
     
     
         HEGGDOCUMENT hDocument = eggDocument_new();

         HEGGFIELD hField1 = eggField_new(EGG_SYS_TYPEINFO, "dict", strlen("dict"),EGG_NOT_ANALYZED | EGG_INDEX_STRING | EGG_STORAGE);
         eggDocument_add(hDocument, hField1);
         
         HEGGFIELD hField2 = eggField_new(EGG_SYS_ANALYNAME, "ImCwsLexAnalyzer", strlen("ImCwsLexAnalyzer"), EGG_NOT_ANALYZED | EGG_INDEX_STRING | EGG_STORAGE);
         eggDocument_add(hDocument, hField2);

         HEGGFIELD hField3 = eggField_new(EGG_SYS_DICTNAME, "cwstest", strlen("cwstest"), EGG_NOT_ANALYZED | EGG_INDEX_STRING | EGG_STORAGE);
         eggDocument_add(hDocument, hField3);

         HEGGFIELD hField4 = eggField_new(EGG_SYS_DICTKEY, "性交易", strlen("性交易"), EGG_NOT_ANALYZED | EGG_INDEX_STRING | EGG_STORAGE);
         eggDocument_add(hDocument, hField4);

         if(eggIndexWriter_add_document(hIndexWriter, hDocument))
         {
             printf("dictword： “性交易”   add OK \n");

         }
         
         eggDocument_delete(hDocument);

     if(eggIndexWriter_optimize(hIndexWriter))
     {
         printf("optimize success! \n");
     }
     eggIndexWriter_close(hIndexWriter);
     eggPath_close(hHandle);

     return 0;
}
