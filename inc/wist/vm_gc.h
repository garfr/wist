/* === inc/wist/vm_gc.h - VM garbage collector === 
 * Copyright (C) 2022 Gavin Ratcliff - All Rights Reserved
 * Part of the Wist reference implementation, under the MIT license.
 * See LICENSE.txt for license information.
*/

#ifndef _WIST_VM_GC_H
#define _WIST_VM_GC_H

#include <wist.h>
#include <wist/defs.h>

/* This is a fake garbage collector that just allocates a linked list of items. */

struct wist_vm_gc_hdr {
    uint8_t mark : 2;
    uint32_t size: 22;
    uint8_t tag : 8;
    struct wist_vm_gc_hdr *next;
};

struct wist_vm_gc {
    struct wist_ctx *ctx;
    struct wist_vm_gc_hdr *objs;
};


void wist_vm_gc_init(struct wist_ctx *ctx, struct wist_vm_gc *gc);
void wist_vm_gc_finish(struct wist_vm_gc *gc);

struct wist_vm_gc_hdr *wist_vm_gc_alloc(struct wist_vm_gc *gc, 
        size_t size);

#define WIST_VM_GC_ALLOC(_gc, _type)                                           \
    ((_type *) wist_vm_gc_alloc(_gc, sizeof(_type)))

#endif /* _WIST_VM_GC_H */
