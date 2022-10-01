/* === inc/wist/lir.h - Lambda intermediate representation === 
 * Copyright (C) 2022 Gavin Ratcliff - All Rights Reserved
 * Part of the Wist reference implementation, under the MIT license.
 * See LICENSE.txt for license information.
*/

#ifndef _WIST_LIR_H
#define _WIST_LIR_H

#include <wist.h>
#include <wist/defs.h>
#include <wist/vector.h>

enum wist_lir_expr_kind {
    WIST_LIR_EXPR_LAM,
    WIST_LIR_EXPR_APP,
    WIST_LIR_EXPR_VAR,
    WIST_LIR_EXPR_LET,
    WIST_LIR_EXPR_INT,
    WIST_LIR_EXPR_MKB,
};

enum wist_lir_block_kind {
    WIST_LIR_BLOCK_TUPLE,
};

struct wist_lir_expr {
    enum wist_lir_expr_kind t;

    union {
        struct {
            struct wist_lir_expr *fun, *arg;
        } app;

        struct {
            struct wist_lir_expr *body;
        } lam;

        struct {
            struct wist_lir_expr *body;
            struct wist_lir_expr *val;
        } let;

        struct {
            struct wist_lir_expr *origin;
            int index;
        } var;

        struct {
            enum wist_lir_block_kind t;
            struct wist_vector fields; /* struct wist_lir_expr * */
        } mkb;

        struct {
            int64_t val;
        } i;
    };
};

void wist_lir_expr_destroy(struct wist_compiler *comp,
        struct wist_lir_expr *expr);

/* === CONSTRUCTORS === */

struct wist_lir_expr *wist_lir_create_app(struct wist_compiler *comp, 
        struct wist_lir_expr *fun, struct wist_lir_expr *arg);
struct wist_lir_expr *wist_lir_create_lam(struct wist_compiler *comp,
        struct wist_lir_expr *body);
struct wist_lir_expr *wist_lir_create_let(struct wist_compiler *comp,
        struct wist_lir_expr *val, struct wist_lir_expr *body);
struct wist_lir_expr *wist_lir_create_var(struct wist_compiler *comp,
        int index, struct wist_lir_expr *origin);
struct wist_lir_expr *wist_lir_create_mkb(struct wist_compiler *comp,
        enum wist_lir_block_kind t, struct wist_vector mkb);
struct wist_lir_expr *wist_lir_create_int(struct wist_compiler *comp, 
        int64_t i);

/* === LIR GENERATION === */

struct wist_lir_expr *wist_compiler_lir_gen_expr(struct wist_compiler *comp, 
        struct wist_ast_expr *expr);

/* === PRETTY PRINTING === */

void wist_lir_print_expr(struct wist_lir_expr *expr);

#endif /* _WIST_LIR_H */
