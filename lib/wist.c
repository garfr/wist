#include <stdio.h>
#include <stdlib.h>

#include <wist.h>

#define makestr(s) #s
#define WIST_VER_MAJ_STR makestr(WIST_VER_MAJ)

const char *wist_ver_string = "v0." WIST_VER_MAJ_STR;
