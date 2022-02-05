#ifndef WIST_INDEX_H
#define WIST_INDEX_H

#include <wist/wist.h>
#include <wist/file.h>

#define WIST_MAP_HEADER
#define WIST_MAP_KEY_TYPE WistStr
#define WIST_MAP_VAL_TYPE WistFile
#define WIST_MAP_TYPE_PREFIX WistFile
#define WIST_MAP_FUN_PREFIX file_map
#include <wist/map.c.h>

#define WIST_MAP_HEADER
#define WIST_MAP_KEY_TYPE WistFileRef
#define WIST_MAP_TYPE_PREFIX WistFileRef
#define WIST_MAP_FUN_PREFIX file_ref_map
#include <wist/map.c.h>

struct WistIndex
{
    unsigned _ref;

    WistFileMap files;
    WistFileRefMap file_refs;
};

WistFileRef *index_file_open(WistIndex *index, WistStrRef path);
void index_file_destroy(WistIndex *index, WistFileRef *ref);

#endif /* WIST_INDEX_H */
