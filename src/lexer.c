#include <ctype.h>
#include <inttypes.h>

#include <wist/lexer.h>
#include <wist/index.h>
#include <wist/mem.h>
#include <wist/error.h>

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
static bool skip_whitespace(WistLexer *lex, uint64_t *out, WistSpan *index);
static WistSym *lex_sym(WistLexer *lex);
static int64_t lex_int(WistLexer *lex);
static WistSpan lex_reset(WistLexer *lex);
static void make_tok_inplace(WistLexer *lex, WistToken *tok, WistTokenType t);

/* === PUBLIC FUNCTIONS === */

WistLexer *
wist_lexer_create(WistIndex *index,
                  WistErrorEngine *err_eng,
                  WistFileRef *start)
{
    WistLexer *lex = WIST_NEW(WistLexer);
    
    lex->first_token = true;
    lex->index = index;
    lex->start_file = start;
    lex->err_eng = err_eng;

    lex->index->_ref++;
    start->file->_ref++;
    
    lex->state = 0;
    lex->states[lex->state] = 1;

    sym_table_create(&lex->syms);
    wist_span_index_create(&lex->spans);
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
    wist_span_index_destroy(&lex->spans);
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

    uint64_t indent;
    bool changed_first_status = false;
    WistSpan index;
    if (skip_whitespace(lex, &indent, &tok.loc))
    {
        if (indent == lex->states[lex->state] && !lex->first_token)
        {
            tok.t = WIST_TOK_NL_SCOLON;
            return tok;
        }
    }
    if (lex->first_token)
    {
        lex->states[lex->state] = indent;
        lex->first_token = false;
        changed_first_status = true;
    }
    
    if (IS_END(lex))
    {
        return tok;
    }

    int c = PEEK_C(lex);

    switch (c)
    {
    case '_':
        make_tok_inplace(lex, &tok, WIST_TOK_UNDERSCORE);
        return tok;
    case '\\':
        make_tok_inplace(lex, &tok, WIST_TOK_BSLASH);
        return tok;
    case ',':
        make_tok_inplace(lex, &tok, WIST_TOK_COMMA);
        return tok;
    case '(':
        make_tok_inplace(lex, &tok, WIST_TOK_LPAREN);
        return tok;
    case ';':
        make_tok_inplace(lex, &tok, WIST_TOK_SCOLON);
        return tok;
    case ')':
        make_tok_inplace(lex, &tok, WIST_TOK_RPAREN);
        return tok;
    case '-':
        SKIP_C(lex);
        if (PEEK_C(lex) == '>')
        {
            make_tok_inplace(lex, &tok, WIST_TOK_ARROW);
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
        make_tok_inplace(lex, &tok, WIST_TOK_COLON);
        return tok;
    case '=':
        make_tok_inplace(lex, &tok, WIST_TOK_EQ);
        return tok;
    }
    if (changed_first_status)
    {
        lex->first_token = true;
        lex->states[lex->state] = 0;
    }
    RESET(lex);
    SKIP_C(lex);
    WistError *err = wist_add_error(lex->err_eng);
    WistStr msg = wist_format("unexpected char '%c'", c);
    WistMultiSpan span = wist_multispan_create(1, RESET(lex));
    wist_fill_error(err, WIST_ERROR_ERR, WIST_ERROR_UNEXPECTED_CHAR, msg, span);
    return lexer_get(lex);
}

static bool
skip_whitespace(WistLexer *lex, uint64_t *out, WistSpan *span)
{
    bool has_newline = false;
    size_t count = 0;
    while (!IS_END(lex) && isspace(PEEK_C(lex)))
    {
        int c = GET_C(lex);
        if (c == '\n')
        {
            *span = wist_add_span(&lex->spans, lex->cur, lex->cur + 1);
            has_newline = true;
            count = 0;
        }
        else if (c == ' ')
            count++;
        else
        {
            printf("no tabs!: '%c'\n", c);
            count += 4;
        }
    }
    *out = count;
    RESET(lex);

    return !IS_END(lex) && has_newline;
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

static void 
make_tok_inplace(WistLexer *lex, 
                 WistToken *tok,
                 WistTokenType t)
{
    SKIP_C(lex);
    tok->t = t;
    tok->loc = RESET(lex);
}
