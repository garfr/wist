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
 * A source location to be embedded directly into the AST.  If the data cannot 
 * fit in a 32 bit integer, [len] is set to zero and [start] serves as an 
 * index into the wist_srcloc_index table associated with this srcloc. 
 */
struct wist_srcloc {
    uint32_t start, len; 
};

struct wist_wsrcloc {
    size_t start, end;
};

struct wist_srcloc_index {
    struct wist_vector locs;
    const uint8_t *src;
    size_t src_len;
};

/* Initializes a srcloc_index. */
void wist_srcloc_index_init(struct wist_ctx *ctx, 
        struct wist_srcloc_index *index, const uint8_t *src, size_t src_len);

/* Releases all data owned by a srcloc_index. */
void wist_srcloc_index_finish(struct wist_ctx *ctx, struct wist_srcloc_index *index);

/* 
 * Converts two 64 bit indexes into a valid srcloc, either a pointer to a 
 * wsrcloc or an inline srcloc. 
 */
struct wist_srcloc wist_srcloc_index_add(struct wist_ctx *ctx, 
        struct wist_srcloc_index *index, uint64_t start, uint64_t end);

/* Returns a slice in the source code represented by the srcloc. */
const uint8_t *wist_srcloc_index_slice(struct wist_srcloc_index *index, 
        struct wist_srcloc loc, size_t *str_len_out);

#endif /* _WIST_SRCLOC_H */
