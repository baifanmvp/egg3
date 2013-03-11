
#ifndef MD5_H
#define MD5_H


#define UTI_MD5_DIGEST_LENGTH 16
#ifdef __cplusplus
extern "C"
{
#endif
/*
 * MD5
 * 2011-09-06
 * maleih@gmail.com
 * */
extern unsigned char *eggUtiMD5(const unsigned char *d, size_t n, unsigned char *md);
#ifdef __cplusplus
}
#endif
/*
 * This is needed to make RSAREF happy on some MS-DOS compilers.
 */

#endif /* !MD5_H */
