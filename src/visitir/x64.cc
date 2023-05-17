#include "visitir/x64.h"
#include "IR/IROperand.h"
#include <cstring>


bool x64::operator==(const x64& x) const
{
    if (x.id_ != id_) return false;
    if (x.id_ == x64Id::reg)
        return *(this->As<x64Reg>()) == *(x.As<x64Reg>());
    if (x.id_ == x64Id::mem)
        return *(this->As<x64Mem>()) == *(x.As<x64Mem>());
    if (x.id_ == x64Id::imm)
        return *(this->As<x64Imm>()) == *(x.As<x64Imm>());
    return false;
}


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch-enum"
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#pragma GCC diagnostic ignored "-Wreturn-type"

std::string x64Reg::ToString() const
{
    switch (reg_)
    {
    case RegTag::rip:   return "%rip";
    case RegTag::rax:
        if (Size() == 1) return "%al";  if (Size() == 2) return "%ax";
        if (Size() == 4) return "%eax"; if (Size() == 8) return "%rax";
    case RegTag::rbx:
        if (Size() == 1) return "%bl";  if (Size() == 2) return "%bx";
        if (Size() == 4) return "%ebx"; if (Size() == 8) return "%rbx";
    case RegTag::rcx:
        if (Size() == 1) return "%cl";  if (Size() == 2) return "%cx";
        if (Size() == 4) return "%ecx"; if (Size() == 8) return "%rcx";
    case RegTag::rdx:
        if (Size() == 1) return "%dl";  if (Size() == 2) return "%dx";
        if (Size() == 4) return "%edx"; if (Size() == 8) return "%rdx";
    case RegTag::rsi:
        if (Size() == 1) return "%sil"; if (Size() == 2) return "%si";
        if (Size() == 4) return "%esi"; if (Size() == 8) return "%rsi";
    case RegTag::rdi:
        if (Size() == 1) return "%dil"; if (Size() == 2) return "%di";
        if (Size() == 4) return "%edi"; if (Size() == 8) return "%rdi";
    case RegTag::rbp:
        if (Size() == 1) return "%bpl"; if (Size() == 2) return "%bp";
        if (Size() == 4) return "%ebp"; if (Size() == 8) return "%rbp";
    case RegTag::rsp:
        if (Size() == 1) return "%spl"; if (Size() == 2) return "%sp";
        if (Size() == 4) return "%esp"; if (Size() == 8) return "%rsp";
    case RegTag::r8:
        if (Size() == 1) return "%r8b"; if (Size() == 2) return "%r8w";
        if (Size() == 4) return "%r8d"; if (Size() == 8) return "%r8";
    case RegTag::r9:
        if (Size() == 1) return "%r9b"; if (Size() == 2) return "%r9w";
        if (Size() == 4) return "%r9d"; if (Size() == 8) return "%r9";
    case RegTag::r10:
        if (Size() == 1) return "%r10b"; if (Size() == 2) return "%r10w";
        if (Size() == 4) return "%r10d"; if (Size() == 8) return "%r10";
    case RegTag::r11:
        if (Size() == 1) return "%r11b"; if (Size() == 2) return "%r11w";
        if (Size() == 4) return "%r11d"; if (Size() == 8) return "%r11";
    case RegTag::r12:
        if (Size() == 1) return "%r12b"; if (Size() == 2) return "%r12w";
        if (Size() == 4) return "%r12d"; if (Size() == 8) return "%r12";
    case RegTag::r13:
        if (Size() == 1) return "%r13b"; if (Size() == 2) return "%r13w";
        if (Size() == 4) return "%r13d"; if (Size() == 8) return "%r13";
    case RegTag::r14:
        if (Size() == 1) return "%r14b"; if (Size() == 2) return "%r14w";
        if (Size() == 4) return "%r14d"; if (Size() == 8) return "%r14";
    case RegTag::r15:
        if (Size() == 1) return "%r15b"; if (Size() == 2) return "%r15w";
        if (Size() == 4) return "%r15d"; if (Size() == 8) return "%r15";
    case RegTag::xmm0:  return "%xmm0";  case RegTag::xmm1:  return "%xmm1";
    case RegTag::xmm2:  return "%xmm2";  case RegTag::xmm3:  return "%xmm3";
    case RegTag::xmm4:  return "%xmm4";  case RegTag::xmm5:  return "%xmm5";
    case RegTag::xmm6:  return "%xmm6";  case RegTag::xmm7:  return "%xmm7";
    case RegTag::xmm8:  return "%xmm8";  case RegTag::xmm9:  return "%xmm9";
    case RegTag::xmm10: return "%xmm10"; case RegTag::xmm11: return "%xmm11";
    case RegTag::xmm12: return "%xmm12"; case RegTag::xmm13: return "%xmm13";
    case RegTag::xmm14: return "%xmm14"; case RegTag::xmm15: return "%xmm15";
    }
}

#pragma GCC diagnostic pop


std::string x64Mem::ToString() const
{
    if (!label_.empty())
        return label_ + "(%rip)";

    std::string loc = "";

    if (offset_ != 0)
        loc += std::to_string(offset_);
    if (base_ == RegTag::none && index_ == RegTag::none)
        return loc;

    loc += '(';

    if (base_ != RegTag::none)
        loc += base_.ToString();
    if (index_ != RegTag::none)
        loc += ", " + index_.ToString();
    if (scale_ != 0)
        loc += ", " + std::to_string(scale_);

    loc += ')';
    return loc;
}

bool x64Mem::operator==(const x64Mem& mem) const
{
    if (!label_.empty())
        return label_ == mem.label_;
    return base_ == mem.base_ &&
        index_ == mem.index_ &&
        scale_ == mem.scale_;
}


x64Imm::x64Imm(const Constant* c) :
    x64(x64Id::imm, c->Type()->Size()), val_(c) {}


unsigned long x64Imm::GetRepr() const
{
    if (val_->Is<IntConst>())
        return val_->As<IntConst>()->Val();

    double val = val_->As<FloatConst>()->Val();
    unsigned long repr = 0;
    std::memcpy(reinterpret_cast<char*>(&repr),
        reinterpret_cast<char*>(&val), sizeof(unsigned long));

    return repr;
}

std::string x64Imm::ToString() const
{
    return '$' + std::to_string(GetRepr());
}


bool x64Imm::operator==(const x64Imm& imm) const
{
    if ((val_->Is<IntConst>() && imm.val_->Is<FloatConst>()) ||
        (val_->Is<FloatConst>() && imm.val_->Is<IntConst>()))
        return false;
    if (val_->Is<IntConst>() && imm.val_->Is<IntConst>())
        return val_->As<IntConst>()->Val() == imm.val_->As<IntConst>()->Val();
    if (val_->Is<FloatConst>() && imm.val_->Is<FloatConst>())
        return val_->As<FloatConst>()->Val() == imm.val_->As<FloatConst>()->Val();
    return false;
}
