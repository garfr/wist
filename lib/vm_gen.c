/* === lib/vm_gen.c - Code generation for the VM === 
 * Copyright (C) 2022 Gavin Ratcliff - All Rights Reserved
 * Part of the Wist reference implementation, under the MIT license.
 * See LICENSE.txt for license information.
*/

#include <wist.h>
#include <wist/lir.h>
#include <wist/compiler.h>
#include <wist/vm.h>
#include <wist/vm_obj.h>
#include <wist/vector.h>

#include <stdio.h>
#include <inttypes.h>

struct code_builder {
    struct wist_ctx *ctx;
    struct wist_vector code;
};

/* === PROTOTYPES === */

/* Code builder. */
static void code_builder_init(struct wist_ctx *ctx, 
        struct code_builder *builder);

static void code_builder_add_8(struct code_builder *builder, uint8_t byte);
static void code_builder_add_64(struct code_builder *builder, uint64_t u64);
static size_t code_builder_count(struct code_builder *builder);
static uint16_t *code_builder_add_16_uninit(struct code_builder *builder);

static void gen_expr_rec(struct code_builder *builder, 
        struct wist_lir_expr *expr);

/* === PUBLICS === */

struct wist_handle *wist_compiler_vm_gen_expr(struct wist_compiler *comp,
        struct wist_vm *vm, struct wist_ast_expr *expr) {
    struct code_builder builder;
    IGNORE(vm);

    struct wist_lir_expr *lir_expr = wist_compiler_lir_gen_expr(comp, expr);

    code_builder_init(comp->ctx, &builder);

    gen_expr_rec(&builder, lir_expr);

    code_builder_add_8(&builder, WIST_VM_OP_RETURN);

    struct wist_vm_closure *closure = WIST_VM_GC_ALLOC(&vm->gc, 
            struct wist_vm_closure);
    closure->code = WIST_VECTOR_DATA(&builder.code, uint8_t);
    closure->code_len = WIST_VECTOR_LEN(&builder.code, uint8_t);
    closure->env.t = WIST_VM_OBJ_UNDEFINED;

    wist_lir_expr_destroy(comp, lir_expr);

    struct wist_handle *handle = wist_vm_add_handle(vm);
    handle->obj.t = WIST_VM_OBJ_CLO;
    handle->obj.gc = WIST_VM_TO_GC_HDR(closure);

    return handle;
}

/* === PRIVATES=== */

static void code_builder_init(struct wist_ctx *ctx, 
        struct code_builder *builder) {
    builder->ctx = ctx;
    WIST_VECTOR_INIT(ctx, &builder->code, uint8_t);
}

static void code_builder_add_8(struct code_builder *builder, uint8_t byte) {
    WIST_VECTOR_PUSH(builder->ctx, &builder->code, uint8_t, &byte);
}

static void code_builder_add_64(struct code_builder *builder, uint64_t _u64) {
    uint8_t *u64 = (uint8_t *) &_u64;
    code_builder_add_8(builder, u64[0]);
    code_builder_add_8(builder, u64[1]);
    code_builder_add_8(builder, u64[2]);
    code_builder_add_8(builder, u64[3]);
    code_builder_add_8(builder, u64[4]);
    code_builder_add_8(builder, u64[5]);
    code_builder_add_8(builder, u64[6]);
    code_builder_add_8(builder, u64[7]);
}

static size_t code_builder_count(struct code_builder *builder) {
    return WIST_VECTOR_LEN(&builder->code, uint8_t);
}

static uint16_t *code_builder_add_16_uninit(struct code_builder *builder) {
    uint16_t *ptr = (uint16_t *) WIST_VECTOR_PUSH_UNINIT(builder->ctx, 
            &builder->code, uint8_t);
    WIST_VECTOR_PUSH_UNINIT(builder->ctx, &builder->code, uint8_t);
    return ptr;
}

static void gen_expr_rec(struct code_builder *builder, 
        struct wist_lir_expr *expr) {
    switch (expr->t) {
        case WIST_LIR_EXPR_INT: 
            code_builder_add_8(builder, WIST_VM_OP_INT64); 
            code_builder_add_64(builder, expr->i.val); 
            break;
        case WIST_LIR_EXPR_LAM: {
            code_builder_add_8(builder, WIST_VM_OP_CLOSURE);
            uint16_t *closure_size = code_builder_add_16_uninit(builder);
            size_t op_count_before = code_builder_count(builder);
            gen_expr_rec(builder, expr->lam.body);
            code_builder_add_8(builder, WIST_VM_OP_RETURN);
            size_t op_count_after = code_builder_count(builder);
            *closure_size = (uint16_t) (op_count_after - op_count_before);
            break;
        }
        default: 
            printf("Cannot generate vm code for expression %d\n", expr->t);
            return;
    }
}
