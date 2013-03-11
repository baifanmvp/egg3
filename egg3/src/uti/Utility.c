#include "Utility.h"
#include "../log/eggPrtLog.h"

PUBLIC size32_t Uti_binary_power(size32_t nSize)
{
    if ((nSize & 0x03FF) != 0x00)
    {
        nSize = (size32_t) ((((size32_t)(-1))<<10)) & nSize;
        nSize += 1024;
    }
    
    return nSize;
}

PUBLIC offset64_t Uti_base_offset(offset64_t nOffset, offset64_t nStartOff, size32_t nClusterSize)
{
    if (nOffset < nStartOff)
    {
        return 0;
    }
    
    offset64_t nRet = nOffset - nStartOff;

    nRet = (~((offset64_t)(nClusterSize - 1))) & nRet;

    return nRet + nStartOff;
}

PUBLIC EBOOL Uti_bz2_compress(epointer lp_in_buf, size32_t n_in_size, epointer* lplp_out_buf, size32_t* lp_out_size)
{
    if(!lp_in_buf || !lplp_out_buf)
    {
        return EGG_FALSE;
    }
    size32_t n_out_size = n_in_size * 8;
    size32_t n_origin_size = n_in_size;
    
    *lplp_out_buf = malloc(n_out_size + sizeof(n_origin_size));
    *(size32_t*)(*lplp_out_buf) = n_origin_size;
    
    int ret = BZ2_bzBuffToBuffCompress((size32_t*)(*lplp_out_buf) + 1,
                                         (unsigned int*)&n_out_size,
                                         lp_in_buf,
                                         n_in_size,
                                         1,
                                         0,
                                         30);
    if(ret != BZ_OK)
    {
        if(BZ_OUTBUFF_FULL  == ret)
        {
            //printf("BZ_OUTBUFF_FULL\n" );
            eggPrtLog_info("Utility", "BZ_OUTBUFF_FULL\n" );
        }
        else
        {
            //printf("compress err!\n");
            eggPrtLog_error("Utility", "compress err!\n");
        }
        free(*lplp_out_buf);
	*lplp_out_buf = EGG_NULL;
        return EGG_FALSE;
    }
    else
    {
        *lp_out_size = n_out_size + sizeof(size32_t);
    }
    
    return EGG_TRUE;
}

PUBLIC EBOOL Uti_bz2_decompress(epointer lp_in_buf, size32_t n_in_size, epointer* lplp_out_buf, size32_t* lp_out_size)
{
    if(!lp_in_buf || !lplp_out_buf)
    {
        return EGG_FALSE;
    }
    
    size32_t n_out_size = *((size32_t*)lp_in_buf);
    
    *lplp_out_buf = malloc(8 * n_out_size);

    int ret = BZ2_bzBuffToBuffDecompress(*lplp_out_buf,
                                         (unsigned int*)&n_out_size,
                                         ((size32_t*)lp_in_buf) + 1,
                                         n_in_size - sizeof(size32_t),
                                         0,
                                         0);
    if(ret != BZ_OK)
    {
        free(*lplp_out_buf);
        return EGG_FALSE;
    }
    else
    {
        *lp_out_size = n_out_size;
    }
    
    return EGG_TRUE;
}
#define SORT_CUTOFF 15
#define SORT_SWAP(pSrc , pDest, pSwap, sz)               \
    do                                                   \
    {                                                    \
        if((long)pSrc != (long)pDest)                    \
        {                                                \
            memcpy(pSwap, pSrc, sz);                     \
            memcpy(pSrc,  pDest, sz);                    \
            memcpy(pDest, pSwap, sz);                    \
        }                                                \
    }while(EGG_FALSE);

#define SORT_COPY(pDest, pSrc, sz)                                      \
    memcpy(pDest, pSrc, sz);

PUBLIC void  Uti_insort (echar* carray, index_t len, size32_t unitSz, SORTCMP fnCmp)
{
	index_t	i, j;
    echar* pTmp = (echar*)malloc(unitSz);

	for (i = 1; i < len; i++) {
		/* invariant:  array[0..i-1] is sorted */
		j = i;
		/* customization bug: SWAP is not used here */
        SORT_COPY(pTmp, carray + j * unitSz, unitSz);
            
		while (j > 0 && fnCmp(carray + (j-1) * unitSz, pTmp) < 0)
        {
            SORT_COPY(carray + j * unitSz, carray + (j-1) * unitSz, unitSz);
        
			j--;
		}
        SORT_COPY(carray + j * unitSz, pTmp, unitSz);
	}
    free(pTmp);
    return ;
}


PRIVATE EBOOL  uti_quickersort (echar* carray, index_t lower, index_t upper, size32_t unitSz, SORTCMP fnCmp)
{
    index_t i, j;
    echar* pSwap = (echar*)malloc(unitSz);
    echar* pivot = (echar*)malloc(unitSz);
    
    if (upper - lower > SORT_CUTOFF)
    {
        SORT_SWAP(carray + lower * unitSz, carray + (upper+lower) / 2 * unitSz, pSwap, unitSz);
        i = lower;
        j = upper + 1;
        SORT_COPY(pivot, carray + lower * unitSz, unitSz);
        while (1)
        {
	    /*
	     * ------------------------- NOTE --------------------------
	     * ignoring BIG NOTE above may lead to an infinite loop here
	     * ---------------------------------------------------------
	     */
            do i++; while (i <= upper && fnCmp(carray + i * unitSz, pivot) > 0 );
            do j--; while (j >= 0 && fnCmp(carray + j * unitSz, pivot) < 0);
            if (j < i) break;
            
            SORT_SWAP(carray + i * unitSz, carray + j * unitSz, pSwap, unitSz);
        }
        SORT_SWAP(carray + lower * unitSz, carray + j * unitSz, pSwap, unitSz);
        uti_quickersort (carray, lower, j - 1,  unitSz, fnCmp);
        uti_quickersort (carray, i, upper,  unitSz, fnCmp);
    }
    
    free(pSwap);
    free(pivot);
    
    return EGG_TRUE;
}

PUBLIC void  Uti_sedgesort (void* array, int cnt, size32_t unitSz, SORTCMP fnCmp)
{
    /*
     * ------------------------- NOTE --------------------------
     * ignoring BIG NOTE above may lead to an infinite loop here
     * ---------------------------------------------------------
     */
    if(cnt)
    {
    uti_quickersort (array, 0, cnt - 1, unitSz, fnCmp);
    
    Uti_insort (array, cnt, unitSz, fnCmp);
    }
    return ;
}


void VintWrite(i8 v, VINTDATA* lp_vint_data)
{
    lp_vint_data->vData[lp_vint_data->len++] = v;
    return ;
}

i8 VintRead(VINTDATA* lp_vint_data)
{
    return lp_vint_data->vData[lp_vint_data->len++];
}

EBOOL Uti_change_relavalue_32(size32_t org, size32_t* lp_int_32, size32_t len)
{
    size32_t iter = org;
    int i = 0;
    while(i != len)
    {
        size32_t tmp = lp_int_32[i];
        lp_int_32[i] = (iter > lp_int_32[i]) ? (iter - lp_int_32[i]) : (lp_int_32[i] - iter);
        iter = tmp;
        i++;
    }
}

EBOOL Uti_change_absovalue_32(size32_t org, size32_t* lp_int_32, size32_t len, int flag)
{
    int i = 0;
    size32_t iter = org;
    while(i != len)
    {
        lp_int_32[i] = iter + lp_int_32[i]*flag;
        iter = lp_int_32[i];
        i++;
    }
    
    return EGG_TRUE;
}


EBOOL Uti_vtoi_32(VINTDATA* lp_vint_data, size32_t** lplp_int_32, size32_t* lp_len)
{
    size32_t n_max_len = lp_vint_data->len;
    lp_vint_data->len = 0;
    int n_total_count = 4;
    *lplp_int_32 = (size32_t*)malloc(sizeof(size32_t) * n_total_count);
    *lp_len = 0;
    
    while(lp_vint_data->len != n_max_len)
    {
        (*lplp_int_32)[(*lp_len) ++] = core_read_vint(VintRead, lp_vint_data);
        if(*lp_len >= n_total_count)
        {
            n_total_count += n_total_count;
            *lplp_int_32 = (size32_t*)realloc(*lplp_int_32, sizeof(size32_t) * n_total_count);
        }
    }
    
    return EGG_TRUE;
}


EBOOL Uti_itov_32(size32_t* lp_int_32, size32_t len, VINTDATA* lp_vint_data)
{
    lp_vint_data->len = 0;
    lp_vint_data->vData = (i8*)malloc(sizeof(size32_t) * len);
    int i = 0;
    while(i != len)
    {
        core_write_vint(lp_int_32[i++], VintWrite, lp_vint_data);
    }
    
    return EGG_TRUE;
}




EBOOL Uti_change_relavalue_64(size64_t org, size64_t* lp_int_64, size32_t len)
{
    size64_t iter = org;
    int i = 0;
    while(i != len)
    {
        size64_t tmp = lp_int_64[i];
        lp_int_64[i] = (iter > lp_int_64[i]) ? (iter - lp_int_64[i]) : (lp_int_64[i] - iter);
        iter = tmp;
        i++;
    }
}

EBOOL Uti_change_absovalue_64(size64_t org, size64_t* lp_int_64, size32_t len, int flag)
{
    int i = 0;
    size64_t iter = org;
    while(i != len)
    {
        lp_int_64[i] = iter + lp_int_64[i]*flag;
        iter = lp_int_64[i];
        i++;
    }
    
    return EGG_TRUE;
}


EBOOL Uti_vtoi_64(VINTDATA* lp_vint_data, size64_t** lplp_int_64, size32_t* lp_len)
{
    size32_t n_max_len = lp_vint_data->len;
    lp_vint_data->len = 0;
    int n_total_count = 4;
    *lplp_int_64 = (size64_t*)malloc(sizeof(size64_t) * n_total_count);
    *lp_len = 0;
    
    while(lp_vint_data->len != n_max_len)
    {
        (*lplp_int_64)[(*lp_len) ++] = core_read_vint(VintRead, lp_vint_data);
        if(*lp_len >= n_total_count)
        {
            n_total_count += n_total_count;
            *lplp_int_64 = (size64_t*)realloc(*lplp_int_64, sizeof(size64_t) * n_total_count);
        }
    }
    
    return EGG_TRUE;
}


EBOOL Uti_itov_64(size64_t* lp_int_64, size32_t len, VINTDATA* lp_vint_data)
{
    lp_vint_data->len = 0;
    lp_vint_data->vData = (i8*)malloc(sizeof(size64_t) * len);
    int i = 0;
    while(i != len)
    {
        core_write_vint(lp_int_64[i++], VintWrite, lp_vint_data);
    }
    
    return EGG_TRUE;
}

EBOOL Uti_binary_lookup(epointer lp_dest_data, index_t n_max_idx, epointer val, index_t* lp_dest_idx, FNBINARYCMP fnCmp)
{
    if (!lp_dest_data || !val)
    {
        return EGG_FALSE;
    }
    if(n_max_idx < 0)
    {
        return EGG_FALSE;
    }
    
    *lp_dest_idx = -1;
    static int cnt = 0;
    index_t n_start_idx = 0;
    index_t n_end_idx = n_max_idx;
    
    while(n_start_idx <= n_end_idx)
    {
        *lp_dest_idx = (n_start_idx + n_end_idx) / 2;
        
        int ret_cmp = (int)fnCmp(lp_dest_data, *lp_dest_idx, val);
        
        if(ret_cmp == 0)
        {
//            printf("look up %d\n", cnt++);
            return EGG_TRUE;
        }
        else if(ret_cmp > 0)
        {
            n_start_idx = *lp_dest_idx + 1;
            
        }
        else
        {
            n_end_idx = *lp_dest_idx - 1;
        }
    }
//    printf("look up not \n");
    return EGG_FALSE;
    
}

/* find first value >= key, assume ascend */
epointer Uti_binary_searchleft(epointer key,
                               epointer array, size32_t nItems, size32_t itemSz,
                               int (*cmp)(epointer key, epointer p))
{
    index_t n_start_idx = 0;
    index_t n_end_idx = nItems;
    index_t n_mid_idx;
    while (n_start_idx < n_end_idx)
    {
        n_mid_idx = (n_start_idx + n_end_idx) / 2;
        int c;
        if ((c = cmp(key, array + n_mid_idx * itemSz)) <= 0)
        {
            n_end_idx = n_mid_idx;
        }
        else
        {
            n_start_idx = n_mid_idx + 1;
        }
    }
    return array + n_start_idx * itemSz;
}

/* find first value > key, assume ascend */
epointer Uti_binary_searchright(epointer key,
                                epointer array, size32_t nItems, size32_t itemSz,
                                int (*cmp)(epointer key, epointer p))
{
    index_t n_start_idx = 0;
    index_t n_end_idx = nItems;
    index_t n_mid_idx;
    while (n_start_idx < n_end_idx)
    {
        n_mid_idx = (n_start_idx + n_end_idx) / 2;
        int c;
        if ((c = cmp(key, array + n_mid_idx * itemSz)) < 0)
        {
            n_end_idx = n_mid_idx;
        }
        else
        {
            n_start_idx = n_mid_idx + 1;
        }
    }
    return array + n_start_idx * itemSz;
}

HEGGMEMCHUNK Uti_memchunk_create()
{
    HEGGMEMCHUNK lp_mChunk = (HEGGMEMCHUNK)malloc(sizeof(EGGMEMCHUNK));
    memset(lp_mChunk, 0, sizeof(EGGMEMCHUNK));
    return lp_mChunk;
}

EBOOL Uti_memchunk_push(HEGGMEMCHUNK lp_mChunk, epointer data, size32_t sz)
{
    if(!lp_mChunk)
    {
        return EGG_FALSE;
    }
    
    while(lp_mChunk->aSz - lp_mChunk->eSz < sz)
    {
        lp_mChunk->aSz = lp_mChunk->aSz ? lp_mChunk->aSz * 2 : EGG_MEMCHUNK_SIZE;
        lp_mChunk->data = realloc(lp_mChunk->data, lp_mChunk->aSz);
    }
    
    memcpy(lp_mChunk->data + lp_mChunk->eSz, data, sz);
    lp_mChunk->eSz += sz;
    
    return EGG_TRUE;
}

EBOOL Uti_memchunk_destroy(HEGGMEMCHUNK lp_mChunk, flag_t flag)
{
    if(lp_mChunk)
    {
        if(flag && lp_mChunk->data)
        {
            free(lp_mChunk->data);
        }
        free(lp_mChunk);
        
        return EGG_TRUE;
    }
    return EGG_FALSE;
}

UTIVECTOR* Uti_vector_create(size32_t atomSize)
{
    UTIVECTOR* lp_uti_vector = (UTIVECTOR*)malloc(sizeof(UTIVECTOR));
    memset(lp_uti_vector, 0, sizeof(UTIVECTOR));
    lp_uti_vector->atomSize = atomSize;
    
    return lp_uti_vector;
}


EBOOL Uti_vector_push(UTIVECTOR* lp_uti_vector, epointer data, count_t count)
{
    if(!lp_uti_vector)
    {
        return EGG_FALSE;
    }
    if(!count)
    {
        return EGG_FALSE;
    }
    flag_t flag = (count + lp_uti_vector->eCount) > lp_uti_vector->aCount ?EGG_TRUE:EGG_FALSE;
    while((count + lp_uti_vector->eCount) > lp_uti_vector->aCount)
    {
        if(!lp_uti_vector->aCount)
        {
            lp_uti_vector->aCount = 1;
        }
        else
        {
            lp_uti_vector->aCount += lp_uti_vector->aCount;
        }
    }
    if(flag)
    {
        char* lp = (char*)malloc(lp_uti_vector->atomSize * lp_uti_vector->aCount);
        memcpy(lp, lp_uti_vector->data, lp_uti_vector->atomSize * lp_uti_vector->eCount);
        free(lp_uti_vector->data);
        lp_uti_vector->data = lp;
//        lp_uti_vector->data = realloc(lp_uti_vector->data, lp_uti_vector->atomSize * lp_uti_vector->aCount);
        
    }
    
    if(!lp_uti_vector->data)
    {
        perror("realloc");
    }
    
    memcpy((char*)lp_uti_vector->data + lp_uti_vector->atomSize * lp_uti_vector->eCount, data, lp_uti_vector->atomSize * count);
    lp_uti_vector->eCount += count;
    
    return EGG_TRUE;
}

EBOOL Uti_vector_transpose(UTIVECTOR* lp_uti_vector)
{
    if(!lp_uti_vector)
    {
        return EGG_FALSE;
    }
    count_t count = lp_uti_vector->eCount / 2;
    index_t idx = 0;

    char* p_dataunit = (char*)malloc(lp_uti_vector->atomSize);
    char* p_data = (char*)(lp_uti_vector->data);
    
    while(idx < count)
    {
        memcpy(p_dataunit, lp_uti_vector->data + idx * lp_uti_vector->atomSize, lp_uti_vector->atomSize);
        
        memcpy(lp_uti_vector->data + idx * lp_uti_vector->atomSize,
               lp_uti_vector->data + (lp_uti_vector->eCount - 1 - idx) * lp_uti_vector->atomSize,
               lp_uti_vector->atomSize);
        
        memcpy(lp_uti_vector->data + (lp_uti_vector->eCount - 1 - idx) * lp_uti_vector->atomSize,
               p_dataunit,
               lp_uti_vector->atomSize);
        idx++;
    }
    
    free(p_dataunit);
    
}

EBOOL Uti_vector_destroy(UTIVECTOR* lp_uti_vector, flag_t flag)
{
    if(lp_uti_vector)
    {
        if(flag && lp_uti_vector->data)
        {
            free(lp_uti_vector->data);
        }
        free(lp_uti_vector);
        
        return EGG_TRUE;
    }
    return EGG_FALSE;
}

EBOOL Uti_filter_repeat(char* array, count_t* pCnt, size32_t unitSz, SORTCMP fnCmp)
{
    count_t n_src_cnt = *pCnt;
    count_t n_dest_cnt = *pCnt;

    index_t i_src = 1;
    index_t i_dest = 1;
    char* reference = array;
    while(i_src < n_src_cnt)
    {
        if(fnCmp(array + i_src * unitSz, array + (i_src - 1) * unitSz) )
        {
            if(i_src != i_dest)
            {
                memcpy(array + i_dest * unitSz, array + i_src * unitSz, unitSz);
            }
            i_dest++;
        }
        
        i_src++;
    }
    
    *pCnt = i_dest;
    
    return EGG_TRUE;
}

EBOOL Uti_ip_valid(char* ip)
{
    unsigned int i1 = -1;
    unsigned int i2 = -1;
    unsigned int i3 = -1;
    unsigned int i4 = -1;
    char tail[100] = {0};
    sscanf(ip, "%d.%d.%d.%d%s", &i1, &i2, &i3, &i4, tail);
    if(*tail != 0)
    {
        return EGG_FALSE;
    }
    if( i1 > 255 ||
        i2 > 255 ||
        i3 > 255 ||
        i4 > 255 )
    {
        return EGG_FALSE;
    }
    else
    {
        return EGG_TRUE;
    }
}

#if 0
int sortcmp(epointer pSrc, epointer pDest)
{
    return - *(int*)(pSrc) + *(int*)(pDest);
}
int sortcmpA(epointer pSrc, epointer pDest)
{
    return *(int*)(pSrc) - *(int*)(pDest);
}
int main()
{
    //int array[] = {1, 3, 3, 4, 5, 5, 6, 6, 6, 7, 8, 9, 9, 9, 10};
    int* array = malloc(sizeof(int)* 100);
    count_t cnt = 100;
    int i = 0;
    
    for(i = 0; i< cnt; i++)
    {
        array[i] = rand()%20;
        printf("array %d\n", array[i]);
    }
    Uti_sedgesort (array, 100, sizeof(int), sortcmp);

    /* Uti_filter_repeat(array, &cnt, sizeof(int), sortcmp); */
    for(i = 0; i< cnt; i++)
    {
        printf("%d ", array[i]);
    }
    printf("\n");
    /* for (;;) */
    /* { */
    /*     int key; */
    /*     scanf("%d", &key); */
    /*     int *p = Uti_binary_searchright(&key, array, cnt, sizeof(int), sortcmpA); */
    /*     printf("-----%d\n", (p - array)); */
    /* } */
    return 0;
}
#endif
