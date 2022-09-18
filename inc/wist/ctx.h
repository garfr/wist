/* === inc/wist/ctx.h - Wist context === 
 * Copyright (C) 2022 Gavin Ratcliff - All Rights Reserved
 * Part of the Wist reference implementation, under the MIT license.
 * See LICENSE.txt for license information.
*/

#ifndef _WIST_CTX_H
#define _WIST_CTX_H

#include <wist.h>

struct wist_ctx {
    wist_alloc_fn alloc_fn;
    void *alloc_ud;
};

/* Memory allocation utilities. */

void *_wist_ctx_alloc(struct wist_ctx *ctx, size_t size);
void _wist_ctx_free(struct wist_ctx *ctx, void *ptr, size_t size);
void *_wist_ctx_realloc(struct wist_ctx *ctx, void *ptr, size_t osz, size_t nsz);

#define WIST_CTX_NEW(_ctx, _type)                                              \
    ((_type *) _wist_ctx_alloc(_ctx, sizeof(_type)))
#define WIST_CTX_NEW_ARR(_ctx, _type, _len)                                    \
    ((_type *) _wist_ctx_alloc(_ctx, sizeof(_type) * (_len)))

#define WIST_CTX_FREE(_ctx, _ptr, _type)                                       \
    _wist_ctx_free(_ctx, _ptr, sizeof(_type))
#define WIST_CTX_FREE_ARR(_ctx, _ptr, _type, _len)                             \
    _wist_ctx_free(_ctx, _ptr, sizeof(_type) * (_len))

#define WIST_CTX_RESIZE(_ctx, _ptr, _type, _olen, _nlen)                       \
    ((_type *) _wist_ctx_realloc(_ctx, _ptr, sizeof(_type) * (_olen),          \
                                            sizeof(_type) * (_nlen)))


#endif /* _WIST_CTX_H */
