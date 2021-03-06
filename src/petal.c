#include <wist/wist.h>
#include <wist/petal.h>
#include <wist/mem.h>
#include <wist/defs.h>
#include <wist/str.h>
#include <wist/index.h>
#include <wist/lexer.h>
#include <wist/sym.h>
#include <wist/uast.h>
#include <wist/parser.h>

/* === PROTOTYPES === */

static WistPetal *wist_petal_create(WistIndex *index, WistPetalBits bits);

/* === PUBLIC FUNCTIONS === */

WistPetal *
wist_petal_parse(WistIndex *index,
                 const char *filename,
                 WistPetalBits bits)
{
    WistPetal *petal = wist_petal_create(index, bits);
    WistStrRef filename_ref = wist_str_from_c(filename);
    petal->start_file = index_file_open(index, filename_ref);
    if (petal->start_file == NULL)
    {
        return NULL;
    }

    WistSpanIndex spans;
    wist_span_index_create(&spans);

    WistSymTable syms;
    sym_table_create(&syms);
    WistErrorEngine *err = wist_error_engine_create(index, &spans);

    WistLexer *lex = wist_lexer_create(petal->index, &spans, &syms, err,
                                       petal->start_file);

    UAst uast;
    uast_create(&uast);

    WistParser parser;
    wist_parser_create(&parser, lex, err);

    wist_parser_parse(&parser, &uast);
    
    if (wist_error_engine_has_errors(err))
    {
        wist_error_engine_print(err);
    }
    else
    {
        for (size_t i = 0; i < uast.ndecls; i++)
        {
            uast_print_decl(uast.roots[i]);
        }
    }
    uast_destroy(&uast);

    wist_parser_destroy(&parser);
    wist_lexer_destroy(lex);
    wist_error_engine_destroy(err);
    wist_span_index_destroy(&spans);
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
