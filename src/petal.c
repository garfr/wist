#include <wist/wist.h>
#include <wist/petal.h>
#include <wist/mem.h>
#include <wist/defs.h>
#include <wist/str.h>
#include <wist/index.h>
#include <wist/lexer.h>
#include <wist/sym.h>

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
    petal->start_file = index_file_open(index, filename_ref);
    if (petal->start_file == NULL)
    {
        return NULL;
    }

    printf("%.*s\n", (int) petal->start_file->file->buf.len,
           (char*)petal->start_file->file->buf.data);
    WistLexer *lex = wist_lexer_create(petal->index, petal->start_file);

    WistToken tok;
    while ((tok = wist_lexer_next(lex)).t != WIST_TOK_EOF)
    {
        wist_token_print(tok);
    }

    WistSymTable syms = lex->syms;
    wist_lexer_destroy(lex);

    sym_table_destroy(&syms);

    return petal;
}

void wist_petal_destroy(WistPetal *petal)
{
    index_file_destroy(petal->index, petal->start_file);
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
