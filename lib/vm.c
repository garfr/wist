/* === lib/vm.c - Wist interpreter virtual machine === 
 * Copyright (C) 2022 Gavin Ratcliff - All Rights Reserved
 * Part of the Wist reference implementation, under the MIT license.
 * See LICENSE.txt for license information.
*/

#include <wist/vm.h>
#include <wist/ctx.h>
#include <wist/defs.h>

#include <stdio.h>
#include <inttypes.h>

#define WIST_VM_RSP_MAX_SIZE 128
#define WIST_VM_ASP_MAX_SIZE 128

struct return_frame {
    struct wist_vm_obj env;
    uint8_t *pc;
};

struct wist_vm *wist_vm_create(struct wist_ctx *ctx) {
    struct wist_vm *vm = WIST_CTX_NEW(ctx, struct wist_vm);
    vm->ctx = ctx;
    wist_vm_gc_init(ctx, &vm->gc);
    WIST_VECTOR_INIT(ctx, &vm->handles, struct wist_handle);
    return vm;
}

void wist_vm_destroy(struct wist_vm *vm) {
    wist_vm_gc_finish(&vm->gc);
    wist_vector_finish(vm->ctx, &vm->handles);
    WIST_CTX_FREE(vm->ctx, vm, struct wist_vm);
}

struct wist_handle *wist_vm_add_handle(struct wist_vm *vm) {
    return WIST_CTX_NEW(vm->ctx, struct wist_handle);
}

struct wist_handle *wist_vm_eval(struct wist_vm *vm, 
        struct wist_handle *closure) {
    struct wist_handle *handle = wist_vm_add_handle(vm);
    handle->obj = wist_vm_interpret(vm, closure->obj);
    return handle;
}

struct wist_vm_obj wist_vm_interpret(struct wist_vm *vm, 
        struct wist_vm_obj clo) {
    IGNORE(vm);

    struct wist_vm_obj accum, env = WIST_VM_OBJ_FIELD1(clo);
    uint8_t *pc;

    pc = WIST_VM_OBJ_CLO_PC(clo);

    struct wist_vm_obj arg_stack[WIST_VM_ASP_MAX_SIZE];
    struct wist_vm_obj *asp = arg_stack;
    struct return_frame return_stack[WIST_VM_RSP_MAX_SIZE];
    struct return_frame *rsp = return_stack;

    accum.t = env.t = WIST_VM_OBJ_UNDEFINED;

    while (1) {
        switch (*pc++){
            case WIST_VM_OP_CLOSURE: {
                uint16_t code_len = *((uint16_t *) pc);
                pc += 2;
                accum = WIST_VM_GC_ALLOC(&vm->gc, 2 + (code_len / 
                            sizeof(struct wist_vm_obj)), WIST_VM_OBJ_CLO);
                memcpy(&WIST_VM_OBJ_FIELD2(accum), pc, code_len);
                pc += code_len;
                WIST_VM_OBJ_FIELD1(accum) = env;
                break;
            }
            case WIST_VM_OP_PUSH: {
                *asp++ = accum;
                break;
            }
            case WIST_VM_OP_APPLY: {
                struct return_frame *frame = rsp++;
                frame->pc = pc;
                frame->env = env;
                env = WIST_VM_OBJ_FIELD1(accum);
                pc = WIST_VM_OBJ_CLO_PC(accum);
                struct wist_vm_obj new_env = WIST_VM_GC_ALLOC(&vm->gc, 
                        2, WIST_VM_OBJ_ENV);
                WIST_VM_OBJ_FIELD1(new_env) = *(--asp);
                WIST_VM_OBJ_FIELD2(new_env) = env;
                env = new_env;
                break;
            }
            case WIST_VM_OP_ACCESS: {
                uint8_t idx = *pc++;
                struct wist_vm_obj iter = env;
                for (uint8_t i = 0; i < idx; i++) {
                    iter = WIST_VM_OBJ_FIELD2(iter);
                }
                accum = WIST_VM_OBJ_FIELD1(iter);
                break;
            }
            case WIST_VM_OP_INT64: {
                accum.t = WIST_VM_OBJ_INT;
                accum.i =  *((int64_t*) pc);;
                pc += 8;
                break;
            }
            case WIST_VM_OP_MKB: {
                uint16_t field_count = *((uint16_t *) pc);
                pc += 2;
                accum = WIST_VM_GC_ALLOC(&vm->gc, field_count, WIST_VM_OBJ_TUPLE);
                for (int i = field_count - 1; i >= 0; i--) {
                    WIST_VM_OBJ_FIELD(accum, i) = *(--asp);
                }
                break;
            }
            case WIST_VM_OP_RETURN: 
                if (rsp == return_stack) {
                    return accum;
                } else {
                    rsp--;
                    pc = rsp->pc;
                    env = rsp->env;
                    break;
                };
            default:
                printf("unimplemented op case in wist_vm_interpret\n");
                break;
        }
    }
}
