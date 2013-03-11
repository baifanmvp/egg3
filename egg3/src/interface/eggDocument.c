#include "eggDocument.h"
struct eggDocument
{
    HEGGFIELD head;
    count_t count;
    int weight;
};
static flag_t g_doc_compress = EGG_FALSE;
extern flag_t g_doc_storage = EGG_FALSE;
#define EGGDOCUMENT_IS_INVALID(hEggDocument)    \
    (!hEggDocument? EGG_TRUE : EGG_FALSE)

PRIVATE HEGGDOCNODE  eggDocNode_compress(HEGGDOCNODE hEggDocNode);

PRIVATE HEGGDOCNODE  eggDocNode_decompress(HEGGDOCNODE hEggDocNode);



HEGGDOCUMENT EGGAPI eggDocument_new()
{
    EGGDOCUMENT* lp_egg_doc = (EGGDOCUMENT*)malloc(sizeof(EGGDOCUMENT));
    
    if (EGGDOCUMENT_IS_INVALID(lp_egg_doc))
    {
        return EGG_NULL;
    }
    
    memset(lp_egg_doc, 0, sizeof(EGGDOCUMENT));

    return lp_egg_doc;
}

EBOOL EGGAPI eggDocument_delete(HEGGDOCUMENT hEggDocument)
{
    if (EGGDOCUMENT_IS_INVALID(hEggDocument))
    {
        return EGG_FALSE;
    }
    HEGGFIELD lp_field_iter = hEggDocument->head;
    
    while(lp_field_iter)
    {
        HEGGFIELD lp_field_tmp = lp_field_iter;
        
        lp_field_iter = eggField_get_next(lp_field_iter);
        
        eggField_delete(lp_field_tmp);
    }

    free(hEggDocument);
    
    hEggDocument = EGG_NULL;
    
    return EGG_TRUE;
}

EBOOL EGGAPI eggDocument_add(HEGGDOCUMENT hEggDocument, HEGGFIELD hEggField)
{
    if (EGGDOCUMENT_IS_INVALID(hEggDocument))
    {
        return EGG_FALSE;
    }
    if(!hEggField)
    {
        return EGG_FALSE;
    }
    
    HEGGFIELD lp_tmp_field = hEggDocument->head;

    eggField_set_next(hEggField, lp_tmp_field);
    
    hEggDocument->head = hEggField;
    hEggDocument->count++;
    
    return EGG_TRUE;
}

HEGGFIELD EGGAPI eggDocument_get_field(HEGGDOCUMENT hEggDocument, const echar* lpNameField)
{
    if (EGGDOCUMENT_IS_INVALID(hEggDocument))
    {
        return EGG_NULL;
    }
    
    if (lpNameField == EGG_NULL)
    {
        return hEggDocument->head;
    }


    HEGGFIELD lp_field_iter = hEggDocument->head;
    count_t n_field_cnt = hEggDocument->count;

    length_t nLen = strlen(lpNameField);
    while (n_field_cnt--)
    {
        length_t n_len = strlen(eggField_get_name(lp_field_iter));
        if (nLen == n_len)
        {
            if (memcmp(lpNameField, eggField_get_name(lp_field_iter), nLen) == 0)
            {
                // find it
                return lp_field_iter;
            }
        }
        lp_field_iter =  eggField_get_next(lp_field_iter);
    }

    return EGG_NULL;
}




size32_t EGGAPI eggDocument_get_doc(HEGGDOCUMENT hEggDocument, const echar* lpNameField, echar** lppDoc)
{
    if (EGGDOCUMENT_IS_INVALID(hEggDocument))
    {
        return EGG_FALSE;
    }

    HEGGFIELD lp_egg_field;
    size32_t n_value_sz = 0;
    if (lpNameField == EGG_NULL)
    {
        return EGG_FALSE;
    }
    else
    {
        lp_egg_field = eggDocument_get_field(hEggDocument, lpNameField);
        if (lp_egg_field == EGG_NULL)
        {
            *lppDoc = EGG_NULL;
            return 0;
        }
        
        echar* lp_value = eggField_get_value(lp_egg_field, (size32_t*)&n_value_sz);
            
        *lppDoc = (echar*)malloc(n_value_sz + 8);
        memcpy(*lppDoc,
               lp_value,
               n_value_sz);
        *(*lppDoc + n_value_sz) = '\0';
    }
    
    return n_value_sz;
}


EBOOL EGGAPI eggDocument_set_doc(HEGGDOCUMENT hEggDocument,
                                 const echar* lpNameField,
                                 const echar* lpDoc, size32_t nDocSize)
{
    if (EGGDOCUMENT_IS_INVALID(hEggDocument))
    {
        return EGG_FALSE;
    }

    if (lpNameField == EGG_NULL)
    {
        return EGG_FALSE;
    }
    else
    {
        HEGGFIELD lp_egg_field = eggDocument_get_field(hEggDocument, lpNameField);
        
        if (lp_egg_field == EGG_NULL)
        {
            return EGG_FALSE;
        }
        
        eggField_set_value(lp_egg_field, lpDoc, nDocSize);
    }

    return EGG_TRUE;
}

EBOOL EGGAPI eggDocument_set_weight(HEGGDOCUMENT hEggDocument, int weight)
{
    if (EGGDOCUMENT_IS_INVALID(hEggDocument))
    {
        return EGG_FALSE;
    }
    
    hEggDocument->weight = weight;
    
    return EGG_TRUE;
}

int EGGAPI eggDocument_get_weight(HEGGDOCUMENT hEggDocument)
{
    if (EGGDOCUMENT_IS_INVALID(hEggDocument))
    {
        return EGG_FALSE;
    }
    
    return hEggDocument->weight;
    
}


HEGGDOCNODE EGGAPI eggDocument_serialization(HEGGDOCUMENT hEggDocument)
{
    if (EGGDOCUMENT_IS_INVALID(hEggDocument))
    {
        return EGG_FALSE;
    }

    HEGGFIELD lp_field_iter = EGG_NULL;
    HEGGFIELD lp_field_head = hEggDocument->head;
    
    size32_t n_docNode_sz = 0;
    count_t n_docNode_cnt = 0;

    count_t count = hEggDocument->count;
    while (count--)
    {
        lp_field_iter = hEggDocument->head;
        
        if(g_doc_storage || eggField_get_type(lp_field_iter) & EGG_STORAGE)
        {
            n_docNode_sz +=  eggField_get_serializationsize(lp_field_iter);
            n_docNode_cnt++;
        }
        
        hEggDocument->head = eggField_get_next(lp_field_iter);
    
    }
    HEGGDOCNODE lp_doc_node = (HEGGDOCNODE)malloc(sizeof(EGGDOCNODE) + n_docNode_sz);
    
    echar* lp_iter_pointer = (echar*)(lp_doc_node + 1);
    
    lp_doc_node->count = n_docNode_cnt;
    lp_doc_node->type = EGG_NOT_COMPRESSED;
    lp_doc_node->size = sizeof(EGGDOCNODE) + n_docNode_sz;
    lp_doc_node->weight = hEggDocument->weight;
    count = hEggDocument->count;
    hEggDocument->head = lp_field_head;

    while (count--)
    {
        lp_field_iter = hEggDocument->head;
        
        if(g_doc_storage || eggField_get_type(lp_field_iter) & EGG_STORAGE)
        {
             HEGGFIELDNODE lp_field_node =  eggField_serialization(lp_field_iter);
            memcpy(lp_iter_pointer, lp_field_node, lp_field_node->size);
            lp_iter_pointer += lp_field_node->size;
            free(lp_field_node);
        }
        
        hEggDocument->head = eggField_get_next(lp_field_iter);
    
    }
    
    hEggDocument->head = lp_field_head;
    
    if(g_doc_compress)
        lp_doc_node = eggDocNode_compress(lp_doc_node);
            
    return lp_doc_node;
}

HEGGDOCUMENT EGGAPI eggDocument_unserialization(HEGGDOCNODE hEggFieldNode)
{
    if (!hEggFieldNode)
    {
        return EGG_FALSE;
    }
    if(g_doc_compress)
        hEggFieldNode = eggDocNode_decompress(hEggFieldNode);

    HEGGDOCUMENT lp_egg_doc = eggDocument_new();
    echar* lp_iter_pointer = (echar*)(hEggFieldNode + 1);
    count_t count = hEggFieldNode->count;
    lp_egg_doc->weight = hEggFieldNode->weight;
    
    while(count--)
    {
        HEGGFIELD lp_egg_field = eggField_unserialization((HEGGFIELDNODE)lp_iter_pointer);
        eggDocument_add(lp_egg_doc, lp_egg_field);
        lp_iter_pointer += ((HEGGFIELDNODE)lp_iter_pointer)->size;
    }
    free(hEggFieldNode);
    return lp_egg_doc;

}

HEGGFIELD EGGAPI eggDocument_remove_field_byname(HEGGDOCUMENT hEggDocument, const echar* lpNameField)
{
    if (EGGDOCUMENT_IS_INVALID(hEggDocument))
    {
        return EGG_NULL;
    }
    
    if (lpNameField == EGG_NULL)
    {
        return EGG_NULL;
    }


    HEGGFIELD lp_field_iter = hEggDocument->head;
    count_t n_field_cnt = hEggDocument->count;
    HEGGFIELD lp_field_last = hEggDocument->head;

    length_t nLen = strlen(lpNameField);
    while (n_field_cnt--)
    {
        length_t n_len = strlen(eggField_get_name(lp_field_iter));
        if (nLen == n_len)
        {
            if (memcmp(lpNameField, eggField_get_name(lp_field_iter), nLen) == 0)
            {
                // find it
                if((unsigned long)lp_field_iter == (unsigned long)(hEggDocument->head))
                {
                    hEggDocument->head = eggField_get_next(hEggDocument->head);
                }
                else
                {
                    eggField_set_next(lp_field_last, eggField_get_next(lp_field_iter));
                }
                hEggDocument->count--;
                return lp_field_iter;
            }
        }
        
        lp_field_last = lp_field_iter;
        lp_field_iter =  eggField_get_next(lp_field_iter);
    }

    return EGG_NULL;
    
}


PRIVATE HEGGDOCNODE  eggDocNode_compress(HEGGDOCNODE hEggDocNode)
{
    epointer lp_result = EGG_NULL;
    size32_t size = 0;
    
    if(Uti_bz2_compress(hEggDocNode + 1, hEggDocNode->size - sizeof(EGGDOCNODE), &lp_result, &size) &&
        size < hEggDocNode->size - sizeof(EGGDOCNODE))
    {
        hEggDocNode->type = EGG_COMPRESSED;
        memcpy(hEggDocNode + 1, lp_result, size);
        hEggDocNode->size = size + sizeof(EGGDOCNODE);
    }
    else
    {
        hEggDocNode->type = EGG_NOT_COMPRESSED;        
        
    }

    if(lp_result)
    {
        free(lp_result);
    }

    return hEggDocNode;
}

PRIVATE HEGGDOCNODE  eggDocNode_decompress(HEGGDOCNODE hEggDocNode)
{
    if(hEggDocNode->type == EGG_COMPRESSED)
    {
        epointer lp_result = EGG_NULL;
        size32_t size = 0;
        
        if(Uti_bz2_decompress(hEggDocNode + 1, hEggDocNode->size - sizeof(hEggDocNode),
                              &lp_result, &size))
        {
            hEggDocNode = realloc(hEggDocNode, size + sizeof(EGGDOCNODE));
            
            memcpy(hEggDocNode + 1, lp_result, size);
            hEggDocNode->type = EGG_NOT_COMPRESSED;
            hEggDocNode->size = size + sizeof(EGGDOCNODE);
            free(lp_result);
        }
        else
        {
//            free(lp_result);
            free(hEggDocNode);
            return EGG_NULL;
        }
    }

    return hEggDocNode;

}
