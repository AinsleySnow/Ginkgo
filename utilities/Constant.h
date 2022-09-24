#ifndef _CONSTANT_H_
#define _CONSTANT_H_

#include <cstdint>
#include <string>
#include <variant>
#include "Type.h"
#include "EnumsforEntry.h"

struct Constant
{
    std::variant<uint64_t, double> data;
    Type type;

    std::string ToString() const
    {
        std::string repr;
        if (data.index() == 0)
            repr = std::to_string(std::get<0>(data));
        else
            repr = std::to_string(std::get<1>(data));
        repr += " " + static_cast<int>(type);
        return repr;
    }

    uint64_t GetU64() const
    {
        return std::get<0>(data);
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
            if (type == Type::int8 || type == Type::int16 || 
                type == Type::int32 || type == Type::int64)
                data = static_cast<uint64_t>(
                    static_cast<int64_t>(std::get<0>(data)) >> std::get<0>(right.data));
            else
                data = std::get<0>(data) >> std::get<0>(right.data);
        }
        return *this;
    }

    Constant operator<<(Constant& right)
    {
        if (type == Type::int8 || type == Type::int16 || 
            type == Type::int32 || type == Type::int64)
            data = static_cast<uint64_t>(
                static_cast<int64_t>(std::get<0>(data)) << std::get<0>(right.data));
        else
            data = std::get<0>(data) << std::get<0>(right.data);
        return *this;
    }

    Constant operator<(Constant& right)
    {
        Constant copy{*this};
        if (copy.data.index() == 0)
        {
            copy.data = static_cast<uint64_t>(std::get<0>(copy.data) < std::get<0>(right.data));
            copy.type = Type::_bool;
        }
        else
        {
            copy.data = static_cast<uint64_t>(std::get<1>(copy.data) < std::get<1>(right.data));
            copy.type = Type::_bool;
        }
        return copy;
    }

    Constant operator>(Constant& right)
    {
        Constant copy{*this};
        if (copy.data.index() == 0)
        {
            copy.data = static_cast<uint64_t>(std::get<0>(copy.data) > std::get<0>(right.data));
            copy.type = Type::_bool;
        }
        else
        {
            copy.data = static_cast<uint64_t>(std::get<1>(copy.data) > std::get<1>(right.data));
            copy.type = Type::_bool;
        }
        return copy;
    }

    Constant operator<=(Constant& right)
    {
        Constant copy{*this};
        if (copy.data.index() == 0)
        {
            copy.data = static_cast<uint64_t>(std::get<0>(copy.data) <= std::get<0>(right.data));
            copy.type = Type::_bool;
        }
        else
        {
            copy.data = static_cast<uint64_t>(std::get<1>(copy.data) <= std::get<1>(right.data));
            copy.type = Type::_bool;
        }
        return copy;
    }

    Constant operator>=(Constant& right)
    {
        Constant copy{*this};
        if (copy.data.index() == 0)
        {
            copy.data = static_cast<uint64_t>(std::get<0>(copy.data) >= std::get<0>(right.data));
            copy.type = Type::_bool;
        }
        else
        {
            copy.data = static_cast<uint64_t>(std::get<1>(copy.data) >= std::get<1>(right.data));
            copy.type = Type::_bool;
        }
        return copy;
    }

    Constant operator==(Constant& right)
    {
        Constant copy{*this};
        if (copy.data.index() == 0)
        {
            copy.data = static_cast<uint64_t>(std::get<0>(copy.data) == std::get<0>(right.data));
            copy.type = Type::_bool;
        }
        else
        {
            copy.data = static_cast<uint64_t>(std::get<1>(copy.data) == std::get<1>(right.data));
            copy.type = Type::_bool;
        }
        return copy;
    }

    Constant operator!=(Constant& right)
    {
        Constant copy{*this};
        if (copy.data.index() == 0)
        {
            copy.data = static_cast<uint64_t>(std::get<0>(copy.data) != std::get<0>(right.data));
            copy.type = Type::_bool;
        }
        else
        {
            copy.data = static_cast<uint64_t>(std::get<1>(copy.data) != std::get<1>(right.data));
            copy.type = Type::_bool;
        }
        return copy;
    }

    Constant operator&(Constant& right)
    {
        Constant copy{*this};
        if (copy.data.index() == 0)
        {
            copy.data = std::get<0>(copy.data) & std::get<0>(right.data);
            copy.type = Type::_bool;
        }
        return copy;
    }

    Constant operator^(Constant& right)
    {
        Constant copy{*this};
        if (copy.data.index() == 0)
        {
            copy.data = std::get<0>(copy.data) ^ std::get<0>(right.data);
            copy.type = Type::_bool;
        }
        return copy;
    }

    Constant operator|(Constant& right)
    {
        Constant copy{*this};
        if (copy.data.index() == 0)
        {
            copy.data = std::get<0>(copy.data) | std::get<0>(right.data);
            copy.type = Type::_bool;
        }
        return copy;
    }

    Constant operator&&(Constant& right)
    {
        Constant copy{*this};
        if (copy.data.index() == 0)
        {
            copy.data = static_cast<uint64_t>(std::get<0>(copy.data) && std::get<0>(right.data));
            copy.type = Type::_bool;
        }
        else
        {
            copy.data = static_cast<uint64_t>(std::get<1>(copy.data) && std::get<1>(right.data));
            copy.type = Type::_bool;
        }
        return copy;
    }

    Constant operator||(Constant& right)
    {
        Constant copy{*this};
        if (copy.data.index() == 0)
        {
            copy.data = static_cast<uint64_t>(std::get<0>(copy.data) || std::get<0>(right.data));
            copy.type = Type::_bool;
        }
        else
        {
            copy.data = static_cast<uint64_t>(std::get<1>(copy.data) || std::get<1>(right.data));
            copy.type = Type::_bool;
        }
        return copy;
    }
};

#endif // _CONSTANT_H_
