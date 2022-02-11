#include <wist/error.h>
#include <wist/mem.h>

#define INIT_ERRS 8

void
wist_error_add(WistErrorEngine *eng,
               WistError *err)
{
    if (eng->errs_used + 1 > eng->errs_alloc)
    {
        eng->errs_alloc *= 2;
        eng->errs = WIST_REALLOC(WistError, eng->errs, eng->errs_alloc);
    }

    eng->errs[eng->errs_used++] = *err;
}

WistErrorEngine *
wist_error_engine_create(void)
{
    WistErrorEngine *eng = WIST_NEW(WistErrorEngine);
    eng->errs = WIST_NEW_ARR(WistError, INIT_ERRS);
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
