/* === inc/wist/compiler.h - Wist compiler === 
 * Copyright (C) 2022 Gavin Ratcliff - All Rights Reserved
 * Part of the Wist reference implementation, under the MIT license.
 * See LICENSE.txt for license information.
*/

#ifndef _WIST_COMPILER_H
#define _WIST_COMPILER_H

#include <wist/ctx.h>
#include <wist/sym.h>
#include <wist/srcloc.h>
#include <wist/diag.h>
#include <wist/lir.h>
#include <wist/objpool.h>

/* 
 * All the needed state to compile one compilation unit of Wist, both in one 
 * sweep and as a serios of expressions or toplevel declarations. 
 */
struct wist_compiler {
    struct wist_ctx *ctx;
    struct wist_parse_result *cur_result;
    struct wist_sym_index syms;
    struct wist_srcloc_index srclocs;
    uint64_t next_type_id;
    struct wist_ast_scope *globals;
    /* Type variables. */
    struct wist_objpool type_var_pool; 
    /* All other types. */
    struct wist_objpool type_pool;
    struct wist_ast_expr *cur_expr; /* Maintained during sema. */
};

struct wist_parse_result {
    bool has_errors;
    struct wist_vector diags;
};

/* Adds a new diagnostic to the current parse result. */
struct wist_diag *wist_compiler_add_diag(struct wist_compiler *comp, 
        enum wist_diag_kind t, enum wist_diag_level);

/* 
 * Adds a source location to the list of important locations in a given 
 * diagnostic. 
 */
void wist_diag_add_loc(struct wist_compiler *comp, struct wist_diag *diag, 
        struct wist_srcloc loc);


#endif /* _WIST_COMPILER_H */

