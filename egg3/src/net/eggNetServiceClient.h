#ifndef EGGNETSERVICECLIENT_H_
#define EGGNETSERVICECLIENT_H_
#include "../interface/eggServiceClientConfig.h"
#include "eggNetSocket.h"

struct eggNetServiceClient;
typedef struct eggNetServiceClient eggNetServiceClient;
typedef struct eggNetServiceClient *HEGGNETSERVICECLIENT;


PUBLIC HEGGNETSERVICECLIENT eggNetServiceClient_new(HEGGSERVICECLIENTCONFIG hEggServiceClientConfig);

PUBLIC EBOOL eggNetServiceClient_connect(HEGGNETSERVICECLIENT hEggNetServiceClient);

PUBLIC EBOOL eggNetServiceClient_send(HEGGNETSERVICECLIENT hEggNetServiceClient, epointer ePointer, size32_t size, int flags);

PUBLIC EBOOL eggNetServiceClient_recv(HEGGNETSERVICECLIENT hEggNetServiceClient, epointer* ePointer, size32_t* lpSize, int flags);

PUBLIC EBOOL eggNetServiceClient_delete(HEGGNETSERVICECLIENT hEggNetServiceClient);

PUBLIC EBOOL eggNetServiceClient_restart_network(HEGGNETSERVICECLIENT hEggNetServiceClient);

PUBLIC EBOOL eggNetServiceClient_delete(HEGGNETSERVICECLIENT);

#endif
