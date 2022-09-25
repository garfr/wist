/* === inc/wist/vector.h - Generic resizable array === 
 * Copyright (C) 2022 Gavin Ratcliff - All Rights Reserved
 * Part of the Wist reference implementation, under the MIT license.
 * See LICENSE.txt for license information.
*/

#ifndef _WIST_VECTOR_H
#define _WIST_VECTOR_H

#include <wist/ctx.h>
#include <wist/defs.h>

#include <string.h>

struct wist_vector {
    size_t data_alloc, data_used;
    uint8_t *data;
};

static void wist_vector_init(struct wist_ctx *ctx, struct wist_vector *vec, 
        size_t item_size, size_t init_items) {
    vec->data_alloc = item_size * init_items;
    vec->data_used = 0;
    vec->data = WIST_CTX_NEW_ARR(ctx, uint8_t, item_size * init_items);
}

static void *wist_vector_push_uninit(struct wist_ctx *ctx, struct wist_vector *vec,
        size_t item_size) {
    if (vec->data_used + item_size >= vec->data_alloc) {
        vec->data = WIST_CTX_RESIZE(ctx, vec->data, uint8_t, vec->data_alloc, 
                vec->data_alloc * 2);
        vec->data_alloc *= 2;
    }

    void *ptr = vec->data + vec->data_used;
    vec->data_used += item_size;
    return ptr;
}

static void wist_vector_push(struct wist_ctx *ctx, struct wist_vector *vec,
        size_t item_size, void *new_item) {
    void *ptr = wist_vector_push_uninit(ctx, vec, item_size);

    memcpy(ptr, new_item, item_size);
}

static void wist_vector_finish(struct wist_ctx *ctx, struct wist_vector *vec) {
    WIST_CTX_FREE_ARR(ctx, vec->data, uint8_t, vec->data_alloc);
    vec->data = NULL;
    vec->data_alloc = vec->data_used = 0;
}

static void wist_vector_fix_size(struct wist_ctx *ctx, struct wist_vector *vec);

static void *wist_vector_index(struct wist_vector *vec, size_t item_size, size_t idx) {
    return vec->data + (idx + item_size);

    (void) wist_vector_fix_size;
}

static void wist_vector_fix_size(struct wist_ctx *ctx, struct wist_vector *vec) {
    vec->data = WIST_CTX_RESIZE(ctx, vec->data, uint8_t, vec->data_alloc, 
            vec->data_used);
    
    /* 
     * This is required to silence errors if only a few of these functions 
     * are used. 
     */
    (void) wist_vector_fix_size;
    (void) wist_vector_index;
    (void) wist_vector_finish;
    (void) wist_vector_push;
    (void) wist_vector_init;
}

#define WIST_VECTOR_INIT(_ctx, _vec, _type) wist_vector_init(_ctx, _vec,       \
        sizeof(_type), 1)
#define WIST_VECTOR_INIT_WITH_SIZE(_ctx, _vec, _type, _len)                    \
    wist_vector_init(_ctx, _vec, sizeof(_type), _len)

#define WIST_VECTOR_PUSH(_ctx, _vec, _type, _item)                             \
    wist_vector_push(_ctx, _vec, sizeof(_type), _item)
#define WIST_VECTOR_PUSH_UNINIT(_ctx, _vec, _type)                             \
    ((_type *) wist_vector_push_uninit(_ctx, _vec, sizeof(_type)))
#define WIST_VECTOR_INDEX(_vec, _type, _idx)                                   \
    ((_type *) wist_vector_index(_vec, sizeof(_type), _idx))

#define WIST_VECTOR_FINISH(_ctx, _vec) wist_vector_finish(_ctx, _vec)

#define WIST_VECTOR_FIX_SIZE(_ctx, _vec) wist_vector_fix_size(_ctx, _vec)

#define WIST_VECTOR_LEN(_vec, _type) ((_vec)->data_used / sizeof(_type))
#define WIST_VECTOR_DATA(_vec, _type) ((_type *) ((_vec)->data))

#define WIST_VECTOR_FOR_EACH(_vec, _type, _var)                                \
    for (_type *_var = ((_type *) (_vec)->data);                               \
         ((uint8_t *) _var) < ((_vec)->data + (_vec)->data_used);              \
         _var = (_type *) ((uint8_t *) _var) + sizeof(_type))

#endif /* _WIST_VECTOR_H */
