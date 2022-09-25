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

#define WIST_VM_ASP_MAX_SIZE 128
#define WIST_VM_RSP_MAX_SIZE 128

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
    return WIST_VECTOR_PUSH_UNINIT(vm->ctx, &vm->handles, struct wist_handle);
}

struct wist_handle *wist_vm_eval(struct wist_vm *vm, 
        struct wist_handle *closure) {
    struct wist_handle *handle = wist_vm_add_handle(vm);
    handle->obj = wist_vm_interpret(vm, closure->obj);
    return handle;
}

struct wist_vm_obj wist_vm_interpret(struct wist_vm *vm, 
        struct wist_vm_obj closure) {
    IGNORE(vm);

    struct wist_vm_obj accum, env;
    uint8_t *pc;

    struct wist_vm_closure *real_closure = WIST_VM_GET_CLOSURE(closure);
    pc = real_closure->code;

    accum.t = env.t = WIST_VM_OBJ_UNDEFINED;

    while (1) {
        switch (*pc++){
            case WIST_VM_OP_CLOSURE: {
                uint16_t code_len = *((uint16_t *) pc);
                pc += 2;
                struct wist_vm_closure *closure = 
                    WIST_VM_GC_ALLOC(&vm->gc, struct wist_vm_closure);
                closure->code = WIST_CTX_NEW_ARR(vm->ctx, uint8_t, code_len);
                closure->code_len = code_len;
                memcpy(closure->code, pc, code_len);
                pc += code_len;
                closure->env = env;
                printf("sub closure\n");
                wist_vm_obj_print_closure(closure);
                accum.t = WIST_VM_OBJ_CLO;
                accum.gc = WIST_VM_TO_GC_HDR(closure);
                break;
            }
            case WIST_VM_OP_INT64: {
                accum.t = WIST_VM_OBJ_INT;
                accum.i =  *((int64_t*) pc);;
                pc += 8;
                break;
            }
            case WIST_VM_OP_RETURN: 
                return accum;
            default:
                printf("unimplemented op case in wist_vm_interpret\n");
                break;
        }
    }
}
