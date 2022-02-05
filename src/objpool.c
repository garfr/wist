#include <wist/defs.h>
#include <wist/mem.h>
#include <wist/objpool.h>

#define POOL_MIN_CHUNK_BYTES 4096
#define POOL_MIN_CHUNK_ITEMS 64

typedef struct Chunk
{
    struct Chunk *next;
    size_t alloc; /* Number of items allocated. */
    uint8_t data[];
} Chunk;

typedef struct FreeList
{
    struct FreeList *next;
} FreeList;

struct WistObjpool
{
    size_t item_sz;
    size_t chunk_sz;
    Chunk *chunk;
    FreeList *free;
    FreeList *free_end;
};

/* === PROTOTYPES === */

void add_chunk(WistObjpool *pool);

/* === PUBLIC FUNCTIONS === */

WistObjpool *
_wist_objpool_create(size_t item_sz)
{
    WistObjpool *pool = WIST_NEW(WistObjpool);
    pool->item_sz = item_sz;
    size_t chunk_sz = POOL_MIN_CHUNK_ITEMS;
    while (chunk_sz * item_sz < POOL_MIN_CHUNK_BYTES)
    {
        chunk_sz *= 2;
    }
    pool->chunk_sz = chunk_sz;

    pool->free = NULL;

    pool->chunk = NULL;

    add_chunk(pool);

    return pool;
}
void
wist_objpool_destroy(WistObjpool *pool)
{
    Chunk *iter = pool->chunk;
    while (iter != NULL)
    {
        Chunk *temp = iter;
        iter = iter->next;
        MEM_FREE(temp);
    }
    MEM_FREE(pool);
}

void *
wist_objpool_alloc(WistObjpool *pool)
{
    if (pool->free == NULL)
    {
        add_chunk(pool);
    }

    void *ptr = pool->free;
    pool->free = pool->free->next;
    return ptr;
}

void
wist_objpool_free(WistObjpool *pool,
                  void *ptr)
{
    FreeList *new = (FreeList *) ptr;
    new->next = pool->free;
    pool->free = new;
}

/* === PRIVATE FUNCTIONS === */

void
add_chunk(WistObjpool *pool)
{
    Chunk *chunk = (Chunk *) WIST_NEW_ARR(uint8_t, sizeof(Chunk) + pool->chunk_sz);
    chunk->next = pool->chunk;
    pool->chunk = chunk;

    uint8_t *start = (uint8_t *) chunk->data;
    FreeList *last = pool->free;
    uint8_t *i;
    for (i = start; i < start + pool->chunk_sz; i += pool->item_size)
    {
        FreeList *temp = (FreeList *) i;
        temp->next = last;
    }
    i -= pool->item_size;
    pool->free = (FreeList *) i;
}
