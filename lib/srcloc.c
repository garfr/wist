/* === lib/srcloc.c - Wist source locations === 
 * Copyright (C) 2022 Gavin Ratcliff - All Rights Reserved
 * Part of the Wist reference implementation, under the MIT license.
 * See LICENSE.txt for license information.
*/

#include <wist/srcloc.h>

#include <stdio.h>

/* 
 * Whenever we do operations on a srcloc, we must be careful to use the 
 * correct start and end position, and check if the srcloc is a "fake" srcloc 
 * that indexes a wsrcloc. 
 */

struct srcloc_segment {
    const uint8_t *src;
    size_t src_len;
};

void wist_srcloc_index_init(struct wist_ctx *ctx, 
        struct wist_srcloc_index *index) {
    WIST_VECTOR_INIT(ctx, &index->locs, struct wist_wsrcloc);
    WIST_VECTOR_INIT(ctx, &index->segments, struct srcloc_segment);
}

/* Adds a text segment to an index. */
void wist_srcloc_index_add_segment(struct wist_ctx *ctx, 
        struct wist_srcloc_index *index, const uint8_t *src, size_t src_len) {
    struct srcloc_segment segment = {
        .src = src,
        .src_len = src_len,
    };

    index->cur_segment = WIST_VECTOR_LEN(&index->segments, struct srcloc_segment);
    if (index->cur_segment > 1) {
        index->cur_base += WIST_VECTOR_INDEX(&index->segments, 
                struct srcloc_segment, index->cur_segment - 1)->src_len;
    }

    WIST_VECTOR_PUSH(ctx, &index->segments, struct srcloc_segment, &segment);
}

void wist_srcloc_index_finish(struct wist_ctx *ctx, struct wist_srcloc_index *index) {
    WIST_VECTOR_FINISH(ctx, &index->locs);
}

struct wist_srcloc wist_srcloc_index_add(struct wist_ctx *ctx, 
        struct wist_srcloc_index *index, uint64_t start, uint64_t end) {
    struct wist_srcloc loc;
    start += index->cur_base;
    end += index->cur_base;

    if (start > UINT16_MAX || end > UINT16_MAX)
    {
        struct wist_wsrcloc wloc = {
            .start = start,
            .end = end,
        };
        WIST_VECTOR_PUSH(ctx, &index->locs, struct wist_wsrcloc, &wloc);
        loc.start = WIST_VECTOR_LEN(&index->locs, struct wist_wsrcloc);
        loc.len = 0;
    } else {
        loc.start = (uint16_t) start;
        loc.len = (uint16_t) (end - start);
    }

    return loc;
}

struct wist_srcloc wist_srcloc_index_combine(struct wist_ctx *ctx,
        struct wist_srcloc_index *index, struct wist_srcloc l1,
        struct wist_srcloc l2) {
    uint64_t start, end;
    if (l1.len == 0)
    {
        start = WIST_VECTOR_INDEX(&index->locs, struct wist_wsrcloc, l1.start)->start;
    } else
    {
        start = (uint64_t) l1.start;
    }

    if (l2.len == 0)
    {
        end = WIST_VECTOR_INDEX(&index->locs, struct wist_wsrcloc, l2.start)->end;
    } else
    {
        end = (uint64_t) (l2.start + l2.len);
    }

    return wist_srcloc_index_add(ctx, index, start, end);
}

const uint8_t *wist_srcloc_index_slice(struct wist_srcloc_index *index, 
        struct wist_srcloc loc, size_t *str_len_out) {
    size_t idx = 0;
    uint64_t start;
    uint64_t end;
    if (loc.len == 0)
    {
        struct wist_wsrcloc *wloc = 
            WIST_VECTOR_INDEX(&index->locs, struct wist_wsrcloc, 
                    (size_t) loc.start);
        start = wloc->start;
        end = wloc->end;
    } else {
        start = loc.start;
        end = start + loc.len;
    }
    
    WIST_VECTOR_FOR_EACH(&index->segments, struct srcloc_segment, segment) {
        if (idx + segment->src_len >= start) {
            *str_len_out = end - start;
            return segment->src + (start - idx);
        }
        idx += segment->src_len;
    }
    printf("Couldn't find segment. \n");
    return NULL;
}
