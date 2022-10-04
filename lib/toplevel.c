/* === lib/toplevel.c - Toplevel state manager === 
 * Copyright (C) 2022 Gavin Ratcliff - All Rights Reserved
 * Part of the Wist reference implementation, under the MIT license.
 * See LICENSE.txt for license information.
*/

#include <wist/toplevel.h>

/* === PROTOTYPES === */

static void toplvl_scope_init(struct wist_ctx *ctx, 
        struct wist_toplvl_scope *scope);
static void toplvl_scope_finish(struct wist_ctx *ctx, 
        struct wist_toplvl_scope *scope);

/* === PUBLICS === */

void wist_toplvl_init(struct wist_ctx *ctx, struct wist_toplvl *toplvl) {
    toplvl->ctx = ctx;
    toplvl_scope_init(ctx, &toplvl->global);
}

void wist_toplvl_finish(struct wist_toplvl *toplvl) {
    toplvl_scope_finish(toplvl->ctx, &toplvl->global);
}

struct wist_toplvl_entry *wist_toplvl_add(struct wist_toplvl *toplvl, 
        struct wist_sym *sym) {
    struct wist_toplvl_entry tmp;
    struct wist_toplvl_entry *entry = 
        WIST_MAP_INSERT(toplvl->ctx, &toplvl->global.entries, &sym, &tmp, 
                struct wist_toplvl_entry);
    return entry;
}

struct wist_toplvl_entry *wist_toplvl_find(struct wist_toplvl *toplvl, 
        struct wist_sym *sym) {
    struct wist_toplvl_entry *entry = 
        WIST_MAP_FIND(toplvl->ctx, &toplvl->global.entries, &sym, 
                struct wist_toplvl_entry);
    return entry;
}

/* === PRIVATES === */

static void toplvl_scope_init(struct wist_ctx *ctx, 
        struct wist_toplvl_scope *scope) {
    WIST_SYM_MAP_INIT(ctx, &scope->entries, struct wist_toplvl_entry);
}

static void toplvl_scope_finish(struct wist_ctx *ctx, 
        struct wist_toplvl_scope *scope) {
    WIST_MAP_FINISH(ctx, &scope->entries);
}
