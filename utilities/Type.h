#ifndef _TYPE_H_
#define _TYPE_H_

#include <string>
#include <variant>

class Type
{
private:
    std::variant<int, std::string> type;

public:
    Type() : type(Type::undef) {}
    Type(const Type& ts) { type = ts.type; }
    Type(int t) : type(t) {}
    explicit Type(const std::string& s) : type(s) {}

    static const int _bool = -1, 
    int8 = 0, uint8 = 1, int16 = 2,
    uint16 = 3, int32 = 4, uint32 = 5,
    int64 = 6, uint64 = 7, float32 = 8, 
    float64 = 9, _void = -2, undef = -3;

    Type& operator=(int t)
    {
        type = t;
        return *this;
    }

    operator int() const
    {
        if (type.index() == 0)
            return std::get<0>(type);
        throw std::bad_variant_access();
    }

    bool operator<(const Type ts) const
    {
        if (type.index() == 0 && ts.type.index() == 0)
            return std::get<0>(type) < std::get<0>(ts.type);
        throw 1002;
    }

    bool operator<(int ts) const
    {
        if (type.index() == 0)
            return std::get<0>(type) < ts;
        throw 1002;
    }

    bool operator>(const Type ts) const
    {
        if (type.index() == 0 && ts.type.index() == 0)
            return std::get<0>(type) > std::get<0>(ts.type);
        throw 1002;
    }

    bool operator>(int ts) const
    {
        if (type.index() == 0)
            return std::get<0>(type) > ts;
        throw 1002;
    }
    
    bool operator==(int ts) const
    {
        if (type.index() == 0)
            return ts == std::get<0>(type);
        return false;
    }

    bool operator==(const std::string& name) const
    {
        if (type.index() == 1)
            return name == std::get<1>(type);
        return false;
    }
};

#endif // _TYPE_H_
