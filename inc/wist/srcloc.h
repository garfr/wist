/* === inc/wist/srcloc.h - Wist source locations === 
 * Copyright (C) 2022 Gavin Ratcliff - All Rights Reserved
 * Part of the Wist reference implementation, under the MIT license.
 * See LICENSE.txt for license information.
*/

#ifndef _WIST_SRCLOC_H
#define _WIST_SRCLOC_H

#include <wist/defs.h>
#include <wist/vector.h>

/* 
 * A source location to be embedded directly into the AST.  If the start or 
 * length cannot fit in a 32 bit integer, [len] is set to zero and [start] 
 * serves as an index into the wist_srcloc_index table associated with this 
 * srcloc. 
 */
struct wist_srcloc {
    uint16_t start, len; 
};

/* A "wide" source location, that can reference any location in the text. */
struct wist_wsrcloc {
    size_t start, end;
};

/* Maps between fake srclocs and their wide counterparts. */
struct wist_srcloc_index {
    struct wist_vector locs;
    struct wist_vector segments;
    size_t cur_segment, cur_base;
};

/* Initializes an index. */
void wist_srcloc_index_init(struct wist_ctx *ctx, 
        struct wist_srcloc_index *index);

/* Adds a text segment to an index. */
void wist_srcloc_index_add_segment(struct wist_ctx *ctx, 
        struct wist_srcloc_index *index, const uint8_t *src, size_t srclen);

/* Releases all data owned by an index. */
void wist_srcloc_index_finish(struct wist_ctx *ctx, struct wist_srcloc_index *index);

/* 
 * Converts two 64 bit indexes into a valid srcloc, handling fake srloc 
 * generation.
 */
struct wist_srcloc wist_srcloc_index_add(struct wist_ctx *ctx, 
        struct wist_srcloc_index *index, uint64_t start, uint64_t end);

/* Creates a new srcloc with all the info between [l1]'s start and [l2]'s end. */
struct wist_srcloc wist_srcloc_index_combine(struct wist_ctx *ctx,
        struct wist_srcloc_index *index, struct wist_srcloc l1, 
        struct wist_srcloc l2);

/* Returns a slice in the source code represented by the srcloc. */
const uint8_t *wist_srcloc_index_slice(struct wist_srcloc_index *index, 
        struct wist_srcloc loc, size_t *str_len_out);

#endif /* _WIST_SRCLOC_H */
