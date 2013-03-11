#include <stdio.h>
#include "egg3/index/eggIdView.h"
#include "egg3/index/eggIndexView.h"
#include "egg3/index/eggFieldView.h"
#include "egg3/Egg3.h"


typedef struct _eggKeyTip
{
    HEGGFIELDVIEW pfield;
    HEGGINDEXVIEW pindex;
    HEGGIDVIEW pids;
    
    fdid_t fdid ;
    struct eggIndexInfo* pindexinfo;
    int nkeycnt;
    offset64_t niteroff ;
    index_t nindexiter;
HEGGINDEXNODEVIEW pnodeview;
    
}eggKeyTip;

eggKeyTip* eggKeyTip_new(char* path)
{
    char fddPath[1024];
    char idxPath[1024];
    char iddPath[1024];
    
    sprintf(idxPath, "%s/egg.idx", path);
    sprintf(fddPath, "%s/egg.fdd", path);
    sprintf(iddPath, "%s/egg.idd", path);

    HEGGFILE lp_egg_field = EggFile_open(fddPath);
    HEGGFILE lp_egg_idx = EggFile_open(idxPath);
    HEGGFILE lp_egg_idd = EggFile_open(iddPath);

    eggKeyTip *lp_tip = (eggKeyTip*)malloc(sizeof(eggKeyTip));
    memset(lp_tip, 0, sizeof(eggKeyTip));
    
    lp_tip->pfield = eggFieldView_new(lp_egg_field);
    lp_tip->pindex = eggIndexView_new(lp_egg_idx, NULL);
    lp_tip->pids = eggIdView_new(lp_egg_idd);
    return lp_tip;
}

EBOOL eggKeyTip_delete(eggKeyTip* ptip)
{
    if(!ptip)
    {
        return EGG_FALSE;
    }
    eggIndexNodeView_delete(ptip->pnodeview);
    if(ptip->pindexinfo)
    {
        free(ptip->pindexinfo);
    }

     eggFieldView_delete(ptip->pfield);
     eggIndexView_delete(ptip->pindex);
     eggIdView_delete(ptip->pids);
    
    free(ptip);
    
}
EBOOL eggKeyTip_set_field(eggKeyTip* ptip, char* fieldname)
{
    eggFieldView_find(ptip->pfield, fieldname, &(ptip->fdid));
    
    if(ptip->pindexinfo)
    {
        free(ptip->pindexinfo);
        ptip->pindexinfo = EGG_NULL;
    }
    
    struct eggIndexInfo *lp_index_info = eggFieldView_getIndexInfo(ptip->pfield, ptip->fdid);
    
    eggIndexView_load_info(ptip->pindex, lp_index_info, ptip->pfield);
    ptip->niteroff = EGGINDEXVIEW_LEAFOFF(ptip->pindex);

    if(ptip->niteroff)
    {
        ptip->pnodeview = eggIndexView_load_node(ptip->pindex, ptip->niteroff);    
    }

    

    return EGG_TRUE;
}

EBOOL eggKeyTip_next_key(eggKeyTip* ptip, char** lpkey, int* psize, int* pCount)
{
    if(!ptip->niteroff)
        return EGG_FALSE;
    if(ptip->nindexiter == EGGINDEXNODEVIEW_RDCNT(ptip->pnodeview))
    {
        ptip->niteroff = eggIndexNodeView_get_nextoff(ptip->pnodeview);
        ptip->nindexiter= 0;
        eggIndexNodeView_delete(ptip->pnodeview);
        ptip->pnodeview = eggIndexView_load_node(ptip->pindex, ptip->niteroff);
    }

    if(!ptip->niteroff || ptip->nindexiter == EGGINDEXNODEVIEW_RDCNT(ptip->pnodeview))
        return EGG_FALSE;


    HEGGINDEXRECORD pRecord = EGGINDEXNODEVIEW_RECORD_INDEX(ptip->pnodeview, ptip->nindexiter);
    ptip->nkeycnt++;
    
    offset64_t n_nodelist_off = 0;
    n_nodelist_off = *(offset64_t*)EGGINDEXRECORD_VAL(pRecord);
//    printf("key : [%s] keylen : %d vallen : %d n_nodelist_off : %llu\n", EGGINDEXRECORD_KEY(pRecord),EGGINDEXRECORD_KSIZE(pRecord), EGGINDEXRECORD_VSIZE(pRecord), n_nodelist_off);
    ptip->nindexiter++;

    *lpkey = (char*)malloc(EGGINDEXRECORD_KSIZE(pRecord)+1);
    
    memcpy(*lpkey, EGGINDEXRECORD_KEY(pRecord), EGGINDEXRECORD_KSIZE(pRecord));
    *((char*)(*lpkey)+EGGINDEXRECORD_KSIZE(pRecord))='\0';
    EGGLISTBLOCK st_list_block;
    if(pCount)
    {
        offset64_t n_id_off = *(offset64_t*)EGGINDEXRECORD_VAL(pRecord);
        ViewStream_read(ptip->pids->hViewStream, &st_list_block, sizeof(st_list_block), n_id_off);
        
       size32_t n_unused_sz = (st_list_block.aCnt - st_list_block.uCnt) * st_list_block.nodeSz;
       offset64_t n_orgdata_off = n_id_off + sizeof(st_list_block) + n_unused_sz;
       size32_t n_orgdata_len = st_list_block.uCnt * st_list_block.nodeSz;
                
       echar* lp_org_block_buf = (echar*)malloc(n_orgdata_len);
       
       ViewStream_read(ptip->pids->hViewStream, lp_org_block_buf, n_orgdata_len, n_orgdata_off);
       
        eggIdNode_set_timeStamp((HEGGIDNODE)lp_org_block_buf,
                                n_orgdata_len/sizeof(EGGIDNODE),
                                EGG_IDTIMESTAMP_BASEVAL);

        ViewStream_update(ptip->pids->hViewStream, lp_org_block_buf, n_orgdata_len, n_orgdata_off);
        free(lp_org_block_buf);
        *pCount = st_list_block.uCnt;
    }
    return EGG_TRUE;
}


int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        printf("argc error!\n");
        exit(-1);
    }
    if(access(argv[1], F_OK))
    {
        printf("path is error!\n");
        exit(-1);
    }
    eggKeyTip* p_tip = eggKeyTip_new(argv[1]);
    eggKeyTip_set_field(p_tip, "content");
    char* lpkey = EGG_NULL;
    int size;
    count_t count = 0;
    while(eggKeyTip_next_key(p_tip, &lpkey, &size, &count) == EGG_TRUE)
    {
        printf("key : [%s] count : [%d]\n", lpkey, count);
        free(lpkey);
    }

    eggKeyTip_delete(p_tip);
    return 0;
}
