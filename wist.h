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

/* === VM === */

/*
 * A VM is where Wist code is interpreted.  Handles to Wist values are tied 
 * to a certain VM and may only be used with that VM. 
 */

struct wist_vm;

/* 
 * A handle to a specific value in the VM.  These can be global or put on a 
 * "handle stack".
 */
/* Creates a new VM from a given context. */
struct wist_vm *wist_vm_create(struct wist_ctx *ctx);

/* Destroys a VM freeing ALL resources allocated to it. */
void wist_vm_destroy(struct wist_vm *vm);

/* Evaluates a closure and returns the final value. */
struct wist_handle *wist_vm_eval(struct wist_vm *vm, struct wist_handle *closure);

struct wist_handle;

enum wist_obj_type {
    WIST_OBJ_CLOSURE,
    WIST_OBJ_INTEGER,
    WIST_OBJ_TUPLE,
};

enum wist_obj_type wist_handle_get_type(struct wist_handle *handle);
int64_t wist_handle_get_int(struct wist_handle *handle);

/* === AST === */

/* 
 * The Wist AST is organized as a disjoint union which can be manipulated 
 * through the visitor interface.
 */

struct wist_ast_expr;
struct wist_ast_scope;
struct wist_ast_type;

/* === COMPILER === */

/* 
 * A Wist compiler is used to parse, validate, and generate bytecode from Wist source 
 * code.  The compiler object holds information used to compile only one piece of code 
 * at a time, so if parallel compilation is needed then create 1 compiler object for 
 * each thread.
 */
struct wist_compiler;

/* 
 * The result of parsing and typechecking a piece of Wist code. Contains all 
 * information on errors, warnings, or info diagnostics created  by the 
 * compiler. 
 */
struct wist_parse_result;

/* Creates a new compiler object owned by [ctx]. */
struct wist_compiler *wist_compiler_create(struct wist_ctx *ctx);

/* If [comp] is not NULL, destroys the compiler. */
void wist_compiler_destroy(struct wist_compiler *comp);

/* Parses and typechecks a string and returns its completed AST node. */
struct wist_parse_result *wist_compiler_parse_expr(struct wist_compiler *comp,
        const uint8_t *src, size_t src_len, struct wist_ast_expr **expr_out);

/* Generates code to evaluate an expression for the VM. */
struct wist_handle *wist_compiler_vm_gen_expr(struct wist_compiler *comp,
        struct wist_vm *vm, struct wist_ast_expr *expr);

/* Releases an AST expression generated by the compiler. */
void wist_ast_expr_destroy(struct wist_compiler *comp, 
        struct wist_ast_expr *expr);

/* 
 * Releases all memory allocated for a parse result.  Any error strings in the 
 * result are invalidated. 
 */
void wist_parse_result_destroy(struct wist_compiler *comp, 
        struct wist_parse_result *result);

/* Returns if there are any errors or fatal errors in [result]. */
bool wist_parse_result_has_errors(struct wist_parse_result *result);

#endif /* _WIST_WIST_H */
