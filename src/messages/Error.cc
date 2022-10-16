#include "Error.h"
#include <string>
#include <cstdio>

extern std::string errormsg[];

void Error(ErrorId id, ...)
{
    printf("%s\n", errormsg[static_cast<int>(id)].c_str());
}
