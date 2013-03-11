#ifndef EGG_NETSOCKET_H
#define EGG_NETSOCKET_H

#include "../EggDef.h"

#include <sys/socket.h>
#include <netdb.h>


E_BEGIN_DECLS

typedef struct eggNetSocket EGGNETSOCKET;
typedef struct eggNetSocket* HEGGNETSOCKET;

struct eggNetSocket
{
    esock_t sockId;
};

HEGGNETSOCKET eggNetSocket_new(int damin, int type, int protocol);

EBOOL eggNetSocket_connect(HEGGNETSOCKET hNetSocket, struct sockaddr* addr, socklen_t alen);

EBOOL eggNetSocket_connect_byhost(HEGGNETSOCKET hNetSocket, char* ip, short port);

EBOOL eggNetSocket_send(HEGGNETSOCKET hNetSocket, epointer ePointer, size32_t size, int flags);

EBOOL eggNetSocket_recv(HEGGNETSOCKET hNetSocket, epointer ePointer, size32_t size, int flags);

EBOOL eggNetSocket_delete(HEGGNETSOCKET hNetSocket);


E_END_DECLS

#endif
