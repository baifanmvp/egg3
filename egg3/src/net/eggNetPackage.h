#ifndef EGG_NETPACKAGE_H_
#define EGG_NETPACKAGE_H_
#include "../EggDef.h"
#include "../uti/Utility.h"
#include <stdarg.h>
E_BEGIN_DECLS

typedef struct eggNetPackage EGGNETPACKAGE;
typedef struct eggNetPackage* HEGGNETPACKAGE;
#pragma pack(push)
#pragma pack(4)

struct eggNetPackage
{
    type_t op;
    size32_t eSize;
    size32_t aSize;
};



typedef struct eggNetUnitPackage EGGNETUNITPACKAGE;
typedef struct eggNetUnitPackage* HEGGNETUNITPACKAGE;

struct eggNetUnitPackage
{
    type_t ty;
    size32_t size;
};
#pragma pack(pop)
    
#define EGG_UNITSIZE_PACKAGE (4096)
HEGGNETPACKAGE eggNetPackage_new(type_t op);

HEGGNETPACKAGE eggNetPackage_add(HEGGNETPACKAGE hNetPackage, void* data, size32_t size, type_t ty);

EBOOL eggNetPackage_fetch(HEGGNETPACKAGE hNetPackage, int cnt, ...);

EBOOL eggNetPackage_fetch_by_iter(HEGGNETPACKAGE hNetPackage, int* pIter, void** data, size32_t* pSize);

size32_t eggNetPackage_get_packagesize(HEGGNETPACKAGE hNetPackage);

EBOOL eggNetPackage_delete(HEGGNETPACKAGE hNetPackage);

/* for back-compliance */
#define eggNetPackage_get_packageSize(hNetPackage) eggNetPackage_get_packagesize(hNetPackage)
/* for back-compliance end */


E_END_DECLS

#endif
