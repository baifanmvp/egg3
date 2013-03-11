/**
   \file ViewStream.h
   \brief 文件流模块
   \par
   \ingroup storage
*/
#ifndef _VIEW_STREAM_H_
#define _VIEW_STREAM_H_
#include "../EggDef.h"
#include "./File.h"
#include "./Cluster.h"
#include "./eggRecoveryLog.h"




E_BEGIN_DECLS


#define MAP_FILE_MAP      0x0001
#define MAP_FILE_NOCACHE  0x0002

/*!
  \def MAP_VIEW_OFFSET
  \brief 流映射的偏移量
*/
#define MAP_VIEW_OFFSET 0x10000


typedef struct {
    offset64_t pos;
    offset64_t len;
} eggPosLen;

/*!
  \typedef HVIEWSTREAM
  \brief 映射流句柄
*/
//typedef _MODULEPTR HVIEWSTREAM;
typedef _MODULEPTR HVIEWSTREAM;
typedef struct tagViewStream VIEWSTREAM;
/*
  \struct tagViewStream
  \brief 映射流结构体
*/
struct tagViewStream
{
    HEGGFILE hEggFile;
#ifdef WIN32
    HANDLE hMappingFile;
#endif
    vstream_t* mapView;

    size32_t sizeOfMapView;
    offset64_t offOfMapView;

    size32_t szFreeArea;
    void *pFreeArea;
    HEGGFILE freeAreaFile;
    char *nameFreeArea;
    eggPosLen *suspendAlloc;
    eggPosLen *suspendFree;
};


/*!
  \fn HVIEWSTREAM ViewStream_new(HEGGFILE)
  \brief 映射流初始化
  \param hEggFile 文件句柄
  \return 映射流句柄实例
*/
extern HVIEWSTREAM ViewStream_new(HEGGFILE hEggFile);

/*!
  \fn EBOOL ViewStream_delete(HVIEWSTREAM)
  \brief 销毁映射流
  \param hViewStream 映射流句柄实例
  \return Rev.
*/
extern EBOOL ViewStream_delete(HVIEWSTREAM hViewStream);

/*!
  \fn EBOOL ViewStream_read(HVIEWSTREAM, epointer, size32_t, offset64_t)
  \brief 映射流读操作 有锁
  \param hViewStream 映射流句柄
  \param ePointer 读操作的指针
  \param nSize 读操作的长度
  \param nOffset 映射流空间中的偏移地址
  \return Rev.
*/
extern EBOOL ViewStream_read(HVIEWSTREAM hViewStream, epointer ePointer, size32_t nSize, offset64_t nOffset);

extern EBOOL ViewStream_read_nolock(HVIEWSTREAM hViewStream, epointer ePointer, size32_t nSize, offset64_t nOffset);
extern EBOOL ViewStream_read_info(HVIEWSTREAM hViewStream, epointer ePointer, size16_t nSize, offset16_t nOffset);

/*!
  \fn EBOOL ViewStream_update(HVIEWSTREAM, ecpointer, size32_t, offset64_t)
  \brief 映射流更新操作 有锁
  \param hViewStream 映射流句柄
  \param ecPointer 操作指针
  \param nSize 操作长度
  \param nOffset 映射流空间中的偏移地址
  \return Rev.
*/
extern EBOOL ViewStream_update(HVIEWSTREAM hViewStream, epointer ePointer, size32_t nSize, offset64_t nOffset);


extern EBOOL ViewStream_update_nolock(HVIEWSTREAM hViewStream, epointer ePointer, size32_t nSize, offset64_t nOffset);
/*!
  \fn EBOOL ViewStream_update_info(HVIEWSTREAWM, ecpointer, size16_t, offset16_t)
  \brief 更新映射流的信息区 有锁
  \param hViewStream 映射流句柄
  \param ecPointer 操作指针
  \param nSize 操作长度
  \param nOffset 映射流空间中的偏移地址
  \return Rev.
*/
extern EBOOL ViewStream_update_info(HVIEWSTREAM hViewStream, ecpointer ecPointer, size16_t nSize, offset16_t nOffset);


/*!
  \fn offset64_t ViewStream_write(HVIEWSTREAM, ecpointer, size32_t)
  \brief 映射流写操作 有锁
  \param hViewStream 映射流句柄
  \param ecPointer 操作指针
  \param nSize 操作长度
  \return Rev.
*/
extern offset64_t ViewStream_write(HVIEWSTREAM hViewStream, epointer ePointer, size32_t nSize);

extern EBOOL ViewStream_free_area(HVIEWSTREAM hViewStream, offset64_t nOffset, size32_t nSize);


extern offset64_t ViewStream_write_nolock(HVIEWSTREAM hViewStream, epointer ePointer, size32_t nSize);
    

//extern EBOOL ViewStream_lock(HVIEWSTREAM hViewStream, offset64_t off64FileMap, size32_t nLen);

//extern EBOOL ViewStream_unlock(HVIEWSTREAM hViewStream, offset64_t off64FileMap, size32_t nLen);

extern EBOOL ViewStream_xlock(HVIEWSTREAM hViewStream, offset64_t off64FileOff, size32_t nLen);
extern EBOOL ViewStream_slock(HVIEWSTREAM hViewStream, offset64_t off64FileOff, size32_t nLen);
extern EBOOL ViewStream_xlock_try(HVIEWSTREAM hViewStream, offset64_t off64FileOff, size32_t nLen);
extern EBOOL ViewStream_slock_try(HVIEWSTREAM hViewStream, offset64_t off64FileOff, size32_t nLen);
extern EBOOL ViewStream_unlock(HVIEWSTREAM hViewStream, offset64_t off64FileOff, size32_t nLen);

extern EBOOL ViewStream_desired_access(HVIEWSTREAM hViewStream, type_t nDesiredAccess);

PUBLIC EBOOL ViewStream_check_offset(HVIEWSTREAM hViewStream, offset64_t off);

extern epointer ViewStream_location(HVIEWSTREAM hViewStream,  size32_t nSize, offset64_t nOffset);

extern EBOOL ViewStream_remap(HVIEWSTREAM hViewStream);

#define ViewStream_is_object(hViewStream) \
    ((hViewStream)?EGG_TRUE:EGG_FALSE)

#define ViewStream_file_size(hViewStream) \
    ((hViewStream)?EggFile_size(((VIEWSTREAM*)(hViewStream))->hEggFile):0)

#define ViewStream_view(hViewStream) \
    ((hViewStream)?(hViewStream->mapView):EGG_NULL)

extern EBOOL ViewStream_startlog(HVIEWSTREAM hViewStream);
extern EBOOL ViewStream_endlog(HVIEWSTREAM hViewStream);

extern EBOOL ViewStream_set_actinfo(HVIEWSTREAM hViewStream,
                                   ActInfo *hActInfo);
extern ActInfo *ViewStream_get_actinfo(HVIEWSTREAM hViewStream);
extern EBOOL ViewStream_clean_actinfo(HVIEWSTREAM hViewStream,
                                     ActInfo *hActInfo);
extern HEGGRECOVERYHANDLE ViewStream_get_recoveryhandle(HVIEWSTREAM hViewStream);
    
char *ViewStream_name(HVIEWSTREAM hViewStream);
E_END_DECLS
    
#endif //_VIEW_STREAM_H_

