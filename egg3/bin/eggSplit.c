#include <egg3/Egg3.h>
#define DATA_FILE_SIZE 50*1024*1024
#define DATA_FILE_NAME "doc.data"

int main(int argc, char** argv)
{

  if(argc <  4)
    {
      printf("argc error !\n");
      exit(-1);
    }

  if(access(argv[1], F_OK) != 0)
    {
      printf("path error !\n");
      exit(-1);
    }
  HEGGHANDLE hOrgHandle = eggPath_open(argv[1]);
  HEGGHANDLE hDestHandle = eggPath_open(argv[2]);
  char* lp_spanfield = argv[3];

        
  HEGGINDEXREADER hOrgReader = eggIndexReader_open(hOrgHandle);

  HEGGINDEXWRITER hDestWriter = eggIndexWriter_open(hDestHandle, lp_spanfield);

  offset64_t n_cursor = 1;
  HEGGDOCUMENT lp_eggDocument = EGG_NULL;

  int n_file_num = 1;
  int n_file_size = DATA_FILE_SIZE;
  char file_path[1024];
  count_t count = 0;
  int optimizenum = 1000;
  if(argc ==  5)
    {
      n_cursor = atoi(argv[4]);
    }
  if(argc ==  6)
    {
      optimizenum = atoi(argv[5]);
    }

  while(lp_eggDocument = eggIndexReader_export_document(hOrgReader, &n_cursor))
  {
      
      /* HEGGFIELD lp_field = eggDocument_get_field(lp_eggDocument, "content"); */
      /* unsigned int len = 0; */
            
      /* if(lp_field) */
      /*     printf("count : [%d] content : %s\n", count, eggField_get_value(lp_field, &len)); */
    //      HEGGFIELD lp_field = eggDocument_removeField_byName(lp_eggDocument, "content");
    // HEGGDOCUMENT lp_newDocument = eggDocument_new();
    // eggDocument_add(lp_newDocument, lp_field);
      eggIndexWriter_add_document(hDestWriter, lp_eggDocument);
      eggDocument_delete(lp_eggDocument);
      //eggDocument_delete(lp_newDocument);
      
      count++;

      if (count % optimizenum == 0)
      {
          
          eggIndexWriter_optimize(hDestWriter);
          printf("count : %d\n", count);
      }
      
  }
  
  eggIndexWriter_optimize(hDestWriter);
  eggIndexWriter_close(hDestWriter);
  eggIndexReader_close(hOrgReader);
  eggPath_close(hOrgHandle );
  if(hDestHandle)
  eggPath_close(hDestHandle );
  return 0;
}
