#ifndef _CONSTANT_H_
#define _CONSTANT_H_

#include <cstdint>
#include <string>
#include <variant>
#include "EnumsforEntry.h"

struct Constant
{
    std::variant<uint64_t, double> data;
    TypeSpec type;

    std::string ToString()
    {
        std::string repr;
        if (data.index() == 0)
            repr = std::to_string(std::get<0>(data));
        else
            repr = std::to_string(std::get<1>(data));
        repr += " " + static_cast<int>(type);
        return repr;
    }
};

#endif // _CONSTANT_H_
