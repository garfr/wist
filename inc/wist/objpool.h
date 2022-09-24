/* === inc/wist/objpool.h - Group object allocator ===
 * Copyright (C) 2022 Gavin Ratcliff - All Rights Reserved
 * Part of the Wist reference implementation, under the MIT license.
 * See LICENSE.txt for license information.
*/

#ifndef _WIST_OBJPOOL_H
#define _WIST_OBJPOOL_H

#include <wist.h>

struct wist_objpool {
    struct wist_ctx *ctx;
    struct wist_objpool_chunk *chunk;
    struct wist_objpool_free *free;
    size_t obj_size, chunk_size;
};

/* Prefer the "typesafe" macros to to the _wist_objpool_* functions. */
void _wist_objpool_init(struct wist_ctx *ctx, struct wist_objpool *pool, size_t obj_size);
void *_wist_objpool_alloc(struct wist_objpool *pool);

/* You can call these directly. */
void wist_objpool_finish(struct wist_objpool *pool);
void wist_objpool_free(struct wist_objpool *pool, void *ptr);

#define WIST_OBJPOOL_INIT(_ctx, _pool, _type) _wist_objpool_init(_ctx, _pool, sizeof(_type))
#define WIST_OBJPOOL_ALLOC(_pool, _type) ((_type *) _wist_objpool_alloc(_pool))

#endif /* _WIST_OBJPOOL_H */
