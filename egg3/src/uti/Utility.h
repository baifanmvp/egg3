#ifndef _EGG_UTILITY_H_
#define _EGG_UTILITY_H_


#include "../EggDef.h"
#include "./VariantInteger.h"
#include <bzlib.h>

E_BEGIN_DECLS

typedef struct eggMemChunk EGGMEMCHUNK;
typedef struct eggMemChunk* HEGGMEMCHUNK;

struct eggMemChunk
{
    size32_t aSz;
    size32_t eSz;
    char* data;
};
#define EGG_MEMCHUNK_SIZE 4096
////////////////////

typedef struct utiVector UTIVECTOR;

struct utiVector
{
    size32_t atomSize;
    count_t aCount;
    count_t eCount;
    epointer data;
};

typedef struct tagVintData
{
    size32_t len;
    i8* vData;
}VINTDATA;

/// <summary>
/// utility method: format special size to minial multiple of 1k.
/// </summary>
/// <param name="nSize">target size</param>
/// <return>return new size</return>
extern    size32_t Uti_binary_power(size32_t nSize);
    
/// <summary>
/// get a mapping cell begin addres with special offset in it
/// </summary>
/// <param name="nOffset">target offset</param>
/// <param name="nStartOff">mapping area start offset</param>
/// <param name="nClusterSize">each mapping size</param>
/// <returns>return offset</returns>
extern    offset64_t Uti_base_offset(offset64_t nOffset, offset64_t nStartOff, size32_t nClusterSize);
    
extern EBOOL Uti_bz2_compress(epointer lp_in_buf, size32_t n_in_size, epointer* lplp_out_buf, size32_t* lp_out_size);

extern EBOOL Uti_bz2_decompress(epointer lp_in_buf, size32_t n_in_size, epointer* lplp_out_buf, size32_t* lp_out_size);


EBOOL Uti_change_relavalue_32(size32_t org, size32_t* lp_int_32, size32_t len);


EBOOL Uti_change_absovalue_32(size32_t org, size32_t* lp_int_32, size32_t len, int flag);


EBOOL Uti_vtoi_32(VINTDATA* lp_vint_data, size32_t** lplp_int_32, size32_t* lp_len) ;

EBOOL Uti_itov_32(size32_t* lp_int_32, size32_t len, VINTDATA* lp_vint_data);

EBOOL Uti_change_relavalue_64(size64_t org, size64_t* lp_int_64, size32_t len);

EBOOL Uti_change_absovalue_64(size64_t org, size64_t* lp_int_64, size32_t len, int flag);

EBOOL Uti_vtoi_64(VINTDATA* lp_vint_data, size64_t** lplp_int_64, size32_t* lp_len);

EBOOL Uti_itov_64(size64_t* lp_int_64, size32_t len, VINTDATA* lp_vint_data);

typedef int (*FNBINARYCMP)(epointer lp_dest_data, index_t n_dest_idx, epointer val);


EBOOL Uti_binary_lookup(epointer lp_dest_data, index_t n_max_idx, epointer val, index_t* lp_dest_idx, FNBINARYCMP fnCmp);

/* find first value >= key, assume ascend */
epointer Uti_binary_searchleft(epointer key,
                               epointer array, size32_t nItems, size32_t itemSz,
                               int (*cmp)(epointer key, epointer p));
/* find first value > key, assume ascend */
epointer Uti_binary_searchright(epointer key,
                                epointer array, size32_t nItems, size32_t itemSz,
                                int (*cmp)(epointer key, epointer p));


//sort

typedef int (*SORTCMP)(epointer pSrc, epointer pDest);

PUBLIC void  Uti_insort (echar* carray, index_t len, size32_t unitSz, SORTCMP fnCmp);

PUBLIC void  Uti_sedgesort (void* array, int len, size32_t unitSz, SORTCMP fnCmp);

//memory chunk
#define Uti_memChunk_data(v) ((v)->data)

#define Uti_memChunk_eSize(v) ((v)->eSz)

HEGGMEMCHUNK Uti_memchunk_create();

EBOOL Uti_memchunk_push(HEGGMEMCHUNK lp_mChunk, epointer data, size32_t sz);

EBOOL Uti_memchunk_destroy(HEGGMEMCHUNK lp_mChunk, flag_t flag);

//vector  

#define Uti_vector_data(v) ((v)->data)

#define Uti_vector_count(v) ((v)->eCount)

UTIVECTOR* Uti_vector_create(size32_t atomSize);

EBOOL Uti_vector_push(UTIVECTOR* lp_uti_vector, epointer data, count_t count);

EBOOL Uti_vector_transpose(UTIVECTOR* lp_uti_vector);

EBOOL Uti_vector_destroy(UTIVECTOR* lp_uti_vector, flag_t flag);

EBOOL Uti_filter_repeat(char* array, count_t* pCnt, size32_t unitSz, SORTCMP fnCmp);

EBOOL Uti_ip_valid(char* ip);

E_END_DECLS



#endif // _EGG_UTILITY_H_
