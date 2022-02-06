#include <wist/sym.h>

/* === PROTOTYPES === */

static bool compare_sym(WistSym *s1, WistSym *s2);
static uint32_t hash_sym(WistSym *s);
static void sym_destructor(WistSym *s);

/* === PUBLIC FUNCTIONS === */

#define WIST_MAP_KEY_TYPE WistSym
#define WIST_MAP_TYPE_PREFIX WistSym
#define WIST_MAP_FUN_PREFIX sym_table
#define WIST_MAP_EQ compare_sym
#define WIST_MAP_HASH hash_sym
#define WIST_MAP_DESTRUCTOR sym_destructor
#define WIST_MAP_IMPLEMENTATION
#include <wist/map.c.h>

/* === PRIVATE FUNCTIONS === */

static bool
compare_sym(WistSym *s1,
            WistSym *s2)
{
    return WIST_STREQ(s1->str, s2->str);
}

static uint32_t
hash_sym(WistSym *s)
{
    return wist_str_hash(wist_str_to_ref(s->str));
}

static void
sym_destructor(WistSym *s)
{
    wist_str_libc_free(s->str);
}

WistSym *
wist_sym_add(WistSymTable *tab, const uint8_t *str, size_t len)
{
    WistSym lookup;
    lookup.str.len = len;
    lookup.str.str = str;
    WistSym *sym = sym_table_find(tab, &lookup);
    if (sym == NULL)
    {
        lookup.str = wist_str_from_slice(str, len);
        return sym_table_insert(tab, &lookup);
    }
    return sym;
}
