#ifndef EGGNETTYPE_H_
#define EGGNETTYPE_H_

#define fetchSize32(retv, p) ( (retv) =  (size32_t)((unsigned char)*(p)++), \
                               (retv) += (size32_t)((unsigned char)*(p)++ << 8), \
                               (retv) += (size32_t)((unsigned char)*(p)++ << 16), \
                               (retv) += (size32_t)((unsigned char)*(p)++ << 24))

#define storeSize32(p, val) ( *(unsigned char*)(p)++ = (unsigned char)(val), \
                              *(unsigned char*)(p)++ = (unsigned char)((val)>>8), \
                              *(unsigned char*)(p)++ = (unsigned char)((val)>>16), \
                              *(unsigned char*)(p)++ = (unsigned char)((val)>>24))
#endif
