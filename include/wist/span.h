#ifndef WIST_SPAN_H
#define WIST_SPAN_H

#include <wist/defs.h>
#include <wist/index.h>

typedef struct
{
    uint32_t start;
    uint16_t len; /* If len == 0, start is an index into a span table. */
} WistSpan;

typedef struct
{
    uint64_t start;
    uint64_t end;
} WistWideSpan;

typedef struct
{
    WistWideSpan *spans;
    size_t spans_use;
    size_t spans_alloc;
} WistSpanIndex;

void wist_span_index_create(WistSpanIndex *index);
void wist_span_index_destroy(WistSpanIndex *index);

WistWideSpan *wist_get_span(WistSpanIndex *index, WistSpan *span);
WistSpan wist_add_span(WistSpanIndex *index, uint64_t start, uint16_t end);
WistSpan wist_combine_span(WistSpanIndex *index, WistSpan start, WistSpan end);

#endif /* WIST_SPAN_H */
