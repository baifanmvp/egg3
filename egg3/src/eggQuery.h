/**
   \file eggQuery.h
   \brief 查询 (API)
   \ingroup egg
*/
#ifndef EGGQUERY_H_
#define EGGQUERY_H_
#include "./EggDef.h"
#include "./eggFieldKey.h"
#include "egglib/emacros.h"
#include "./eggAnalyzer.h"
#include "./storage/eggIdNode.h"

E_BEGIN_DECLS

/*!
  \typedef _ITFPTR HQUERY
  \brief
*/
struct eggQuery;
typedef struct eggQuery *HEGGQUERY;


/*!
  \fn HQUERY EGGAIP eggQuery_new_string
  \brief 查询string
  \param hAnalyzer 分词器句柄，不分词为NULL
  \return
*/
HEGGQUERY EGGAPI eggQuery_new_string(const char* fieldName, const echar* keyword, size16_t keywordSz, char *analyzerName);

HEGGQUERY EGGAPI eggQuery_new_phrase(const char* fieldName, const echar* keyword, size16_t keywordSz, char *analyzerName);

HEGGQUERY EGGAPI eggQuery_new_sentence(const char* fieldName, const echar* keyword, size16_t keywordSz, char *analyzerName);

HEGGQUERY EGGAPI eggQuery_new_string_bytype(const char* fieldName, const echar* keyword, size16_t keywordSz, type_t type, ...);

HEGGQUERY EGGAPI eggQuery_new_phrase_bytype(const char* fieldName, const echar* keyword, size16_t keywordSz, type_t type, ...);

HEGGQUERY EGGAPI eggQuery_new_sentence_bytype(const char* fieldName, const echar* keyword, size16_t keywordSz, type_t type, ...);

/* 设置hQuery的词库名称 */
HEGGQUERY EGGAPI eggQuery_set_dictname(HEGGQUERY hQuery, char *dictName);

/*!
  \fn HQUERY EGGAIP eggQuery_new_int32
  \brief 查询32位integer
  \return
*/
HEGGQUERY EGGAPI eggQuery_new_int32(const char* fieldName, int32_t number1);

/*!
  \fn HQUERY EGGAIP eggQuery_new_int64
  \brief 查询64位integer
  \return
*/
HEGGQUERY EGGAPI eggQuery_new_int64(const char* fieldName, int64_t number1);

/*!
  \fn HQUERY EGGAIP eggQuery_new_double
  \brief 查询double
  \return
*/
HEGGQUERY EGGAPI eggQuery_new_double(const char* fieldName, double number1);

/*!
  \fn HQUERY EGGAIP eggQuery_new_doublerange
  \brief 查询范围, double, number1<= ... <= number2
  \return
*/
HEGGQUERY EGGAPI eggQuery_new_doublerange(const char* fieldName, double number1, double number2);
/*!
  \fn HQUERY EGGAIP eggQuery_new_doublerange_en
  \brief 查询范围, double, number1<= ... < number2
  \return
*/
HEGGQUERY EGGAPI eggQuery_new_doublerange_en(const char* fieldName, double number1, double number2);
/*!
  \fn HQUERY EGGAIP eggQuery_new_doublerange_nn
  \brief 查询范围, double, number1< ... < number2
  \return
*/
HEGGQUERY EGGAPI eggQuery_new_doublerange_nn(const char* fieldName, double number1, double number2);
/*!
  \fn HQUERY EGGAIP eggQuery_new_doublerange_ne
  \brief 查询范围, double, number1< ... <= number2
  \return
*/
HEGGQUERY EGGAPI eggQuery_new_doublerange_ne(const char* fieldName, double number1, double number2);
/*!
  \fn HQUERY EGGAIP eggQuery_new_doublerange_gt
  \brief 查询范围, double, number1< ... 
  \return
*/
HEGGQUERY EGGAPI eggQuery_new_doublerange_gt(const char* fieldName, double number1);
/*!
  \fn HQUERY EGGAIP eggQuery_new_doublerange_ge
  \brief 查询范围, double, number1<= ... 
  \return
*/
HEGGQUERY EGGAPI eggQuery_new_doublerange_ge(const char* fieldName, double number1);
/*!
  \fn HQUERY EGGAIP eggQuery_new_doublerange_lt
  \brief 查询范围, double,  ... < number2
  \return
*/
HEGGQUERY EGGAPI eggQuery_new_doublerange_lt(const char* fieldName, double number2);
/*!
  \fn HQUERY EGGAIP eggQuery_new_doublerange_le
  \brief 查询范围, double,  ... <= number2
  \return
*/
HEGGQUERY EGGAPI eggQuery_new_doublerange_le(const char* fieldName, double number2);
/*!
  \fn HQUERY EGGAIP eggQuery_new_doublerange_all
  \brief 查询范围, double, -unlimited < ... < +unlimited
  \return
*/
HEGGQUERY EGGAPI eggQuery_new_doublerange_all(const char* fieldName);




/*!
  \fn HQUERY EGGAIP eggQuery_new_int32range
  \brief 查询范围, 32位integer, number1 <= ... <= number2
  \return
*/
HEGGQUERY EGGAPI eggQuery_new_int32range(const char* fieldName, int32_t number1, int32_t number2);
/*!
  \fn HQUERY EGGAIP eggQuery_new_int32range_en
  \brief 查询范围, 32位integer, number1 <= ... < number2
  \return
*/
HEGGQUERY EGGAPI eggQuery_new_int32range_en(const char* fieldName, int32_t number1, int32_t number2);
/*!
  \fn HQUERY EGGAIP eggQuery_new_int32range_nn
  \brief 查询范围, 32位integer, number1 < ... < number2
  \return
*/
HEGGQUERY EGGAPI eggQuery_new_int32range_nn(const char* fieldName, int32_t number1, int32_t number2);
/*!
  \fn HQUERY EGGAIP eggQuery_new_int32range_ne
  \brief 查询范围, 32位integer, number1 < ... <= number2
  \return
*/
HEGGQUERY EGGAPI eggQuery_new_int32range_ne(const char* fieldName, int32_t number1, int32_t number2);
/*!
  \fn HQUERY EGGAIP eggQuery_new_int32range_gt
  \brief 查询范围, 32位integer, number1 < ...
  \return
*/
HEGGQUERY EGGAPI eggQuery_new_int32range_gt(const char* fieldName, int32_t number1);
/*!
  \fn HQUERY EGGAIP eggQuery_new_int32range_ge
  \brief 查询范围, 32位integer, number1 <= ... 
  \return
*/
HEGGQUERY EGGAPI eggQuery_new_int32range_ge(const char* fieldName, int32_t number1);
/*!
  \fn HQUERY EGGAIP eggQuery_new_int32range_lt
  \brief 查询范围, 32位integer, ... < number2
  \return
*/
HEGGQUERY EGGAPI eggQuery_new_int32range_lt(const char* fieldName, int32_t number2);
/*!
  \fn HQUERY EGGAIP eggQuery_new_int32range_le
  \brief 查询范围, 32位integer, ... <= number2
  \return
*/
HEGGQUERY EGGAPI eggQuery_new_int32range_le(const char* fieldName, int32_t number2);
/*!
  \fn HQUERY EGGAIP eggQuery_new_int32range_all
  \brief 查询范围, 32位integer, -unlimited < ... < +unlimited
  \return
*/
HEGGQUERY EGGAPI eggQuery_new_int32range_all(const char* fieldName);




/*!
  \fn HQUERY EGGAIP eggQuery_new_int64range
  \brief 查询范围, 64位integer, number1 <= ... <= number2
  \return
*/
HEGGQUERY EGGAPI eggQuery_new_int64range(const char* fieldName, int64_t number1, int64_t number2);
/*!
  \fn HQUERY EGGAIP eggQuery_new_int64range_en
  \brief 查询范围, 64位integer, number1 <= ... < number2
  \return
*/
HEGGQUERY EGGAPI eggQuery_new_int64range_en(const char* fieldName, int64_t number1, int64_t number2);
/*!
  \fn HQUERY EGGAIP eggQuery_new_int64range_nn
  \brief 查询范围, 64位integer, number1 < ... < number2
  \return
*/
HEGGQUERY EGGAPI eggQuery_new_int64range_nn(const char* fieldName, int64_t number1, int64_t number2);
/*!
  \fn HQUERY EGGAIP eggQuery_new_int64range_ne
  \brief 查询范围, 64位integer, number1 < ... <= number2
  \return
*/
HEGGQUERY EGGAPI eggQuery_new_int64range_ne(const char* fieldName, int64_t number1, int64_t number2);
/*!
  \fn HQUERY EGGAIP eggQuery_new_int64range_gt
  \brief 查询范围, 64位integer, number1 < ... 
  \return
*/
HEGGQUERY EGGAPI eggQuery_new_int64range_gt(const char* fieldName, int64_t number1);
/*!
  \fn HQUERY EGGAIP eggQuery_new_int64range_ge
  \brief 查询范围, 64位integer, number1 <= ... 
  \return
*/
HEGGQUERY EGGAPI eggQuery_new_int64range_ge(const char* fieldName, int64_t number1);
/*!
  \fn HQUERY EGGAIP eggQuery_new_int64range_lt
  \brief 查询范围, 64位integer,  ... < number2
  \return
*/
HEGGQUERY EGGAPI eggQuery_new_int64range_lt(const char* fieldName, int64_t number2);
/*!
  \fn HQUERY EGGAIP eggQuery_new_int64range_le
  \brief 查询范围, 64位integer, ... <= number2
  \return
*/
HEGGQUERY EGGAPI eggQuery_new_int64range_le(const char* fieldName, int64_t number2);
/*!
  \fn HQUERY EGGAIP eggQuery_new_int64range_all
  \brief 查询范围, 64位integer, -unlimited < ... < +unlimted
  \return
*/
HEGGQUERY EGGAPI eggQuery_new_int64range_all(const char* fieldName);



/*!
  \fn HQUERY EGGAIP eggQuery_new_stringrange
  \brief 查询范围, string, start <= ... <= end
  \return
*/
HEGGQUERY EGGAPI eggQuery_new_stringrange(const char* fieldName, char *start, char *end);
/*!
  \fn HQUERY EGGAIP eggQuery_new_stringrange_en
  \brief 查询范围, string, start <= ... < end
  \return
*/
HEGGQUERY EGGAPI eggQuery_new_stringrange_en(const char* fieldName, char *start, char *end);
/*!
  \fn HQUERY EGGAIP eggQuery_new_stringrange_nn
  \brief 查询范围, string, start < ... < end
  \return
*/
HEGGQUERY EGGAPI eggQuery_new_stringrange_nn(const char* fieldName, char *start, char *end);
/*!
  \fn HQUERY EGGAIP eggQuery_new_stringrange_ne
  \brief 查询范围, string, start < ... <= end
  \return
*/
HEGGQUERY EGGAPI eggQuery_new_stringrange_ne(const char* fieldName, char *start, char *end);
/*!
  \fn HQUERY EGGAIP eggQuery_new_stringrange_gt
  \brief 查询范围, string, start < ... 
  \return
*/
HEGGQUERY EGGAPI eggQuery_new_stringrange_gt(const char* fieldName, char *start);
/*!
  \fn HQUERY EGGAIP eggQuery_new_stringrange_ge
  \brief 查询范围, string, start <= ... 
  \return
*/
HEGGQUERY EGGAPI eggQuery_new_stringrange_ge(const char* fieldName, char *start);
/*!
  \fn HQUERY EGGAIP eggQuery_new_stringrange_lt
  \brief 查询范围, string, ... < end
  \return
*/
HEGGQUERY EGGAPI eggQuery_new_stringrange_lt(const char* fieldName, char *end);
/*!
  \fn HQUERY EGGAIP eggQuery_new_stringrange_le
  \brief 查询范围, string, ... <= end
  \return
*/
HEGGQUERY EGGAPI eggQuery_new_stringrange_le(const char* fieldName, char *end);
/*!
  \fn HQUERY EGGAIP eggQuery_new_stringrange_all
  \brief 查询范围, string, -unlimited < ... < +unlimited
  \return
*/
HEGGQUERY EGGAPI eggQuery_new_stringrange_all(const char* fieldName);


/*!
  \fn HQUERY EGGAIP eggQuery_and
  \brief logic and
  \param hQueryExpress1, hQueryExpress2 不能再被引用
  \return
*/
HEGGQUERY EGGAPI eggQuery_and(HEGGQUERY hQueryExpress1, HEGGQUERY hQueryExpress2);

/*!
  \fn HQUERY EGGAIP eggQuery_and_phrase
  \brief logic and。 hQueryExpress1 和 hQueryExpress2 为同一短语的两部分。内部通过打分，若认为express1和express2不靠在一起的，会被滤掉。
  \param hQueryExpress1, hQueryExpress2 不能再被引用
  \return
*/
HEGGQUERY EGGAPI eggQuery_and_phrase(HEGGQUERY hQueryExpress1, HEGGQUERY hQueryExpress2);

/*!
  \fn HQUERY EGGAIP eggQuery_or
  \brief logic or
  \param hQueryExpress1, hQueryExpress2 不能再被引用
  \return
*/
HEGGQUERY EGGAPI eggQuery_or(HEGGQUERY hQueryExpress1, HEGGQUERY hQueryExpress2);

/*!
  \fn HQUERY EGGAIP eggQuery_minus
  \brief logic or
  \param hQueryExpress1, hQueryExpress2 不能再被引用
  \return
*/
HEGGQUERY EGGAPI eggQuery_minus(HEGGQUERY hQueryExpress1, HEGGQUERY hQueryExpress2);


HEGGQUERY EGGAPI eggQuery_boost_preference(HEGGQUERY hQueryExpress1, eggBoost_t boost);

/*!
  \fn EBOOL EGGAPI egg_Query_delete(HQUERY)
  \brief
  \param hQuery
  \return
*/
EBOOL EGGAPI eggQuery_delete(HEGGQUERY hQuery);


char * EGGAPI eggQuery_serialise(HEGGQUERY hQuery, int *sz);
HEGGQUERY EGGAPI eggQuery_unserialise(char *query, int sz);


/*!
  \fn HEGGQUERY EGGAPI egg_Query_dup(HQUERY)
  \brief
  \param hQuery
  \return
*/
HEGGQUERY EGGAPI eggQuery_dup(HEGGQUERY hQuery);


EBOOL EGGAPI eggQuery_equal(HEGGQUERY hQuery, HEGGQUERY hQuery2);


HEGGQUERY eggQuery_find_rangetype(HEGGQUERY hQuery);

HEGGQUERY eggQuery_parse_expression(char* format, ...);
#define EGGQUERYOPT_RANGE 2
#define EGGQUERYOPT_TERM 3
#define EGGQUERYOPT_PHRASE 4
#define EGGQUERYOPT_TERMOR 5

typedef struct {
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
    
} *HEGGQUERYEXP;
HEGGQUERYEXP eggQuery_init_exp(HEGGQUERY hQuery);
HEGGQUERYEXP eggQuery_next_exp(HEGGQUERYEXP hQueryExp, HEGGIDNODE lastResult, count_t lastResultCnt);
HEGGIDNODE eggQuery_finalize_exp(HEGGQUERYEXP hQueryExp, count_t *cnt, HEGGFIELDKEY *hhFieldKey);


/* for back-compliance */
HEGGQUERY EGGAPI eggQuery_set_dictName(HEGGQUERY hQuery, char *dictName);
HEGGQUERYEXP eggQuery_nextExp(HEGGQUERYEXP hQueryExp, HEGGIDNODE lastResult, count_t lastResultCnt);
HEGGQUERY EGGAPI eggQuery_new_stringRangeNN(const char* fieldName, char *start, char *end);
HEGGQUERY EGGAPI eggQuery_new_stringRangeNE(const char* fieldName, char *start, char *end);
HEGGQUERY EGGAPI eggQuery_new_stringRangeLT(const char* fieldName, char *end);
HEGGQUERY EGGAPI eggQuery_new_stringRangeLE(const char* fieldName, char *end);
HEGGQUERY EGGAPI eggQuery_new_stringRangeGT(const char* fieldName, char *start);
HEGGQUERY EGGAPI eggQuery_new_stringRangeGE(const char* fieldName, char *start);
HEGGQUERY EGGAPI eggQuery_new_stringRangeEN(const char* fieldName, char *start, char *end);
HEGGQUERY EGGAPI eggQuery_new_stringRangeALL(const char* fieldName);
HEGGQUERY EGGAPI eggQuery_new_stringRange(const char* fieldName, char *start, char *end);
HEGGQUERY EGGAPI eggQuery_new_doubleRangeNN(const char* fieldName, double number1, double number2);
HEGGQUERY EGGAPI eggQuery_new_doubleRangeNE(const char* fieldName, double number1, double number2);
HEGGQUERY EGGAPI eggQuery_new_doubleRangeLT(const char* fieldName, double number2);
HEGGQUERY EGGAPI eggQuery_new_doubleRangeLE(const char* fieldName, double number2);
HEGGQUERY EGGAPI eggQuery_new_doubleRangeGT(const char* fieldName, double number1);
HEGGQUERY EGGAPI eggQuery_new_doubleRangeGE(const char* fieldName, double number1);
HEGGQUERY EGGAPI eggQuery_new_doubleRangeEN(const char* fieldName, double number1, double number2);
HEGGQUERY EGGAPI eggQuery_new_doubleRangeALL(const char* fieldName);
HEGGQUERY EGGAPI eggQuery_new_doubleRange(const char* fieldName, double number1, double number2);
HEGGQUERY EGGAPI eggQuery_newString(const char* fieldName, const echar* keyword, size16_t keywordSz, type_t type, ...);
HEGGQUERY EGGAPI eggQuery_newSentence(const char* fieldName, const echar* keyword, size16_t keywordSz, type_t type, ...);
HEGGQUERY EGGAPI eggQuery_newPhrase(const char* fieldName, const echar* keyword, size16_t keywordSz, type_t type, ...);
HEGGQUERYEXP eggQuery_initExp(HEGGQUERY hQuery);
HEGGIDNODE eggQuery_finalExp(HEGGQUERYEXP hQueryExp, count_t *cnt, HEGGFIELDKEY *hhFieldKey);
HEGGQUERY EGGAPI eggQuery_boostPreference(HEGGQUERY hQueryExpress1, eggBoost_t boost);
HEGGQUERY EGGAPI eggQuery_andPhrase(HEGGQUERY hQueryExpress1, HEGGQUERY hQueryExpress2);
HEGGQUERY EGGAPI eggQuery_new_int32Range(const char* fieldName, int32_t number1, int32_t number2);
HEGGQUERY EGGAPI eggQuery_new_int32RangeEN(const char* fieldName, int32_t number1, int32_t number2);
HEGGQUERY EGGAPI eggQuery_new_int32RangeNN(const char* fieldName, int32_t number1, int32_t number2);
HEGGQUERY EGGAPI eggQuery_new_int32RangeNE(const char* fieldName, int32_t number1, int32_t number2);
HEGGQUERY EGGAPI eggQuery_new_int32RangeGT(const char* fieldName, int32_t number1);
HEGGQUERY EGGAPI eggQuery_new_int32RangeGE(const char* fieldName, int32_t number1);
HEGGQUERY EGGAPI eggQuery_new_int32RangeLT(const char* fieldName, int32_t number2);
HEGGQUERY EGGAPI eggQuery_new_int32RangeLE(const char* fieldName, int32_t number2);
HEGGQUERY EGGAPI eggQuery_new_int32RangeALL(const char* fieldName);
HEGGQUERY EGGAPI eggQuery_new_int64Range(const char* fieldName, int64_t number1, int64_t number2);
HEGGQUERY EGGAPI eggQuery_new_int64RangeEN(const char* fieldName, int64_t number1, int64_t number2);
HEGGQUERY EGGAPI eggQuery_new_int64RangeNN(const char* fieldName, int64_t number1, int64_t number2);
HEGGQUERY EGGAPI eggQuery_new_int64RangeNE(const char* fieldName, int64_t number1, int64_t number2);
HEGGQUERY EGGAPI eggQuery_new_int64RangeGT(const char* fieldName, int64_t number1);
HEGGQUERY EGGAPI eggQuery_new_int64RangeGE(const char* fieldName, int64_t number1);
HEGGQUERY EGGAPI eggQuery_new_int64RangeLT(const char* fieldName, int64_t number2);
HEGGQUERY EGGAPI eggQuery_new_int64RangeLE(const char* fieldName, int64_t number2);
HEGGQUERY EGGAPI eggQuery_new_int64RangeALL(const char* fieldName);

/* for back-compliance end */

E_END_DECLS

#endif //EGGQUERY_H_
