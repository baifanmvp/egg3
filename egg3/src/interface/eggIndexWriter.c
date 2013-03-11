#include "../eggIndexWriter.h"
#include "../storage/eggRecoveryLog.h"
#include <cwsplugin/cwsplugin.h>

struct eggIndexWriterRemote
{
    HEGGHANDLE hEggHandle;
};

HEGGINDEXWRITER EGGAPI eggIndexWriter_open(void *hEggHandle_, char *analyzerName)
{
    if (!hEggHandle_) return EGG_NULL;
    HEGGHANDLE hEggHandle = (HEGGHANDLE)hEggHandle_;
    
    if(POINTER_IS_INVALID(hEggHandle->eggIndexWriter_open))
    {
        return EGG_FALSE;
    }

    return hEggHandle->eggIndexWriter_open(hEggHandle, analyzerName);
}

EBOOL EGGAPI eggIndexWriter_close(HEGGINDEXWRITER hEggIndexWriter)
{
    if (!hEggIndexWriter) return EGG_FALSE;
    if(POINTER_IS_INVALID(hEggIndexWriter->hEggHandle->eggIndexWriter_close))
    {
        return EGG_FALSE;
    }
    
    return hEggIndexWriter->hEggHandle->eggIndexWriter_close(hEggIndexWriter);
}

EBOOL EGGAPI eggIndexWriter_ref_indexcache(HEGGINDEXWRITER hEggIndexWriter, HEGGINDEXCACHE hIndexCache)
{
    if (!hEggIndexWriter) return EGG_FALSE;
    if(POINTER_IS_INVALID(hEggIndexWriter->hEggHandle->eggIndexWriter_ref_indexcache))
    {
        return EGG_FALSE;
    }
    
    return hEggIndexWriter->hEggHandle->eggIndexWriter_ref_indexcache(hEggIndexWriter, hIndexCache);
}


EBOOL EGGAPI eggIndexWriter_set_analyzer(HEGGINDEXWRITER hEggIndexWriter, char *analyzerName)
{
    if (!hEggIndexWriter) return EGG_FALSE;
    if(POINTER_IS_INVALID(hEggIndexWriter->hEggHandle->eggIndexWriter_set_analyzer))
    {
        return EGG_FALSE;
    }
    
    return hEggIndexWriter->hEggHandle->eggIndexWriter_set_analyzer(hEggIndexWriter, analyzerName);
}

EBOOL EGGAPI eggIndexWriter_add_document(HEGGINDEXWRITER hEggIndexWriter, HEGGDOCUMENT hEggDocument)
{
    if (!hEggIndexWriter) return EGG_FALSE;
    if(POINTER_IS_INVALID(hEggIndexWriter->hEggHandle->eggIndexWriter_add_document))
    {
        return EGG_FALSE;
    }
        
    return hEggIndexWriter->hEggHandle->eggIndexWriter_add_document(hEggIndexWriter,
                                                                    hEggDocument);
}

EBOOL EGGAPI eggIndexWriter_optimize(HEGGINDEXWRITER hEggIndexWriter)
{
    if (!hEggIndexWriter) return EGG_FALSE;
    if(POINTER_IS_INVALID(hEggIndexWriter->hEggHandle->eggIndexWriter_optimize))
    {
        return EGG_FALSE;
    }
    
    return hEggIndexWriter->hEggHandle->eggIndexWriter_optimize(hEggIndexWriter);
}

HEGGINDEXREADER EGGAPI  eggIndexWriter_init_reader(HEGGINDEXWRITER hEggIndexWriter)
{
    if (!hEggIndexWriter) return EGG_NULL;
    if(POINTER_IS_INVALID(hEggIndexWriter->hEggHandle->eggIndexWriter_init_reader))
    {
        return EGG_FALSE;
    }
    
    return hEggIndexWriter->hEggHandle->eggIndexWriter_init_reader(hEggIndexWriter);
}

EBOOL EGGAPI eggIndexWriter_reindex_document(HEGGINDEXWRITER hEggIndexWriter_, HEGGDOCUMENT hEggDocument, EGGDID dId)
{
    if (!hEggIndexWriter_) return EGG_FALSE;
    if(POINTER_IS_INVALID(hEggIndexWriter_->hEggHandle->eggIndexWriter_reindex_document))
    {
        return EGG_FALSE;
    }
    
    return hEggIndexWriter_->hEggHandle->eggIndexWriter_reindex_document(hEggIndexWriter_, hEggDocument, dId);
}


EBOOL EGGAPI eggIndexWriter_delete_document(HEGGINDEXWRITER hEggIndexWriter_, EGGDID dId)
{
    if (!hEggIndexWriter_) return EGG_FALSE;
    if(POINTER_IS_INVALID(hEggIndexWriter_->hEggHandle->eggIndexWriter_delete_document))
    {
        return EGG_FALSE;
    }
    
    return hEggIndexWriter_->hEggHandle->eggIndexWriter_delete_document(hEggIndexWriter_, dId);
}

EBOOL EGGAPI eggIndexWriter_modify_document(HEGGINDEXWRITER hEggIndexWriter_, EGGDID dId, HEGGDOCUMENT hEggDocument)
{
    if (!hEggIndexWriter_) return EGG_FALSE;
    if(POINTER_IS_INVALID(hEggIndexWriter_->hEggHandle->eggIndexWriter_modify_document))
    {
        return EGG_FALSE;
    }
    
    return hEggIndexWriter_->hEggHandle->eggIndexWriter_modify_document(hEggIndexWriter_, dId, hEggDocument);
}

EBOOL EGGAPI eggIndexWriter_incrementmodify_document(HEGGINDEXWRITER hEggIndexWriter_, EGGDID dId, HEGGDOCUMENT hEggDocument)
{
    if (!hEggIndexWriter_) return EGG_FALSE;
    if(POINTER_IS_INVALID(hEggIndexWriter_->hEggHandle->eggIndexWriter_incrementmodify_document))
    {
        return EGG_FALSE;
    }
    
    return hEggIndexWriter_->hEggHandle->eggIndexWriter_incrementmodify_document(hEggIndexWriter_, dId, hEggDocument);
}

EBOOL EGGAPI eggIndexWriter_add_field(HEGGINDEXWRITER hEggIndexWriter_, char *fieldName, type_t fieldType, ... /* char *analyzerName */)
{
    if (!hEggIndexWriter_) return EGG_FALSE;
    if(POINTER_IS_INVALID(hEggIndexWriter_->hEggHandle->eggIndexWriter_add_field))
    {
        return EGG_FALSE;
    }
    EBOOL retv;
    if (fieldType & EGG_OTHER_ANALYZED)
    {
        char *analyzerName = NULL;
        
        char *p = NULL;
        va_list ap_arg;
        va_start(ap_arg, fieldType);
        p = va_arg(ap_arg, char*);
        if (p && p[0])
        {
            analyzerName = p;
        }
        retv = hEggIndexWriter_->hEggHandle->eggIndexWriter_add_field(hEggIndexWriter_,
                                                                      fieldName, fieldType,
                                                                      analyzerName);
        va_end(ap_arg);
        
    }
    else
    {
        retv = hEggIndexWriter_->hEggHandle->eggIndexWriter_add_field(hEggIndexWriter_,
                                                                      fieldName, fieldType);
    }
    return retv;
}

EBOOL EGGAPI eggIndexWriter_modify_field(HEGGINDEXWRITER hEggIndexWriter_, char *oldFieldName, char *fieldName, type_t fieldType, ... /* char *analyzerName */)
{
    if (!hEggIndexWriter_) return EGG_FALSE;
    if(POINTER_IS_INVALID(hEggIndexWriter_->hEggHandle->eggIndexWriter_add_field))
    {
        return EGG_FALSE;
    }
    EBOOL retv;
    if (fieldType & EGG_OTHER_ANALYZED)
    {
        char *analyzerName = NULL;
        
        char *p = NULL;
        va_list ap_arg;
        va_start(ap_arg, fieldType);
        p = va_arg(ap_arg, char*);
        if (p && p[0])
        {
            analyzerName = p;
        }
        retv = hEggIndexWriter_->hEggHandle->eggIndexWriter_modify_field(hEggIndexWriter_, oldFieldName,
                                                                         fieldName, fieldType,
                                                                         analyzerName);
        va_end(ap_arg);
        
    }
    else
    {
        retv = hEggIndexWriter_->hEggHandle->eggIndexWriter_modify_field(hEggIndexWriter_, oldFieldName,
                                                                         fieldName, fieldType);
    }
    return retv;    
}

EBOOL EGGAPI eggIndexWriter_delete_field(HEGGINDEXWRITER hEggIndexWriter_, char *fieldName)
{
    if (!hEggIndexWriter_) return EGG_FALSE;
    if(POINTER_IS_INVALID(hEggIndexWriter_->hEggHandle->eggIndexWriter_delete_field))
    {
        return EGG_FALSE;
    }
    
    return hEggIndexWriter_->hEggHandle->eggIndexWriter_delete_field(hEggIndexWriter_, fieldName);
    
}
