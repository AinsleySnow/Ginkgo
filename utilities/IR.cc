#include "Constant.h"
#include "IR.h"

std::string Quadruple::ToString()
{
    std::string repr{ std::to_string(static_cast<int>(op)) + '|' };
    if (arg1.index() == 0)
        repr += std::get<0>(arg1).ToString();
    else
        repr += std::get<1>(arg1);
    repr += '|';

    if (arg2.index() == 0)
        repr += std::get<0>(arg2).ToString();
    else
        repr += std::get<1>(arg2);
    repr += '|';

    repr += arg3;
    return repr;
}

void IR::Append(const Quadruple& q)
{
    data.push_back(q);
}

void IR::AppendLable(const std::string& lable)
{
    Quadruple quad{
        IROper::lable,
        lable
    };
    data.push_back(quad);
}

void IR::Join(IR& ir)
{
    data.splice(data.end(), ir.data);
}

std::string IR::GetLastVar() const
{
    return data.end()->arg3;
}

Constant IR::GetConstant() const
{
    return std::get<0>(Identifier.value());
}

unsigned long IR::lableIndex;
std::string IR::GetLable()
{
    return "L" + lableIndex++;
}
