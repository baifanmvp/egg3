#include "soBuild.h"


PRIVATE _OBJECTPTR soBuild_serialize_default(_OBJECTPTR);
PRIVATE _OBJECTPTR soBuild_unserialize_default(_OBJECTPTR);


PUBLIC soBuild* soBuild_new(FnSerialize fnSerialize,
                            FnSerialize fnUnserialize)
{
    soBuild* pSoBuild = (soBuild*)malloc(sizeof(soBuild));
    if (!pSoBuild)
    {
        perror("[*ERROR]-soBuild_new malloc");
        exit(-1);
    }

    memset(pSoBuild, 0, sizeof(soBuild));

    pSoBuild->fnSerialize = (fnSerialize) ? fnSerialize : soBuild_serialize_default;
    pSoBuild->fnUnserialize = (fnUnserialize) ? fnUnserialize : soBuild_unserialize_default;

    return pSoBuild;
}

PUBLIC WBOOL soBuild_delete(soBuild* pSoBuild)
{
    w_return_val_if_fail(pSoBuild_is_object(pSoBuild), W_FALSE);

    SOEGG_FREE(pSoBuild->name);
    SOEGG_FREE(pSoBuild->sname);
    SOEGG_FREE(pSoBuild->address);
    SOEGG_FREE(pSoBuild->area);
    
    free(pSoBuild);

    return W_TRUE;
}

PUBLIC WBOOL soBuild_set_name(soBuild* pSoBuild,
                              const wchar* pName)
{
    w_return_val_if_fail(soBuild_is_object(pSoBuild), W_FALSE);
    w_return_val_if_fail(pName, W_FALSE);

    SOEGG_FREE(pSoBuild->name);

    length_t n_name_len = strlen(pName);
    pSoBuild->name = (wchar*)malloc(n_name_len + 8);
    memcpy(pSoBuild->name, pName, n_name_len);
    *(pSoBuild->name + n_name_len) = '\0';
        
    return W_TRUE;
}

PUBLIC WBOOL soBuild_set_sname(soBuild* pSoBuild,
                               const wchar* pSName)
{
    w_return_val_if_fail(soBuild_is_object(pSoBuild), W_FALSE);
    w_return_val_if_fail(pSName, W_FALSE);

    SOEGG_FREE(pSoBuild->sname);

    length_t n_sname_len = strlen(pSName);
    pSoBuild->sname = (wchar*)malloc(n_sname_len + 8);
    memcpy(pSoBuild->sname, pSName, n_sname_len);
    *(pSoBuild->sname + n_sname_len) = '\0';
    
    return W_TRUE;
}

PUBLIC WBOOL soBuild_set_address(soBuild* pSoBuild,
                                 const wchar* pAddress)
{
    w_return_val_if_fail(soBuild_is_object(pSoBuild), W_FALSE);
    w_return_val_if_fail(pAddress, W_FALSE);

    SOEGG_FREE(pSoBuild->address);

    length_t n_address_len = strlen(pAddress);
    pSoBuild->address = (wchar*)malloc(n_address_len + 8);
    memcpy(pSoBuild->address, pAddress, n_address_len);
    *(pSoBuild->address + n_address_len) = '\0';
    
    return W_TRUE;
}

PUBLIC WBOOL soBuild_set_area(soBuild* pSoBuild,
                              const wchar* pArea)
{
    w_return_val_if_fail(soBuild_is_object(pSoBuild), W_FALSE);
    w_return_val_if_fail(pArea, W_FALSE);

    SOEGG_FREE(pSoBuild->area);

    length_t n_area_len = strlen(pArea);
    pSoBuild->area = (wchar*)malloc(n_area_len + 8);
    memcpy(pSoBuild->area, pArea, n_area_len);
    *(pSoBuild->area + n_area_len) = '\0';
    
    return W_TRUE;
}

PUBLIC WBOOL soBuild_set_size(soBuild* pSoBuild,
                              size16_t nSize)
{
    w_return_val_if_fail(soBuild_is_object(pSoBuild), W_FALSE);
    w_return_val_if_fail(nSize > 0, W_FALSE);

    pSoBuild->size = nSize;
    
    return W_TRUE;
}

PUBLIC WBOOL soBuild_set_price(soBuild* pSoBuild,
                               size16_t nPrice)
{
    w_return_val_if_fail(soBuild_is_object(pSoBuild), W_FALSE);
    w_return_val_if_fail(nPrice > 0, W_FALSE);

    pSoBuild->price = nPrice;
    
    return W_TRUE;
}

PUBLIC eggDocument* soBuild_serialize(soBuild* pSoBuild)
{
    w_return_val_if_fail(soBuild_is_object(pSoBuild), W_FALSE);
    w_return_val_if_fail(pSoBuild->fnSerialize, W_FALSE);

    return (eggDocument*)(pSoBuild->fnSerialize)((_OBJECTPTR)pSoBuild);
}

PUBLIC WBOOL soBuild_unserialize()
{
}



PRIVATE _OBJECTPTR soBuild_serialize_default(_OBJECTPTR)
{
    soBuild* pSoBuild = (soBuild*)_OBJECTPTR;
    w_return_val_if_fail(soBuild_is_object(pSoBuild), W_FALSE);

    eggDocument* p_egg_document = eggDocument_new();

    HFIELD p_name_field = egg_Field_new(BUILD_NAME_FIELD,
                                        pSoBuild->name, strlen(pSoBuild->name),
                                        ANALYZED);
    HFIELD p_sname_field = egg_Field_new(BUILD_SNAME_FIELD,
                                         pSoBuild->sname, strlen(pSoBuild->sname),
                                         ANALYZED);
    HFIELD p_address_field = egg_Field_new(BUILD_ADDRESS_FIELD,
                                           pSoBuild->address, strlen(pSoBuild->address),
                                           ANALYZED);
    HFIELD p_area_field = egg_Field_new(BUILD_AREA_FIELD,
                                        pSoBuild->area, strlen(pSoBuild->area),
                                        ANALYZED);

    HFIELD p_size_field = egg_Field_new(BUILD_SIZE_FIELD,
                                        &(pSoBuild->size), sizeof(size16_t),
                                        NOT_ANALYZED | ORDER_BY_NUMBER);

    HFIELD p_price_field = egg_Field_new(BUILD_PRICE_FIELD,
                                         &(pSoBuild->price), sizeof(size16_t),
                                         NOT_ANALYZED | ORDER_BY_NUMBER);

    eggDocument_add_field(p_egg_document, p_name_field);
    eggDocument_add_field(p_egg_document, p_sname_field);
    eggDocument_add_field(p_egg_document, p_address_field);
    eggDocument_add_field(p_egg_document, p_area_field);
    eggDocument_add_field(p_egg_document, p_size_field);
    eggDocument_add_field(p_egg_document, p_price_field);
    
    return p_egg_document;
}

PRIVATE _OBJECTPTR soBuild_unserialize_default(_OBJECTPTR)
{
}

