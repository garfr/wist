#include <stdio.h>
#include <stdlib.h>

#include <wist/wist.h>

int
main(int argc, const char *argv[])
{
    if (argc < 2)
    {
        printf("Error: Expected filename.\n");
        return EXIT_FAILURE;
    }

    WistErrorEngine *err_eng = wist_error_engine_create();
    WistIndex *index = wist_index_create();
    WistPetal *petal = wist_petal_parse(index,
                                        err_eng,
                                        argv[1],
                                        WIST_PETAL_BITS_NONE);
    if (petal == NULL)
    {
        printf("Couldn't open file: %s.\n", argv[1]);
        return EXIT_FAILURE;
    }

    wist_petal_destroy(petal);
    wist_index_destroy(index);
    wist_error_engine_destroy(err_eng);

    return EXIT_SUCCESS;
}
