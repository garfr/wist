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
#include <wist/sema.h>
#include <wist/vm.h>

#include <stdio.h>

/* === PROTOTYPES === */

/* === PUBLICS === */

struct wist_compiler *wist_compiler_create(struct wist_ctx *ctx) {
    struct wist_compiler *comp = WIST_CTX_NEW(ctx, struct wist_compiler);

    comp->globals = NULL;
    comp->ctx = ctx;
    wist_toplvl_init(ctx, &comp->toplvl);
    wist_sym_index_init(comp->ctx, &comp->syms);
    wist_srcloc_index_init(comp->ctx, &comp->srclocs);
    WIST_OBJPOOL_INIT(comp->ctx, &comp->type_pool, struct wist_ast_type);

    return comp;
}

void wist_compiler_destroy(struct wist_compiler *comp) {
    if (comp == NULL) {
        return;
    }

    wist_objpool_finish(&comp->type_pool);
    wist_sym_index_finish(comp->ctx, &comp->syms);
    wist_srcloc_index_finish(comp->ctx, &comp->srclocs);
    wist_toplvl_finish(&comp->toplvl);

    WIST_CTX_FREE(comp->ctx, comp, struct wist_compiler);
}

struct wist_parse_result *wist_compiler_parse_decl(struct wist_compiler *comp,
        const uint8_t *src, size_t src_len, struct wist_ast_decl **decl_out) {
    struct wist_parse_result *result = WIST_CTX_NEW(comp->ctx, struct wist_parse_result);
    wist_srcloc_index_add_segment(comp->ctx, &comp->srclocs, src, src_len);
    result->has_errors = false;
    WIST_VECTOR_INIT(comp->ctx, &result->diags, struct wist_diag);
    comp->cur_result = result;

    size_t tokens_len = 0;
    struct wist_token *tokens = wist_lex(comp, src, src_len, &tokens_len);
    if (tokens == NULL)
    {
        return result;
    }

    struct wist_ast_decl *decl = wist_parse_decl(comp, tokens, tokens_len);

    if (!wist_sema_infer_decl(comp, decl)) {
        return result;
    }

    wist_ast_print_decl(comp, decl);

    *decl_out = decl;

    return result;
}

void wist_compiler_vm_connect(struct wist_compiler *comp, struct wist_vm *vm) {
    vm->toplvl = &comp->toplvl;
}

struct wist_parse_result *wist_compiler_parse_expr(struct wist_compiler *comp,
        const uint8_t *src, size_t src_len, struct wist_ast_expr **expr_out) {
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

    if (wist_parse_result_has_errors(result)) {
        return result;
    }

    if (!wist_sema_infer_expr(comp, expr)) {
        return result;
    }

    wist_ast_print_expr(comp, expr);

    if (wist_parse_result_has_errors(result)) {
    }

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

/* === PRIVATES === */
