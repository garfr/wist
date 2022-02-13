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

WistIndex *wist_index_create();
WistPetal *wist_petal_parse(WistIndex *index, const char *filename, 
                            WistPetalBits bits);

void wist_petal_destroy(WistPetal *petal);
void wist_index_destroy(WistIndex *index);


#endif /* WIST_WIST_H */
