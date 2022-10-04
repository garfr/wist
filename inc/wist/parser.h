/* === inc/wist/parser.h - Wist parser === 
 * Copyright (C) 2022 Gavin Ratcliff - All Rights Reserved
 * Part of the Wist reference implementation, under the MIT license.
 * See LICENSE.txt for license information.
*/

#ifndef _WIST_PARSER_H
#define _WIST_PARSER_H

#include <wist.h>
#include <wist/lexer.h>

struct wist_ast_expr *wist_parse_expr(struct wist_compiler *comp, 
        struct wist_token *tokens, size_t tokens_len);
struct wist_ast_decl *wist_parse_decl(struct wist_compiler *comp, 
        struct wist_token *tokens, size_t tokens_len);

#endif /* _WIST_PARSER_H */
