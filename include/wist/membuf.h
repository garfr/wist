#ifndef WIST_MEMBUF_H
#define WIST_MEMBUF_H

#include <wist/defs.h>
#include <wist/str.h>

#define WIST_MEMBUF_MALLOC 1
#define WIST_MEMBUF_MAP 2

typedef struct
{
    const uint8_t *data;
    size_t len;
    uint8_t t;
} WistMembuf;

bool wist_membuf_open_file(WistStrRef ref, WistMembuf *out);
void wist_membuf_destroy(WistMembuf buf);

#endif /* WIST_MEMBUF_H */
