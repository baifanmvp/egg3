#include "../eggIndexReader.h"

HEGGINDEXREADER EGGAPI eggIndexReader_open(void *hEggHandle_)
{
    if (!hEggHandle_) return EGG_NULL;
    HEGGHANDLE hEggHandle = (HEGGHANDLE)hEggHandle_;
    if(POINTER_IS_INVALID(hEggHandle->eggIndexReader_open))
    {
        return EGG_NULL;
    }

    return hEggHandle->eggIndexReader_open(hEggHandle);
}

EBOOL EGGAPI eggIndexReader_close(HEGGINDEXREADER hIndexReader)
{
    if (!hIndexReader) return EGG_FALSE;
    
    if(POINTER_IS_INVALID(hIndexReader->hEggHandle->eggIndexReader_close))
    {
        return EGG_FALSE;
    }
    return hIndexReader->hEggHandle->eggIndexReader_close(hIndexReader);
}
EBOOL EGGAPI eggIndexReader_ref_indexcache(HEGGINDEXREADER hEggIndexReader, HEGGINDEXCACHE hIndexCache)
{
    if (!hEggIndexReader) return EGG_FALSE;
    if(POINTER_IS_INVALID(hEggIndexReader->hEggHandle->eggIndexReader_ref_indexcache))
    {
        return EGG_FALSE;
    }
    
    return hEggIndexReader->hEggHandle->eggIndexReader_ref_indexcache(hEggIndexReader, hIndexCache);

}

EBOOL EGGAPI eggIndexReader_get_document(HEGGINDEXREADER hIndexReader, EGGDID dId, HEGGDOCUMENT* ppeggDocument)
{
    if (!hIndexReader) return EGG_FALSE;
    
    if(POINTER_IS_INVALID(hIndexReader->hEggHandle->eggIndexReader_get_document))
    {
        return EGG_FALSE;
    }
    
    return hIndexReader->hEggHandle->eggIndexReader_get_document(hIndexReader,
                                                                 dId,
                                                                 ppeggDocument);
}

EBOOL EGGAPI eggIndexReader_get_documentset(HEGGINDEXREADER hIndexReader, HEGGSCOREDOC hScoreDoc, count_t nDocCnt, HEGGDOCUMENT** pppeggDocument)
{
    if (!hIndexReader) return EGG_FALSE;
    
    if(POINTER_IS_INVALID(hIndexReader->hEggHandle->eggIndexReader_get_documentset))
    {
        return EGG_FALSE;
    }
    
    return hIndexReader->hEggHandle->eggIndexReader_get_documentset(hIndexReader,
                                                                 hScoreDoc,
                                                                 nDocCnt,
                                                                 pppeggDocument);
    
}


HEGGDOCUMENT EGGAPI eggIndexReader_export_document(HEGGINDEXREADER hIndexReader, offset64_t* pCursor)
{
    if (!hIndexReader) return EGG_NULL;
    
    if(POINTER_IS_INVALID(hIndexReader->hEggHandle->eggIndexReader_export_document))
    {
        return EGG_FALSE;
    }
    
    
    return hIndexReader->hEggHandle->eggIndexReader_export_document(hIndexReader,
                                                                    pCursor);
    
}

count_t EGGAPI eggIndexReader_get_doctotalcnt(HEGGINDEXREADER hIndexReader)
{
    if (!hIndexReader) return 0;
    
    if(POINTER_IS_INVALID(hIndexReader->hEggHandle->eggIndexReader_get_doctotalcnt))
    {
        return 0;
    }
    
    
    return hIndexReader->hEggHandle->eggIndexReader_get_doctotalcnt(hIndexReader);
    
}

EBOOL EGGAPI eggIndexReader_get_fieldnameinfo(HEGGINDEXREADER hIndexReader, HEGGFIELDNAMEINFO *hhFieldNameInfo, count_t *lpCntFieldNameInfo)
{
    if (!hIndexReader) return EGG_FALSE;
    
    if(POINTER_IS_INVALID(hIndexReader->hEggHandle->eggIndexReader_get_fieldnameinfo))
    {
        return EGG_FALSE;
    }
    
    return hIndexReader->hEggHandle->eggIndexReader_get_fieldnameinfo(hIndexReader,
                                                                      hhFieldNameInfo,
                                                                      lpCntFieldNameInfo);
}

EBOOL EGGAPI eggIndexReader_get_singlefieldnameinfo(HEGGINDEXREADER hIndexReader, char *fieldName, HEGGFIELDNAMEINFO *hhFieldNameInfo)
{
    if (!hIndexReader) return EGG_FALSE;
    
    if(POINTER_IS_INVALID(hIndexReader->hEggHandle->eggIndexReader_get_singlefieldnameinfo))
    {
        return EGG_FALSE;
    }
    
    return hIndexReader->hEggHandle->eggIndexReader_get_singlefieldnameinfo(hIndexReader,
                                                                fieldName,
                                                                hhFieldNameInfo);
}

EBOOL EGGAPI eggIndexReader_free(HEGGINDEXREADER hIndexReader)
{
    if (!hIndexReader) return EGG_NULL;
    if(POINTER_IS_INVALID(hIndexReader->hEggHandle->eggIndexReader_free))
    {
        return EGG_FALSE;
    }
    
    return hIndexReader->hEggHandle->eggIndexReader_free(hIndexReader);
}
