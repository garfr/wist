/* === lib/lexer.h - Wist lexer === 
 * Copyright (C) 2022 Gavin Ratcliff - All Rights Reserved
 * Part of the Wist reference implementation, under the MIT license.
 * See LICENSE.txt for license information.
*/

#include <wist/compiler.h>
#include <wist/lexer.h>
#include <wist/vector.h>

#include <ctype.h>
#include <stdio.h>

struct wist_lexer {
    struct wist_compiler *comp;
    const uint8_t *src;
    size_t src_len;

    /* 
     * We keep the interval between the start and end of the currently 
     * processing token, and then set start to end when we are done 
     * processing it. 
     * */
    size_t start, end;

    /* 
     * If there are multiple unexpected characters in a row, we only report 
     * the first one, so that a nonsense string like "$&o8a" will not be 
     * reported as 5 seperate errors.  When [invalid_char_mode] is true, we 
     * skip past invalid chars but do not report them.
     */
    bool invalid_char_mode;
};

#define INIT_WIST_TOKENS 8

#define SKIP_C(_lexer) ((_lexer)->end++)
#define BACKUP_C(_lexer) ((_lexer)->end--)
#define PEEK_C(_lexer) ((_lexer)->src[(_lexer)->end])
#define RESET(_lexer) ((_lexer)->start = (_lexer)->end)
#define IS_EOI(_lexer) ((_lexer)->end >= (_lexer)->src_len)

/* For pretty printing tokens. */
const char *token_to_string_map[] = {
    [WIST_TOKEN_BACKSLASH] = "Backslash",
    [WIST_TOKEN_SYM] = "Sym",
    [WIST_TOKEN_THIN_ARROW] = "Thin_Arrow",
    [WIST_TOKEN_LPAREN] = "LParen",
    [WIST_TOKEN_RPAREN] = "RParen",
    [WIST_TOKEN_EOI] = "Eoi"
};

/* === PROTOTYPES === */

static struct wist_token make_token(struct wist_lexer *lexer, 
        enum wist_token_kind t);
static void skip_whitespace(struct wist_lexer *lexer);
static struct wist_token lex_next(struct wist_lexer *lexer);
static struct wist_srcloc lexer_get_loc(struct wist_lexer *lexer);

/* === PUBLICS === */

struct wist_token *wist_lex(struct wist_compiler *comp, const uint8_t *src, 
        size_t src_len, size_t *tokens_len_out) {
    struct wist_vector vec;
    WIST_VECTOR_INIT(comp->ctx, &vec, struct wist_token);

    struct wist_lexer lexer = {
        .comp = comp,
        .src = src,
        .src_len = src_len,
        .invalid_char_mode = false,
    };

    struct wist_token tok;
    while ((tok = lex_next(&lexer)).t != WIST_TOKEN_EOI) {
        WIST_VECTOR_PUSH(comp->ctx, &vec, struct wist_token, &tok);
    }
    /* This is the terminating EOI token. */
    WIST_VECTOR_PUSH(comp->ctx, &vec, struct wist_token, &tok);
    
    *tokens_len_out = WIST_VECTOR_LEN(&vec, struct wist_token);
    /* If we overallocated tokens, properly allocate them. */
    WIST_VECTOR_FIX_SIZE(comp->ctx, &vec);
    return WIST_VECTOR_DATA(&vec, struct wist_token);
}

void wist_token_print(struct wist_compiler *comp, struct wist_token tok) {
    printf("%s", token_to_string_map[tok.t]);

    switch (tok.t)
    {
        case WIST_TOKEN_SYM:
            printf(": '%.*s'", (int) tok.sym->str_len, (const char *) tok.sym->str);
            break;
        case WIST_TOKEN_EOI:
        case WIST_TOKEN_BACKSLASH:
        case WIST_TOKEN_THIN_ARROW:
        case WIST_TOKEN_LPAREN:
        case WIST_TOKEN_RPAREN:
            break;
        default:
            printf("Unimplemented switch case in wist_token_print\n");
    }

    size_t src_len = 0;
    const uint8_t *src = wist_srcloc_index_slice(&comp->srclocs, tok.loc, &src_len);
    printf(" - '%.*s'\n", (int) src_len, (const char *) src);
}

/* === PRIVATES === */

static struct wist_token make_token(struct wist_lexer *lexer, 
        enum wist_token_kind t) {
    struct wist_token tok;
    tok.t = t;
    SKIP_C(lexer);
    tok.loc = lexer_get_loc(lexer);
    RESET(lexer);
    lexer->invalid_char_mode = false; /* We sucessfully returned a token, so 
                                         we are out of invalid char mode. */
    return tok;
}

static void skip_whitespace(struct wist_lexer *lexer) {
    while (isspace(PEEK_C(lexer))) {
        SKIP_C(lexer);
    }
}

static struct wist_token lex_next(struct wist_lexer *lexer) {
    int c;
start:

    skip_whitespace(lexer);
    RESET(lexer);

    if (IS_EOI(lexer)) {
        return make_token(lexer, WIST_TOKEN_EOI);
    }

    c = PEEK_C(lexer);

    /* Lex a symbol. */
    if (isalpha(c)) {
        while (!IS_EOI(lexer) && (isalpha((c = PEEK_C(lexer))) || isdigit(c) 
                    || c == '_')) {
            SKIP_C(lexer);
        }

        BACKUP_C(lexer);
        const uint8_t *str = lexer->src + lexer->start;
        size_t len = (lexer->end - lexer->start) + 1;
        struct wist_token tok = make_token(lexer, WIST_TOKEN_SYM);
        tok.sym = wist_sym_index_search(lexer->comp->ctx, &lexer->comp->syms, 
                str, len);
        return tok;
    }

    switch (c)
    {
        case '\\':
            return make_token(lexer, WIST_TOKEN_BACKSLASH);
        case '(':
            return make_token(lexer, WIST_TOKEN_LPAREN);
        case ')':
            return make_token(lexer, WIST_TOKEN_RPAREN);
        case '-': {
            SKIP_C(lexer);
            if ((c = PEEK_C(lexer)) == '>') {
                return make_token(lexer, WIST_TOKEN_THIN_ARROW);
            }
            break;

        }
    }

    if (!lexer->invalid_char_mode)
    {
        struct wist_diag *diag = wist_compiler_add_diag(lexer->comp, WIST_DIAG_UNEXPECTED_CHAR, WIST_DIAG_ERROR);
        struct wist_srcloc loc = lexer_get_loc(lexer);
        wist_diag_add_loc(lexer->comp, diag, loc);
        lexer->invalid_char_mode = true;
    }

    SKIP_C(lexer);
    goto start;

}

static struct wist_srcloc lexer_get_loc(struct wist_lexer *lexer) {
    return wist_srcloc_index_add(lexer->comp->ctx, &lexer->comp->srclocs, 
            lexer->start, lexer->end);
}
