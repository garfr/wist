/*
 * Must define before including. They will be undefined at the end of this file:
 *
 * WIST_MAP_KEY_TYPE
 * WIST_MAP_FUN_PREFIX
 * WIST_MAP_TYPE_PREFIX
 * WIST_MAP_HASH
 * WIST_MAP_EQ
 *
 * Optionals to define:
 * WIST_MAP_VAL_TYPE
 * WIST_MAP_DESTRUCTOR
 */

#include <stddef.h>

#include <wist/mem.h>

#define CAT(a, b) a##b
#define PASTE(a, b) CAT(a, b)
#define JOIN(prefix, name) PASTE(prefix, PASTE(_, name))

#define TMANGLE(name) PASTE(WIST_MAP_TYPE_PREFIX, name)
#define FMANGLE(name) JOIN(WIST_MAP_FUN_PREFIX, name)

#define MAP_TYPE TMANGLE(Map)
#define MAP_ENTRY_TYPE TMANGLE(MapEntry)
#define MAP_ITER_TYPE TMANGLE(MapIter)
#define KEY_TYPE WIST_MAP_KEY_TYPE

#ifdef WIST_MAP_VAL_TYPE
#define VAL_TYPE WIST_MAP_VAL_TYPE
#endif /* WIST_MAP_KEY_TYPE */

#ifdef WIST_MAP_HEADER

typedef struct MAP_ENTRY_TYPE
{
    KEY_TYPE key;
#ifdef VAL_TYPE
    VAL_TYPE val;
#endif /* VAL_TYPE */
    struct MAP_ENTRY_TYPE *next;
} MAP_ENTRY_TYPE;

typedef struct
{
    MAP_ENTRY_TYPE **buckets;
    size_t nbuckets;
    size_t buckets_filled;
} MAP_TYPE;

typedef struct
{
    size_t idx;
    MAP_ENTRY_TYPE *cur;
    MAP_TYPE *map;
} MAP_ITER_TYPE;

void FMANGLE(create)(MAP_TYPE *out);
void FMANGLE(destroy)(MAP_TYPE *map);

MAP_ENTRY_TYPE *FMANGLE(iter_fnext)(MAP_ITER_TYPE *);
MAP_ITER_TYPE FMANGLE(iter_create)(MAP_TYPE *);

#ifdef VAL_TYPE
VAL_TYPE *FMANGLE(iter_next)(MAP_ITER_TYPE *);
VAL_TYPE *FMANGLE(insert)(MAP_TYPE *map, KEY_TYPE *key, VAL_TYPE *val);
VAL_TYPE *FMANGLE(find)(MAP_TYPE *map, KEY_TYPE *key);
#else
KEY_TYPE *FMANGLE(iter_next)(MAP_ITER_TYPE *);
KEY_TYPE *FMANGLE(insert)(MAP_TYPE *map, KEY_TYPE *key);
KEY_TYPE *FMANGLE(find)(MAP_TYPE *map, KEY_TYPE *key);
#endif /* VAL_TYPE */

#endif /* WIST_MAP_HEADER */

#ifdef WIST_MAP_IMPLEMENTATION

#ifndef INIT_BUCKETS
#define INIT_BUCKETS 8
#endif /* INIT_BUCKETS */

void
FMANGLE(create)(MAP_TYPE *out)
{
    out->buckets = WIST_NEW_ARR(MAP_ENTRY_TYPE *, INIT_BUCKETS);
    out->nbuckets = INIT_BUCKETS;
    out->buckets_filled = 0;
}

void
FMANGLE(destroy)(MAP_TYPE *map)
{
    for (size_t i = 0; i < map->nbuckets; i++)
    {
        MAP_ENTRY_TYPE *entry = map->buckets[i];
        while (entry != NULL)
        {
            MAP_ENTRY_TYPE *temp = entry;
            entry = entry->next;
#ifdef WIST_MAP_DESTRUCTOR
#ifdef VAL_TYPE
            WIST_MAP_DESTRUCTOR(&temp->key, &temp->val);
#else
            WIST_MAP_DESTRUCTOR(&temp->key);
#endif /* VAL_TYPE */
#endif /* WIST_MAP_DESTRUCTOR */
            WIST_FREE(temp);
        }
    }
    WIST_FREE(map->buckets);
}

#ifdef VAL_TYPE
VAL_TYPE *
FMANGLE(insert)(MAP_TYPE *map,
                          KEY_TYPE *key,
                          VAL_TYPE *val)
#else
KEY_TYPE *
FMANGLE(insert)(MAP_TYPE *map, KEY_TYPE *key)
#endif /* VAL_TYPE */
{
    uint32_t hash = WIST_MAP_HASH(key);
    size_t idx = hash % map->nbuckets;
    MAP_ENTRY_TYPE *iter = map->buckets[idx];
    if (iter == NULL)
    {
        map->buckets_filled++;
    }
    while (iter != NULL)
    {
        if (WIST_MAP_EQ(key, &iter->key))
        {
#ifdef VAL_TYPE
            return &iter->val;
#else
            return &iter->key;
#endif /* VAL_TYPE */
        }
        iter = iter->next;
    }

    MAP_ENTRY_TYPE *new = WIST_NEW(MAP_ENTRY_TYPE);
    new->key = *key;
#ifdef VAL_TYPE
    new->val = *val;
#endif /* VAL_TYPE */
    new->next = map->buckets[idx];

    map->buckets[idx] = new;
#ifdef VAL_TYPE
    return &new->val;
#else
    return &new->key;
#endif /* VAL_TYPE */
}

#ifdef VAL_TYPE
VAL_TYPE *
#else
KEY_TYPE *
#endif /* VAL_TYPE */
FMANGLE(iter_next)(MAP_ITER_TYPE *iter)
{
    #ifdef VAL_TYPE
    return &FMANGLE(iter_fnext)(iter)->val;
    #else
    return &FMANGLE(iter_fnext)(iter)->key;
    #endif /* VAL_TYPE */
}

MAP_ENTRY_TYPE *
FMANGLE(iter_fnext)(MAP_ITER_TYPE *iter)
{
    if (iter->cur == NULL)
    {
        if (++iter->idx >= iter->map->nbuckets)
        {
            return NULL;
        }
        iter->cur = iter->map->buckets[iter->idx];
        return FMANGLE(iter_fnext)(iter);
    }
    MAP_ENTRY_TYPE *ret = iter->cur;
    iter->cur = iter->cur->next;
    return ret;
}

MAP_ITER_TYPE 
FMANGLE(iter_create)(MAP_TYPE *map)
{
    MAP_ITER_TYPE ret = {
        .idx = 0,
        .cur = map->buckets[0],
        .map = map,
    };
    return ret;
}

#ifdef VAL_TYPE
VAL_TYPE *
#else
KEY_TYPE *
#endif /* VAL_TYPE */
FMANGLE(find)(MAP_TYPE *map,
              KEY_TYPE *key)
{
    uint32_t hash = WIST_MAP_HASH(key);
    size_t idx = hash % map->nbuckets;

    MAP_ENTRY_TYPE *iter = map->buckets[idx];
    while (iter != NULL)
    {
        if (WIST_MAP_EQ(key, &iter->key))
        {
#ifdef VAL_TYPE
            return &iter->val;
#else
            return &iter->key;
#endif /* VAL_TYPE */
        }
        iter = iter->next;
    }

    return NULL;
}

#endif /* WIST_MAP_IMPLEMENTATION */

#undef WIST_MAP_IMPLEMENTATION
#undef WIST_MAP_KEY_TYPE
#undef WIST_MAP_VAL_TYPE
#undef WIST_MAP_FUN_PREFIX
#undef WIST_MAP_TYPE_PREFIX
#undef MAP_TYPE
#undef TMANGLE
#undef FMANGLE
#ifdef VAL_TYPE
#undef VAL_TYPE
#endif /* VAL_TYPE */

#ifdef WIST_MAP_DESTRUCTOR
#undef WIST_MAP_DESTRUCTOR
#endif /* WIST_MAP_DESTRUCTOR */
#undef KEY_TYPE
#undef MAP_ENTRY_TYPE
#undef WIST_MAP_HASH
#undef WIST_MAP_EQ
#undef PASTE
#undef CAT
#undef JOIN
#undef WIST_MAP_HEADER

