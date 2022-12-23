#include "IRType.h"
#include <memory>


std::vector<std::unique_ptr<IRType>> IRType::customedtypes_;

const static IntType int8 = IntType(IntType::intype::int8, true);
const static IntType int16 = IntType(IntType::intype::int16, true);
const static IntType int32 = IntType(IntType::intype::int32, true);
const static IntType int64 = IntType(IntType::intype::int64, true);
const static IntType uint8 = IntType(IntType::intype::int8, false);
const static IntType uint16 = IntType(IntType::intype::int16, false);
const static IntType uint32 = IntType(IntType::intype::int32, false);
const static IntType uint64 = IntType(IntType::intype::int64, false);
const static FloatType float32 = FloatType(FloatType::fltype::flt32);
const static FloatType float64 = FloatType(FloatType::fltype::flt64);


const IntType* IRType::GetInt8(bool s)
{
    if (s) return &int8;
    return &uint8;
}

const IntType* IRType::GetInt16(bool s)
{
    if (s) return &int16;
    return &uint16;
}

const IntType* IRType::GetInt32(bool s)
{
    if (s) return &int32;
    return &uint32;
}

const IntType* IRType::GetInt64(bool s)
{
    if (s) return &int64;
    return &uint64;
}

const FloatType* IRType::GetFloat32()
{
    return &float32;
}

const FloatType* IRType::GetFloat64()
{
    return &float64;
}

const FuncType* IRType::GetFunction(const IRType* retty, const std::vector<const IRType*>& list, bool vol)
{
    auto ty = std::make_unique<FuncType>(std::move(retty), list, vol);
    auto addr = ty.get();
    customedtypes_.push_back(std::move(ty));
    return addr;
}

const PtrType* IRType::GetPointer(const IRType* point2)
{
    auto ty = std::make_unique<PtrType>(point2);
    auto addr = ty.get();
    customedtypes_.push_back(std::move(ty));
    return addr;
}

const ArrayType* IRType::GetArray(size_t size, const IRType* elety)
{
    auto ty = std::make_unique<ArrayType>(size, elety);
    auto addr = ty.get();
    customedtypes_.push_back(std::move(ty));
    return addr;
}

const StructType* IRType::GetStruct(const std::vector<const IRType*>& list)
{
    auto ty = std::make_unique<StructType>(list);
    auto addr = ty.get();
    customedtypes_.push_back(std::move(ty));
    return addr;
}

const UnionType* IRType::GetUnion(const std::vector<const IRType*>& list)
{
    auto ty = std::make_unique<UnionType>(list);
    auto addr = ty.get();
    customedtypes_.push_back(std::move(ty));
    return addr;
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
