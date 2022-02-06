#include <stdio.h>

#include <wist/mem.h>
#include <wist/membuf.h>

bool
wist_membuf_open_file(WistStrRef ref, WistMembuf *out)
{
    WistStr ref_nt = wist_str_null_terminate(ref);
    FILE *f = fopen(ref_nt.str, "rb");
    wist_str_libc_free(ref_nt);
    if (f == NULL)
    {
        return false;
    }

    fseek(f, 0, SEEK_END);
    size_t sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buf = WIST_NEW_ARR(uint8_t, sz);
    fread(buf, 1, sz, f);
    fclose(f);

    out->data = buf;
    out->len = sz;
    out->t = WIST_MEMBUF_MALLOC;

    return true;
}

void
wist_membuf_destroy(WistMembuf buf)
{
    if (buf.t == WIST_MEMBUF_MALLOC)
    {
        WIST_FREE((uint8_t *)buf.data);
    }
}
