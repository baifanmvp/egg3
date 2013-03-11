#ifndef EGG_NETHTTP_H
#define EGG_NETHTTP_H
#include "eggNetSocket.h"
E_BEGIN_DECLS

#define EGG_HTTP_HEAD \
    "%s /%s HTTP/1.1\r\nHost: %s\r\nContent-Type: application/soap+xml;charset=utf-8\r\nContent-Length: %d\r\n\r\n"

typedef struct eggNetHttp EGGNETHTTP;
typedef struct eggNetHttp* HEGGNETHTTP;

struct eggNetHttp
{
    HEGGNETSOCKET hSocket;
    char* host;
    char* page;
    char* ty; //POST or GET
    int port;
    
};

PUBLIC HEGGNETHTTP eggNetHttp_new(char* url, char* ty);

PUBLIC EBOOL eggNetHttp_connect(HEGGNETHTTP hNetHttp);

PUBLIC EBOOL eggNetHttp_send(HEGGNETHTTP hNetHttp, epointer ePointer, size32_t size, int flags);

PUBLIC EBOOL eggNetHttp_recv(HEGGNETHTTP hNetHttp, epointer* ePointer, size32_t* lpSize, int flags);

PUBLIC EBOOL eggNetHttp_delete(HEGGNETHTTP hNetHttp);

PUBLIC EBOOL eggNetHttp_restart_network(HEGGNETHTTP hNetHttp);

E_END_DECLS



#endif
