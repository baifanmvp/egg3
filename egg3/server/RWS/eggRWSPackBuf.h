#ifndef EGG_RWS_PACKBUF_H
#define EGG_RWS_PACKBUF_H
#include <egg3/Egg3.h>
typedef struct eggRWSPackBuf  EGGRWSPACKBUF;
typedef struct eggRWSPackBuf* HEGGRWSPACKBUF;

struct eggRWSPackBuf
{
    int fd;
    char* fpath;
    size64_t fsize;
    offset64_t off;
};

HEGGRWSPACKBUF eggRWSPackBuf_new(char* fpath);

EBOOL eggRWSPackBuf_delete(HEGGRWSPACKBUF hRWSPackBuf);

EBOOL eggRWSPackBuf_setOff(HEGGRWSPACKBUF hRWSPackBuf, offset64_t off);

offset64_t eggRWSPackBuf_getOff(HEGGRWSPACKBUF hRWSPackBuf);

size64_t eggRWSPackBuf_get_fsize(HEGGRWSPACKBUF hRWSPackBuf);

HEGGNETPACKAGE eggRWSPackBuf_fetch(HEGGRWSPACKBUF hRWSPackBuf);

EBOOL eggRWSPackBuf_add(HEGGRWSPACKBUF hRWSPackBuf, HEGGNETPACKAGE hNetPackage);

#endif
