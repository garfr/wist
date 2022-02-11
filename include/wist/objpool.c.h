/*
 * Must define before including:
 *
 * WIST_OBJPOOL_TYPE
 * WIST_OBJPOOL_FUN_PREFIX
 * WIST_OBJPOOL_TYPE_PREFIX
 * 
 * Optionals:
 * WIST_OBJPOOL_DESTRUCTOR
 */

#include <wist/mem.h>

#define CAT(a, b) a##b
#define PASTE(a, b) CAT(a, b)
#define JOIN(prefix, name) PASTE(prefix, PASTE(_, name))

#define TMANGLE(name) PASTE(WIST_OBJPOOL_TYPE_PREFIX, name)
#define FMANGLE(name) JOIN(WIST_OBJPOOL_FUN_PREFIX, name)

#define POOL_TYPE TMANGLE(Pool)
#define ITEM_TYPE WIST_OBJPOOL_TYPE
#define CHUNK_TYPE TMANGLE(PoolChunk)

#define ITEMS_PER_CHUNK 64

#ifdef WIST_OBJPOOL_HEADER

#ifndef WIST_OBJPOOL_FREE_H
#define WIST_OBJPOOL_FREE_H
typedef struct ObjpoolFree
{
    struct ObjpoolFree *next;
} ObjpoolFree;
#endif /* WIST_OBJPOOL_FREE_H */

typedef struct CHUNK_TYPE
{
    struct CHUNK_TYPE *next;
    ITEM_TYPE data[ITEMS_PER_CHUNK];
} CHUNK_TYPE;

typedef struct
{
    CHUNK_TYPE *chunk;
    ObjpoolFree *free;
} POOL_TYPE;

void FMANGLE(create)(POOL_TYPE *out);
void FMANGLE(destroy)(POOL_TYPE *pool);
ITEM_TYPE *FMANGLE(alloc)(POOL_TYPE *pool);
void FMANGLE(free)(POOL_TYPE *pool, ITEM_TYPE *item_to_free);

#endif /* WIST_OBJPOOL_HEADER */

#ifdef WIST_OBJPOOL_IMPLEMENTATION

static void
FMANGLE(add_chunk)(POOL_TYPE *pool)
{
    CHUNK_TYPE *chunk = WIST_NEW(CHUNK_TYPE);
    chunk->next = pool->chunk;
    pool->chunk = chunk;

    for (size_t i = 0; i < ITEMS_PER_CHUNK; i++)
    {
        ITEM_TYPE *item = &chunk->data[i];
        ObjpoolFree *free = (ObjpoolFree *) item;
        free->next = pool->free;
        pool->free = free;
    }
}

void
FMANGLE(create)(POOL_TYPE *out)
{
    out->chunk = NULL;
    out->free = NULL;
    FMANGLE(add_chunk)(out);
}

void
FMANGLE(destroy)(POOL_TYPE *pool)
{
    CHUNK_TYPE *chunk = pool->chunk;
    while (chunk != NULL)
    {
        CHUNK_TYPE *temp = chunk;
#ifdef WIST_OBJPOOL_DESTRUCTOR
        for (size_t i = 0; i < ITEMS_PER_CHUNK; i++)
            WIST_OBJPOOL_DESTRUCTOR(&chunk->data[i]);
#endif /* WIST_OBJPOOL_DESTRUCTOR */

        chunk = chunk->next;
        WIST_FREE(temp);
    }
}

ITEM_TYPE *
FMANGLE(alloc)(POOL_TYPE *pool)
{
    if (pool->free == NULL)
    {
        FMANGLE(add_chunk)(pool);
    }

    ITEM_TYPE *item = (ITEM_TYPE *) pool->free;
    pool->free = pool->free->next;
    return item;
}

void FMANGLE(free)
    (POOL_TYPE *pool,
     ITEM_TYPE *item_to_free)
{
#ifdef WIST_OBJPOOL_DESTRUCTOR
WIST_OBJPOOL_DESTRUCTOR(item_to_free);
#endif /* WIST_OBJPOOL_DESTRUCTOR */
    ObjpoolFree *free = (ObjpoolFree *)item_to_free;
    free->next = pool->free;
    pool->free = free;
}

#endif /* WIST_OBJPOOL_IMPLEMENTATION */

#undef WIST_OBJPOOL_TYPE
#undef WIST_OBJPOOL_FUN_PREFIX
#undef WIST_OBJPOOL_TYPE_PREFIX
#undef CAST
#undef PASTE
#undef JOIN
#undef TMANGLE
#undef FMANGLE
#undef POOL_TYPE
#undef ITEM_TYPE
#undef CHUNK_TYPE
#undef ITEMS_PER_CHUNK

#ifdef WIST_OBJPOOL_DESTRUCTOR
#undef WIST_OBJPOOL_DESTRUCTOR
#endif /* WIST_OBJPOOL_DESTRUCTOR */

#ifdef WIST_OBJPOOL_HEADER
#undef WIST_OBJPOOL_HEADER
#endif /* WIST_OBJPOOL_HEADER */

#ifdef WIST_OBJPOOL_IMPLEMENTATION
#undef WIST_OBJPOOL_IMPLEMENTATION
#endif /* WIST_OBJPOOL_IMPLEMENTATION */
