#ifndef WIST_SPAN_H
#define WIST_SPAN_H

#include <wist/defs.h>

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

typedef struct
{
    size_t alloc;
    size_t used;
    WistSpan *spans;
} WistMultiSpan;

void wist_span_index_create(WistSpanIndex *index);
void wist_span_index_destroy(WistSpanIndex *index);

void wist_get__span_boundary(WistSpanIndex *spans, WistSpan *span, size_t *os, 
                       size_t *oe);

WistWideSpan *wist_get_span(WistSpanIndex *index, WistSpan *span);
WistSpan wist_add_span(WistSpanIndex *index, uint64_t start, uint16_t end);
WistSpan wist_combine_span(WistSpanIndex *index, WistSpan start, WistSpan end);

WistMultiSpan wist_multispan_create(int num_spans, ...);
void wist_multispan_destroy(WistMultiSpan *span);

#endif /* WIST_SPAN_H */
