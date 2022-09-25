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
