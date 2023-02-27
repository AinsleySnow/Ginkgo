#include "ast/CType.h"
#include "ast/Declaration.h"
#include "IR/IRType.h"
#include <memory>


bool QualType::SetToken(Tag t)
{
    switch (t)
    {
        case Tag::_const:
            token_ |= static_cast<unsigned>(QualTag::_const); return true;
        case Tag::_restrict:
            token_ |= static_cast<unsigned>(QualTag::_restrict); return true;
        case Tag::_volatile:
            token_ |= static_cast<unsigned>(QualTag::_volatile); return true;
        case Tag::_atomic:
            token_ |= static_cast<unsigned>(QualTag::_atomic); return true;
        default:
            return false;
    }
}


bool StorageType::SetToken(Tag t)
{
    if (token_) return false;
    switch (t)
    {
        case Tag::_static:
            token_ |= static_cast<unsigned>(StorageTag::_static); return true;
        case Tag::_extern:
            token_ |= static_cast<unsigned>(StorageTag::_extern); return true;
        case Tag::_typedef:
            token_ |= static_cast<unsigned>(StorageTag::_typedef); return true;
        case Tag::_thread_local:
            token_ |= static_cast<unsigned>(StorageTag::_thread_local); return true;
        case Tag::_auto:
            token_ |= static_cast<unsigned>(StorageTag::_auto); return true;
        case Tag::_register:
            token_ |= static_cast<unsigned>(StorageTag::_register); return true;
        default:
            return false;
    }
}


bool FuncSpec::SetSpec(Tag t)
{
    switch (t)
    {
    case Tag::_inline:
        token_ |= (unsigned)FuncTag::_inline; return true;
    case Tag::_noreturn:
        token_ |= (unsigned)FuncTag::_noreturn; return true;
    default: return false;
    }
}


CArithmType::CArithmType(TypeTag tt) : type_(tt)
{
    switch (tt)
    {
    case TypeTag::int8: case TypeTag::uint8:
        size_ = 1; break;
    case TypeTag::int16: case TypeTag::uint16:
        size_ = 2; break;
    case TypeTag::int32: case TypeTag::uint32:
        size_ = 4; break;
    case TypeTag::int64: case TypeTag::uint64:
        size_ = 8; break;
    default: break;
    }
}

std::unique_ptr<CPtrType> CArithmType::AttachPtr(const Ptr* ptr) const
{
    auto current = ptr->point2_.get();
    auto ptrtype = std::make_unique<CPtrType>();
    auto* ptrcur = &ptrtype;
    (*ptrcur)->Qual() = current->qual_;

    current = current->point2_.get();
    while (current)
    {
        (*ptrcur)->Point2() = std::make_unique<CPtrType>();
        ptrcur = reinterpret_cast<
            std::unique_ptr<CPtrType>*>(&(*ptrcur)->Point2());
        (*ptrcur)->Qual() = current->qual_;
        current = current->point2_.get();
    }

    (*ptrcur)->Point2() = std::make_unique<CArithmType>(*this);
    return ptrtype;
}

const IRType* CArithmType::ToIRType(ITypePool*) const
{
    switch (type_)
    {
    case TypeTag::int8: return IntType::GetInt8(true);
    case TypeTag::int16: return IntType::GetInt16(true);
    case TypeTag::int32: return IntType::GetInt32(true);
    case TypeTag::int64: return IntType::GetInt64(true);
    case TypeTag::uint8: return IntType::GetInt8(false);
    case TypeTag::uint16: return IntType::GetInt16(false);
    case TypeTag::uint32: return IntType::GetInt32(false);
    case TypeTag::uint64: return IntType::GetInt64(false);
    case TypeTag::flt32: return FloatType::GetFloat32();
    case TypeTag::flt64: return FloatType::GetFloat64();
    }
    return nullptr;
}

bool CArithmType::Compatible(const CType* other) const
{
    /*auto arithm = other->ToArithm();
    if (arithm) return size_ == arithm->size_;
    auto ptr = other->ToPtr();
    if (ptr) return size_ == 8;
    return false;*/
    return true;
}

bool CArithmType::operator>(const CArithmType& rhs) const
{
    /*if (IsInteger() && rhs.IsFloat()) return false;
    else if (IsFloat() && rhs.IsInteger()) return true;
    else if (IsInteger() && rhs.IsInteger())
        return (spec_ & ~static_cast<unsigned>(Spec::_unsigned)) >
            (rhs.spec_ & ~static_cast<unsigned>(Spec::_unsigned));
    else if (IsFloat() && rhs.IsFloat())
        return spec_ > rhs.spec_;
    return false;*/
    return true;
}

bool CArithmType::operator<(const CArithmType& rhs) const
{
    return !(*this > rhs);
}

std::string CArithmType::ToString() const
{
    /*std::string name = IsUnsigned() ? "unsigned " : "";
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
    
    return name;*/
    return "";
}


void CFuncType::AddParam(const CType* t)
{
    paramlist_.push_back(std::move(t));
}

const FuncType* CFuncType::ToIRType(ITypePool* pool) const
{
    auto functy = FuncType::GetFuncType(
        pool, ReturnType()->ToIRType(pool), Variadic());
    for (auto& param : paramlist_)
        if (!param->IsVoid())
            functy->AddParam(param->ToIRType(pool));
    return functy;
}


const VoidType* CVoidType::ToIRType(ITypePool*) const
{
    return VoidType::GetVoidType();
}
