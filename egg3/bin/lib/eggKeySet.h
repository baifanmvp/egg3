#ifndef EGG_KEYSET_H_
#define EGG_KEYSET_H_
#include "egg2/Egg2.h"
#include "egg2/EggDef.h"
#include "egg2/index/eggIndexView.h"
#include "egg2/index/eggFieldView.h"
E_BEGIN_DECLS

typedef struct _eggKeySet EGGKEYSET;
typedef struct _eggKeySet* HEGGKEYSET;

struct _eggKeySet
{
    HEGGFIELDVIEW hFieldView;
    HEGGINDEXVIEW hIndexView;
};

HEGGKEYSET eggKeySet_new(char* path);

E_END_DECLS
#endif

