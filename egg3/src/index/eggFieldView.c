#include "../EggDef.h"
#include "eggFieldView.h"
#include "../storage/File.h"
#include "../storage/ViewStream.h"
#include "../storage/eggRecoveryLog.h"
#include "eggIndexView.h"
#include "../log/eggPrtLog.h"
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

static pthread_mutex_t register_mutex = PTHREAD_MUTEX_INITIALIZER;

static int eggFieldView_format(HEGGFILE hEggFile);
static fdid_t eggFieldView_findRegion(HEGGFIELDVIEW hFieldView, char *fieldName, uint32_t type, uint64_t pos1, uint64_t pos2);
static EBOOL eggFieldView_modifyAnalyzerType(HEGGFIELDVIEW hFieldView, fdid_t fdid, type_t type, char *analyzerName);


HEGGFIELDVIEW eggFieldView_new(HEGGFILE hEggFile)
{
     if (eggFieldView_format(hEggFile) < 0) 
     { 
         return EGG_NULL; 
     } 
    
     HEGGFIELDVIEW hFieldView = (HEGGFIELDVIEW)calloc(1, sizeof(struct eggFieldView));
    if (!hFieldView)
    {
        return EGG_NULL;
    }

    hFieldView->hEggFile = hEggFile;
    pthread_mutex_init(&hFieldView->mutex, NULL);
    return hFieldView;   
}

EBOOL eggFieldView_delete(HEGGFIELDVIEW hFieldView)   
{   
    if (hFieldView)   
    {
        EggFile_close(hFieldView->hEggFile);
        pthread_mutex_destroy(&hFieldView->mutex);
            
        free(hFieldView);   
        return EGG_TRUE;   
    }   

    return EGG_FALSE;
}

static EBOOL eggFieldView_modifyAnalyzerType(HEGGFIELDVIEW hFieldView,
                                   fdid_t fdid,
                                   type_t type, char *analyzerName)
{
    EBOOL retv = EGG_TRUE;
    eggFieldBuf efb = {};
    EggFile_read(hFieldView->hEggFile, &efb, sizeof(efb),
                 fdid - offsetof(eggFieldBuf, type) + MAP_VIEW_OFFSET);

    if ((EGG_MASK_INDEX & efb.type) != (EGG_MASK_INDEX & type))
    {
        /* fprintf(stderr, "%s:%d:%s WARN modify type[%d] != [%d] is Not compliant. Ignore\n", */
        /*         __FILE__, __LINE__, __func__, (int)(EGG_MASK_INDEX & efb.type), */
        /*         (int)(EGG_MASK_INDEX & type)); */
        eggPrtLog_warn("eggFieldView", "%s:%d:%s WARN modify type[%d] != [%d] is Not compliant. Ignore\n",
                __FILE__, __LINE__, __func__, (int)(EGG_MASK_INDEX & efb.type),
                (int)(EGG_MASK_INDEX & type));
                
        retv = EGG_FALSE;       /* NOT compliant */
    }
    else if (efb.type & EGG_INDEX_STRING
             && (EGG_MASK_ANALYZER & efb.type) != (EGG_MASK_ANALYZER & type))
    {
        if (type & EGG_NOT_ANALYZED)
        {
            efb.type &= ~EGG_MASK_ANALYZER;
            efb.type |= EGG_NOT_ANALYZED;
        }
        else if (type & EGG_CWS_ANALYZED)
        {
            efb.type &= ~EGG_MASK_ANALYZER;
            efb.type |= EGG_CWS_ANALYZED;
        }
        else if (type & EGG_ANALYZED)
        {
            efb.type &= ~EGG_MASK_ANALYZER;
            efb.type |= EGG_CWS_ANALYZED;
        }
        else if (type & EGG_CN_ANALYZED)
        {
            efb.type &= ~EGG_MASK_ANALYZER;
            efb.type |= EGG_CN_ANALYZED;
        }
        else if (type & EGG_CY_ANALYZED)
        {
            efb.type &= ~EGG_MASK_ANALYZER;
            efb.type |= EGG_CY_ANALYZED;
        }
        else if (type & EGG_CX_ANALYZED)
        {
            efb.type &= ~EGG_MASK_ANALYZER;
            efb.type |= EGG_CX_ANALYZED;
        }
        else if (type & EGG_OTHER_ANALYZED)
        {
            if (analyzerName && analyzerName[0])
            {
                if (strlen(analyzerName) >= sizeof(efb.analyzerName))
                {
                    /* fprintf(stderr, "%s:%d:%s ERR fieldName[%s] analyzerName strlen(%s) >= %d truncate it\n", __FILE__, __LINE__, __func__, efb.name, analyzerName, sizeof(efb.analyzerName)); */

                    eggPrtLog_error("eggFieldView", "%s:%d:%s ERR fieldName[%s] analyzerName strlen(%s) >= %d truncate it\n", __FILE__, __LINE__, __func__, efb.name, analyzerName, sizeof(efb.analyzerName));

                    retv = EGG_FALSE;
                    goto ret;
                }
                strncpy(efb.analyzerName, analyzerName, sizeof(efb.analyzerName)-1);
            }
            else
            {
//                fprintf(stderr, "%s:%d:%s ERR EGG_OTHER_ANALYZED but analyzerName is NULL\n", __FILE__, __LINE__, __func__);
                eggPrtLog_error("eggFieldView", "%s:%d:%s ERR EGG_OTHER_ANALYZED but analyzerName is NULL\n", __FILE__, __LINE__, __func__);
                retv = EGG_FALSE;
                goto ret;
            }
            efb.type &= ~EGG_MASK_ANALYZER;
            efb.type |= EGG_OTHER_ANALYZED;
        }
        EggFile_write(hFieldView->hEggFile, &efb, sizeof(efb),
                      fdid - offsetof(eggFieldBuf, type) + MAP_VIEW_OFFSET);

    }
    
ret:    
    return retv;
}

fdid_t eggFieldView_register(HEGGFIELDVIEW hFieldView, char *fieldName, type_t type, ... /* char *analyzerName */)
{
    if (!hFieldView)
    {
        return 0;
    }
    if (!(type & (EGG_INDEX_STRING
                  | EGG_INDEX_INT32
                  | EGG_INDEX_INT64
                  | EGG_INDEX_DOUBLE)))
    {
        return 0;
    }
    fdid_t field_id = 0;
    char *analyzerName = NULL;
    if (type & EGG_OTHER_ANALYZED)
    {
        char *p = NULL;
        va_list ap_arg;
        va_start(ap_arg, type);
        p = va_arg(ap_arg, char*);
        if (p && p[0])
        {
            analyzerName = strdup(p);
            assert(analyzerName);
        }
        va_end(ap_arg);
    }
    uint64_t stable_position = 0;
    EGGINDEXINFO eggIndexInfo = {};
    eggFieldBuf fb = {};
    uint64_t fsize = 0;
    
    pthread_mutex_lock(&hFieldView->mutex);
    
    EggFile_lock_wr_wait(hFieldView->hEggFile, SEEK_SET, MAP_VIEW_OFFSET, 0);
        
    /* find first */
    EggFile_read(hFieldView->hEggFile, &stable_position, sizeof(uint64_t), MAP_VIEW_OFFSET);
    field_id = eggFieldView_findRegion(hFieldView, fieldName, type, MAP_VIEW_OFFSET, stable_position);
    if (field_id > 0)
    {
        field_id -= MAP_VIEW_OFFSET;

        if (eggFieldView_modifyAnalyzerType(hFieldView, field_id, type, analyzerName) == EGG_FALSE)
        {

            ;         /* Only register compliant field. Not delete. */
            
            /*
            char c = '\0';
            EggFile_write(hFieldView->hEggFile,
                          &c, sizeof(c),
                          field_id - offsetof(eggFieldBuf, type) + MAP_VIEW_OFFSET);
        
            goto newAlloc;
            */


            /* not compliant return error */
            field_id = 0;
            //fprintf(stderr, "%s:%d:%s ERR cannot modify fieldName[%s], not compliant\n", __FILE__, __LINE__, __func__, fieldName);
            eggPrtLog_error("eggFieldView", "%s:%d:%s ERR cannot modify fieldName[%s], not compliant\n", __FILE__, __LINE__, __func__, fieldName);
        }

    }
    else
    {
    newAlloc:
        
        fsize = EggFile_size(hFieldView->hEggFile);
        field_id = fsize + offsetof(eggFieldBuf, type);
        field_id -= MAP_VIEW_OFFSET;
        if (strlen(fieldName) >= sizeof(fb.name))
        {
            /* fprintf(stderr, "%s:%d:%s WARN strlen(%s) >= %d, truncate it\n", */
            /*         __FILE__, __LINE__, __func__, */
            /*         fieldName, sizeof(fb.name)); */

            eggPrtLog_error("eggFieldView", "%s:%d:%s WARN strlen(%s) >= %d, truncate it\n",
                    __FILE__, __LINE__, __func__,
                    fieldName, sizeof(fb.name));

        }
        strncpy(fb.name, fieldName, sizeof(fb.name)-1);
        if (type & EGG_INDEX_INT32)
        {
            eggIndexInfo.type = type;
            if (type & EGG_RANGE_INDEX)
            {
                eggIndexInfo.rdSize = 20+4+12;
                eggIndexInfo.rdCnt = 453; /* indexnode_16 + (indexrecord_20 + keysize + eggrecordid_12) * (cnt + 1) */
            }
            else
            {
                eggIndexInfo.rdSize = 52+36;
                eggIndexInfo.rdCnt = 77;
            }
            eggIndexInfo.fid = field_id;
            memcpy((char *)&fb + offsetof(eggFieldBuf, type), &eggIndexInfo, sizeof(EGGINDEXINFO));
        }
        else if (type & EGG_INDEX_INT64)
        {
            eggIndexInfo.type = type;
            if (type & EGG_RANGE_INDEX) 
            {
                eggIndexInfo.rdSize = 20+8+12;
                eggIndexInfo.rdCnt = 408;
            }
            else
            {
                eggIndexInfo.rdSize = 56+36;
                eggIndexInfo.rdCnt = 71;
            }
            eggIndexInfo.fid = field_id;            
            memcpy((char *)&fb + offsetof(eggFieldBuf, type), &eggIndexInfo, sizeof(EGGINDEXINFO));
        }
        else if (type & EGG_INDEX_DOUBLE)
        {
            eggIndexInfo.type = type;
            if (type & EGG_RANGE_INDEX)
            {
                eggIndexInfo.rdSize = 20+8+12;
                eggIndexInfo.rdCnt = 408;
            }
            else
            {
                eggIndexInfo.rdSize = 56+36;
                eggIndexInfo.rdCnt = 71;
            }
            eggIndexInfo.fid = field_id;            
            memcpy((char *)&fb + offsetof(eggFieldBuf, type), &eggIndexInfo, sizeof(EGGINDEXINFO));
            
        }
        else /* (type & EGG_INDEX_STRING) */
        {
            eggIndexInfo.type = type | EGG_INDEX_STRING;
            if (type & EGG_RANGE_INDEX)
            {
                eggIndexInfo.rdSize = 20+EGG_BTREE_STRING_MAX+12;
                eggIndexInfo.rdCnt = 109;
            }
            else
            {
                eggIndexInfo.rdSize = 112+36;
                eggIndexInfo.rdCnt = 35;
            }

            eggIndexInfo.fid = field_id;
            memcpy((char *)&fb + offsetof(eggFieldBuf, type), &eggIndexInfo, sizeof(EGGINDEXINFO));
        }

        if (type & EGG_OTHER_ANALYZED)
        {
            if (analyzerName)
            {
                if (strlen(analyzerName) >= sizeof(fb.analyzerName))
                {
                    //fprintf(stderr, "%s:%d:%s ERR fieldName[%s] analyzerName strlen(%s) >= %d truncate it\n", __FILE__, __LINE__, __func__, fieldName, analyzerName, sizeof(fb.analyzerName));
                    eggPrtLog_error("eggFieldView", "%s:%d:%s ERR fieldName[%s] analyzerName strlen(%s) >= %d truncate it\n", __FILE__, __LINE__, __func__, fieldName, analyzerName, sizeof(fb.analyzerName));
                }
                strncpy(fb.analyzerName, analyzerName, sizeof(fb.analyzerName)-1);
            }
        }
        
        EggFile_write(hFieldView->hEggFile, &fb, sizeof(fb), fsize);
        fsize += sizeof(fb);

        EggFile_write(hFieldView->hEggFile, &fsize, sizeof(uint64_t), MAP_VIEW_OFFSET);

    }
    
    free(analyzerName);
    EggFile_unlock(hFieldView->hEggFile, SEEK_SET, MAP_VIEW_OFFSET, 0);    
    pthread_mutex_unlock(&hFieldView->mutex);
    return field_id;
}

EBOOL eggFieldView_unregister(HEGGFIELDVIEW hFieldView, char *fieldName)
{
    if (!hFieldView)
    {
        return EGG_FALSE;
    }

    pthread_mutex_lock(&hFieldView->mutex);
    
    fdid_t field_id = 0;
    
    uint64_t stable_position = 0;

    /* find first */
    EggFile_lock_wr_wait(hFieldView->hEggFile, SEEK_SET, MAP_VIEW_OFFSET, sizeof(uint64_t));
    EggFile_read(hFieldView->hEggFile, &stable_position, sizeof(uint64_t), MAP_VIEW_OFFSET);
    EggFile_unlock(hFieldView->hEggFile, SEEK_SET, MAP_VIEW_OFFSET, sizeof(uint64_t));
    
    field_id = eggFieldView_findRegion(hFieldView, fieldName, 0, MAP_VIEW_OFFSET, stable_position);
    if (field_id > 0)
    {
        /* delete */
        char c = '\0';
        eggFieldView_xlock(hFieldView, field_id - MAP_VIEW_OFFSET);
        EggFile_write(hFieldView->hEggFile,
                      &c, sizeof(c),
                      field_id - offsetof(eggFieldBuf, type));
        eggFieldView_unlock(hFieldView, field_id - MAP_VIEW_OFFSET);
    }
    
    pthread_mutex_unlock(&hFieldView->mutex);    
    return EGG_TRUE;
}

EBOOL eggFieldView_find(HEGGFIELDVIEW hFieldView, char *fieldName, fdid_t* lpFdid)
{
    
    if (!hFieldView)
    {
        return EGG_FALSE;
    }
    if (lpFdid == EGG_NULL)
    {
        return EGG_FALSE;
    }

    pthread_mutex_lock(&hFieldView->mutex);
    
    *lpFdid = 0;
    uint64_t stable_size = 0;
    EggFile_lock_wr_wait(hFieldView->hEggFile, SEEK_SET, MAP_VIEW_OFFSET, sizeof(uint64_t));
    EggFile_read(hFieldView->hEggFile, &stable_size, sizeof(uint64_t), MAP_VIEW_OFFSET);
    EggFile_unlock(hFieldView->hEggFile, SEEK_SET, MAP_VIEW_OFFSET, sizeof(uint64_t));

    *lpFdid = eggFieldView_findRegion(hFieldView, fieldName, 0, MAP_VIEW_OFFSET, stable_size);
    
    if (*lpFdid == 0)
    {
        pthread_mutex_unlock(&hFieldView->mutex);
        return EGG_FALSE;
    }
    else
    {
        *lpFdid -= MAP_VIEW_OFFSET;
        pthread_mutex_unlock(&hFieldView->mutex);
        return EGG_TRUE;
    }
}

type_t eggFieldView_get_type(HEGGFIELDVIEW hFieldView, char *fieldName)
{
    if (!hFieldView)
    {
        return EGG_FALSE;
    }
    pthread_mutex_lock(&hFieldView->mutex);

    offset64_t n_field_off = 0;
    uint64_t stable_size = 0;
    EggFile_lock_wr_wait(hFieldView->hEggFile, SEEK_SET, MAP_VIEW_OFFSET, sizeof(uint64_t));
    EggFile_read(hFieldView->hEggFile, &stable_size, sizeof(uint64_t), MAP_VIEW_OFFSET);
    EggFile_unlock(hFieldView->hEggFile, SEEK_SET, MAP_VIEW_OFFSET, sizeof(uint64_t));
    n_field_off = eggFieldView_findRegion(hFieldView, fieldName, 0, MAP_VIEW_OFFSET, stable_size);

    if (n_field_off == 0)
    {
        pthread_mutex_unlock(&hFieldView->mutex);
        return EGG_FALSE;
    }
    type_t type = 0;
    eggFieldView_xlock(hFieldView, n_field_off - MAP_VIEW_OFFSET);
    EggFile_read(hFieldView->hEggFile, &type, sizeof(type), n_field_off);
    eggFieldView_unlock(hFieldView, n_field_off - MAP_VIEW_OFFSET);
    pthread_mutex_unlock(&hFieldView->mutex);
    return type;

}




static  int eggFieldView_format(HEGGFILE hEggFile)
{
    if (EggFile_size(hEggFile) >= sizeof(eggFieldBuf) + MAP_VIEW_OFFSET)
    {
        /* already done */
        return 0;
    }
    char hd[MAP_VIEW_OFFSET] = "";
    EggFile_write(hEggFile, hd, sizeof(hd), 0);
    
    char b[sizeof(eggFieldBuf)] = "";
    //*(uint64_t *)(b) = sizeof(eggFieldBuf);
    *(uint64_t *)(b) = sizeof(eggFieldBuf) + MAP_VIEW_OFFSET; 
/* header information.  for lock mechanism. */
    EggFile_write(hEggFile, &b, sizeof(eggFieldBuf), MAP_VIEW_OFFSET);
    return 0;
}

HEGGINDEXINFO eggFieldView_get_indexinfo(HEGGFIELDVIEW hFieldView, fdid_t fdid)
{
    pthread_mutex_lock(&hFieldView->mutex);    
    fdid += MAP_VIEW_OFFSET;
    HEGGINDEXINFO hInfo;
    hInfo = calloc(1, sizeof(EGGINDEXINFO));
    assert(hInfo);
    EggFile_read(hFieldView->hEggFile, hInfo, sizeof(EGGINDEXINFO), fdid);
    pthread_mutex_unlock(&hFieldView->mutex);    
    return hInfo;
}

HEGGINDEXINFO eggFieldView_iter(HEGGFIELDVIEW hFieldView, fdid_t *pfdid)
{
    if (!pfdid || !hFieldView)
    {
        return NULL;
    }
    
    pthread_mutex_lock(&hFieldView->mutex);
    
    size64_t fsize = EggFile_size(hFieldView->hEggFile);
    if (fsize < sizeof(eggFieldBuf) + MAP_VIEW_OFFSET)
    {
        pthread_mutex_unlock(&hFieldView->mutex);        
        return NULL;
    }
    
    fdid_t fdid;
    if (*pfdid == 0)
    {
        fdid = MAP_VIEW_OFFSET + sizeof(eggFieldBuf) + offsetof(eggFieldBuf, type);
    }
    else
    {
        fdid = *pfdid + MAP_VIEW_OFFSET;
    }
    
    uint64_t stable_size = 0;
    EggFile_lock_wr_wait(hFieldView->hEggFile, SEEK_SET, MAP_VIEW_OFFSET, sizeof(uint64_t));
    EggFile_read(hFieldView->hEggFile, &stable_size, sizeof(uint64_t), MAP_VIEW_OFFSET);
    EggFile_unlock(hFieldView->hEggFile, SEEK_SET, MAP_VIEW_OFFSET, sizeof(uint64_t));
    
    if (fdid >= stable_size)
    {
        pthread_mutex_unlock(&hFieldView->mutex);        
        return NULL;
    }

    eggFieldView_slock(hFieldView, fdid - MAP_VIEW_OFFSET);
    static char b[sizeof(EGGINDEXINFO)] = "";
    HEGGINDEXINFO hInfo;
    hInfo = calloc(1, sizeof(EGGINDEXINFO));
    assert(hInfo);
    if (EggFile_read(hFieldView->hEggFile, hInfo, sizeof(EGGINDEXINFO), fdid) == EGG_FALSE)
    {
        free(hInfo);
        hInfo = NULL;
    }
    else if (memcmp(hInfo, b, sizeof(EGGINDEXINFO)) == 0)
    {
        free(hInfo);
        hInfo = NULL;
    }
    
    *pfdid = fdid - MAP_VIEW_OFFSET + sizeof(eggFieldBuf);
    
    eggFieldView_unlock(hFieldView, fdid - MAP_VIEW_OFFSET);
    
    pthread_mutex_unlock(&hFieldView->mutex);    
    return hInfo;
}

HEGGFIELDNAMEINFO eggFieldView_get_singlefieldnameinfo(HEGGFIELDVIEW hFieldView, char *fieldName)
{
    if (!hFieldView)
    {
        return NULL;
    }
    pthread_mutex_lock(&hFieldView->mutex);

    offset64_t n_field_off = 0;
    uint64_t stable_size = 0;
    EggFile_lock_wr_wait(hFieldView->hEggFile, SEEK_SET, MAP_VIEW_OFFSET, sizeof(uint64_t));
    EggFile_read(hFieldView->hEggFile, &stable_size, sizeof(uint64_t), MAP_VIEW_OFFSET);
    EggFile_unlock(hFieldView->hEggFile, SEEK_SET, MAP_VIEW_OFFSET, sizeof(uint64_t));
    n_field_off = eggFieldView_findRegion(hFieldView, fieldName, 0, MAP_VIEW_OFFSET, stable_size);
    fdid_t fdid = n_field_off - MAP_VIEW_OFFSET;

    if (n_field_off == 0)
    {
        pthread_mutex_unlock(&hFieldView->mutex);
        return NULL;
    }
    type_t type = 0;
    eggFieldBuf efb = {};
    eggFieldView_xlock(hFieldView, fdid);
    EggFile_read(hFieldView->hEggFile, &efb, sizeof(efb),
                 n_field_off - offsetof(eggFieldBuf, type));
    eggFieldView_unlock(hFieldView, fdid);

    pthread_mutex_unlock(&hFieldView->mutex);
    
    HEGGFIELDNAMEINFO hFieldNameInfo = calloc(1, sizeof(EGGFIELDNAMEINFO));
    assert(hFieldNameInfo);
    hFieldNameInfo->fdid = fdid;
    hFieldNameInfo->name = strdup(efb.name);
    assert(hFieldNameInfo->name);
    if (efb.analyzerName && efb.analyzerName[0])
    {
        hFieldNameInfo->analyzerName = strdup(efb.analyzerName);
        assert(hFieldNameInfo->analyzerName);
    }
    hFieldNameInfo->type = efb.type;

    return hFieldNameInfo;
    
}

HEGGFIELDNAMEINFO eggFieldView_get_singlefieldnameinfo_byfid(HEGGFIELDVIEW hFieldView, fdid_t fdid)
{
    if (!hFieldView)
    {
        return NULL;
    }

    pthread_mutex_lock(&hFieldView->mutex);

    uint64_t stable_size = 0;
    EggFile_lock_wr_wait(hFieldView->hEggFile, SEEK_SET, MAP_VIEW_OFFSET, sizeof(uint64_t));
    EggFile_read(hFieldView->hEggFile, &stable_size, sizeof(uint64_t), MAP_VIEW_OFFSET);
    EggFile_unlock(hFieldView->hEggFile, SEEK_SET, MAP_VIEW_OFFSET, sizeof(uint64_t));
    if (stable_size <= fdid + MAP_VIEW_OFFSET - offsetof(eggFieldBuf, type)
        || fdid < sizeof(eggFieldBuf) + offsetof(eggFieldBuf, type))
    {
        pthread_mutex_unlock(&hFieldView->mutex);        
        return NULL;
    }
    type_t type = 0;
    eggFieldBuf efb = {};
    eggFieldView_xlock(hFieldView, fdid);
    EggFile_read(hFieldView->hEggFile, &efb, sizeof(efb),
                 fdid + MAP_VIEW_OFFSET - offsetof(eggFieldBuf, type));
    eggFieldView_unlock(hFieldView, fdid);

    pthread_mutex_unlock(&hFieldView->mutex);

    HEGGFIELDNAMEINFO hFieldNameInfo = calloc(1, sizeof(EGGFIELDNAMEINFO));
    assert(hFieldNameInfo);
    hFieldNameInfo->fdid = fdid;
    hFieldNameInfo->name = strdup(efb.name);
    assert(hFieldNameInfo->name);
    if (efb.analyzerName && efb.analyzerName[0])
    {
        hFieldNameInfo->analyzerName = strdup(efb.analyzerName);
        assert(hFieldNameInfo->analyzerName);
    }
    hFieldNameInfo->type = efb.type;

    return hFieldNameInfo;
    
}

HEGGFIELDNAMEINFO eggFieldView_get_fieldnameinfo(HEGGFIELDVIEW hFieldView, size32_t *cnt)
{
    if (!hFieldView || !cnt)
    {
        return NULL;
    }
    *cnt = 0;

    pthread_mutex_lock(&hFieldView->mutex);    
    
    size64_t fsize = EggFile_size(hFieldView->hEggFile);
    if (fsize < sizeof(eggFieldBuf) + MAP_VIEW_OFFSET)
    {
        pthread_mutex_unlock(&hFieldView->mutex);        
        return NULL;
    }
    
    uint64_t stable_size = 0;
    EggFile_lock_wr_wait(hFieldView->hEggFile, SEEK_SET, MAP_VIEW_OFFSET, sizeof(uint64_t));
    EggFile_read(hFieldView->hEggFile, &stable_size, sizeof(uint64_t), MAP_VIEW_OFFSET);
    EggFile_unlock(hFieldView->hEggFile, SEEK_SET, MAP_VIEW_OFFSET, sizeof(uint64_t));
    uint64_t startOff = MAP_VIEW_OFFSET + sizeof(eggFieldBuf);

                                /* type does not modified. */
                                /* no lock is needed */
    
    *cnt = (stable_size - startOff) / sizeof(eggFieldBuf);
    if (!*cnt)
    {
        pthread_mutex_unlock(&hFieldView->mutex);        
        return NULL;
    }
    eggFieldBuf *fieldBuf = malloc(*cnt * sizeof(eggFieldBuf));
    assert(fieldBuf);
    if (EggFile_read(hFieldView->hEggFile, fieldBuf, *cnt * sizeof(eggFieldBuf), startOff) == EGG_FALSE)
    {
        free(fieldBuf);
        pthread_mutex_unlock(&hFieldView->mutex);
        *cnt = 0;
        return NULL;
    }
    
    struct eggFieldNameInfo *nameInfo = malloc(*cnt * sizeof(eggFieldNameInfo));
    assert(nameInfo);
    int i;
    fdid_t fdid = sizeof(eggFieldBuf) + offsetof(eggFieldBuf, type);
    for (i = 0; i < *cnt; i++)
    {
        nameInfo[i].fdid = fdid;
        fdid += sizeof(eggFieldBuf);
        nameInfo[i].name = strdup(fieldBuf[i].name);
        assert(nameInfo[i].name);
        nameInfo[i].analyzerName = strdup(fieldBuf[i].analyzerName);
        assert(nameInfo[i].analyzerName);
        nameInfo[i].type = fieldBuf[i].type;
    }
    free(fieldBuf);
    
    pthread_mutex_unlock(&hFieldView->mutex);
    return nameInfo;
}

char *eggFieldView_serialise_fieldnameinfo(HEGGFIELDNAMEINFO hFieldNameInfo,
                                          count_t cntFieldNameInfo,
                                          size32_t *size)
{
    if (!hFieldNameInfo || cntFieldNameInfo == 0)
    {
        *size = 0;
        return NULL;
    }
    
    *size = sizeof(count_t);
    count_t idx = 0;
    while (idx < cntFieldNameInfo)
    {
        *size += sizeof(fdid_t) + strlen(hFieldNameInfo[idx].name)+1
                 + strlen(hFieldNameInfo[idx].analyzerName)+1
                 + sizeof(type_t);
        idx++;
    }
    char *buf = malloc(*size);
    assert(buf);
    idx = 0;
    char *p = buf;
    *(count_t *)p = cntFieldNameInfo;
    p += sizeof(count_t);
    while (idx < cntFieldNameInfo)
    {
        *(fdid_t *)p = hFieldNameInfo[idx].fdid;
        p += sizeof(fdid_t);
        strcpy(p, hFieldNameInfo[idx].name);
        p += strlen(hFieldNameInfo[idx].name) + 1;
        strcpy(p, hFieldNameInfo[idx].analyzerName);
        p += strlen(hFieldNameInfo[idx].analyzerName) + 1;
        *(type_t *)p = hFieldNameInfo[idx].type;
        p += sizeof(type_t);
        idx++;
    }
    return buf;
    
}

HEGGFIELDNAMEINFO eggFieldView_unserialise_fieldnameinfo(char *buf, size32_t size,
                                                        count_t *cntFieldNameInfo)
{
    if (!buf || size < sizeof(count_t))
    {
        *cntFieldNameInfo = 0;
        return NULL;
    }
    char *p = buf;
    *cntFieldNameInfo = *(count_t *)p;
    p += sizeof(count_t);
    HEGGFIELDNAMEINFO hFieldNameInfo = malloc(*cntFieldNameInfo * sizeof(EGGFIELDNAMEINFO));
    assert(hFieldNameInfo);
    count_t idx = 0;
    while (p < buf + size && idx < *cntFieldNameInfo)
    {
        hFieldNameInfo[idx].fdid = *(fdid_t *)p;
        p += sizeof(fdid_t);
        hFieldNameInfo[idx].name = strdup(p);
        assert(hFieldNameInfo[idx].name);
        p += strlen(hFieldNameInfo[idx].name) + 1;
        hFieldNameInfo[idx].analyzerName = strdup(p);
        assert(hFieldNameInfo[idx].analyzerName);
        p += strlen(hFieldNameInfo[idx].analyzerName) + 1;
        hFieldNameInfo[idx].type = *(type_t*)p;
        p += sizeof(type_t);
        idx++;
    }
    if (idx != *cntFieldNameInfo || p > buf + size)
    {
        /* fprintf(stderr, "%s:%d:%s ERR violate (idx %llu == *cntFieldNameInfo %llu) && (p <= buf + size %llu)\n", */
        /*         __FILE__, __LINE__, __func__, (long long unsigned)idx, */
        /*         (long long unsigned)*cntFieldNameInfo, */
        /*         (long long unsigned)size); */

        eggPrtLog_error("eggFieldView", "%s:%d:%s ERR violate (idx %llu == *cntFieldNameInfo %llu) && (p <= buf + size %llu)\n",
                __FILE__, __LINE__, __func__, (long long unsigned)idx,
                (long long unsigned)*cntFieldNameInfo,
                (long long unsigned)size);

                
        free(hFieldNameInfo);
        *cntFieldNameInfo = 0;
        return NULL;
    }
    return hFieldNameInfo;

}

EBOOL eggFieldView_delete_fieldnameinfo(HEGGFIELDNAMEINFO hFieldNameInfo,
                                        count_t cntFieldNameInfo)
{
    if (!hFieldNameInfo || !cntFieldNameInfo)
    {
        return EGG_TRUE;
    }

    count_t idx;
    for (idx = 0; idx < cntFieldNameInfo; idx++)
    {
        free(hFieldNameInfo[idx].name);
        free(hFieldNameInfo[idx].analyzerName);        
    }
    free(hFieldNameInfo);
    return EGG_TRUE;
}



int eggFieldView_release_indexinfo(HEGGFIELDVIEW hFieldView, fdid_t fdid, HEGGINDEXINFO hInfo)
{
    if (!hInfo || fdid == 0 || !hFieldView)
    {
        return 0;
    }

    pthread_mutex_lock(&hFieldView->mutex);
    
    //EggFile_startlog(hFieldView->hEggFile);
    
    fdid += MAP_VIEW_OFFSET;

    EggFile_write(hFieldView->hEggFile,
                  hInfo,
                  sizeof(EGGINDEXINFO),
                  fdid);
    
    pthread_mutex_unlock(&hFieldView->mutex);
    //EggFile_endlog(hFieldView->hEggFile);    
    return 0;
}

EBOOL eggFieldView_set_actinfo(HEGGFIELDVIEW hFieldView, ActInfo *hActInfo)
{
    EBOOL retv;
    if (!hFieldView)
    {
        return EGG_FALSE;
    }

    retv = EggFile_set_actinfo(hFieldView->hEggFile, hActInfo);
    
    return retv;
}

EBOOL eggFieldView_clean_actinfo(HEGGFIELDVIEW hFieldView, ActInfo *hActInfo)
{
    EBOOL retv;
    if (!hFieldView)
    {
        return EGG_FALSE;
    }

    retv = EggFile_set_actinfo(hFieldView->hEggFile, NULL);
    
    return retv;
}


int eggFieldView_xlock(HEGGFIELDVIEW hFieldView, fdid_t fdid)
{
    fdid += MAP_VIEW_OFFSET;
    return EggFile_lock_wr_wait(hFieldView->hEggFile,
                              SEEK_SET, fdid,
                              sizeof(eggFieldBuf)-offsetof(eggFieldBuf, type));
}
int eggFieldView_slock(HEGGFIELDVIEW hFieldView, fdid_t fdid)
{
    fdid += MAP_VIEW_OFFSET;
    return EggFile_lock_rd_wait(hFieldView->hEggFile,
                              SEEK_SET, fdid,
                              sizeof(eggFieldBuf)-offsetof(eggFieldBuf, type));
}
int eggFieldView_unlock(HEGGFIELDVIEW hFieldView, fdid_t fdid)
{
    fdid += MAP_VIEW_OFFSET;
    return EggFile_unlock(hFieldView->hEggFile,
                          SEEK_SET, fdid,
                          sizeof(eggFieldBuf)-offsetof(eggFieldBuf, type));
}

typedef struct {
    uint64_t fdid;
    char fieldName[EGGFIELDNAMELEN+1];
    uint32_t type;
} FieldIdCache;
static FieldIdCache s_FieldIdCache[10];
static int s_iFieldIdCache;
static fdid_t eggFieldView_findRegion(HEGGFIELDVIEW hFieldView, char *fieldName, uint32_t type, uint64_t pos1, uint64_t pos2)
{
    
    if (!hFieldView)
    {
        return 0;
    }
    if (!fieldName || !fieldName[0])
    {
        return 0;
    }
    if (pos1 + sizeof(eggFieldBuf) > pos2)
    {
        return 0;
    }

/*     int icache; */
/*     icache = s_iFieldIdCache; */
/*     do */
/*     { */
/*         if (strcmp(s_FieldIdCache[icache].fieldName, fieldName) == 0) */
/* //            && type == s_FieldIdCache[icache].type) */
/*         { */
/*             break; */
/* //          return s_FieldIdCache[icache].fdid; */
/*         } */
/*         icache = (icache + 1) */
/*             % (sizeof(s_FieldIdCache)/sizeof(s_FieldIdCache[0])); */
/*     }while (icache != s_iFieldIdCache && s_FieldIdCache[icache].fieldName[0]); */
    
    fdid_t field_id = 0;    
    eggFieldBuf *pfb = malloc(pos2 - pos1);
    int count = (pos2 - pos1) / sizeof(eggFieldBuf);
    assert(pfb);
    EggFile_read(hFieldView->hEggFile, pfb, pos2-pos1, pos1);
    int i;
    for (i = 0; i < count; i++)
    {
        if (strlen(fieldName) >= sizeof(pfb->name))
        {
            /* fprintf(stderr, "%s:%d:%s WARN strlen(%s) >= %d, truncate it\n", */
            /*         __FILE__, __LINE__, __func__, */
            /*         fieldName, sizeof(pfb->name)); */
            eggPrtLog_warn("eggFieldView", "%s:%d:%s WARN strlen(%s) >= %d, truncate it\n",
                    __FILE__, __LINE__, __func__,
                    fieldName, sizeof(pfb->name));

        }
        if (strncmp(pfb[i].name, fieldName, sizeof(pfb->name)-1) == 0)
//            && pfb[i].type  == type)
        {
            field_id = pos1 + i * sizeof(eggFieldBuf) + offsetof(eggFieldBuf, type);
            break;
        }
    }
    /* if (field_id != 0) */
    /* { */
    /*     s_FieldIdCache[icache].fdid = field_id; */
    /*     memcpy(s_FieldIdCache[icache].fieldName, pfb[i].name, EGGFIELDNAMELEN+1); */
    /*     s_FieldIdCache[icache].type = pfb[i].type; */
    /* } */
    free(pfb);
    
    return field_id;
}


EBOOL eggFieldView_set_fieldweight(HEGGFIELDVIEW hFieldView, char *fieldName, fweight_t fieldWeightOff)
{

    pthread_mutex_lock(&hFieldView->mutex);
    
    fdid_t field_off = 0;
    
    uint64_t stable_size = 0;
    
    EggFile_lock_wr_wait(hFieldView->hEggFile, SEEK_SET, MAP_VIEW_OFFSET, sizeof(uint64_t));
    EggFile_read(hFieldView->hEggFile, &stable_size, sizeof(uint64_t), MAP_VIEW_OFFSET);
    EggFile_unlock(hFieldView->hEggFile, SEEK_SET, MAP_VIEW_OFFSET, sizeof(uint64_t));
    
    field_off = eggFieldView_findRegion(hFieldView, fieldName, 0, MAP_VIEW_OFFSET, stable_size);
    if (field_off == 0)
    {
        pthread_mutex_unlock(&hFieldView->mutex);
        return EGG_FALSE;
    }
    fdid_t fdid = field_off - MAP_VIEW_OFFSET;

    eggFieldView_xlock(hFieldView, fdid);
    
    EBOOL retv;
    retv = EggFile_write(hFieldView->hEggFile,
                         &fieldWeightOff, sizeof(fweight_t),
                         field_off + sizeof(EGGINDEXINFO));

    eggFieldView_unlock(hFieldView, fdid);
    
    pthread_mutex_unlock(&hFieldView->mutex);
    return retv;
}


fweight_t eggFieldView_get_fieldweight(HEGGFIELDVIEW hFieldView, char *fieldName)
{

    pthread_mutex_lock(&hFieldView->mutex);
    
    fdid_t field_off = 0;
    uint64_t stable_size = 0;

    EggFile_lock_wr_wait(hFieldView->hEggFile, SEEK_SET, MAP_VIEW_OFFSET, sizeof(uint64_t));
    EggFile_read(hFieldView->hEggFile, &stable_size, sizeof(uint64_t), MAP_VIEW_OFFSET);
    EggFile_unlock(hFieldView->hEggFile, SEEK_SET, MAP_VIEW_OFFSET, sizeof(uint64_t));
    
    field_off = eggFieldView_findRegion(hFieldView, fieldName, 0, MAP_VIEW_OFFSET, stable_size);
    if (field_off == 0)
    {
        pthread_mutex_unlock(&hFieldView->mutex);
        return 0;
    }
    fdid_t fdid = field_off - MAP_VIEW_OFFSET;

    eggFieldView_slock(hFieldView, fdid);
    
    fweight_t fieldWeightOff = 0;
    EBOOL r;
    r = EggFile_read(hFieldView->hEggFile,
                     &fieldWeightOff, sizeof(fweight_t),
                     field_off + sizeof(EGGINDEXINFO));

    eggFieldView_unlock(hFieldView, fdid);
        
    if (r == EGG_FALSE)
    {
        pthread_mutex_unlock(&hFieldView->mutex);        
        return 0;
    }
    else
    {
        pthread_mutex_unlock(&hFieldView->mutex);
        return fieldWeightOff;
    }
}

fweight_t eggFieldView_get_fieldweight_byfid(HEGGFIELDVIEW hFieldView, fdid_t fdid)
{

    pthread_mutex_lock(&hFieldView->mutex);
    
    eggFieldView_slock(hFieldView, fdid);
    
    fweight_t fieldWeightOff = 0;
    EBOOL r;
    r = EggFile_read(hFieldView->hEggFile,
                     &fieldWeightOff, sizeof(fweight_t),
                     fdid + MAP_VIEW_OFFSET + sizeof(EGGINDEXINFO));

    eggFieldView_unlock(hFieldView, fdid);
        
    if (r == EGG_FALSE)
    {
        pthread_mutex_unlock(&hFieldView->mutex);        
        return 0;
    }
    else
    {
        pthread_mutex_unlock(&hFieldView->mutex);
        return fieldWeightOff;
    }
}

