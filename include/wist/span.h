#ifndef WIST_SPAN_H
#define WIST_SPAN_H

#include <wist/defs.h>

typedef struct
{
    uint32_t start;
    uint16_t len; /* If len == 0, start is an index into a span table. */
    uint16_t macro_info;
} WistSpan;

typedef struct
{
    uint32_t start;
    uint32_t end;
} WistWideSpan;

typedef struct
{
    WistIndex *index;

    WistWideSpan *spans;
    size_t spans_use;
    size_t spans_alloc;
} WistSpanIndex;

#endif /* WIST_SPAN_H */
