#include <stdio.h>
#include <inttypes.h>

#include <wist/token.h>

void
wist_token_print(WistToken tok)
{
    switch (tok.t)
    {
    case WIST_TOK_EOF:
        printf("EOF\n");
        break;
    case WIST_TOK_COLON:
        printf("Colon\n");
        break;
    case WIST_TOK_EQ:
        printf("Eq\n");
        break;
    case WIST_TOK_SYM:
        printf("Sym: '%.*s'\n",
               (int) tok.sym->str.len,
               (char *) tok.sym->str.str);
        break;
    case WIST_TOK_INT:
        printf("Int: %" PRId64 "\n", tok.i);
        break;
    case WIST_TOK_BSLASH:
        printf("Blackslash\n");
        break;
    case WIST_TOK_LPAREN:
        printf("Left paren\n");
        break;
    case WIST_TOK_RPAREN:
        printf("Right paren\n");
        break;
    case WIST_TOK_UNDERSCORE:
        printf("Underscore\n");
        break;
    case WIST_TOK_ARROW:
        printf("Arrow\n");
        break;
    case WIST_TOK_COMMA:
        printf("Comma\n");
        break;
    default:
        printf("NO TOKENI PRINT CASE\n");
    }
}
