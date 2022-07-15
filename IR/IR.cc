#include "IR.h"

std::string triple::ToString()
{
    return "op = " + std::to_string(static_cast<int>(op)) + ' ' +
        "arg1 = " + arg1 + ' ' +
        "arg2 = " + arg2 + ' ';
}

void IR::Append(const triple& t)
{
    data.push_back(t);
}

IRIter IR::iter()
{
    return data.cbegin();
}
