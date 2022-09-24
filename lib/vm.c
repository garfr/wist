/* === lib/vm.c - Wist interpreter virtual machine === 
 * Copyright (C) 2022 Gavin Ratcliff - All Rights Reserved
 * Part of the Wist reference implementation, under the MIT license.
 * See LICENSE.txt for license information.
*/

#include <wist/vm.h>
#include <wist/ctx.h>

struct wist_vm *wist_vm_create(struct wist_ctx *ctx) {
    struct wist_vm *vm = WIST_CTX_NEW(ctx, struct wist_vm);
    vm->ctx = ctx;
    return vm;
}

void wist_vm_destroy(struct wist_vm *vm) {
    WIST_CTX_FREE(vm->ctx, vm, struct wist_vm);
}
