/* === lib/objpool.h - Group object allocator ===
 * Copyright (C) 2022 Gavin Ratcliff - All Rights Reserved
 * Part of the Wist reference implementation, under the MIT license.
 * See LICENSE.txt for license information.
*/

#include <wist/objpool.h>
#include <wist/ctx.h>

struct wist_objpool_free {
    struct wist_objpool_free *next;
};

struct wist_objpool_chunk {
    size_t used;
    struct wist_objpool_chunk *next;
    uint8_t *data;
};

/* === PROTOTYPES === */

static struct wist_objpool_chunk *new_chunk(struct wist_objpool *pool);

/* === PUBLICS === */

void _wist_objpool_init(struct wist_ctx *ctx, struct wist_objpool *pool, 
        size_t obj_size) {
    pool->ctx = ctx;

    if (obj_size < sizeof(struct wist_objpool_free)) {
        obj_size = sizeof(struct wist_objpool_free);
    }

    pool->obj_size = obj_size;
    pool->chunk_size = obj_size * 64;

    pool->free = NULL;
    pool->chunk = NULL;
    pool->chunk = new_chunk(pool);
}

void *_wist_objpool_alloc(struct wist_objpool *pool) {
    if (pool->free == NULL) {
        new_chunk(pool);
    }

    void *ptr = pool->free;
    pool->free = pool->free->next;
    return ptr;
}

void wist_objpool_finish(struct wist_objpool *pool) {
    struct wist_objpool_chunk *iter = pool->chunk, *follow = NULL;
    while (iter != NULL){
        follow = iter;
        iter = iter->next;
        WIST_CTX_FREE_ARR(pool->ctx, follow->data, uint8_t, pool->chunk_size);
        WIST_CTX_FREE(pool->ctx, follow, struct wist_objpool_chunk);
    }
}

void wist_objpool_free(struct wist_objpool *pool, void *ptr) {
    struct wist_objpool_free *free = (struct wist_objpool_free *) ptr;
    free->next = pool->free;
    pool->free = free;
}

/* === PRIVATES === */

static struct wist_objpool_chunk *new_chunk(struct wist_objpool *pool) {
    struct wist_objpool_chunk *chunk = WIST_CTX_NEW(pool->ctx, struct wist_objpool_chunk);
    chunk->data = WIST_CTX_NEW_ARR(pool->ctx, uint8_t, pool->chunk_size);
    for (size_t byte = 0; byte < pool->chunk_size; byte += pool->obj_size) {
        struct wist_objpool_free *free = (struct wist_objpool_free *) &chunk->data[byte];
        free->next = pool->free;
        pool->free = free;
    }

    chunk->used = 0;
    chunk->next = pool->chunk;
    pool->chunk = chunk;
    return chunk;
}
