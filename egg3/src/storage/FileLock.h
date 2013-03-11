#ifndef _FILE_LOCK_
#define _FILE_LOCK_
#include "../EggDef.h"
#include <unistd.h>
#include <fcntl.h>

typedef struct flock EFLOCKINFO;
typedef struct flock* HEFLOCKINFO;

#define EFlockInfo_object(hEFlockInfo)          \
    ((EFLOCKINFO*)(hEFlockInfo))

#define EFlockInfo_is_object(hEFlockInfo)          \
    (hEFlockInfo ? EGG_TRUE : EGG_FALSE)

#define EGG_RDLCK(hEFlockInfo, whence, start, len)  \
    {                                                                   \
        EFLOCKINFO* lp_eFlock_info = EFlockInfo_object((hEFlockInfo));  \
        lp_eFlock_info->l_whence = whence;                              \
        lp_eFlock_info->l_start = start;                                \
        lp_eFlock_info->l_len = len;                                    \
        lp_eFlock_info->l_type = F_RDLCK;                               \
    }

#define EGG_WDLCK(hEFlockInfo, whence, start, len)                      \
    {                                                                   \
        EFLOCKINFO* lp_eFlock_info = EFlockInfo_object((hEFlockInfo));  \
        lp_eFlock_info->l_whence = whence;                              \
        lp_eFlock_info->l_start = start;                                \
        lp_eFlock_info->l_len = len;                                    \
        lp_eFlock_info->l_type = F_WRLCK;                               \
    }

#define EGG_UNLCK(hEFlockInfo, whence, start, len)                      \
    {                                                                   \
        EFLOCKINFO* lp_eFlock_info = EFlockInfo_object((hEFlockInfo));  \
        lp_eFlock_info->l_whence = whence;                              \
        lp_eFlock_info->l_start = start;                                \
        lp_eFlock_info->l_len = len;                                    \
        lp_eFlock_info->l_type = F_UNLCK;                               \
    }



extern inline EBOOL egg_set_lock(HEFLOCKINFO hEFlockInfo, file_t fd);

extern inline EBOOL egg_set_wlock(HEFLOCKINFO hEFlockInfo, file_t fd);

extern inline EBOOL egg_get_lock(HEFLOCKINFO hEFlockInfo, file_t fd);




#endif  /* _FILE_LOCK_ */
