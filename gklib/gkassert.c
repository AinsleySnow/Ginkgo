#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

void __Ginkgo_assert(bool cond, char* expr, char* file, int line, char* func)
{
    if (cond) return;
    fprintf(stderr, "%s:%d, in function %s: Assertion '%s' failed.\n", file, line, func, expr);
    abort();
}
