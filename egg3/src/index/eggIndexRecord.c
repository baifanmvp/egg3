#include "eggIndexRecord.h"


PRIVATE int eggIndexRecord_cmp_with_str (HEGGINDEXRECORD pSrcNode, HEGGINDEXRECORD pDestNode);

PRIVATE int eggIndexRecord_cmp_with_int32(HEGGINDEXRECORD pSrcRecord, HEGGINDEXRECORD pDestRecord);

PRIVATE int eggIndexRecord_cmp_with_int64(HEGGINDEXRECORD pSrcRecord, HEGGINDEXRECORD pDestRecord);

PRIVATE int eggIndexRecord_cmp_with_double(HEGGINDEXRECORD pSrcRecord, HEGGINDEXRECORD pDestRecord);

//static flag_t g_rd_md5 = EGG_TRUE;

HEGGINDEXRECORD eggIndexRecord_new(size16_t aSz, void* pKey, size32_t kSz, void* pVal, size32_t vSz)
{
    void* lp_rd_key = pKey;
    char md5_key[UTI_MD5_DIGEST_LENGTH];
    
    if((size32_t)aSz < sizeof(EGGINDEXRECORD) + kSz + vSz)
    {
//        kSz = MD5_DIGEST_LENGTH;
        eggUtiMD5(pKey, kSz, md5_key);
        kSz = UTI_MD5_DIGEST_LENGTH;
        lp_rd_key = md5_key;
    }
    
    if((size32_t)aSz < sizeof(EGGINDEXRECORD) + kSz + vSz)
    {
        return EGG_NULL;
    }
    
    HEGGINDEXRECORD lp_record = (HEGGINDEXRECORD)malloc(aSz);
    
    lp_record->childOff = 0;
    lp_record->hostOff = 0;
    
    EGGINDEXRECORD_KSIZE(lp_record) = (size16_t)kSz; 
    EGGINDEXRECORD_VSIZE(lp_record) = (size16_t)vSz;
    
    if(kSz && lp_rd_key)
        memcpy(EGGINDEXRECORD_KEY(lp_record), lp_rd_key, kSz);
    
    if(vSz && pVal)
        memcpy(EGGINDEXRECORD_VAL(lp_record), pVal, vSz);

    return lp_record;
}

EBOOL eggIndexRecord_delete(HEGGINDEXRECORD lpRecord)
{
    if(EGGINDEXRECORD_IS_INVALID(lpRecord))
    {
        return EGG_FALSE;
    }
    
    free(lpRecord);
    return EGG_TRUE;
}

RDCMP eggIndexNode_get_fncmp(type_t type)
{
    if(type & EGG_INDEX_STRING)
    {
        return eggIndexRecord_cmp_with_str;
    }
    else if(type & EGG_INDEX_INT32)
    {
        return eggIndexRecord_cmp_with_int32;
    }
    else if(type & EGG_INDEX_INT64)
    {
        return eggIndexRecord_cmp_with_int64;
    }
    else if(type & EGG_INDEX_DOUBLE)
    {
        return eggIndexRecord_cmp_with_double;
    }
    
    return EGG_NULL;
}

 int eggIndexRecord_cmp_with_id(HEGGINDEXRECORD pSrcRecord, HEGGINDEXRECORD pDestRecord)
{
    if(EGGINDEXRECORD_IS_INVALID(pSrcRecord))
    {
        return -2;
    }
    
    if(EGGINDEXRECORD_IS_INVALID(pDestRecord))
    {
        return -2;
    }
    HEGGRECORDDOCID p_src_id = (HEGGRECORDDOCID)EGGINDEXRECORD_VAL(pSrcRecord);
    
    HEGGRECORDDOCID p_dest_id = (HEGGRECORDDOCID)EGGINDEXRECORD_VAL(pDestRecord);

    if(p_src_id->did > p_dest_id->did > 0)
    {
        return -1;
    }
    else if(p_src_id->did < p_dest_id->did )
    {
        return 1;
    }
    else
    {
        return 0;
    }
}


PRIVATE int eggIndexRecord_cmp_with_str (HEGGINDEXRECORD pSrcRecord, HEGGINDEXRECORD pDestRecord)
{
    if(EGGINDEXRECORD_IS_INVALID(pSrcRecord))
    {
        return EGG_FALSE;
    }
    
    if(EGGINDEXRECORD_IS_INVALID(pDestRecord))
    {
        return EGG_FALSE;
    }
    
    char* lp_src_key = EGGINDEXRECORD_KEY(pSrcRecord);
    size16_t n_src_sz = EGGINDEXRECORD_KSIZE(pSrcRecord);
    
    char* lp_dest_key = EGGINDEXRECORD_KEY(pDestRecord);
    size16_t n_dest_sz = EGGINDEXRECORD_KSIZE(pDestRecord);

    size16_t n_cmp_sz = (n_src_sz < n_dest_sz? n_src_sz : n_dest_sz);
    /* printf("lp_src_key : %s\n", lp_src_key); */
    /* printf("lp_dest_key : %s\n", lp_dest_key); */
    int n_cmp_ret = memcmp(lp_src_key, lp_dest_key, n_cmp_sz);
    if(!n_cmp_ret)
    {
        n_cmp_ret = (int)(n_src_sz - n_dest_sz);
    }
    return n_cmp_ret;
    
}
PRIVATE int eggIndexRecord_cmp_with_int32(HEGGINDEXRECORD pSrcRecord, HEGGINDEXRECORD pDestRecord)
{
    int n_src_key = *((int*)EGGINDEXRECORD_KEY(pSrcRecord));
    
    int n_dest_key = *((int*)EGGINDEXRECORD_KEY(pDestRecord));

    return n_src_key - n_dest_key;

}
PRIVATE int eggIndexRecord_cmp_with_int64(HEGGINDEXRECORD pSrcRecord, HEGGINDEXRECORD pDestRecord)
{
    if(EGGINDEXRECORD_IS_INVALID(pSrcRecord))
    {
        return -2;
    }
    
    if(EGGINDEXRECORD_IS_INVALID(pDestRecord))
    {
        return -2;
    }
    
    long long n_src_key = *((long long*)EGGINDEXRECORD_KEY(pSrcRecord));
    
    long long n_dest_key = *((long long*)EGGINDEXRECORD_KEY(pDestRecord));

    if(n_src_key - n_dest_key > 0)
    {
        return 1;
    }
    else if(n_src_key - n_dest_key < 0)
    {
        return -1;
    }
    else
    {
        return 0;
    }

}
PRIVATE int eggIndexRecord_cmp_with_double(HEGGINDEXRECORD pSrcRecord, HEGGINDEXRECORD pDestRecord)
{
    if(EGGINDEXRECORD_IS_INVALID(pSrcRecord))
    {
        return -2;
    }
    
    if(EGGINDEXRECORD_IS_INVALID(pDestRecord))
    {
        return -2;
    }
    
    double n_src_key = *((double*)EGGINDEXRECORD_KEY(pSrcRecord));
    
    double n_dest_key = *((double*)EGGINDEXRECORD_KEY(pDestRecord));

    if(n_src_key - n_dest_key > 0)
    {
        return 1;
    }
    else if(n_src_key - n_dest_key < 0)
    {
        return -1;
    }
    else
    {
        return 0;
    }

}

