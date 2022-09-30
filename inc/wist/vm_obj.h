/* === inc/wist/vm_obj.h - Value definition for the VM === 
 * Copyright (C) 2022 Gavin Ratcliff - All Rights Reserved
 * Part of the Wist reference implementation, under the MIT license.
 * See LICENSE.txt for license information.
*/

#ifndef _WIST_VM_OBJ_H
#define _WIST_VM_OBJ_H

#include <wist.h>

struct wist_vm_gc_hdr;

enum wist_vm_op {
#define OPCODE(name, _args) WIST_VM_OP_##name,
#include <wist/vm_ops.h>
#undef OPCODE
__WIST_VM_OP_COUNT /* The number of opcodes. */
};

enum wist_vm_obj_kind {
    WIST_VM_OBJ_CLO = WIST_OBJ_CLOSURE,
    WIST_VM_OBJ_INT = WIST_OBJ_INTEGER,
    WIST_VM_OBJ_TUPLE = WIST_OBJ_TUPLE,

    /* These are internal types the user won't (shouldn't) see. */
    WIST_VM_OBJ_ENV,
    WIST_VM_OBJ_MARK,

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

#include <wist/vm_gc.h>

#define WIST_VM_TO_GC_HDR(_ptr) (&(_ptr)->hdr)

struct wist_vm_obj wist_vm_obj_create_gc(enum wist_vm_obj_kind, 
        struct wist_vm_gc_hdr *gc);

void wist_vm_obj_print_op(uint8_t op);
void wist_vm_obj_print_clo(struct wist_vm_obj clo);

#define WIST_VM_OBJ_CLO_PC(_obj) ((uint8_t *) &((_obj).gc->fields[1]))
#define WIST_VM_OBJ_FIELD1(_obj) ((_obj).gc->fields[0])
#define WIST_VM_OBJ_FIELD2(_obj) ((_obj).gc->fields[1])
#define WIST_VM_OBJ_FIELD(_obj, _n) ((_obj).gc->fields[_n])

#endif /* _WIST_VM_OBJ_H */
