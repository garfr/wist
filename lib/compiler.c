/* === lib/compiler.h - Wist compiler === 
 * Copyright (C) 2022 Gavin Ratcliff - All Rights Reserved
 * Part of the Wist reference implementation, under the MIT license.
 * See LICENSE.txt for license information.
*/

#include <wist/compiler.h>

/* === PROTOTYPES === */

/* === PUBLICS === */

struct wist_compiler *wist_compiler_create(struct wist_ctx *ctx) {
    struct wist_compiler *comp = WIST_CTX_NEW(ctx, struct wist_compiler);

    comp->ctx = ctx;

    return comp;
}

void wist_compiler_destroy(struct wist_compiler *comp) {
    if (comp == NULL) {
        return;
    }

    WIST_CTX_FREE(comp->ctx, comp, struct wist_compiler);
}

/* === PRIVATES === */
