#include <egg3/Egg3.h>
int main()
{
     HEGGHANDLE hHandle = eggPath_open("file:///tmp/");
     
     HEGGINDEXWRITER hIndexWriter = eggIndexWriter_open(hHandle, "");
     
     
     {
         HEGGDOCUMENT hDocument = eggDocument_new();

         HEGGFIELD hField1 = eggField_new("title", "hello egg", strlen("hello egg")+1,
                                          EGG_NOT_ANALYZED | EGG_INDEX_STRING | EGG_STORAGE);
         HEGGFIELD hField2 = eggField_new("content", "egg is a good docdb", strlen("egg is a good docdb")+1,
                                          EGG_ANALYZED | EGG_INDEX_STRING | EGG_STORAGE);
         int time = 20120101;
         HEGGFIELD hField3 = eggField_new("time", (void*)&time, sizeof(time),
                                          EGG_NOT_ANALYZED | EGG_INDEX_INT32 | EGG_STORAGE);
         
         eggDocument_add(hDocument, hField1);
         eggDocument_add(hDocument, hField2);
         eggDocument_add(hDocument, hField3);
         
         eggDocument_set_weight(hDocument, 2);
         if(eggIndexWriter_add_document(hIndexWriter, hDocument))
         {
             printf("Doc1： ｛title: “hello egg”， content: “egg is good docdb”， time: 20120101｜weight＝2｝   add OK \n");

         }
         
         eggDocument_delete(hDocument);
     }

     {
         HEGGDOCUMENT hDocument = eggDocument_new();

         HEGGFIELD hField1 = eggField_new("title", "hello god", strlen("hello god")+1,
                                          EGG_NOT_ANALYZED | EGG_INDEX_STRING | EGG_STORAGE);
         HEGGFIELD hField2 = eggField_new("content", "god is a good boy", strlen("god is a good boy") + 1,
                                          EGG_ANALYZED | EGG_INDEX_STRING | EGG_STORAGE);
         int time = 20120102;
         HEGGFIELD hField3 = eggField_new("time", (void*)&time, sizeof(time),
                                          EGG_NOT_ANALYZED | EGG_INDEX_INT32 | EGG_STORAGE);
         
         eggDocument_add(hDocument, hField1);
         eggDocument_add(hDocument, hField2);
         eggDocument_add(hDocument, hField3);
         
         eggDocument_set_weight(hDocument, 1);
         if(eggIndexWriter_add_document(hIndexWriter, hDocument))
         {
             printf("Doc2： ｛title: “hello god”， content: “god is good boy”， time: 20120102｜weight＝1｝   add OK \n");
         }
         
         eggDocument_delete(hDocument);
     }
     
     {
         HEGGDOCUMENT hDocument = eggDocument_new();

         HEGGFIELD hField1 = eggField_new("title", "hello world", strlen("hello world") + 1,
                                          EGG_NOT_ANALYZED | EGG_INDEX_STRING | EGG_STORAGE);
         HEGGFIELD hField2 = eggField_new("content", "world is a good place", strlen("world is a good place") + 1,
                                          EGG_ANALYZED | EGG_INDEX_STRING | EGG_STORAGE);
         int time = 20120103;
         HEGGFIELD hField3 = eggField_new("time", (void*)&time, sizeof(time),
                                          EGG_NOT_ANALYZED | EGG_INDEX_INT32 | EGG_STORAGE);
         
         eggDocument_add(hDocument, hField1);
         eggDocument_add(hDocument, hField2);
         eggDocument_add(hDocument, hField3);
         
         eggDocument_set_weight(hDocument, 3);
         
         if(eggIndexWriter_add_document(hIndexWriter, hDocument))
         {
             printf("Doc3： ｛title: “hello world”， content: “world is good place”， time: 20120103｜weight＝3｝   add OK \n");
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
