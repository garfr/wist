#ifndef WIST_INDEX_H
#define WIST_INDEX_H

#include <wist/wist.h>
#include <wist/file.h>
#include <wist/span.h>

#define WIST_MAP_HEADER
#define WIST_MAP_KEY_TYPE WistFile
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

    size_t next_idx;
    WistFileMap files;
    WistFileRefMap file_refs;
};

WistFileRef *index_file_open(WistIndex *index, WistStrRef path);
void index_file_destroy(WistIndex *index, WistFileRef *ref);

WistStrRef index_get_span(WistIndex *index, WistSpanIndex *spans, 
                          WistSpan *span);
WistFile *index_get_span_file(WistIndex *index, WistSpanIndex *spans, 
                              WistSpan *span);

#endif /* WIST_INDEX_H */
