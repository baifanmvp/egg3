#include "FileLock.h"


PUBLIC inline EBOOL egg_set_lock(HEFLOCKINFO hEFlockInfo, file_t fd)
{
    if (!EFlockInfo_is_object(hEFlockInfo))
    {
        return EGG_FALSE;
    }
    
    int ret =  fcntl(fd, F_SETLK, hEFlockInfo);
    
    if (ret == -1)
    {
        return EGG_FALSE;
    }
    
    return EGG_TRUE;
}

PUBLIC inline EBOOL egg_set_wlock(HEFLOCKINFO hEFlockInfo, file_t fd)
{
    if (!EFlockInfo_is_object(hEFlockInfo))
    {
        return EGG_FALSE;
    }

    int ret =  fcntl(fd, F_SETLKW, hEFlockInfo);
    
    if (ret == -1)
    {
        return EGG_FALSE;
    }
    
    return EGG_TRUE;
}

PUBLIC inline EBOOL egg_get_lock(HEFLOCKINFO hEFlockInfo, file_t fd)
{
    if (!EFlockInfo_is_object(hEFlockInfo))
    {
        return EGG_FALSE;
    }
    
    int ret =  fcntl(fd, F_GETLK, hEFlockInfo);
    
    if (ret == -1)
    {
        return EGG_FALSE;
    }
    
    return EGG_TRUE;

}
