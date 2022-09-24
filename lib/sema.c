/* === lib/sema.c - AST semantics analysis === 
 * Copyright (C) 2022 Gavin Ratcliff - All Rights Reserved
 * Part of the Wist reference implementation, under the MIT license.
 * See LICENSE.txt for license information.
*/

#include <wist/sema.h>
#include <wist/compiler.h>
#include <wist/ast.h>

#include <stdio.h>

/* There seem to be many implementations of HM type inference using this style 
 * of an algorithm (Algorithm W), down to the function names, order of steps 
 * in those functions, etc.  I am not sure what the original implementation 
 * that looked like this was, but the one I referenced when implementing 
 * the initial type inference code was: 
 * https://gist.github.com/louthy/bafb2b8b5701c0842ca405c638b58e80. 
 * So credit to them.
 */

/* 
 * Use to maintain a list of types that are not generic in the current 
 * scope. 
 */
struct type_chain {
    struct wist_ast_type *type;
    struct type_chain *next;
};

/* Maps from one type to another for creating a "fresh" type. */
struct type_type_map {
    struct wist_ast_type *key, *val;
    struct type_type_map *next;
};

/*
 * We pass over all the types and map non-instance type vars to generic 
 * types. 
 */
struct type_var_renamer {
    uint64_t *bindings;
    size_t next_id;
};

/* === PROTOTYPES === */

static struct wist_ast_type *infer_expr_rec(struct wist_compiler *comp,
        struct wist_ast_scope *scope, struct wist_ast_expr *expr, 
        struct type_chain *non_generics);
static struct wist_ast_type *fresh_type(struct wist_compiler *comp, 
        struct wist_ast_type *type, struct type_chain *non_generics);
static struct wist_ast_type *fresh_type_rec(struct wist_compiler *comp, 
        struct wist_ast_type *type, struct type_chain *non_generics,
        struct type_type_map *mappings);
static struct type_type_map *type_type_map_find(struct type_type_map *map, 
        struct wist_ast_type *type);
static struct wist_ast_type *prune(struct wist_compiler *comp, 
        struct wist_ast_type *type);
static void unify(struct wist_compiler *comp, struct wist_ast_type *t1, 
        struct wist_ast_type *t2);

/* Repairs and cleanup for our post-inference AST. */
static struct wist_ast_type *prune_full_type(struct wist_compiler *comp,
        struct wist_ast_type *type, struct type_var_renamer *renamer);
static void prune_full_expr(struct wist_compiler *comp,
        struct wist_ast_expr *expr, struct type_var_renamer *renamer);

/* Type variable renaming. */
static void type_var_renamer_init(struct wist_compiler *comp, 
        struct type_var_renamer *renamer);
static void type_var_renamer_finish(struct wist_compiler *comp, 
        struct type_var_renamer *renamer);
static uint64_t type_var_renamer_rename(struct type_var_renamer *rename, 
        uint64_t id);

/* === PUBLICS === */

void wist_sema_infer_expr(struct wist_compiler *comp, 
        struct wist_ast_scope *scope, struct wist_ast_expr *expr) {
    struct type_var_renamer renamer;
    /* Make sure our type variables start at 0 again. */

    comp->next_type_id = 0;
    WIST_OBJPOOL_INIT(comp->ctx, &comp->type_var_pool, struct wist_ast_type);
    infer_expr_rec(comp, scope, expr, NULL);

    type_var_renamer_init(comp, &renamer);
    prune_full_expr(comp, expr, &renamer);

    type_var_renamer_finish(comp, &renamer); 

    /* Release all the type variables we just pruned/renamed. */
    wist_objpool_finish(&comp->type_var_pool);
}

/* === PRIVATES === */

static struct wist_ast_type *infer_expr_rec(struct wist_compiler *comp,
        struct wist_ast_scope *scope, struct wist_ast_expr *expr, 
        struct type_chain *non_generics) {
    switch (expr->t) {
        case WIST_AST_EXPR_VAR: {
            struct wist_ast_var_entry *entry = wist_ast_scope_find(scope, expr->var.sym);
            expr->var.var = entry;
            expr->type = fresh_type(comp, entry->type, non_generics);
            break;
        }
        case WIST_AST_EXPR_LAM: {
            struct wist_ast_type *arg_ty = wist_ast_create_var_type(comp);
            struct wist_ast_scope *new_scope = wist_ast_scope_push(comp, scope);
            struct wist_ast_var_entry *entry = wist_ast_scope_insert(comp, 
                    new_scope, expr->lam.sym, arg_ty);

            expr->lam.scope = new_scope;
            expr->lam.var = entry;
            struct type_chain *new_non_generics = WIST_CTX_NEW(comp->ctx, 
                    struct type_chain);
            new_non_generics->next = non_generics;
            new_non_generics->type = arg_ty;
            struct wist_ast_type *ret_ty = infer_expr_rec(comp, new_scope,
                    expr->lam.body, new_non_generics);

            WIST_CTX_FREE(comp->ctx, new_non_generics, struct type_chain);
            expr->type = wist_ast_create_fun_type(comp, arg_ty, ret_ty);
            break;
        }
        case WIST_AST_EXPR_APP: {
            struct wist_ast_type *fun_type = infer_expr_rec(comp, scope, 
                    expr->app.fun, non_generics);
            struct wist_ast_type *arg_type = infer_expr_rec(comp, scope,
                    expr->app.arg, non_generics);
            struct wist_ast_type *ret_type = wist_ast_create_var_type(comp);
            unify(comp, wist_ast_create_fun_type(comp, arg_type, ret_type), fun_type);
            expr->type = ret_type;
            break;
        }
        case WIST_AST_EXPR_INT: {
            expr->type = wist_ast_create_int_type(comp);
            break;
        }
        default:
            printf("Invalid case in infer_expr_rec\n");
            return NULL;

    }
    return expr->type;
};

static struct wist_ast_type *fresh_type(struct wist_compiler *comp, 
        struct wist_ast_type *type, struct type_chain *non_generics) {
    struct type_type_map *mappings = NULL;
    return fresh_type_rec(comp, type, non_generics, mappings);

}

static bool type_eq(struct wist_ast_type *t1, struct wist_ast_type *t2) {
    if (t1->t != t2->t) {
        return false;
    }

    switch (t1->t) {
        case WIST_AST_TYPE_VAR:
            return t1->var.id == t2->var.id;
        case WIST_AST_TYPE_FUN:
            return type_eq(t1->fun.in, t2->fun.in) 
                && type_eq(t1->fun.out, t2->fun.out);
        case WIST_AST_TYPE_INT:
        case WIST_AST_TYPE_GEN:
            return true;
    }

    printf("Invalid case in type_eq.\n");
    return NULL;
}

static struct type_type_map *type_type_map_find(struct type_type_map *map, 
        struct wist_ast_type *type) {
    while (map != NULL) {
        if (type_eq(map->key, type)) {
            return map;
        }
        map = map->next;
    }
    return NULL;
}

static bool is_generic(struct wist_ast_type *type, 
        struct type_chain *non_generics) {
    while (non_generics != NULL) {
        if (type_eq(non_generics->type, type)) {
            return false;
        }
        non_generics = non_generics->next;
    }
    return true;
}

static struct wist_ast_type *fresh_type_rec(struct wist_compiler *comp, 
        struct wist_ast_type *_type, struct type_chain *non_generics,
        struct type_type_map *mappings) {
    struct wist_ast_type *type = prune(comp, _type);
    switch (type->t) {
        case WIST_AST_TYPE_VAR: {
            if (is_generic(type, non_generics)) {
                struct type_type_map *find = type_type_map_find(mappings, type);
                if (find == NULL) {
                    struct wist_ast_type *new_type = 
                        wist_ast_create_var_type(comp);
                    struct type_type_map *new_mappings = 
                        WIST_CTX_NEW(comp->ctx, struct type_type_map);
                    new_mappings->next = mappings;
                    new_mappings->key = type;
                    new_mappings->val = new_type;
                    mappings = new_mappings;
                    return new_type;
                } else {
                    return find->val;
                }
            } else {
                return type;
            }
        }
        case WIST_AST_TYPE_FUN: {
            return wist_ast_create_fun_type(comp, type->fun.in, type->fun.out);
        }
        case WIST_AST_TYPE_INT: {
            return wist_ast_create_int_type(comp);
        }
        case WIST_AST_TYPE_GEN:
            break; /* These are created after type inference. */
    }

    printf("invalid case in fresh_type_rec.\n");
    return NULL;
}

static struct wist_ast_type *prune(struct wist_compiler *comp, 
        struct wist_ast_type *type) {
    if (type->t == WIST_AST_TYPE_VAR && type->var.instance != NULL) {
        struct wist_ast_type *new_instance = prune(comp, type->var.instance);
        type->var.instance = new_instance;
        return new_instance;
    }
    return type;
}

static void unify(struct wist_compiler *comp, struct wist_ast_type *_t1, 
        struct wist_ast_type *_t2) {
    struct wist_ast_type *t1 = prune(comp, _t1);
    struct wist_ast_type *t2 = prune(comp, _t2);
    if (t1->t == WIST_AST_TYPE_VAR) {
        if (!type_eq(t1, t2)) {
            t1->var.instance = t2;
        }
        return;
    } else if (t2->t == WIST_AST_TYPE_VAR) {
        return unify(comp, t2, t1);
    } 
    if (t1->t != t2->t) {
        printf("type mismatch!\n");
        return;
    }
    switch (t1->t) {
        case WIST_AST_TYPE_FUN: {
            unify(comp, t1->fun.in, t2->fun.in);
            unify(comp, t1->fun.out, t2->fun.out);
            break;
        }
        case WIST_AST_TYPE_INT:
            break;
        case WIST_AST_TYPE_GEN: 
            break; /* Impossible, added after type inference. */
        case WIST_AST_TYPE_VAR: 
            break; /* Impossible, we checked above. */
    }
}

static struct wist_ast_type *prune_full_type(struct wist_compiler *comp,
        struct wist_ast_type *type, struct type_var_renamer *renamer) {
    type = prune(comp, type);

    switch (type->t) {
        case WIST_AST_TYPE_FUN: {
            type->fun.in = prune_full_type(comp, type->fun.in, renamer);
            type->fun.out = prune_full_type(comp, type->fun.out, renamer);
            break;
        }
        case WIST_AST_TYPE_VAR: {
            int64_t new_id = type_var_renamer_rename(renamer, type->var.id);
            type = wist_ast_create_gen_type(comp, new_id);
        }
        case WIST_AST_TYPE_INT:
        case WIST_AST_TYPE_GEN:
            break;
    }

    return type;
}

static void prune_full_expr(struct wist_compiler *comp,
        struct wist_ast_expr *expr, struct type_var_renamer *renamer) {
    expr->type = prune_full_type(comp, expr->type, renamer);
    switch (expr->t) {
        case WIST_AST_EXPR_LAM: 
            prune_full_expr(comp, expr->lam.body, renamer);
            break;
        case WIST_AST_EXPR_APP: 
            prune_full_expr(comp, expr->app.fun, renamer);
            prune_full_expr(comp, expr->app.arg, renamer);
            break;
        case WIST_AST_EXPR_VAR:
        case WIST_AST_EXPR_INT:
            break;
    }
}

static void type_var_renamer_init(struct wist_compiler *comp, 
        struct type_var_renamer *renamer) {
    if (comp->next_type_id == 0) {
        /* We will never call type_var_rename because there are no type vars. */
        return; 
    }
    renamer->bindings = WIST_CTX_NEW_ARR(comp->ctx, uint64_t, comp->next_type_id);
    renamer->next_id = 0;
}

static void type_var_renamer_finish(struct wist_compiler *comp, 
        struct type_var_renamer *renamer) {
    if (renamer->bindings == NULL) {
        return; /* We never initialized because there were no type vars. */
    }

    WIST_CTX_FREE_ARR(comp->ctx, renamer->bindings, uint64_t,  comp->next_type_id);
    renamer->bindings = NULL;
    renamer->next_id = 0;
}

static uint64_t type_var_renamer_rename(struct type_var_renamer *rename, 
        uint64_t id) {
    /* 0 means that it is unassigned to a binding, so bindings start at 1. */
    if (rename->bindings[id] == 0) {
        rename->bindings[id] = ++rename->next_id;
    }
    return rename->bindings[id] - 1;
}
