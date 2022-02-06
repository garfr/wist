#ifndef WIST_SYM_H
#define WIST_SYM_H

#include <wist/str.h>

typedef struct
{
    WistStr str;
} WistSym;

#define WIST_MAP_HEADER
#define WIST_MAP_KEY_TYPE WistSym
#define WIST_MAP_TYPE_PREFIX WistSym
#define WIST_MAP_FUN_PREFIX sym_table
#include <wist/map.c.h>

typedef WistSymMap WistSymTable;

WistSym *wist_sym_add(WistSymTable *tab, const uint8_t *str, size_t len);

#endif /* WIST_SYM_H */
