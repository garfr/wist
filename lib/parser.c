/* === lib/parser.c - Wist parser === 
 * Copyright (C) 2022 Gavin Ratcliff - All Rights Reserved
 * Part of the Wist reference implementation, under the MIT license.
 * See LICENSE.txt for license information.
*/

#include <wist/parser.h>
#include <wist/compiler.h>
#include <wist/ast.h>

#include <stdio.h>

struct wist_parser {
    struct wist_compiler *comp;
    struct wist_token *tokens;
    size_t tokens_len, cur_tok;
};

#define PEEK_TOK(_parser) ((_parser)->tokens[(_parser)->cur_tok])
#define SKIP_TOK(_parser) ((_parser)->cur_tok++)
#define NEXT_TOK(_parser) ((_parser)->tokens[(_parser)->cur_tok++])

/* === PROTOTYPES === */

static struct wist_ast_expr *parse_expr(struct wist_parser *parser);
static struct wist_ast_expr *parse_lam_expr(struct wist_parser *parser);
static struct wist_ast_expr *parse_app_expr(struct wist_parser *parser);
static struct wist_ast_expr *parse_atomic_expr(struct wist_parser *parser);

/* === PUBLICS === */

struct wist_ast_expr *wist_parse_expr(struct wist_compiler *comp, 
        struct wist_token *tokens, size_t tokens_len) {
    struct wist_parser parser = {
        .comp = comp,
        .tokens = tokens,
        .tokens_len = tokens_len,
    };

    return parse_expr(&parser);
}

/* === PRIVATES === */

static struct wist_ast_expr *parse_expr(struct wist_parser *parser) {
    return parse_lam_expr(parser);
}

static struct wist_ast_expr *parse_lam_expr(struct wist_parser *parser) {
    struct wist_token bs_tok = PEEK_TOK(parser);
    if (bs_tok.t == WIST_TOKEN_BACKSLASH) {
        SKIP_TOK(parser);
        struct wist_token sym_tok = NEXT_TOK(parser);
        SKIP_TOK(parser); /* Thin arrow. */
        struct wist_ast_expr *body = parse_expr(parser);
        struct wist_srcloc full_loc = 
            wist_srcloc_index_combine(parser->comp->ctx,  
                    &parser->comp->srclocs, bs_tok.loc, body->loc);
        return wist_ast_create_lam(parser->comp, full_loc, sym_tok.sym, body);
    }
    return parse_app_expr(parser);
}

static struct wist_ast_expr *parse_app_expr(struct wist_parser *parser) {
    struct wist_ast_expr *arg, *full = parse_atomic_expr(parser);
    while ((arg = parse_atomic_expr(parser)) != NULL) {
        struct wist_srcloc full_loc = 
            wist_srcloc_index_combine(parser->comp->ctx, 
                    &parser->comp->srclocs, full->loc, arg->loc);
        struct wist_ast_expr *new = wist_ast_create_app(parser->comp, full_loc, full, arg);
        full = new;
    }
    return full;
}

static struct wist_ast_expr *parse_atomic_expr(struct wist_parser *parser) {
    struct wist_token tok = PEEK_TOK(parser);

    switch (tok.t) {
        case WIST_TOKEN_LPAREN:
        {
            struct wist_token start_tok = NEXT_TOK(parser);
            struct wist_ast_expr *expr = parse_expr(parser);
            struct wist_token end_tok = NEXT_TOK(parser); 
            expr->loc = wist_srcloc_index_combine(parser->comp->ctx, 
                    &parser->comp->srclocs, start_tok.loc, end_tok.loc);
            return expr;
        }
        case WIST_TOKEN_SYM:
            SKIP_TOK(parser);
            return wist_ast_create_var(parser->comp, tok.loc, tok.sym);
        case WIST_TOKEN_EOI:
        case WIST_TOKEN_RPAREN:
            return NULL;
        default:
            wist_token_print(parser->comp, tok);
            printf("Unhandled case in parse_atomic_expr\n");
            return NULL;
    }
}
