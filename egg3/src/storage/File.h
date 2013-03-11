/**
   \file File.h
   \brief 文件操作基础模块
   \par
   \ingroup storage
*/
#ifndef _EGG_FILE_H_
#define _EGG_FILE_H_

#include "../EggDef.h"
#include "eggRecoveryLog.h"



E_BEGIN_DECLS


/*!
  \typedef void* HEGGFILE
  文件句柄
*/
typedef void* HEGGFILE;

/*!
  \typedef struct tagEggFile EGGFILE
  文件句柄
*/

typedef struct tagEggFile EGGFILE;

/*
  \struct tagEggFile
  \brief 文件结构体
*/

struct tagEggFile
{
    EHANDLE hFile;
    size64_t size;
    char *name;
    
    ActInfo *hActInfo;
    HEGGRECOVERYHANDLE hEggRecoveryHandle;
};


/*!
  \fn HEGGFILE EggFile_open(const path_t*)
  \brief 文件打开函数
  \param filepath 文件路径
  \return 文件句柄实例
*/
extern HEGGFILE EggFile_open(const path_t* filepath);

/*!
  \fn EBOOL EggFile_close(HEGGFILE)
  \brief 文件关闭函数
  \param hEggFile 文件句柄实例
  \return
*/
extern EBOOL EggFile_close(HEGGFILE hEggFile);

/*!
  \fn EBOOL EggFile_read(HEGGFILE , epointer , length_t , offset64_t)
  \brief 文件读取操作函数
  \param hEggFile 文件句柄实例
  \param ePointer 读取内容存放的地址
  \param nLen 读取内容的长度
  \param nOff 文件中的偏移地址
  \return
*/
extern EBOOL EggFile_read(HEGGFILE hEggFile, epointer ePointer, length_t nLen, offset64_t nOff);

/*!
  \fn EBOOL EggFile_write(HEGGFILE, ecpointer, length_t, offset64_t)
  \brief 文件写入操作函数
  \param hEggFile 文件句柄实例
  \param ecPointer 写入内容的地址
  \param nLen 要写入内容的长度
  \param nOff 文件中的偏移地址
  \return
*/
extern EBOOL EggFile_write(HEGGFILE hEggFile, ecpointer ecPointer, length_t nLen, offset64_t nOff);

/*!
  \fn EBOOL EggFile_update(HEGGFILE)
  \brief
  \param hEggFile
  \return
*/
extern EBOOL EggFile_update(HEGGFILE hEggFile);


/*!
  \fn EBOOL EggFile_lock_wr_wait(HEGGFILE , short , off_t , off_t )
  \brief 文件锁函数(阻塞写锁)
  \param hEggFile 文件句柄实例
  \param whence 文件锁的起始方式
  \param start 文件锁的起始位置
  \param len 文件锁的起始长度
  \return
*/

PUBLIC EBOOL EggFile_lock_wr_wait(HEGGFILE hEggFile, short whence, off_t start, off_t len);

/*!
  \fn EBOOL EggFile_lock_wr_try(HEGGFILE , short , off_t , off_t )
  \brief 文件锁函数(非阻塞写锁)
  \param hEggFile 文件句柄实例
  \param whence 文件锁的起始方式
  \param start 文件锁的起始位置
  \param len 文件锁的起始长度
  \return
*/
PUBLIC EBOOL EggFile_lock_wr_try(HEGGFILE hEggFile, short whence, off_t start, off_t len);
/*!
  \fn EBOOL EggFile_lock_rd_wait(HEGGFILE , short , off_t , off_t )
  \brief 文件锁函数(阻塞读锁)
  \param hEggFile 文件句柄实例
  \param whence 文件锁的起始方式
  \param start 文件锁的起始位置
  \param len 文件锁的起始长度
  \return
*/
PUBLIC EBOOL EggFile_lock_rd_wait(HEGGFILE hEggFile, short whence, off_t start, off_t len);

/*!
  \fn EBOOL EggFile_lock_rd_try(HEGGFILE , short , off_t , off_t )
  \brief 文件锁函数(非阻塞读锁)
  \param hEggFile 文件句柄实例
  \param whence 文件锁的起始方式
  \param start 文件锁的起始位置
  \param len 文件锁的起始长度
  \return
*/
PUBLIC EBOOL EggFile_lock_rd_try(HEGGFILE hEggFile, short whence, off_t start, off_t len);

/*!
  \fn EBOOL EggFile_unlock(HEGGFILE , short , off_t , off_t )
  \brief 文件锁函数(解锁)
  \param hEggFile 文件句柄实例
  \param whence 文件锁的起始方式
  \param start 文件锁的起始位置
  \param len 文件锁的起始长度
  \return
*/
PUBLIC EBOOL EggFile_unlock(HEGGFILE hEggFile, short whence, off_t start, off_t len);

/*!
  \fn size64_t EggFile_size(HEGGFILE )
  \brief 获取文件大小
  \param hEggFile 文件句柄实例
  \return
*/
extern size64_t EggFile_size(HEGGFILE hEggFile);


/*!
  \fn char *EggFile_name(HEGGFILE)
  \brief 获取文件名字（绝对路径）
  \param hEggFile 文件句柄实例
  \return
*/
extern char *EggFile_name(HEGGFILE hEggFile);

/*!
  \def EggFile_is_object(hEggFile)
  \brief
*/
#define EggFile_is_object(hEggFile) \
    ((hEggFile)?EGG_TRUE:EGG_FALSE)

/*!
  \def EggFile_object(_MODULEPTR)
  \brief
*/
#define EggFile_object(_MODULEPTR) \
    ((EGGFILE*)_MODULEPTR)


/*!
  \fn EBOOL EggFile_startlog(HEGGFILE )
  \brief 日志记录起始点
  \param hEggFile 文件句柄实例
  \return
*/

extern EBOOL EggFile_startlog(HEGGFILE hEggFile);

/*!
  \fn EBOOL EggFile_endlog(HEGGFILE )
  \brief 日志记录结束点
  \param hEggFile 文件句柄实例
  \return
*/

extern EBOOL EggFile_endlog(HEGGFILE hEggFile);

extern ActInfo *EggFile_get_actinfo(HEGGFILE hEggFile);
extern EBOOL EggFile_set_actinfo(HEGGFILE hEggFile,
                                ActInfo *hActInfo);
extern HEGGRECOVERYHANDLE EggFile_get_recoveryhandle(HEGGFILE hEggFile);

E_END_DECLS

#endif //_EGG_FILE_H_
