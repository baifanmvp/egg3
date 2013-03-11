/**
   \file eggServiceClient.h
   \brief net文件目录 (API)
   \ingroup egg
*/
#ifndef EGG_SERVICECLIENT_H_
#define EGG_SERVICECLIENT_H_

#include "EggDef.h"

E_BEGIN_DECLS


typedef struct eggServiceClient EGGSERVICECLIENT;
typedef struct eggServiceClient* HEGGSERVICECLIENT;

/*!
  \fn HHTTP EGGAPI eggServiceClient_open(const path_t*)
  \param name "::configfile:/path/" or "ip:port/path/"
  \brief 服务打开函数
*/
HEGGSERVICECLIENT EGGAPI eggServiceClient_open(const char* name);

HEGGSERVICECLIENT eggServiceClient_dup(HEGGSERVICECLIENT hEggServiceClient);

/*!
  \fn EBOOL EGGAPI eggServiceClient_close(hEggServiceClient)
  \brief 关闭函数
  \return
*/
EBOOL EGGAPI eggServiceClient_close(HEGGSERVICECLIENT hEggServiceClient);

char* EGGAPI eggServiceClient_get_name(HEGGSERVICECLIENT hEggServiceClient);
char* EGGAPI eggServiceClient_get_eggdirpathname(HEGGSERVICECLIENT hEggServiceClient);
char* EGGAPI eggServiceClient_get_configfile(HEGGSERVICECLIENT hEggServiceClient);

PUBLIC EBOOL eggServiceClient_change_eggdirpath(HEGGSERVICECLIENT hEggServiceClient, char *eggDirPath);

E_END_DECLS

#endif //EGG_SERVICECLIENT_H_

