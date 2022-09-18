/* === lib/srcloc.c - Wist source locations === 
 * Copyright (C) 2022 Gavin Ratcliff - All Rights Reserved
 * Part of the Wist reference implementation, under the MIT license.
 * See LICENSE.txt for license information.
*/

#include <wist/srcloc.h>

void wist_srcloc_index_init(struct wist_ctx *ctx, 
        struct wist_srcloc_index *index, const uint8_t *src, size_t src_len) {
    index->src = src;
    index->src_len = src_len;
    WIST_VECTOR_INIT(ctx, &index->locs, struct wist_wsrcloc);
}

void wist_srcloc_index_finish(struct wist_ctx *ctx, struct wist_srcloc_index *index) {
    WIST_VECTOR_FINISH(ctx, &index->locs);
}

struct wist_srcloc wist_srcloc_index_add(struct wist_ctx *ctx, 
        struct wist_srcloc_index *index, uint64_t start, uint64_t end) {
    struct wist_srcloc loc;
    if (start > UINT32_MAX || end > UINT32_MAX)
    {
        struct wist_wsrcloc wloc = {
            .start = start,
            .end = end,
        };
        WIST_VECTOR_PUSH(ctx, &index->locs, struct wist_wsrcloc, &wloc);
        loc.start = WIST_VECTOR_LEN(&index->locs, struct wist_wsrcloc);
        loc.len = 0;
    } else {
        loc.start = (uint32_t) start;
        loc.len = (uint32_t) (end - start);
    }

    return loc;
}

const uint8_t *wist_srcloc_index_slice(struct wist_srcloc_index *index, 
        struct wist_srcloc loc, size_t *str_len_out) {
    uint64_t start;
    if (loc.len == 0)
    {
        struct wist_wsrcloc *wloc = 
            WIST_VECTOR_INDEX(&index->locs, struct wist_wsrcloc, 
                    (size_t) loc.start);
        start = wloc->start;
        *str_len_out = wloc->end - wloc->start;
    } else {
        start = loc.start;
        *str_len_out = loc.len;
    }
    
    return index->src + start;
}
