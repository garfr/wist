#include <stdarg.h>

#include <wist/span.h>
#include <wist/mem.h>

#define INIT_SPANS 16

void
wist_span_index_create(WistSpanIndex *index)
{
    index->spans = WIST_NEW_ARR(WistWideSpan, INIT_SPANS);

    index->spans_use = 0;
    index->spans_alloc = INIT_SPANS;
}

void
wist_span_index_destroy(WistSpanIndex *index)
{
    WIST_FREE(index->spans);
}

WistWideSpan *
wist_get_span(WistSpanIndex *index,
              WistSpan *span)
{
    return &index->spans[span->start];
}

WistSpan
wist_add_span(WistSpanIndex *index,
              uint64_t start,
              uint16_t end)
{
    WistSpan ret;
    if (start > INT32_MAX || (end - start) > INT16_MAX)
    {
        if (index->spans_use + 1 > index->spans_alloc)
        {
            index->spans_alloc *= 2;
            index->spans = WIST_REALLOC(WistWideSpan, index->spans,
                                        index->spans_alloc);
        }
        WistWideSpan *wspan = &index->spans[index->spans_use];
        wspan->start = start;
        wspan->end = end;
        ret.start = index->spans_use++;
        ret.len = 0;
        return ret;
    }

    ret.start = start;
    ret.len = end - start;
    return ret;
}

WistSpan 
wist_combine_span(WistSpanIndex *index, 
                  WistSpan start, 
                  WistSpan end)
{
    size_t start_idx = start.start;
    if (start.len == 0)
    {
        start_idx = wist_get_span(index, &start)->start;
    }
    size_t end_idx = end.start + end.len;
    if (end.len == 0)
    {
        WistWideSpan *wspan = wist_get_span(index, &end);
        end_idx = wspan->end;
    }
    return wist_add_span(index, start_idx, end_idx);
}

WistMultiSpan 
wist_multispan_create(int num_spans,
                      ...)
{
    va_list args;
    va_start(args, num_spans);
    WistMultiSpan mspan;
    mspan.alloc = num_spans;
    mspan.spans = WIST_NEW_ARR(WistSpan, num_spans);
    mspan.used = num_spans;
    for (int i = 0; i < num_spans; i++)
    {
        mspan.spans[i] = va_arg(args, WistSpan);
    }
    va_end(args);
    
    return mspan;
}

void 
wist_multispan_destroy(WistMultiSpan *span)
{
    WIST_FREE(span->spans);
}

void 
wist_get_span_boundary(WistSpanIndex *spans, 
                  WistSpan *span, 
                  size_t *os, 
                  size_t *oe)
{
    if (span->len ==0)
    {
        WistWideSpan *wspan = wist_get_span(spans, span);
        *os = wspan->start;
        *oe = wspan->end;
    }
    *os = span->start;
    *oe = span->start + span->len;
}
