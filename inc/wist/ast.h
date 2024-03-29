/* === inc/wist/ast.h - Abstract syntax tree === 
 * Copyright (C) 2022 Gavin Ratcliff - All Rights Reserved
 * Part of the Wist reference implementation, under the MIT license.
 * See LICENSE.txt for license information.
*/

#ifndef _WIST_AST_H
#define _WIST_AST_H

#include <wist.h>
#include <wist/srcloc.h>
#include <wist/sym.h>
#include <wist/vector.h>

struct wist_ast_var_entry {
    struct wist_sym *sym;
    struct wist_ast_type *type;
    struct wist_ast_var_entry *next;
};

struct wist_ast_scope {
    struct wist_ast_scope *up;
    struct wist_ast_var_entry *vars;
};

enum wist_ast_type_kind {
    WIST_AST_TYPE_FUN,
    WIST_AST_TYPE_TUPLE,
    WIST_AST_TYPE_INT,
    WIST_AST_TYPE_GEN,

    /* 
     * Vars are only used during type inference. By the end of type inference 
     * they will be replaced by WIST_AST_TYPE_GENERIC values. 
     */
    WIST_AST_TYPE_VAR,     
};

struct wist_ast_type {
    enum wist_ast_type_kind t;

    union  {
        struct {
            struct wist_ast_type *in;
            struct wist_ast_type *out;
        } fun;

        struct {
            uint64_t id;
            struct wist_ast_type *instance;
        } var;

        struct {
            uint64_t id;
        } gen;

        struct {
            struct wist_vector fields; /* struct wist_ast_type * */
        } tuple;
    };
};

enum wist_ast_expr_kind {
    WIST_AST_EXPR_LAM,
    WIST_AST_EXPR_VAR,
    WIST_AST_EXPR_GVAR,
    WIST_AST_EXPR_APP,
    WIST_AST_EXPR_LET,
    WIST_AST_EXPR_INT,
    WIST_AST_EXPR_TUPLE,
};

struct wist_ast_expr {
    enum wist_ast_expr_kind t;
    /* [type] is not assigned during creation, but during semantic analysis. */
    struct wist_ast_type *type; 
    struct wist_srcloc loc;

    union {
        struct {
            struct wist_sym *sym;
            /* [var] is NULL until semantic analysis is over. */
            struct wist_ast_var_entry *var;
        } var;

        struct {
            struct wist_sym *sym;
            /* [var] is NULL until semantic analysis is over. */
            struct wist_toplvl_entry *var;
        } gvar;

        struct {
            struct wist_ast_expr *body;
            struct wist_sym *sym;
            /* [var] and [scope] are NULL until semantic analysis is over. */
            struct wist_ast_var_entry *var;
            struct wist_ast_scope *scope;
        } lam;

        struct {
            struct wist_ast_expr *fun, *arg;
        } app;

        struct {
            struct wist_sym *sym;
            struct wist_ast_expr *val, *body;

            /* [var] and [scope] are NULL until semantic analysis is over. */
            struct wist_ast_var_entry *var;
            struct wist_ast_scope *scope;
        } let;

        struct {
            int64_t val; /* TODO: Use a bigint to store data larger than integers can. */
        } i;

        struct {
            struct wist_vector fields; /* struct wist_ast_expr * */
        } tuple;
    };
};

enum wist_ast_decl_kind {
    WIST_AST_DECL_BIND,
};

struct wist_ast_decl {
    enum wist_ast_decl_kind t;
    struct wist_srcloc loc;

    union {
        struct {
            struct wist_ast_type *type;
            struct wist_sym *sym;
            struct wist_ast_expr *body;
        } bind;
    };
};

/* === CONSTRUCTORS === */

struct wist_ast_expr *wist_ast_create_lam(struct wist_compiler *comp, 
        struct wist_srcloc loc, struct wist_sym *sym, 
        struct wist_ast_expr *body);

struct wist_ast_expr *wist_ast_create_app(struct wist_compiler *comp, 
        struct wist_srcloc loc, struct wist_ast_expr *fun, 
        struct wist_ast_expr *arg);

struct wist_ast_expr *wist_ast_create_var(struct wist_compiler *comp, 
        struct wist_srcloc loc, struct wist_sym *sym);

struct wist_ast_expr *wist_ast_create_int(struct wist_compiler *comp, 
        struct wist_srcloc loc, int64_t i);

struct wist_ast_expr *wist_ast_create_tuple(struct wist_compiler *comp, 
        struct wist_srcloc loc, struct wist_vector fields);

struct wist_ast_expr *wist_ast_create_let(struct wist_compiler *comp, 
        struct wist_srcloc loc, struct wist_sym *sym, struct wist_ast_expr *val,
        struct wist_ast_expr *body);

struct wist_ast_type *wist_ast_create_fun_type(struct wist_compiler *comp,
        struct wist_ast_type *in, struct wist_ast_type *out);
struct wist_ast_type *wist_ast_create_var_type(struct wist_compiler *comp);
struct wist_ast_type *wist_ast_create_tuple_type(struct wist_compiler *comp, 
        struct wist_vector fields);
struct wist_ast_type *wist_ast_create_gen_type(struct wist_compiler *comp, 
        uint64_t id);
struct wist_ast_type *wist_ast_create_int_type(struct wist_compiler *comp);

struct wist_ast_decl *wist_ast_create_bind(struct wist_compiler *comp,
        struct wist_srcloc src, struct wist_sym *sym, 
        struct wist_ast_expr *body);

/* === PRETTY PRINTING === */

void wist_ast_print_expr(struct wist_compiler *comp, struct wist_ast_expr *expr);
void wist_ast_print_decl(struct wist_compiler *comp, struct wist_ast_decl *decl);
void wist_ast_print_type(struct wist_compiler *comp, struct wist_ast_type *type);

/* === SCOPES === */

struct wist_ast_var_entry *wist_ast_scope_find(struct wist_ast_scope *scope, struct wist_sym *sym);
struct wist_ast_scope *wist_ast_scope_push(struct wist_compiler *comp, struct wist_ast_scope *scope);
struct wist_ast_var_entry *wist_ast_scope_insert(struct wist_compiler *comp, 
        struct wist_ast_scope *scope, struct wist_sym *sym, 
        struct wist_ast_type *type);

#endif /* _WIST_AST_H */
