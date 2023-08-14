#include "IR/IRType.h"
#include "IR/IROperand.h"
#include <memory>


const static IntType int8 = IntType(1, true);
const static IntType int16 = IntType(2, true);
const static IntType int32 = IntType(4, true);
const static IntType int64 = IntType(8, true);
const static IntType uint8 = IntType(1, false);
const static IntType uint16 = IntType(2, false);
const static IntType uint32 = IntType(4, false);
const static IntType uint64 = IntType(8, false);
const static FloatType float32 = FloatType(4);
const static FloatType float64 = FloatType(8);


const IntType* IntType::GetInt8(bool s)
{
    if (s) return &int8;
    return &uint8;
}

const IntType* IntType::GetInt16(bool s)
{
    if (s) return &int16;
    return &uint16;
}

const IntType* IntType::GetInt32(bool s)
{
    if (s) return &int32;
    return &uint32;
}

const IntType* IntType::GetInt64(bool s)
{
    if (s) return &int64;
    return &uint64;
}

#define GET_INT_HELPER(size)                            \
auto ptr = std::make_unique<IntType>(size, align, s);   \
auto raw = ptr.get();                                   \
pool->Add(std::move(ptr));                              \
return raw

const IntType* IntType::GetInt8(Pool<IRType>* pool, size_t align, bool s) { GET_INT_HELPER(1); }
const IntType* IntType::GetInt16(Pool<IRType>* pool, size_t align, bool s) { GET_INT_HELPER(2); }
const IntType* IntType::GetInt32(Pool<IRType>* pool, size_t align, bool s) { GET_INT_HELPER(4); }
const IntType* IntType::GetInt64(Pool<IRType>* pool, size_t align, bool s) { GET_INT_HELPER(8); }

#undef GET_INT_HELPER


const FloatType* FloatType::GetFloat32()
{
    return &float32;
}

const FloatType* FloatType::GetFloat64()
{
    return &float64;
}

#define GET_FLOAT_HELPER(size)                          \
auto ptr = std::make_unique<FloatType>(size, align);    \
auto raw = ptr.get();                                   \
pool->Add(std::move(ptr));                              \
return raw

const FloatType* FloatType::GetFloat32(Pool<IRType>* pool, size_t align) { GET_FLOAT_HELPER(4); }
const FloatType* FloatType::GetFloat64(Pool<IRType>* pool, size_t align) { GET_FLOAT_HELPER(8); }

#undef GET_FLOAT_HELPER


FuncType* FuncType::GetFuncType(
    Pool<IRType>* pool, const IRType* retty, bool vol)
{
    auto ty = std::make_unique<FuncType>(retty, vol);
    auto addr = ty.get();
    pool->Add(std::move(ty));
    return addr;
}

void FuncType::AddParam(const IRType* ty)
{
    param_.push_back(ty);
}


PtrType* PtrType::GetPtrType(Pool<IRType>* pool, const IRType* point2)
{
    auto ty = std::make_unique<PtrType>(point2);
    auto addr = ty.get();
    pool->Add(std::move(ty));
    return addr;
}

PtrType* PtrType::GetPtrType(Pool<IRType>* pool, size_t align, const IRType* point2)
{
    auto ptr = GetPtrType(pool, point2);
    ptr->align_ = align;
    return ptr;
}


ArrayType* ArrayType::GetArrayType(
    Pool<IRType>* pool, size_t count, const IRType* elety)
{
    auto ty = std::make_unique<ArrayType>(count, elety);
    auto addr = ty.get();
    pool->Add(std::move(ty));
    return addr;
}

ArrayType* ArrayType::GetArrayType(
    Pool<IRType>* pool, size_t count, size_t align, const IRType* elety)
{
    auto array = GetArrayType(pool, count, elety);
    array->align_ = align;
    return array;
}

ArrayType::ArrayType(size_t count, const IRType* t) :
    IRType(TypeId::array), type_(t)
{
    size_ = count * t->Size();
    align_ = t->Align();
    count_ = count;
}

ArrayType::ArrayType(size_t count, size_t align, const IRType* t) :
    IRType(TypeId::array), type_(t)
{
    size_ = count * t->Size();
    align_ = align;
    count_ = count;
}

StructType* StructType::GetStructType(Pool<IRType>* pool, const std::string& name)
{
    auto ty = std::make_unique<StructType>(name);
    auto addr = ty.get();
    pool->Add(std::move(ty));
    return addr;
}

UnionType* UnionType::GetUnionType(Pool<IRType>* pool, const std::string& name)
{
    auto ty = std::make_unique<UnionType>(name);
    auto addr = ty.get();
    pool->Add(std::move(ty));
    return addr;
}

bool IRType::operator<(const IRType& rhs) const
{
    if (!rhs.IsArithm() || !this->IsArithm())
        return false;
    if (this->Is<FloatType>() && !rhs.Is<FloatType>())
        return false;
    if (!this->Is<FloatType>() && rhs.Is<FloatType>())
        return true;
    return this->Size() < rhs.Size();
}

bool IRType::operator>(const IRType& rhs) const
{
    if (!rhs.IsArithm() || !this->IsArithm())
        return false;
    if (this->Is<FloatType>() && !rhs.Is<FloatType>())
        return true;
    if (!this->Is<FloatType>() && rhs.Is<FloatType>())
        return false;
    return this->Size() > rhs.Size();
}

bool IRType::operator==(const IRType& rhs) const
{
    if (this->Is<FloatType>() && rhs.Is<FloatType>())
        return this->Size() == rhs.Size();
    if (this->Is<IntType>() && rhs.Is<IntType>())
        return this->Size() == rhs.Size() &&
            this->As<IntType>()->IsSigned() == rhs.As<IntType>()->IsSigned();
    return false;
}


std::string IntType::ToString() const
{
    auto size = std::to_string(size_ * 8);
    return signed_ ? 'i' + size : 'u' + size;
}

std::string FloatType::ToString() const
{
    return 'f' + std::to_string(size_ * 8);
}

std::string PtrType::ToString() const
{
    return type_->ToString() + '*';
}

std::string ArrayType::ToString() const
{
    return "[" + std::to_string(count_) + " x " 
        + type_->ToString() + "]";
}

std::string FuncType::ToString() const
{
    std::string paramlist{ '(' };
    if (!param_.empty())
    {
        for (auto t = param_.begin(); t < param_.end() - 1; ++t)
            paramlist += (*t)->ToString() + ", ";
        paramlist += param_.back()->ToString();
    }
    if (variadic_)
        paramlist += ", ...)";
    else
        paramlist += ')';
    return retype_->ToString() + ' ' + paramlist;
}


std::string StructType::ToString() const
{
    std::string fields{ "struct { " };
    for (const auto& pfield : fields_)
        fields += pfield->ToString() + ", ";
    fields.back() = '}';
    *(fields.end() - 1) = ' ';
    return fields;
}


std::string UnionType::ToString() const
{
    std::string fields{ "union { " };
    for (const auto& pfield : fields_)
        fields += pfield->ToString() + ", ";
    fields.back() = '}';
    *(fields.end() - 1) = ' ';
    return fields;
}


const static VoidType _void = VoidType();

const VoidType* VoidType::GetVoidType()
{
    return &_void;
}
