/* === inc/wist/vm_ops.h - VM opcodes === 
 * Copyright (C) 2022 Gavin Ratcliff - All Rights Reserved
 * Part of the Wist reference implementation, under the MIT license.
 * See LICENSE.txt for license information.
*/

/* There is no include guard, because this is meant to be used to generate code. */

OPCODE(INT64, 8)
OPCODE(RETURN, 0)
OPCODE(CLOSURE, 2)
OPCODE(PUSH, 0)
OPCODE(PUSHMARK, 0)
OPCODE(ACCESS, 1)
OPCODE(APPLY, 0)
OPCODE(APPTERM, 0)
OPCODE(MKB, 2)
OPCODE(GRAB, 0)
