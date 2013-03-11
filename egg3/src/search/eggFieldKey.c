#include "../eggFieldKey.h"
#include "../log/eggPrtLog.h"
#include <assert.h>

HEGGFIELDKEY eggFieldKey_new(char *fieldName, char *key, size32_t keySz)
{
    if (!fieldName || !fieldName[0] || !key || keySz == 0)
    {
        return NULL;
    }
    HEGGFIELDKEY hEggFieldKey;
    hEggFieldKey = (HEGGFIELDKEY)calloc(1, sizeof(EGGFIELDKEY));
    hEggFieldKey->fieldName = strdup(fieldName);
    assert(hEggFieldKey->fieldName);
    hEggFieldKey->keySz = keySz;
    hEggFieldKey->key = malloc(keySz);
    assert(hEggFieldKey->key);
    memcpy(hEggFieldKey->key, key, keySz);
    return hEggFieldKey;
}
HEGGFIELDKEY eggFieldKey_append(HEGGFIELDKEY head, HEGGFIELDKEY tail)
{
    if (!head)
    {
        return tail;
    }
    if (!tail)
    {
        return head;
    }

    HEGGFIELDKEY pFieldKey = head;
    while(pFieldKey->next)
        pFieldKey = pFieldKey->next;
    pFieldKey->next = tail;
    return head;
}
HEGGFIELDKEY eggFieldKey_dup(HEGGFIELDKEY hEggFieldKey)
{
    if (!hEggFieldKey)
    {
        return NULL;
    }
    HEGGFIELDKEY pFieldKey, newFieldKey, qFieldKey;
    pFieldKey = hEggFieldKey;
    newFieldKey = (HEGGFIELDKEY)calloc(1, sizeof(EGGFIELDKEY));
    newFieldKey->fieldName = strdup(pFieldKey->fieldName);
    assert(newFieldKey->fieldName);
    newFieldKey->keySz = pFieldKey->keySz;
    newFieldKey->key = malloc(newFieldKey->keySz);
    assert(newFieldKey->key);
    memcpy(newFieldKey->key, pFieldKey->key, pFieldKey->keySz);
    pFieldKey = pFieldKey->next;
    qFieldKey = newFieldKey;
    while (pFieldKey)
    {
        HEGGFIELDKEY ptmp;
        ptmp = (HEGGFIELDKEY)calloc(1, sizeof(EGGFIELDKEY));
        ptmp->fieldName = strdup(pFieldKey->fieldName);
        assert(ptmp->fieldName);
        ptmp->keySz = pFieldKey->keySz;
        ptmp->key = malloc(ptmp->keySz);
        assert(ptmp->key);
        memcpy(ptmp->key, pFieldKey->key, pFieldKey->keySz);
        
        qFieldKey->next = ptmp;
        qFieldKey = qFieldKey->next;
        pFieldKey = pFieldKey->next;
    }
    return newFieldKey;
}
EBOOL eggFieldKey_equal(HEGGFIELDKEY hEggFieldKey, HEGGFIELDKEY hEggFieldKey2)
{
    if (hEggFieldKey == hEggFieldKey2)
    {
        return EGG_TRUE;
    }
    if (!hEggFieldKey || !hEggFieldKey2)
    {
        return EGG_FALSE;
    }
    
    HEGGFIELDKEY pFieldKey, qFieldKey;
    
    int nFieldKey, nFieldKey2;
    nFieldKey = nFieldKey2 = 0;
    for (pFieldKey = hEggFieldKey; pFieldKey; pFieldKey = pFieldKey->next)
    {
        nFieldKey++;
    }
    for (qFieldKey = hEggFieldKey2; qFieldKey; qFieldKey = qFieldKey->next)
    {
        nFieldKey2++;
    }
    if (nFieldKey != nFieldKey2)
    {
        return EGG_FALSE;
    }
    
    
    for (pFieldKey = hEggFieldKey; pFieldKey; pFieldKey = pFieldKey->next)
    {
        for (qFieldKey = hEggFieldKey2; qFieldKey; qFieldKey = qFieldKey->next)
        {
            if (qFieldKey->fieldName == pFieldKey->fieldName)
            {
                ;
            }
            else if (strcmp(qFieldKey->fieldName, pFieldKey->fieldName) == 0)
            {
                ;
            }
            else
            {
                continue;
            }

            if (qFieldKey->keySz == pFieldKey->keySz)
            {
                ;
            }
            else
            {
                continue;
            }

            if (qFieldKey->key == pFieldKey->key)
            {
                ;
            }
            else if (qFieldKey->keySz == 0)
            {
                ;
            }
            else if (qFieldKey->key && pFieldKey->key
                     && memcmp(qFieldKey->key, pFieldKey->key, qFieldKey->keySz) == 0)
            {
                ;
            }
            else
            {
                continue;
            }
            
            break;
                    
        }
        
        if (!qFieldKey)
        {
            return EGG_FALSE;
        }
    }
    
    return EGG_TRUE;
}
void eggFieldKey_del(HEGGFIELDKEY hEggFieldKey)
{
    HEGGFIELDKEY pFieldKey = hEggFieldKey;
    while (pFieldKey)
    {
        HEGGFIELDKEY ptmp;
        ptmp = pFieldKey;
        pFieldKey = ptmp->next;
        free(ptmp->fieldName);
        free(ptmp->key);
        free(ptmp);
    }
    return ;
}
char * eggFieldKey_serialise(HEGGFIELDKEY hEggFieldKey, size32_t *sz)
{
    *sz = 0;
    size32_t count = 0;
    HEGGFIELDKEY head;
    head = hEggFieldKey;
    while (hEggFieldKey)
    {
        *sz += strlen(hEggFieldKey->fieldName) + 1;
        *sz += hEggFieldKey->keySz;
        *sz += sizeof(hEggFieldKey->keySz);
        count++;
        hEggFieldKey = hEggFieldKey->next;
    }
    char *p, *buf;
    *sz += sizeof(size32_t) + sizeof(size32_t);
    buf = p = malloc(*sz);
    assert(p);
    *(size32_t *)p = *sz;
    p += sizeof(size32_t);
    *(size32_t *)p = count;
    p += sizeof(size32_t);
    hEggFieldKey = head;
    while (hEggFieldKey)
    {
        strcpy(p, hEggFieldKey->fieldName);
        p += strlen(p) + 1;
        *(size32_t *)p = hEggFieldKey->keySz;
        p += sizeof(hEggFieldKey->keySz);
        memcpy(p, hEggFieldKey->key, hEggFieldKey->keySz);
        p += hEggFieldKey->keySz;
        
        hEggFieldKey = hEggFieldKey->next;
    }
    
    return buf;
}
HEGGFIELDKEY eggFieldKey_unserialise(char *buf)
{
    HEGGFIELDKEY head, hEggFieldKey;
    char *end;
    size32_t size, count;
    size = *(size32_t *)buf;
    end = buf + size;
    buf += sizeof(size32_t);
    if (size <= 2 * sizeof(size32_t))
    {
        return NULL;
    }    
    count = *(size32_t *)buf;
    buf += sizeof(size32_t);
    if (count == 0)
    {
        return NULL;
    }
    head = (HEGGFIELDKEY)calloc(1, sizeof(EGGFIELDKEY));
    assert(head);
    head->fieldName = strdup(buf);
    assert(head->fieldName);
    buf += strlen(buf) + 1;
    head->keySz = *(size32_t*)buf;
    buf += sizeof(size32_t);
    head->key = malloc(head->keySz);
    assert(head->key);
    memcpy(head->key, buf, head->keySz);
    buf += head->keySz;
    hEggFieldKey = head;
    while (--count)
    {
        HEGGFIELDKEY ptmp;

        ptmp = (HEGGFIELDKEY)calloc(1, sizeof(EGGFIELDKEY));
        ptmp->fieldName = strdup(buf);
        assert(ptmp->fieldName);
        buf += strlen(buf) + 1;
        ptmp->keySz = *(size32_t*)buf;
        buf += sizeof(size32_t);
        ptmp->key = malloc(ptmp->keySz);
        assert(ptmp->key);
        memcpy(ptmp->key, buf, ptmp->keySz);
        buf += ptmp->keySz;

        hEggFieldKey->next = ptmp;
        hEggFieldKey = hEggFieldKey->next;
    }
    if (buf > end)
    {
        //printf("%s: error memory corrupt\n", __func__);
        eggPrtLog_error("eggFieldKey", "%s: error memory corrupt\n", __func__);
    }
    return head;
}
