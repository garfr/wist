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
