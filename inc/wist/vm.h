/* === inc/wist/vm.h - Wist interpreter virtual machine === 
 * Copyright (C) 2022 Gavin Ratcliff - All Rights Reserved
 * Part of the Wist reference implementation, under the MIT license.
 * See LICENSE.txt for license information.
*/

#ifndef _WIST_VM_H
#define _WIST_VM_H

#include <wist.h>

struct wist_vm {
    struct wist_ctx *ctx;
};

#endif /* _WIST_VM_H */
