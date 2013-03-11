/**
   \file EggDef.h
   \brief
   \defgroup egg
   \ingroup egg
*/
#ifndef _EGG_DEFINE_H_
#define _EGG_DEFINE_H_
#include "./EggError.h"
#include "./eggDebug.h"
#include "./egglib/etype.h"
#ifndef WIN32
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/sysinfo.h>
#include <errno.h>
#endif

#define MAP_VIEW_OFFSET 0x10000


#ifndef _EGG_OFFSET_TYPE_DEFINE
#define _EGG_OFFSET_TYPE_DEFINE
typedef u8  offset8_t;
typedef u16 offset16_t;
typedef u32 offset32_t;
typedef u64 offset64_t;
typedef offset32_t offset_t;
//typedef offset32_t off_t;
#endif //_EGG_OFFSET_TYPE_DEFINE

#ifndef _EGG_POSITION_TYPE_DEFINE
#define _EGG_POSITION_TYPE_DEFINE
typedef u8  position8_t;
typedef u16 position16_t;
typedef u32 position32_t;
typedef u64 position64_t;
typedef position32_t position_t;
typedef position32_t pos_t;
#endif //_EGG_POSITION_TYPE_DEFINE

#ifndef _EGG_SIZE_TYPE_DEFINE
#define _EGG_SIZE_TYPE_DEFINE
typedef u8  size8_t;
typedef u16 size16_t;
typedef u32 size32_t;
typedef u64 size64_t;
#endif //_EGG_SIZE_TYPE_DEFINE



typedef echar mkey_t;
typedef echar document_t;
typedef echar path_t;
typedef echar vstream_t;
typedef echar field_t;
typedef echar query_t;
typedef echar array_t;
typedef echar section_t;
typedef echar ebyte;

typedef i16 type_t;
typedef i16 eop_t;
typedef i32 index_t;
typedef u32 count_t;
typedef u32 second_t;
//typedef u32 tag_t;
typedef u32 length_t;
typedef u32 sort_t;
typedef u32 access_t;
typedef u32 page_t;
typedef u32 flag_t;

typedef u64 did_t;
typedef u64 fdid_t;

typedef int file_t;
typedef int esig_t;
typedef int esock_t;

typedef double score_t;

#define STRUCT_POS(type, member)  ((&( ((type *)0)->member)))
#define struct_offset(type, member)  ((offset64_t)(&( ((type *)0)->member)))
#define POINTER_IS_INVALID(pointer) ((!pointer) ? 1 : 0)


#define  EGG_COMPOSE_HTTPPATH(httpPath, hostName, localPath)            \
    do                                                                  \
    {                                                                   \
        sprintf((httpPath), "%s/cgi-bin/eggChunk.fcgi?EGG_DIR_PATH=%s", \
                (hostName), (localPath));                               \
    }while(EGG_FALSE);


#define DID_SPACE_COUNT 100000
#define E_VALUE (2.71828183)
#define EGG_NULL   0
#define EGG_FALSE 0x0000
#define EGG_TRUE   0x0001
#define EGG_PATH_ERROR   0x0002
#define EGG_SYSCALL_TRUE   0x0000
#define EGG_SYSCALL_FALSE   0x0000

#define EGG_DOC_REBUILD  0X0001
#define EGG_DOC_NOT_REBUILD  0X0002

#define EGG_ROOT_UPDATE   0x0001
#define EGG_NO_ROOT 0x0002


#define EGG_SORT_PART 15

#define EGG_DOC_DEL 1

#define IDX_FILE "/egg.idx"
#define ID_FILE  "/egg.idd"
#define DAT_FILE "/egg.dat"
#define FD_FILE  "/egg.fdd"
#define LOCK_FILE  "/egg.lck"

#define EGG_FIDX "/egg.idx"
#define EGG_FID  "/egg.idd"
#define EGG_FDAT "/egg.dat"
#define EGG_FFD  "/egg.fdd"
#define EGG_FFW  "/egg.fdw"
#define EGG_FDAT_IDT  "/egg.dat.idt"

#define CONTRL_FILE_NAME "/multi.cntrl"

/*
  FIELD
*/

//ANALYZED
#define  EGG_ANALYZED                            (1 << 1)
#define  EGG_NOT_ANALYZED                        (1 << 2)
#define  EGG_CWS_ANALYZED                        EGG_ANALYZED
#define  EGG_CN_ANALYZED                         (1 << 9)
#define  EGG_CY_ANALYZED                         (1 << 10)
#define  EGG_CX_ANALYZED                         (1 << 11)
#define  EGG_OTHER_ANALYZED                      (1 << 12)
#define  EGG_OTHER_DICT                          (1 << 13)

#define EGG_MASK_ANALYZER (EGG_NOT_ANALYZED |  EGG_CWS_ANALYZED | EGG_CN_ANALYZED | EGG_CY_ANALYZED | EGG_CX_ANALYZED | EGG_OTHER_ANALYZED)
//index type
#define  EGG_NOT_INDEX                           (1 << 0)
#define  EGG_NORMAL_INDEX                        (1 << 14)
#define  EGG_RANGE_INDEX                         (1 << 15)

#define  EGG_INDEX_STRING                    (1 << 3)
#define  EGG_INDEX_INT32                     (1 << 4)
#define  EGG_INDEX_INT64                     (1 << 5)
#define  EGG_INDEX_DOUBLE                    (1 << 6)
#define EGG_MASK_INDEX (EGG_NOT_INDEX |  EGG_NORMAL_INDEX | EGG_RANGE_INDEX | EGG_INDEX_STRING | EGG_INDEX_INT32 | EGG_INDEX_INT64 | EGG_INDEX_DOUBLE)


#define EGG_BTREE_STRING_MAX 116

//storage
#define  EGG_STORAGE                             (1 << 7)
#define  EGG_NOT_STORAGE                         (1 << 8)


#define  EGG_COMPRESSED                      (1 << 0)
#define  EGG_NOT_COMPRESSED                  (1 << 1)

#define EGG_TOPSORT_ORDERBY                  (1 << 0)
#define EGG_TOPSORT_WEIGHT                   (1 << 1)
#define EGG_TOPSORT_SCORE                    (1 << 2)
#define EGG_TOPSORT_NOT                      (1 << 3)

//field mode
//#define EGG_ANALYZER_MODE     (1 << 0)
//#define EGG_NOANALYZER_MODE   (1 << 1)
#define EGG_FIELDNAME_MODE    (1 << 0)
#define EGG_FIELDID_MODE      (1 << 1)
#define EGG_FIELDTABLE_MODE   (1 << 2)


#define EGG_CLIENT_TYPE_FCGI     1
#define EGG_CLIENT_TYPE_TCP      2
#define EGG_CLIENT_TYPE_RWS      3
#define EGG_CLIENT_TYPE_CLUSTER  4


#endif //_EGG_DEFINE_H_
