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
static bool compare_strings(WistStr *s1, WistStr *s2);
static uint32_t hash_string(WistStr *s);

/* === PUBLIC FUNCTIONS === */

WistIndex *
wist_index_create()
{
    WistIndex *index = WIST_NEW(WistIndex);
    index->_ref = 1;

    file_map_create(&index->files);
    file_ref_map_create(&index->file_refs);

    return index;
}

void
wist_index_destroy(WistIndex *index)
{
    if (--index->_ref == 0)
    {
        WIST_FREE(index);
    }
}

WistFileRef *
index_file_open(WistIndex *index,
                WistStrRef path)
{
    WistStr abs_path = wist_fs_realpath(path);
    WistFile *file = file_map_find(&index->files, &abs_path);
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

void index_file_destroy(WistIndex *index, WistFileRef *ref);

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
    return file_map_insert(&index->files, &abs_path, &file);
}

static bool
compare_strings(WistStr *s1, WistStr *s2)
{
    return WIST_STREQ(*s1, *s2);
}

static bool
compare_file_refs(WistFileRef *s1, WistFileRef *s2)
{
    return WIST_STREQ(s1->rel_path, s2->rel_path) && s1->file == s2->file;
}

static uint32_t
hash_string(WistStr *s)
{
    return s->len;
}

static uint32_t
hash_file_ref(WistFileRef *f)
{
    return f->rel_path.len;
}

#define WIST_MAP_KEY_TYPE WistStr
#define WIST_MAP_VAL_TYPE WistFile
#define WIST_MAP_TYPE_PREFIX WistFile
#define WIST_MAP_FUN_PREFIX file_map
#define WIST_MAP_EQ compare_strings
#define WIST_MAP_HASH hash_string
#define WIST_MAP_IMPLEMENTATION
#include <wist/map.c.h>

#define WIST_MAP_KEY_TYPE WistFileRef
#define WIST_MAP_TYPE_PREFIX WistFileRef
#define WIST_MAP_FUN_PREFIX file_ref_map
#define WIST_MAP_EQ compare_file_refs
#define WIST_MAP_HASH hash_file_ref
#define WIST_MAP_IMPLEMENTATION
#include <wist/map.c.h>
