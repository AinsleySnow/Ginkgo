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
    unsigned long numcopy = 0;
    auto byte = reinterpret_cast<const char*>(&num_);

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    for (int i = 0; i < type_->Size(); ++i)
        reinterpret_cast<char*>(&numcopy)[i] = *(byte + i);
#else
    for (int i = 7; i >= 8 - type_->Size(); --i)
        reinterpret_cast<char*>(&numcopy)[i] = *(byte + i);
#endif

    if (type_->ToInteger()->IsSigned())
        return std::to_string((long)numcopy);
    else
        return std::to_string(numcopy);
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
