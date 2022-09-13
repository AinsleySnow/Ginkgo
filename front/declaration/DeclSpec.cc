#include "DeclSpec.h"

inline void DeclSpec::markType(int type)
{
    if (!(rawSpecifiers & type)) rawSpecifiers |= type;
    else throw 1001;
}

inline void DeclSpec::markTypeLong()
{
    if (!(rawSpecifiers & 8)) rawSpecifiers |= 8;
    else if (!(rawSpecifiers & 16)) rawSpecifiers |= 16;
    else throw 1001;
}

void DeclSpec::MarkSpec(Tag t)
{
    switch (t)
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
        markTypeLong(); break;
    default:
        return;
    }
}

inline void DeclSpec::markQual(int qual)
{
    rawQualifiers |= qual;
}

void DeclSpec::SetStorage(Tag t)
{
    switch (t)
    {
        case Tag::_static:
            if (!rawStorageCls) throw 1010;
            rawStorageCls = 
                static_cast<int>(StorageSpec::_static);
            break;
        case Tag::_auto:
            if (!rawStorageCls) throw 1010;
            rawStorageCls = 
                static_cast<int>(StorageSpec::_auto);
            break;
        case Tag::_thread_local:
            if (!rawStorageCls) throw 1010;
            rawStorageCls = 
                static_cast<int>(StorageSpec::_thread_local);
            break;
        case Tag::_register:
            if (!rawStorageCls) throw 1010;
            rawStorageCls = 
                static_cast<int>(StorageSpec::_register);
            break;
        case Tag::_extern:
            if (!rawStorageCls) throw 1010;
            rawStorageCls = 
                static_cast<int>(StorageSpec::_extern);
            break;
        default:
            break;
    }
}

void DeclSpec::MarkQual(Tag t)
{
    switch (t)
    {
    case Tag::_const:
        markQual(_const); break;
    case Tag::_atomic:
        markQual(_atomic); break;
    case Tag::_restrict:
        markQual(_restrict); break;
    case Tag::_volatile:
        markQual(_volatile); break;
    default:
        return;
    }
}

TypeSpec DeclSpec::GetSpec()
{
    switch (rawSpecifiers)
    {
    // int and its equivalent
    case _int: case _signed: case _int | _signed:
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
    case _longlong | _signed: case _longlong | _int:
    case _signed | _longlong | _int:
        return TypeSpec::int64;
    case _unsigned | _longlong: case _unsigned | _long:
    case _long | _int | _unsigned: 
    case _unsigned | _longlong | _int:
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
        throw 1001;
    }
}

unsigned int DeclSpec::GetQual()
{
    return rawQualifiers;
}

StorageSpec DeclSpec::GetStorage()
{
    return static_cast<StorageSpec>(rawStorageCls);
}

void DeclSpec::Join(const DeclSpec* ds)
{
    rawQualifiers |= ds->rawQualifiers;
    
    if ((rawSpecifiers & _long) && (ds->rawSpecifiers & _long))
    {
        rawSpecifiers &= ~_long;
        if (!((rawSpecifiers >> 5) & 1))
            rawSpecifiers |= _long << 1;
        else
            throw 1001;
    }
    
    if (ds->rawStorageCls)
    {
        if (!rawStorageCls)
            rawStorageCls = ds->rawStorageCls;
        throw 1010;
    }

    unsigned int sum = rawSpecifiers + ds->rawSpecifiers;
    rawSpecifiers ^= ds->rawSpecifiers;
    if (sum != rawSpecifiers)
        throw 1001;
}
