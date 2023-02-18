#include "IR/IRType.h"
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

const FloatType* FloatType::GetFloat32()
{
    return &float32;
}

const FloatType* FloatType::GetFloat64()
{
    return &float64;
}


void IRTypePool::AddIRType(std::unique_ptr<IRType> ty)
{
    typelist_.push_back(std::move(ty));
}


FuncType* FuncType::GetFuncType(
    IRTypePool& pool, const IRType* retty, bool vol)
{
    auto ty = std::make_unique<FuncType>(retty, vol);
    auto addr = ty.get();
    pool.AddIRType(std::move(ty));
    return addr;
}

void FuncType::AddParam(const IRType* ty)
{
    param_.push_back(ty);
}

PtrType* PtrType::GetPtrType(IRTypePool& pool, const IRType* point2)
{
    auto ty = std::make_unique<PtrType>(point2);
    auto addr = ty.get();
    pool.AddIRType(std::move(ty));
    return addr;
}

ArrayType* ArrayType::GetArrayType(
    IRTypePool& pool, size_t size, const IRType* elety)
{
    auto ty = std::make_unique<ArrayType>(size, elety);
    auto addr = ty.get();
    pool.AddIRType(std::move(ty));
    return addr;
}

StructType* StructType::GetStructType(
    IRTypePool& pool, const std::vector<const IRType*>& list)
{
    auto ty = std::make_unique<StructType>(list);
    auto addr = ty.get();
    pool.AddIRType(std::move(ty));
    return addr;
}

UnionType* UnionType::GetUnionType(
    IRTypePool& pool, const std::vector<const IRType*>& list)
{
    auto ty = std::make_unique<UnionType>(list);
    auto addr = ty.get();
    pool.AddIRType(std::move(ty));
    return addr;
}

bool IRType::operator<(const IRType& rhs) const
{
    if (!rhs.IsArithm() || !this->IsArithm())
        return false;
    if (this->IsFloat() && !rhs.IsFloat())
        return false;
    if (!this->IsFloat() && rhs.IsFloat())
        return true;
    return this->Size() < rhs.Size();
}

bool IRType::operator>(const IRType& rhs) const
{
    if (!rhs.IsArithm() || !this->IsArithm())
        return false;
    if (this->IsFloat() && !rhs.IsFloat())
        return true;
    if (!this->IsFloat() && rhs.IsFloat())
        return false;
    return this->Size() > rhs.Size();
}


std::string IntType::ToString() const
{
    return "int" + std::to_string(size_ * 8);
}

std::string FloatType::ToString() const
{
    return "float" + std::to_string(size_ * 8);
}

std::string PtrType::ToString() const
{
    return type_->ToString() + '*';
}

std::string ArrayType::ToString() const
{
    return "[ " + std::to_string(size_) + " x " 
        + type_->ToString() + " ]";
}

std::string FuncType::ToString() const
{
    std::string paramlist{ '(' };
    for (const auto& t : param_)
        paramlist += t->ToString() + ", ";
    if (variadic_)
        paramlist += "...)";
    else
    {
        paramlist.pop_back();
        paramlist.back() = ')';            
    }
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
