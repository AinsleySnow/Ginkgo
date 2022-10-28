#include "Error.h"
#include <cstdio>

extern "C" const char* error[];

void Error(ErrorId id, ...)
{
    printf("%s\n", error[static_cast<int>(id)]);
}
