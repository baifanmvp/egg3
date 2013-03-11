#include <egg3/Egg3.h>
int main()
{
   HEGGHANDLE hEggHandle =  eggPath_open("file:///tmp/");
   HEGGINDEXWRITER hIndexWriter = eggIndexWriter_open(hEggHandle,  "");

   HEGGDOCUMENT hDocument = eggDocument_new();
   HEGGFIELD hField1 = eggField_new("content", "god is a bad boy", strlen("god is a bad boy") + 1, EGG_ANALYZED | EGG_INDEX_STRING | EGG_STORAGE);
   int time=20120104;
   HEGGFIELD hField2 = eggField_new("time", (char*)&time, sizeof(time), EGG_NOT_ANALYZED | EGG_INDEX_INT32 | EGG_STORAGE);
   HEGGFIELD hField3 = eggField_new("url", "www.god.com", strlen("www.god.com") + 1, EGG_NOT_ANALYZED | EGG_INDEX_STRING | EGG_STORAGE);

   eggDocument_add(hDocument, hField1);
   eggDocument_add(hDocument, hField2);
   eggDocument_add(hDocument, hField3);

   EGGDID did;
   EGGDID_DOCID(&did) = 2;
   printf("set document to modify \n");
   EBOOL ret = eggIndexWriter_modify_document( hIndexWriter, did, hDocument);
   if(ret == EGG_FALSE)
   {
   // modify false
       printf("modify false !\n");
   }
   else if(ret == EGG_TRUE)
   {
   // modify ture
       printf("modify ture !\n");
   }
   
   eggIndexWriter_optimize(hIndexWriter);
   
   eggIndexWriter_close(hIndexWriter);
   
   eggPath_close(hEggHandle);
   return 0;
}
