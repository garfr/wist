/* === inc/wist/sema.h - AST semantics analysis === 
 * Copyright (C) 2022 Gavin Ratcliff - All Rights Reserved
 * Part of the Wist reference implementation, under the MIT license.
 * See LICENSE.txt for license information.
*/

#ifndef _WIST_SEMA_H
#define _WIST_SEMA_H

#include <wist.h>

bool wist_sema_infer_expr(struct wist_compiler *comp, 
        struct wist_ast_expr *expr);
bool wist_sema_infer_decl(struct wist_compiler *comp, 
        struct wist_ast_decl *decl);

#endif /* _WIST_SEMA_H */
