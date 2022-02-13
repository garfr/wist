#ifndef WIST_ERROR_H
#define WIST_ERROR_H

#include <wist/wist.h>
#include <wist/span.h>
#include <wist/str.h>

typedef enum
{
    WIST_ERROR_ERR,
    WIST_ERROR_FATAL,
    WIST_ERROR_WARNING,
} WistErrorLevel;

typedef enum
{
    WIST_ERROR_UNEXPECTED_CHAR,
} WistErrorCode;

typedef struct
{
    WistErrorLevel level;
    WistErrorCode code;
    WistStr msg;
    WistMultiSpan span;
} WistError;

typedef struct
{
    WistError *errs;
    size_t errs_used;
    size_t errs_alloc;
} WistErrorEngine;

WistErrorEngine *wist_error_engine_create(void);
void wist_error_engine_destroy(WistErrorEngine *eng);
int wist_error_engine_has_errors(WistErrorEngine *eng);
void wist_error_engine_print(WistErrorEngine *eng, WistSpanIndex *spans, 
                             WistIndex *index);

WistError *wist_add_error(WistErrorEngine *eng);

void wist_fill_error(WistError *err, WistErrorLevel level, WistErrorCode code, 
                     WistStr msg, WistMultiSpan span);
                     
#endif /* WIST_ERROR_H */
