/* === inc/wist/vm.h - Wist interpreter virtual machine === 
 * Copyright (C) 2022 Gavin Ratcliff - All Rights Reserved
 * Part of the Wist reference implementation, under the MIT license.
 * See LICENSE.txt for license information.
*/

#ifndef _WIST_VM_H
#define _WIST_VM_H

#include <wist.h>
#include <wist/vm_obj.h>
#include <wist/vm_gc.h>
#include <wist/lexer.h>

struct wist_handle {
    struct wist_vm_obj obj;
};

struct wist_vm {
    struct wist_ctx *ctx;
    struct wist_vm_gc gc;
    struct wist_vector handles;
};

struct wist_vm_obj wist_vm_interpret(struct wist_vm *vm, 
        struct wist_vm_obj closure);

struct wist_handle *wist_vm_add_handle(struct wist_vm *vm);

#endif /* _WIST_VM_H */
