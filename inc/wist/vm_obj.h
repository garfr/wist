/* === inc/wist/vm_obj.h - Value definition for the VM === 
 * Copyright (C) 2022 Gavin Ratcliff - All Rights Reserved
 * Part of the Wist reference implementation, under the MIT license.
 * See LICENSE.txt for license information.
*/

#ifndef _WIST_VM_OBJ_H
#define _WIST_VM_OBJ_H

#include <wist/vm_gc.h>

enum wist_vm_op {
#define OPCODE(name, _args) WIST_VM_OP_##name,
#include <wist/vm_ops.h>
#undef OPCODE
__WIST_VM_OP_COUNT /* The number of opcodes. */
};

enum wist_vm_obj_kind {
    WIST_VM_OBJ_CLO = WIST_OBJ_CLOSURE,
    WIST_VM_OBJ_INT = WIST_OBJ_INTEGER,

    /* These are internal types the user won't (shouldn't) see. */
    WIST_VM_OBJ_ENV,

    /* Used when values have not been initialized for debugging. */
    WIST_VM_OBJ_UNDEFINED, 
};

struct wist_vm_obj {
    enum wist_vm_obj_kind t;

    union {
        int64_t i; 
        struct wist_vm_gc_hdr *gc;
    };
};

struct wist_vm_env {
    struct wist_vm_gc_hdr hdr;
    struct wist_vm_obj val;
    struct wist_vm_obj next;
};

struct wist_vm_closure {
    struct wist_vm_gc_hdr hdr;
    uint8_t *code;
    size_t code_len;
    struct wist_vm_obj env;
};

#define WIST_VM_TO_GC_HDR(_ptr) (&(_ptr)->hdr)

#define WIST_VM_GET_CLOSURE(_obj) ((struct wist_vm_closure *) (_obj).gc)
#define WIST_VM_GET_ENV(_obj) ((struct wist_vm_env *) (_obj).gc)

void wist_vm_obj_print_closure(struct wist_vm_closure *clo);
void wist_vm_obj_print_op(uint8_t op);

#endif /* _WIST_VM_OBJ_H */
