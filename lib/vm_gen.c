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

struct index_map {
    struct index_map *next;
    struct wist_lir_expr *expr;
    uint64_t index;
};

struct code_builder {
    struct wist_ctx *ctx;
    struct wist_vector code;
    struct index_map *indices;
};

/* === PROTOTYPES === */

/* Code builder. */
static void code_builder_init(struct wist_ctx *ctx, 
        struct code_builder *builder);

static void code_builder_add_8(struct code_builder *builder, uint8_t byte);
static void code_builder_add_16(struct code_builder *builder, uint16_t u16);
static void code_builder_add_64(struct code_builder *builder, uint64_t u64);
static size_t code_builder_count(struct code_builder *builder);
static size_t code_builder_add_16_uninit(struct code_builder *builder);

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

    struct wist_vm_obj clo = WIST_VM_GC_ALLOC(&vm->gc, 2 + 
            (WIST_VECTOR_LEN(&builder.code, uint8_t) 
           / sizeof(struct wist_vm_obj)), WIST_VM_OBJ_CLO);
    memcpy(WIST_VM_OBJ_CLO_PC(clo), WIST_VECTOR_DATA(&builder.code, uint8_t), 
            WIST_VECTOR_LEN(&builder.code, uint8_t));

    wist_lir_expr_destroy(comp, lir_expr);

    struct wist_handle *handle = wist_vm_add_handle(vm);
    handle->obj = clo;

    return handle;
}

/* === PRIVATES=== */

static void code_builder_init(struct wist_ctx *ctx, 
        struct code_builder *builder) {
    builder->ctx = ctx;
    builder->indices = NULL;
    WIST_VECTOR_INIT(ctx, &builder->code, uint8_t);
}

static void code_builder_add_8(struct code_builder *builder, uint8_t byte) {
    WIST_VECTOR_PUSH(builder->ctx, &builder->code, uint8_t, &byte);
}

static void code_builder_add_16(struct code_builder *builder, uint16_t _u16) {
    uint8_t *u16 = (uint8_t *) &_u16;
    code_builder_add_8(builder, u16[0]);
    code_builder_add_8(builder, u16[1]);
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

static size_t code_builder_add_16_uninit(struct code_builder *builder) {
    size_t idx = WIST_VECTOR_LEN(&builder->code, uint8_t);
    code_builder_add_8(builder, 0);
    code_builder_add_8(builder, 0);
    return idx;
}

static void gen_expr_rec(struct code_builder *builder, 
        struct wist_lir_expr *expr) {
    switch (expr->t) {
        case WIST_LIR_EXPR_INT: 
            code_builder_add_8(builder, WIST_VM_OP_INT64); 
            code_builder_add_64(builder, expr->i.val); 
            break;
        case WIST_LIR_EXPR_LAM: {
            struct index_map *map = builder->indices;
            while (map != NULL) {
                map->index++;
                map = map->next;
            }
            struct index_map *new_index = WIST_CTX_NEW(builder->ctx, struct index_map);
            new_index->index = 0;
            new_index->expr = expr;
            new_index->next = builder->indices;
            builder->indices = new_index;
            code_builder_add_8(builder, WIST_VM_OP_CLOSURE);
            size_t closure_size_idx = code_builder_add_16_uninit(builder);
            size_t op_count_before = code_builder_count(builder);

            gen_expr_rec(builder, expr->lam.body);

            code_builder_add_8(builder, WIST_VM_OP_RETURN);

            size_t op_count_after = code_builder_count(builder);
            uint16_t *closure_pos = (uint16_t *)
                WIST_VECTOR_INDEX(&builder->code, uint8_t, closure_size_idx);
            uint16_t closure_sz = (uint16_t) (op_count_after - op_count_before);
            *closure_pos = closure_sz;

            map = builder->indices;
            builder->indices = map->next;
            WIST_CTX_FREE(builder->ctx, map, struct index_map);
            map = builder->indices;
            while (map != NULL) {
                map->index--;
                map = map->next;
            }
            break;
        }
        case WIST_LIR_EXPR_APP: {
            gen_expr_rec(builder, expr->app.arg);
            code_builder_add_8(builder, WIST_VM_OP_PUSH);
            gen_expr_rec(builder, expr->app.fun);
            code_builder_add_8(builder, WIST_VM_OP_APPLY);
            break;
        }
        case WIST_LIR_EXPR_VAR: {
            struct index_map *map = builder->indices;
            while (map != NULL) {
                if (map->expr == expr->var.origin) {
                    code_builder_add_8(builder, WIST_VM_OP_ACCESS);
                    code_builder_add_8(builder, map->index);
                    break;
                }
                map = map->next;
            }
            break;
        }
        case WIST_LIR_EXPR_MKB: {
            WIST_VECTOR_FOR_EACH(&expr->mkb.fields, struct wist_lir_expr *, field) {
                gen_expr_rec(builder, *field);
                code_builder_add_8(builder, WIST_VM_OP_PUSH);
            }
            code_builder_add_8(builder, WIST_VM_OP_MKB);
            code_builder_add_16(builder, 
                    WIST_VECTOR_LEN(&expr->mkb.fields, struct wist_lir_expr *));
            break;
        }
        default: 
            printf("Cannot generate vm code for expression %d\n", expr->t);
            return;
    }
}
