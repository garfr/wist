#ifndef WIST_MEM_H
#define WIST_MEM_H

#include <stddef.h>

#define WIST_NEW(__type) ((__type *) _wist_alloc(1, sizeof(__type)))
#define WIST_NEW_ARR(__type, __len) ((__type *) _wist_alloc(__len, sizeof(__type)))
#define WIST_REALLOC(__type, __ptr, __nlen) ((__type *) _wist_realloc(__ptr,   \
                                            sizeof(__type) * (__nlen)))
#define WIST_FREE(__ptr) _wist_free(__ptr)

/* PRIVATE */

void *_wist_alloc(size_t len, size_t sz);
void _wist_free(void *ptr);
void *_wist_realloc(void *ptr, size_t len);

#endif /* WIST_MEM_H */
