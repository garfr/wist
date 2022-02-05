#include <stdlib.h>

#include <wist/mem.h>

void *
_wist_alloc(size_t len,
            size_t sz)
{
    void *ptr = calloc(len, sz);
    if (ptr == NULL)
    {
        printf("Wist: OOM.\n");
        exit(EXIT_FAILURE);
    }
    return ptr;
}

void
_wist_free(void *ptr)
{
    free(ptr);
}

void *
_wist_realloc(void *ptr, size_t len)
{
    void *ret = realloc(ptr, len);
    if (ret == NULL)
    {
        printf("Wist: OOM.\n");
        exit(EXIT_FAILURE);
    }
    return ret;
}
