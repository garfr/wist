/* === inc/wist/compiler.h - Wist compiler === 
 * Copyright (C) 2022 Gavin Ratcliff - All Rights Reserved
 * Part of the Wist reference implementation, under the MIT license.
 * See LICENSE.txt for license information.
*/

#ifndef _WIST_COMPILER_H
#define _WIST_COMPILER_H

#include <wist/ctx.h>
#include <wist/sym.h>
#include <wist/srcloc.h>

struct wist_compiler {
    struct wist_ctx *ctx;
    struct wist_parse_result *cur_result;
    struct wist_sym_index syms;
    struct wist_srcloc_index srclocs;
};

#endif /* _WIST_COMPILER_H */

