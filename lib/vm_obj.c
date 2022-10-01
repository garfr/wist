/* === lib/vm_obj.c - Value definition for the VM === 
 * Copyright (C) 2022 Gavin Ratcliff - All Rights Reserved
 * Part of the Wist reference implementation, under the MIT license.
 * See LICENSE.txt for license information.
*/

#include <wist/vm_obj.h>
#include <wist/vm.h>

#include <stdio.h>
#include <stddef.h>
#include <inttypes.h>

static const char *vm_op_to_string[] = {
#define OPCODE(name, _size) [WIST_VM_OP_##name] = #name,
#include <wist/vm_ops.h>
#undef OPCODE
};

void wist_vm_obj_print_op(uint8_t op) {
        printf("%s\n", vm_op_to_string[op]);
}

void wist_vm_obj_print_clo(struct wist_vm *vm, struct wist_vm_obj clo) {
    int clo_count = 0;
    uint8_t *pc = WIST_VM_OBJ_CLO_PC(vm, clo);
    uint8_t op;

    while (1) {
        op = *(pc++);
        printf("%s", vm_op_to_string[op]);
        switch (op) {
            case WIST_VM_OP_INT64: {
                uint64_t *val = (uint64_t *) pc;
                pc += 8;
                printf(" : %" PRIu64, *val);
                break;
            }
            case WIST_VM_OP_CLOSURE: {
                clo_count++;
                uint16_t *val = (uint16_t *) pc;
                pc += 2;
                printf(" : %" PRIu16, *val);
                break;
            }
            case WIST_VM_OP_MKB: {
                uint16_t *val = (uint16_t *) pc;
                pc += 2;
                printf(" : %" PRIu16, *val);
                break;
            }
            case WIST_VM_OP_ACCESS: {
                uint8_t *val = (uint8_t *) pc;
                pc += 1;
                printf(" : %" PRIu8, *val);
                break;
            }
            case WIST_VM_OP_RETURN:
                if (clo_count == 0){
                    printf("\n");
                    return;
                }
                clo_count--;
                break;
            case WIST_VM_OP_PUSH:
            case WIST_VM_OP_PUSHMARK:
            case WIST_VM_OP_APPLY:
            case WIST_VM_OP_APPTERM:
            case WIST_VM_OP_GRAB:
                break;
        }
        printf("\n");
    }
}

struct wist_vm_obj wist_vm_obj_create_gc(enum wist_vm_obj_kind t, 
        struct wist_vm_gc_hdr *gc) {
    struct wist_vm_obj obj = {
        .t = t,
        .gc = gc,
    };
    return obj;
}
