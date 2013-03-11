#include "../eggQuery.h"
#include "../eggFieldKey.h"
#include "../EggDef.h"
#include "../eggAnalyzer.h"
#include "../log/eggPrtLog.h"
#include <assert.h>
#include <stddef.h>
#include <strings.h>
#include <float.h>
#include <stdint.h>

#define EGGQUERYOPT_DATA 0

#define EGGQUERYOPT_OR 16
#define EGGQUERYOPT_AND 17
#define EGGQUERYOPT_ANDPHRASE 18
#define EGGQUERYOPT_MINUS 19

struct eggQuery
{
    type_t opt;
    union {
        struct {
            char *fieldName;
            void *key1;
            size16_t key1Sz;
            void *key2;
            size16_t key2Sz;
            char *analyzerName;
            char *dictName;
        };
        struct {
            HEGGFIELDKEY hFieldKey;
            HEGGIDNODE hIdNodes;
            count_t cnt;
        };
    };
    eggBoost_t boost;
    
    struct eggQuery *p;
    struct eggQuery *q;
    
};
typedef struct eggQuery eggQuery;

#define ALLOC_QUERY(p) (((p) = calloc(1, sizeof(struct eggQuery))), assert((p)))
#define FREE_QUERY(p) (free(p), p = 0)

HEGGQUERY EGGAPI eggQuery_new_string(const char* fieldName, const echar* keyword, size16_t keywordSz, char *analyzerName)
{
    if (!keyword || keywordSz == 0)
    {
        return NULL;
    }
    HEGGQUERY root = NULL;

    ALLOC_QUERY(root);
    if (keywordSz > 3 && strncmp(keyword, "$$$", 3) == 0)
    {
        keywordSz -= 3;
        keyword += 3;
        
        root->opt = EGGQUERYOPT_TERMOR;
        root->fieldName = strdup(fieldName);
        assert(root->fieldName); 
        root->key1Sz = keywordSz;
        root->key1 = malloc(root->key1Sz);
        assert(root->key1); 
        memcpy(root->key1, keyword, root->key1Sz);
        root->key2 = 0; 
        root->key2Sz = 0;
    }
    else
    {
        root->opt = EGGQUERYOPT_TERM;
        root->fieldName = strdup(fieldName); 
        assert(root->fieldName); 
        root->key1Sz = keywordSz; 
        root->key1 = malloc(keywordSz); 
        assert(root->key1); 
        memcpy(root->key1, keyword, keywordSz); 
        root->key2 = 0; 
        root->key2Sz = 0;
    }
    if (!analyzerName || !analyzerName[0])
    {
        root->analyzerName = 0;
    }
    else
    {
        root->analyzerName = strdup(analyzerName);
        assert(root->analyzerName);
    }
    return root;
}

HEGGQUERY EGGAPI eggQuery_set_dictname(HEGGQUERY hQuery, char *dictName)
{
    if (!dictName || !dictName[0] || !hQuery)
    {
        return hQuery;
    }

    if (hQuery->opt == EGGQUERYOPT_TERM
        || hQuery->opt == EGGQUERYOPT_TERMOR             
        || hQuery->opt == EGGQUERYOPT_RANGE
        || hQuery->opt == EGGQUERYOPT_PHRASE)
    {
        if (hQuery->analyzerName)
        {
            free(hQuery->dictName);
            hQuery->dictName = strdup(dictName);
            assert(hQuery->dictName);
        }
    }
    else if (hQuery->opt == EGGQUERYOPT_OR
             || hQuery->opt == EGGQUERYOPT_AND
             || hQuery->opt == EGGQUERYOPT_ANDPHRASE
             || hQuery->opt == EGGQUERYOPT_MINUS)
    {
        eggQuery_set_dictname(hQuery->p, dictName);
        eggQuery_set_dictname(hQuery->q, dictName);
    }


    return hQuery;
}

HEGGQUERY EGGAPI eggQuery_new_phrase(const char* fieldName, const echar* keyword, size16_t keywordSz, char *analyzerName)
{
    if (!keyword || keywordSz == 0)
    {
        return NULL;
    }
    HEGGQUERY root = NULL;

    ALLOC_QUERY(root); 
    root->opt = EGGQUERYOPT_PHRASE;
    root->fieldName = strdup(fieldName); 
    assert(root->fieldName); 
    root->key1Sz = keywordSz; 
    root->key1 = malloc(keywordSz); 
    assert(root->key1); 
    memcpy(root->key1, keyword, keywordSz); 
    root->key2 = 0; 
    root->key2Sz = 0;
    if (!analyzerName || !analyzerName[0])
    {
        root->analyzerName = 0;
    }
    else
    {
        root->analyzerName = strdup(analyzerName);
        assert(root->analyzerName);
    }
    return root;
}

HEGGQUERY EGGAPI eggQuery_new_sentence(const char* fieldName, const echar* keyword, size16_t keywordSz, char *analyzerName)
{
    if (!keyword || keywordSz == 0)
    {
        return NULL;
    }
    HEGGQUERY root = NULL;
    
    if (keywordSz == 1)
    {
        root = eggQuery_new_string(fieldName, keyword, keywordSz, analyzerName);
        return root;
    }
    int inqumark = 0;
    const char *after_qumark = keyword;
    const char *p;
    if (keyword[0] == '"')
    {
        inqumark = 1;
        after_qumark = p = keyword + 1;
    }
    else
    {
        inqumark = 0;
        after_qumark = p = keyword;
    }
    while (p < keyword + keywordSz)
    {
        if (p[0] == '"' && p[-1] != '\\')
        {
            if (!inqumark)
            {
                HEGGQUERY hQuery;
                hQuery = eggQuery_new_string(fieldName, after_qumark, p - after_qumark,
                                             analyzerName);
                if (root)
                {
                    root = eggQuery_and(root, hQuery);
                }
                else
                {
                    root = hQuery;
                }
                inqumark = 1;
                after_qumark = p + 1;
            }
            else
            {
                HEGGQUERY hQuery;
                hQuery = eggQuery_new_phrase(fieldName, after_qumark, p - after_qumark,
                                             analyzerName);
                if (root)
                {
                    root = eggQuery_and(root, hQuery);
                }
                else
                {
                    root = hQuery;
                }
                inqumark = 0;
                after_qumark = p + 1;                
            }
        }
        p++;
    }
    if (!inqumark)
    {
        HEGGQUERY hQuery;
        hQuery = eggQuery_new_string(fieldName, after_qumark, p - after_qumark,
                                     analyzerName);
        if (root)
        {
            root = eggQuery_and(root, hQuery);
        }
        else
        {
            root = hQuery;
        }
    }
    else
    {
        HEGGQUERY hQuery;
        hQuery = eggQuery_new_phrase(fieldName, after_qumark, p - after_qumark,
                                     analyzerName);
        if (root)
        {
            root = eggQuery_and(root, hQuery);            
        }
        else
        {
            root = hQuery;
        }
    }

    return root;
}

HEGGQUERY EGGAPI eggQuery_new_string_bytype(const char* fieldName, const echar* keyword, size16_t keywordSz, type_t type, ...)
{
    char *analyzerName = NULL;
        
    if (type & EGG_NOT_ANALYZED)
    {
        analyzerName = NULL;
    }
    else if (type & EGG_CN_ANALYZED)
    {
        analyzerName = ANALYZER_CNLEX;
    }
    else if (type & EGG_CY_ANALYZED)
    {
        analyzerName = ANALYZER_CYLEX;
    }
    else if (type & EGG_CWS_ANALYZED)
    {
        analyzerName = ANALYZER_CWSLEX;
    }
    else if (type & EGG_OTHER_ANALYZED)
    {
        va_list ap_arg;
        va_start(ap_arg, type);
        analyzerName = va_arg(ap_arg, char*);
        va_end(ap_arg);
    }
    else
    {
        analyzerName = NULL;
    }
    return eggQuery_new_string(fieldName, keyword, keywordSz, analyzerName);
}

HEGGQUERY EGGAPI eggQuery_new_phrase_bytype(const char* fieldName, const echar* keyword, size16_t keywordSz, type_t type, ...)
{
    char *analyzerName = NULL;
        
    if (type & EGG_NOT_ANALYZED)
    {
        analyzerName = NULL;
    }
    else if (type & EGG_CN_ANALYZED)
    {
        analyzerName = ANALYZER_CNLEX;
    }
    else if (type & EGG_CY_ANALYZED)
    {
        analyzerName = ANALYZER_CYLEX;
    }
    else if (type & EGG_CWS_ANALYZED)
    {
        analyzerName = ANALYZER_CWSLEX;
    }
    else if (type & EGG_OTHER_ANALYZED)
    {
        va_list ap_arg;
        va_start(ap_arg, type);
        analyzerName = va_arg(ap_arg, char*);
        va_end(ap_arg);
    }
    else
    {
        analyzerName = NULL;
    }
    return eggQuery_new_phrase(fieldName, keyword, keywordSz, analyzerName);
}

HEGGQUERY EGGAPI eggQuery_new_sentence_bytype(const char* fieldName, const echar* keyword, size16_t keywordSz, type_t type, ...)
{
    char *analyzerName = NULL;
        
    if (type & EGG_NOT_ANALYZED)
    {
        analyzerName = NULL;
    }
    else if (type & EGG_CN_ANALYZED)
    {
        analyzerName = ANALYZER_CNLEX;
    }
    else if (type & EGG_CY_ANALYZED)
    {
        analyzerName = ANALYZER_CYLEX;
    }
    else if (type & EGG_CWS_ANALYZED)
    {
        analyzerName = ANALYZER_CWSLEX;
    }
    else if (type & EGG_OTHER_ANALYZED)
    {
        va_list ap_arg;
        va_start(ap_arg, type);
        analyzerName = va_arg(ap_arg, char*);
        va_end(ap_arg);
    }
    else
    {
        analyzerName = NULL;
    }
    return eggQuery_new_sentence(fieldName, keyword, keywordSz, analyzerName);
}


HEGGQUERY EGGAPI eggQuery_new_int32(const char* fieldName, int32_t number1)
{
    HEGGQUERY query;
    ALLOC_QUERY(query);
    query->opt = EGGQUERYOPT_TERM;
    query->fieldName = strdup(fieldName);
    query->key1 = malloc(4);
    assert(query->key1);
    *(int32_t *)query->key1 = number1;
    query->key1Sz = 4;

    return query;
}

HEGGQUERY EGGAPI eggQuery_new_int64(const char* fieldName, int64_t number1)
{
    HEGGQUERY query;
    ALLOC_QUERY(query);
    query->opt = EGGQUERYOPT_TERM;
    query->fieldName = strdup(fieldName);
    query->key1 = malloc(8);
    assert(query->key1);
    *(int64_t *)query->key1 = number1;
    query->key1Sz = 8;

    return query;
}

HEGGQUERY EGGAPI eggQuery_new_double(const char* fieldName, double number1)
{
    HEGGQUERY query;
    ALLOC_QUERY(query);
    query->opt = EGGQUERYOPT_TERM;
    query->fieldName = strdup(fieldName);
    query->key1 = malloc(8);
    assert(query->key1);
    *(double *)query->key1 = number1;
    query->key1Sz = 8;

    return query;
}

HEGGQUERY EGGAPI eggQuery_new_doublerange(const char* fieldName, double number1, double number2)
{
    if (number1 > number2)
    {
        return NULL;
    }

    HEGGQUERY query;
    ALLOC_QUERY(query);
    query->opt = EGGQUERYOPT_RANGE;
    query->fieldName = strdup(fieldName);
    query->key1 = malloc(8);
    assert(query->key1);
    *(double *)query->key1 = number1;
    query->key1Sz = 8;
    query->key2 = malloc(8);
    assert(query->key2);
    *(double *)query->key2 = number2;
    query->key2Sz = 8;

    return query;
}
HEGGQUERY EGGAPI eggQuery_new_doublerange_en(const char* fieldName, double number1, double number2)
{
    if (number2 == DBL_MIN)
    {
        return NULL;
    }

    double number2l;
    number2l = number2 - DBL_EPSILON;
    return eggQuery_new_doublerange(fieldName, number1, number2l);
}
HEGGQUERY EGGAPI eggQuery_new_doublerange_nn(const char* fieldName, double number1, double number2)
{
    if (number1 == DBL_MAX)
    {
        return NULL;
    }
    if (number2 == DBL_MIN)
    {
        return NULL;
    }       
    double number1g, number2l;
    number1g = number1 + DBL_EPSILON;
    number2l = number2 - DBL_EPSILON;
    return eggQuery_new_doublerange(fieldName, number1g, number2l);
}
HEGGQUERY EGGAPI eggQuery_new_doublerange_ne(const char* fieldName, double number1, double number2)
{
    if (number1 == DBL_MAX)
    {
        return NULL;
    }
    double number1g;
    number1g = number1 + DBL_EPSILON;
    return eggQuery_new_doublerange(fieldName, number1g, number2);
}
HEGGQUERY EGGAPI eggQuery_new_doublerange_gt(const char* fieldName, double number1)
{
    if (number1 == DBL_MAX)
    {
        return NULL;
    }
    double number1g, number2;
    number1g = number1 + DBL_EPSILON;
    number2 = DBL_MAX;
    return eggQuery_new_doublerange(fieldName, number1g, number2);
}
HEGGQUERY EGGAPI eggQuery_new_doublerange_ge(const char* fieldName, double number1)
{
    double number2;
    number2 = DBL_MAX;
    return eggQuery_new_doublerange(fieldName, number1, number2);
}
HEGGQUERY EGGAPI eggQuery_new_doublerange_lt(const char* fieldName, double number2)
{
    if (number2 == DBL_MIN)
    {
        return NULL;
    }
    double number1, number2l;
    number1 = DBL_MIN;
    number2l = number2 - DBL_EPSILON;
    return eggQuery_new_doublerange(fieldName, number1, number2l);
}
HEGGQUERY EGGAPI eggQuery_new_doublerange_le(const char* fieldName, double number2)
{
    double number1;
    number1 = DBL_MIN;
    return eggQuery_new_doublerange(fieldName, number1, number2);
}
HEGGQUERY EGGAPI eggQuery_new_doublerange_all(const char* fieldName)
{
    double number1, number2;
    number1 = DBL_MIN;
    number1 = DBL_MAX;    
    return eggQuery_new_doublerange(fieldName, number1, number2);
}

HEGGQUERY EGGAPI eggQuery_new_int32range(const char* fieldName, int32_t number1, int32_t number2)
{
    if (number1 > number2)
    {
        return NULL;
    }
    
    HEGGQUERY query;
    ALLOC_QUERY(query);
    query->opt = EGGQUERYOPT_RANGE;
    query->fieldName = strdup(fieldName);
    query->key1 = malloc(4);
    assert(query->key1);
    *(int32_t *)query->key1 = number1;
    query->key1Sz = 4;
    query->key2 = malloc(4);
    assert(query->key2);
    *(int32_t *)query->key2 = number2;
    query->key2Sz = 4;

    return query;
}
HEGGQUERY EGGAPI eggQuery_new_int32range_en(const char* fieldName, int32_t number1, int32_t number2)
{
    if (number2 == INT32_MIN)
    {
        return NULL;
    }
    int32_t number2l;
    number2l = number2 -1;
    return eggQuery_new_int32range(fieldName, number1, number2);
}
HEGGQUERY EGGAPI eggQuery_new_int32range_nn(const char* fieldName, int32_t number1, int32_t number2)
{
    if (number1 == INT32_MAX)
    {
        return NULL;
    }
    if (number2 == INT32_MIN)
    {
        return NULL;
    }
    int32_t number1g, number2l;
    number1g = number1 + 1;
    number2l = number2 -1;
    return eggQuery_new_int32range(fieldName, number1g, number2l);
}
HEGGQUERY EGGAPI eggQuery_new_int32range_ne(const char* fieldName, int32_t number1, int32_t number2)
{
    if (number1 == INT32_MAX)
    {
        return NULL;
    }
    int32_t number1g;
    number1g = number1 + 1;
    return eggQuery_new_int32range(fieldName, number1g, number2);
}
HEGGQUERY EGGAPI eggQuery_new_int32range_gt(const char* fieldName, int32_t number1)
{
    if (number1 == INT32_MAX)
    {
        return NULL;
    }
    int32_t number1g, number2;
    number1g = number1 + 1;
    number2 = INT32_MAX;
    return eggQuery_new_int32range(fieldName, number1g, number2);
}
HEGGQUERY EGGAPI eggQuery_new_int32range_ge(const char* fieldName, int32_t number1)
{
    int32_t number2;
    number2 = INT32_MAX;
    return eggQuery_new_int32range(fieldName, number1, number2);
}
HEGGQUERY EGGAPI eggQuery_new_int32range_lt(const char* fieldName, int32_t number2)
{
    if (number2 == INT32_MIN)
    {
        return NULL;
    }
    int32_t number1, number2l;
    number1 = INT32_MIN;
    number2l = number2 - 1;
    return eggQuery_new_int32range(fieldName, number1, number2l);
}
HEGGQUERY EGGAPI eggQuery_new_int32range_le(const char* fieldName, int32_t number2)
{
    int32_t number1;
    number1 = INT32_MIN;
    return eggQuery_new_int32range(fieldName, number1, number2);
}
HEGGQUERY EGGAPI eggQuery_new_int32range_all(const char* fieldName)
{
    int32_t number1, number2;
    number1 = INT32_MIN;
    number2 = INT32_MAX;    
    return eggQuery_new_int32range(fieldName, number1, number2);
}

HEGGQUERY EGGAPI eggQuery_new_int64range(const char* fieldName, int64_t number1, int64_t number2)
{
    if (number1 > number2)
    {
        return NULL;
    }

    HEGGQUERY query;
    ALLOC_QUERY(query);
    query->opt = EGGQUERYOPT_RANGE;
    query->fieldName = strdup(fieldName);
    query->key1 = malloc(8);
    assert(query->key1);
    *(int64_t *)query->key1 = number1;
    query->key1Sz = 8;
    query->key2 = malloc(8);
    assert(query->key2);
    *(int64_t *)query->key2 = number2;
    query->key2Sz = 8;

    return query;
}
HEGGQUERY EGGAPI eggQuery_new_int64range_en(const char* fieldName, int64_t number1, int64_t number2)
{
    if (number2 == INT64_MIN)
    {
        return NULL;
    }
    int64_t number2l;
    number2l = number2 - 1;
    return eggQuery_new_int64range(fieldName, number1, number2l);
}
HEGGQUERY EGGAPI eggQuery_new_int64range_nn(const char* fieldName, int64_t number1, int64_t number2)
{
    if (number1 == INT64_MAX)
    {
        return NULL;
    }
    if (number2 == INT64_MIN)
    {
        return NULL;
    }
    int64_t number1g, number2l;
    number1g = number1 + 1;
    number2l = number2 - 1;
    return eggQuery_new_int64range(fieldName, number1g, number2l);
}
HEGGQUERY EGGAPI eggQuery_new_int64range_ne(const char* fieldName, int64_t number1, int64_t number2)
{
    if (number1 == INT64_MAX)
    {
        return NULL;
    }
    int64_t number1g;
    number1g = number1 + 1;
    return eggQuery_new_int64range(fieldName, number1g, number2);
}
HEGGQUERY EGGAPI eggQuery_new_int64range_gt(const char* fieldName, int64_t number1)
{
    if (number1 == INT64_MAX)
    {
        return NULL;
    }
    int64_t number1g, number2;
    number1g = number1 + 1;
    number2 = INT64_MAX;
    return eggQuery_new_int64range(fieldName, number1g, number2);
}
HEGGQUERY EGGAPI eggQuery_new_int64range_ge(const char* fieldName, int64_t number1)
{
    int64_t number2;
    number2 = INT64_MAX;
    return eggQuery_new_int64range(fieldName, number1, number2);
}
HEGGQUERY EGGAPI eggQuery_new_int64range_lt(const char* fieldName, int64_t number2)
{
    if (number2 == INT64_MIN)
    {
        return NULL;
    }
    int64_t number1, number2l;
    number1 = INT64_MIN;
    number2l = number2 - 1;
    return eggQuery_new_int64range(fieldName, number1, number2l);
}
HEGGQUERY EGGAPI eggQuery_new_int64range_le(const char* fieldName, int64_t number2)
{
    int64_t number1;
    number1 = INT64_MIN;
    return eggQuery_new_int64range(fieldName, number1, number2);
}
HEGGQUERY EGGAPI eggQuery_new_int64range_all(const char* fieldName)
{
    int64_t number1, number2;
    number1 = INT64_MIN;
    number1 = INT64_MAX;    
    return eggQuery_new_int64range(fieldName, number1, number2);
}

HEGGQUERY EGGAPI eggQuery_new_stringrange(const char* fieldName, char *start, char *end)
{

    if (start && strlen(start) > EGG_BTREE_STRING_MAX)
    {
        /* fprintf(stderr, "%s:%d:%s [%s] too long, will be truncated to length %d\n", __func__, __LINE__, __FILE__, */
        /*         start, EGG_BTREE_STRING_MAX); */
        eggPrtLog_error("eggQuery", "%s:%d:%s [%s] too long, will be truncated to length %d\n", __func__, __LINE__, __FILE__,
                start, EGG_BTREE_STRING_MAX);

    }
    if (end && strlen(end) > EGG_BTREE_STRING_MAX)
    {
        /* fprintf(stderr, "%s:%d:%s [%s] too long, will be truncated to length %d\n", __func__, __LINE__, __FILE__, */
        /*         end, EGG_BTREE_STRING_MAX); */
        eggPrtLog_error("eggQuery", "%s:%d:%s [%s] too long, will be truncated to length %d\n", __func__, __LINE__, __FILE__,
                end, EGG_BTREE_STRING_MAX);
    }

    
    HEGGQUERY query;
    ALLOC_QUERY(query);
    query->opt = EGGQUERYOPT_RANGE;
    query->fieldName = strdup(fieldName);
    if (!start)
    {
        query->key1 = strdup("");
        assert(query->key1);        
        query->key1Sz = 0;
    }
    else
    {
        query->key1 = strdup(start);
        assert(query->key1);
        query->key1Sz = strlen(start);
        if (query->key1Sz > EGG_BTREE_STRING_MAX)
        {
            query->key1Sz = EGG_BTREE_STRING_MAX; /* truncate */
        }
    }
    if (!end)
    {
        query->key2 = strdup("");
        assert(query->key2);        
        query->key2Sz = 0;
    }
    else
    {
        query->key2 = strdup(end);
        assert(query->key2);
        query->key2Sz = strlen(end);
        if (query->key2Sz > EGG_BTREE_STRING_MAX)
        {
            query->key2Sz = EGG_BTREE_STRING_MAX; /* truncate */
        }
    }

    return query;
}

static unsigned char *string_minus_epsilon(const unsigned char *str, unsigned char *buf, size_t bufsz)
{
    if (!str || !str[0])
    {
        return NULL;
    }
    buf[bufsz-1] = '\0';
    int i;
    for (i = 0; i < bufsz - 1 && str[i]; i++)
    {
        buf[i] = str[i];
    }
    buf[i-1] = (unsigned char)str[i-1] - 1;
    for (; i < bufsz - 1; i++)
    {
        buf[i] = (unsigned char)'\377';
    }
    return buf;
}
static unsigned char *string_add_epsilon(const unsigned char *str, unsigned char *buf, size_t bufsz)
{
    buf[bufsz-1] = '\0';
    if (!str || !str[0])
    {
        buf[0] = '\001';
        buf[1] = '\0';
        return buf;
    }
    
    int i;
    for (i = 0; i < bufsz - 1 && str[i]; i++)
    {
        buf[i] = str[i];
    }
    if (i < bufsz - 1)
    {
        buf[i] = '\001';
        buf[i+1] = '\0';
    }
    else
    {
        for (i--; i >= 0; i--)
        {
            if (++buf[i])
            {
                buf[i+1] = '\0';
                break;
            }
        }
        if (i < 0)
        {
            return NULL;
        }
    }
    return buf;
}
HEGGQUERY EGGAPI eggQuery_new_stringrange_en(const char* fieldName, char *start, char *end)
{
    if (!end || !end[0])
    {
        return NULL;
    }

    if (start && strlen(start) > EGG_BTREE_STRING_MAX)
    {
        /* fprintf(stderr, "%s:%d:%s [%s] too long, will be truncated to length %d\n", __func__, __LINE__, __FILE__, */
        /*         start, EGG_BTREE_STRING_MAX); */
        eggPrtLog_error("eggQuery", "%s:%d:%s [%s] too long, will be truncated to length %d\n", __func__, __LINE__, __FILE__,
                start, EGG_BTREE_STRING_MAX);
        
    }
    if (end && strlen(end) > EGG_BTREE_STRING_MAX)
    {
        /* fprintf(stderr, "%s:%d:%s [%s] too long, will be truncated to length %d\n", __func__, __LINE__, __FILE__, */
        /*         end, EGG_BTREE_STRING_MAX); */
        eggPrtLog_error("eggQuery", "%s:%d:%s [%s] too long, will be truncated to length %d\n", __func__, __LINE__, __FILE__,
                end, EGG_BTREE_STRING_MAX);

    }
    char *end2;
    unsigned char buf[EGG_BTREE_STRING_MAX+1];
    end2 = string_minus_epsilon(end, buf, EGG_BTREE_STRING_MAX+1);

    return eggQuery_new_stringrange(fieldName, start, end2);
}
HEGGQUERY EGGAPI eggQuery_new_stringrange_nn(const char* fieldName, char *start, char *end)
{
    if (!end || !end[0])
    {
        return NULL;
    }

    if (start && strlen(start) > EGG_BTREE_STRING_MAX)
    {
        /* fprintf(stderr, "%s:%d:%s [%s] too long, will be truncated to length %d\n", __func__, __LINE__, __FILE__, */
        /*         start, EGG_BTREE_STRING_MAX); */
        eggPrtLog_error("eggQuery", "%s:%d:%s [%s] too long, will be truncated to length %d\n", __func__, __LINE__, __FILE__,
                start, EGG_BTREE_STRING_MAX);
    }

    if (strlen(end) > EGG_BTREE_STRING_MAX)
    {
        /* fprintf(stderr, "%s:%d:%s [%s] too long, will be truncated to length %d\n", __func__, __LINE__, __FILE__, */
        /*         end, EGG_BTREE_STRING_MAX); */
        eggPrtLog_error("eggQuery", "%s:%d:%s [%s] too long, will be truncated to length %d\n", __func__, __LINE__, __FILE__,
                end, EGG_BTREE_STRING_MAX);

    }
    
    int i;
    char *end2, *start2;
    
    unsigned char buf[EGG_BTREE_STRING_MAX+1];
    end2 = string_minus_epsilon(end, buf, EGG_BTREE_STRING_MAX+1);

    unsigned char buf2[EGG_BTREE_STRING_MAX+1];
    start2 = string_add_epsilon(start, buf2, EGG_BTREE_STRING_MAX+1);

    if (start2 == NULL)
    {
        return NULL;
    }
    return eggQuery_new_stringrange(fieldName, start2, end2);
}
HEGGQUERY EGGAPI eggQuery_new_stringrange_ne(const char* fieldName, char *start, char *end)
{
    if (!end || !end[0])
    {
        return NULL;
    }
    if (start && strlen(start) > EGG_BTREE_STRING_MAX)
    {
        /* fprintf(stderr, "%s:%d:%s [%s] too long, will be truncated to length %d\n", __func__, __LINE__, __FILE__, */
        /*         start, EGG_BTREE_STRING_MAX); */
        eggPrtLog_error("eggQuery", "%s:%d:%s [%s] too long, will be truncated to length %d\n", __func__, __LINE__, __FILE__,
                start, EGG_BTREE_STRING_MAX);

    }
    if (end && strlen(end) > EGG_BTREE_STRING_MAX)
    {
        /* fprintf(stderr, "%s:%d:%s [%s] too long, will be truncated to length %d\n", __func__, __LINE__, __FILE__, */
        /*         end, EGG_BTREE_STRING_MAX); */
        eggPrtLog_error("eggQuery", "%s:%d:%s [%s] too long, will be truncated to length %d\n", __func__, __LINE__, __FILE__,
                end, EGG_BTREE_STRING_MAX);

    }
    
    int i;

    char *start2;
    
    unsigned char buf[EGG_BTREE_STRING_MAX+1];
    start2 = string_add_epsilon(start, buf, EGG_BTREE_STRING_MAX+1);
    
    if (start2 == NULL)
    {
        return NULL;
    }

    return eggQuery_new_stringrange(fieldName, start2, end);
}
HEGGQUERY EGGAPI eggQuery_new_stringrange_gt(const char* fieldName, char *start)
{

    if (start && strlen(start) > EGG_BTREE_STRING_MAX)
    {
        /* fprintf(stderr, "%s:%d:%s [%s] too long, will be truncated to length %d\n", __func__, __LINE__, __FILE__, */
        /*         start, EGG_BTREE_STRING_MAX); */
        eggPrtLog_error("eggQuery", "%s:%d:%s [%s] too long, will be truncated to length %d\n", __func__, __LINE__, __FILE__,
                start, EGG_BTREE_STRING_MAX);
    }
    
    int i;
    
    unsigned char end[EGG_BTREE_STRING_MAX+1];
    end[EGG_BTREE_STRING_MAX] = '\0';
    for (i = 0; i < EGG_BTREE_STRING_MAX; i++)
    {
        end[i] = '\377';
    }

    char *start2;
    unsigned char buf[EGG_BTREE_STRING_MAX+1];
    start2 = string_add_epsilon(start, buf, EGG_BTREE_STRING_MAX+1);
    if (start2 == NULL)
    {
        return NULL;
    }

    return eggQuery_new_stringrange(fieldName, start2, end);
    
}
HEGGQUERY EGGAPI eggQuery_new_stringrange_ge(const char* fieldName, char *start)
{

    if (start && strlen(start) > EGG_BTREE_STRING_MAX)
    {
        /* fprintf(stderr, "%s:%d:%s [%s] too long, will be truncated to length %d\n", __func__, __LINE__, __FILE__, */
        /*         start, EGG_BTREE_STRING_MAX); */
        eggPrtLog_error("eggQuery", "%s:%d:%s [%s] too long, will be truncated to length %d\n", __func__, __LINE__, __FILE__,
                start, EGG_BTREE_STRING_MAX);

    }

    
    int i;
    
    unsigned char end2[EGG_BTREE_STRING_MAX+1];
    end2[EGG_BTREE_STRING_MAX] = '\0';
    for (i = 0; i < EGG_BTREE_STRING_MAX; i++)
    {
        end2[i] = '\377';
    }

    return eggQuery_new_stringrange(fieldName, start, end2);
}
HEGGQUERY EGGAPI eggQuery_new_stringrange_lt(const char* fieldName, char *end)
{
    if (!end || !end[0])
    {
        return NULL;
    }
    if (end && strlen(end) > EGG_BTREE_STRING_MAX)
    {
        /* fprintf(stderr, "%s:%d:%s [%s] too long, will be truncated to length %d\n", __func__, __LINE__, __FILE__, */
        /*         end, EGG_BTREE_STRING_MAX); */
        eggPrtLog_error("eggQuery", "%s:%d:%s [%s] too long, will be truncated to length %d\n", __func__, __LINE__, __FILE__,
                end, EGG_BTREE_STRING_MAX);

    }
    
    int i;
    
    char *start = NULL;

    char *end2;
    unsigned char buf[EGG_BTREE_STRING_MAX+1];
    end2 = string_minus_epsilon(end, buf, EGG_BTREE_STRING_MAX+1);
    
    return eggQuery_new_stringrange(fieldName, start, end2);
}
HEGGQUERY EGGAPI eggQuery_new_stringrange_le(const char* fieldName, char *end)
{
    if (end && strlen(end) > EGG_BTREE_STRING_MAX)
    {
        /* fprintf(stderr, "%s:%d:%s [%s] too long, will be truncated to length %d\n", __func__, __LINE__, __FILE__, */
        /*         end, EGG_BTREE_STRING_MAX); */
        eggPrtLog_error("eggQuery", "%s:%d:%s [%s] too long, will be truncated to length %d\n", __func__, __LINE__, __FILE__,
                end, EGG_BTREE_STRING_MAX);

    }
    
    char *start = NULL;
    
    return eggQuery_new_stringrange(fieldName, start, end);
}
HEGGQUERY EGGAPI eggQuery_new_stringrange_all(const char* fieldName)
{
    char *start = NULL;
    int i;
    
    unsigned char end[EGG_BTREE_STRING_MAX+1];
    end[EGG_BTREE_STRING_MAX] = '\0';
    for (i = 0; i < EGG_BTREE_STRING_MAX; i++)
    {
        end[i] = '\377';
    }

    return eggQuery_new_stringrange(fieldName, start, end);
}

HEGGQUERY EGGAPI eggQuery_and(HEGGQUERY hQueryExpress1, HEGGQUERY hQueryExpress2)
{
    if (!hQueryExpress1)
    {
        return NULL;
    }
    else if (!hQueryExpress2)
    {
        return NULL;
    }
    HEGGQUERY query;
    ALLOC_QUERY(query);
    query->opt = EGGQUERYOPT_AND;
    query->p = hQueryExpress1;
    query->q = hQueryExpress2;
    return query;
}
HEGGQUERY EGGAPI eggQuery_and_phrase(HEGGQUERY hQueryExpress1, HEGGQUERY hQueryExpress2)
{
    if (!hQueryExpress1)
    {
        return NULL;
    }
    else if (!hQueryExpress2)
    {
        return NULL;
    }
    HEGGQUERY query;
    ALLOC_QUERY(query);
    query->opt = EGGQUERYOPT_ANDPHRASE;
    query->p = hQueryExpress1;
    query->q = hQueryExpress2;
    return query;
}
HEGGQUERY EGGAPI eggQuery_or(HEGGQUERY hQueryExpress1, HEGGQUERY hQueryExpress2)
{
    if (!hQueryExpress1)
    {
        return hQueryExpress2;
    }
    else if (!hQueryExpress2)
    {
        return hQueryExpress1;
    }
    
    HEGGQUERY query;
    ALLOC_QUERY(query);
    query->opt = EGGQUERYOPT_OR;
    query->p = hQueryExpress1;
    query->q = hQueryExpress2;
    return query;
    
}
HEGGQUERY EGGAPI eggQuery_minus(HEGGQUERY hQueryExpress1, HEGGQUERY hQueryExpress2)
{
    if (!hQueryExpress1)
    {
        return NULL;
    }
    else if (!hQueryExpress2)
    {
        return hQueryExpress1;
    }
    
    HEGGQUERY query;
    ALLOC_QUERY(query);
    query->opt = EGGQUERYOPT_MINUS;
    query->p = hQueryExpress1;
    query->q = hQueryExpress2;
    return query;
    
}

HEGGQUERY EGGAPI eggQuery_boost_preference(HEGGQUERY hQueryExpress1, eggBoost_t boost)
{
    if (!hQueryExpress1)
    {
        return NULL;
    }
    
    hQueryExpress1->boost = boost;
    return hQueryExpress1;
}

EBOOL EGGAPI eggQuery_delete(HEGGQUERY hQuery)
{
    if (!hQuery)
        return EGG_TRUE;
    
    if (hQuery->opt == EGGQUERYOPT_AND)
    {
        eggQuery_delete(hQuery->p);
        eggQuery_delete(hQuery->q);
    }
    if (hQuery->opt == EGGQUERYOPT_ANDPHRASE)
    {
        eggQuery_delete(hQuery->p);
        eggQuery_delete(hQuery->q);
    }
    else if (hQuery->opt == EGGQUERYOPT_OR)
    {
        eggQuery_delete(hQuery->p);
        eggQuery_delete(hQuery->q);
    }
    else if (hQuery->opt == EGGQUERYOPT_MINUS)
    {
        eggQuery_delete(hQuery->p);
        eggQuery_delete(hQuery->q);
    }
    else
    {
        free(hQuery->fieldName);
        free(hQuery->key1);
        free(hQuery->key2);
        free(hQuery->analyzerName);
        free(hQuery->dictName);
    }
    free(hQuery);
    return EGG_TRUE;
}
HEGGQUERY EGGAPI eggQuery_dup(HEGGQUERY hQuery)
{
    if (!hQuery)
        return NULL;

    if (hQuery->opt == EGGQUERYOPT_OR
        || hQuery->opt == EGGQUERYOPT_AND
        || hQuery->opt == EGGQUERYOPT_ANDPHRASE
        || hQuery->opt == EGGQUERYOPT_MINUS)
    {
        HEGGQUERY query;
        ALLOC_QUERY(query);
        query->opt = hQuery->opt;
        query->p = eggQuery_dup(hQuery->p);
        query->q = eggQuery_dup(hQuery->q);
        query->boost = hQuery->boost;
        return query;
    }
    else if (hQuery->opt == EGGQUERYOPT_TERM
             || hQuery->opt == EGGQUERYOPT_TERMOR             
             || hQuery->opt == EGGQUERYOPT_RANGE
             || hQuery->opt == EGGQUERYOPT_PHRASE)
    {
        HEGGQUERY query;
        ALLOC_QUERY(query);
        query->opt = hQuery->opt;
        query->fieldName = strdup(hQuery->fieldName);
        assert(query->fieldName);
        query->key1Sz = hQuery->key1Sz;
        if (query->key1Sz > 0)
        {
            query->key1 = malloc(query->key1Sz);
            assert(query->key1);
            memcpy(query->key1, hQuery->key1, query->key1Sz);
        }
        query->key2Sz = hQuery->key2Sz;
        if (query->key2Sz > 0)
        {
            query->key2 = malloc(query->key2Sz);
            assert(query->key2);
            memcpy(query->key2, hQuery->key2, query->key2Sz);
        }
        if(hQuery->analyzerName)
        {
            query->analyzerName = strdup(hQuery->analyzerName);
            assert(query->analyzerName);
        }
        if(hQuery->dictName)
        {
            query->dictName = strdup(hQuery->dictName);
            assert(query->dictName);
        }

        query->boost = hQuery->boost;
        return query;
        
    }
    else if (hQuery->opt == EGGQUERYOPT_DATA)
    {
        HEGGQUERY query;
        ALLOC_QUERY(query);
        query->opt = hQuery->opt;
        query->cnt = hQuery->cnt;
        query->hIdNodes = malloc(query->cnt * sizeof(EGGIDNODE));
        assert(query->hIdNodes);
        memcpy(query->hIdNodes, hQuery->hIdNodes, query->cnt * sizeof(EGGIDNODE));
        query->hFieldKey = eggFieldKey_dup(hQuery->hFieldKey);
        query->boost = hQuery->boost;
        return query;
        
    }
    else
    {
        return NULL;
    }
}


EBOOL EGGAPI eggQuery_equal(HEGGQUERY hQuery, HEGGQUERY hQuery2)
{
    if (hQuery == hQuery2)
    {
        return EGG_TRUE;
    }
    if (!hQuery || !hQuery2)
    {
        return EGG_FALSE;
    }

    if (hQuery->boost != hQuery2->boost)
    {
        return EGG_FALSE;
    }
    if (hQuery->opt == hQuery2->opt)
    {
        if (hQuery->opt == EGGQUERYOPT_OR
            || hQuery->opt == EGGQUERYOPT_AND
            || hQuery->opt == EGGQUERYOPT_ANDPHRASE
            || hQuery->opt == EGGQUERYOPT_MINUS)
        {
            return eggQuery_equal(hQuery->p, hQuery2->p)
                && eggQuery_equal(hQuery->q, hQuery2->q);
        }
        else if (hQuery->opt == EGGQUERYOPT_TERM
                 || hQuery->opt == EGGQUERYOPT_TERMOR
                 || hQuery->opt == EGGQUERYOPT_RANGE
                 || hQuery->opt == EGGQUERYOPT_PHRASE)
        {
            if (hQuery->fieldName == hQuery2->fieldName)
            {
                ;
            }
            else if (hQuery->fieldName && hQuery2->fieldName
                && strcmp(hQuery->fieldName,  hQuery2->fieldName) == 0)
            {
                ;
            }
            else
            {
                return EGG_FALSE;                
            }

            if (hQuery->key1Sz != hQuery2->key1Sz)
            {
                return EGG_FALSE;
            }
            if (hQuery->key1 == hQuery2->key1)
            {
                ;
            }
            else if (hQuery->key1Sz == 0)
            {
                ;
            }
            else if (hQuery->key1 && hQuery2->key1
                     && memcmp(hQuery->key1, hQuery2->key1, hQuery->key1Sz) == 0)
            {
                ;
            }
            else
            {
                return EGG_FALSE;
            }

            if (hQuery->key2Sz != hQuery2->key2Sz)
            {
                return EGG_FALSE;
            }
            if (hQuery->key2 == hQuery2->key2)
            {
                ;
            }
            else if (hQuery->key2Sz == 0)
            {
                ;
            }
            else if (hQuery->key2 && hQuery2->key2
                     && memcmp(hQuery->key2, hQuery2->key2, hQuery->key2Sz) == 0)
            {
                ;
            }
            else
            {
                return EGG_FALSE;
            }
            
            
            if (hQuery->analyzerName == hQuery2->analyzerName)
            {
                ;
            }
            else if(hQuery->analyzerName && hQuery2->analyzerName
                    && strcmp(hQuery->analyzerName, hQuery2->analyzerName) == 0)
            {
                ;
            }
            else
            {
                return EGG_FALSE;
            }
            
            if(hQuery->dictName == hQuery2->dictName)
            {
                ;
            }
            else if(hQuery->dictName && hQuery2->dictName
                    && strcmp(hQuery->dictName, hQuery2->dictName) == 0)
            {
                ;
            }
            else
            {
                return EGG_FALSE;
            }
            return EGG_TRUE;
                
        }
        else if (hQuery->opt == EGGQUERYOPT_DATA)
        {
            if (hQuery->cnt != hQuery2->cnt)
            {
                return EGG_FALSE;
            }
            
            if (hQuery->cnt == 0)
            {
                ;
            }
            else if (hQuery->hIdNodes == hQuery2->hIdNodes)
            {
                ;
            }
            else if (hQuery->hIdNodes && hQuery2->hIdNodes
                     && memcpy(hQuery->hIdNodes, hQuery2->hIdNodes,
                               hQuery->cnt * sizeof(EGGIDNODE)) == 0)
            {
                ;
            }
            else
            {
                return EGG_FALSE;
            }
            
            return eggFieldKey_equal(hQuery->hFieldKey, hQuery2->hFieldKey);
        }
        else
        {
            return EGG_FALSE;
        }
    }
    else
    {
        return EGG_FALSE;
    }
}

static int treeWalkPrefix(HEGGQUERY hQuery,
                           int (*fn)(HEGGQUERY, void *), void *p)
{
    if (!hQuery || !fn)
    {
        return 0;
    }
    fn(hQuery, p);    
    treeWalkPrefix(hQuery->p, fn, p);
    treeWalkPrefix(hQuery->q, fn, p);

    return 0;
}
static int getQueryRecordSz(HEGGQUERY hQuery, void *p)
{
    int sz;
    if (!hQuery)
    {
        sz = 0;
        return sz;
    }
    if (hQuery->opt == EGGQUERYOPT_OR
        || hQuery->opt == EGGQUERYOPT_AND
        || hQuery->opt == EGGQUERYOPT_ANDPHRASE
        || hQuery->opt == EGGQUERYOPT_MINUS)
    {
        sz = sizeof(size32_t) + sizeof(type_t) + sizeof(eggBoost_t);

    }
    else if (hQuery->opt == EGGQUERYOPT_TERM
             || hQuery->opt == EGGQUERYOPT_TERMOR
             || hQuery->opt == EGGQUERYOPT_PHRASE
             || hQuery->opt == EGGQUERYOPT_RANGE)
    {
        sz = sizeof(size32_t) + sizeof(type_t) + sizeof(eggBoost_t)
            + strlen(hQuery->fieldName)+1
            + 2 * sizeof(size16_t) + hQuery->key1Sz + hQuery->key2Sz
            + (hQuery->analyzerName
               ? strlen(hQuery->analyzerName)+1 : 1)
            + (hQuery->dictName
               ? strlen(hQuery->dictName)+1 : 1);
    }
    else if (hQuery->opt == EGGQUERYOPT_DATA)
    {
        char *hEggFieldKey;
        size32_t hEggFieldKeySz;
        hEggFieldKey = eggFieldKey_serialise(hQuery->hFieldKey, &hEggFieldKeySz);
        free(hEggFieldKey);
        sz = sizeof(size32_t) + sizeof(type_t) + sizeof(eggBoost_t)
            + hEggFieldKeySz 
            + sizeof(EGGIDNODE) * hQuery->cnt;
        
    }
    *(int *)p += sz;
    return sz;
}
static int dumpQueryRecord(HEGGQUERY hQuery, char **pp)
{
    if (!hQuery || !pp)
    {
        return 0;
    }

    if (hQuery->opt == EGGQUERYOPT_OR
        || hQuery->opt == EGGQUERYOPT_AND
        || hQuery->opt == EGGQUERYOPT_ANDPHRASE
        || hQuery->opt == EGGQUERYOPT_MINUS)
    {
        *(size32_t *)*pp = sizeof(hQuery->opt);
        *pp += sizeof(size32_t);
        *(type_t *)*pp = hQuery->opt;
        *pp += sizeof(hQuery->opt);
        *(eggBoost_t *)*pp = hQuery->boost;
        *pp += sizeof(hQuery->boost);
    }
    else if (hQuery->opt == EGGQUERYOPT_TERM
             || hQuery->opt == EGGQUERYOPT_TERMOR
             || hQuery->opt == EGGQUERYOPT_PHRASE
             || hQuery->opt == EGGQUERYOPT_RANGE)
    {
        int sz;
        sz = sizeof(type_t)
            + strlen(hQuery->fieldName)+1
            + 2 * sizeof(size16_t) + hQuery->key1Sz + hQuery->key2Sz
            + (hQuery->analyzerName
               ? strlen(hQuery->analyzerName)+1 : 1)
            + (hQuery->dictName
               ? strlen(hQuery->dictName)+1 : 1);
            
        *(size32_t *)*pp = sz;
        *pp += sizeof(size32_t);
        *(type_t *)*pp = hQuery->opt;
        *pp += sizeof(hQuery->opt);
        strcpy(*pp, hQuery->fieldName);
        *pp += strlen(hQuery->fieldName) + 1;
        *(size16_t *)*pp = hQuery->key1Sz;
        *pp += sizeof(hQuery->key1Sz);
        memcpy(*pp, hQuery->key1, hQuery->key1Sz);
        *pp += hQuery->key1Sz;
        *(size16_t *)*pp = hQuery->key2Sz;
        *pp += sizeof(hQuery->key2Sz);
        memcpy(*pp, hQuery->key2, hQuery->key2Sz);
        *pp += hQuery->key2Sz;
        if (hQuery->analyzerName)
        {
            strcpy(*pp, hQuery->analyzerName);
            *pp += strlen(hQuery->analyzerName) + 1;
        }
        else
        {
            **pp = '\0';
            *pp += 1;
        }
        if (hQuery->dictName)
        {
            strcpy(*pp, hQuery->dictName);
            *pp += strlen(hQuery->dictName) + 1;
        }
        else
        {
            **pp = '\0';
            *pp += 1;
        }
        
        *(eggBoost_t *)*pp = hQuery->boost;
        *pp += sizeof(hQuery->boost);

    }
    else if (hQuery->opt == EGGQUERYOPT_DATA)
    {
        char *hEggFieldKey;
        size32_t hEggFieldKeySz;
        hEggFieldKey = eggFieldKey_serialise(hQuery->hFieldKey, &hEggFieldKeySz);

        size32_t sz;
        sz = sizeof(type_t) + hEggFieldKeySz
            + sizeof(EGGIDNODE) * hQuery->cnt;
        
        *(size32_t *)*pp = sz;
        *pp += sizeof(size32_t);
        *(type_t *)*pp = hQuery->opt;
        *pp += sizeof(hQuery->opt);
        memcpy(*pp, hEggFieldKey, hEggFieldKeySz);
        free(hEggFieldKey);
        *pp += hEggFieldKeySz;
        memcpy(*pp, hQuery->hIdNodes, sizeof(EGGIDNODE) * hQuery->cnt);
        *pp += sizeof(EGGIDNODE) * hQuery->cnt;
        *(eggBoost_t *)*pp = hQuery->boost;
        *pp += sizeof(hQuery->boost);

    }

    return 0;
}
char * EGGAPI eggQuery_serialise(HEGGQUERY hQuery, int *sz)
{
    if (!sz || !hQuery)
    {
        return NULL;
    }
    *sz = 0;
    treeWalkPrefix(hQuery, getQueryRecordSz, sz);

    char *rp = calloc(1, *sz);
    assert(rp);
    char *endp = rp;
    treeWalkPrefix(hQuery, dumpQueryRecord, &endp);
    return rp;
}
static HEGGQUERY getNextQueryRecord(char *query, int sz, char **endp)
{
    if (!endp || !query || sz < sizeof(size32_t))
    {
        return NULL;
    }
    int recordsz;
    if ((sizeof(size32_t) + (recordsz = (int)*(size32_t *)query)) > sz)
    {
        *endp = query;
        return NULL;
    }
    query += sizeof(size32_t);
    HEGGQUERY hQuery = NULL;
    ALLOC_QUERY(hQuery);
    hQuery->opt = *(type_t *)query;
    query += sizeof(type_t);
    if (hQuery->opt == EGGQUERYOPT_OR
        || hQuery->opt == EGGQUERYOPT_AND
        || hQuery->opt == EGGQUERYOPT_ANDPHRASE
        || hQuery->opt == EGGQUERYOPT_MINUS)
    {
        hQuery->boost = *(eggBoost_t *)query;
        query += sizeof(eggBoost_t);
    }
    else if (hQuery->opt == EGGQUERYOPT_TERM
             || hQuery->opt == EGGQUERYOPT_TERMOR
             || hQuery->opt == EGGQUERYOPT_PHRASE
             || hQuery->opt == EGGQUERYOPT_RANGE)
    {
        hQuery->fieldName = strdup(query);
        assert(hQuery->fieldName);
        query += strlen(query) + 1;
        hQuery->key1Sz = *(size16_t *)query;
        query += sizeof(size16_t);
        hQuery->key1 = malloc(hQuery->key1Sz);
        assert(hQuery->key1);
        memcpy(hQuery->key1, query, hQuery->key1Sz);
        query += hQuery->key1Sz;
        hQuery->key2Sz = *(size16_t *)query;
        query += sizeof(size16_t);        
        if (hQuery->key2Sz)
        {
            hQuery->key2 = malloc(hQuery->key2Sz);
            assert(hQuery->key2);
            memcpy(hQuery->key2, query, hQuery->key2Sz);
            query += hQuery->key2Sz;
        }
        else
        {
            hQuery->key2 = strdup("");
            assert(hQuery->key2);
        }
        if (query[0])
        {
            hQuery->analyzerName = strdup(query);
            query += strlen(query) + 1;
        }
        else
        {
            query++;
        }
        if (query[0])
        {
            hQuery->dictName = strdup(query);
            query += strlen(query) + 1;
        }
        else
        {
            query++;
        }
        hQuery->boost = *(eggBoost_t *)query;
        query += sizeof(eggBoost_t);

    }
    else if (hQuery->opt == EGGQUERYOPT_DATA)
    {
        size32_t hEggFieldKeySz;
        hEggFieldKeySz = *(size32_t*)query;
        hQuery->hFieldKey = eggFieldKey_unserialise(query);
        query += hEggFieldKeySz;
        int hIdNodesSz;
        hIdNodesSz = recordsz - sizeof(type_t) - hEggFieldKeySz;
        hQuery->hIdNodes = malloc(hIdNodesSz);
        assert(hQuery->hIdNodes);
        memcpy(hQuery->hIdNodes, query, hIdNodesSz);
        hQuery->cnt = (hIdNodesSz) / sizeof(EGGIDNODE);
        query += hIdNodesSz;
        hQuery->boost = *(eggBoost_t *)query;
        query += sizeof(eggBoost_t);
    }

    *endp = query;
    return hQuery;
}
static char **AnalyzeString(HEGGANALYZER hAnalyzer, const char *dictName, const char *keyword);
static int analyzeQueryRecord(HEGGQUERY *hhQuery)
{
    HEGGQUERY hQuery = *hhQuery;
    if (!hQuery
        || ! (hQuery->opt == EGGQUERYOPT_TERM
              || hQuery->opt == EGGQUERYOPT_TERMOR
              || hQuery->opt == EGGQUERYOPT_PHRASE))
    {
        return 0;
    }
    if (hQuery->key1Sz == 0
        || hQuery->key1 == 0)
    {
        return 0;
    }
    HEGGANALYZER hAnalyzer =  eggAnalyzer_get(hQuery->analyzerName);
    if (!hAnalyzer)
    {
        free(hQuery->analyzerName);
        hQuery->analyzerName = 0;
        free(hQuery->dictName);
        hQuery->dictName = 0;
        
        return 0;
    }

    HEGGQUERY root = NULL;
    char **tokens;
    if (((char*)(hQuery->key1))[hQuery->key1Sz-1] != '\0')
    {
        char *keyword;
        keyword = calloc(1, hQuery->key1Sz+1);
        assert(keyword);
        memcpy(keyword, hQuery->key1, hQuery->key1Sz);
        tokens = AnalyzeString(hAnalyzer, hQuery->dictName, keyword);
        free(keyword);
    }
    else
    {
        tokens = AnalyzeString(hAnalyzer, hQuery->dictName, hQuery->key1);
    }
    if (!tokens || !*tokens)
    {
        free(hQuery->analyzerName);
        hQuery->analyzerName = 0;
        free(hQuery->dictName);
        hQuery->dictName = 0;
        free(tokens);
        return 0;
    }
    
    char **p = tokens; 
    while (*p) 
    { 
        if (root == NULL) 
        { 
            HEGGQUERY query; 
            ALLOC_QUERY(query); 
            query->opt = hQuery->opt; 
            query->fieldName = strdup(hQuery->fieldName);
            assert(query->fieldName);
            query->key1 = *p; 
            query->key1Sz = strlen(*p); 
            query->key2 = 0;
            query->boost = hQuery->boost;
            root = query;
            *p = 0; 
        } 
        else 
        { 
            HEGGQUERY child; 
            HEGGQUERY parent; 
            ALLOC_QUERY(child); 
            child->opt = hQuery->opt; 
            child->fieldName = strdup(hQuery->fieldName);
            assert(child->fieldName); 
            child->key1 = *p; 
            child->key1Sz = strlen(*p); 
            child->key2 = 0;
            child->boost = hQuery->boost;
            *p = 0; 
            ALLOC_QUERY(parent);
            if (hQuery->opt == EGGQUERYOPT_PHRASE)
            {
                parent->opt = EGGQUERYOPT_ANDPHRASE;
            }
            else if (hQuery->opt == EGGQUERYOPT_TERM)
            {
                parent->opt = EGGQUERYOPT_AND;
            }
            else if (hQuery->opt == EGGQUERYOPT_TERMOR)
            {
                parent->opt = EGGQUERYOPT_OR;
            }
            else
            {
                //printf("Warning hQuery->opt ERRVAL\n", __func__);
                eggPrtLog_warn("eggQuery", "Warning hQuery->opt ERRVAL\n", __func__);
                parent->opt = EGGQUERYOPT_AND;
            }
            parent->boost = root->boost;
            parent->p = root; 
            parent->q = child; 
            root = parent; 
        } 
        ++p; 
    } 
    free(tokens); 
    eggQuery_delete(hQuery);
    *hhQuery = root;
    return 0;
}
static HEGGQUERY EGGAPI eggQuery_unserialise_internal(char *query, int sz, char **endquery);
HEGGQUERY EGGAPI eggQuery_unserialise(char *query, int sz)
{
    char *q;
    HEGGQUERY h;
    h = eggQuery_unserialise_internal(query, sz, &q);
    if (h && q != query + sz)
    {
        /* fprintf(stderr, "[%s] query buf too much: buf[%d] used[%d] \n", __func__, */
        /*         sz, q-query); */
        eggPrtLog_error("eggQuery", "[%s] query buf too much: buf[%d] used[%d] \n", __func__,
                sz, q-query);
        
    }
    return h;
}

static HEGGQUERY EGGAPI eggQuery_unserialise_internal(char *query, int sz, char **endquery)
{
    HEGGQUERY hQuery;
    hQuery = getNextQueryRecord(query, sz, endquery);
    if (!hQuery)
    {
        return NULL;
    }

    if (hQuery->opt == EGGQUERYOPT_OR
        || hQuery->opt == EGGQUERYOPT_AND
        || hQuery->opt == EGGQUERYOPT_ANDPHRASE
        || hQuery->opt == EGGQUERYOPT_MINUS)
    {
        HEGGQUERY p, q;
        p = eggQuery_unserialise_internal(*endquery, query+sz-*endquery, endquery);
        q = eggQuery_unserialise_internal(*endquery, query+sz-*endquery, endquery);
        hQuery->p = p;
        hQuery->q = q;
    }
    else if (hQuery->opt == EGGQUERYOPT_TERM
             || hQuery->opt == EGGQUERYOPT_TERMOR
             || hQuery->opt == EGGQUERYOPT_PHRASE
             || hQuery->opt == EGGQUERYOPT_RANGE)
    {
        analyzeQueryRecord(&hQuery);
    }
    else if (hQuery->opt == EGGQUERYOPT_DATA)
    {
        ;
    }
    return hQuery;
}
static char **AnalyzeString(HEGGANALYZER hAnalyzer, const char *dictName, const char *keyword)
{
    ImTokenList *pTokenList;
    hAnalyzer->fnTokenize(hAnalyzer->pHandle, keyword, &pTokenList, dictName);

    unsigned int c;
    ImTokenList_get_size(pTokenList, &c);
    char **pret = (char **)calloc(1+c, sizeof(char *));
    assert(pret);
    int i;
    for (i = 0; i < c; i++)
    {
        ImToken *pToken;
        ImTokenList_item(pTokenList, i, &pToken);
        ImToken_get_data(pToken, &pret[i]);
    }
    ImTokenList_delete(pTokenList);
    return pret;
}

static HEGGQUERY eggQueryList_appendList(HEGGQUERY first, HEGGQUERY second);
static int eggQueryList_destroyList(HEGGQUERY query);
static HEGGQUERY eggQueryList_findRangeType_internal(HEGGQUERY hQueryExp);
static HEGGQUERYEXP eggQuery_init_exp_internal(HEGGQUERY *hhQuery)
{
    if (!*hhQuery)
    {
        return NULL;
    }

    analyzeQueryRecord(hhQuery);
    
    HEGGQUERY hQuery = *hhQuery;
    if (!hQuery)
    {
        return NULL;
    }
    if (hQuery->opt == EGGQUERYOPT_AND
        || hQuery->opt == EGGQUERYOPT_ANDPHRASE
        || hQuery->opt == EGGQUERYOPT_OR
        || hQuery->opt == EGGQUERYOPT_MINUS)
    {
        HEGGQUERY hExp1, hExp2;
        hExp1 = (HEGGQUERY)eggQuery_init_exp_internal(&hQuery->p);
        hExp2 = (HEGGQUERY)eggQuery_init_exp_internal(&hQuery->q);
        HEGGQUERY query;
        ALLOC_QUERY(query);
        query->opt = hQuery->opt;
        query = eggQueryList_appendList(hExp2, query);
        query = eggQueryList_appendList(hExp1, query);
        return (HEGGQUERYEXP)query;
    }
    else if (hQuery->opt == EGGQUERYOPT_RANGE)
    {
        HEGGQUERY query;
        ALLOC_QUERY(query);
        query->opt = hQuery->opt;
        query->fieldName = strdup(hQuery->fieldName);
        assert(query->fieldName);
        query->key1Sz = hQuery->key1Sz;
        query->key1 = malloc(hQuery->key1Sz);
        assert(query->key1);
        memcpy(query->key1, hQuery->key1, hQuery->key1Sz);
        query->key2Sz = hQuery->key2Sz;
        query->key2 = malloc(hQuery->key2Sz);
        assert(query->key2);
        memcpy(query->key2, hQuery->key2, hQuery->key2Sz);
        
        return (HEGGQUERYEXP)query;
    }
    else if (hQuery->opt == EGGQUERYOPT_TERM
             || hQuery->opt == EGGQUERYOPT_TERMOR
             ||hQuery->opt == EGGQUERYOPT_PHRASE)
    {
        
        HEGGQUERY query;
        ALLOC_QUERY(query);
        query->opt = hQuery->opt;
        query->fieldName = strdup(hQuery->fieldName);
        query->key1Sz = hQuery->key1Sz;
        query->key1 = malloc(hQuery->key1Sz);
        assert(query->key1);
        memcpy(query->key1, hQuery->key1, hQuery->key1Sz);
        query->key2 = 0;
        query->key2Sz = 0;            
        return (HEGGQUERYEXP)query;
        
    }
    else if (hQuery->opt == EGGQUERYOPT_DATA)
    {
        HEGGQUERY query;
        ALLOC_QUERY(query);
        query->opt = hQuery->opt;        
        query->hFieldKey = eggFieldKey_dup(hQuery->hFieldKey);
        query->cnt = hQuery->cnt;
        query->hIdNodes = malloc(sizeof(struct eggIdNode) * hQuery->cnt);
        assert(query->hIdNodes);
        memcpy(query->hIdNodes, hQuery->hIdNodes, sizeof(struct eggIdNode) * hQuery->cnt);
        return (HEGGQUERYEXP)query;
    }
    return NULL;
}
HEGGQUERYEXP eggQuery_init_exp(HEGGQUERY hQuery)
{
    HEGGQUERYEXP hQueryExp;
    HEGGQUERY hQuery2;

    hQuery2 = eggQuery_dup(hQuery);

    hQueryExp = eggQuery_init_exp_internal(&hQuery2);
	
	if (0)
    {                       /* make sure have only one range search */
        HEGGQUERY hq;
        hq = eggQueryList_findRangeType_internal((HEGGQUERY)hQueryExp);
        if (hq && hq->q)
        {
            /* fprintf(stderr, "%s:%d:%s WARN: do not support two range search\n", */
            /*         __FILE__, __LINE__, __func__); */
            eggPrtLog_warn("eggQuery", "%s:%d:%s WARN: do not support two range search\n",
                    __FILE__, __LINE__, __func__);


            eggQueryList_destroyList((HEGGQUERY)hQueryExp);
            hQueryExp = NULL;
        }
        eggQueryList_destroyList(hq);
    }
    
    eggQuery_delete(hQuery2);
    return hQueryExp;
}
HEGGQUERY eggQuery_find_rangetype(HEGGQUERY hQuery)
{
    HEGGQUERYEXP hQueryExp;
    HEGGQUERY hQuery2, hRetQuery;

    hQuery2 = eggQuery_dup(hQuery);

    hQueryExp = eggQuery_init_exp_internal(&hQuery2);

    hRetQuery = eggQueryList_findRangeType_internal((HEGGQUERY)hQueryExp);
    if (hRetQuery)
    {
        eggQueryList_destroyList(hRetQuery->q);
        hRetQuery->p = 0;
        hRetQuery->q = 0;
    }
    
    eggQuery_delete(hQuery2);
    eggQueryList_destroyList((HEGGQUERY)hQueryExp);    
    return hRetQuery;
}
static HEGGQUERY eggQueryList_findRangeType_internal(HEGGQUERY hQuery)
{
    HEGGQUERY hRetQuery = NULL;
    while (hQuery)
    {
        if (hQuery->opt == EGGQUERYOPT_RANGE)
        {
            HEGGQUERY hNew;
            ALLOC_QUERY(hNew);
            hNew->opt = hQuery->opt;
            hNew->fieldName = strdup(hQuery->fieldName);
            assert(hNew->fieldName);
            hNew->key1Sz = hQuery->key1Sz;
            if (hNew->key1Sz > 0)
            {
                hNew->key1 = malloc(hNew->key1Sz);
                assert(hNew->key1);
                memcpy(hNew->key1, hQuery->key1, hNew->key1Sz);
            }
            hNew->key2Sz = hQuery->key2Sz;
            if (hNew->key2Sz > 0)
            {
                hNew->key2 = malloc(hNew->key2Sz);
                assert(hNew->key2);
                memcpy(hNew->key2, hQuery->key2, hNew->key2Sz);
            }
            if(hQuery->analyzerName)
            {
                hNew->analyzerName = strdup(hQuery->analyzerName);
                assert(hNew->analyzerName);
            }
            if(hQuery->dictName)
            {
                hNew->dictName = strdup(hQuery->dictName);
                assert(hNew->dictName);
            }
            
            if (hRetQuery)
            {
                hRetQuery->p = hNew;
                hNew->q = hRetQuery;
            }
            hRetQuery = hNew;
        }
        hQuery = hQuery->q;
    }
    return hRetQuery;
}
static HEGGQUERY eggQueryList_appendList(HEGGQUERY first, HEGGQUERY second)
{
    if (!first)
    {
        return second;
    }
    else if (!second)
    {
        return first;
    }

    HEGGQUERY plast = NULL;
    HEGGQUERY p = first;
    while (p)
    {
        plast = p;
        p = p->q;
    }
    plast->q = second;
    second->p = plast;
    return first;
}
static int eggQueryList_destroyList(HEGGQUERY query)
{
    while(query)
    {
        HEGGQUERY next;
        next = query->q;

        if (query->opt == EGGQUERYOPT_DATA)
        {
            eggFieldKey_del(query->hFieldKey);
            free(query->hIdNodes);
            free(query);
        }
        else if (query->opt == EGGQUERYOPT_OR
                 || query->opt == EGGQUERYOPT_AND
                 || query->opt == EGGQUERYOPT_ANDPHRASE
                 || query->opt == EGGQUERYOPT_MINUS)
        {
            free(query);
        }
        else if (query->opt == EGGQUERYOPT_RANGE)
        {
            free(query->fieldName);
            free(query->key1);
            free(query->key2);
            free(query->analyzerName);
            free(query->dictName);            
            free(query);
        }
        else if (query->opt == EGGQUERYOPT_TERM
                 || query->opt == EGGQUERYOPT_TERMOR                 
                 ||query->opt == EGGQUERYOPT_PHRASE)
        {
            free(query->fieldName);
            free(query->key1);
            free(query->analyzerName);
            free(query->dictName);            
            free(query);
        }
        query = next;
    }
    return 0;
}

static int eggQueryList_is_end_of_andphrase(HEGGQUERY hQuery)
{
    if (!hQuery->q)
    {
        return 1;
    }
    
    if (hQuery->q->opt == EGGQUERYOPT_ANDPHRASE
          || hQuery->q->opt == EGGQUERYOPT_PHRASE)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

HEGGQUERYEXP eggQuery_next_exp(HEGGQUERYEXP hQueryExp, HEGGIDNODE lastResult, count_t lastResultCnt)
{
    if (!hQueryExp)
    {
        return NULL;
    }
    
    HEGGQUERY hQuery;
    hQuery = (HEGGQUERY)hQueryExp;
    if (hQuery->opt == EGGQUERYOPT_RANGE)
    {
        free(hQuery->fieldName);
        free(hQuery->key1);
        free(hQuery->key2);
        hQuery->opt = EGGQUERYOPT_DATA;
        hQuery->hFieldKey = 0;
        hQuery->hIdNodes = lastResult;
        hQuery->cnt = lastResultCnt;
    }
    else if (hQuery->opt == EGGQUERYOPT_TERM
             || hQuery->opt == EGGQUERYOPT_TERMOR             
             ||hQuery->opt == EGGQUERYOPT_PHRASE)
    {
        hQuery->opt = EGGQUERYOPT_DATA;
        HEGGFIELDKEY hEggFieldKey = eggFieldKey_new(hQuery->fieldName,
                                                    hQuery->key1, hQuery->key1Sz);
        free(hQuery->fieldName);
        free(hQuery->key1);
        hQuery->hFieldKey = hEggFieldKey;
        hQuery->hIdNodes = lastResult;
        hQuery->cnt = lastResultCnt;
    }
    else
    {
//        printf("%s: hQueryExp != TERM or RANGE\n", __func__);
        eggPrtLog_error("eggQuery", "%s: hQueryExp != TERM or RANGE\n", __func__);
        hQuery->opt = EGGQUERYOPT_DATA;
        return (HEGGQUERYEXP)hQuery;
    }

    HEGGQUERY nextQuery;
    nextQuery = hQuery->q;
    while (nextQuery)
    {
        if (nextQuery->opt == EGGQUERYOPT_DATA)
        {
            hQuery = nextQuery;
            nextQuery = nextQuery->q;
            continue;
        }
        else if (nextQuery->opt == EGGQUERYOPT_OR)
        {
            HEGGQUERY hquery2;
            hquery2 = nextQuery->p;
            HEGGQUERY hquery1;
            hquery1 = hquery2->p;
            if (hquery1->p)
            {
                hquery1->p->q = nextQuery;
            }
            nextQuery->p = hquery1->p;
            
            HEGGIDNODE hIdNodes = 0;
            count_t cnt = 0;
            hIdNodes = eggIdNode_merge_or(hquery1->hIdNodes, 
                                          hquery2->hIdNodes,
                                          hquery1->cnt, hquery2->cnt, &cnt);
            HEGGFIELDKEY hEggFieldKey = hquery1->hFieldKey;
            hEggFieldKey = eggFieldKey_append(hEggFieldKey, hquery2->hFieldKey);
            hquery1->hFieldKey = 0;
            hquery2->hFieldKey = 0;
            free(hquery1->hIdNodes);
            free(hquery1);
            free(hquery2->hIdNodes);
            free(hquery2);
            nextQuery->opt = EGGQUERYOPT_DATA;
            nextQuery->hFieldKey = hEggFieldKey;
            nextQuery->hIdNodes = hIdNodes;
            nextQuery->cnt = cnt;
            hQuery = nextQuery;
            nextQuery = nextQuery->q;
        }
        else if (nextQuery->opt == EGGQUERYOPT_AND)
        {
            HEGGQUERY hquery2;
            hquery2 = nextQuery->p;
            HEGGQUERY hquery1;
            hquery1 = hquery2->p;
            if (hquery1->p)
            {
                hquery1->p->q = nextQuery;
            }
            nextQuery->p = hquery1->p;
            
            HEGGIDNODE hIdNodes = 0;
            count_t cnt = 0;
            hIdNodes = eggIdNode_merge_and(hquery1->hIdNodes, 
                                          hquery2->hIdNodes,
                                           hquery1->cnt, hquery2->cnt, &cnt);
            HEGGFIELDKEY hEggFieldKey = hquery1->hFieldKey;
            hEggFieldKey = eggFieldKey_append(hEggFieldKey, hquery2->hFieldKey);
            hquery1->hFieldKey = 0;
            hquery2->hFieldKey = 0;
            free(hquery1->hIdNodes);
            free(hquery1);
            free(hquery2->hIdNodes);
            free(hquery2);
            nextQuery->opt = EGGQUERYOPT_DATA;
            nextQuery->hFieldKey = hEggFieldKey;
            nextQuery->hIdNodes = hIdNodes;
            nextQuery->cnt = cnt;
            hQuery = nextQuery;
            nextQuery = nextQuery->q;
            
        }
        else if (nextQuery->opt == EGGQUERYOPT_ANDPHRASE)
        {
            HEGGQUERY hquery2;
            hquery2 = nextQuery->p;
            HEGGQUERY hquery1;
            hquery1 = hquery2->p;
            if (hquery1->p)
            {
                hquery1->p->q = nextQuery;
            }
            nextQuery->p = hquery1->p;
            
            HEGGIDNODE hIdNodes = 0;
            count_t cnt = 0;
            hIdNodes = eggIdNode_merge_and(hquery1->hIdNodes, 
                                          hquery2->hIdNodes,
                                           hquery1->cnt, hquery2->cnt, &cnt);
            if (eggQueryList_is_end_of_andphrase(nextQuery))
            {
                /* 一个由eggQuery_new_phrase生成的eggQuery只做一次 */
                //int ret_2 = eggIdNode_filter_by_score(hIdNodes, &cnt);
                int ret_2 = eggIdNode_filter_by_continuation(hIdNodes, &cnt);
                                
            }
            HEGGFIELDKEY hEggFieldKey = hquery1->hFieldKey;
            hEggFieldKey = eggFieldKey_append(hEggFieldKey, hquery2->hFieldKey);
            hquery1->hFieldKey = 0;
            hquery2->hFieldKey = 0;
            free(hquery1->hIdNodes);
            free(hquery1);
            free(hquery2->hIdNodes);
            free(hquery2);
            nextQuery->opt = EGGQUERYOPT_DATA;
            nextQuery->hFieldKey = hEggFieldKey;
            nextQuery->hIdNodes = hIdNodes;
            nextQuery->cnt = cnt;
            hQuery = nextQuery;
            nextQuery = nextQuery->q;
            
        }
        else if (nextQuery->opt == EGGQUERYOPT_MINUS)
        {
            HEGGQUERY hquery2;
            hquery2 = nextQuery->p;
            HEGGQUERY hquery1;
            hquery1 = hquery2->p;
            if (hquery1->p)
            {
                hquery1->p->q = nextQuery;
            }
            nextQuery->p = hquery1->p;
            
            HEGGIDNODE hIdNodes = 0;
            count_t cnt = 0;
            hIdNodes = eggIdNode_merge_minus(hquery1->hIdNodes, 
                                             hquery2->hIdNodes,
                                             hquery1->cnt, hquery2->cnt, &cnt);
            HEGGFIELDKEY hEggFieldKey = hquery1->hFieldKey;
            hEggFieldKey = eggFieldKey_append(hEggFieldKey, hquery2->hFieldKey);
            hquery1->hFieldKey = 0;
            hquery2->hFieldKey = 0;
            free(hquery1->hIdNodes);
            free(hquery1);
            free(hquery2->hIdNodes);
            free(hquery2);
            nextQuery->opt = EGGQUERYOPT_DATA;
            nextQuery->hFieldKey = hEggFieldKey;
            nextQuery->hIdNodes = hIdNodes;
            nextQuery->cnt = cnt;
            hQuery = nextQuery;
            nextQuery = nextQuery->q;
            
        }        
        else
        {
            return (HEGGQUERYEXP)nextQuery;
        }
        
    }
    return (HEGGQUERYEXP)hQuery;
}

HEGGIDNODE eggQuery_finalize_exp(HEGGQUERYEXP hQueryExp, count_t *cnt, HEGGFIELDKEY *hhFieldKey)
{
    if (!cnt)
    {
        return NULL;
    }
    if (!hQueryExp)
    {
        *cnt = 0;
        *hhFieldKey = NULL;
        return NULL;
    }
    HEGGIDNODE hIdNodes;
    HEGGQUERY query;
    query = (HEGGQUERY)hQueryExp;
    if (query->opt == EGGQUERYOPT_DATA
        && query->p == 0 && query->q == 0)
    {
        hIdNodes = query->hIdNodes;
        *cnt = query->cnt;
        *hhFieldKey = query->hFieldKey;
        free(query);
        return hIdNodes;
    }

//    printf("[%s]UsageError and Clean\n", __func__);
    eggPrtLog_error("eggQuery", "[%s]UsageError and Clean\n", __func__);
    hIdNodes = 0;
    *cnt = 0;
    HEGGQUERY head;
    head = query;
    while (head->p)
    {
        head = head->p;
    }
    eggQueryList_destroyList(head);
    
    return hIdNodes;
    
}


/*
  query parse
 */
typedef struct eggQueryOpcode EGGQUERYOPCODE;
typedef struct eggQueryOpcode* HEGGQUERYOPCODE;


typedef struct eggQueryOpObject* HEGGQUERYOPOBJECT;
typedef struct eggQueryOpObject EGGQUERYOPOBJECT;
#pragma pack(1)
struct eggQueryOpObject
{
    char mark[2];//mark[0]='0' mark[1]='x' 
    void* object;
};

struct eggQueryOpcode
{
    ebyte op;
    ebyte in;
    ebyte out;
};


/* EGGQUERYOPCODE eggArithOpSet[128] = {['+'].in=3, */
/*                                      ['+'].out=2, */
/*                                      ['+'].op='+', */
/*                                      ['-'].in=3, */
/*                                      ['-'].out=2, */
/*                                      ['-'].op='-', */
/*                                      ['*'].in=5, */
/*                                      ['*'].out=4, */
/*                                      ['*'].op='*', */
/*                                      ['/'].in=5, */
/*                                      ['/'].out=4, */
/*                                      ['/'].op='/', */
/*                                      ['('].in=1, */
/*                                      ['('].out=8, */
/*                                      ['('].op='(', */
/*                                      [')'].in=8, */
/*                                      [')'].out=1, */
/*                                      [')'].op=')'}; */

EGGQUERYOPCODE eggLogicOpSet[128] = {['&'].in=3,
                                     ['&'].out=2,
                                     ['&'].op='&',
                                     ['|'].in=3,
                                     ['|'].out=2,
                                     ['|'].op='|',
                                     ['('].in=1,
                                     ['('].out=8,
                                     ['('].op='(',
                                     [')'].in=8,
                                     [')'].out=1,
                                     [')'].op=')'};

HEGGQUERYOPCODE eggQueryOpSet = eggLogicOpSet;//eggArithOpSet;
typedef int (*GET_RESULT_BY_OP) (ebyte op, HEGGQUERYOPOBJECT* objects, int* p_pos);

static char* eggQuery_infix_to_suffix(char* pInfix, int nInfix);

static void* eggQuery_suffix_to_result(char* pSufix, int nSufix, GET_RESULT_BY_OP fnOp);

static int get_result_by_logic (ebyte op, HEGGQUERYOPOBJECT* objects, int* p_pos);


static int get_result_by_arith (ebyte op, HEGGQUERYOPOBJECT* objects, int* p_pos);

int get_result_by_op (ebyte op, HEGGQUERYOPOBJECT* objects, int* p_pos);

//(0xaddr1 + 0xaddr2)

HEGGQUERY eggQuery_parse_expression(char* format, ...)
{
    int n_infix_len = strlen(format) + 1;
    
    char* p_format_iter = format;
    while(p_format_iter = strcasestr(p_format_iter, "query"))
    {
        n_infix_len += sizeof(EGGQUERYOPOBJECT) - 5;
        p_format_iter += 5;
    }
    
    char* p_infix = (char*)calloc(1, n_infix_len);
    char* p_infix_iter = p_infix;
    
    EGGQUERYOPOBJECT st_op_object;
    st_op_object.mark[0] = '0';
    st_op_object.mark[1] = 'x';
    
    char* p_format_last = format;
    p_format_iter = format;
    
    va_list arg_list;
    va_start(arg_list, format);
    
    while(p_format_iter = strcasestr(p_format_iter, "query"))
    {
        memcpy(p_infix_iter, p_format_last, (long)(p_format_iter) - (long)(p_format_last));
        p_infix_iter += (long)(p_format_iter) - (long)(p_format_last);
        
        st_op_object.object = va_arg(arg_list, void*);
        memcpy(p_infix_iter, &st_op_object, sizeof(st_op_object));
        p_infix_iter += sizeof(st_op_object);
            
        p_format_iter += 5;
        p_format_last = p_format_iter;
    }
    strcpy(p_infix_iter, p_format_last);

    va_end(arg_list);

    char* p_sufix = eggQuery_infix_to_suffix(p_infix, n_infix_len);

    HEGGQUERY lp_query = eggQuery_suffix_to_result(p_sufix, n_infix_len, get_result_by_logic);
    
    return  lp_query;
    
}

static char* eggQuery_infix_to_suffix(char* pInfix, int nInfix)
{
    int n_infix_len = nInfix;
    char* p_sufix = (char*)calloc(1, n_infix_len + 1);
    
    HEGGQUERYOPCODE p_op_sign_stack = (HEGGQUERYOPCODE)calloc(1, sizeof(EGGQUERYOPCODE) * n_infix_len);
    int op_sign_pos = -1;
    
//    HEGGQUERYOPOBJECT* p_op_num_stack = (HEGGQUERYOPOBJECT*)calloc(1,  sizeof(HEGGQUERYOPOBJECT) * n_infix_len);
    int op_num_pos = 0;
    
    int p_infix_idx = 0;
    int p_sufix_idx = 0;
    char* p_infix_iter = pInfix;
    char* p_sufix_iter = p_sufix;
    
    while(*p_infix_iter)
    {
        if(eggQueryOpSet[*p_infix_iter].op == *p_infix_iter)//*p_infix_iter is op
        {
            while(op_sign_pos > -1)
            {
                if(p_op_sign_stack[op_sign_pos].in > eggQueryOpSet[*p_infix_iter].out) 
                {
                    *p_sufix_iter = p_op_sign_stack[op_sign_pos].op;
                    p_sufix_iter += 1;
                }
                else //'(' or ')'
                {
                    break;
                }
                
                op_sign_pos--;
            }
            p_op_sign_stack[++op_sign_pos] =  eggQueryOpSet[*p_infix_iter];
            p_infix_iter++;
        }
        else
        {
            if(*p_infix_iter == '0' && *(p_infix_iter + 1) == 'x')
            {
                memcpy(p_sufix_iter, p_infix_iter, sizeof(EGGQUERYOPOBJECT));
                
                p_sufix_iter += sizeof(EGGQUERYOPOBJECT);
                p_infix_iter += sizeof(EGGQUERYOPOBJECT);
            }
            else
            {
                return EGG_NULL;
            }
        }
    }
    
    while(op_sign_pos > -1)
    {
        *p_sufix_iter = p_op_sign_stack[op_sign_pos].op;
        p_sufix_iter += 1;
        op_sign_pos--;
     
    }
    
    *p_sufix_iter = 0;
    return p_sufix;
}

static void* eggQuery_suffix_to_result(char* pSufix, int nSufix, GET_RESULT_BY_OP fnOp)
{
    HEGGQUERYOPOBJECT* p_op_object_stack = (HEGGQUERYOPOBJECT*)calloc(1,  sizeof(HEGGQUERYOPOBJECT) * nSufix);
    int n_object_len = -1;
    char* p_sufix_iter = pSufix;
    
    while(*p_sufix_iter)
    {
        if(eggQueryOpSet[*p_sufix_iter].op == *p_sufix_iter)//*p_infix_iter is op
        {
            fnOp(*p_sufix_iter, p_op_object_stack, &n_object_len);
            p_sufix_iter++;
        }
        else
        {
            if(*p_sufix_iter == '0' && *(p_sufix_iter + 1) == 'x')
            {
                p_op_object_stack[++n_object_len] = (HEGGQUERYOPOBJECT)p_sufix_iter;
                p_sufix_iter += sizeof(EGGQUERYOPOBJECT);
            }
            else
            {
                return EGG_NULL;
            }
        }
    }
    
    if(n_object_len)
    {
        return EGG_NULL;
    }
    else
    {
        return p_op_object_stack[n_object_len]->object;
    }
}

static int get_result_by_logic (ebyte op, HEGGQUERYOPOBJECT* objects, int* p_pos)
{
    HEGGQUERY result;
    switch(op)
    {
    case '&':
        result =  eggQuery_and( objects[*p_pos - 1]->object, objects[*p_pos]->object);
        (*p_pos) -= 2;
        break;
        
    case '|':
        result =  eggQuery_or( objects[*p_pos - 1]->object, objects[*p_pos]->object);
        (*p_pos) -= 2;
        break;
        
    case '(': return 1;
    case ')': return 1;
        
    }

    objects[++(*p_pos)]->object = result;
    return 1;
}


static int get_result_by_arith (ebyte op, HEGGQUERYOPOBJECT* objects, int* p_pos)
{
    int result;
    switch(op)
    {
    case '+':
        result = *(int*)(objects[*p_pos - 1]->object)  + *(int*)(objects[*p_pos]->object);
        (*p_pos) -= 2;
        break;
    case '-':
        result = *(int*)(objects[*p_pos - 1]->object)  - *(int*)(objects[*p_pos]->object);
        (*p_pos) -= 2;
        break;
    case '*':
        result = *(int*)(objects[*p_pos - 1]->object)  * *(int*)(objects[*p_pos]->object);
        (*p_pos) -= 2;
        break;
    case '/':
        result = *(int*)(objects[*p_pos - 1]->object)  / *(int*)(objects[*p_pos]->object);
        (*p_pos) -= 2;
        break;
    case '(': return 1;
    case ')': return 1;
        
    }

    *(int*)(objects[++(*p_pos)]->object) = result;
    return 1;
}


/* for back-compliance */

HEGGQUERY EGGAPI eggQuery_set_dictName(HEGGQUERY hQuery, char *dictName)
{
    return eggQuery_set_dictname(hQuery, dictName);
}
HEGGQUERYEXP eggQuery_nextExp(HEGGQUERYEXP hQueryExp, HEGGIDNODE lastResult, count_t lastResultCnt)
{
    return eggQuery_next_exp(hQueryExp, lastResult, lastResultCnt);
}
HEGGQUERY EGGAPI eggQuery_new_stringRangeNN(const char* fieldName, char *start, char *end)
{
    return eggQuery_new_stringrange_nn(fieldName, start, end);
}
HEGGQUERY EGGAPI eggQuery_new_stringRangeNE(const char* fieldName, char *start, char *end)
{
    return eggQuery_new_stringrange_ne(fieldName, start, end);
}
HEGGQUERY EGGAPI eggQuery_new_stringRangeLT(const char* fieldName, char *end)
{
    return eggQuery_new_stringrange_lt(fieldName, end);
}
HEGGQUERY EGGAPI eggQuery_new_stringRangeLE(const char* fieldName, char *end)
{
    return eggQuery_new_stringrange_le(fieldName, end);
}
HEGGQUERY EGGAPI eggQuery_new_stringRangeGT(const char* fieldName, char *start)
{
    return eggQuery_new_stringrange_gt(fieldName, start);
}
HEGGQUERY EGGAPI eggQuery_new_stringRangeGE(const char* fieldName, char *start)
{
    return eggQuery_new_stringrange_ge(fieldName, start);
}
HEGGQUERY EGGAPI eggQuery_new_stringRangeEN(const char* fieldName, char *start, char *end)
{
    return eggQuery_new_stringrange_en(fieldName, start, end);
}
HEGGQUERY EGGAPI eggQuery_new_stringRangeALL(const char* fieldName)
{
    return eggQuery_new_stringrange_all(fieldName);
}
HEGGQUERY EGGAPI eggQuery_new_stringRange(const char* fieldName, char *start, char *end)
{
    return eggQuery_new_stringrange(fieldName, start, end);
}
HEGGQUERY EGGAPI eggQuery_new_doubleRangeNN(const char* fieldName, double number1, double number2)
{
    return eggQuery_new_doublerange_nn(fieldName, number1, number2);
}
HEGGQUERY EGGAPI eggQuery_new_doubleRangeNE(const char* fieldName, double number1, double number2)
{
    return eggQuery_new_doublerange_ne(fieldName, number1, number2);
}
HEGGQUERY EGGAPI eggQuery_new_doubleRangeLT(const char* fieldName, double number2)
{
    return eggQuery_new_doublerange_lt(fieldName, number2);
}
HEGGQUERY EGGAPI eggQuery_new_doubleRangeLE(const char* fieldName, double number2)
{
    return eggQuery_new_doublerange_le(fieldName, number2);
}
HEGGQUERY EGGAPI eggQuery_new_doubleRangeGT(const char* fieldName, double number1)
{
    return eggQuery_new_doublerange_gt(fieldName, number1);
}
HEGGQUERY EGGAPI eggQuery_new_doubleRangeGE(const char* fieldName, double number1)
{
    return eggQuery_new_doublerange_ge(fieldName, number1);
}
HEGGQUERY EGGAPI eggQuery_new_doubleRangeEN(const char* fieldName, double number1, double number2)
{
    return eggQuery_new_doublerange_en(fieldName, number1, number2);
}
HEGGQUERY EGGAPI eggQuery_new_doubleRangeALL(const char* fieldName)
{
    return eggQuery_new_doublerange_all(fieldName);
}
HEGGQUERY EGGAPI eggQuery_new_doubleRange(const char* fieldName, double number1, double number2)
{
    return eggQuery_new_doublerange(fieldName, number1, number2);
}
HEGGQUERY EGGAPI eggQuery_newString(const char* fieldName, const echar* keyword, size16_t keywordSz, type_t type, ...)
{
        char *analyzerName = NULL;
        
    if (type & EGG_NOT_ANALYZED)
    {
        analyzerName = NULL;
    }
    else if (type & EGG_CN_ANALYZED)
    {
        analyzerName = ANALYZER_CNLEX;
    }
    else if (type & EGG_CY_ANALYZED)
    {
        analyzerName = ANALYZER_CYLEX;
    }
    else if (type & EGG_CWS_ANALYZED)
    {
        analyzerName = ANALYZER_CWSLEX;
    }
    else if (type & EGG_OTHER_ANALYZED)
    {
        va_list ap_arg;
        va_start(ap_arg, type);
        analyzerName = va_arg(ap_arg, char*);
        va_end(ap_arg);
    }
    else
    {
        analyzerName = NULL;
    }
    return eggQuery_new_string(fieldName, keyword, keywordSz, analyzerName);
}
HEGGQUERY EGGAPI eggQuery_newSentence(const char* fieldName, const echar* keyword, size16_t keywordSz, type_t type, ...)
{
    char *analyzerName = NULL;
        
    if (type & EGG_NOT_ANALYZED)
    {
        analyzerName = NULL;
    }
    else if (type & EGG_CN_ANALYZED)
    {
        analyzerName = ANALYZER_CNLEX;
    }
    else if (type & EGG_CY_ANALYZED)
    {
        analyzerName = ANALYZER_CYLEX;
    }
    else if (type & EGG_CWS_ANALYZED)
    {
        analyzerName = ANALYZER_CWSLEX;
    }
    else if (type & EGG_OTHER_ANALYZED)
    {
        va_list ap_arg;
        va_start(ap_arg, type);
        analyzerName = va_arg(ap_arg, char*);
        va_end(ap_arg);
    }
    else
    {
        analyzerName = NULL;
    }
    return eggQuery_new_sentence(fieldName, keyword, keywordSz, analyzerName);    
}
HEGGQUERY EGGAPI eggQuery_newPhrase(const char* fieldName, const echar* keyword, size16_t keywordSz, type_t type, ...)
{

        char *analyzerName = NULL;
        
    if (type & EGG_NOT_ANALYZED)
    {
        analyzerName = NULL;
    }
    else if (type & EGG_CN_ANALYZED)
    {
        analyzerName = ANALYZER_CNLEX;
    }
    else if (type & EGG_CY_ANALYZED)
    {
        analyzerName = ANALYZER_CYLEX;
    }
    else if (type & EGG_CWS_ANALYZED)
    {
        analyzerName = ANALYZER_CWSLEX;
    }
    else if (type & EGG_OTHER_ANALYZED)
    {
        va_list ap_arg;
        va_start(ap_arg, type);
        analyzerName = va_arg(ap_arg, char*);
        va_end(ap_arg);
    }
    else
    {
        analyzerName = NULL;
    }
    return eggQuery_new_phrase(fieldName, keyword, keywordSz, analyzerName);
}
HEGGQUERYEXP eggQuery_initExp(HEGGQUERY hQuery)
{
    return eggQuery_init_exp(hQuery);
}
HEGGQUERY eggQuery_findRangeType(HEGGQUERY hQuery)
{
    return eggQuery_find_rangetype(hQuery);
}
HEGGIDNODE eggQuery_finalExp(HEGGQUERYEXP hQueryExp, count_t *cnt, HEGGFIELDKEY *hhFieldKey)
{
    return eggQuery_finalize_exp(hQueryExp, cnt, hhFieldKey);
}
HEGGQUERY EGGAPI eggQuery_boostPreference(HEGGQUERY hQueryExpress1, eggBoost_t boost)
{
    return eggQuery_boost_preference(hQueryExpress1, boost);
}
HEGGQUERY EGGAPI eggQuery_andPhrase(HEGGQUERY hQueryExpress1, HEGGQUERY hQueryExpress2)
{
    return eggQuery_and_phrase(hQueryExpress1, hQueryExpress2);
}

HEGGQUERY EGGAPI eggQuery_new_int32Range(const char* fieldName, int32_t number1, int32_t number2)
{
    return eggQuery_new_int32range(fieldName, number1, number2);
}
HEGGQUERY EGGAPI eggQuery_new_int32RangeEN(const char* fieldName, int32_t number1, int32_t number2)
{
    return eggQuery_new_int32range_en(fieldName, number1, number2);
}
HEGGQUERY EGGAPI eggQuery_new_int32RangeNN(const char* fieldName, int32_t number1, int32_t number2)
{
    return eggQuery_new_int32range_nn(fieldName, number1, number2);
}
HEGGQUERY EGGAPI eggQuery_new_int32RangeNE(const char* fieldName, int32_t number1, int32_t number2)
{
    return eggQuery_new_int32range_ne(fieldName, number1, number2);
}

HEGGQUERY EGGAPI eggQuery_new_int32RangeGT(const char* fieldName, int32_t number1)
{
    return eggQuery_new_int32range_gt(fieldName, number1);
}

HEGGQUERY EGGAPI eggQuery_new_int32RangeGE(const char* fieldName, int32_t number1)
{
    return eggQuery_new_int32range_ge(fieldName, number1);
}

HEGGQUERY EGGAPI eggQuery_new_int32RangeLT(const char* fieldName, int32_t number2)
{
    return eggQuery_new_int32range_lt(fieldName, number2);
}

HEGGQUERY EGGAPI eggQuery_new_int32RangeLE(const char* fieldName, int32_t number2)
{
    return eggQuery_new_int32range_le(fieldName, number2);
}

HEGGQUERY EGGAPI eggQuery_new_int32RangeALL(const char* fieldName)
{
    return eggQuery_new_int32range_all(fieldName);
}

HEGGQUERY EGGAPI eggQuery_new_int64Range(const char* fieldName, int64_t number1, int64_t number2)
{
    return eggQuery_new_int64range(fieldName, number1, number2);
}

HEGGQUERY EGGAPI eggQuery_new_int64RangeEN(const char* fieldName, int64_t number1, int64_t number2)
{
    return eggQuery_new_int64range_en(fieldName, number1, number2);
}

HEGGQUERY EGGAPI eggQuery_new_int64RangeNN(const char* fieldName, int64_t number1, int64_t number2)
{
    return eggQuery_new_int64range_nn(fieldName, number1, number2);
}

HEGGQUERY EGGAPI eggQuery_new_int64RangeNE(const char* fieldName, int64_t number1, int64_t number2)
{
    return eggQuery_new_int64range_ne(fieldName, number1, number2);
}

HEGGQUERY EGGAPI eggQuery_new_int64RangeGT(const char* fieldName, int64_t number1)
{
    return eggQuery_new_int64range_gt(fieldName, number1);
}

HEGGQUERY EGGAPI eggQuery_new_int64RangeGE(const char* fieldName, int64_t number1)
{
    return eggQuery_new_int64range_ge(fieldName, number1);
}

HEGGQUERY EGGAPI eggQuery_new_int64RangeLT(const char* fieldName, int64_t number2)
{
    return eggQuery_new_int64range_lt(fieldName, number2);
}

HEGGQUERY EGGAPI eggQuery_new_int64RangeLE(const char* fieldName, int64_t number2)
{
    return eggQuery_new_int64range_le(fieldName, number2);
}

HEGGQUERY EGGAPI eggQuery_new_int64RangeALL(const char* fieldName)
{
    return eggQuery_new_int64range_all(fieldName);
}

/* for back-compliance end */
