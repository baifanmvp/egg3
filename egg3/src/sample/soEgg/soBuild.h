#ifndef _SAMPLE_SO_BUILD_H_
#define _SAMPLE_SO_BUILD_H_
#include <walrus/walrus.h>


W_BEGIN_DECLS

typedef enum
{
    
} BuildStatus;
    

typedef struct _soBuild soBuild;

typedef _OBJECTPTR (*FnSerialize) (_OBJECTPTR obj);
typedef WBOOL (*Fn_Free) (pssArticle* a);

struct _soBuild
{
    FnSerialize fnSerialize;
    FnSerialize fnUnserialize;
    
    wchar* name;
    wchar* sname;
    wchar* address;
    wchar* area;
    size16_t size;
    size16_t price;
    
    BuildStatus status;
};


GLOBAL soBuild* soBuild_new(FnSerialize fnSerialize,
                            FnSerialize fnUnserialize);

GLOBAL WBOOL soBuild_delete(soBuild* pSoBuild);

GLOBAL WBOOL soBuild_set_name(soBuild* pSoBuild,
                              const wchar* pName);

GLOBAL WBOOL soBuild_set_sname(soBuild* pSoBuild,
                               const wchar* pSName);

GLOBAL WBOOL soBuild_set_address(soBuild* pSoBuild,
                                 const wchar* pAddress);

GLOBAL WBOOL soBuild_set_area(soBuild* pSoBuild,
                              const wchar* pArea);

GLOBAL WBOOL soBuild_set_size(soBuild* pSoBuild,
                              size16_t nSize);

GLOBAL WBOOL soBuild_set_price(soBuild* pSoBuild,
                               size16_t nPrice);

GLOBAL eggDocument* soBuild_serialize(soBuild* pSoBuild);

GLOBAL WBOOL soBuild_unserialize();


#define soBuild_is_object(pSoBuild) \
    ((pSoBuild)?W_TRUE:W_FALSE)



#define BUILD_NAME_FIELD     "build_name"
#define BUILD_SNAME_FIELD    "build_sname"
#define BUILD_ADDRESS_FIELD  "build_address"
#define BUILD_AREA_FIELD     "build_area"
#define BUILD_SIZE_FIELD     "build_size"
#define BUILD_PRICE_FIELD    "build_price"



W_END_DECLS


#endif //_SAMPLE_SO_BUILD_H_

