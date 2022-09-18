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

enum wist_ast_expr_kind {
    WIST_AST_EXPR_LAM,
    WIST_AST_EXPR_VAR,
    WIST_AST_EXPR_APP,
};

struct wist_ast_expr {
    enum wist_ast_expr_kind t;
    struct wist_srcloc loc;

    union {
        struct {
            struct wist_sym *sym;
        } var;

        struct {
            struct wist_ast_expr *body;
            struct wist_sym *sym;
        } lam;

         struct {
             struct wist_ast_expr *fun, *arg;
         } app;
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

/* === PRETTY PRINTING === */

void wist_ast_print_expr(struct wist_compiler *comp, struct wist_ast_expr *expr);

#endif /* _WIST_AST_H */
