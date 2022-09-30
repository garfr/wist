/* === lib/parser.c - Wist parser === 
 * Copyright (C) 2022 Gavin Ratcliff - All Rights Reserved
 * Part of the Wist reference implementation, under the MIT license.
 * See LICENSE.txt for license information.
*/

#include <wist/parser.h>
#include <wist/compiler.h>
#include <wist/ast.h>

#include <stdio.h>
#include <inttypes.h>

struct wist_parser {
    struct wist_compiler *comp;
    struct wist_token *tokens;
    size_t tokens_len, cur_tok;
};

#define PEEK_TOK(_parser) ((_parser)->tokens[(_parser)->cur_tok])
#define SKIP_TOK(_parser) ((_parser)->cur_tok++)
#define NEXT_TOK(_parser) ((_parser)->tokens[(_parser)->cur_tok++])

/* === PROTOTYPES === */

/* Parsing helpers*/
static bool expect(struct wist_parser *parser, struct wist_token *tok, 
        enum wist_token_kind t);

/* Each of these parse_* expressions represents a terminal in the grammar. */
static struct wist_ast_expr *parse_expr(struct wist_parser *parser);
static struct wist_ast_expr *parse_lam_expr(struct wist_parser *parser);
static struct wist_ast_expr *parse_app_expr(struct wist_parser *parser);
static struct wist_ast_expr *parse_atomic_expr(struct wist_parser *parser);
static struct wist_ast_expr *parse_tuple(struct wist_parser *parser, 
        struct wist_token start_tok, struct wist_ast_expr *first_val);
/* 
 * parse_*_maybe variants work the same as the parse_* version but they will 
 * report an error if the terminal is not matched. 
 */
static struct wist_ast_expr *parse_atomic_expr_maybe(struct wist_parser *parser);

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

static bool expect(struct wist_parser *parser, struct wist_token *tok_out, 
        enum wist_token_kind t) {
    struct wist_token tok = NEXT_TOK(parser);
    if (tok_out != NULL)
    {
        *tok_out = tok;
    }

    if (tok.t != t) {
        struct wist_diag *diag = wist_compiler_add_diag(parser->comp, 
                WIST_DIAG_EXPECTED_TOKEN, WIST_DIAG_ERROR);
        wist_diag_add_loc(parser->comp, diag, tok.loc);
        diag->expected_token = t;
        return false;
    } 

    return true;
}

static struct wist_ast_expr *parse_expr(struct wist_parser *parser) {
    return parse_lam_expr(parser);
}

static struct wist_ast_expr *parse_lam_expr(struct wist_parser *parser) {
    struct wist_token bs_tok = PEEK_TOK(parser);
    if (bs_tok.t == WIST_TOKEN_BACKSLASH) {
        SKIP_TOK(parser);
        struct wist_token sym_tok;

        struct wist_sym *sym;
        if (!expect(parser, &sym_tok, WIST_TOKEN_SYM))
        {
            sym = NULL;
        } else {
            sym = sym_tok.sym;
        }

        expect(parser, NULL, WIST_TOKEN_THIN_ARROW); 
        struct wist_ast_expr *body = parse_expr(parser);

        struct wist_srcloc full_loc = 
            wist_srcloc_index_combine(parser->comp->ctx,  
                    &parser->comp->srclocs, bs_tok.loc, body->loc);
        return wist_ast_create_lam(parser->comp, full_loc, sym, body);
    }
    return parse_app_expr(parser);
}

static struct wist_ast_expr *parse_app_expr(struct wist_parser *parser) {
    struct wist_ast_expr *arg, *full = parse_atomic_expr(parser);
    while ((arg = parse_atomic_expr_maybe(parser)) != NULL) {
        struct wist_srcloc full_loc = 
            wist_srcloc_index_combine(parser->comp->ctx, 
                    &parser->comp->srclocs, full->loc, arg->loc);
        struct wist_ast_expr *new = wist_ast_create_app(parser->comp, full_loc, full, arg);
        full = new;
    }
    return full;
}

static struct wist_ast_expr *parse_tuple(struct wist_parser *parser, 
        struct wist_token start_tok, struct wist_ast_expr *first_val) {
    struct wist_vector fields;
    WIST_VECTOR_INIT(parser->comp->ctx, &fields, struct wist_ast_expr *);

    WIST_VECTOR_PUSH(parser->comp->ctx, &fields, struct wist_ast_expr *, 
            &first_val);

    while (PEEK_TOK(parser).t == WIST_TOKEN_COMMA) {
        SKIP_TOK(parser);
        struct wist_ast_expr *field = parse_atomic_expr(parser);
        WIST_VECTOR_PUSH(parser->comp->ctx, &fields, struct wist_ast_expr *, 
                &field);
    }
    
    struct wist_token end_tok;
    expect(parser, &end_tok, WIST_TOKEN_RPAREN);

    struct wist_srcloc full_loc = wist_srcloc_index_combine(parser->comp->ctx, 
            &parser->comp->srclocs, start_tok.loc, end_tok.loc);

    return wist_ast_create_tuple(parser->comp, full_loc, fields);
}

static struct wist_ast_expr *parse_atomic_expr_maybe(struct wist_parser *parser) {
    struct wist_token tok = PEEK_TOK(parser);

    switch (tok.t) {
        case WIST_TOKEN_LPAREN: {
            struct wist_token start_tok = NEXT_TOK(parser);
            struct wist_ast_expr *expr = parse_expr(parser);
            if (PEEK_TOK(parser).t == WIST_TOKEN_COMMA) {
                return parse_tuple(parser, start_tok, expr);
            }
            struct wist_token end_tok = NEXT_TOK(parser); 
            if (expr != NULL) {
                expr->loc = wist_srcloc_index_combine(parser->comp->ctx, 
                        &parser->comp->srclocs, start_tok.loc, end_tok.loc);
            }
            return expr;
        }
        case WIST_TOKEN_SYM:
            SKIP_TOK(parser);
            return wist_ast_create_var(parser->comp, tok.loc, tok.sym);
        case WIST_TOKEN_INT:
            SKIP_TOK(parser);
            return wist_ast_create_int(parser->comp, tok.loc, tok.i);
        case WIST_TOKEN_EOI:
        case WIST_TOKEN_RPAREN:
        case WIST_TOKEN_COMMA:
            return NULL;
        default: {
            struct wist_diag *diag = wist_compiler_add_diag(parser->comp, 
                    WIST_DIAG_EXPECTED_EXPR, WIST_DIAG_ERROR);
            diag->expected_expr = tok;
            wist_diag_add_loc(parser->comp, diag, tok.loc);
            return NULL;
        }
    }
}

static struct wist_ast_expr *parse_atomic_expr(struct wist_parser *parser) {
    struct wist_ast_expr *expr = parse_atomic_expr_maybe(parser);
    if (expr == NULL) {
        struct wist_token tok = PEEK_TOK(parser);
        struct wist_diag *diag = wist_compiler_add_diag(parser->comp, 
                WIST_DIAG_EXPECTED_EXPR, WIST_DIAG_ERROR);
        diag->expected_expr = PEEK_TOK(parser);
        wist_diag_add_loc(parser->comp, diag, tok.loc);
    }

    return expr;
}
