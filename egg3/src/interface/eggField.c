#include "eggField.h"
#include "../log/eggPrtLog.h"
struct eggField
{
    type_t type;
    size16_t mask;
    echar* name;
    size32_t size;
    echar* buf;
    echar* analyzerName;
    echar* dictName;
    struct eggField* next;
};

#define EGGFIELD_IS_INVALID(hEggField) \
    (!hEggField ? EGG_TRUE : EGG_FALSE)

#define COMPRESS_CRITICAL_VALUE (10*1024)





HEGGFIELD EGGAPI eggField_new(const echar* lpName,
                              const echar* lpValue, size32_t nSize,
                              type_t type, ...)
{
    if (lpName == EGG_NULL /* && strlen(lpName) > EGGFIELDNAMELEN */)
    {
        return EGG_NULL;
    }

    /* 当lpValue == NULL || nSize == 0 时, 会造一个新field但不产生document */
    /*
    if(lpValue ==EGG_NULL || nSize == 0)
    {
        return EGG_NULL;
    }
    */

    ////////////////////
    if((type & EGG_RANGE_INDEX) && (type & EGG_INDEX_STRING) && (nSize > EGG_BTREE_STRING_MAX) )
    {
        //printf("ERROR : valSize limit is 116 byte  in (EGG_INDEX_STRING && EGG_RANGE_INDEX )!\n");
        eggPrtLog_error("eggField", "ERROR : valSize limit is 116 byte  in (EGG_INDEX_STRING && EGG_RANGE_INDEX )!\n");
        return EGG_NULL;
        
    }

    if((type & EGG_RANGE_INDEX) && !(type & EGG_NOT_ANALYZED))
    {
        //printf("ERROR : EGG_RANGE_INDEX only use EGG_NOT_ANALYZED !\n");
        eggPrtLog_error("eggField", "ERROR : EGG_RANGE_INDEX only use EGG_NOT_ANALYZED !\n");
        return EGG_NULL;
        
    }

    HEGGFIELD hEggField = (HEGGFIELD)malloc(sizeof(EGGFIELD));
    
    if (!hEggField)
    {
        return EGG_NULL;
    }
    memset(hEggField, 0, sizeof(EGGFIELD));

    
    if(!(type & (EGG_NOT_ANALYZED | EGG_NOT_INDEX | EGG_CN_ANALYZED | EGG_CY_ANALYZED | EGG_CWS_ANALYZED | EGG_OTHER_ANALYZED | EGG_CX_ANALYZED) ))
    {
        type |= EGG_ANALYZED;
    }
    
    if(!(type & (EGG_INDEX_DOUBLE | EGG_INDEX_INT64 | EGG_INDEX_INT32 | EGG_INDEX_STRING) ))
    {
        type |= EGG_INDEX_STRING;
    }
    
    if(!(type & (EGG_NOT_STORAGE | EGG_STORAGE ) ))
    {
        type |= EGG_STORAGE;
    }
    
    char* lp_field_analyzer = EGG_NULL;
    if(type & EGG_OTHER_ANALYZED)
    {
        va_list ap_arg;
        va_start(ap_arg, type);
        lp_field_analyzer = strdup(va_arg(ap_arg, char*));
    }
    else
    {
        if (type & EGG_ANALYZED)
        {
            lp_field_analyzer = strdup(ANALYZER_CWSLEX);
        }
        else if (type & EGG_CY_ANALYZED)
        {
            lp_field_analyzer = strdup(ANALYZER_CYLEX);
        }
        else if (type & EGG_CN_ANALYZED)
        {
            lp_field_analyzer = strdup(ANALYZER_CNLEX);
        }
        else if (type & EGG_CX_ANALYZED)
        {
            lp_field_analyzer = strdup(ANALYZER_CXLEX);
        }
        else
        {
            lp_field_analyzer = strdup("");
        }
    }
    
    eggField_dup(hEggField, lpName, strlen(lpName),
                 lpValue, nSize, type, lp_field_analyzer);

    return hEggField;
}

EBOOL EGGAPI eggField_delete(HEGGFIELD hEggField)
{
    if (EGGFIELD_IS_INVALID(hEggField))
    {
        return EGG_FALSE;
    }
    
    if (hEggField->name)
    {
        free(hEggField->name);
    }
    if (hEggField->analyzerName)
    {
        free(hEggField->analyzerName);
    }

    if(hEggField->type & EGG_OTHER_DICT)
    {
        if(hEggField->dictName)
            free(hEggField->dictName);
    }
    
    if (hEggField->buf)
    {
        free(hEggField->buf);
    }
    
    free(hEggField);
    hEggField = EGG_NULL;   
    return EGG_FALSE;
}


EBOOL EGGAPI eggField_dup(HEGGFIELD hEggField,
                          const echar* lpName, size32_t nNameSize,
                          const echar* lpValue, size32_t nValSize,
                          type_t type, char* analyzerName)
{
    if (EGGFIELD_IS_INVALID(hEggField))
    {
        return EGG_FALSE;
    }


    if (lpName != EGG_NULL)
    {
        if (hEggField->name != EGG_NULL)
        {
            free(hEggField->name);
            hEggField->name = EGG_NULL;
        }
        hEggField->name = (echar*)malloc(nNameSize + 8);
        memcpy(hEggField->name, lpName, nNameSize);
        *(hEggField->name + nNameSize) = '\0';
    }

    if (lpValue != EGG_NULL)
    {
        if (hEggField->size <= nValSize)
        {
            if (hEggField->buf)
            {
                free(hEggField->buf);
                hEggField->buf = EGG_NULL;
            }
            
            hEggField->buf = (echar*)malloc(nValSize + 8);
        }
    
        memcpy(hEggField->buf, lpValue, nValSize);
        *(hEggField->buf + nValSize) = '\0';
        hEggField->size = nValSize;
    }
    
    hEggField->type |= type;
    hEggField->analyzerName = analyzerName;
    return EGG_TRUE;
}

echar* EGGAPI eggField_get_value(HEGGFIELD hEggField, size32_t* lpnSize)
{
    if (EGGFIELD_IS_INVALID(hEggField))
    {
        return EGG_NULL;
    }

    if (lpnSize)
    {
        *lpnSize = hEggField->size;        
    }
    
        
    return hEggField->buf;
}

EBOOL EGGAPI eggField_set_value(HEGGFIELD hEggField, const echar* lpValue, size32_t nSize)
{
    if (EGGFIELD_IS_INVALID(hEggField))
    {
        return EGG_FALSE;
    }
    
    if(nSize > hEggField->size)
    {
        hEggField->buf = (echar*)realloc(hEggField->buf, nSize);
    }
    
    memcpy(hEggField->buf, lpValue, nSize);
    hEggField->size = nSize;

    return EGG_TRUE;
}
EBOOL EGGAPI eggField_append_value(HEGGFIELD hEggField, const echar* lpValue, size32_t nSize)
{
    if (EGGFIELD_IS_INVALID(hEggField))
    {
        return EGG_FALSE;
    }
    
    if (!(hEggField->type & EGG_INDEX_STRING))
    {
        return EGG_FALSE;
    }
    
    hEggField->buf = (echar*)realloc(hEggField->buf, hEggField->size + nSize + 1);
    
    hEggField->buf[hEggField->size + nSize] = '\0';
    
    memcpy(hEggField->buf + hEggField->size, lpValue, nSize);
    
    hEggField->size += nSize;

    return EGG_TRUE;
}


type_t EGGAPI eggField_get_type(HEGGFIELD hEggField)
{
    if (EGGFIELD_IS_INVALID(hEggField))
    {
        return EGG_NULL;
    }
    return hEggField->type;
}

echar* EGGAPI eggField_get_name(HEGGFIELD hEggField)
{
    if (EGGFIELD_IS_INVALID(hEggField))
    {
        return EGG_NULL;
    }
    return hEggField->name;
}

HEGGFIELD EGGAPI eggField_get_next(HEGGFIELD hEggField)
{
    if (EGGFIELD_IS_INVALID(hEggField))
    {
        return EGG_NULL;
    }
    return hEggField->next;
}


EBOOL EGGAPI eggField_set_next(HEGGFIELD hEggField, HEGGFIELD hOtherField)
{
    hEggField->next = hOtherField;
    return EGG_TRUE;
}


EBOOL  EGGAPI eggField_set_mask(HEGGFIELD hEggField, size16_t mask)
{
    if (EGGFIELD_IS_INVALID(hEggField))
    {
        return EGG_FALSE;
    }
    
    hEggField->mask = mask;
    
    return EGG_TRUE;
}


size16_t EGGAPI eggField_get_mask(HEGGFIELD hEggField)
{
    if (EGGFIELD_IS_INVALID(hEggField))
    {
        return 0;
    }

    return hEggField->mask;
}


size32_t EGGAPI eggField_get_size(HEGGFIELD hEggField)
{
    if (EGGFIELD_IS_INVALID(hEggField))
    {
        return EGG_NULL;
    }
    return hEggField->size;
}

echar* EGGAPI eggField_get_analyzername(HEGGFIELD hEggField)
{
    if (EGGFIELD_IS_INVALID(hEggField))
    {
        return EGG_NULL;
    }
    return hEggField->analyzerName;
}

EBOOL EGGAPI eggField_set_analyzername(HEGGFIELD hEggField, const echar* analyzerName)
{
    if (EGGFIELD_IS_INVALID(hEggField))
    {
        return EGG_FALSE;
    }
    if(hEggField->analyzerName)
    {
        free(hEggField->analyzerName);
    }
    hEggField->analyzerName = strdup(analyzerName);
    return EGG_TRUE;
}


EBOOL EGGAPI eggField_set_dictname(HEGGFIELD hEggField, const echar* dictName)
{
    if (EGGFIELD_IS_INVALID(hEggField))
    {
        return EGG_FALSE;
    }
    if(hEggField->dictName)
    {
        free(hEggField->dictName);
    }
    hEggField->dictName = strdup(dictName);
    hEggField->type |= EGG_OTHER_DICT;
    return EGG_TRUE;
}


char* EGGAPI eggField_get_dictname(HEGGFIELD hEggField)
{
    if (EGGFIELD_IS_INVALID(hEggField))
    {
        return EGG_NULL;
    }

    return hEggField->dictName;
}


EBOOL EGGAPI eggField_compress(HEGGFIELD hEggField)
{
    if (EGGFIELD_IS_INVALID(hEggField))
    {
        return EGG_FALSE;
    }


    if(hEggField->type & EGG_COMPRESSED || hEggField->size > COMPRESS_CRITICAL_VALUE)
    {
        epointer lp_compress_result = EGG_NULL;
        size32_t n_compress_size = 0;
        
        if(Uti_bz2_compress(hEggField->buf, hEggField->size, &lp_compress_result, &n_compress_size))
        {
            free(hEggField->buf);
            hEggField->buf = lp_compress_result;
            hEggField->size = n_compress_size;
            return EGG_TRUE;
        }
        else
        {
            hEggField->type |= EGG_NOT_COMPRESSED;
            return EGG_FALSE;
        }

    }
    return EGG_FALSE;
}



EBOOL EGGAPI eggField_decompress(HEGGFIELD hEggField)
{
    if (EGGFIELD_IS_INVALID(hEggField))
    {
        return EGG_FALSE;
    }


    if(hEggField->type & EGG_COMPRESSED)
    {
        epointer lp_decompress_result = EGG_NULL;
        size32_t n_decompress_size = 0;
        
        if(Uti_bz2_decompress(hEggField->buf, hEggField->size, &lp_decompress_result, &n_decompress_size))
        {
            free(hEggField->buf);
            hEggField->buf = lp_decompress_result;
            hEggField->size = n_decompress_size;
            return EGG_TRUE;
        }
        else
        {
            return EGG_FALSE;
        }
    }
    
    return EGG_TRUE;
}

size32_t EGGAPI eggField_get_serializationsize(HEGGFIELD hEggField)
{
    size32_t n_field_len = sizeof(EGGFIELDNODE) + strlen(hEggField->name) + 1 + hEggField->size;
    n_field_len += strlen(hEggField->analyzerName) + 1;
   if(hEggField->type & EGG_OTHER_DICT)
   {
       n_field_len += strlen(hEggField->dictName) + 1;
   }
   return n_field_len;
}

HEGGFIELDNODE EGGAPI eggField_serialization(HEGGFIELD hEggField)
{
    if (EGGFIELD_IS_INVALID(hEggField))
    {
        return EGG_NULL;
    }
    
    size32_t n_fieldNode_sz = eggField_get_serializationsize(hEggField);
    HEGGFIELDNODE lp_field_node = (HEGGFIELDNODE)malloc(n_fieldNode_sz);
    memset(lp_field_node, 0, n_fieldNode_sz);
    size32_t n_data_len  = 0;
    lp_field_node->size = n_fieldNode_sz;
    lp_field_node->type = hEggField->type;
    lp_field_node->bufOff = sizeof(EGGFIELDNODE);

    n_data_len = strlen(hEggField->name) + 1;
    memcpy((char*)lp_field_node + (unsigned long )lp_field_node->bufOff, hEggField->name, n_data_len);
    lp_field_node->bufOff += n_data_len;

    
    n_data_len = strlen(hEggField->analyzerName) + 1;
    memcpy((char*)lp_field_node + (unsigned long )lp_field_node->bufOff, hEggField->analyzerName, n_data_len);
    lp_field_node->bufOff += n_data_len;
    
    if(hEggField->type & EGG_OTHER_DICT)
    {
        n_data_len = strlen(hEggField->dictName) + 1;
        memcpy((char*)lp_field_node + (unsigned long )lp_field_node->bufOff, hEggField->dictName, n_data_len);
        lp_field_node->bufOff += n_data_len;
    }

    memcpy((char*)lp_field_node + (unsigned long )lp_field_node->bufOff, hEggField->buf, hEggField->size);
    
    return lp_field_node;
}


HEGGFIELD EGGAPI eggField_unserialization(HEGGFIELDNODE hFieldNode)
{
    if (!hFieldNode)
    {
        return EGG_NULL;
    }
    unsigned long  n_mem_off = sizeof(EGGFIELDNODE);
    size32_t n_data_len  = 0;
    HEGGFIELD lp_field = (HEGGFIELD) malloc(sizeof(EGGFIELD));
    memset(lp_field, 0, sizeof(EGGFIELD));
    lp_field->type = hFieldNode->type;

    lp_field->name = strdup((char*)hFieldNode + n_mem_off);
    n_mem_off += strlen((char*)hFieldNode + n_mem_off) + 1;
    
    lp_field->analyzerName = strdup((char*)hFieldNode + n_mem_off);
    n_mem_off += strlen((char*)hFieldNode + n_mem_off) + 1;
    
    if(hFieldNode->type & EGG_OTHER_DICT)
    {
        
        lp_field->dictName = strdup((char*)hFieldNode + n_mem_off);
        n_mem_off += strlen((char*)hFieldNode + n_mem_off) + 1;
        
    }
    lp_field->size = hFieldNode->size - n_mem_off;
    lp_field->buf = (char*)malloc(hFieldNode->size);
    memcpy(lp_field->buf, (char*)hFieldNode + n_mem_off, lp_field->size);

    return lp_field;
    
}





