/* === lib/handle.c - Public API handle functions=== 
 * Copyright (C) 2022 Gavin Ratcliff - All Rights Reserved
 * Part of the Wist reference implementation, under the MIT license.
 * See LICENSE.txt for license information.
*/

#include <wist.h>
#include <wist/vm_obj.h>
#include <wist/vm.h>

enum wist_obj_type wist_handle_get_type(struct wist_handle *handle) {
    return (enum wist_obj_type) handle->obj.t;
}

int64_t wist_handle_get_int(struct wist_handle *handle) {
    return handle->obj.i;
}

void wist_handle_stack_push(struct wist_vm *vm) {
    vm->cur_frame++;
    vm->frames[vm->cur_frame].cur_handle = 0;
}

void wist_handle_stack_pop(struct wist_vm *vm) {
    if (vm->cur_frame == 0) {
        vm->frames[0].cur_handle = 0;
    } else {
        vm->cur_frame--;
    }
}
