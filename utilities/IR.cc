#include "IR.h"

std::string Quadruple::ToString()
{
    return "op = " + std::to_string(static_cast<int>(op)) + ' ' +
        "arg1 = " + arg1 + ' ' +
        "arg2 = " + arg2 + ' ' +
        "arg3 = " + arg3;
}

void IR::Append(const Quadruple& q)
{
    data.push_back(q);
}

void IR::Join(IR& ir)
{
    data.splice(ir.data.end(), ir.data);
}

std::string_view IR::GetLastVar() const
{
    return data.end()->arg3;
}
