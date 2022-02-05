#include <string.h>

#include <wist/str.h>
#include <wist/mem.h>

WistStr
wist_str_null_terminate(WistStrRef ref)
{
    char *buf = WIST_NEW_ARR(uint8_t, ref.len + 1);
    memcpy(buf, ref.str, ref.len);
    buf[ref.len] = '\0';

    WistStr str = {
        .str = buf,
        .len = ref.len + 1,
    };

    return str;
}

WistStrRef
wist_str_from_c(const char *c_str)
{
    WistStrRef ref = {
        .str = (const uint8_t *) c_str,
        .len = strlen(c_str) + 1,
    };
    return ref;
}

void
wist_str_libc_free(WistStr str)
{
    WIST_FREE((uint8_t *)str.str);
}

WistStr
wist_str_clone(WistStrRef ref)
{
    uint8_t *buf = WIST_NEW_ARR(uint8_t, ref.len);
    memcpy(buf, ref.str, ref.len);

    WistStr ret = {
        .str = buf,
        .len = ref.len,
    };

    return ret;
}

WistStrRef
wist_str_to_ref(WistStr str)
{
    WistStrRef ref = {
        .str = str.str,
        .len = str.len,
    };
    
    return ref;
}
