#include "ast/CType.h"
#include "ast/Declaration.h"
#include "IR/IRType.h"
#include "visitast/Identifier.h"
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


CArithmType::CArithmType(TypeTag tt, size_t a) : CType(CTypeId::arithm), type_(tt)
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
    case TypeTag::flt32:
        size_ = 4; break;
    case TypeTag::flt64:
        size_ = 8; break;
    default: break;
    }
    if (a != 0)
        align_ = a;
    else
        align_ = size_;
}

CArithmType::CArithmType(TypeTag tt) : CArithmType(tt, 0)
{
}

const IRType* CArithmType::ToIRType(Pool<IRType>* pool) const
{
    switch (type_)
    {
    case TypeTag::int8:
        if (align_ != size_) return IntType::GetInt8(pool, Align(), true);
        else                 return IntType::GetInt8(true);
    case TypeTag::int16:
        if (align_ != size_) return IntType::GetInt16(pool, Align(), true);
        else                 return IntType::GetInt16(true);
    case TypeTag::int32:
        if (align_ != size_) return IntType::GetInt32(pool, Align(), true);
        else                 return IntType::GetInt32(true);
    case TypeTag::int64:
        if (align_ != size_) return IntType::GetInt64(pool, Align(), true);
        else                 return IntType::GetInt64(true);
    case TypeTag::uint8:
        if (align_ != size_) return IntType::GetInt8(pool, Align(), false);
        else                 return IntType::GetInt8(false);
    case TypeTag::uint16:
        if (align_ != size_) return IntType::GetInt16(pool, Align(), false);
        else                 return IntType::GetInt16(false);
    case TypeTag::uint32:
        if (align_ != size_) return IntType::GetInt32(pool, Align(), false);
        else        return IntType::GetInt32(false);
    case TypeTag::uint64:
        if (align_ != size_) return IntType::GetInt64(pool, Align(), false);
        else                 return IntType::GetInt64(false);
    case TypeTag::flt32:
        if (align_ != size_) return FloatType::GetFloat32(pool, Align());
        else                 return FloatType::GetFloat32();
    case TypeTag::flt64:
        if (align_ != size_) return FloatType::GetFloat64(pool, Align());
        else                 return FloatType::GetFloat64();
    }
    return nullptr;
}

bool CArithmType::Compatible(const CType& other) const
{
    if (other.Is<CArithmType>())
        return size_ == other.Size();
    if (other.Is<CPtrType>())
        return size_ == other.Size();
    return false;
}

bool CArithmType::operator>(const CArithmType& rhs) const
{
    if (IsInteger() && rhs.IsFloat()) return false;
    else if (IsFloat() && rhs.IsInteger()) return true;
    else if (IsInteger() && rhs.IsInteger())
        return this->size_ > rhs.size_;
    else if (IsFloat() && rhs.IsFloat())
        return this->size_ > rhs.size_;
    return false;
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
    paramlist_.push_back(t);
}

const FuncType* CFuncType::ToIRType(Pool<IRType>* pool) const
{
    auto functy = FuncType::GetFuncType(
        pool, ReturnType()->ToIRType(pool), Variadic());
    for (auto& param : paramlist_)
        if (!param->Is<CVoidType>())
            functy->AddParam(param->ToIRType(pool));
    return functy;
}

std::unique_ptr<CType> CFuncType::Clone() const
{
    auto func = std::make_unique<CFuncType>();

    // Alignment and size for function types are always 0.

    func->Qual() = Qual();
    func->Storage() = Storage();
    func->variadic_ = variadic_;
    func->inline_ = inline_;
    func->noreturn_ = noreturn_;
    func->paramlist_ = paramlist_;
    func->return_ = std::move(return_->Clone());
    return std::move(func);
}


std::string CPtrType::ToString() const
{
    return "";
}

const PtrType* CPtrType::ToIRType(Pool<IRType>* pool) const
{
    auto point2 = point2_->ToIRType(pool);
    return align_ ?
        PtrType::GetPtrType(pool, align_, point2) :
        PtrType::GetPtrType(pool, point2);
}

std::unique_ptr<CType> CPtrType::Clone() const
{
    auto ptr = std::make_unique<CPtrType>();
    ptr->size_ = Size();
    ptr->align_ = Align();
    ptr->Qual() = Qual();
    ptr->Storage() = Storage();
    ptr->point2_ = std::move(point2_->Clone());
    return std::move(ptr);
}


const IRType* CArrayType::ToIRType(Pool<IRType>* pool) const
{
    // if an array type occurs in a function's parameter list,
    // it is possible to be incomplete since the parameter is
    // passed as if it were a pointer.
    if (!IsComplete() && !IsParam())
        return VoidType::GetVoidType();

    auto arrayof = arrayof_->ToIRType(pool);
    if (isparam_)
        return PtrType::GetPtrType(pool, arrayof);

    auto array = align_ ?
        ArrayType::GetArrayType(pool, count_, align_, arrayof) :
        ArrayType::GetArrayType(pool, count_, arrayof);

    array->VariableLen() = variable_;
    array->Static() = static_;
    return array;
}

std::string CArrayType::ToString() const
{
    return "";
}

std::unique_ptr<CType> CArrayType::Clone() const
{
    auto array = std::make_unique<CArrayType>();
    array->size_ = Size();
    array->align_ = Align();
    array->Qual() = Qual();
    array->Storage() = Storage();
    array->arrayof_ = std::move(arrayof_->Clone());
    array->count_ = count_;
    array->isparam_ = isparam_;
    array->variable_ = variable_;
    array->static_ = static_;
    return std::move(array);
}


std::string CEnumType::ToString() const
{
    return "";
}

const IRType* CEnumType::ToIRType(Pool<IRType>* pool) const
{
    if (!IsComplete())
        return VoidType::GetVoidType();

    if (align_ != 0)
        underlying_->Align() = align_;
    return static_cast<const IntType*>(underlying_->ToIRType(pool));
}

std::unique_ptr<CType> CEnumType::Clone() const
{
    auto underlying = underlying_->Clone();
    auto ty = std::make_unique<CEnumType>(name_, std::move(underlying), align_);
    ty->members_ = members_;
    return std::move(ty);
}


static size_t FindLCM(size_t a, size_t b)
{
    size_t max = (a > b) ? a : b;
    size_t min = (a < b) ? a : b;
    for (size_t i = 1; i <= min; i++)
    {
        size_t multiple = max * i;
        if (multiple % min == 0)
            return multiple;
    }
    return min; // make the compiler happy
}


#define TOIR_HELPER(name)                                       \
{                                                               \
    auto ty = name::Get##name(pool, irname_, size_, align_);    \
    for (auto& [_, t, o] : members_)                            \
        ty->AddField(t->ToIRType(pool), o);                     \
    return ty;                                                  \
}

const IRType* CHeterType::ToIRType(Pool<IRType>* pool) const
{
    if (!IsComplete())
        return VoidType::GetVoidType();
    if (id_ == CTypeId::_struct)
        TOIR_HELPER(StructType)
    else
        TOIR_HELPER(UnionType)
}

#undef TOIR_HELPER

bool CHeterType::AddMember(const std::string& n, const CType* t, bool m, int i)
{
    // can we update size_ and align_?
    // (to avoid suming up a same field's size again and again)
    bool cancalc = members_.size() <= i;
    // to avoid repeating add a same field
    if (members_.size() <= i)
        members_.push_back(std::make_tuple(m, t, offset_));
    fieldindex_[n] = i;
    return cancalc;
}

void CHeterType::AlignOffsetBy(size_t align)
{
    if (offset_ % align == 0)
        return;
    offset_ += align - offset_ % align;
}

void CHeterType::UpdateSize()
{
    tailpadding_ = offset_ % align_ ?
        align_ - offset_ % align_ : 0;
    size_ = offset_ + tailpadding_;
}


void CStructType::AddStructMember(
    const std::string& n, const CType* t, bool m, int i)
{
    AlignOffsetBy(t->Align());
    if (!AddMember(n, t, m, i))
        return;
    // Align is always a power of 2
    if (align_ < t->Align())
        align_ = t->Align();
    offset_ += t->Size();
    UpdateSize();
}


void CUnionType::AddUnionMember(const std::string& n, const CType* t, bool m, int i)
{
    if (!AddMember(n, t, m, i))
        return;
    if (align_ < t->Align())
        align_ = t->Align();
    if (size_ < t->Size())
        size_ = t->Size();
}


const VoidType* CVoidType::ToIRType(Pool<IRType>*) const
{
    return VoidType::GetVoidType();
}
