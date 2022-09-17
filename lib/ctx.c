/* === lib/ctx.c - Wist context === 
 * Copyright (C) 2022 Gavin Ratcliff - All Rights Reserved
 * Part of the Wist reference implementation, under the MIT license.
 * See LICENSE.txt for license information.
*/

#include <wist/defs.h>
#include <wist/ctx.h>

#include <stdlib.h>

/* === PROTOTYPES === */

static void *libc_alloc(void *ud, void *ptr, size_t osz, size_t nsz);

/* === PUBLICS === */

struct wist_ctx *wist_ctx_create(void) {
    return wist_ctx_create_with_allocator(libc_alloc, NULL);
}

struct wist_ctx *wist_ctx_create_with_allocator(wist_alloc_fn fn, void *ud) {
    struct wist_ctx *ctx = fn(ud, NULL, 0, sizeof(struct wist_ctx));
    if (ctx == NULL) {
        return NULL;
    }

    ctx->alloc_fn = fn;
    ctx->alloc_ud = ud;
    return ctx;
}

void wist_ctx_destroy(struct wist_ctx *ctx) {
    if (ctx == NULL) {
        return;
    }

    ctx->alloc_fn(ctx->alloc_ud, ctx, sizeof(struct wist_ctx), 0);
}

void *_wist_ctx_alloc(struct wist_ctx *ctx, size_t size) {
    void *ptr = ctx->alloc_fn(ctx->alloc_ud, NULL, 0, size);
    if (ptr == NULL)
    {
        /* TODO: Add OOM handling. */
    }
    return ptr;
}

void _wist_ctx_free(struct wist_ctx *ctx, void *ptr, size_t size) {
    ctx->alloc_fn(ctx->alloc_ud, ptr, size, 0);
}

void *_wist_ctx_realloc(struct wist_ctx *ctx, void *ptr, size_t osz, size_t nsz) {
    void *nptr = ctx->alloc_fn(ctx->alloc_ud, ptr, osz, nsz);
    if (nptr == NULL)
    {
        /* TODO: Add OOM handling. */
    }

    return nptr;
}

/* === PRIVATES === */

static void *libc_alloc(void *ud, void *ptr, size_t osz, size_t nsz) {
    IGNORE(ud);

    if (ptr == NULL && osz == 0) {
        return calloc(1, nsz);
    }

    if (ptr != NULL && nsz == 0) {
        free(ptr);
        return NULL;
    }

    return realloc(ptr, nsz);
}

