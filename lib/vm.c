/* === lib/vm.c - Wist interpreter virtual machine === 
 * Copyright (C) 2022 Gavin Ratcliff - All Rights Reserved
 * Part of the Wist reference implementation, under the MIT license.
 * See LICENSE.txt for license information.
*/

#include <wist/vm.h>
#include <wist/ctx.h>
#include <wist/defs.h>
#include <wist/toplevel.h>

#include <stdio.h>
#include <inttypes.h>

#define WIST_VM_RSP_MAX_SIZE 128
#define WIST_VM_ASP_MAX_SIZE 128

struct return_frame {
    union {
        struct {
            struct wist_vm_obj env;
            uint8_t *pc;
            int extra_args;
        } frame;
        struct wist_vm_obj env;
    };
};

struct wist_vm *wist_vm_create(struct wist_ctx *ctx) {
    struct wist_vm *vm = WIST_CTX_NEW(ctx, struct wist_vm);
    vm->ctx = ctx;
    wist_vm_gc_init(ctx, &vm->gc);
    vm->cur_frame = 0;
    vm->frames[vm->cur_frame].cur_handle = 0;
    WIST_VECTOR_INIT(ctx, &vm->handles, struct wist_handle);
    WIST_VECTOR_INIT(ctx, &vm->code_area, uint8_t);
    return vm;
}

void wist_vm_destroy(struct wist_vm *vm) {
    wist_vm_gc_finish(&vm->gc);
    wist_vector_finish(vm->ctx, &vm->handles);
    WIST_CTX_FREE(vm->ctx, vm, struct wist_vm);
}

struct wist_handle *wist_vm_add_handle(struct wist_vm *vm) {
    struct wist_handle_frame *frame = &vm->frames[vm->cur_frame];
    return &frame->handles[frame->cur_handle++];
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

    pc = WIST_VM_OBJ_CLO_PC(vm, clo);

    struct wist_vm_obj arg_stack[WIST_VM_ASP_MAX_SIZE];
    struct wist_vm_obj *asp = arg_stack;
    struct return_frame return_stack[WIST_VM_RSP_MAX_SIZE];
    struct return_frame *rsp = return_stack;
    uint32_t extra_args = 0;

    accum.t = WIST_VM_OBJ_UNDEFINED;
    struct wist_vm_obj empty_env = WIST_VM_GC_ALLOC(&vm->gc, 0, WIST_VM_OBJ_ENV);

    while (1) {
        switch (*pc++){
            case WIST_VM_OP_CLOSURE: {
                uint16_t code_len = *((uint16_t *) pc);
                pc += 2;
                accum = WIST_VM_GC_ALLOC(&vm->gc, 2, WIST_VM_OBJ_CLO);
                WIST_VM_OBJ_FIELD2(accum).idx = pc - WIST_VECTOR_DATA(&vm->code_area, uint8_t);
                pc += code_len;
                size_t env_count = extra_args + WIST_VM_OBJ_FIELD_COUNT(env);
                struct wist_vm_obj full_env = WIST_VM_GC_ALLOC(&vm->gc, env_count, WIST_VM_OBJ_ENV);
                for (size_t i = 0; i < extra_args; i++) {
                    WIST_VM_OBJ_FIELD(full_env, i) = (--rsp)->env;
                }
                for (size_t i = 0; i < WIST_VM_OBJ_FIELD_COUNT(env); i++) {
                    WIST_VM_OBJ_FIELD(full_env, i + extra_args) = WIST_VM_OBJ_FIELD(env, i);
                }
                WIST_VM_OBJ_FIELD1(accum) = full_env;
                extra_args = 0;
                env = full_env;
                break;
            }
            case WIST_VM_OP_PUSH: {
                *asp++ = accum;
                break;
            }
            case WIST_VM_OP_LET: {
                (rsp++)->env = accum;
                extra_args++;
                break;
            }
            case WIST_VM_OP_ENDLET: {
                if (extra_args > 0) {
                    rsp--;
                    extra_args--;
                } else {
                    extra_args = WIST_VM_OBJ_FIELD_COUNT(env) - 1;
                    for (size_t i = 0; i < extra_args; i++) {
                        (rsp++)->env = WIST_VM_OBJ_FIELD(env, i + 1);
                    }
                    env = empty_env;
                }
                break;
            }
            case WIST_VM_OP_PUSHMARK: {
                asp->t = WIST_VM_OBJ_MARK;
                asp++;
                break;
            }
            case WIST_VM_OP_SETGLOBAL: {
                struct wist_sym *sym = *((struct wist_sym **) pc);
                pc += sizeof(struct wist_sym **);
                struct wist_toplvl_entry *entry = 
                    wist_toplvl_find(vm->toplvl, sym);
                entry->val = accum;
                break;
            }
            case WIST_VM_OP_GETGLOBAL: {
                struct wist_sym *sym = *((struct wist_sym **) pc);
                pc += sizeof(struct wist_sym **);
                struct wist_toplvl_entry *entry = 
                    wist_toplvl_find(vm->toplvl, sym);
                accum = entry->val;
                break;
            }
            case WIST_VM_OP_GRAB: {
                if ((asp - 1)->t == WIST_VM_OBJ_MARK) {
                    asp--;
                    accum = WIST_VM_GC_ALLOC(&vm->gc, 2, WIST_VM_OBJ_CLO);
                    WIST_VM_OBJ_FIELD2(accum).idx = pc - WIST_VECTOR_DATA(&vm->code_area, uint8_t);
                    size_t env_count = extra_args + WIST_VM_OBJ_FIELD_COUNT(env);
                    struct wist_vm_obj full_env = WIST_VM_GC_ALLOC(&vm->gc, env_count, WIST_VM_OBJ_CLO);
                    for (size_t i = 0; i < extra_args; i++) {
                        WIST_VM_OBJ_FIELD(full_env, i) = (--rsp)->env;
                    }
                    for (size_t i = 0; i < WIST_VM_OBJ_FIELD_COUNT(env); i++) {
                        WIST_VM_OBJ_FIELD(full_env, i + extra_args) = WIST_VM_OBJ_FIELD(env, i);
                    }
                    WIST_VM_OBJ_FIELD1(accum) = full_env;

                    rsp--;
                    pc = rsp->frame.pc;
                    env = rsp->frame.env;
                    extra_args = rsp->frame.extra_args;
                } else {
                    asp--;
                    rsp->env = *asp;
                    rsp++;
                    extra_args++;
                }
                break;
            }
            case WIST_VM_OP_APPLY: {
                struct return_frame *frame = rsp++;
                frame->frame.pc = pc;
                frame->frame.env = env;
                frame->frame.extra_args = extra_args;
                extra_args = 1;
                frame = rsp++;
                frame->env = *(--asp);
                env = WIST_VM_OBJ_FIELD1(accum);
                pc = WIST_VM_OBJ_CLO_PC(vm, accum);
                break;
            }
            case WIST_VM_OP_APPTERM: {
                pc = WIST_VM_OBJ_CLO_PC(vm, accum);
                env = WIST_VM_OBJ_FIELD1(accum);
                rsp -= extra_args;
                extra_args = 1;
                (rsp++)->env = *(--asp);
                break;
            }
            case WIST_VM_OP_ACCESS: {
                uint8_t idx = *pc++;
                if (idx < extra_args) {
                    accum = (rsp - (1 + idx))->env;
                } else {
                    struct wist_vm_obj iter = env;
                    for (uint8_t i = extra_args; i < idx; i++) {
                        iter = WIST_VM_OBJ_FIELD2(iter);
                    }
                    accum = WIST_VM_OBJ_FIELD1(iter);
                }
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
                    if ((asp - 1)->t == WIST_VM_OBJ_MARK) {
                        rsp -= extra_args; /* Drop all the extra args on the return stack. */
                        asp--; /* Move past the mark. */
                        rsp--;
                        pc = rsp->frame.pc;
                        env = rsp->frame.env;
                        extra_args = rsp->frame.extra_args;
                    } else {
                        rsp -= (extra_args);
                        (rsp + 1)->env = *(--asp);
                        env = WIST_VM_OBJ_FIELD1(accum);
                        pc = WIST_VM_OBJ_CLO_PC(vm, accum);
                        extra_args = 1;
                    }
                    break;
                };
            default:
                printf("unimplemented op case in wist_vm_interpret : %d\n", *(pc - 1));
                break;
        }
    }
}
