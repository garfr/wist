/* === lib/sym.c - Wist symbols === 
 * Copyright (C) 2022 Gavin Ratcliff - All Rights Reserved
 * Part of the Wist reference implementation, under the MIT license.
 * See LICENSE.txt for license information.
*/

#include <wist/sym.h>
#include <wist/defs.h>
#include <wist/ctx.h>

#include <stdio.h>

void wist_sym_index_init(struct wist_sym_index *index) {
    index->syms = NULL;
}

void wist_sym_index_finish(struct wist_ctx *ctx, struct wist_sym_index *index) {
    struct wist_sym *first = index->syms, *follow;

    while (first != NULL) {
        follow = first;
        first = first->next;
        WIST_CTX_FREE_ARR(ctx, (uint8_t *) follow->str, uint8_t, follow->str_len);
        WIST_CTX_FREE(ctx, follow, struct wist_sym);
    }
}

struct wist_sym *wist_sym_index_search(struct wist_ctx *ctx, 
        struct wist_sym_index *index, const uint8_t *str, size_t str_len) {
    struct wist_sym *iter = index->syms;
    while (iter != NULL) {
        if (STREQ(iter->str, iter->str_len, str, str_len)) {
            return iter;
        }
        iter = iter->next;
    }

    struct wist_sym *new = WIST_CTX_NEW(ctx, struct wist_sym);
    new->str_len = str_len;
    uint8_t *new_str = WIST_CTX_NEW_ARR(ctx, uint8_t, str_len);
    memcpy(new_str, str, str_len);
    new->str = new_str;
    new->next = index->syms;
    index->syms = new;
    return new;
}
