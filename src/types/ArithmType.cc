#include "ArithmType.h"


ArithmType::ArithmType(Tag spec)
{
    switch (spec)
    {
    case Tag::_bool:
        spec_ = static_cast<unsigned>(Spec::_bool);
        raw_ = unsigned(spec); size_ = 4; return;
    case Tag::_char:
        spec_ = static_cast<unsigned>(Spec::int8);
        raw_ = unsigned(spec); size_ = 1; return;
    case Tag::_short:
        spec_ = static_cast<unsigned>(Spec::int16);
        raw_ = unsigned(spec); size_ = 2; return;
    case Tag::_int: case Tag::_signed:
        spec_ = static_cast<unsigned>(Spec::int32);
        raw_ = unsigned(spec); size_ = 4; return;
    case Tag::_unsigned:
        spec_ = static_cast<unsigned>(Spec::int32) | 
                static_cast<unsigned>(Spec::_unsigned);
        raw_ = unsigned(spec); size_ = 4; return;
    case Tag::_long:
        spec_ = static_cast<unsigned>(Spec::int64);
        raw_ = unsigned(spec); size_ = 8; return;
    case Tag::_float:
        spec_ = static_cast<unsigned>(Spec::float32);
        raw_ = unsigned(spec); size_ = 4; return;
    case Tag::_double:
        spec_ = static_cast<unsigned>(Spec::float64);
        raw_ = unsigned(spec); size_ = 8; return;
    default: return;
    }
}


bool ArithmType::Compatible(const Type* other) const
{
    auto arithm = dynamic_cast<const ArithmType*>(other);
    if (arithm) return size_ == arithm->size_;
    auto ptr = dynamic_cast<const PtrType*>(other);
    if (ptr) return size_ == 8;
    return false;
}


bool ArithmType::IsComplete() const
{
    return true;
}

bool ArithmType::IsInteger() const
{
    return spec_ & static_cast<unsigned>(Spec::intmask);
}

bool ArithmType::IsScalar() const
{
    return spec_ & (static_cast<unsigned>(Spec::intmask) |
                    static_cast<unsigned>(Spec::floatmask));
}

bool ArithmType::IsFloat() const
{
    return spec_ & static_cast<unsigned>(Spec::floatmask);
}

bool ArithmType::IsUnsigned() const
{
    return spec_ & static_cast<unsigned>(Spec::_unsigned);
}

uint64_t ArithmType::GetSize() const
{
    return size_;
}


bool ArithmType::Raw2Spec()
{
    switch (raw_)
    {      
    // int and its equivalent
    case int(Tag::_bool):
        size_ = 4; spec_ = int(Spec::_bool); return true;

    case int(Tag::_int): case int(Tag::_signed):
    case int(Tag::_int) | int(Tag::_signed):
        size_ = 4; spec_ = int(Spec::int32) | int(Spec::_unsigned);
        return true;
    case int(Tag::_int) | int(Tag::_unsigned):
    case int(Tag::_unsigned):
        size_ = 4; spec_ = int(Spec::int32);
        return true;

    // short and its equivalent
    case int(Tag::_short): case int(Tag::_short) | int(Tag::_int):
    case int(Tag::_signed) | int(Tag::_short):
    case int(Tag::_signed) | int(Tag::_short) | int(Tag::_int):
        size_ = 2; spec_ = int(Spec::int16);
        return true;
    case int(Tag::_short) | int(Tag::_unsigned):
    case int(Tag::_short) | int(Tag::_unsigned) | int(Tag::_int):
        size_ = 2; spec_ = int(Spec::int16) | int(Spec::_unsigned);
        return true;

    // long and its equivalent
    case int(Tag::_longlong): case int(Tag::_long):
    case int(Tag::_long) | int(Tag::_signed):
    case int(Tag::_long) | int(Tag::_int):
    case int(Tag::_signed) | int(Tag::_long) | int(Tag::_int):
    case int(Tag::_longlong) | int(Tag::_signed):
    case int(Tag::_longlong) | int(Tag::_int):
    case int(Tag::_signed) | int(Tag::_longlong) | int(Tag::_int):
        size_ = 8; spec_ = int(Spec::int64);
        return true;
    case int(Tag::_unsigned) | int(Tag::_longlong):
    case int(Tag::_unsigned) | int(Tag::_long):
    case int(Tag::_long) | int(Tag::_int) | int(Tag::_unsigned): 
    case int(Tag::_unsigned) | int(Tag::_longlong) | int(Tag::_int):
        size_ = 8; spec_ = int(Spec::int64) | int(Spec::_unsigned);
        return true;

    // char
    case int(Tag::_char):
    case int(Tag::_char) | int(Tag::_signed):
        size_ = 1; spec_ = int(Spec::int8);
        return true;
    case int(Tag::_char) | int(Tag::_unsigned):
        size_ = 1; spec_ = int(Spec::int8) | int(Spec::_unsigned);
        return true;

    // double
    case int(Tag::_double): case int(Tag::_double) | int(Tag::_long):
        size_ = 8; spec_ = int(Spec::float64);
        return true;

    // float
    case int(Tag::_float):
        size_ = 4; spec_ = int(Spec::float32);
        return true;

    // error
    default: return false;
    }
}

bool ArithmType::SetSpec(Tag t)
{
    if (t == Tag::_long)
    {
        if (!(raw_ & unsigned(Tag::_long)))
            raw_ |= unsigned(Tag::_long);
        else
        {
            if (!(raw_ & (unsigned(Tag::_long) << 1)))
                raw_ |= unsigned(Tag::_long) << 1;
            else return false;
        }
        return Raw2Spec();
    }

    if (!(raw_ & unsigned(t)))
        raw_ |= static_cast<unsigned>(t);
    else return false;

    return Raw2Spec();
}


bool ArithmType::operator>(const ArithmType& rhs) const
{
    if (IsInteger() && rhs.IsFloat()) return false;
    else if (IsFloat() && rhs.IsInteger()) return true;
    else if (IsInteger() && rhs.IsInteger())
        return (spec_ & ~static_cast<unsigned>(Spec::_unsigned)) >
            (rhs.spec_ & ~static_cast<unsigned>(Spec::_unsigned));
    else if (IsFloat() && rhs.IsFloat())
        return spec_ > rhs.spec_;
    return false;
}

bool ArithmType::operator<(const ArithmType& rhs) const
{
    return !(*this > rhs);
}


std::string ArithmType::ToString() const
{
    std::string name = IsUnsigned() ? "unsigned " : "";
    if (raw_ & int(Tag::_bool))
        return "bool";
    if (raw_ & int(Tag::_char))
    {
        name += "char";
        return name;
    }   
    if (raw_ & int(Tag::_short))
        name += "short ";
    if (raw_ & int(Tag::_long))
        name += "long ";
    if (raw_ & int(Tag::_longlong))
        name += "long long ";
    if (raw_ & int(Tag::_int))
    {
        name += "int";
        return name;
    }
    if (raw_ & int(Tag::_double))
        name += "double ";
    if (raw_ & int(Tag::_float))
        name += "float";
    
    return name;
}
