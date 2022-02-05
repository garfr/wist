#ifndef WIST_OBJPOOL_H
#define WIST_OBJPOOL_H

typedef struct WistObjpool WistObjpool;

#define wist_objpool_create(__type) _wist_objpool_create(sizeof(__type))

void wist_objpool_destroy(WistObjpool *pool);

void *wist_objpool_alloc(WistObjpool *pool);
void wist_objpool_free(WistObjpool *pool, void *ptr);

/* PRIVATE */
WistObjpool *_wist_objpool_create(size_t item_sz);

#endif /* WIST_OBJPOOL_H */
