#include <stdlib.h>
#include <inttypes.h>

#include <wist/index.h>
#include <wist/mem.h>
#include <wist/fs.h>
#include <wist/str.h>

/* === PROTOTYPES === */

static WistFileRef *find_or_create_ref(WistIndex *index, WistFile *file,
                                WistStrRef path);
static WistFileRef *create_ref(WistIndex *index, WistFile *file,
                               WistStrRef path);
static WistFile *create_file(WistIndex *index, WistStr abs_path);
static bool compare_strings(WistFile *s1, WistFile *s2);
static uint32_t hash_string(WistFile *s);
static bool compare_file_refs(WistFileRef *s1, WistFileRef *s2);
static uint32_t hash_file_ref(WistFileRef *f);
static void file_ref_destructor(WistFileRef *ref);
static void file_destructor(WistFile *file);

/* === PUBLIC FUNCTIONS === */

WistIndex *
wist_index_create()
{
    WistIndex *index = WIST_NEW(WistIndex);
    index->_ref = 1;
    index->next_idx = 0;
    file_map_create(&index->files);
    file_ref_map_create(&index->file_refs);

    return index;
}

void
wist_index_destroy(WistIndex *index)
{
    if (--index->_ref == 0)
    {
        file_map_destroy(&index->files);
        file_ref_map_destroy(&index->file_refs);
        WIST_FREE(index);
    }
}

WistFileRef *
index_file_open(WistIndex *index,
                WistStrRef path)
{
    WistStr abs_path = wist_fs_realpath(path);
    WistFile lookup = {
        .abs_path = abs_path,
    };
    WistFile *file = file_map_find(&index->files, &lookup);
    if (file == NULL)
    {
        file = create_file(index, abs_path);
        if (file == NULL)
        {
            return NULL;
        }
        return create_ref(index, file, path);
    }
    else {
        wist_str_libc_free(abs_path);
        WistFileRef *ref = find_or_create_ref(index, file, path);
        return ref;
    }
}

void
index_file_destroy(WistIndex *index,
                   WistFileRef *ref)
{
    WistFile *file = ref->file;
    file->_ref--;
    if (file->_ref == 0)
    {
        wist_str_libc_free(file->abs_path);
        wist_membuf_destroy(file->buf);
    }
}

/* === PRIVATE FUNCTIONS === */

static WistFileRef *
create_ref(WistIndex *index,
           WistFile *file,
           WistStrRef path)
{
    WistFileRef ref;
    file->_ref++;
    ref.file = file;
    WistStr owned_path = wist_str_clone(path);
    ref.rel_path = owned_path;
    return file_ref_map_insert(&index->file_refs,&ref);
}

static WistFileRef *
find_or_create_ref(WistIndex *index,
                   WistFile *file,
                   WistStrRef path)
{
    WistFileRef lookup_ref;
    lookup_ref.rel_path.str = path.str;
    lookup_ref.rel_path.len = path.len;
    lookup_ref.file = file;
    WistFileRef *ref = file_ref_map_find(&index->file_refs, &lookup_ref);
    if (ref != NULL)
    {
        return create_ref(index, file, path);
    }
    return ref;
}

static WistFile *
create_file(WistIndex *index, WistStr abs_path)
{
    WistFile file;
    file._ref = 0;
    WistStrRef abs_path_ref = wist_str_to_ref(abs_path);
    if (!wist_membuf_open_file(abs_path_ref, &file.buf))
    {
        return NULL;
    }
    file.abs_path = abs_path;
    file.idx_start = index->next_idx;
    file.idx_end = (index->next_idx += file.buf.len) + 1;
    return file_map_insert(&index->files, &file);
}

static bool
compare_strings(WistFile *s1, WistFile *s2)
{
    return WIST_STREQ(s1->abs_path, s2->abs_path);
}

static bool
compare_file_refs(WistFileRef *s1, WistFileRef *s2)
{
    return WIST_STREQ(s1->rel_path, s2->rel_path) && s1->file == s2->file;
}

static uint32_t
hash_file(WistFile *f)
{
    return wist_str_hash(wist_str_to_ref(f->abs_path));
}

static uint32_t
hash_file_ref(WistFileRef *f)
{
    return wist_str_hash(wist_str_to_ref(f->rel_path));
}

static void
file_ref_destructor(WistFileRef *ref)
{
    wist_str_libc_free(ref->rel_path);
}

static void
file_destructor(WistFile *file)
{
    wist_str_libc_free(file->abs_path);
}

WistFile *
index_get_span_file(WistIndex *index, 
                    WistSpanIndex *spans, 
                    WistSpan *span)
{
    WistFileMapIter iter = file_map_iter_create(&index->files);
    WistWideSpan wspan;
    if (span->len == 0)
        wspan = *wist_get_span(spans, span);
    else
    {
        wspan.start = span->start;
        wspan.end = span->start + span->len;
    }
    WistFile *file;
    while ((file = file_map_iter_next(&iter)) != NULL)
    {
        if (file->idx_start <= wspan.start && file->idx_end >= wspan.end)
        {
            return file;
        }
    }
    
    printf("THIS SHOULD NEVER HAPPEN\n");
    exit(EXIT_FAILURE);
}

WistStrRef 
index_get_span(WistIndex *index, 
               WistSpanIndex *spans,
               WistSpan *span)
{
    WistStrRef ref;
    WistFile *file = index_get_span_file(index, spans, span);
    WistWideSpan wspan;
    if (span->len == 0)
        wspan = *wist_get_span(spans, span);
    else
    {
        wspan.start = span->start;
        wspan.end = span->start + span->len;
    }

    ref.str = file->buf.data + (wspan.start - file->idx_start);
    ref.len = wspan.end - wspan.start;
    return ref;
}

#define WIST_MAP_KEY_TYPE WistFile
#define WIST_MAP_TYPE_PREFIX WistFile
#define WIST_MAP_FUN_PREFIX file_map
#define WIST_MAP_EQ compare_strings
#define WIST_MAP_DESTRUCnTOR file_destructor
#define WIST_MAP_HASH hash_file
#define WIST_MAP_IMPLEMENTATION
#include <wist/map.c.h>

#define WIST_MAP_KEY_TYPE WistFileRef
#define WIST_MAP_TYPE_PREFIX WistFileRef
#define WIST_MAP_FUN_PREFIX file_ref_map
#define WIST_MAP_EQ compare_file_refs
#define WIST_MAP_HASH hash_file_ref
#define WIST_MAP_DESTRUCTOR file_ref_destructor
#define WIST_MAP_IMPLEMENTATION
#include <wist/map.c.h>
