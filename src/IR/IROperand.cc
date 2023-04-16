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
    bool issigned = type_->ToInteger()->IsSigned();
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


namespace std
{
string to_string(RegTag rt)
{
    switch (rt)
    {
    case RegTag::rip:   return "%rip";
    case RegTag::rax:   return "%rax";      case RegTag::rbx:   return "%rbx";
    case RegTag::rcx:   return "%rcx";      case RegTag::rdx:   return "%rdx";
    case RegTag::rsi:   return "%rsi";      case RegTag::rdi:   return "%rdi";
    case RegTag::rbp:   return "%rbp";      case RegTag::rsp:   return "%rsp";
    case RegTag::eax:   return "%eax";      case RegTag::ebx:   return "%ebx";
    case RegTag::ecx:   return "%ecx";      case RegTag::edx:   return "%edx";
    case RegTag::esi:   return "%esi";      case RegTag::edi:   return "%edi";
    case RegTag::ebp:   return "%ebp";      case RegTag::esp:   return "%esp";
    case RegTag::ax:    return "%ax";       case RegTag::bx:    return "%bx";
    case RegTag::cx:    return "%cx";       case RegTag::dx:    return "%dx";
    case RegTag::si:    return "%si";       case RegTag::di:    return "%di";
    case RegTag::bp:    return "%bp";       case RegTag::sp:    return "%sp";
    case RegTag::al:    return "%al";       case RegTag::bl:    return "%bl";
    case RegTag::cl:    return "%cl";       case RegTag::dl:    return "%dl";
    case RegTag::sil:   return "%sil";      case RegTag::dil:   return "%dil";
    case RegTag::bpl:   return "%bpl";      case RegTag::spl:   return "%spl";
    case RegTag::r8:    return "%r8";       case RegTag::r9:    return "%r9";
    case RegTag::r10:   return "%r10";      case RegTag::r11:   return "%r11";
    case RegTag::r12:   return "%r12";      case RegTag::r13:   return "%r13";
    case RegTag::r14:   return "%r14";      case RegTag::r15:   return "%r15";
    case RegTag::r8d:   return "%r8d";      case RegTag::r9d:   return "%r9d";
    case RegTag::r10d:  return "%r10d";     case RegTag::r11d:  return "%r11d";
    case RegTag::r12d:  return "%r12d";     case RegTag::r13d:  return "%r13d";
    case RegTag::r14d:  return "%r14d";     case RegTag::r15d:  return "%r15d";
    case RegTag::r8w:   return "%r8w";      case RegTag::r9w:   return "%r9w";
    case RegTag::r10w:  return "%r10w";     case RegTag::r11w:  return "%r11w";
    case RegTag::r12w:  return "%r12w";     case RegTag::r13w:  return "%r13w";
    case RegTag::r14w:  return "%r14w";     case RegTag::r15w:  return "%r15w";
    case RegTag::r8b:   return "%r8b";      case RegTag::r9b:   return "%r9b";
    case RegTag::r10b:  return "%r10b";     case RegTag::r11b:  return "%r11b";
    case RegTag::r12b:  return "%r12b";     case RegTag::r13b:  return "%r13b";
    case RegTag::r14b:  return "%r14b";     case RegTag::r15b:  return "%r15b";
    case RegTag::xmm0:  return "%xmm0";     case RegTag::xmm1:  return "%xmm1";
    case RegTag::xmm2:  return "%xmm2";     case RegTag::xmm3:  return "%xmm3";
    case RegTag::xmm4:  return "%xmm4";     case RegTag::xmm5:  return "%xmm5";
    case RegTag::xmm6:  return "%xmm6";     case RegTag::xmm7:  return "%xmm7";
    case RegTag::xmm8:  return "%xmm8";     case RegTag::xmm9:  return "%xmm9";
    case RegTag::xmm10: return "%xmm10";    case RegTag::xmm11: return "%xmm11";
    case RegTag::xmm12: return "%xmm12";    case RegTag::xmm13: return "%xmm13";
    case RegTag::xmm14: return "%xmm14";    case RegTag::xmm15: return "%xmm15";
    }

    return ""; // make the compiler happy
}
}

x64Reg* x64Reg::CreateX64Reg(Pool<IROperand>* pool, RegTag reg)
{
    auto reg_ = std::make_unique<x64Reg>(reg);
    auto raw = reg_.get();
    pool->Add(std::move(reg_));
    return raw;
}

bool x64Reg::PartOf(x64Reg& reg) const
{
    return PartOf(reg.reg_);
}

bool x64Reg::PartOf(RegTag tag) const
{
    int cur = static_cast<int>(reg_);
    int t = static_cast<int>(tag);
    if (cur <= int(RegTag::rip) || t >= int(RegTag::r15b))
        return false;
    return (cur - t) % 8 == 0;
}

std::string x64Reg::ToString() const
{
    return std::to_string(reg_);
}

x64Mem* x64Mem::CreateX64Mem(Pool<IROperand>* pool, long offset,
    const x64Reg* base, const x64Reg* index, size_t scale)
{
    auto mem = std::make_unique<x64Mem>(offset, base, index, scale);
    auto raw = mem.get();
    pool->Add(std::move(mem));
    return raw;
}

std::string x64Mem::ToString() const
{
    if (!label_.empty())
        return label_ + "(%rip)";

    std::string loc = "";

    if (offset_ != 0)
        loc += std::to_string(offset_);
    if (base_->operator==(RegTag::none) && index_->operator==(RegTag::none))
        return loc;

    loc += '(';

    if (base_->operator!=(RegTag::none))
        loc += base_->ToString();
    if (index_ ->operator!=(RegTag::none))
        loc += ", " + index_->ToString();
    if (scale_ != 0)
        loc += ", " + std::to_string(scale_);

    loc += ')';
    return loc;
}
