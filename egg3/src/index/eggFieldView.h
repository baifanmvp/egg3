#ifndef EGGFIELDVIEW_H_
#define EGGFIELDVIEW_H_
#include "../egglib/etype.h"
#include "../storage/File.h"
#include <stdint.h>
#include <pthread.h>
E_BEGIN_DECLS
typedef offset64_t fweight_t;
struct eggIndexInfo;

#define EGGFIELDMAX 255
#define EGGFIELDFILESZMAX ((EGGFIELDMAX+1) * sizeof(eggFieldBuf)) /* 65536 */
#define EGGFIELDNAMELEN (sizeof(((eggFieldBuf*)0)->name)-1)

#pragma pack(push)
#pragma pack(4)

typedef struct 
{
    char name[160];
    char analyzerName[40];
    type_t type;
    char b[54];
} eggFieldBuf;
#pragma pack(pop)


struct eggFieldNameInfo
{
    fdid_t fdid;
    char *name;
    char *analyzerName;
    type_t type;
};
typedef struct eggFieldNameInfo eggFieldNameInfo;
typedef struct eggFieldNameInfo *HEGGFIELDNAMEINFO;
typedef struct eggFieldNameInfo EGGFIELDNAMEINFO;

struct eggFieldView;
typedef struct eggFieldView *HEGGFIELDVIEW;
struct eggFieldView
{
    HEGGFILE hEggFile;
    pthread_mutex_t mutex;
};

HEGGFIELDVIEW eggFieldView_new(HEGGFILE hEggFile);

EBOOL eggFieldView_delete(HEGGFIELDVIEW hFieldView);

fdid_t eggFieldView_register(HEGGFIELDVIEW hFieldView, char *fieldName, type_t type, ... /* char *analyzerName */);

EBOOL eggFieldView_unregister(HEGGFIELDVIEW hFieldView, char *fieldName);

EBOOL eggFieldView_set_fieldweight(HEGGFIELDVIEW hFieldView, char *fieldName, fweight_t fieldWeightOff);

fweight_t eggFieldView_get_fieldweight(HEGGFIELDVIEW hFieldView, char *fieldName);

fweight_t eggFieldView_get_fieldweight_byfid(HEGGFIELDVIEW hFieldView, fdid_t fdid);

EBOOL eggFieldView_find(HEGGFIELDVIEW hFieldView, char *fieldName, fdid_t* lpFdid);

type_t eggFieldView_get_type(HEGGFIELDVIEW hFieldView, char *fieldName);

HEGGFIELDNAMEINFO eggFieldView_get_singlefieldnameinfo(HEGGFIELDVIEW hFieldView, char *fieldName);

HEGGFIELDNAMEINFO eggFieldView_get_singlefieldnameinfo_byfid(HEGGFIELDVIEW hFieldView, fdid_t fdid);

HEGGFIELDNAMEINFO eggFieldView_get_fieldnameinfo(HEGGFIELDVIEW hFieldView, size32_t *cnt);

char *eggFieldView_serialise_fieldnameinfo(HEGGFIELDNAMEINFO hFieldNameInfo,
                                          count_t cntFieldNameInfo,
                                          size32_t *size);
HEGGFIELDNAMEINFO eggFieldView_unserialise_fieldnameinfo(char *buf, size32_t size,
                                                        count_t *cntFieldNameInfo);
EBOOL eggFieldView_delete_fieldnameinfo(HEGGFIELDNAMEINFO hFieldNameInfo,
                                        count_t cntFieldNameInfo);

struct eggIndexInfo *eggFieldView_get_indexinfo(HEGGFIELDVIEW hFieldView, fdid_t fdid);

struct eggIndexInfo *eggFieldView_iter(HEGGFIELDVIEW hFieldView, fdid_t *pfdid);

int eggFieldView_release_indexinfo(HEGGFIELDVIEW hFieldView, fdid_t fdid, struct eggIndexInfo *hInfo);

EBOOL eggFieldView_set_actinfo(HEGGFIELDVIEW hFieldView, ActInfo *hActInfo);
EBOOL eggFieldView_clean_actinfo(HEGGFIELDVIEW hFieldView, ActInfo *hActInfo);

int eggFieldView_xlock(HEGGFIELDVIEW hFieldView, fdid_t fdid);
int eggFieldView_slock(HEGGFIELDVIEW hFieldView, fdid_t fdid);
int eggFieldView_unlock(HEGGFIELDVIEW hFieldView, fdid_t fdid);


/* for back-compliance */

#define eggFieldView_unserialise_fieldNameInfo(buf, size, cntFieldNameInfo) eggFieldView_unserialise_fieldnameinfo(buf, size, cntFieldNameInfo)
#define eggFieldView_serialise_fieldNameInfo(hFieldNameInfo, cntFieldNameInfo, size) eggFieldView_serialise_fieldnameinfo(hFieldNameInfo, cntFieldNameInfo, size) 
#define eggFieldView_releaseIndexInfo(hFieldView, fdid, hInfo) eggFieldView_release_indexinfo(hFieldView, fdid, hInfo) 
#define eggFieldView_get_singleFieldNameInfoByFid(hFieldView, fdid) eggFieldView_get_singlefieldnameinfo_byfid(hFieldView, fdid) 
#define eggFieldView_get_singleFieldNameInfo(hFieldView, fieldName) eggFieldView_get_singlefieldnameinfo(hFieldView, fieldName) 
#define eggFieldView_get_fieldNameInfo(hFieldView, cnt) eggFieldView_get_fieldnameinfo(hFieldView, cnt) 
#define eggFieldView_getIndexInfo(hFieldView, fdid) eggFieldView_get_indexinfo(hFieldView, fdid) 
#define eggFieldView_getFieldWeightByFid(hFieldView, fdid) eggFieldView_get_fieldweight_byfid(hFieldView, fdid) 
#define eggFieldView_getFieldWeight(hFieldView, fieldName) eggFieldView_get_fieldweight(hFieldView, fieldName) 
#define eggFieldView_delete_fieldNameInfo(hFieldNameInfo, cntFieldNameInfo) eggFieldView_delete_fieldnameinfo(hFieldNameInfo, cntFieldNameInfo)

/* for back-compliance end */


E_END_DECLS
#endif //EGGFIELDVIEW_H_

