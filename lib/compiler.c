/* === lib/compiler.h - Wist compiler === 
 * Copyright (C) 2022 Gavin Ratcliff - All Rights Reserved
 * Part of the Wist reference implementation, under the MIT license.
 * See LICENSE.txt for license information.
*/

#include <wist/defs.h>
#include <wist/compiler.h>
#include <wist/lexer.h>
#include <wist/parser.h>
#include <wist/ast.h>

/* === PROTOTYPES === */

/* === PUBLICS === */

struct wist_compiler *wist_compiler_create(struct wist_ctx *ctx) {
    struct wist_compiler *comp = WIST_CTX_NEW(ctx, struct wist_compiler);

    comp->ctx = ctx;

    return comp;
}

void wist_compiler_destroy(struct wist_compiler *comp) {
    if (comp == NULL) {
        return;
    }

    wist_sym_index_finish(comp->ctx, &comp->syms);
    wist_srcloc_index_finish(comp->ctx, &comp->srclocs);

    WIST_CTX_FREE(comp->ctx, comp, struct wist_compiler);
}

struct wist_parse_result *wist_compiler_parse_expr(struct wist_compiler *comp,
        const uint8_t *src, size_t src_len, struct wist_ast_expr **expr_out) {
    IGNORE(expr_out);

    wist_sym_index_init(&comp->syms);
    wist_srcloc_index_init(comp->ctx, &comp->srclocs, src, src_len);

    struct wist_parse_result *result = WIST_CTX_NEW(comp->ctx, struct wist_parse_result);
    result->has_errors = false;
    WIST_VECTOR_INIT(comp->ctx, &result->diags, struct wist_diag);

    comp->cur_result = result;

    size_t tokens_len = 0;
    struct wist_token *tokens = wist_lex(comp, src, src_len, &tokens_len);
    if (tokens == NULL)
    {
        return result;
    }

    struct wist_ast_expr *expr = wist_parse_expr(comp, tokens, tokens_len);
    if (expr == NULL)
    {
        goto cleanup;
    }

    wist_ast_print_expr(comp, expr);

    *expr_out = expr;

cleanup:
    WIST_CTX_FREE_ARR(comp->ctx, tokens, struct wist_token, tokens_len);

    return result;
}

void wist_parse_result_destroy(struct wist_compiler *comp, 
        struct wist_parse_result *result) {
    WIST_VECTOR_FOR_EACH(&result->diags, struct wist_diag, diag) {
        WIST_VECTOR_FINISH(comp->ctx, &diag->locs);
    }

    WIST_VECTOR_FINISH(comp->ctx, &result->diags);
    WIST_CTX_FREE(comp->ctx, result, struct wist_parse_result);
}

bool wist_parse_result_has_errors(struct wist_parse_result *result) {
    return result->has_errors;
}

struct wist_diag *wist_compiler_add_diag(struct wist_compiler *comp, 
        enum wist_diag_kind t, enum wist_diag_level level) {
    struct wist_diag *diag = WIST_VECTOR_PUSH_UNINIT(comp->ctx, 
            &comp->cur_result->diags, struct wist_diag);

    if (level == WIST_DIAG_ERROR || level == WIST_DIAG_FATAL) {
        comp->cur_result->has_errors = true;
    }

    diag->t = t;
    diag->level = level;
    WIST_VECTOR_INIT(comp->ctx, &diag->locs, struct wist_diag);

    return diag;
}

void wist_diag_add_loc(struct wist_compiler *comp, struct wist_diag *diag, 
        struct wist_srcloc loc) {
    WIST_VECTOR_PUSH(comp->ctx, &diag->locs, struct wist_srcloc, &loc);
}

void wist_ast_expr_destroy(struct wist_compiler *comp, 
        struct wist_ast_expr *expr) {
    switch (expr->t) {
        case WIST_AST_EXPR_LAM:
            wist_ast_expr_destroy(comp, expr->lam.body);
            break;
        case WIST_AST_EXPR_APP:
            wist_ast_expr_destroy(comp, expr->app.fun);
            wist_ast_expr_destroy(comp, expr->app.arg);
            break;
        case WIST_AST_EXPR_VAR:
            break;
    }

    WIST_CTX_FREE(comp->ctx, expr, struct wist_ast_expr);
}

/* === PRIVATES === */
