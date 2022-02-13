#ifndef WIST_STR_H
#define WIST_STR_H

#include <string.h>

#include <wist/defs.h>

/* Owned string. */
typedef struct
{
    const uint8_t *str;
    size_t len;
} WistStr;

/* Non-owning slice of a string. */
typedef struct
{
    const uint8_t *str;
    size_t len;
} WistStrRef;

#define WIST_STREQ(__str1, __str2)  ((__str1).len == (__str2).len ?          \
                                       ((strncmp((__str1).str, (__str2).str,   \
                                                 (__str1).len) == 0)) : 0)

WistStr wist_str_null_terminate(WistStrRef ref);
WistStrRef wist_str_from_c(const char *c_str);
WistStr wist_str_from_slice(const uint8_t *str, size_t len);
WistStr wist_str_clone(WistStrRef ref);
WistStrRef wist_str_to_ref(WistStr str);

uint32_t wist_str_hash(WistStrRef ref);

/* Frees a string allocated with libc functions. */
void wist_str_libc_free(WistStr str);

/* String formatter. */
WistStr wist_format(const char *str, ...);

#endif /* WIST_STR_H */
