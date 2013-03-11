#include "./eggKeySet.h"

PRIVATE EBOOL eggKeySet_file_build(char* lpPath, char* lpName);

HEGGKEYSET eggKeySet_new(char* path)
{
    return EGG_NULL;
}

EBOOL eggKeySet_delete(HEGGKEYSET hKeySet)
{
    
}


PRIVATE EBOOL eggKeySet_file_build(char* lpPath, char* lpName)
{
    echar rgsz_file_path[2048] = {0};

    sprintf(rgsz_file_path, "%s%s", lpPath, lpName);
    
    if(access(rgsz_file_path, F_OK) != 0)
    {
        file_t fid = open(rgsz_file_path, O_RDWR | O_CREAT, 00644);

        if (fid < 0)
        {
            fprintf(stderr, "[%s] [%s][%s]\n", __func__, strerror(errno), rgsz_file_path);
        }

        epointer lp_init_tmp = (epointer)malloc(MAP_VIEW_OFFSET);
        memset(lp_init_tmp, 0, MAP_VIEW_OFFSET);
        lseek64(fid, 0, SEEK_SET);
        size16_t nRet = write(fid, lp_init_tmp, MAP_VIEW_OFFSET);
        
        if (nRet < 0)
        {
            fprintf(stderr, "[%s] [%s]\n", __func__, strerror(errno));
        }
        
        free(lp_init_tmp);

        
        close(fid);
    }
}
