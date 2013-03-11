/**
   \file ScoreDoc.h
   \brief 打分文章ID集合
   \ingroup egg
*/
#ifndef EGG_SCOREDOC_H_
#define EGG_SCOREDOC_H_
#include "./EggDef.h"
#include "./eggDid.h"

E_BEGIN_DECLS


/*!
  \typedef struct tagScoreDoc SCOREDOC
  \brief  打分文章ID
*/
typedef struct eggScoreDoc EGGSCOREDOC;
typedef struct eggScoreDoc* HEGGSCOREDOC;
/*!
  \struct tagScoreDoc
  \brief 打分文章ID结构体
*/
#pragma pack(push)
#pragma pack(4)

struct eggScoreDoc
{
    /// 文章ID
    EGGDID idDoc;
    /// 文章打分分数
    score_t score;
    
    int weight;

    char orderBy[8];
    int sort_tmp_val;
};
#pragma pack(pop)


/*!
  \def eggScoreDoc_new(hScoreDoc)
  \brief 打分文章ID构造方法。对\n hEggScoreDoc 进行分配内存空间。
*/
#define EGGSCOREDOC_NEW(hScoreDoc)                              \
    do                                                          \
    {                                                           \
        hScoreDoc = (HEGGSCOREDOC)calloc(1, sizeof(EGGSCOREDOC));       \
        hScoreDoc->idDoc = {0};                                 \
        hScoreDoc->score = 0;                                   \
        hScoreDoc->weight = 0;                                   \
    }while(EGG_FALSE);

/*!
  \def ScoreDoc_new_n(hScoreDoc, n)
  \brief 打分文章ID集合构造方法。对\n hScoreDoc 进行分配内存空间，\a n 指定文章ID的个数。
*/
#define SCOREDOC_NEW_N(hScoreDocs, n)                               \
    do                                                              \
    {                                                               \
        (hScoreDocs) = (HEGGSCOREDOC)malloc(sizeof(EGGSCOREDOC)*n); \
    }while(EGG_FALSE)

/*!
  \def ScoreDoc_delete(hScoreDoc)
  \brief 打分文章ID销毁方法。
*/
#define EGGSCOREDOC_DELETE(hScoreDoc) \
    do                                \
    {                                 \
        free(hScoreDoc);              \
    }while(EGG_FALSE)




/*!
  \def eggScoreDoc_id_i(hScoreDoc, i)
  \brief 从\a hScoreDoc 集合中返回指定 \a i 位置的文章ID。
*/
#define EGGSCOREDOC_ID_I(hScoreDoc, i) \
    (((HEGGSCOREDOC)(hScoreDoc) + i)->idDoc)

/*!
  \def ScoreDoc_score_i(hScoreDoc, i)
  \brief 从\a hScoreDoc 集合中返回指定 \a i 位置的文章打分分数。
*/
#define EGGSCOREDOC_SCORE_I(hScoreDoc, i) \
    (((HEGGSCOREDOC)(hScoreDoc) + i)->score)

/*!
  \def ScoreDoc_weight_i(hScoreDoc, i)
  \brief 从\a hScoreDoc 集合中返回指定 \a i 位置的文章打分分数。
*/
#define EGGSCOREDOC_WEIGHT_I(hScoreDoc, i) \
    (((HEGGSCOREDOC)(hScoreDoc) + i)->weight)

/*!
  \def ScoreDoc_orderby_i(hScoreDoc, i)
  \brief 从\a hScoreDoc 集合中返回指定 \a i 位置的文章打分分数。
*/
#define EGGSCOREDOC_ORDERBY_I(hScoreDoc, i) \
    (((HEGGSCOREDOC)(hScoreDoc) + i)->orderBy)


/*!
  \def ScoreDoc_is_object(hScoreDoc)
  \brief 判断 \a hScoreDoc 是否为有效对象
*/
#define EGGSCOREDOC_IS_INVALID(hScoreDoc) \
    ((!hScoreDoc)?EGG_TRUE:EGG_FALSE)

int eggScoreDoc_cmp_score(HEGGSCOREDOC hSrcScoreDoc, HEGGSCOREDOC hDestScoreDoc);

int eggScoreDoc_cmp_score2(HEGGSCOREDOC hSrcScoreDoc, index_t idx, HEGGSCOREDOC hDestScoreDoc);

int eggScoreDoc_cmp_weight(HEGGSCOREDOC hSrcScoreDoc, HEGGSCOREDOC hDestScoreDoc);

int eggScoreDoc_cmp_weight2(HEGGSCOREDOC hSrcScoreDoc, index_t idx, HEGGSCOREDOC hDestScoreDoc);

int eggScoreDoc_cmp_orderby_int32_asc(HEGGSCOREDOC hSrcScoreDoc, HEGGSCOREDOC hDestScoreDoc);
int eggScoreDoc_cmp_orderby2_int32_asc(HEGGSCOREDOC hSrcScoreDoc, index_t idx, HEGGSCOREDOC hDestScoreDoc);
int eggScoreDoc_cmp_orderby_int32_desc(HEGGSCOREDOC hSrcScoreDoc, HEGGSCOREDOC hDestScoreDoc);
int eggScoreDoc_cmp_orderby2_int32_desc(HEGGSCOREDOC hSrcScoreDoc, index_t idx, HEGGSCOREDOC hDestScoreDoc);
int eggScoreDoc_cmp_orderby_int64_asc(HEGGSCOREDOC hSrcScoreDoc, HEGGSCOREDOC hDestScoreDoc);
int eggScoreDoc_cmp_orderby2_int64_asc(HEGGSCOREDOC hSrcScoreDoc, index_t idx, HEGGSCOREDOC hDestScoreDoc);
int eggScoreDoc_cmp_orderby_int64_desc(HEGGSCOREDOC hSrcScoreDoc, HEGGSCOREDOC hDestScoreDoc);
int eggScoreDoc_cmp_orderby2_int64_desc(HEGGSCOREDOC hSrcScoreDoc, index_t idx, HEGGSCOREDOC hDestScoreDoc);
int eggScoreDoc_cmp_orderby_double_asc(HEGGSCOREDOC hSrcScoreDoc, HEGGSCOREDOC hDestScoreDoc);
int eggScoreDoc_cmp_orderby2_double_asc(HEGGSCOREDOC hSrcScoreDoc, index_t idx, HEGGSCOREDOC hDestScoreDoc);
int eggScoreDoc_cmp_orderby_double_desc(HEGGSCOREDOC hSrcScoreDoc, HEGGSCOREDOC hDestScoreDoc);
int eggScoreDoc_cmp_orderby2_double_desc(HEGGSCOREDOC hSrcScoreDoc, index_t idx, HEGGSCOREDOC hDestScoreDoc);


int eggScoreDoc_cmp_orderbytmpval_int32_asc(HEGGSCOREDOC hSrcScoreDoc, HEGGSCOREDOC hDestScoreDoc);
int eggScoreDoc_cmp_orderbytmpval2_int32_asc(HEGGSCOREDOC hSrcScoreDoc, index_t idx, HEGGSCOREDOC hDestScoreDoc);
int eggScoreDoc_cmp_orderbytmpval_int32_desc(HEGGSCOREDOC hSrcScoreDoc, HEGGSCOREDOC hDestScoreDoc);
int eggScoreDoc_cmp_orderbytmpval2_int32_desc(HEGGSCOREDOC hSrcScoreDoc, index_t idx, HEGGSCOREDOC hDestScoreDoc);
int eggScoreDoc_cmp_orderbytmpval_int64_asc(HEGGSCOREDOC hSrcScoreDoc, HEGGSCOREDOC hDestScoreDoc);
int eggScoreDoc_cmp_orderbytmpval2_int64_asc(HEGGSCOREDOC hSrcScoreDoc, index_t idx, HEGGSCOREDOC hDestScoreDoc);
int eggScoreDoc_cmp_orderbytmpval_int64_desc(HEGGSCOREDOC hSrcScoreDoc, HEGGSCOREDOC hDestScoreDoc);
int eggScoreDoc_cmp_orderbytmpval2_int64_desc(HEGGSCOREDOC hSrcScoreDoc, index_t idx, HEGGSCOREDOC hDestScoreDoc);
int eggScoreDoc_cmp_orderbytmpval_double_asc(HEGGSCOREDOC hSrcScoreDoc, HEGGSCOREDOC hDestScoreDoc);
int eggScoreDoc_cmp_orderbytmpval2_double_asc(HEGGSCOREDOC hSrcScoreDoc, index_t idx, HEGGSCOREDOC hDestScoreDoc);
int eggScoreDoc_cmp_orderbytmpval_double_desc(HEGGSCOREDOC hSrcScoreDoc, HEGGSCOREDOC hDestScoreDoc);
int eggScoreDoc_cmp_orderbytmpval2_double_desc(HEGGSCOREDOC hSrcScoreDoc, index_t idx, HEGGSCOREDOC hDestScoreDoc);




E_END_DECLS

#endif //_EGG_SCORE_DOCUMENT_H_
