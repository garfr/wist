#ifndef WIST_ERROR_H
#define WIST_ERROR_H

#include <wist/wist.h>
#include <wist/span.h>

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

struct WistError
{
    WistErrorLevel level;
    WistErrorCode code;
    union
    {
        struct
        {
            WistSpan bad_char;
            WistSpan bad_token;
        } unexpected_char;
    };
};

struct WistErrorEngine
{
    WistError *errs;
    size_t errs_used;
    size_t errs_alloc;
};

WistError *wist_add_error(WistErrorEngine *eng);

#endif /* WIST_ERROR_H */
