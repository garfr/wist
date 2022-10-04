/* === bin/wisti.c - Wist interpreter === 
 * Copyright (C) 2022 Gavin Ratcliff - All Rights Reserved
 * Part of the Wist reference implementation, under the MIT license.
 * See LICENSE.txt for license information.
*/

#include <wist.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

int main()
{
    struct wist_ctx *ctx = wist_ctx_create();
    if (ctx == NULL)
    {
        printf("failed to create wist context\n");
        return EXIT_FAILURE;
    }

    struct wist_compiler *comp = wist_compiler_create(ctx);

    struct wist_vm *vm = wist_vm_create(ctx);

    const char toplvl_src[] = "x = x";
    const char expr_src[] = "x";

    struct wist_ast_decl *decl;
    struct wist_ast_expr *expr;

    struct wist_parse_result *result = wist_compiler_parse_decl(comp, 
            (const uint8_t *) toplvl_src, strlen(toplvl_src), &decl);
    if (wist_parse_result_has_errors(result))
    {
        printf("errors found in toplvl\n");
        return EXIT_FAILURE;
    }

    wist_compiler_vm_connect(comp, vm);

    struct wist_handle *handle = wist_compiler_vm_gen_decl(comp, vm, decl);

    wist_vm_eval(vm, handle);

    result = wist_compiler_parse_expr(comp, (const uint8_t *) expr_src, 
            strlen(expr_src), &expr);
    if (wist_parse_result_has_errors(result))
    {
        printf("errors found in expression\n");
        return EXIT_FAILURE;
    }

    handle = wist_compiler_vm_gen_expr(comp, vm, expr);
    handle = wist_vm_eval(vm, handle);

    printf("%d %" PRId64 "\n", wist_handle_get_type(handle), wist_handle_get_int(handle));

    wist_parse_result_destroy(comp, result);
    wist_ast_decl_destroy(comp, decl);

    wist_vm_destroy(vm);
    wist_compiler_destroy(comp);

    wist_ctx_destroy(ctx);

    return EXIT_SUCCESS;
}

