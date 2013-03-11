#include <egg3/Egg3.h>
int main()
{
   HEGGHANDLE hEggHandle =  eggPath_open("file:///tmp/");
   HEGGINDEXWRITER hIndexWriter = eggIndexWriter_open(hEggHandle,  "");
   
   EGGDID did;
   EGGDID_DOCID(&did) = 2;
   printf("set document to delete \n");
   EBOOL ret = eggIndexWriter_delete_document(hIndexWriter, did);
   if(ret == EGG_FALSE)
   {
   // modify false
       printf("delete false !\n");
   }
   else if(ret == EGG_TRUE)
   {
   // modify ture
       printf("delete ture !\n");
   }
   
   eggIndexWriter_optimize(hIndexWriter);
   
   eggIndexWriter_close(hIndexWriter);
   
   eggPath_close(hEggHandle);
   return 0;
}
