#include "../eggScoreDoc.h"

int eggScoreDoc_cmp_score(HEGGSCOREDOC hSrcScoreDoc, HEGGSCOREDOC hDestScoreDoc)
{
    if(hSrcScoreDoc->score > hDestScoreDoc->score)
    {
        return 1;
    }
    else if(hSrcScoreDoc->score < hDestScoreDoc->score)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}

int eggScoreDoc_cmp_score2(HEGGSCOREDOC hSrcScoreDoc, index_t idx, HEGGSCOREDOC hDestScoreDoc)
{
    if((hSrcScoreDoc + idx)->score > hDestScoreDoc->score)
    {
        return 1;
    }
    else if((hSrcScoreDoc + idx)->score < hDestScoreDoc->score)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}

int eggScoreDoc_cmp_weight(HEGGSCOREDOC hSrcScoreDoc, HEGGSCOREDOC hDestScoreDoc)
{
    if(hSrcScoreDoc->weight > hDestScoreDoc->weight)
    {
        return 1;
    }
    else if(hSrcScoreDoc->weight < hDestScoreDoc->weight)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}

int eggScoreDoc_cmp_weight2(HEGGSCOREDOC hSrcScoreDoc, index_t idx, HEGGSCOREDOC hDestScoreDoc)
{
    if((hSrcScoreDoc + idx)->weight > hDestScoreDoc->weight)
    {
        return 1;
    }
    else if((hSrcScoreDoc + idx)->weight < hDestScoreDoc->weight)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}

int eggScoreDoc_cmp_orderby_int32_asc(HEGGSCOREDOC hSrcScoreDoc, HEGGSCOREDOC hDestScoreDoc)
{
    if (*(int32_t*)hSrcScoreDoc->orderBy > *(int32_t*)hDestScoreDoc->orderBy)
    {
        return 1;
    }
    else if (*(int32_t*)hSrcScoreDoc->orderBy < *(int32_t*)hDestScoreDoc->orderBy)
    {
        return -1;
    }
    else
    {
        return 0;
    }

}
int eggScoreDoc_cmp_orderby_int32_desc(HEGGSCOREDOC hSrcScoreDoc, HEGGSCOREDOC hDestScoreDoc)
{
    if (*(int32_t*)hSrcScoreDoc->orderBy < *(int32_t*)hDestScoreDoc->orderBy)
    {
        return 1;
    }
    else if (*(int32_t*)hSrcScoreDoc->orderBy > *(int32_t*)hDestScoreDoc->orderBy)
    {
        return -1;
    }
    else
    {
        return 0;
    }
    
}

int eggScoreDoc_cmp_orderby2_int32_asc(HEGGSCOREDOC hSrcScoreDoc, index_t idx, HEGGSCOREDOC hDestScoreDoc)
{
    if (*(int32_t*)(hSrcScoreDoc+idx)->orderBy > *(int32_t*)hDestScoreDoc->orderBy)
    {
        return 1;
    }
    else if (*(int32_t*)(hSrcScoreDoc+idx)->orderBy < *(int32_t*)hDestScoreDoc->orderBy)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}
int eggScoreDoc_cmp_orderby2_int32_desc(HEGGSCOREDOC hSrcScoreDoc, index_t idx, HEGGSCOREDOC hDestScoreDoc)
{
    if (*(int32_t*)(hSrcScoreDoc+idx)->orderBy < *(int32_t*)hDestScoreDoc->orderBy)
    {
        return 1;
    }
    else if (*(int32_t*)(hSrcScoreDoc+idx)->orderBy > *(int32_t*)hDestScoreDoc->orderBy)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}
int eggScoreDoc_cmp_orderby_int64_asc(HEGGSCOREDOC hSrcScoreDoc, HEGGSCOREDOC hDestScoreDoc)
{
    if (*(int64_t*)hSrcScoreDoc->orderBy > *(int64_t*)hDestScoreDoc->orderBy)
    {
        return 1;
    }
    else if (*(int64_t*)hSrcScoreDoc->orderBy < *(int64_t*)hDestScoreDoc->orderBy)
    {
        return -1;
    }
    else
    {
        return 0;
    }
    
}
int eggScoreDoc_cmp_orderby_int64_desc(HEGGSCOREDOC hSrcScoreDoc, HEGGSCOREDOC hDestScoreDoc)
{
    if (*(int64_t*)hSrcScoreDoc->orderBy < *(int64_t*)hDestScoreDoc->orderBy)
    {
        return 1;
    }
    else if (*(int64_t*)hSrcScoreDoc->orderBy > *(int64_t*)hDestScoreDoc->orderBy)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}
int eggScoreDoc_cmp_orderby2_int64_asc(HEGGSCOREDOC hSrcScoreDoc, index_t idx, HEGGSCOREDOC hDestScoreDoc)
{
    if (*(int64_t*)(hSrcScoreDoc+idx)->orderBy > *(int64_t*)hDestScoreDoc->orderBy)
    {
        return 1;
    }
    else if (*(int64_t*)(hSrcScoreDoc+idx)->orderBy < *(int64_t*)hDestScoreDoc->orderBy)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}
int eggScoreDoc_cmp_orderby2_int64_desc(HEGGSCOREDOC hSrcScoreDoc, index_t idx, HEGGSCOREDOC hDestScoreDoc)
{
    if (*(int64_t*)(hSrcScoreDoc+idx)->orderBy < *(int64_t*)hDestScoreDoc->orderBy)
    {
        return 1;
    }
    else if (*(int64_t*)(hSrcScoreDoc+idx)->orderBy > *(int64_t*)hDestScoreDoc->orderBy)
    {
        return -1;
    }
    else
    {
        return 0;
    }
     
}
int eggScoreDoc_cmp_orderby_double_asc(HEGGSCOREDOC hSrcScoreDoc, HEGGSCOREDOC hDestScoreDoc)
{
    if (*(double*)hSrcScoreDoc->orderBy > *(double*)hDestScoreDoc->orderBy)
    {
        return 1;
    }
    else if (*(double*)hSrcScoreDoc->orderBy < *(double*)hDestScoreDoc->orderBy)
    {
        return -1;
    }
    else
    {
        return 0;
    }
    
}
int eggScoreDoc_cmp_orderby_double_desc(HEGGSCOREDOC hSrcScoreDoc, HEGGSCOREDOC hDestScoreDoc)
{
    if (*(double*)hSrcScoreDoc->orderBy < *(double*)hDestScoreDoc->orderBy)
    {
        return 1;
    }
    else if (*(double*)hSrcScoreDoc->orderBy > *(double*)hDestScoreDoc->orderBy)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}
int eggScoreDoc_cmp_orderby2_double_asc(HEGGSCOREDOC hSrcScoreDoc, index_t idx, HEGGSCOREDOC hDestScoreDoc)
{
    if (*(double*)(hSrcScoreDoc+idx)->orderBy > *(double*)hDestScoreDoc->orderBy)
    {
        return 1;
    }
    else if (*(double*)(hSrcScoreDoc+idx)->orderBy < *(double*)hDestScoreDoc->orderBy)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}
int eggScoreDoc_cmp_orderby2_double_desc(HEGGSCOREDOC hSrcScoreDoc, index_t idx, HEGGSCOREDOC hDestScoreDoc)
{
    if (*(double*)(hSrcScoreDoc+idx)->orderBy < *(double*)hDestScoreDoc->orderBy)
    {
        return 1;
    }
    else if (*(double*)(hSrcScoreDoc+idx)->orderBy > *(double*)hDestScoreDoc->orderBy)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}


int eggScoreDoc_cmp_orderbytmpval_int32_asc(HEGGSCOREDOC hSrcScoreDoc, HEGGSCOREDOC hDestScoreDoc)
{
    if (*(int32_t*)hSrcScoreDoc->orderBy > *(int32_t*)hDestScoreDoc->orderBy)
    {
        return 1;
    }
    else if (*(int32_t*)hSrcScoreDoc->orderBy < *(int32_t*)hDestScoreDoc->orderBy)
    {
        return -1;
    }
    else if (hSrcScoreDoc->sort_tmp_val < hDestScoreDoc->sort_tmp_val) /* !!! */
    {
        return 1;
    }
    else if (hSrcScoreDoc->sort_tmp_val > hDestScoreDoc->sort_tmp_val)
    {
        return -1;
    }
    else
    {
        return 0;
    }

    
}
int eggScoreDoc_cmp_orderbytmpval_int32_desc(HEGGSCOREDOC hSrcScoreDoc, HEGGSCOREDOC hDestScoreDoc)
{
    if (*(int32_t*)hSrcScoreDoc->orderBy < *(int32_t*)hDestScoreDoc->orderBy)
    {
        return 1;
    }
    else if (*(int32_t*)hSrcScoreDoc->orderBy > *(int32_t*)hDestScoreDoc->orderBy)
    {
        return -1;
    }
    else if (hSrcScoreDoc->sort_tmp_val < hDestScoreDoc->sort_tmp_val) /* !!! */
    {
        return 1;
    }
    else if (hSrcScoreDoc->sort_tmp_val > hDestScoreDoc->sort_tmp_val)
    {
        return -1;
    }
    else
    {
        return 0;
    }
    
}

int eggScoreDoc_cmp_orderbytmpval2_int32_asc(HEGGSCOREDOC hSrcScoreDoc, index_t idx, HEGGSCOREDOC hDestScoreDoc)
{
    if (*(int32_t*)(hSrcScoreDoc+idx)->orderBy > *(int32_t*)hDestScoreDoc->orderBy)
    {
        return 1;
    }
    else if (*(int32_t*)(hSrcScoreDoc+idx)->orderBy < *(int32_t*)hDestScoreDoc->orderBy)
    {
        return -1;
    }
    else if ((hSrcScoreDoc+idx)->sort_tmp_val < hDestScoreDoc->sort_tmp_val) /* !!! */
    {
        return 1;
    }
    else if ((hSrcScoreDoc+idx)->sort_tmp_val > hDestScoreDoc->sort_tmp_val)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}
int eggScoreDoc_cmp_orderbytmpval2_int32_desc(HEGGSCOREDOC hSrcScoreDoc, index_t idx, HEGGSCOREDOC hDestScoreDoc)
{
    if (*(int32_t*)(hSrcScoreDoc+idx)->orderBy < *(int32_t*)hDestScoreDoc->orderBy)
    {
        return 1;
    }
    else if (*(int32_t*)(hSrcScoreDoc+idx)->orderBy > *(int32_t*)hDestScoreDoc->orderBy)
    {
        return -1;
    }
    else if ((hSrcScoreDoc+idx)->sort_tmp_val < hDestScoreDoc->sort_tmp_val) /* !!! */
    {
        return 1;
    }
    else if ((hSrcScoreDoc+idx)->sort_tmp_val > hDestScoreDoc->sort_tmp_val)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}
int eggScoreDoc_cmp_orderbytmpval_int64_asc(HEGGSCOREDOC hSrcScoreDoc, HEGGSCOREDOC hDestScoreDoc)
{
    if (*(int64_t*)hSrcScoreDoc->orderBy > *(int64_t*)hDestScoreDoc->orderBy)
    {
        return 1;
    }
    else if (*(int64_t*)hSrcScoreDoc->orderBy < *(int64_t*)hDestScoreDoc->orderBy)
    {
        return -1;
    }
    else if (hSrcScoreDoc->sort_tmp_val < hDestScoreDoc->sort_tmp_val) /* !!! */
    {
        return 1;
    }
    else if (hSrcScoreDoc->sort_tmp_val > hDestScoreDoc->sort_tmp_val)
    {
        return -1;
    }
    else
    {
        return 0;
    }
    
}
int eggScoreDoc_cmp_orderbytmpval_int64_desc(HEGGSCOREDOC hSrcScoreDoc, HEGGSCOREDOC hDestScoreDoc)
{
    if (*(int64_t*)hSrcScoreDoc->orderBy < *(int64_t*)hDestScoreDoc->orderBy)
    {
        return 1;
    }
    else if (*(int64_t*)hSrcScoreDoc->orderBy > *(int64_t*)hDestScoreDoc->orderBy)
    {
        return -1;
    }
    else if (hSrcScoreDoc->sort_tmp_val < hDestScoreDoc->sort_tmp_val) /* !!! */
    {
        return 1;
    }
    else if (hSrcScoreDoc->sort_tmp_val > hDestScoreDoc->sort_tmp_val)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}
int eggScoreDoc_cmp_orderbytmpval2_int64_asc(HEGGSCOREDOC hSrcScoreDoc, index_t idx, HEGGSCOREDOC hDestScoreDoc)
{
    if (*(int64_t*)(hSrcScoreDoc+idx)->orderBy > *(int64_t*)hDestScoreDoc->orderBy)
    {
        return 1;
    }
    else if (*(int64_t*)(hSrcScoreDoc+idx)->orderBy < *(int64_t*)hDestScoreDoc->orderBy)
    {
        return -1;
    }
    else if ((hSrcScoreDoc+idx)->sort_tmp_val < hDestScoreDoc->sort_tmp_val) /* !!! */
    {
        return 1;
    }
    else if ((hSrcScoreDoc+idx)->sort_tmp_val > hDestScoreDoc->sort_tmp_val)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}
int eggScoreDoc_cmp_orderbytmpval2_int64_desc(HEGGSCOREDOC hSrcScoreDoc, index_t idx, HEGGSCOREDOC hDestScoreDoc)
{
    if (*(int64_t*)(hSrcScoreDoc+idx)->orderBy < *(int64_t*)hDestScoreDoc->orderBy)
    {
        return 1;
    }
    else if (*(int64_t*)(hSrcScoreDoc+idx)->orderBy > *(int64_t*)hDestScoreDoc->orderBy)
    {
        return -1;
    }
    else if ((hSrcScoreDoc+idx)->sort_tmp_val < hDestScoreDoc->sort_tmp_val) /* !!! */
    {
        return 1;
    }
    else if ((hSrcScoreDoc+idx)->sort_tmp_val > hDestScoreDoc->sort_tmp_val)
    {
        return -1;
    }
    else
    {
        return 0;
    }
     
}
int eggScoreDoc_cmp_orderbytmpval_double_asc(HEGGSCOREDOC hSrcScoreDoc, HEGGSCOREDOC hDestScoreDoc)
{
    if (*(double*)hSrcScoreDoc->orderBy > *(double*)hDestScoreDoc->orderBy)
    {
        return 1;
    }
    else if (*(double*)hSrcScoreDoc->orderBy < *(double*)hDestScoreDoc->orderBy)
    {
        return -1;
    }
    else if (hSrcScoreDoc->sort_tmp_val < hDestScoreDoc->sort_tmp_val) /* !!! */
    {
        return 1;
    }
    else if (hSrcScoreDoc->sort_tmp_val > hDestScoreDoc->sort_tmp_val)
    {
        return -1;
    }
    else
    {
        return 0;
    }
    
}
int eggScoreDoc_cmp_orderbytmpval_double_desc(HEGGSCOREDOC hSrcScoreDoc, HEGGSCOREDOC hDestScoreDoc)
{
    if (*(double*)hSrcScoreDoc->orderBy < *(double*)hDestScoreDoc->orderBy)
    {
        return 1;
    }
    else if (*(double*)hSrcScoreDoc->orderBy > *(double*)hDestScoreDoc->orderBy)
    {
        return -1;
    }
    else if (hSrcScoreDoc->sort_tmp_val < hDestScoreDoc->sort_tmp_val) /* !!! */
    {
        return 1;
    }
    else if (hSrcScoreDoc->sort_tmp_val > hDestScoreDoc->sort_tmp_val)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}
int eggScoreDoc_cmp_orderbytmpval2_double_asc(HEGGSCOREDOC hSrcScoreDoc, index_t idx, HEGGSCOREDOC hDestScoreDoc)
{
    if (*(double*)(hSrcScoreDoc+idx)->orderBy > *(double*)hDestScoreDoc->orderBy)
    {
        return 1;
    }
    else if (*(double*)(hSrcScoreDoc+idx)->orderBy < *(double*)hDestScoreDoc->orderBy)
    {
        return -1;
    }
    else if ((hSrcScoreDoc+idx)->sort_tmp_val < hDestScoreDoc->sort_tmp_val) /* !!! */
    {
        return 1;
    }
    else if ((hSrcScoreDoc+idx)->sort_tmp_val > hDestScoreDoc->sort_tmp_val)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}
int eggScoreDoc_cmp_orderbytmpval2_double_desc(HEGGSCOREDOC hSrcScoreDoc, index_t idx, HEGGSCOREDOC hDestScoreDoc)
{
    if (*(double*)(hSrcScoreDoc+idx)->orderBy < *(double*)hDestScoreDoc->orderBy)
    {
        return 1;
    }
    else if (*(double*)(hSrcScoreDoc+idx)->orderBy > *(double*)hDestScoreDoc->orderBy)
    {
        return -1;
    }
    else if ((hSrcScoreDoc+idx)->sort_tmp_val < hDestScoreDoc->sort_tmp_val) /* !!! */
    {
        return 1;
    }
    else if ((hSrcScoreDoc+idx)->sort_tmp_val > hDestScoreDoc->sort_tmp_val)
    {
        return -1;
    }
    else
    {
        return 0;
    }
}
