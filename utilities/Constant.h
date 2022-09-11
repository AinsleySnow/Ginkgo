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

    Constant& operator++()
    {
        if (data.index() == 0)
            data = (std::get<0>(data))++;
        else
            data = (std::get<1>(data))++;
        return *this;
    }

    Constant& operator--()
    {
        if (data.index() == 0)
            data = (std::get<0>(data))--;
        else
            data = (std::get<1>(data))--;
        return *this;
    }

    Constant operator+()
    {
        return *this;
    }

    Constant operator-()
    {
        if (data.index() == 0)
            data = -(std::get<0>(data));
        else
            data = -(std::get<1>(data));
        return *this;
    }

    Constant operator~()
    {
        if (data.index() == 0)
            data = ~(std::get<0>(data));
        return *this;
    }

    Constant operator!()
    {
        if (data.index() == 0)
            data = static_cast<uint64_t>(!(std::get<0>(data)));
        else
            data = static_cast<uint64_t>(!(std::get<1>(data)));
        return *this;
    }

    Constant operator+(Constant& right)
    {
        if (data.index() == 0)
            data = std::get<0>(data) + std::get<0>(right.data);
        else
            data = std::get<1>(data) + std::get<1>(right.data);
        return *this;
    }
    
    Constant operator-(Constant& right)
    {
        if (data.index() == 0)
            data = std::get<0>(data) - std::get<0>(right.data);
        else
            data = std::get<1>(data) - std::get<1>(right.data);
        return *this;
    }

    Constant operator*(Constant& right)
    {
        if (data.index() == 0)
            data = std::get<0>(data) * std::get<0>(right.data);
        else
            data = std::get<1>(data) * std::get<1>(right.data);
        return *this;
    }

    Constant operator/(Constant& right)
    {
        if (data.index() == 0)
            data = std::get<0>(data) / std::get<0>(right.data);
        else
            data = std::get<1>(data) / std::get<1>(right.data);
        return *this;
    }

    Constant operator%(Constant& right)
    {
        if (data.index() == 0)
            data = std::get<0>(data) % std::get<0>(right.data);
        return *this;
    }

    Constant operator>>(Constant& right)
    {
        if (data.index() == 0)
        {
            if (type == TypeSpec::int8 || type == TypeSpec::int16 || 
                type == TypeSpec::int32 || type == TypeSpec::int64)
                data = static_cast<uint64_t>(
                    static_cast<int64_t>(std::get<0>(data)) >> std::get<0>(right.data));
            else
                data = std::get<0>(data) >> std::get<0>(right.data);
        }
        return *this;
    }

    Constant operator<<(Constant& right)
    {
        if (type == TypeSpec::int8 || type == TypeSpec::int16 || 
            type == TypeSpec::int32 || type == TypeSpec::int64)
            data = static_cast<uint64_t>(
                static_cast<int64_t>(std::get<0>(data)) >> std::get<0>(right.data));
        else
            data = std::get<0>(data) >> std::get<0>(right.data);
        return *this;
    }

    Constant operator<(Constant& right)
    {
        Constant copy{*this};
        if (copy.data.index() == 0)
        {
            copy.data = static_cast<uint64_t>(std::get<0>(copy.data) < std::get<0>(right.data));
            copy.type = TypeSpec::_bool;
        }
        else
        {
            copy.data = static_cast<uint64_t>(std::get<1>(copy.data) < std::get<1>(right.data));
            copy.type = TypeSpec::_bool;
        }
        return copy;
    }

    Constant operator>(Constant& right)
    {
        Constant copy{*this};
        if (copy.data.index() == 0)
        {
            copy.data = static_cast<uint64_t>(std::get<0>(copy.data) > std::get<0>(right.data));
            copy.type = TypeSpec::_bool;
        }
        else
        {
            copy.data = static_cast<uint64_t>(std::get<1>(copy.data) > std::get<1>(right.data));
            copy.type = TypeSpec::_bool;
        }
        return copy;
    }

    Constant operator<=(Constant& right)
    {
        Constant copy{*this};
        if (copy.data.index() == 0)
        {
            copy.data = static_cast<uint64_t>(std::get<0>(copy.data) <= std::get<0>(right.data));
            copy.type = TypeSpec::_bool;
        }
        else
        {
            copy.data = static_cast<uint64_t>(std::get<1>(copy.data) <= std::get<1>(right.data));
            copy.type = TypeSpec::_bool;
        }
        return copy;
    }

    Constant operator>=(Constant& right)
    {
        Constant copy{*this};
        if (copy.data.index() == 0)
        {
            copy.data = static_cast<uint64_t>(std::get<0>(copy.data) >= std::get<0>(right.data));
            copy.type = TypeSpec::_bool;
        }
        else
        {
            copy.data = static_cast<uint64_t>(std::get<1>(copy.data) >= std::get<1>(right.data));
            copy.type = TypeSpec::_bool;
        }
        return copy;
    }

    Constant operator==(Constant& right)
    {
        Constant copy{*this};
        if (copy.data.index() == 0)
        {
            copy.data = static_cast<uint64_t>(std::get<0>(copy.data) == std::get<0>(right.data));
            copy.type = TypeSpec::_bool;
        }
        else
        {
            copy.data = static_cast<uint64_t>(std::get<1>(copy.data) == std::get<1>(right.data));
            copy.type = TypeSpec::_bool;
        }
        return copy;
    }

    Constant operator!=(Constant& right)
    {
        Constant copy{*this};
        if (copy.data.index() == 0)
        {
            copy.data = static_cast<uint64_t>(std::get<0>(copy.data) != std::get<0>(right.data));
            copy.type = TypeSpec::_bool;
        }
        else
        {
            copy.data = static_cast<uint64_t>(std::get<1>(copy.data) != std::get<1>(right.data));
            copy.type = TypeSpec::_bool;
        }
        return copy;
    }

    Constant operator&(Constant& right)
    {
        Constant copy{*this};
        if (copy.data.index() == 0)
        {
            copy.data = std::get<0>(copy.data) & std::get<0>(right.data);
            copy.type = TypeSpec::_bool;
        }
        return copy;
    }

    Constant operator^(Constant& right)
    {
        Constant copy{*this};
        if (copy.data.index() == 0)
        {
            copy.data = std::get<0>(copy.data) ^ std::get<0>(right.data);
            copy.type = TypeSpec::_bool;
        }
        return copy;
    }

    Constant operator|(Constant& right)
    {
        Constant copy{*this};
        if (copy.data.index() == 0)
        {
            copy.data = std::get<0>(copy.data) | std::get<0>(right.data);
            copy.type = TypeSpec::_bool;
        }
        return copy;
    }

    Constant operator&&(Constant& right)
    {
        Constant copy{*this};
        if (copy.data.index() == 0)
        {
            copy.data = static_cast<uint64_t>(std::get<0>(copy.data) && std::get<0>(right.data));
            copy.type = TypeSpec::_bool;
        }
        else
        {
            copy.data = static_cast<uint64_t>(std::get<1>(copy.data) && std::get<1>(right.data));
            copy.type = TypeSpec::_bool;
        }
        return copy;
    }

    Constant operator||(Constant& right)
    {
        Constant copy{*this};
        if (copy.data.index() == 0)
        {
            copy.data = static_cast<uint64_t>(std::get<0>(copy.data) || std::get<0>(right.data));
            copy.type = TypeSpec::_bool;
        }
        else
        {
            copy.data = static_cast<uint64_t>(std::get<1>(copy.data) || std::get<1>(right.data));
            copy.type = TypeSpec::_bool;
        }
        return copy;
    }
};

#endif // _CONSTANT_H_
