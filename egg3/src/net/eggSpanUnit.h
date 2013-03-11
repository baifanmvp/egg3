#ifndef EGGSPANUNIT_H
#define EGGSPANUNIT_H

#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>

typedef unsigned long long SPANPOINT;

#define SPANPOINTMIN  ((SPANPOINT)1)
#define SPANPOINTMAX  UINT64_MAX

typedef struct {
    SPANPOINT start;
    SPANPOINT end;
} SPANRANGE;

#define isValidSPANRANGE(range) (SPANPOINTMIN <= range.start \
                                 && range.start <= range.end \
                                 && range.end <= SPANPOINTMAX)
#define cmpSPANRANGE(r1, r2) (r1.end >= r2.start                \
                              ? r1.start <= r2.end ? 0 : 1      \
                              : -1)


typedef struct {
    char *eggDirPath;
    char *hostAddress;
    SPANRANGE range;
} eggSpanUnit;
typedef eggSpanUnit *HEGGSPANUNIT;
typedef eggSpanUnit EGGSPANUNIT;


/*! from eggSpanUnit to inquire string.
 @param[in] hEggSpanUnit 
 @param[out] bufsize inquire string size
 @return inquire string malloc'ed, should be free'ed
*/
char *eggSpanUnit_to_inquirestring(HEGGSPANUNIT hEggSpanUnit, size_t *bufsize);

/*! from inquire string to eggSpanUnit
 @param[in] inquireString
 @param[in,out] len in: inquireString size; out: inquireString size used
 @return eggSpanUnit malloc'ed, should be free'ed
*/
HEGGSPANUNIT eggSpanUnit_from_inquirestring(char *inquireString, int *len);

/*! from eggSpanUnits to result string
 @param[in] hEggSpanUnit eggSpanUnit array pointer
 @param[in] count eggSpanUnit array count 
 @param[out] bufsize result string size
 @return result string malloc'ed, should be free'ed
*/
char *eggSpanUnit_to_string(HEGGSPANUNIT hEggSpanUnit, int count,
                                   size_t *bufsize);

/*! from result string to eggSpanUnits 
 @param[in] buf result string
 @param[in,out] bufsize in: result string size; out: result string used.
 @param[out] cntUnit_needMore When *bufsize == 0, cntUnit_needMore contains bytes needed; When *bufsize > 0, cntUnit_needMore contains eggSpanUnit array count.
 @return eggSpanUnit array, should be free'ed optionally use eggSpanUnit_free.
*/
HEGGSPANUNIT eggSpanUnit_get_spanunits(char *buf, size_t *bufsize,
                                      int *cntUnit_needMore);

int eggSpanUnit_free(HEGGSPANUNIT hEggSpanUnit, int count);

#endif  /* EGGSPANUNIT_H */
