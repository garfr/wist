/* === lib/ast.c - Abstract syntax tree === 
 * Copyright (C) 202tRatcliff - All Rights Reserved
 * Part of the Wist reference implementation, under the MIT license.
 * See LICENSE.txt for license information.
*/

#include <wist/ast.h>
#include <wist/compiler.h>

#include <stdio.h>
#include <inttypes.h>

const char *ast_expr_to_string_map[] = {
    [WIST_AST_EXPR_LAM] = "Lambda",
    [WIST_AST_EXPR_APP] = "Application",
    [WIST_AST_EXPR_VAR] = "Variable",
    [WIST_AST_EXPR_INT] = "Integer",
    [WIST_AST_EXPR_TUPLE] = "Tuple",
};

const char *ast_type_to_string_map[] = {
    [WIST_AST_TYPE_FUN] = "Function",
    [WIST_AST_TYPE_VAR] = "Variable",
    [WIST_AST_TYPE_GEN] = "Generic",
    [WIST_AST_TYPE_TUPLE] = "Tuple",
    [WIST_AST_TYPE_INT] = "Integer",
};

/* === PROTOTYPES === */

static struct wist_ast_expr *wist_ast_create_expr(struct wist_compiler *comp, 
        enum wist_ast_expr_kind t, struct wist_srcloc loc);
static struct wist_ast_type *wist_ast_create_type(struct wist_compiler *comp, 
        enum wist_ast_type_kind t);

static void wist_ast_print_expr_indent(struct wist_compiler *comp, 
        struct wist_ast_expr *expr, int indent);
static void wist_ast_print_type_indent(struct wist_compiler *comp, 
        struct wist_ast_type *type, int indent);

static void wist_ast_scope_destroy(struct wist_compiler *comp, 
        struct wist_ast_scope *scope);

static void wist_ast_type_destroy(struct wist_compiler *comp,
        struct wist_ast_type *type);

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

struct wist_ast_expr *wist_ast_create_int(struct wist_compiler *comp, 
        struct wist_srcloc loc, int64_t i) {
    struct wist_ast_expr *expr = wist_ast_create_expr(comp, WIST_AST_EXPR_INT, loc);
    expr->i.val = i;
    return expr;
}

struct wist_ast_expr *wist_ast_create_tuple(struct wist_compiler *comp, 
        struct wist_srcloc loc, struct wist_vector fields) {
    struct wist_ast_expr *expr = wist_ast_create_expr(comp, WIST_AST_EXPR_TUPLE, loc);
    expr->tuple.fields = fields;
    return expr;
}

struct wist_ast_type *wist_ast_create_fun_type(struct wist_compiler *comp,
        struct wist_ast_type *in, struct wist_ast_type *out) {
    struct wist_ast_type *type = wist_ast_create_type(comp, WIST_AST_TYPE_FUN);
    type->fun.in = in;
    type->fun.out = out;
    return type;
}

struct wist_ast_type *wist_ast_create_tuple_type(struct wist_compiler *comp, 
        struct wist_vector fields) {
    struct wist_ast_type *type = wist_ast_create_type(comp, WIST_AST_TYPE_TUPLE);
    type->tuple.fields = fields;
    return type;
}

struct wist_ast_type *wist_ast_create_gen_type(struct wist_compiler *comp, 
        uint64_t id) {
    struct wist_ast_type *type = wist_ast_create_type(comp, WIST_AST_TYPE_GEN);
    type->gen.id = id;
    return type;
}

struct wist_ast_type *wist_ast_create_var_type(struct wist_compiler *comp) {
    struct wist_ast_type *type = WIST_OBJPOOL_ALLOC(&comp->type_var_pool, struct wist_ast_type);
    type->t = WIST_AST_TYPE_VAR;
    type->var.id = comp->next_type_id++;
    type->var.instance = NULL;
    return type;
}

struct wist_ast_type *wist_ast_create_int_type(struct wist_compiler *comp) {
    return wist_ast_create_type(comp, WIST_AST_TYPE_INT);
}

void wist_ast_print_expr(struct wist_compiler *comp, struct wist_ast_expr *expr) {
    wist_ast_print_expr_indent(comp, expr, 0);
    printf("\n");
}

void wist_ast_print_type(struct wist_compiler *comp, struct wist_ast_type *type) {
    wist_ast_print_type_indent(comp, type, 0);
    printf("\n");
}

struct wist_ast_var_entry *wist_ast_scope_find(struct wist_ast_scope *scope, 
        struct wist_sym *sym) {
    while (scope != NULL) {
        struct wist_ast_var_entry *entry = scope->vars;
        while (entry != NULL) {
            if (entry->sym == sym) {
                return entry;
            }
            entry = entry->next;
        }
        scope = scope->up;
    }
    return NULL;
}

struct wist_ast_scope *wist_ast_scope_push(struct wist_compiler *comp, 
        struct wist_ast_scope *scope) {
    struct wist_ast_scope *new_scope = WIST_CTX_NEW(comp->ctx, struct wist_ast_scope);
    new_scope->up = scope;
    new_scope->vars = NULL;
    return new_scope;
}

struct wist_ast_var_entry *wist_ast_scope_insert(struct wist_compiler *comp, 
        struct wist_ast_scope *scope, struct wist_sym *sym, 
        struct wist_ast_type *type) {
    struct wist_ast_var_entry *new_entry = WIST_CTX_NEW(comp->ctx, 
            struct wist_ast_var_entry);
    new_entry->sym = sym;
    new_entry->type = type;
    new_entry->next = scope->vars;
    scope->vars = new_entry;
    return new_entry;
}

void wist_ast_expr_destroy(struct wist_compiler *comp, 
        struct wist_ast_expr *expr) {
    switch (expr->t) {
        case WIST_AST_EXPR_LAM:
            wist_ast_scope_destroy(comp, expr->lam.scope);
            wist_ast_expr_destroy(comp, expr->lam.body);
            break;
        case WIST_AST_EXPR_APP:
            wist_ast_expr_destroy(comp, expr->app.fun);
            wist_ast_expr_destroy(comp, expr->app.arg);
            break;
        case WIST_AST_EXPR_TUPLE: {
            WIST_VECTOR_FOR_EACH(&expr->tuple.fields, struct wist_ast_expr *, 
                    field) {
                wist_ast_expr_destroy(comp, *field);
            }
            WIST_VECTOR_FINISH(comp->ctx, &expr->tuple.fields);
            break;
        }
        case WIST_AST_EXPR_VAR:
        case WIST_AST_EXPR_INT:
            break;
    }

    wist_ast_type_destroy(comp, expr->type);
    WIST_CTX_FREE(comp->ctx, expr, struct wist_ast_expr);
}

/* === PRIVATES === */


static void wist_ast_type_destroy(struct wist_compiler *comp,
        struct wist_ast_type *type) {
    switch (type->t) {
        case WIST_AST_TYPE_FUN:
            wist_ast_type_destroy(comp, type->fun.in);
            wist_ast_type_destroy(comp, type->fun.out);
            break;
        case WIST_AST_TYPE_TUPLE:
            WIST_VECTOR_FOR_EACH(&type->tuple.fields, struct wist_ast_type *, 
                    field) {
                wist_ast_type_destroy(comp, *field);
            }
            WIST_VECTOR_FINISH(comp->ctx, &type->tuple.fields);
            break;
        case WIST_AST_TYPE_INT:
        case WIST_AST_TYPE_GEN:
            break;
        case WIST_AST_TYPE_VAR:
            return; /* These are not allocated with the rest of the types. */
    }

}

static void wist_ast_scope_destroy(struct wist_compiler *comp, 
        struct wist_ast_scope *scope) {
    struct wist_ast_var_entry *var = scope->vars, *follow = NULL; 
    while (var != NULL)
    {
        follow = var;
        var = var->next;
        WIST_CTX_FREE(comp->ctx, follow, struct wist_ast_var_entry);
    }

    WIST_CTX_FREE(comp->ctx, scope, struct wist_ast_scope);
}

static struct wist_ast_expr *wist_ast_create_expr(struct wist_compiler *comp,
        enum wist_ast_expr_kind t, struct wist_srcloc loc) {
    struct wist_ast_expr *expr = WIST_CTX_NEW(comp->ctx, struct wist_ast_expr);
    expr->t = t;
    expr->loc = loc;
    return expr;
}

static struct wist_ast_type *wist_ast_create_type(struct wist_compiler *comp, 
        enum wist_ast_type_kind t) {
    struct wist_ast_type *type = WIST_OBJPOOL_ALLOC(&comp->type_pool, struct wist_ast_type);
    type->t = t;
    return type;
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
        case WIST_AST_EXPR_INT:
            printf(" : %" PRId64, expr->i.val);
            break;
        case WIST_AST_EXPR_TUPLE: {
            WIST_VECTOR_FOR_EACH(&expr->tuple.fields, struct wist_ast_expr *, field) {
                printf("\n");
                wist_ast_print_expr_indent(comp, *field, indent + 1);
            }
        }
    }

    if (expr->type != NULL) {
        printf("\n");
        wist_ast_print_type_indent(comp, expr->type, indent + 1);
    } else {
        //printf("\n\nEXPR HAS NULL TYPE\n\n");
    }
}

static void wist_ast_print_type_indent(struct wist_compiler *comp, 
        struct wist_ast_type *type, int indent) {
    for (int i = 0; i < indent; i++)
    {
        printf("\t");
    }

    printf("%s", ast_type_to_string_map[type->t]);
    switch (type->t) {
        case WIST_AST_TYPE_VAR:
            printf(" : %" PRId64, type->var.id);
            if (type->var.instance != NULL) {
                printf("\n");
                wist_ast_print_type_indent(comp, type->var.instance, indent + 1);
            }
            break;
        case WIST_AST_TYPE_FUN:
            printf(" : \n");
            wist_ast_print_type_indent(comp, type->fun.in, indent + 1);
            printf("\n");
            wist_ast_print_type_indent(comp, type->fun.out, indent + 1);
            break;
        case WIST_AST_TYPE_TUPLE: {
            printf(" : ");
            WIST_VECTOR_FOR_EACH(&type->tuple.fields, struct wist_ast_type *, 
                    field) {
                printf("\n");
                wist_ast_print_type_indent(comp, *field, indent + 1);
            }
            break;
        }
        case WIST_AST_TYPE_GEN:
            printf(" : '%c", (char) ('a' + type->gen.id));
            break;
        case WIST_AST_TYPE_INT:
            break;
    }
}
