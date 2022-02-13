#ifndef WIST_FILE_H
#define WIST_FILE_H

#include <wist/wist.h>
#include <wist/membuf.h>
#include <wist/str.h>

typedef struct
{
    unsigned _ref;
    WistStr abs_path;
    WistMembuf buf;
    /* First and last bytes this file holds in the current crate. */
    size_t idx_start, idx_end;
} WistFile;

typedef struct
{
    WistFile *file;
    WistStr rel_path;
} WistFileRef;

#endif /* WIST_FILE_H */
