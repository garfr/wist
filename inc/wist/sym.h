/* === inc/wist/sym.h - Wist symbols === 
 * Copyright (C) 2022 Gavin Ratcliff - All Rights Reserved
 * Part of the Wist reference implementation, under the MIT license.
 * See LICENSE.txt for license information.
*/

#ifndef _WIST_SYM_H
#define _WIST_SYM_H

#include <wist.h>

/* 
 * A symbol is a unique string of characters, interned by the lexer and used 
 * by the rest of the compiler to make string handling easier.  Two symbols 
 * containing the same string will be the same symbol, making equality as 
 * cheap as pointer comparison. 
 */

struct wist_sym {
    const uint8_t *str;
    size_t str_len;
    struct wist_sym *next; /* TODO: Use an actual hashmap to speed up symbol interning. */
};

/* Manages all existing symbols. */
struct wist_sym_index {
    struct wist_sym *syms;
};

/* Initializes a new (empty) symbol index. */
void wist_sym_index_init(struct wist_sym_index *index);

/* Releases the symbol index. */
void wist_sym_index_finish(struct wist_ctx *ctx, struct wist_sym_index *index);

/*
 * Looks for matching string in the table, if not found it inserts it and 
 * returns it. 
 */
struct wist_sym *wist_sym_index_search(struct wist_ctx *ctx, 
        struct wist_sym_index *index, const uint8_t *str, size_t str_len);

#endif /* _WIST_SYM_H */
