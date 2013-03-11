#include "egg3/Egg3.h"


#include <stdio.h>
#include <time.h>
#include <sys/time.h>




int main(int argc, char* argv[])
{
    char* dir_path = argv[1];
    struct timeval tv_start, tv_end;
    int i = 0;
    
    ImLexAnalyzer* p_la = (ImLexAnalyzer*)ImCwsLexAnalyzer_new(0);
    
    gettimeofday(&tv_start, EGG_NULL);
    HEGGDIRECTORY hDirectory = eggDirectory_open(dir_path);
    
    HEGGINDEXWRITER hIndexWrite = eggIndexWriter_open(hDirectory,  p_la);
    FILE* fp_file = fopen("kfc.txt", "r+");

    char *buf = EGG_NULL;
    size_t fileSize = 0;
    int count = 0;
    char databuf[4096];
    while (getline(&buf, &fileSize, fp_file) != -1 && count < 500000)
    {
        count++;
        if(count%1001 == 0)
        {
            //    printf("count : %d\n", count);
        }
        if (count%100001 == 0 )
        {
              printf("count : %d\n", count);
//              break;
//              eggIndexWriter_optimize(hIndexWrite);
        }

        HEGGDOCUMENT hDocument = eggDocument_new();
        
//        sprintf(databuf, "body : %s", buf);
        HEGGFIELD hField1 = eggField_new("body", buf, strlen(buf), EGG_CN_ANALYZED | EGG_INDEX_STRING | EGG_STORAGE);
        //HEGGFIELD hField2 = eggField_new("price", (char*)&count, sizeof(count), EGG_NOT_ANALYZED | EGG_INDEX_INT32 | EGG_STORAGE);
        eggDocument_add(hDocument, hField1);

        HEGGFIELD hField2 = eggField_new("title", "helloworld", strlen("helloworld"), EGG_NOT_ANALYZED | EGG_INDEX_STRING | EGG_STORAGE);
        //HEGGFIELD hField2 = eggField_new("price", (char*)&count, sizeof(count), EGG_NOT_ANALYZED | EGG_INDEX_INT32 | EGG_STORAGE);
//        eggDocument_add(hDocument, hField1);

      eggDocument_add(hDocument, hField2);
        // eggDocument_add(hDocument, hField3);
        
        eggIndexWriter_add_document(hIndexWrite, hDocument);
    
    
        eggDocument_delete(hDocument);
//        break;
    }
        
   eggIndexWriter_optimize(hIndexWrite);
        
        
    eggIndexWriter_close(hIndexWrite);
    eggDirectory_close(hDirectory);
    fclose(fp_file);
    gettimeofday(&tv_end, EGG_NULL);
    ImLexAnalyzer_delete(p_la);
    printf("time : %f\n", (tv_end.tv_sec - tv_start.tv_sec) + (double)(tv_end.tv_usec - tv_start.tv_usec)/1000000 );
    return ;
}

