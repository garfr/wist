/* === inc/wist/toplevel.h - Toplevel state manager === 
 * Copyright (C) 2022 Gavin Ratcliff - All Rights Reserved
 * Part of the Wist reference implementation, under the MIT license.
 * See LICENSE.txt for license information.
*/

#ifndef _WIST_TOPLEVEL_H
#define _WIST_TOPLEVEL_H

#include <wist/map.h>
#include <wist/ast.h>
#include <wist/vm_obj.h>

struct wist_toplvl_entry {
    struct wist_ast_type *type;
    struct wist_vm_obj val;
};

struct wist_toplvl_scope {
    struct wist_map entries;
};

struct wist_toplvl {
    struct wist_toplvl_scope global;
    struct wist_ctx *ctx;
};


void wist_toplvl_init(struct wist_ctx *ctx, struct wist_toplvl *toplvl);
void wist_toplvl_finish(struct wist_toplvl *toplvl);

struct wist_toplvl_entry *wist_toplvl_add(struct wist_toplvl *toplvl, 
        struct wist_sym *sym);
struct wist_toplvl_entry *wist_toplvl_find(struct wist_toplvl *toplvl, 
        struct wist_sym *sym);

#endif /* _WIST_TOPLEVEL_H */
