#include "IR/IROperand.h"
#include "IR/Value.h"
#include <climits>
#include <cfloat>


IntConst* IntConst::CreateIntConst(Function* func, unsigned long ul)
{
    if (ul < UINT8_MAX)
        return CreateIntConst(func, ul, IntType::GetInt8(false));
    else if (ul < UINT16_MAX)
        return CreateIntConst(func, ul, IntType::GetInt16(false));
    else if (ul < UINT32_MAX)
        return CreateIntConst(func, ul, IntType::GetInt32(false));
    return CreateIntConst(func, ul, IntType::GetInt64(false));
}

IntConst* IntConst::CreateIntConst(Function* func, unsigned long ul, const IntType* t)
{
    auto intconst = std::make_unique<IntConst>(ul, t);
    auto raw = intconst.get();
    func->AddIROperand(std::move(intconst));
    return raw;
}

std::string IntConst::ToString() const
{
    int bits = type_->Size() * 8;
    bool issigned = type_->ToInteger()->IsSigned();
    unsigned long mask = ((unsigned long)(-1ll >> (bits - 1))) >> (64 - bits + 1);

    if (issigned && (1 << (bits - 1)) & num_)
        return std::to_string((long)(mask & num_) | -1ll >> (64 - bits + 1));
    else if (issigned)
        return std::to_string((long)(mask & num_));
    else return std::to_string(mask & num_);
}

FloatConst* FloatConst::CreateFloatConst(Function* func, double d)
{
    if (d < FLT_MAX)
        return CreateFloatConst(func, d, FloatType::GetFloat32());
    return CreateFloatConst(func, d, FloatType::GetFloat64());
}

FloatConst* FloatConst::CreateFloatConst(Function* func, double d, const FloatType* t)
{
    auto floatconst = std::make_unique<FloatConst>(d, t);
    auto raw = floatconst.get();
    func->AddIROperand(std::move(floatconst));
    return raw;
}

std::string FloatConst::ToString() const
{
    if (type_->Size() == 4)
        return std::to_string((float)num_);
    else
        return std::to_string(num_);
}

Register* Register::CreateRegister(
    Function* func, const std::string& name, const IRType* ty)
{
    auto reg = std::make_unique<Register>(name, ty);
    auto raw = reg.get();
    func->AddIROperand(std::move(reg));
    return raw;
}

std::string Register::ToString() const
{
    return type_->ToString() + ' ' + name_;
}
