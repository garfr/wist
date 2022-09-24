/* === lib/vm_gen.c - Code generation for the VM === 
 * Copyright (C) 2022 Gavin Ratcliff - All Rights Reserved
 * Part of the Wist reference implementation, under the MIT license.
 * See LICENSE.txt for license information.
*/

#include <wist.h>
#include <wist/lir.h>
#include <wist/compiler.h>

struct wist_vm_handle *wist_compiler_vm_gen_expr(struct wist_compiler *comp,
        struct wist_vm *vm, struct wist_ast_expr *expr) {
    IGNORE(vm);

    struct wist_lir_expr *lir_expr = wist_compiler_lir_gen_expr(comp, expr);

    wist_lir_print_expr(lir_expr);

    wist_lir_expr_destroy(comp, lir_expr);

    return NULL;
}
