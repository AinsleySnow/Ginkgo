#include "IR.h"

std::string Triple::ToString()
{
    return "op = " + std::to_string(static_cast<int>(op)) + ' ' +
        "arg1 = " + arg1 + ' ' +
        "arg2 = " + arg2 + ' ';
}

void IR::Append(const Triple& t)
{
    data.push_back(t);
}
