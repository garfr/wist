/* === inc/wist/lexer.h - Wist lexer === 
 * Copyright (C) 2022 Gavin Ratcliff - All Rights Reserved
 * Part of the Wist reference implementation, under the MIT license.
 * See LICENSE.txt for license information.
*/

#ifndef _WIST_LEXER_H
#define _WIST_LEXER_H

#include <wist.h>
#include <wist/srcloc.h>
#include <wist/sym.h>

enum wist_token_kind {
    WIST_TOKEN_BACKSLASH,  /* \\ */
    WIST_TOKEN_SYM,        /* [a-zA-Z][a-zA-Z0-9_]* */
    WIST_TOKEN_INT, /* -?[0-9] */
    WIST_TOKEN_THIN_ARROW, /* -> */
    WIST_TOKEN_LPAREN,     /* ( */
    WIST_TOKEN_RPAREN,     /* ) */


    WIST_TOKEN_EOI,
};

struct wist_token {
    enum wist_token_kind t;
    struct wist_srcloc loc;

    union {
        struct wist_sym *sym;
        int64_t i;
    };
};

struct wist_token *wist_lex(struct wist_compiler *compiler, const uint8_t *src, 
        size_t src_len, size_t *tokens_len_out);

void wist_token_print(struct wist_compiler *comp, struct wist_token token);

#endif /* _WIST_LEXER_H */
