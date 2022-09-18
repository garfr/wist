/* === lib/ast.c - Abstract syntax tree === 
 * Copyright (C) 2022 Gavin Ratcliff - All Rights Reserved
 * Part of the Wist reference implementation, under the MIT license.
 * See LICENSE.txt for license information.
*/

#include <wist/ast.h>
#include <wist/compiler.h>

#include <stdio.h>

const char *ast_expr_to_string_map[] = {
    [WIST_AST_EXPR_LAM] = "Lambda",
    [WIST_AST_EXPR_VAR] = "Variable",
    [WIST_AST_EXPR_APP] = "Application"
};

/* === PROTOTYPES === */

static struct wist_ast_expr *wist_ast_create_expr(struct wist_compiler *comp, 
        enum wist_ast_expr_kind t, struct wist_srcloc loc);

static void wist_ast_print_expr_indent(struct wist_compiler *comp, 
        struct wist_ast_expr *expr, int indent);

/* === PUBLICS === */

struct wist_ast_expr *wist_ast_create_lam(struct wist_compiler *comp, 
        struct wist_srcloc loc, struct wist_sym *sym, 
        struct wist_ast_expr *body) {
    struct wist_ast_expr *expr = wist_ast_create_expr(comp, WIST_AST_EXPR_LAM, loc);
    expr->lam.sym = sym;
    expr->lam.body = body;
    return expr;
}

struct wist_ast_expr *wist_ast_create_app(struct wist_compiler *comp, 
        struct wist_srcloc loc, struct wist_ast_expr *fun, 
        struct wist_ast_expr *arg) {
    struct wist_ast_expr *expr = wist_ast_create_expr(comp, WIST_AST_EXPR_APP, loc);
    expr->app.fun = fun;
    expr->app.arg = arg;
    return expr;
}

struct wist_ast_expr *wist_ast_create_var(struct wist_compiler *comp, 
        struct wist_srcloc loc, struct wist_sym *sym) {
    struct wist_ast_expr *expr = wist_ast_create_expr(comp, WIST_AST_EXPR_VAR, loc);
    expr->var.sym = sym;
    return expr;
}

void wist_ast_print_expr(struct wist_compiler *comp, struct wist_ast_expr *expr) {
    wist_ast_print_expr_indent(comp, expr, 0);
    printf("\n");
}


/* === PRIVATE === */

static struct wist_ast_expr *wist_ast_create_expr(struct wist_compiler *comp,
        enum wist_ast_expr_kind t, struct wist_srcloc loc) {
    struct wist_ast_expr *expr = WIST_CTX_NEW(comp->ctx, struct wist_ast_expr);
    expr->t = t;
    expr->loc = loc;
    return expr;
}

static void wist_ast_print_expr_indent(struct wist_compiler *comp, 
        struct wist_ast_expr *expr, int indent) {
    for (int i = 0; i < indent; i++)
    {
        printf("\t");
    }

    size_t str_len = 0;
    const uint8_t *str = wist_srcloc_index_slice(&comp->srclocs, expr->loc, 
            &str_len);
    printf("%s : '%.*s'", ast_expr_to_string_map[expr->t], (int) str_len, 
            (const char *) str);
    switch (expr->t) {
        case WIST_AST_EXPR_LAM:
            printf(" : '%.*s'\n", (int) expr->lam.sym->str_len, (const char *) 
                    expr->lam.sym->str);
            wist_ast_print_expr_indent(comp, expr->lam.body, indent + 1);
            break;
        case WIST_AST_EXPR_APP:
            printf("\n");
            wist_ast_print_expr_indent(comp, expr->app.fun, indent + 1);
            printf("\n");
            wist_ast_print_expr_indent(comp, expr->app.arg, indent + 1);
            break;
        case WIST_AST_EXPR_VAR:
            printf(" : '%.*s'", (int) expr->var.sym->str_len, (const char *) 
                    expr->var.sym->str);
            break;
    }
}
