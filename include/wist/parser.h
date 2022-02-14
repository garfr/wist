#ifndef WIST_PARSER_H
#define WIST_PARSER_H

#include <wist/uast.h>
#include <wist/lexer.h>
#include <wist/error.h>

typedef struct
{
    WistLexer *lex;
    WistToken tok;
    WistErrorEngine *err;
} WistParser;

void wist_parser_create(WistParser *out, WistLexer *lexer,
                        WistErrorEngine *err);
void wist_parser_parse(WistParser *parser, UAst *uast);
void wist_parser_destroy(WistParser *parser);

#endif /* WIST_PARSER_H */

