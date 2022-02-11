#ifndef WIST_TOKEN_H
#define WIST_TOKEN_H

#include <wist/defs.h>
#include <wist/sym.h>
#include <wist/span.h>

typedef enum
{
    WIST_TOK_EOF,
    WIST_TOK_SYM,
    WIST_TOK_COLON,
    WIST_TOK_EQ,
    WIST_TOK_INT,
    WIST_TOK_BSLASH,
    WIST_TOK_LPAREN,
    WIST_TOK_RPAREN,
    WIST_TOK_UNDERSCORE,
    WIST_TOK_ARROW,
    WIST_TOK_COMMA,
    WIST_TOK_SCOLON,
    WIST_TOK_NL_SCOLON,
} WistTokenType;

typedef struct
{
    WistSpan loc;
    uint8_t t;
    union
    {
        int64_t i;
        WistSym *sym;
    };
} WistToken;

void wist_token_print(WistSpanIndex *index, WistToken tok);

#endif /* WIST_TOKEN_H */

