#include <egg3/Egg3.h>
#define DATA_FILE_SIZE 50*1024*1024
#define DATA_FILE_NAME "doc.data"
typedef struct eggDocWeight
{
  count_t cnt;
}EGGDOCWEIGHT;
int main(int argc, char** argv)
{

  if(argc <  3)
    {
      printf("argc error !\n");
      exit(-1);
    }

  if(access(argv[1], F_OK) != 0)
    {
      printf("path error !\n");
      exit(-1);
    }
  EGGDOCWEIGHT doc_weight[15000] = {0};
  HEGGHANDLE hOrgHandle = eggPath_open(argv[1]);
  char* lp_spanfield = argv[2];

        
  HEGGINDEXREADER hOrgReader = eggIndexReader_open(hOrgHandle);


  offset64_t n_cursor = 1;
  HEGGDOCUMENT lp_eggDocument = EGG_NULL;

  int n_file_num = 1;
  int n_file_size = DATA_FILE_SIZE;
  char file_path[1024];
  count_t count = 0;
  int optimizenum = 10000;

  while(lp_eggDocument = eggIndexReader_export_document(hOrgReader, &n_cursor))
  {
      
       HEGGFIELD lp_field = eggDocument_get_field(lp_eggDocument, lp_spanfield);
       int len = 0;
       if(lp_field) 
	 {
	   int idx = *(int*)eggField_get_value(lp_field, &len);
	   doc_weight[idx].cnt ++;
	 }
      eggDocument_delete(lp_eggDocument);
      
      count++;

      if (count % optimizenum == 0)
      {
          
          printf("count : %d\n", count);
      }
      
  }
  int i = 0;
  while(i < 15000)
    {
      printf("idx : %d, cnt : %d\n", i, doc_weight[i].cnt);
      i++;
    }
  eggIndexReader_close(hOrgReader);
  eggPath_close(hOrgHandle );

  return 0;
}
