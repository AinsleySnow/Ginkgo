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

const FloatType* FloatType::GetFloat32()
{
    return &float32;
}

const FloatType* FloatType::GetFloat64()
{
    return &float64;
}


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


ArrayType* ArrayType::GetArrayType(
    Pool<IRType>* pool, size_t count, const IRType* elety)
{
    auto ty = std::make_unique<ArrayType>(count, elety);
    auto addr = ty.get();
    pool->Add(std::move(ty));
    return addr;
}

ArrayType::ArrayType(size_t count, const IRType* t) :
    IRType(TypeId::array), type_(t)
{
    size_ = count * t->Size();
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
    return 'i' + std::to_string(size_ * 8);
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


void HeterType::AlignSizeBy(size_t align)
{
    if (size_ % align == 0)
        return;
    size_ += align - size_ % align;
}

size_t HeterType::CalcAlign()
{
    align_ = fields_[0]->Align();
    for (int i = 1; i < fields_.size(); ++i)
        align_ = FindLCM(align_, fields_[i]->Align());
    return align_;
}


size_t StructType::CalcSize()
{
    if (size_) return size_;

    size_ = fields_[0]->Size();
    for (int i = 1; i < fields_.size(); ++i)
    {
        AlignSizeBy(fields_[i]->Align());
        size_ += fields_[i]->Size();
    }
    AlignSizeBy(align_);
    return size_;
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


size_t UnionType::CalcSize()
{
    if (size_) return size_;

    for (auto f : fields_)
        if (f->Size() > size_)
            size_ = f->Size();
    AlignSizeBy(align_);
    return size_;
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
