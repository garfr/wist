#include <wist/wist.h>
#include <wist/petal.h>
#include <wist/mem.h>
#include <wist/defs.h>
#include <wist/str.h>
#include <wist/index.h>

/* === PROTOTYPES === */

static WistPetal *wist_petal_create(WistIndex *index, WistPetalBits bits);

/* === PUBLIC FUNCTIONS === */

WistPetal *
wist_petal_parse(WistIndex *index,
                 const char *filename,
                 WistPetalBits bits)
{
    (void) filename;

    WistPetal *petal = wist_petal_create(index, bits);
    WistStrRef filename_ref = wist_str_from_c(filename);
    WistFileRef *start_file = index_file_open(index, filename_ref);
    if (start_file == NULL)
    {
        return NULL;
    }

    WistFile *file = start_file->file;
    printf("%.*s\n", (int) file->buf.len, (char*)file->buf.data);
    return petal;
}

void wist_petal_destroy(WistPetal *petal)
{
    wist_index_destroy(petal->index);
    petal->index = NULL;
    WIST_FREE(petal);
}

/* === PRIVATE FUNCTIONS === */

static WistPetal *
wist_petal_create(WistIndex *index,
                  WistPetalBits bits)
{
    (void) bits;

    WistPetal *petal = WIST_NEW(WistPetal);
    petal->index = index;

    return petal;
}
