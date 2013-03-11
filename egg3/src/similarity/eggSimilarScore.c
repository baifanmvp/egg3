#include "eggSimilarScore.h"

PRIVATE double eggCalc(HDOCOFFSETTABLE peggDoc, int len);

PRIVATE EBOOL egg_word_continuation(HDOCOFFSETTABLE peggDoc, int len, int wordCnt);

PRIVATE int egg_word_different(HDOCOFFSETTABLE peggDoc, int idx, int wordCnt);

PRIVATE UTIVECTOR* similar_get_doc_offset(HEGGIDNODE hIdNodes, count_t nIdCnt);

PUBLIC EBOOL similar_score_document(HEGGIDNODE hIdNodes, count_t nIdCnt, HEGGSCOREDOC hScoreDoc)
{
    if(!nIdCnt || !hIdNodes || !hScoreDoc)
    {
        return EGG_FALSE;
    }

    //    printf("n_cnt : %d\n", nIdCnt);
    Uti_sedgesort (hIdNodes, nIdCnt, sizeof(EGGIDNODE), eggIdNode_cmp_mask);
    
    index_t n_ids_idx = 0;
    hScoreDoc->score = 0;
    while(n_ids_idx != nIdCnt)
    {
        count_t n_equalMask_cnt = 0;
        
        EGGIDNODE_EQUALMASK_CNT(hIdNodes, n_ids_idx, n_equalMask_cnt, nIdCnt - n_ids_idx);
        
	//	printf("n_equalMask_cnt : %d\n", n_equalMask_cnt);
	 hScoreDoc->score += similar_score_with_field(hIdNodes + n_ids_idx, n_equalMask_cnt);

        n_ids_idx += n_equalMask_cnt;
    }

    return EGG_TRUE;

}



//  从 小 到 大 
int eggScore_cmpkeyoff(DOCOFFSETTABLE* offsettable1, DOCOFFSETTABLE* offsettable2)
{
  if(offsettable1->off > offsettable2->off)
    {
      return -1;
    }
  else if(offsettable1->off < offsettable2->off)
    {
      return 1;
    }
  else
    {
      return 0;
    }
}

PUBLIC score_t similar_score_with_field(HEGGIDNODE hIdNodes, count_t nIdCnt)
{
    if(!hIdNodes || !nIdCnt)
    {
        return 0;
    }
    //    printf("similar_ %d\n", nIdCnt);
    UTIVECTOR* lp_off16_vector = similar_get_doc_offset(hIdNodes, nIdCnt);

    HDOCOFFSETTABLE h_doc_offset_table = Uti_vector_data(lp_off16_vector);
    count_t n_cnt_off =  Uti_vector_count(lp_off16_vector);
    index_t n_idx_off = 0;
    
    Uti_sedgesort (h_doc_offset_table, n_cnt_off, sizeof(DOCOFFSETTABLE), eggScore_cmpkeyoff);
    int i = 0;

    score_t ret_score = eggCalc(h_doc_offset_table, n_cnt_off);
    
    Uti_vector_destroy(lp_off16_vector, EGG_TRUE);
    

    return  ret_score;
}

PUBLIC EBOOL similar_word_with_continuation(HEGGIDNODE hIdNodes, count_t nIdCnt)
{
    if(!hIdNodes || !nIdCnt)
    {
        return EGG_FALSE;
    }
    //    printf("similar_ %d\n", nIdCnt);
    UTIVECTOR* lp_off16_vector = similar_get_doc_offset(hIdNodes, nIdCnt);

    HDOCOFFSETTABLE h_doc_offset_table = Uti_vector_data(lp_off16_vector);
    count_t n_cnt_off =  Uti_vector_count(lp_off16_vector);
    index_t n_idx_off = 0;
    
    Uti_sedgesort (h_doc_offset_table, n_cnt_off, sizeof(DOCOFFSETTABLE), eggScore_cmpkeyoff);
    int i = 0;
    
    EBOOL ret = egg_word_continuation(h_doc_offset_table, n_cnt_off, nIdCnt);
    
    Uti_vector_destroy(lp_off16_vector, EGG_TRUE);
    

    return  ret;
}




#define SMALLDATA 0.01;

PRIVATE double eggCalc(HDOCOFFSETTABLE peggDoc, int len)
{
    int num = len;
    if ( len == 1 )
    {
        return 1.0;
    }
	//double ratio = peggDoc[0].off;
	int maxlen = peggDoc[len-1].off > 200 ? peggDoc[len-1].off : 200 ;
	double sum = 0.0;
	int i = 0;
	for ( ; i < num - 1 ;  )
	{
		double item = 1.0;
		DOCOFFSETTABLE kii = peggDoc[i];
		int li = kii.off;
		int lj  = 0;
		DOCOFFSETTABLE kij;
		int j = i + 1;
		kij = peggDoc[j];
		lj = kij.off;
		while( lj == li + 1 && j < num)
		{			
			int flag = 1;
			int k = i;
			for ( ; k < j ; k++ )
			{
				if ( peggDoc[j].echKey == peggDoc[k].echKey )
				{
					flag = 0;
				}
			}
			if ( !flag || ( j -i ) > 30 )
			{
				break;
			}
			item *= SMALLDATA;			
            if (j == num - 1)
            	break;
			j++;
            if ( ( j -i ) > 30 )
            {
                j--;
                break;
            }
			li = lj;
			kij = peggDoc[j];
			lj = kij.off;
		}
		if ( j - i > 1 )
		{
			double pw = 0.0;
			int k = i;
			for( ; k < j; k++)
			{
				pw += peggDoc[k].off;
			}
			pw /= k;
			pw = (2 - pw / maxlen) ; //最多扩大2倍，线性反比
			item = pw/item;			
		}
		else if( j - i == 1 )
		{

			double pw  = (li + lj) / 2;
			pw =  (2 - pw / maxlen);
			
			double pw1 = (peggDoc[j].off - peggDoc[i].off);
			
			if ( peggDoc[j].off == 65535 ) 
			{
				pw1 = 0.0;
			}
			else if ( peggDoc[j].echKey != peggDoc[i].echKey )
			{
				pw1 *= 2.0;
			}
			item = pw1 + pw;		
		}
		//cout << " item = " << item << endl;
		sum += item;
		i = j;
	}
//	printf("sum : %f\n", sum);
	/* if(sum > 0.08 && sum < 0.09) */
	/* { */
	/* 	printf("sum : %f\n", sum); */
	/* } */
	return sum;
}


PRIVATE EBOOL egg_word_continuation(HDOCOFFSETTABLE peggDoc, int len, int wordCnt)
{
    int n_idx = 0;
    
    while(n_idx != len)
    {
        int i = n_idx;
        
        if( (n_idx + wordCnt)  > len)
        {
            return EGG_FALSE;
        }
        
        while(++i != (wordCnt + n_idx))
        {
            if((peggDoc[i].off - peggDoc[i - 1].off) != 1)
            {
                break;
            }
        }

        if(i == (wordCnt + n_idx))
        {
            // asert egg_word_diff

            n_idx = egg_word_different(peggDoc, n_idx, wordCnt);
            
            if(n_idx == -1)
            {
                return  EGG_TRUE;
            }
            else
            {
                n_idx++;
            }
            
        }
        else
        {
            n_idx = i;
        }
    }
    
    return  EGG_TRUE;

}




PRIVATE int egg_word_different(HDOCOFFSETTABLE peggDoc, int idx, int wordCnt)
{
    int max_pos = wordCnt + idx;
    while(idx != max_pos)
    {
        int n_idx_tmp = idx;
        while(++n_idx_tmp != max_pos)
        {
            if(peggDoc[idx].echKey == peggDoc[n_idx_tmp].echKey)
            {
                //false find same key ,return 
                return idx;
            }
        }
        idx++;
    }
    
    //success! return -1
    return -1;

}

PRIVATE UTIVECTOR* similar_get_doc_offset(HEGGIDNODE hIdNodes, count_t nIdCnt)
{
   if(!hIdNodes || !nIdCnt)
    {
        return 0;
    }
    //    printf("similar_ %d\n", nIdCnt);
    count_t n_off_max = hIdNodes[0].keyCnt;
    index_t n_ids_idx = 0;
    UTIVECTOR* lp_off16_vector = Uti_vector_create(sizeof(DOCOFFSETTABLE));

    while(n_ids_idx != nIdCnt)
    {
        size16_t* lp_off = (size16_t*)EGGIDNODE_POS(hIdNodes + n_ids_idx);
        index_t n_tmp_idx = 0;
        while(n_tmp_idx != EGG_POS_COUNT && lp_off[n_tmp_idx])
          {
          //            h_doc_offset_table[ lp_off[n_tmp_idx] ].echKey = (echar*)(hIdNodes + n_ids_idx);
            DOCOFFSETTABLE st_doc_offset;
            st_doc_offset.echKey = (echar*)(hIdNodes + n_ids_idx);
            st_doc_offset.off = lp_off[n_tmp_idx];

            Uti_vector_push(lp_off16_vector, &st_doc_offset, 1);
            n_tmp_idx++;
        }

        n_ids_idx++;
    }

    return lp_off16_vector;
}
