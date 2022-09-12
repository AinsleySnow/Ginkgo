#ifndef _TYPESPEC_H_
#define _TYPESPEC_H_

#include <string>
#include <variant>

class TypeSpec
{
private:
    std::variant<int, std::string> type;

public:
    TypeSpec() : type(TypeSpec::undef) {}
    TypeSpec(const TypeSpec& ts) { type = ts.type; }
    TypeSpec(int t) : type(t) {}
    explicit TypeSpec(const std::string& s) : type(s) {}

    static const int _bool = -1, 
    int8 = 0, uint8 = 1, int16 = 2,
    uint16 = 3, int32 = 4, uint32 = 5,
    int64 = 6, uint64 = 7, float32 = 8, 
    float64 = 9, _void = -2, undef = -3;

    TypeSpec& operator=(int t)
    {
        type = t;
        return *this;
    }

    operator int()
    {
        if (type.index() == 0)
            return std::get<0>(type);
        throw std::bad_variant_access();
    }

    bool operator<(const TypeSpec ts) const
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

    bool operator>(const TypeSpec ts) const
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

#endif // _TYPESPEC_H_
