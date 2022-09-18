/* === wist.h - Wist public API === 
 * Copyright (C) 2022 Gavin Ratcliff - All Rights Reserved
 * Part of the Wist reference implementation, under the MIT license.
 * See LICENSE.txt for license information.
*/

#ifndef _WIST_WIST_H
#define _WIST_WIST_H

#define WIST_VER_MAJ 0
#define WIST_VER_MIN 1

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/* === CONTEXT === */

/* 
 * The state object for a indepedent executing piece of Wist.
 * Data cannot be shared between seperate contexts easily, and destroying a 
 * context invalidates all owned objects.
 */
struct wist_ctx;

/* 
 * [ud] will always be the alloc_ud passed on context creation.
 *
 * A valid allocator function does the following:
 * If [ptr] is NULL, this must return NULL or a pointer to a unique chunk [nsz] bytes.
 * Else, if [nsz] is 0, [ptr] should be freed.
 * Else must return NULL or a pointer to a new chunk of [nsz] bytes, or [ptr], 
 * with [ptr] now being valid for [nsz] bytes.
 */
typedef void *(*wist_alloc_fn)(void *ud, void *ptr, size_t osz, size_t nsz);

/* Creates a Wist context with default libc memory allocator. */
struct wist_ctx *wist_ctx_create(void);

/* Creates a Wist context with specified allocator. */
struct wist_ctx *wist_ctx_create_with_allocator(wist_alloc_fn fn, void *ud);

/* If [ctx] is not NULL, destroys the context. */
void wist_ctx_destroy(struct wist_ctx *ctx);

/* === AST === */

/* 
 * The Wist AST is organized as a disjoint union which can be manipulated 
 * through the visitor interface.
 */
struct wist_ast_expr;

/* === COMPILER === */

/* 
 * A Wist compiler is used to parse, validate, and generate bytecode from Wist source 
 * code.  The compiler object holds information used to compile only one piece of code 
 * at a time, so if parallel compilation is needed then create 1 compiler object for 
 * each thread.
 */
struct wist_compiler;

struct wist_parse_result {
    bool has_errors;
};

/* Creates a new compiler object owned by [ctx]. */
struct wist_compiler *wist_compiler_create(struct wist_ctx *ctx);

/* If [comp] is not NULL, destroys the compiler. */
void wist_compiler_destroy(struct wist_compiler *comp);

/* Parses and typechecks a string and returns its completed AST node. */
struct wist_parse_result wist_compiler_parse_expr(struct wist_compiler *comp,
        const uint8_t *src, size_t src_len, struct wist_ast_expr **expr_out);

/* 
 * Releases all memory allocated for a parse result.  Any error strings in the 
 * result are invalidated. 
 */
void wist_parse_result_finish(struct wist_compiler *comp, 
        struct wist_parse_result *result);

#endif /* _WIST_WIST_H */
