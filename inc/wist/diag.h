/* === inc/wist/diag.h - Wist compiler diagnostics === 
 * Copyright (C) 2022 Gavin Ratcliff - All Rights Reserved
 * Part of the Wist reference implementation, under the MIT license.
 * See LICENSE.txt for license information.
*/

#ifndef _WIST_DIAG_H
#define _WIST_DIAG_H

#include <wist/lexer.h>

/* 
 * Each diagnostic kind has a correct number of important source locations 
 * and an associated error message. 
 */
enum wist_diag_kind {
    WIST_DIAG_UNEXPECTED_CHAR,
    WIST_DIAG_EXPECTED_EXPR,
    WIST_DIAG_EXPECTED_TOKEN,
};

enum wist_diag_level {
    WIST_DIAG_INFO,
    WIST_DIAG_WARNING,
    WIST_DIAG_ERROR,
    WIST_DIAG_FATAL,
};

struct wist_diag {
    enum wist_diag_kind t;      /* What kind of diagnostic this is. */
    enum wist_diag_level level; /* Importance of diagnostic. */
    struct wist_vector locs;    /* Set of locations in source code to be displayed. */

    union {
        enum wist_token_kind expected_token;
    };
};

#endif /* _WIST_DIAG_H */
