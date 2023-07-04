#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

void __Ginkgo_assert(bool cond, char* expr, char* file, int line, char* func)
{
    if (cond) return;
    fprintf(stderr, "Assert %s fail in %s at %s, line %d.\n", expr, file, func, line);
    abort();
}
