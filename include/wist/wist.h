#ifndef WIST_WIST_H
#define WIST_WIST_H

#define WIST_MINOR_VERSION 1
#define WIST_MAJOR_VERSION 0

#define WIST_VERSION "0.1"

typedef struct WistIndex WistIndex;

typedef struct WistPetal WistPetal;

typedef enum
{
    WIST_PETAL_BITS_NONE,
} WistPetalBits;

typedef struct WistErrorEngine WistErrorEngine;
typedef struct WistError WistError;

WistIndex *wist_index_create();
WistPetal *wist_petal_parse(WistIndex *index, WistErrorEngine *err_end,
                            const char *filename, WistPetalBits bits);

void wist_petal_destroy(WistPetal *petal);
void wist_index_destroy(WistIndex *index);

WistErrorEngine *wist_error_engine_create(void);
void wist_error_engine_destroy(WistErrorEngine *eng);

#endif /* WIST_WIST_H */
