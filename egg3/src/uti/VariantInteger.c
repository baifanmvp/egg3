#include "VariantInteger.h"


PUBLIC size32_t core_read_vint(FnReadByte fnReadByte, epointer user_data)
{
    i8 u = fnReadByte(user_data);
    size32_t v = u & 0x7F;

    size32_t shift;
    for (shift = 7; (u & 0x80) != 0; shift += 7)
    {
        u = fnReadByte(user_data);
        v |= (u & 0x7F) << shift;
    }
    
    return v;
}

PUBLIC size64_t core_read_vlong(FnReadByte fnReadByte, epointer user_data)
{
    i8 u = fnReadByte(user_data);
    size64_t v = u & 0x7F;

    size64_t shift;
    for (shift = 7; (u & 0x80) != 0; shift += 7)
    {
        u = fnReadByte(user_data);
        v |= (u & 0x7F) << shift;
    }
    
    return v;

}

PUBLIC EBOOL core_write_vint(size32_t v, FnWriteByte fnWriteByte, epointer user_data)
{
    
    while ((v & ~0x7F) != 0)
    {
        fnWriteByte((i8)((v & 0x7F) | 0x80), user_data);
        v >>= 7;
    }

    fnWriteByte((i8)v, user_data);
    
    return EGG_TRUE;
}

PUBLIC EBOOL core_write_vlong(size64_t v, FnWriteByte fnWriteByte, epointer user_data)
{
    while ((v & ~0x7F) != 0)
    {
        fnWriteByte((i8)((v & 0x7F) | 0x80), user_data);
        v >>= 7;
    }

    fnWriteByte((i8)v, user_data);
    
    return EGG_TRUE;
    
}
