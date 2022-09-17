/* === bin/wisti.c - Wist interpreter === 
 * Copyright (C) 2022 Gavin Ratcliff - All Rights Reserved
 * Part of the Wist reference implementation, under the MIT license.
 * See LICENSE.txt for license information.
*/

#include <wist.h>

#include <stdio.h>
#include <stdlib.h>

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

    wist_compiler_destroy(comp);

    wist_ctx_destroy(ctx);

    return EXIT_SUCCESS;
}

