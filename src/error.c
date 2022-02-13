#include <stdlib.h>

#include <wist/error.h>
#include <wist/mem.h>
#include <wist/index.h>

#define INIT_ERRS 8

WistError *
wist_add_error(WistErrorEngine *eng)
{
    if (eng->errs_used + 1 > eng->errs_alloc)
    {
        eng->errs_alloc *= 2;
        eng->errs = WIST_REALLOC(WistError, eng->errs, eng->errs_alloc);
    }

    return &eng->errs[eng->errs_used++];
}

WistErrorEngine *
wist_error_engine_create(WistIndex *index, WistSpanIndex *spans)
{
    WistErrorEngine *eng = WIST_NEW(WistErrorEngine);
    eng->errs = WIST_NEW_ARR(WistError, INIT_ERRS);
    eng->index = index;
    eng->spans = spans;
    eng->errs_used = 0;
    eng->errs_alloc = INIT_ERRS;
    return eng;
}

void
wist_error_engine_destroy(WistErrorEngine *eng)
{
    WIST_FREE(eng->errs);
    WIST_FREE(eng);
}

void 
wist_fill_error(WistError *err, 
                WistErrorLevel level, 
                WistErrorCode code, 
                WistStr msg, 
                WistMultiSpan span)
{
    err->level = level;
    err->code = code;
    err->msg = msg;
    err->span = span;
}

int 
wist_error_engine_has_errors(WistErrorEngine *eng)
{
    return eng->errs_used > 0;
}

static const char *err_level_to_string[] =
{
    [WIST_ERROR_ERR] = "error",
    [WIST_ERROR_WARNING] = "warning",
};

void 
wist_error_engine_print(WistErrorEngine *eng)
{
    for (size_t i = 0; i < eng->errs_used; i++)
    {
        WistError *err = &eng->errs[i];
        printf("%s: %.*s\n", err_level_to_string[err->level], 
               (int) err->msg.len, (char*) err->msg.str);

        
        if (err->span.used > 1)
        {
            printf("Can only print one span.\n");
            exit(EXIT_FAILURE);
        }
        size_t min_start = SIZE_MAX, max_end = 0;
        for (size_t i = 0; i < err->span.used; i++)
        {
            size_t start, end;
            wist_get_span_boundary(eng->spans, &err->span.spans[i], &start, &end);
            if (start < min_start)
            {
                min_start = start;
            }
            if (end > max_end)
            {
                max_end = end;
            }
        }
        WistSpan full_span = wist_add_span(eng->spans, min_start, max_end);
        WistFile *file = index_get_span_file(eng->index, eng->spans, &full_span);
        WistMembuf *buf = &file->buf;
        
        if (min_start >= file->idx_start + buf->len)
        {
            min_start--;
            max_end = min_start;

        }
        size_t line = 1;
        for (size_t i = file->idx_start; i < min_start; i++)
        {
            if (buf->data[i] == '\n')
            {
                line++;
            }
        }

        size_t last_newline;
        for (last_newline = min_start; last_newline > file->idx_start && 
             buf->data[last_newline - file->idx_start] != '\n';
             last_newline--) 
            ; /* DO NOTHING */      
      
        if (buf->data[last_newline - file->idx_start] == '\n')
        {
            last_newline += 1;
        }
        
        printf("   |\n");
        
        size_t i = last_newline;
        for (; i < max_end; )
        {
            printf(" %d | ", line);
            size_t end_idx = i;
            while (buf->data[end_idx - file->idx_start] != '\n' && end_idx <= max_end)
            {
                end_idx++;
            }
            
            for (size_t j = i; j < end_idx; j++)
            {
                int c = buf->data[j - file->idx_start];
                printf("%c", buf->data[j - file->idx_start]);
            }
            printf("\n   | ");
            for (size_t k = 0; k < err->span.used; k++)
            {
                WistSpan *span = &err->span.spans[k];
                size_t sstart, send;
                wist_get_span_boundary(eng->spans, span, &sstart, &send);
                if (i <= sstart && end_idx >= send)
                {
                    size_t spaces = sstart - i;
                    for (size_t l = 0; l < spaces; l++)
                    {   
                        printf(" ");
                    }
                    size_t carets = send - sstart;
                    for (size_t l = 0; l < carets; l++)
                    {
                        printf("^");
                    }
                }
            }
            printf("\n");
            i = end_idx + 1;
            line++;
        }
        

        /*
        for (size_t i = 0; i < err->span.used; i++)
        {
            WistSpan *span = &err->span.spans[i];
            WistStrRef ref = index_get_span(index, spans, span);
            printf("\"%.*s\"\n", (int) ref.len, (char*) ref.str);
        }
        */
    }    
}

