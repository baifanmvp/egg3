#include "./eggNetPackage.h"
#include "../log/eggPrtLog.h"
#include <errno.h>

HEGGNETPACKAGE eggNetPackage_new( type_t op)
{
    HEGGNETPACKAGE lp_net_package = (HEGGNETPACKAGE)malloc(sizeof(EGGNETPACKAGE) + EGG_UNITSIZE_PACKAGE);
    lp_net_package->op = op;
    lp_net_package->eSize = 0;
    lp_net_package->aSize = EGG_UNITSIZE_PACKAGE;
    
    return lp_net_package;
}

HEGGNETPACKAGE eggNetPackage_add(HEGGNETPACKAGE hNetPackage, void* data, size32_t size, type_t ty)
{
    if(POINTER_IS_INVALID(hNetPackage))
    {
        return EGG_NULL;
    }
    size32_t n_org_aSize = hNetPackage->aSize;
    EGGNETUNITPACKAGE st_unit_package = {0};
    
    st_unit_package.ty = ty;
    st_unit_package.size = size;

    while(hNetPackage->aSize < hNetPackage->eSize + sizeof(st_unit_package) + size)
    {
        hNetPackage->aSize += hNetPackage->aSize;
    }
    if(n_org_aSize != hNetPackage->aSize)
    {
//        printf("realloc\n");
        hNetPackage = (HEGGNETPACKAGE)realloc(hNetPackage, sizeof(EGGNETPACKAGE) + hNetPackage->aSize);
        if(!hNetPackage)
        {
            perror("realloc:");
            eggPrtLog_error("eggNetPackage", "realloc %s Exit\n", strerror(errno));
	    fprintf(stderr, "realloc %s Exit\n", strerror(errno));
	    exit(-1);
        }
    }
    char* lp_package_data = hNetPackage + 1;
    
    memcpy(lp_package_data + hNetPackage->eSize, &st_unit_package, sizeof(st_unit_package));
    hNetPackage->eSize += sizeof(st_unit_package);
    if(data && size)
    {
        memcpy(lp_package_data + hNetPackage->eSize, data, size);
        hNetPackage->eSize += size;
    }
    return hNetPackage;
}




EBOOL eggNetPackage_fetch(HEGGNETPACKAGE hNetPackage, int cnt, ...)
{
    if(POINTER_IS_INVALID(hNetPackage))
    {
        return EGG_FALSE;
    }
    va_list ap;
    va_start(ap, cnt);
    
    int i = 0;
    void** p_data;
    size32_t* p_size;
    size32_t n_package_len = hNetPackage->eSize;
    size32_t n_iter_len = 0;
    char* lp_package_str = (char*)(hNetPackage + 1);
    
    for(i = 0; i < cnt && n_iter_len < n_package_len; i += 2)
    {
        p_data = va_arg(ap, void**);
        p_size = va_arg(ap, size32_t*);

        HEGGNETUNITPACKAGE lp_iter_package = lp_package_str + n_iter_len;
        if(p_size)
            *p_size = lp_iter_package->size;
        
        *p_data = lp_iter_package + 1;
        
        n_iter_len += lp_iter_package->size + sizeof(EGGNETUNITPACKAGE);   
    }
    
    va_end(ap);
    return EGG_TRUE;
}

EBOOL eggNetPackage_fetch_by_iter(HEGGNETPACKAGE hNetPackage, int* pIter, void** data, size32_t* pSize)
{
    if(POINTER_IS_INVALID(hNetPackage))
    {
        return EGG_FALSE;
    }
    
    if(*pIter >= hNetPackage->eSize)
    {
        return EGG_FALSE;
    }
    
    size32_t n_package_len = hNetPackage->eSize;
    size32_t n_iter_len = 0;
    char* lp_package_str = (char*)(hNetPackage + 1);
    
    HEGGNETUNITPACKAGE lp_iter_package = lp_package_str + *pIter;
    *data = lp_iter_package + 1;
    *pSize = lp_iter_package->size;

    *pIter += sizeof(EGGNETUNITPACKAGE) + lp_iter_package->size;
    
    return EGG_TRUE;
}

size32_t eggNetPackage_get_packagesize(HEGGNETPACKAGE hNetPackage)
{
    if(POINTER_IS_INVALID(hNetPackage))
    {
        return EGG_FALSE;
    }
    return hNetPackage->eSize + sizeof(EGGNETPACKAGE);
}

size32_t eggNetPackage_get_datasize(HEGGNETPACKAGE hNetPackage)
{
    if(POINTER_IS_INVALID(hNetPackage))
    {
        return EGG_FALSE;
    }
    return hNetPackage->eSize;
}

EBOOL eggNetPackage_delete(HEGGNETPACKAGE hNetPackage)
{
    if(POINTER_IS_INVALID(hNetPackage))
    {
        return EGG_FALSE;
    }
    
    free(hNetPackage);
    return EGG_TRUE;
}
