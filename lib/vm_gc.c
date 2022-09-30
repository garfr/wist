/* === lib/vm_gc.c - VM garbage collector === 
 * Copyright (C) 2022 Gavin Ratcliff - All Rights Reserved
 * Part of the Wist reference implementation, under the MIT license.
 * See LICENSE.txt for license information.
*/

#include <wist/vm_gc.h>
#include <wist/vm_obj.h>
#include <wist/ctx.h>

void wist_vm_gc_init(struct wist_ctx *ctx, struct wist_vm_gc *gc) {
    gc->ctx = ctx;
    gc->objs = NULL;
}

void wist_vm_gc_finish(struct wist_vm_gc *gc) {
    struct wist_vm_gc_hdr *iter = gc->objs, *follow = NULL; 

    while (iter != NULL) {
        follow = iter;
        iter = iter->next;
        WIST_CTX_FREE_ARR(gc->ctx, follow, uint8_t, 
                follow->field_count * sizeof(struct wist_vm_obj) 
              + sizeof(struct wist_vm_gc_hdr));
    }
}

struct wist_vm_gc_hdr *wist_vm_gc_alloc(struct wist_vm_gc *gc, 
        size_t field_count) {
    struct wist_vm_gc_hdr *new = 
        (struct wist_vm_gc_hdr *) WIST_CTX_NEW_ARR(gc->ctx, uint8_t, 
                sizeof(struct wist_vm_gc_hdr) 
              + sizeof(struct wist_vm_obj) * field_count);
    new->field_count = field_count;
    new->mark = 0x0;
    new->tag = 0;
    return new;
}
