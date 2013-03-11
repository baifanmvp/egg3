#include "emem.h"


PUBLIC EBOOL EMemCmp(const void* src, const void* dest, size32_t nLen)
{
    if (!nLen)
    {
        return EGG_FALSE;
    }

    while (--nLen && *(uechar*)src == *(uechar*)dest)
    {
        src = (uechar*)src + 1;
        dest = (uechar*)dest + 1;
    }

    int ret = (*((uechar*)src) - *((uechar*)dest));

    return (ret)?((ret > 0)?EGG_CMP_GREATER:EGG_CMP_LESS):EGG_CMP_EQUAL;
}

PUBLIC EBOOL EMemMove(void* dest, const void* src, size32_t nLen)
{
    if(!nLen)
    {
        return EGG_TRUE;
    }
    
    if (dest <= src || (uechar*)dest >= ((uechar*)src + nLen))
    {
        while (nLen--)
        {
            *(uechar*)dest = *(uechar*)src;
            dest = (uechar*)dest + 1;
            src = (uechar*)src + 1;
        }
    }
    else
    {
        dest = (uechar*)dest + nLen - 1;
        src = (uechar*)src + nLen - 1;

        while (nLen--)
        {
            *(uechar*)dest = *(uechar*)src;
            dest = (uechar*)dest - 1;
            src = (uechar*)src - 1;
        }
    }

    return EGG_TRUE;
}

PUBLIC EBOOL EMemCpy(void* dest, const void* src, size32_t nLen)
{
    while (nLen--)
    {
        *(uechar*)dest = *(uechar*)src;
        dest = (uechar*)dest + 1;
        src = (uechar*)src + 1;
    }
    
    return EGG_TRUE;
}
