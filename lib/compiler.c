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

struct wist_parse_result wist_compiler_parse_expr(struct wist_compiler *comp,
        const uint8_t *src, size_t src_len, struct wist_ast_expr **expr_out) {
    IGNORE(expr_out);

    wist_sym_index_init(&comp->syms);
    wist_srcloc_index_init(comp->ctx, &comp->srclocs, src, src_len);

    struct wist_parse_result result = {
        .has_errors = false,
    };
    comp->cur_result = &result;

    size_t tokens_len = 0;
    struct wist_token *tokens = wist_lex(comp, src, src_len, &tokens_len);
    if (tokens == NULL)
    {
        return result;
    }

    struct wist_ast_expr *expr = wist_parse_expr(comp, tokens, tokens_len);

    wist_ast_print_expr(comp, expr);

    *expr_out = expr;

    WIST_CTX_FREE_ARR(comp->ctx, tokens, struct wist_token, tokens_len);

    return result;
}

void wist_parse_result_finish(struct wist_compiler *comp, 
        struct wist_parse_result *result) {
    IGNORE(comp);
    IGNORE(result);
}

/* === PRIVATES === */
