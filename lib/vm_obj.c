/* === lib/vm_obj.c - Value definition for the VM === 
 * Copyright (C) 2022 Gavin Ratcliff - All Rights Reserved
 * Part of the Wist reference implementation, under the MIT license.
 * See LICENSE.txt for license information.
*/

#include <wist/vm_obj.h>

#include <stdio.h>
#include <stddef.h>
#include <inttypes.h>

static const char *vm_op_to_string[] = {
#define OPCODE(name, _size) [WIST_VM_OP_##name] = #name,
#include <wist/vm_ops.h>
#undef OPCODE
};

void wist_vm_obj_print_closure(struct wist_vm_closure *clo) {
    size_t i = 0;
    while (i < clo->code_len)
    {
        printf("%s", vm_op_to_string[clo->code[i]]);
        switch (clo->code[i++]) {
#define OPCODE(name, size) case WIST_VM_OP_##name:                             \
            switch (size) {                                                    \
                case 0: printf("\n"); break;                                   \
                case 1: printf(": %" PRIu8, *((uint8_t *) &clo->code[i]));     \
                        i += 1; break;                                         \
                case 2: printf(": %" PRIu16, *((uint16_t *) &clo->code[i]));   \
                        i += 2; break;                                         \
                case 4: printf(" : %" PRIu32, *((uint32_t *) &clo->code[i]));  \
                        i += 4; break;                                         \
                case 8: printf(": %" PRIu64, *((uint64_t *) &clo->code[i]));   \
                        i += 8; break;                                         \
            } if (size > 0) { printf(" (%d) \n", size); } break;
#include <wist/vm_ops.h>
#undef OPCODE
        }
    }
}

void wist_vm_obj_print_op(uint8_t op) {
        printf("%s\n", vm_op_to_string[op]);
}
