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
                 WistErrorEngine *err_eng,
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

    WistLexer *lex = wist_lexer_create(petal->index, err_eng,
                                       petal->start_file);

    WistSpanIndex *spans = &lex->spans;
    WistSymTable syms = lex->syms;

/*
    
    WistToken tok;
    while ((tok = wist_lexer_next(lex)).t != WIST_TOK_EOF)
    {
        wist_token_print(spans, tok);
    }        
    */
   // /*
    UAst uast;
    uast_create(&uast);

    WistParser parser;
    wist_parser_create(&parser, lex, err_eng);

    wist_parser_parse(&parser, &uast);

    uast_print_expr(uast.root);
    wist_parser_destroy(&parser);
    uast_destroy(&uast);
//    */
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
