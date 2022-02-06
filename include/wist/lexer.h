#ifndef WIST_LEXER_H
#define WIST_LEXER_H

#include <wist/wist.h>
#include <wist/file.h>
#include <wist/token.h>
#include <wist/membuf.h>
#include <wist/sym.h>

typedef struct
{
    WistIndex *index;
    WistFileRef *start_file;
    WistToken peek;
    bool has_peek;
    uint64_t cur, start;
    WistMembuf buf; /* The currently active buf. */
    WistSymTable syms;
    WistSpanIndex spans;
} WistLexer;

WistLexer *wist_lexer_create(WistIndex *index, WistFileRef *start);
void wist_lexer_destroy(WistLexer *lex);

WistToken wist_lexer_peek(WistLexer *lex);
WistToken wist_lexer_next(WistLexer *lex);

#endif /* WIST_LEXER_H */
