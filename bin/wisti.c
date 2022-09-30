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

//    const char src[] = "(\\x -> x) (3, 4)";
    const char src[] = "(\\x -> x) (3, 3)";

    struct wist_ast_expr *expr;

    struct wist_parse_result *result = wist_compiler_parse_expr(comp, 
            (const uint8_t *) src, strlen(src), &expr);
    if (wist_parse_result_has_errors(result))
    {
        printf("errors found in expression\n");
        return EXIT_FAILURE;
    }

    struct wist_handle *val = wist_compiler_vm_gen_expr(comp, vm, expr);

    struct wist_handle *computed = wist_vm_eval(vm, val);

    printf("%d\n", wist_handle_get_type(computed));

    wist_parse_result_destroy(comp, result);
    wist_ast_expr_destroy(comp, expr);

    wist_vm_destroy(vm);
    wist_compiler_destroy(comp);

    wist_ctx_destroy(ctx);

    return EXIT_SUCCESS;
}

