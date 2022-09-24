/* === lib/lir.c - Lambda intermediate representation ===
 * Copyright (C) 2022 Gavin Ratcliff - All Rights Reserved
 * Part of the Wist reference implementation, under the MIT license.
 * See LICENSE.txt for license information.
*/

#include <wist/lir.h>
#include <wist/ast.h>
#include <wist/compiler.h>

#include <stdio.h>
#include <inttypes.h>

struct lam_map {
    struct wist_ast_var_entry *var;
    struct wist_lir_expr *origin;
    struct lam_map *next;
};

const char *lir_expr_to_string_map[] = {
    [WIST_LIR_EXPR_LAM] = "Lambda",
    [WIST_LIR_EXPR_APP] = "Application",
    [WIST_LIR_EXPR_VAR] = "Variable",
    [WIST_LIR_EXPR_INT] = "Integer",
};

/* === PROTOTYPES === */

static struct wist_lir_expr *wist_lir_create_expr(struct wist_compiler *comp,
        enum wist_lir_expr_kind t);
static struct wist_lir_expr *gen_expr_rec(struct wist_compiler *comp, 
        struct wist_ast_expr *expr, struct lam_map *map);

static void print_expr_indent(struct wist_lir_expr *expr, int indent);

/* === PUBLICS === */

void wist_lir_expr_destroy(struct wist_compiler *comp,
        struct wist_lir_expr *expr) {
    switch (expr->t) {
        case WIST_LIR_EXPR_LAM: 
            wist_lir_expr_destroy(comp, expr->lam.body);
            break;
        case WIST_LIR_EXPR_APP: 
            wist_lir_expr_destroy(comp, expr->app.fun);
            wist_lir_expr_destroy(comp, expr->app.arg);
            break;
        case WIST_LIR_EXPR_VAR:
        case WIST_LIR_EXPR_INT:
            break;
    }

    WIST_CTX_FREE(comp->ctx, expr, struct wist_lir_expr);
}

struct wist_lir_expr *wist_lir_create_app(struct wist_compiler *comp, 
        struct wist_lir_expr *fun, struct wist_lir_expr *arg) {
    struct wist_lir_expr *expr = wist_lir_create_expr(comp, WIST_LIR_EXPR_APP);
    expr->app.fun = fun;
    expr->app.arg = arg;
    return expr;
}

struct wist_lir_expr *wist_lir_create_lam(struct wist_compiler *comp,
        struct wist_lir_expr *body) {
    struct wist_lir_expr *expr = wist_lir_create_expr(comp, WIST_LIR_EXPR_LAM);
    expr->lam.body = body;
    return expr;
}

struct wist_lir_expr *wist_lir_create_var(struct wist_compiler *comp,
        struct wist_lir_expr *origin) {
    struct wist_lir_expr *expr = wist_lir_create_expr(comp, WIST_LIR_EXPR_VAR);
    expr->var.origin = origin;
    return expr;
}

struct wist_lir_expr *wist_lir_create_int(struct wist_compiler *comp, 
        int64_t i) {
    struct wist_lir_expr *expr = wist_lir_create_expr(comp, WIST_LIR_EXPR_INT);
    expr->i.val = i;
    return expr;
}

struct wist_lir_expr *wist_compiler_lir_gen_expr(struct wist_compiler *comp, 
        struct wist_ast_expr *expr) {
    struct lam_map *map = NULL;
    return gen_expr_rec(comp, expr, map);
}

void wist_lir_print_expr(struct wist_lir_expr *expr) {
    print_expr_indent(expr, 0);
    printf("\n");
}

/* === PRIVATES === */

static struct wist_lir_expr *wist_lir_create_expr(struct wist_compiler *comp,
        enum wist_lir_expr_kind t) {
    struct wist_lir_expr *expr = WIST_CTX_NEW(comp->ctx, struct wist_lir_expr);
    expr->t = t;
    return expr;
}

static struct wist_lir_expr *gen_expr_rec(struct wist_compiler *comp, 
        struct wist_ast_expr *expr, struct lam_map *map) {
    switch (expr->t) {
        case WIST_AST_EXPR_APP: {
            struct wist_lir_expr *fun = 
                gen_expr_rec(comp, expr->app.fun, map);
            struct wist_lir_expr *arg = 
                gen_expr_rec(comp, expr->app.arg, map);
            return wist_lir_create_app(comp, fun, arg);
        }
        case WIST_AST_EXPR_LAM: {
            struct lam_map *new_map = WIST_CTX_NEW(comp->ctx, struct lam_map);
            new_map->next = map;
            new_map->var = expr->lam.var;
            struct wist_lir_expr *lir_expr = wist_lir_create_lam(comp, NULL);
            new_map->origin = lir_expr;
            struct wist_lir_expr *body = 
                gen_expr_rec(comp, expr->lam.body, new_map);
            WIST_CTX_FREE(comp->ctx, new_map, struct lam_map);
            lir_expr->lam.body = body;
            return lir_expr;
        }
        case WIST_AST_EXPR_VAR: {
            while (map != NULL) {
                if (expr->var.var == map->var) {
                    return wist_lir_create_var(comp, map->origin);
                }
                map = map->next;
            }
            printf("Impossible case of no binding for lambda in gen_expr_rec\n");
            return NULL;
        }
        case WIST_AST_EXPR_INT:
            return wist_lir_create_int(comp, expr->i.val);
    }

    printf("Invalid case of wist_compiler_lir_gen_expr\n");
    return NULL;
}

static void print_expr_indent(struct wist_lir_expr *expr, int indent) {
    for (int i = 0; i < indent; i++) {
        printf("\t");
    }

    printf("%s", lir_expr_to_string_map[expr->t]);
    switch (expr->t) {
        case WIST_LIR_EXPR_LAM: 
            printf(" : %p\n", expr);
            print_expr_indent(expr->lam.body, indent + 1);
            break;
        case WIST_LIR_EXPR_APP: 
            printf("\n");
            print_expr_indent(expr->app.fun, indent + 1);
            printf("\n");
            print_expr_indent(expr->app.arg, indent + 1);
            break;
        case WIST_LIR_EXPR_VAR:
            printf(" : %p", expr->var.origin);
            break;
        case WIST_LIR_EXPR_INT:
            printf(" : %" PRId64, expr->i.val);
            break;
    }
}
