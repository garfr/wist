#include <ctype.h>

#include <wist/lexer.h>
#include <wist/index.h>
#include <wist/mem.h>

#define IS_END(lex) ((lex)->cur >= (lex)->buf.len)
#define GET_C(lex) (lex->buf.data[(lex)->cur++])
#define PEEK_C(lex) ((lex)->buf.data[(lex)->cur])
#define SKIP_C(lex) ((lex)->cur++)
#define RESET(lex) lex_reset(lex)

#define ALPHA 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': \
    case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n':      \
    case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u':      \
    case 'v': case 'w': case 'x': case 'y': case 'z':                          \
    case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G':      \
    case 'H': case 'I': case 'J': case 'K': case 'L': case 'M': case 'N':      \
    case 'O': case 'P': case 'Q': case 'R': case 'S': case 'W': case 'X':      \
    case 'Y': case 'Z'

#define DIGIT '1': case '2': case '3': case '4': case '5': case '6': case '7': \
    case '8': case '9': case '0'
/* === PROTOTYPES === */

static WistToken lexer_get(WistLexer *lex);
static void skip_whitespace(WistLexer *lex);
static WistSym *lex_sym(WistLexer *lex);
static int64_t lex_int(WistLexer *lex);
static WistSpan lex_reset(WistLexer *lex);

/* === PUBLIC FUNCTIONS === */

WistLexer *
wist_lexer_create(WistIndex *index,
                  WistFileRef *start)
{
    WistLexer *lex = WIST_NEW(WistLexer);

    lex->index = index;
    lex->start_file = start;

    lex->index->_ref++;
    start->file->_ref++;

    sym_table_create(&lex->syms);
    lex->start = lex->cur = 0;
    lex->buf = start->file->buf;
    lex->has_peek = false;

    return lex;
}

void
wist_lexer_destroy(WistLexer *lex)
{

    wist_index_destroy(lex->index);
    index_file_destroy(lex->index, lex->start_file);
    WIST_FREE(lex);
}

WistToken
wist_lexer_peek(WistLexer *lex)
{
    if (lex->has_peek)
    {
        return lex->peek;
    }
    lex->has_peek = true;
    return lex->peek = lexer_get(lex);
}

WistToken
wist_lexer_next(WistLexer *lex)
{
    if (lex->has_peek)
    {
        lex->has_peek = false;
        return lex->peek;
    }
    return lexer_get(lex);
}

/* === PRIVATE FUNCTIONS === */

static WistToken
lexer_get(WistLexer *lex)
{
    WistToken tok;
    tok.t = WIST_TOK_EOF;

    skip_whitespace(lex);

    if (IS_END(lex))
    {
        return tok;
    }

    int c = PEEK_C(lex);

    switch (c)
    {
    case '_':
        SKIP_C(lex);
        tok.t = WIST_TOK_UNDERSCORE;
        tok.loc = RESET(lex);
        return tok;
    case '\\':
        SKIP_C(lex);
        tok.t = WIST_TOK_BSLASH;
        tok.loc = RESET(lex);
        return tok;
    case ',':
        SKIP_C(lex);
        tok.t = WIST_TOK_COMMA;
        tok.loc = RESET(lex);
        return tok;
    case '(':
        SKIP_C(lex);
        tok.t = WIST_TOK_LPAREN;
        tok.loc = RESET(lex);
        return tok;
    case ')':
        SKIP_C(lex);
        tok.t = WIST_TOK_RPAREN;
        tok.loc = RESET(lex);
        return tok;
    case '-':
        SKIP_C(lex);
        if (PEEK_C(lex) == '>')
        {
            SKIP_C(lex);
            tok.t = WIST_TOK_ARROW;
            tok.loc = RESET(lex);
            return tok;
        }
        break;
    case ALPHA:
        tok.t = WIST_TOK_SYM;
        tok.sym = lex_sym(lex);
        tok.loc = RESET(lex);
        return tok;
    case DIGIT:
        tok.t = WIST_TOK_INT;
        tok.i = lex_int(lex);
        tok.loc = RESET(lex);
        return tok;
    case ':':
        SKIP_C(lex);
        tok.t = WIST_TOK_COLON;
        tok.loc = RESET(lex);
        return tok;
    case '=':
        SKIP_C(lex);
        tok.t = WIST_TOK_EQ;
        tok.loc = RESET(lex);
        return tok;
    }
    printf("ERROR: '%c'\n", c);
    return tok;
}

static void
skip_whitespace(WistLexer *lex)
{
    while (!IS_END(lex) && isspace(PEEK_C(lex)))
    {
        SKIP_C(lex);
    }
    RESET(lex);
}

static WistSym *
lex_sym(WistLexer *lex)
{
    int c;
    for (;;)
    {
        if (IS_END(lex))
        {
            return wist_sym_add(&lex->syms,
                                lex->buf.data + lex->start,
                                lex->cur - lex->start);
        }

        c = PEEK_C(lex);
        switch (c)
        {
        case ALPHA:
        case DIGIT:
        case '_':
            SKIP_C(lex);
            continue;
        default:
            return wist_sym_add(&lex->syms,
                                lex->buf.data + lex->start,
                                lex->cur - lex->start);
        }
    }
}

static int64_t
lex_int(WistLexer *lex)
{
    int c;
    int64_t total = 0;
    for (;;)
    {
        if (IS_END(lex))
        {
            return total;
        }

        c = PEEK_C(lex);
        switch (c)
        {
        case DIGIT:
            total *= 10;
            total += c - '0';
            SKIP_C(lex);
            continue;
        default:
            return total;
        }
    }
}

static WistSpan
lex_reset(WistLexer *lex)
{
    WistSpan ret = wist_add_span(&lex->spans, lex->start, lex->cur);
    lex->start = lex->cur;
    return ret;
}
