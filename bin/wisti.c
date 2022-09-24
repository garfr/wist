/* === bin/wisti.c - Wist interpreter === 
 * Copyright (C) 2022 Gavin Ratcliff - All Rights Reserved
 * Part of the Wist reference implementation, under the MIT license.
 * See LICENSE.txt for license information.
*/

#include <wist.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
    struct wist_ctx *ctx = wist_ctx_create();
    if (ctx == NULL)
    {
        printf("failed to create wist context\n");
        return EXIT_FAILURE;
    }

    printf("wist context created\n");

    struct wist_compiler *comp = wist_compiler_create(ctx);

    printf("wist compiler created\n");

    const char src[] = "(\\x -> x) 3";

    struct wist_ast_expr *expr;

    struct wist_parse_result *result = wist_compiler_parse_expr(comp, 
            (const uint8_t *) src, strlen(src), &expr);
    if (wist_parse_result_has_errors(result))
    {
        printf("errors found in expression\n");
        return EXIT_FAILURE;
    }

    wist_parse_result_destroy(comp, result);
    wist_ast_expr_destroy(comp, expr);

    wist_compiler_destroy(comp);

    wist_ctx_destroy(ctx);

    return EXIT_SUCCESS;
}

