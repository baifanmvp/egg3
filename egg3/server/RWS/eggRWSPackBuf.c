#include "eggRWSPackBuf.h"
#define EGG_PAK_FILE "eggBuf.pak"
HEGGRWSPACKBUF eggRWSPackBuf_new(char* fdir)
{
    if(POINTER_IS_INVALID(fdir))
    {
        return EGG_NULL;
    }
    char path[1024];
    HEGGRWSPACKBUF lp_packbuf = (HEGGRWSPACKBUF)malloc(sizeof(EGGRWSPACKBUF));
    memset(lp_packbuf, 0, sizeof(EGGRWSPACKBUF));
    
    sprintf(path, "%s/%s", fdir, EGG_PAK_FILE);
    lp_packbuf->fd =  open(path, O_RDWR | O_CREAT, 0644) ;
    lp_packbuf->fpath = strdup(path);
    
    struct stat st_file_stat;
    fstat(lp_packbuf->fd, &st_file_stat);
    lp_packbuf->fsize = st_file_stat.st_size;
    
    return lp_packbuf;
    
}
EBOOL eggRWSPackBuf_delete(HEGGRWSPACKBUF hRWSPackBuf)
{
    if(POINTER_IS_INVALID(hRWSPackBuf))
    {
        return EGG_FALSE;
    }
    
    close(hRWSPackBuf->fd);
    free(hRWSPackBuf->fpath);
    free(hRWSPackBuf);
    
    return EGG_TRUE;
}


EBOOL eggRWSPackBuf_setOff(HEGGRWSPACKBUF hRWSPackBuf, offset64_t off)
{
    if(POINTER_IS_INVALID(hRWSPackBuf))
    {
        return EGG_FALSE;
    }
    
    hRWSPackBuf->off = off;
    lseek(hRWSPackBuf->fd, hRWSPackBuf->off, SEEK_SET);
    
    return EGG_TRUE;
}

offset64_t eggRWSPackBuf_getOff(HEGGRWSPACKBUF hRWSPackBuf)
{
    if(POINTER_IS_INVALID(hRWSPackBuf))
    {
        return EGG_FALSE;
    }
    
    return hRWSPackBuf->off;
}
size64_t eggRWSPackBuf_get_fsize(HEGGRWSPACKBUF hRWSPackBuf)
{
    if(POINTER_IS_INVALID(hRWSPackBuf))
    {
        return EGG_FALSE;
    }
    
    return hRWSPackBuf->fsize;
}

HEGGNETPACKAGE eggRWSPackBuf_fetch(HEGGRWSPACKBUF hRWSPackBuf)
{
    if(POINTER_IS_INVALID(hRWSPackBuf))
    {
        return EGG_NULL;
    }

    if(hRWSPackBuf->fsize < hRWSPackBuf->off + sizeof(struct eggNetPackage))
    {
        return EGG_NULL;
    }
    
    struct eggNetPackage st_pack_info = {0};
    read(hRWSPackBuf->fd, (HEGGNETPACKAGE)&st_pack_info, sizeof(struct eggNetPackage));
    hRWSPackBuf->off += sizeof(struct eggNetPackage);
    HEGGNETPACKAGE lp_pack_res = (HEGGNETPACKAGE)malloc(sizeof(EGGNETPACKAGE) + st_pack_info.eSize);
    
    memcpy(lp_pack_res, (HEGGNETPACKAGE)&st_pack_info, sizeof(EGGNETPACKAGE));
    
    if(hRWSPackBuf->fsize < hRWSPackBuf->off + st_pack_info.eSize)
    {
        free(lp_pack_res);
        return EGG_NULL;
    }
    
    read(hRWSPackBuf->fd, lp_pack_res+1, st_pack_info.eSize);
    hRWSPackBuf->off += st_pack_info.eSize;
    return lp_pack_res;
    
}

EBOOL eggRWSPackBuf_add(HEGGRWSPACKBUF hRWSPackBuf, HEGGNETPACKAGE hNetPackage)
{
    if(POINTER_IS_INVALID(hRWSPackBuf))
    {
        return EGG_FALSE;
    }
    
    if(POINTER_IS_INVALID(hNetPackage))
    {
        return EGG_FALSE;
    }
    
    write(hRWSPackBuf->fd, hNetPackage, sizeof(struct eggNetPackage) + hNetPackage->eSize);
    
    hRWSPackBuf->off += sizeof(struct eggNetPackage) + hNetPackage->eSize;
    
    if(hRWSPackBuf->off > hRWSPackBuf->fsize)
    {
        hRWSPackBuf->fsize = hRWSPackBuf->off;
    }
    
    return EGG_TRUE;
}

