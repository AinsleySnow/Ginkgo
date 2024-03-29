#include "IR/IROperand.h"
#include <climits>
#include <cfloat>


IntConst* IntConst::CreateIntConst(Pool<IROperand>* pool, unsigned long ul)
{
    if (ul < UINT8_MAX)
        return CreateIntConst(pool, ul, IntType::GetInt8(false));
    else if (ul < UINT16_MAX)
        return CreateIntConst(pool, ul, IntType::GetInt16(false));
    else if (ul < UINT32_MAX)
        return CreateIntConst(pool, ul, IntType::GetInt32(false));
    return CreateIntConst(pool, ul, IntType::GetInt64(false));
}

IntConst* IntConst::CreateIntConst(Pool<IROperand>* pool, unsigned long ul, const IntType* t)
{
    auto intconst = std::make_unique<IntConst>(ul, t);
    auto raw = intconst.get();
    pool->Add(std::move(intconst));
    return raw;
}

std::string IntConst::ToString() const
{
    int bits = type_->Size() * 8;
    bool issigned = type_->As<IntType>()->IsSigned();
    unsigned long mask = ((unsigned long)(-1ll)) >> (64 - bits);

    if (issigned && (1 << (bits - 1)) & num_)
        return type_->ToString() + ' ' +
            std::to_string((long)(mask & num_) | ((~INT64_MAX) >> (64 - bits)));
    else if (issigned)
        return type_->ToString() + ' ' + std::to_string((long)(mask & num_));
    else return type_->ToString() + ' ' + std::to_string(mask & num_);
}

FloatConst* FloatConst::CreateFloatConst(Pool<IROperand>* pool, double d)
{
    if (d < FLT_MAX)
        return CreateFloatConst(pool, d, FloatType::GetFloat32());
    return CreateFloatConst(pool, d, FloatType::GetFloat64());
}

FloatConst* FloatConst::CreateFloatConst(Pool<IROperand>* pool, double d, const FloatType* t)
{
    auto floatconst = std::make_unique<FloatConst>(d, t);
    auto raw = floatconst.get();
    pool->Add(std::move(floatconst));
    return raw;
}

std::string FloatConst::ToString() const
{
    if (type_->Size() == 4)
        return type_->ToString() + ' ' + std::to_string((float)num_);
    else
        return type_->ToString() + ' ' + std::to_string(num_);
}

StrConst* StrConst::CreateStrConst(Pool<IROperand>* pool, const std::string& str, const ArrayType* ty)
{
    auto strconst = std::make_unique<StrConst>(str, ty);
    auto raw = strconst.get();
    pool->Add(std::move(strconst));
    return raw;
}


Register* Register::CreateRegister(
    Pool<IROperand>* pool, const std::string& name, const IRType* ty)
{
    auto reg = std::make_unique<Register>(name, ty);
    auto raw = reg.get();
    pool->Add(std::move(reg));
    return raw;
}

std::string Register::ToString() const
{
    return type_->ToString() + ' ' + name_;
}
