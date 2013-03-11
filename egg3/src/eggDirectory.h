/**
   \file eggDirectory.h
   \brief 文件目录 (API)
   \ingroup egg
*/
#ifndef EGG_DIRECTORY_H_
#define EGG_DIRECTORY_H_

#include "EggDef.h"
#include "storage/File.h"

E_BEGIN_DECLS

typedef struct eggDirectory EGGDIRECTORY;
typedef struct eggDirectory* HEGGDIRECTORY;


/*!
  \fn HDIRECTORY EGGAPI egg_Directory_open(const path_t*)
  \brief 目录打开函数
  \param filepath 文件目录字符串
  \return 文件目录句柄
*/
HEGGDIRECTORY EGGAPI eggDirectory_open(const path_t* filepath);


/*!
  \fn EBOOL EGGAPI egg_Directory_close(HDIRECTORY)
  \brief 目录关闭函数
  \param hDirectory 文件目录句柄
  \return
*/
EBOOL EGGAPI eggDirectory_close(HEGGDIRECTORY hEggDirectory);


path_t* EGGAPI eggDirectory_get_name(HEGGDIRECTORY hEggDirectory);


/*!
  \fn EGGFILE EGGAPI egg_Directory_get_file(hDirectory)
  \brief
  \param hDirectory
  \return
*/
EBOOL EGGAPI eggDirectory_init(HEGGDIRECTORY hEggDirectory);

HEGGFILE EGGAPI eggDirectory_get_file(HEGGDIRECTORY hEggDirectory, const echar* lpFileName);

HEGGDIRECTORY eggDirectory_dup(HEGGDIRECTORY hEggDirectory);
EBOOL eggDirectory_delete(HEGGDIRECTORY hEggDirectory);

HEGGDIRECTORY EGGAPI eggDirectory_load_path(const path_t* filepath);

E_END_DECLS

#endif //_EGG_DIRECTORY_H_

