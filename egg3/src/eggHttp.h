/**
   \file eggHttp.h
   \brief net文件目录 (API)
   \ingroup egg
*/
#ifndef EGG_HTTP_H_
#define EGG_HTTP_H_

#include "EggDef.h"

E_BEGIN_DECLS


typedef struct eggHttp EGGHTTP;
typedef struct eggHttp* HEGGHTTP;

/*!
  \fn HHTTP EGGAPI egg_Http_open(const path_t*)
  \brief 目录打开函数
  \param filepath 文件目录字符串
  \return 文件目录句柄
*/
HEGGHTTP EGGAPI eggHttp_open(const char* url);

HEGGHTTP eggHttp_dup(HEGGHTTP hEggHttp);

/*!
  \fn EBOOL EGGAPI egg_Http_close(HHTTP)
  \brief 目录关闭函数
  \param hHttp 文件目录句柄
  \return
*/
EBOOL EGGAPI eggHttp_close(HEGGHTTP hEggHttp);

char* EGGAPI eggHttp_get_name(HEGGHTTP hEggHttp);

E_END_DECLS

#endif //EGG_HTTP_H_

