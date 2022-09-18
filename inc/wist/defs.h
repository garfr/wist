/* === inc/wist/defs.h - Helpful misc. definitions === 
 * Copyright (C) 2022 Gavin Ratcliff - All Rights Reserved
 * Part of the Wist reference implementation, under the MIT license.
 * See LICENSE.txt for license information.
*/

#ifndef _WIST_DEFS_H
#define _WIST_DEFS_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#define IGNORE(_name) (void) _name

#define SLICE_EQ(_s1, _sl1, _s2, _sl2)                                          \
        ((_sl1) == (_sl2)                                                       \
     && strncmp((const char *) (_s1), (const char *) (_s2), _sl1))

#endif /* _WIST_DEFS_H */
