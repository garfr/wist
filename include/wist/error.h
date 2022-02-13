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
    WIST_ERROR_EXPECTED_PATTERN,
    WIST_ERROR_EXPECTED_EXPR,
    WIST_ERROR_EXPECTED_DECL,
    WIST_ERROR_EXPECTED_CLOSE_DELIMITER,
    WIST_ERROR_EXPECTED_LAMBDA_ARROW,
    WIST_ERROR_EXPECTED_END_OF_DECL,
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
    WistIndex *index;
    WistSpanIndex *spans;
} WistErrorEngine;

WistErrorEngine *wist_error_engine_create(WistIndex *index, WistSpanIndex *spans);
void wist_error_engine_destroy(WistErrorEngine *eng);
int wist_error_engine_has_errors(WistErrorEngine *eng);
void wist_error_engine_print(WistErrorEngine *eng);

WistError *wist_add_error(WistErrorEngine *eng);

void wist_fill_error(WistError *err, WistErrorLevel level, WistErrorCode code, 
                     WistStr msg, WistMultiSpan span);
                     
#endif /* WIST_ERROR_H */
