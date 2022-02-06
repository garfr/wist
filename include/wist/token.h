#ifndef WIST_TOKEN_H
#define WIST_TOKEN_H

#include <wist/defs.h>
#include <wist/sym.h>

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
} WistTokenType;

typedef struct
{
    uint8_t t;

    union
    {
        int64_t i;
        WistSym *sym;
    };
} WistToken;

void wist_token_print(WistToken tok);

#endif /* WIST_TOKEN_H */
