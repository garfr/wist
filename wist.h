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

/* === COMPILER === */

/* 
 * A wist compiler is used to parse, validate, and generate bytecode from Wist source 
 * code.  The compiler object holds information used to compile only one piece of code 
 * at a time, so if parallel compilation is needed then create 1 compiler object for 
 * each thread.
 */
struct wist_compiler;

/* Creates a new compiler object owned by [ctx]. */
struct wist_compiler *wist_compiler_create(struct wist_ctx *ctx);

/* If [comp] is not NULL, destroys the compiler. */
void wist_compiler_destroy(struct wist_compiler *comp);

#endif /* _WIST_WIST_H */
