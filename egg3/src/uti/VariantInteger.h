#ifndef _CORE_VARIANT_INTEGER_H_
#define _CORE_VARIANT_INTEGER_H_
#include "../EggDef.h"


E_BEGIN_DECLS

typedef void (*FnWriteByte) (i8 v, epointer user_data);
typedef i8   (*FnReadByte)  (epointer user_data);


extern size32_t core_read_vint(FnReadByte fnReadByte, epointer user_data);

extern size64_t core_read_vlong(FnReadByte fnReadByte, epointer user_data);

extern EBOOL core_write_vint(size32_t v, FnWriteByte fnWriteByte, epointer user_data);

extern EBOOL core_write_vlong(size64_t v, FnWriteByte fnWriteByte, epointer user_data);


E_END_DECLS


#endif //_CORE_VARIANT_INTEGER_H_

