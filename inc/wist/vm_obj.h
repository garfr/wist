/* === inc/wist/vm_obj.h - Value definition for the VM === 
 * Copyright (C) 2022 Gavin Ratcliff - All Rights Reserved
 * Part of the Wist reference implementation, under the MIT license.
 * See LICENSE.txt for license information.
*/

#ifndef _WIST_VM_OBJ_H
#define _WIST_VM_OBJ_H

typedef uint32_t wist_vm_op;

struct wist_vm_proto {
    wist_vm_op ops*;
    size_t op_len;
};

enum wist_vm_obj_kind {
    WIST_VM_OBJ_INT,
    WIST_VM_OBJ_CLO,
};

struct wist_vm_obj {
    enum wist_vm_obj_kind t;

    union {
        int64_t i;
        struct {
            struct wist_vm_proto *proto;
        } clo;
    };
};

#endif /* _WIST_VM_OBJ_H */
