#include "DeclSpec.h"

inline void DeclSpec::markType(int type)
{
    if (!(rawSpecifiers & type)) rawSpecifiers |= type;
    else throw 0;
}

inline void DeclSpec::markTypeLong()
{
    if (!(rawSpecifiers & 8)) rawSpecifiers |= 8;
    else if (!(rawSpecifiers & 16)) rawSpecifiers |= 16;
    else throw 0;
}

void DeclSpec::MarkSpec(const Node& n)
{
    switch (n.GetTag())
    {
    case Tag::_int:
        markType(_int); break;
    case Tag::_char:
        markType(_char); break;
    case Tag::_short:
        markType(_short); break;
    case Tag::_signed:
        markType(_signed); break;
    case Tag::_unsigned:
        markType(_unsigned); break;
    case Tag::_bool:
        markType(_bool); break;
    case Tag::_double:
        markType(_double); break;
    case Tag::_float:
        markType(_float); break;
    case Tag::_long:
        markTypeLong();
    }
}

inline void DeclSpec::markQual(int qual)
{
    if (!(rawQualifiers & qual)) rawQualifiers |= qual;
    else throw 0;
}

void DeclSpec::MarkQual(const Node& n)
{
    switch (n.GetTag())
    {
    case Tag::_const:
        markType(_int); break;
    case Tag::_char:
        markType(_char); break;
    case Tag::_short:
        markType(_short); break;
    case Tag::_signed:
        markType(_signed); break;
    case Tag::_unsigned:
        markType(_unsigned); break;
    case Tag::_bool:
        markType(_bool); break;
    case Tag::_double:
        markType(_double); break;
    case Tag::_float:
        markType(_float); break;
    case Tag::_long:
        markTypeLong();
    }
}

TypeSpec DeclSpec::GetSpec()
{
    switch (rawSpecifiers)
    {
    // int and its equivalent
    case _int: case _int | _signed:
        return TypeSpec::int32;
    case _int | _unsigned: case _unsigned:
        return TypeSpec::uint32;

    // short and its equivalent
    case _short: case _short | _int:
    case _signed | _short: case _signed | _short | _int:
        return TypeSpec::int16;
    case _short | _unsigned: case _short | _unsigned | _int:
        return TypeSpec::uint16;

    // long and its equivalent
    case _longlong: case _long: case _long | _signed:
    case _long | _int: case _signed | _long | _int:
        return TypeSpec::int64;
    case _unsigned | _longlong: case _unsigned | _long:
    case _long | _int | _unsigned:
        return TypeSpec::uint64;

    // char
    case _char: case _char | _signed:
        return TypeSpec::int8;
    case _char | _unsigned:
        return TypeSpec::uint8;

    // double
    case _double: case _double | _long:
        return TypeSpec::float64;

    // float
    case _float:
        return TypeSpec::float32;

    // error
    default:
        throw 0;
    }
}

unsigned int DeclSpec::GetQual()
{
    return rawQualifiers;
}

void DeclSpec::Join(const DeclSpec& ds)
{
    unsigned int sum = rawQualifiers + ds.rawQualifiers;
    rawQualifiers ^= ds.rawQualifiers;
    if (sum != rawQualifiers)
        throw 0;
    
    if ((rawSpecifiers & _long) && (ds.rawSpecifiers & _long))
    {
        rawSpecifiers &= ~_long;
        if (!((rawSpecifiers >> 5) & 1))
            rawSpecifiers |= _long << 1;
        else
            throw 0;
    }
    
    sum = rawSpecifiers + ds.rawSpecifiers;
    rawSpecifiers ^= ds.rawSpecifiers;
    if (sum != rawSpecifiers)
        throw 0;
}
