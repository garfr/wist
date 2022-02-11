#include <stdio.h>
#include <inttypes.h>

#include <wist/span.h>
#include <wist/token.h>

const char *tok_type_to_str[] =
{
    [WIST_TOK_EOF] = "EOF",
    [WIST_TOK_SYM] = "SYM",
    [WIST_TOK_COLON] = "COLON",
    [WIST_TOK_EQ] = "EQ",
    [WIST_TOK_INT] = "INT",
    [WIST_TOK_BSLASH] = "BLASH",
    [WIST_TOK_LPAREN] = "LPAREN",
    [WIST_TOK_RPAREN] = "RPAREN",
    [WIST_TOK_UNDERSCORE] = "UNDERSCORE",
    [WIST_TOK_ARROW] = "ARROW",
    [WIST_TOK_COMMA] = "COMMA",
    [WIST_TOK_SCOLON] = "SCOLON",
    [WIST_TOK_NL_SCOLON] = "NL SCOLON",
};

void
wist_token_print(WistSpanIndex *index, WistToken tok)
{
    printf("%s", tok_type_to_str[tok.t]);

    switch (tok.t)
    {
    case WIST_TOK_EOF:
    case WIST_TOK_COLON:
    case WIST_TOK_EQ:
    case WIST_TOK_BSLASH:
    case WIST_TOK_LPAREN:
    case WIST_TOK_RPAREN:
    case WIST_TOK_UNDERSCORE:
    case WIST_TOK_ARROW:
    case WIST_TOK_COMMA:
    case WIST_TOK_NL_SCOLON:
        break;
    case WIST_TOK_SYM:
        printf(" : '%.*s'",
               (int) tok.sym->str.len,
               (char *) tok.sym->str.str);
        break;
    case WIST_TOK_INT:
        printf(" : %" PRId64, tok.i);
        break;
    default:
        printf("NO TOKENI PRINT CASE\n");
    }

    if (index != NULL)
    {
        if (tok.loc.len == 0)
        {
            WistWideSpan *span = wist_get_span(index, &tok.loc);
            printf(" : (%" PRIu64 ", %" PRIu64 ")\n", span->start, span->end);
        }
        else
        {
            printf(" : (%" PRIu32 ", %" PRIu32 ")\n", tok.loc.start,
                   tok.loc.start + tok.loc.len);
        }
    }
    else
    {
        printf("\n");
    }
}
