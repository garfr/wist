#include <windows.h>

#include <wist/fs.h>
#include <wist/mem.h>

WistStr
wist_fs_realpath(WistStrRef rel)
{
    char tmp[MAX_PATH];

    WistStr rel_nt = wist_str_null_terminate(rel);
    size_t sz = GetFullPathName(rel_nt.str, MAX_PATH, tmp, NULL);

    wist_str_libc_free(rel_nt);

    char *buf = WIST_NEW_ARR(uint8_t, sz);
    memcpy(buf, tmp, sz);

    WistStr ret = {
        .str = (const uint8_t *) buf,
        .len = sz,
    };

    return ret;
}
