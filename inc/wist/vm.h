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

#define WIST_MAX_HANDLE_FRAMES 256
#define WIST_HANDLES_PER_FRAME 32

struct wist_handle {
    struct wist_vm_obj obj;
};


struct wist_handle_frame {
    struct wist_handle handles[WIST_HANDLES_PER_FRAME];
    size_t cur_handle;
};

struct wist_vm {
    struct wist_ctx *ctx;
    struct wist_vm_gc gc;
    struct wist_vector handles;
    struct wist_vector code_area;
    struct wist_toplvl *toplvl;

    struct wist_handle_frame frames[WIST_MAX_HANDLE_FRAMES];
    size_t cur_frame;
};

struct wist_vm_obj wist_vm_interpret(struct wist_vm *vm, 
        struct wist_vm_obj closure);

struct wist_handle *wist_vm_add_handle(struct wist_vm *vm);

#define WIST_VM_OBJ_CLO_PC(_vm, _obj)                                          \
    WIST_VECTOR_INDEX(&(_vm)->code_area, uint8_t, WIST_VM_OBJ_FIELD2(_obj).idx)

#endif /* _WIST_VM_H */
