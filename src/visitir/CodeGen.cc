#include "visitir/CodeGen.h"
#include "visitir/SysVConv.h"
#include "visitir/x64.h"
#include "IR/Value.h"
#include <climits>
#include <memory>
#include <string>


static std::string Cond2Str(Condition cond, bool issigned)
{
    std::string str = "";
    switch (cond)
    {
    case Condition::eq: str = "e"; break;
    case Condition::ne: str = "ne"; break;
    case Condition::gt: str = issigned ? "g" : "a"; break;
    case Condition::le: str = issigned ? "le" : "be"; break;
    case Condition::ge: str = issigned ? "ge" : "ae"; break;
    case Condition::lt: str = issigned ? "l" : "b"; break;
    }

    return str;
}

static Condition NotCond(Condition cond)
{
    switch (cond)
    {
    case Condition::eq: return Condition::ne;
    case Condition::ne: return Condition::eq;
    case Condition::gt: return Condition::le;
    case Condition::le: return Condition::gt;
    case Condition::ge: return Condition::lt;
    case Condition::lt: return Condition::ge;
    }
    return Condition::ne;
}

static RegTag X64Phys2RegTag(x64Phys phys)
{
    return static_cast<RegTag>(static_cast<int>(phys) + 2);
}

static x64Phys RegTag2X64Phys(RegTag tag)
{
    return static_cast<x64Phys>(static_cast<int>(tag) - 2);
}


std::string CodeGen::GetFpLabel(unsigned long repr, size_t size)
{
    return GetFpLabel(std::make_pair(repr, 0), size);
}

std::string CodeGen::GetFpLabel(std::pair<unsigned long, unsigned long> pair, size_t size)
{
    FpRepr repr{ pair.first, pair.second, size };
    if (auto res = fpconst_.find(repr); res != fpconst_.end())
        return res->second;
    auto label = ".fp" + std::to_string(labelindex_++);
    fpconst_[repr] = label;
    return label;
}

std::string CodeGen::GetLabel() const
{
    return ".L" + std::to_string(labelindex_++);
}

std::string CodeGen::GetLabel(const BasicBlock* bb) const
{
    if (auto label = bb2label_.find(bb); label != bb2label_.end())
        return label->second;
    bb2label_[bb] = GetLabel();
    return bb2label_[bb];
}


const x64* CodeGen::MapPossibleFloat(const IROperand* op)
{
    auto fp = op->As<FloatConst>();
    if (!fp)
        return alloc_.GetIROpMap(op);

    if (auto res = tempmap_.find(op); res != tempmap_.end())
        return res->second.get();

    auto imm = alloc_.GetIROpMap(op)->As<x64Imm>();
    tempmap_[op] = std::make_unique<x64Mem>(
        op->Type()->Size(), GetFpLabel(imm->GetRepr(), imm->Size()));
    return tempmap_[op].get();
}

const x64* CodeGen::MapPossibleRegister(const IROperand* op)
{
    if (!op->Type()->Is<PtrType>())
        return alloc_.GetIROpMap(op);

    auto mappedop = alloc_.GetIROpMap(op);
    if (mappedop->Is<x64Reg>())
    {
        // Address in a register. Convert it to a x64Mem.
        auto reg = mappedop->As<x64Reg>();
        tempmap_[op] = std::make_unique<x64Mem>(8, 0, *reg, RegTag::none, 0);
        return tempmap_[op].get();
    }
    else
        return mappedop;
}

const x64* CodeGen::LoadPointer(const Register* reg)
{
    auto mapped = alloc_.GetIROpMap(reg);
    if (auto m = mapped->As<x64Reg>(); m)
    {
        tempmap_[reg] = std::make_unique<x64Mem>(8, 0, *m, RegTag::none, 0);
        return tempmap_[reg].get();
    }

    auto mem = mapped->As<x64Mem>();
    if (!mem->LoadTwice())
        return mem;

    // use the second spare GP register here to avoid
    // possible conflict with the helper functions
    x64Reg temp{ GetSpareIntReg(1) };
    asmfile_.EmitMov(mem, &temp);
    tempmap_[reg] = std::make_unique<x64Mem>(8, 0, temp, RegTag::none, 0);
    return tempmap_[reg].get();
}


static inline size_t GetAlign(size_t num, size_t align)
{
    if (num % align == 0)
        return 0;
    return align - num % align;
}

void CodeGen::AlignRspBy(size_t add, size_t align)
{
    AdjustRsp(-GetAlign(stacksize_ + add, align));
}

void CodeGen::AdjustRsp(long offset)
{
    x64Reg rsp{ RegTag::rsp };
    if (offset < 0)
        asmfile_.EmitBinary("sub", -offset, &rsp);
    else if (offset > 0)
        asmfile_.EmitBinary("add", offset, &rsp);
    stacksize_ += (-offset);
}

void CodeGen::DeallocFrame()
{
    // Since rbp takes 8 bytes on the stack,
    // and it will be restored by leave.
    // stacksize_ itself will never go below to 0,
    // though, since it is initalized by 8.
    AdjustRsp(stacksize_ - 8);
}


void CodeGen::MapPtrParam2Mem(const x64* ptr)
{
    auto mem = ptr->As<x64Mem>();
    if (!mem)
    {
        PushEmitHelper(ptr);
        return;
    }

    x64Reg reg{ GetSpareIntReg(0) };
    if (mem->LoadTwice())
        asmfile_.EmitMov(ptr, &reg);
    else
        asmfile_.EmitLeaq(ptr, &reg);
    PushEmitHelper(&reg);
}

void CodeGen::MapPtrParam2Reg(const x64* ptr, const x64Reg* loc)
{
    auto mem = ptr->As<x64Mem>();
    if (mem && mem->LoadTwice())
        asmfile_.EmitMov(mem, loc);
    else if (auto r = loc->As<x64Reg>(); mem && !(mem->Base() == r->Tag() &&
        mem->Offset() == 0 && mem->Index() == RegTag::none))
        asmfile_.EmitLeaq(ptr, loc);
    else if (/* !mem && */ *ptr != loc->As<x64Reg>()->Tag())
        asmfile_.EmitMov(ptr, loc);
}

void CodeGen::MapFltParam2Mem(const x64* flt)
{
    AdjustRsp(-8);
    x64Mem rsp{ flt->Size(), 0, RegTag::rsp, RegTag::none, 0 };
    VecMovEmitHelper(flt, &rsp);
}

void CodeGen::MapFltParam2Reg(const x64* flt, const x64Reg* loc)
{
    VecMovEmitHelper(flt, loc);
}

void CodeGen::MapOtherParam2Mem(const x64* param)
{
    if (param->Is<x64Mem>())
    {
        asmfile_.EmitMov(param, RegTag::rax);
        PushEmitHelper(RegTag::rax, 8);
    }
    else
        PushEmitHelper(param);
}

void CodeGen::MapOtherParam2Reg(const x64* param, const x64Reg* loc)
{
    auto reg = loc->As<x64Reg>();
    if (auto p = param->As<x64Reg>(); p && *p == reg->Tag())
        return;
    asmfile_.EmitMov(param, loc);
}

// PassParam must correctly adjust the value of rsp, as the VisitCallInstr method
// will add the size of the parameters and padding by rsp directly.
void CodeGen::PassParam(
    const SysVConv& conv, const std::vector<const IROperand*>& param)
{
    for (int i = param.size() - 1; i >= 0; --i)
    {
        auto load2reg = conv.PlaceOfArgv(i);
        auto mapped = MapPossibleFloat(param[i]);
        if (!load2reg && param[i]->Type()->Is<PtrType>())
            MapPtrParam2Mem(mapped);
        else if (!load2reg && param[i]->Type()->Is<FloatType>())
            MapFltParam2Mem(mapped);
        else if (!load2reg)
            MapOtherParam2Mem(mapped);
        else if (load2reg && param[i]->Type()->Is<PtrType>())
            MapPtrParam2Reg(mapped, load2reg->As<x64Reg>());
        else if (load2reg && param[i]->Type()->Is<FloatType>())
            MapFltParam2Reg(mapped, load2reg->As<x64Reg>());
        else if (load2reg)
            MapOtherParam2Reg(mapped, load2reg->As<x64Reg>());
    }
}


static bool IsVecReg(x64Phys phys)
{
    return static_cast<int>(phys) > 15;
}

void CodeGen::SaveCalleeSaved()
{
    auto regs = alloc_.UsedCalleeSaved();
    for (auto reg : regs)
    {
        if (IsVecReg(reg))
            PushXmmReg(X64Phys2RegTag(reg));
        else
            PushEmitHelper(X64Phys2RegTag(reg), 8);
    }
}

void CodeGen::RestoreCalleeSaved()
{
    auto regs = alloc_.UsedCalleeSaved();
    for (auto i = regs.rbegin(); i != regs.rend(); ++i)
    {
        if (IsVecReg(*i))
            PopXmmReg(X64Phys2RegTag(*i));
        else
            PopEmitHelper(X64Phys2RegTag(*i), 8);
    }
}

void CodeGen::SaveCallerSaved()
{
    auto regs = alloc_.UsedCallerSaved();
    for (auto reg: regs)
    {
        if (IsVecReg(reg))
            PushXmmReg(X64Phys2RegTag(reg));
        else
            PushEmitHelper(X64Phys2RegTag(reg), 8);
    }
}

void CodeGen::RestoreCallerSaved()
{
    auto regs = alloc_.UsedCallerSaved();
    for (auto i = regs.rbegin(); i != regs.rend(); ++i)
    {
        if (IsVecReg(*i))
            PopXmmReg(X64Phys2RegTag(*i));
        else
            PopEmitHelper(X64Phys2RegTag(*i), 8);
    }
}


RegTag CodeGen::GetSpareIntReg(int i) const
{
    auto notused = alloc_.NotUsedIntReg();
    if (notused.empty() && i == 0)
        return RegTag::rax;
    else if (notused.empty() && i == 1)
        return RegTag::r11;
    else if (i == 0)
        return X64Phys2RegTag(*notused.begin());
    else
        return X64Phys2RegTag(*std::next(notused.begin(), i));
}

RegTag CodeGen::GetSpareVecReg(int i) const
{
    auto notused = alloc_.NotUsedVecReg();
    if (notused.empty())
        return RegTag::xmm0;
    else if (i == 0)
        return X64Phys2RegTag(*notused.begin());
    else
        return X64Phys2RegTag(*std::next(notused.begin(), i));
}


void CodeGen::GetElePtrImmHelper(
    const x64Mem* ptr, const x64Imm* i, const x64* rlt, size_t scl)
{
    if (ptr->GlobalLoc())
    {
        LeaqEmitHelper(ptr, rlt);
        auto offset = scl * i->GetRepr().first;
        if (offset)
            asmfile_.EmitBinary("add", offset, rlt);
    }
    else if (ptr->LoadTwice())
    {
        x64Reg reg{ GetSpareIntReg(0) };
        x64Mem mem{ 8, 0, reg, i->GetRepr().first, scl };
        asmfile_.EmitMov(ptr, &reg);
        LeaqEmitHelper(&mem, rlt);
    }
    else
    {
        auto offset = i->GetRepr().first * scl;
        x64Mem mem{
            8, static_cast<long>(offset + ptr->Offset()),
            ptr->Base(), ptr->Index(), ptr->Scale()
        };
        LeaqEmitHelper(&mem, rlt);
    }
}

void CodeGen::GetElePtrRegHelper(
    const x64Mem* ptr, const x64Reg* i, const x64* rlt, size_t scl)
{
    if (!ptr->LoadTwice()) // eq to (ptr->GlobalLoc() || ptr->LoadOnce())
    {
        LeaqEmitHelper(ptr, rlt);
        x64Reg reg{ GetSpareIntReg(0) };
        asmfile_.EmitMov(i, &reg);
        asmfile_.EmitBinary("mul", scl, &reg);
        asmfile_.EmitBinary("add", &reg, rlt);
    }
    else
    {
        x64Reg reg{ GetSpareIntReg(0) };
        asmfile_.EmitMov(ptr, &reg);
        x64Mem mem{ 8, 0, reg, *i, scl };
        LeaqEmitHelper(&mem, rlt);
    }
}

void CodeGen::GetElePtrMemHelper(
    const x64Mem* ptr, const x64Mem* i, const x64* rlt, size_t scl)
{
    x64Reg reg{ GetSpareIntReg(0) };

    if (!ptr->LoadTwice()) // eq to (ptr->GlobalLoc() || ptr->LoadOnce())
        asmfile_.EmitLeaq(ptr, &reg);
    else
        asmfile_.EmitMov(ptr, &reg);

    x64Reg temp{ GetSpareIntReg(1) };
    asmfile_.EmitMov(i, &temp);
    x64Mem mem{ 8, 0, reg, temp, scl };
    LeaqEmitHelper(&mem, rlt);
}


void CodeGen::BinaryGenHelper(
    const std::string& name, const BinaryInstr* bi)
{
    auto lhs = alloc_.GetIROpMap(bi->Lhs());
    auto rhs = alloc_.GetIROpMap(bi->Rhs());
    auto ans = alloc_.GetIROpMap(bi->Result());

    if (*rhs == *ans)
    {
        auto tag = GetSpareIntReg(0);
        asmfile_.EmitMov(lhs, tag);
        x64Reg temp{ tag, rhs->Size() };
        asmfile_.EmitBinary(name, rhs, &temp);
        asmfile_.EmitMov(&temp, ans);
        return;
    }

    if (*lhs != *ans)
        MovEmitHelper(lhs, ans);

    if (ans->Is<x64Reg>() || rhs->Is<x64Reg>())
        asmfile_.EmitBinary(name, rhs, ans);
    else
    {
        auto tag = GetSpareIntReg(0);
        x64Reg temp{ tag, rhs->Size() };
        asmfile_.EmitMov(rhs, &temp);
        asmfile_.EmitBinary(name, &temp, ans);
    }
}

void CodeGen::VarithmGenHelper(
    const std::string& name, const BinaryInstr* bi)
{
    auto lhs = MapPossibleFloat(bi->Lhs());
    auto rhs = MapPossibleFloat(bi->Rhs());
    auto ans = alloc_.GetIROpMap(bi->Result());

    if (lhs->Is<x64Reg>() && ans->Is<x64Reg>())
        asmfile_.EmitVarithm(name, rhs, lhs, ans);
    else if (!ans->Is<x64Reg>())
    {
        x64Reg temp{ GetSpareVecReg(0), ans->Size() };
        asmfile_.EmitVarithm(name, rhs, lhs, &temp);
        asmfile_.EmitVmov(&temp, ans);
    }
    else // if (!lhs->Is<x64Reg>())
    {
        x64Reg temp{ GetSpareVecReg(0), ans->Size() };
        asmfile_.EmitVmov(lhs, &temp);
        asmfile_.EmitVarithm(name, rhs, &temp, ans);
    }
}

void CodeGen::ShiftGenHelper(const std::string& name, const BinaryInstr* bin)
{
    auto lhs = alloc_.GetIROpMap(bin->Lhs());
    auto rhs = alloc_.GetIROpMap(bin->Rhs());
    auto ans = alloc_.GetIROpMap(bin->Result());

    if (!(rhs->Is<x64Reg>() && *rhs == RegTag::rcx))
        asmfile_.EmitMov(rhs, RegTag::rcx);
    if (*lhs != *ans)
        MovEmitHelper(lhs, ans);

    x64Reg rcx{ RegTag::rcx, rhs->Size() };
    asmfile_.EmitBinary(name, &rcx, ans);
}

const x64* CodeGen::PrepareDivMod(const BinaryInstr* bin)
{
    auto lhs = alloc_.GetIROpMap(bin->Lhs());
    auto rhs = alloc_.GetIROpMap(bin->Rhs());
    auto ans = alloc_.GetIROpMap(bin->Result());
    bool sign =
        bin->Lhs()->Type()->As<IntType>()->IsSigned() ||
        bin->Rhs()->Type()->As<IntType>()->IsSigned();

    // the first op in DivInstr may not be in %*ax.
    // move it to %*ax if it is not.
    if (lhs->Size() < 4)
    {
        x64Reg rax{ RegTag::rax, 4 };
        if (sign)
            asmfile_.EmitMovs(lhs->Size(), 4, &rax);
        else
            asmfile_.EmitMovz(lhs->Size(), 4, &rax);
    }
    else if (!(*lhs == RegTag::rax))
        asmfile_.EmitMov(lhs, RegTag::rax);

    bool usetemp = false;
    x64Reg temp{ GetSpareIntReg(1), 4 };
    if (rhs->Size() < 4)
    {
        if (sign)
            asmfile_.EmitMovs(rhs->Size(), 4, &temp);
        else
            asmfile_.EmitMovz(rhs->Size(), 4, &temp);
        usetemp = true;
    }
    else if (rhs->Is<x64Imm>())
    {
        asmfile_.EmitMov(rhs, &temp);
        usetemp = true;
    }

    asmfile_.EmitCxtx(lhs->Size() < 4 ? 4 : lhs->Size());
    asmfile_.EmitUnary(sign ? "idiv" : "div", usetemp ? &temp : rhs);
    return ans;
}


void CodeGen::LeaqEmitHelper(const x64* addr, const x64* dest)
{
    auto mem = addr->As<x64Mem>();
    if (dest->Is<x64Reg>())
    {
        asmfile_.EmitLeaq(mem, dest);
        return;
    }

    RegTag reg = GetSpareIntReg(0);
    asmfile_.EmitLeaq(addr, reg);
    asmfile_.EmitMov(reg, dest);
}

void CodeGen::MovEmitHelper(const x64* from, const x64* to)
{
    if (auto imm = from->As<x64Imm>(); imm && to->Is<x64Mem>())
    {
        auto val = imm->GetRepr().first;
        if (val > UINT32_MAX)
            goto usereg;
    }
    if (from->Is<x64Reg>() || from->Is<x64Imm>() || to->Is<x64Reg>())
    {
        asmfile_.EmitMov(from, to);
        return;
    }

usereg:
    auto tag = GetSpareIntReg(0);
    asmfile_.EmitMov(from, tag);
    asmfile_.EmitMov(tag, to);
}


// the destination of movz or movs must be a register
#define MOVZ_MOVS_HELPER(name)              \
if (to->Is<x64Reg>())                       \
{                                           \
    asmfile_.name(from, to);                \
    return;                                 \
}                                           \
                                            \
x64Reg reg{ GetSpareIntReg(0), to->Size() };\
asmfile_.name(from, &reg);                  \
asmfile_.EmitMov(&reg, to)

void CodeGen::MovzEmitHelper(const x64* from, const x64* to) { MOVZ_MOVS_HELPER(EmitMovz); }
void CodeGen::MovsEmitHelper(const x64* from, const x64* to) { MOVZ_MOVS_HELPER(EmitMovs); }

#undef MOVZ_MOVS_HELPER

#define MOVZ_MOVS_HELPER(name)          \
if (op->Is<x64Reg>())                   \
{                                       \
    asmfile_.name(from, to, op);        \
    return;                             \
}                                       \
                                        \
x64Reg reg{ GetSpareIntReg(0), from };  \
asmfile_.EmitMov(op, &reg);             \
asmfile_.name(from, to, &reg);          \
asmfile_.EmitMov(&reg, op)

void CodeGen::MovzEmitHelper(size_t from, size_t to, const x64* op) { MOVZ_MOVS_HELPER(EmitMovz); }
void CodeGen::MovsEmitHelper(size_t from, size_t to, const x64* op) { MOVZ_MOVS_HELPER(EmitMovs); }

#undef MOVZ_MOVS_HELPER


void CodeGen::VecMovEmitHelper(const x64* src, const x64* dest)
{
    if (src->Is<x64Reg>() && dest->Is<x64Reg>())
        asmfile_.EmitVmovap(src->As<x64Reg>(), dest->As<x64Reg>());
    else if (!src->Is<x64Reg>() && dest->Is<x64Reg>() ||
              src->Is<x64Reg>() && !dest->Is<x64Reg>())
        asmfile_.EmitVmov(src, dest);
    else
    {
        auto tag = GetSpareVecReg(0);
        asmfile_.EmitVmov(src, tag);
        asmfile_.EmitVmov(tag, dest);
    }
}

void CodeGen::VecMovEmitHelper(const x64* src, RegTag dest)
{
    if (src->Is<x64Reg>())
        asmfile_.EmitVmovap(src->As<x64Reg>(), dest);
    else
        asmfile_.EmitVmov(src, dest);
}

void CodeGen::VecMovEmitHelper(RegTag src, const x64* dest)
{
    if (dest->Is<x64Reg>())
        asmfile_.EmitVmovap(src, dest->As<x64Reg>());
    else
        asmfile_.EmitVmov(src, dest);
}


void CodeGen::VcvtEmitHelper(const x64* op1, const x64* op2)
{
    if (op2->Is<x64Reg>())
    {
        asmfile_.EmitVcvt(op1, op2);
        return;
    }
    auto tag = GetSpareVecReg(0);
    asmfile_.EmitVcvt(op1, tag);
    asmfile_.EmitVmov(tag, op2);
}

void CodeGen::VcvtsiEmitHelper(const x64* op1, const x64* op2)
{
    if (op2->Is<x64Reg>())
    {
        asmfile_.EmitVcvtsi(op1, op2);
        return;
    }
    auto tag = GetSpareVecReg(0);
    asmfile_.EmitVcvtsi(op1, tag);
    asmfile_.EmitVmov(tag, op2);
}

void CodeGen::VcvttEmitHelper(const x64* op1, const x64* op2)
{
    if (op2->Is<x64Reg>())
    {
        asmfile_.EmitVcvtt(op1, op2);
        return;
    }
    auto tag = GetSpareVecReg(0);
    asmfile_.EmitVcvtt(op1, tag);
    asmfile_.EmitVmov(tag, op2);
}

void CodeGen::UcomEmitHelper(const x64* op1, const x64* op2)
{
    if (op2->Is<x64Reg>())
    {
        asmfile_.EmitUcom(op1, op2);
        return;
    }
    auto tag = GetSpareVecReg(0);
    asmfile_.EmitVmov(op2, tag);
    asmfile_.EmitUcom(op1, tag);
}


void CodeGen::PushEmitHelper(const x64* reg)
{
    auto original = 0;

    if (reg->Size() != 8)
    {
        MovzEmitHelper(reg->Size(), 8, reg);
        original = reg->Size();
        const_cast<x64*>(reg)->Size() = 8;
    }
    stacksize_ += reg->Size();
    asmfile_.EmitPush(reg);

    if (original)
        const_cast<x64*>(reg)->Size() = original;
}

void CodeGen::PushEmitHelper(RegTag tag, size_t size)
{
    if (size != 8)
    {
        x64Reg reg{ tag };
        asmfile_.EmitMovz(size, 8, &reg);
        size = 8;
    }
    asmfile_.EmitPush(tag, size);
    stacksize_ += size;
}

void CodeGen::PopEmitHelper(const x64* reg)
{
    size_t original = 0;
    if (reg->Size() != 8)
    {
        original = reg->Size();
        const_cast<x64*>(reg)->Size() = 8;
    }
    asmfile_.EmitPop(reg);
    stacksize_ -= reg->Size();
    if (original)
        const_cast<x64*>(reg)->Size() = original;
}

void CodeGen::PopEmitHelper(RegTag tag, size_t size)
{
    if (size != 8)
        size = 8;
    asmfile_.EmitPop(tag, size);
    stacksize_ -= size;
}

void CodeGen::PushXmmReg(RegTag tag)
{
    AdjustRsp(-16);
    x64Mem rsp{ 16, 0, RegTag::rsp, RegTag::none, 0 };
    asmfile_.EmitVmov(tag, &rsp);
}

void CodeGen::PopXmmReg(RegTag tag)
{
    x64Mem rsp{ 16, 0, RegTag::rsp, RegTag::none, 0 };
    asmfile_.EmitVmov(&rsp, tag);
    AdjustRsp(16);
}


void CodeGen::CMovEmitHelper(Condition cond, bool issigned, const x64* op1, const x64* op2)
{
    if (op2->Is<x64Reg>())
    {
        auto size = op1->Size() == 1 ? 2 : op1->Size();
        x64Reg reg{ RegTag::none };
        if (auto imm = op1->As<x64Imm>(); imm)
        {
            reg = x64Reg(GetSpareIntReg(0), op2->Size());
            op1 = &reg;
            asmfile_.EmitMov(imm, op1);
        }
        if (op1->Size() != size)
        {
            auto temp = op1->Size();
            const_cast<x64*>(op1)->Size() = size;
            const_cast<x64*>(op2)->Size() = size;
            size = temp;
        }
        asmfile_.EmitCMov(Cond2Str(cond, issigned), op1, op2);
        if (op1->Size() != size)
        {
            const_cast<x64*>(op1)->Size() = size;
            const_cast<x64*>(op2)->Size() = size;
        }
    }
    else
    {
        // We can't use cmov if op2 is a mem reference.
        auto label = GetLabel();
        asmfile_.EmitJmp(Cond2Str(NotCond(cond), issigned), label);
        MovEmitHelper(op1, op2);
        asmfile_.EmitLabel(label);        
    }
}

void CodeGen::CmpEmitHelper(const x64* op1, const x64* op2)
{
    x64Reg reg{ RegTag::none };
    if (auto imm = op1->As<x64Imm>(); imm && imm->GetRepr().first > UINT32_MAX)
    {
        reg = x64Reg(GetSpareIntReg(0), op2->Size());
        op1 = &reg;
        asmfile_.EmitMov(imm, op1);
    }

    if (!(op2->Is<x64Imm>() || (op1->Is<x64Mem>() && op2->Is<x64Mem>())))
    {
        asmfile_.EmitCmp(op1, op2);
        return;
    }
    auto tag = GetSpareIntReg(1);
    asmfile_.EmitMov(op2, tag);
    asmfile_.EmitCmp(op1, tag);
}

void CodeGen::SetEmitHelper(Condition cond, bool issigned, const x64* op)
{
    std::string scond = Cond2Str(cond, issigned);
    if (op->Is<x64Mem>())
    {
        asmfile_.EmitSet(scond, op);
        if (op->Size() != 1)
            MovzEmitHelper(1, op->Size(), op);
    }
    else if (op->Is<x64Reg>())
    {
        auto tag = op->As<x64Reg>()->Tag();
        asmfile_.EmitSet(scond, tag);
        if (op->Size() != 1)
            MovzEmitHelper(1, op->Size(), op);
    }
}

void CodeGen::TestEmitHelper(const x64* op1, const x64* op2)
{
    if (!(op1->Is<x64Mem>() && op2->Is<x64Mem>()))
    {
        x64Reg reg{ RegTag::none };
        if (auto imm = op1->As<x64Imm>(); imm && imm->GetRepr().first > UINT32_MAX)
        {
            reg = GetSpareIntReg(0);
            op1 = &reg;
            asmfile_.EmitMov(imm, op1);
        }
        asmfile_.EmitTest(op1, op2);
        return;
    }
    auto tag = GetSpareIntReg(0);
    asmfile_.EmitMov(op1, tag);
    asmfile_.EmitTest(tag, op2);
}


void CodeGen::VisitModule(Module* mod)
{
    asmfile_.EmitPseudoInstr(".file", { "\"" + mod->Name() + "\"" });
    asmfile_.EmitBlankLine();
    for (auto val : *mod)
        val->Accept(this);

    if (fpconst_.empty())
        return;

    asmfile_.EmitPseudoInstr(".section .rodata");
    for (const auto& [repr, label] : fpconst_)
    {
        asmfile_.EmitPseudoInstr(".align", { std::to_string(repr.size_) });
        asmfile_.EmitLabel(label);
        if (repr.size_ == 4)
            asmfile_.EmitPseudoInstr(".long", { std::to_string(repr.first_) });
        else if (repr.size_ == 8)
            asmfile_.EmitPseudoInstr(".quad", { std::to_string(repr.first_) });
        else if (repr.size_ == 16)
        {
            asmfile_.EmitPseudoInstr(".quad", { std::to_string(repr.second_) });
            asmfile_.EmitPseudoInstr(".quad", { std::to_string(repr.first_) });
        }
        asmfile_.EmitBlankLine();
    }
}

void CodeGen::VisitGlobalVar(GlobalVar* var)
{
    // stripping the leading '@'
    auto name = var->Name().substr(1);
    auto size = var->Type()->Size();

    var->GetTree()->Accept(this);
    auto repr = var->GetTree()->repr_;
    if (repr[0] == '"')
    {
        asmfile_.EmitPseudoInstr(".section .rodata");
        asmfile_.EmitLabel(name);
        asmfile_.EmitPseudoInstr(".string", { std::move(repr) });
    }
    else
    {
        auto strsz = std::to_string(size);
        asmfile_.EmitPseudoInstr(".data");
        asmfile_.EmitPseudoInstr(".globl", { name });
        asmfile_.EmitPseudoInstr(".align", { strsz });
        asmfile_.EmitPseudoInstr(".type", { name, "@object" });
        asmfile_.EmitPseudoInstr(".size", { name, strsz });
        asmfile_.EmitLabel(name);

        if (!var->GetTree())
            asmfile_.EmitPseudoInstr(".zero", { strsz });
        else
        {
            std::string pseudo = "";
            switch (size)
            {
            case 1: pseudo = ".byte"; break;
            case 2: pseudo = ".value"; break;
            case 4: pseudo = ".long"; break;
            case 8: pseudo = ".quad"; break;
            }
            asmfile_.EmitPseudoInstr(pseudo, { var->GetTree()->repr_ });
        }
    }
    asmfile_.EmitBlankLine();
}

void CodeGen::VisitFunction(Function* func)
{
    if (func->Empty())
        return;

    alloc_.EnterFunction(func);
    auto name = func->Name().substr(1);

    asmfile_.EmitPseudoInstr(".text");
    asmfile_.EmitPseudoInstr(".globl", { name });
    asmfile_.EmitPseudoInstr(".type", { name, "@function" });
    asmfile_.EmitLabel(func->Name().substr(1));

    stacksize_ = 8;
    asmfile_.EmitPush(RegTag::rbp, 8);
    x64Reg rsp{ RegTag::rsp };
    x64Reg rbp{ RegTag::rbp };
    asmfile_.EmitMov(&rsp, &rbp);
    AdjustRsp(-alloc_.RspOffset());

    if (func->Variadic())
    {
        AlignRspBy(0, 8);
        auto baseline = stacksize_;
        AdjustRsp(-176);
        asmfile_.EmitMov(RegTag::rdi, baseline);
        asmfile_.EmitMov(RegTag::rsi, baseline + 8);
        asmfile_.EmitMov(RegTag::rdx, baseline + 16);
        asmfile_.EmitMov(RegTag::rcx, baseline + 24);
        asmfile_.EmitMov(RegTag::r8, baseline + 32);
        asmfile_.EmitMov(RegTag::r9, baseline + 40);

        auto rax = x64Reg(RegTag::rax, 1);
        auto label = GetLabel();
        asmfile_.EmitTest(&rax, &rax);
        asmfile_.EmitJmp("e", label);

        asmfile_.EmitVmov(RegTag::xmm0, baseline + 48);
        asmfile_.EmitVmov(RegTag::xmm1, baseline + 64);
        asmfile_.EmitVmov(RegTag::xmm2, baseline + 80);
        asmfile_.EmitVmov(RegTag::xmm3, baseline + 96);
        asmfile_.EmitVmov(RegTag::xmm4, baseline + 112);
        asmfile_.EmitVmov(RegTag::xmm5, baseline + 128);
        asmfile_.EmitVmov(RegTag::xmm6, baseline + 144);
        asmfile_.EmitVmov(RegTag::xmm7, baseline + 160);

        asmfile_.EmitLabel(label);
    }

    SaveCalleeSaved();
    asmfile_.Write2Mem();

    for (auto bb : *func)
        VisitBasicBlock(bb);

    asmfile_.Dump2File();
    asmfile_.EmitBlankLine();
}

void CodeGen::VisitBasicBlock(BasicBlock* bb)
{
    asmfile_.EnterBlock(bb);
    asmfile_.EmitLabel(GetLabel(bb));
    for (auto inst : *bb)
        inst->Accept(this);
}


void CodeGen::VisitNode(OpNode* op)
{
    if (op->op_->Is<IntConst>())
        op->repr_ = std::to_string(op->op_->As<IntConst>()->Val());
    else if (op->op_->Is<FloatConst>())
    {
        if (op->op_->Type()->Size() == 4)
            op->repr_ = std::to_string(FloatRepr(static_cast<float>(op->op_->As<FloatConst>()->Val())).first);
        else if (op->op_->Type()->Size() == 8)
            op->repr_ = std::to_string(FloatRepr(op->op_->As<FloatConst>()->Val()).first);
        else if (op->op_->Type()->Size() == 16)
        {
            auto repr = FloatRepr(op->op_->As<FloatConst>()->Val());
            // werid and seems like remedy but does work
            op->repr_ = std::to_string(repr.first) +
                "\n    .long " + std::to_string(repr.second);
        }
    }
    else if (op->op_->Is<StrConst>())
        op->repr_ = op->op_->As<StrConst>()->ToString();
    else // strip the leading '@'
        op->repr_ = op->op_->As<Register>()->Name().substr(1);
}

void CodeGen::VisitNode(BinaryNode* bin)
{
    bin->left_->Accept(this);
    bin->right_->Accept(this);

    if (bin->id_ == Instr::InstrId::add)
        bin->repr_ = std::move(bin->left_->repr_) + '+' + std::move(bin->right_->repr_);
    else if (bin->id_ == Instr::InstrId::sub)
        bin->repr_ = std::move(bin->left_->repr_) + '-' + std::move(bin->right_->repr_);
}

void CodeGen::VisitNode(UnaryNode* un)
{
    un->op_->Accept(this);
    if (un->id_ == Instr::InstrId::geteleptr)
        un->repr_ = std::move(un->op_->repr_);
}


void CodeGen::VisitRetInstr(RetInstr* inst)
{
    if (!inst->ReturnValue())
        goto ret;
    else if (inst->ReturnValue()->Type()->Is<IntType>())
    {
        auto mapped = alloc_.GetIROpMap(inst->ReturnValue());
        if (mapped->Is<x64Reg>() && *mapped->As<x64Reg>() == RegTag::rax)
            goto ret;
        asmfile_.EmitMov(mapped, RegTag::rax);
    }
    else if (inst->ReturnValue()->Type()->Is<FloatType>())
    {
        auto mapped = MapPossibleFloat(inst->ReturnValue());
        if (mapped->Is<x64Reg>() && *mapped->As<x64Reg>() == RegTag::xmm0)
            goto ret;
        VecMovEmitHelper(MapPossibleFloat(inst->ReturnValue()), RegTag::xmm0);
    }

ret:
    // what if there's multiple ret instruction in a function?
    auto oldstacksize = stacksize_;
    RestoreCalleeSaved();
    DeallocFrame();
    stacksize_ = oldstacksize;

    asmfile_.EmitLeave();
    asmfile_.EmitRet();
}


void CodeGen::VisitBrInstr(BrInstr* inst)
{
    if (inst->Cond())
    {
        auto cond = MapPossibleFloat(inst->Cond());
        asmfile_.EmitCmp((unsigned long)0, cond);
        asmfile_.EmitJmp("ne", GetLabel(inst->GetTrueBlk()));
        asmfile_.EmitJmp("", GetLabel(inst->GetFalseBlk()));
    }
    else asmfile_.EmitJmp("", GetLabel(inst->GetTrueBlk()));
}


void CodeGen::VisitSwitchInstr(SwitchInstr* inst)
{
    // I use a somewhat straight forward way to translate
    // the switch instruction. no jump table is used, though.

    auto ident = alloc_.GetIROpMap(inst->GetIdent());
    for (auto [tag, bb] : inst->GetValueBlkPairs())
    {
        CmpEmitHelper(ident, alloc_.GetIROpMap(tag));
        asmfile_.EmitJmp("z", GetLabel(bb));
    }
    asmfile_.EmitJmp("", GetLabel(inst->GetDefault()));
}


void CodeGen::VisitCallInstr(CallInstr* inst)
{
    // Layout of the stack:
    //         high address
    // +------------------------+
    // |       Local Vars       |
    // +------------------------+
    // |   Caller Saved Regs    |
    // +------------------------+
    // |        Padding         |
    // +------------------------+
    // |  Argvs Passed on Stack |
    // +------------------------+  <- rsp in caller
    // |     Frame of Callee    |
    // +------------------------+
    //         low address

    SaveCallerSaved();
    SysVConv conv{ inst->Proto() };
    conv.MapArgv();

    auto [_, vec] = conv.CountRegs();
    if (inst->Proto()->Variadic())
    {
        if (vec == 0)
            asmfile_.EmitInstr("    xorq %rax, %rax\n");
        else
            asmfile_.EmitInstr("    movb $" + std::to_string(vec) + ", %al\n");
    }

    auto extra = GetAlign(stacksize_, 16);
    AdjustRsp(-(conv.Padding() + extra));
    PassParam(conv, inst->ArgvList());
    if (inst->FuncAddr())
        asmfile_.EmitCall(alloc_.GetIROpMap(inst->FuncAddr()));
    else
        asmfile_.EmitCall(inst->FuncName().substr(1) + "@PLT");
    // adjust rsp directly here since conv.StackSize()
    // has already included 'padding'
    AdjustRsp(conv.StackSize() + extra);
    RestoreCallerSaved();

    if (inst->Result() && inst->Result()->Type()->Is<IntType>())
    {
        auto x64reg = alloc_.GetIROpMap(inst->Result());
        if (!x64reg->Is<x64Reg>() || x64reg->As<x64Reg>()->Tag() != RegTag::rax)
            asmfile_.EmitMov(RegTag::rax, x64reg);
    }
    else if (inst->Result() && inst->Result()->Type()->Is<FloatType>())
    {
        auto vecreg = alloc_.GetIROpMap(inst->Result());
        if (!vecreg->Is<x64Reg>() || vecreg->As<x64Reg>()->Tag() != RegTag::xmm0)
            VecMovEmitHelper(RegTag::xmm0, vecreg);
    }
}


void CodeGen::VisitAddInstr(AddInstr* inst)     { BinaryGenHelper("add", inst); }
void CodeGen::VisitFaddInstr(FaddInstr* inst)   { VarithmGenHelper("add", inst); }
void CodeGen::VisitSubInstr(SubInstr* inst)     { BinaryGenHelper("sub", inst); }
void CodeGen::VisitFsubInstr(FsubInstr* inst)   { VarithmGenHelper("sub", inst); }
void CodeGen::VisitFmulInstr(FmulInstr* inst)   { VarithmGenHelper("mul", inst); }
void CodeGen::VisitFdivInstr(FdivInstr* inst)   { VarithmGenHelper("div", inst); }
void CodeGen::VisitShlInstr(ShlInstr* inst)     { ShiftGenHelper("shl", inst); }
void CodeGen::VisitLshrInstr(LshrInstr* inst)   { ShiftGenHelper("shr", inst); }
void CodeGen::VisitAshrInstr(AshrInstr* inst)   { ShiftGenHelper("sar", inst); }
void CodeGen::VisitAndInstr(AndInstr* inst)     { BinaryGenHelper("and", inst); }
void CodeGen::VisitOrInstr(OrInstr* inst)       { BinaryGenHelper("or", inst); }
void CodeGen::VisitXorInstr(XorInstr* inst)     { BinaryGenHelper("xor", inst); }

void CodeGen::VisitMulInstr(MulInstr* inst)
{
    bool issigned =
        inst->Lhs()->Type()->As<IntType>()->IsSigned() ||
        inst->Rhs()->Type()->As<IntType>()->IsSigned();
    BinaryGenHelper(issigned ? "imul" : "mul", inst);
}

void CodeGen::VisitDivInstr(DivInstr* inst)
{
    auto ans = PrepareDivMod(inst);
    // if result register is not %*ax, move it to the result register.
    if (!(*ans == RegTag::rax))
        asmfile_.EmitMov(RegTag::rax, ans);
}

void CodeGen::VisitModInstr(ModInstr* inst)
{
    auto ans = PrepareDivMod(inst);
    // if result register is not %*dx, move it to the result register.
    if (!(*alloc_.GetIROpMap(inst->Result()) == RegTag::rdx))
        asmfile_.EmitMov(RegTag::rdx, ans);
}


void CodeGen::VisitStoreInstr(StoreInstr* inst)
{
    auto dest = inst->Dest();
    auto value = inst->Value();
    auto mappeddest = LoadPointer(dest);

    if (value->Type()->Is<FloatType>())
    {
        VecMovEmitHelper(MapPossibleFloat(value), mappeddest);
        return;
    }

    auto mappedval = alloc_.GetIROpMap(value);
    if (value->Type()->Is<PtrType>() &&
        mappedval->Is<x64Mem>() && !mappedval->As<x64Mem>()->LoadTwice())
        LeaqEmitHelper(mappedval, mappeddest);
    else
        MovEmitHelper(mappedval, mappeddest);
}

void CodeGen::VisitLoadInstr(LoadInstr* inst)
{
    auto result = inst->Result();
    auto mappedptr = LoadPointer(inst->Pointer());

    if (result->Type()->Is<FloatType>())
        VecMovEmitHelper(mappedptr, MapPossibleFloat(result));
    else
        MovEmitHelper(mappedptr, alloc_.GetIROpMap(result));
}

void CodeGen::VisitGetElePtrInstr(GetElePtrInstr* inst)
{
    // inst->Result() must be a pointer.
    auto size = inst->Result()->Type()->As<PtrType>()->Point2()->Size();
    auto pointer = MapPossibleRegister(inst->Pointer())->As<x64Mem>();
    auto index = alloc_.GetIROpMap(inst->OpIndex());
    auto result = alloc_.GetIROpMap(inst->Result());

    if (index->Is<x64Imm>())
        GetElePtrImmHelper(pointer, index->As<x64Imm>(), result, size);
    else if (index->Is<x64Reg>())
        GetElePtrRegHelper(pointer, index->As<x64Reg>(), result, size);
    else if (index->Is<x64Mem>())
        GetElePtrMemHelper(pointer, index->As<x64Mem>(), result, size);
}


void CodeGen::VisitTruncInstr(TruncInstr* inst)
{
    auto from = alloc_.GetIROpMap(inst->Value());
    auto to = alloc_.GetIROpMap(inst->Dest());

    // Cannot use MovEmitHelper here, since the helping
    // method supposes its two operands have the same size.
    x64Reg temp{ RegTag::none };
    if (to->Is<x64Reg>())
        temp = x64Reg(to->As<x64Reg>()->Tag(), from->Size());
    else // if (to->Is<x64Mem>())
        temp = x64Reg(GetSpareIntReg(0), from->Size());

    asmfile_.EmitMov(from, &temp);
    if (to->Size() == 4)
    {
        temp.Size() = 4;
        asmfile_.EmitMov(&temp, &temp);
    }
    else
    {
        temp.Size() = 8;
        auto mask = ~(0x8000000000000000 >> (64 - to->Size() * 8));
        asmfile_.EmitBinary("and", mask, &temp);
    }

    if (to->Is<x64Mem>())
    {
        temp.Size() = to->Size();
        asmfile_.EmitMov(&temp, to);
    }
}

void CodeGen::VisitFtruncInstr(FtruncInstr* inst)
{
    auto from = MapPossibleFloat(inst->Value());
    auto to = alloc_.GetIROpMap(inst->Dest());
    VcvtEmitHelper(from, to);
}

void CodeGen::VisitZextInstr(ZextInstr* inst)
{
    auto from = alloc_.GetIROpMap(inst->Value());
    auto to = alloc_.GetIROpMap(inst->Dest());
    MovzEmitHelper(from, to);
}

void CodeGen::VisitSextInstr(SextInstr* inst)
{
    auto from = alloc_.GetIROpMap(inst->Value());
    auto to = alloc_.GetIROpMap(inst->Dest());
    MovsEmitHelper(from, to);
}

void CodeGen::VisitFextInstr(FextInstr* inst)
{
    auto from = MapPossibleFloat(inst->Value());
    auto to = alloc_.GetIROpMap(inst->Dest());
    VcvtEmitHelper(from, to);
}

void CodeGen::VisitFtoUInstr(FtoUInstr* inst)
{
    auto from = MapPossibleFloat(inst->Value());
    auto to = alloc_.GetIROpMap(inst->Dest());
    VcvttEmitHelper(from, to);
}

void CodeGen::VisitFtoSInstr(FtoSInstr* inst)
{
    auto from = MapPossibleFloat(inst->Value());
    auto to = alloc_.GetIROpMap(inst->Dest());
    VcvttEmitHelper(from, to);
}

void CodeGen::VisitUtoFInstr(UtoFInstr* inst)
{
    auto from = alloc_.GetIROpMap(inst->Value());
    auto to = alloc_.GetIROpMap(inst->Dest());
    if (from->Size() != 8)
    {
        MovzEmitHelper(from->Size(), 8, from);
        VcvtsiEmitHelper(from, to);
        return;
    }

    auto movebiguint = GetLabel();
    auto end = GetLabel();
    TestEmitHelper(from, from);
    asmfile_.EmitJmp("s", movebiguint);
    // For uint less than 64 bit,
    // just use a single vcvtsixx2xx instruction
    VcvtsiEmitHelper(from, to);
    asmfile_.EmitJmp("", end);
    // The case that the uint occupies all the
    // 64 bits of the GP register.
    // The assemble snippet below equivalent to
    // floor(from / 2) + has-reminder ? 1 : 0.
    // Note that this implementation is somewhat awkward,
    // since we can't overwrite the value in 'from'.
    asmfile_.EmitLabel(movebiguint);
    x64Reg temp1{ GetSpareIntReg(0) };
    x64Reg temp2{ GetSpareIntReg(1) };
    asmfile_.EmitMov(from, &temp1);
    asmfile_.EmitUnary("shr", &temp1);
    asmfile_.EmitMov(from, &temp2);
    asmfile_.EmitBinary("and", 1, &temp2);
    asmfile_.EmitBinary("add", &temp2, &temp1);
    VcvtsiEmitHelper(&temp1, to);
    asmfile_.EmitLabel(end);
}

void CodeGen::VisitStoFInstr(StoFInstr* inst)
{
    auto from = alloc_.GetIROpMap(inst->Value());
    auto to = alloc_.GetIROpMap(inst->Dest());
    VcvtsiEmitHelper(from, to);
}


void CodeGen::VisitPtrtoIInstr(PtrtoIInstr* inst)
{
    auto from = alloc_.GetIROpMap(inst->Value());
    auto to = alloc_.GetIROpMap(inst->Dest());

    auto mem = from->As<x64Mem>();
    if ((mem && mem->LoadTwice()) || !mem)
        MovEmitHelper(from, to);
    else // if (mem && mem->LoadOnce())
        LeaqEmitHelper(from, to);
}

void CodeGen::VisitItoPtrInstr(ItoPtrInstr* inst)
{
    auto from = alloc_.GetIROpMap(inst->Value());
    auto to = alloc_.GetIROpMap(inst->Dest());
    // LoadTwice has been set in the register allocator,
    // and the destination of itoptr must be a load-twice
    // pointer if it is mapped onto the stack. So its value
    // can be set by a single mov instruction.
    MovEmitHelper(from, to);
}


void CodeGen::VisitIcmpInstr(IcmpInstr* inst)
{
    auto lhs = alloc_.GetIROpMap(inst->Op1());
    auto rhs = alloc_.GetIROpMap(inst->Op2());
    auto ans = alloc_.GetIROpMap(inst->Result());
    auto issigned = inst->Op1()->Type()->As<IntType>()->IsSigned() ||
        inst->Op2()->Type()->As<IntType>()->IsSigned();

    CmpEmitHelper(rhs, lhs);
    SetEmitHelper(inst->Cond(), issigned, ans);
    if (ans->Size() != 1)
        MovzEmitHelper(1, ans->Size(), ans);
}


void CodeGen::VisitFcmpInstr(FcmpInstr* inst)
{
    auto lhs = MapPossibleFloat(inst->Op1());
    auto rhs = MapPossibleFloat(inst->Op2());
    auto ans = alloc_.GetIROpMap(inst->Result());

    UcomEmitHelper(rhs, lhs);
    asmfile_.EmitSet(Cond2Str(inst->Cond(), true), ans);
    if (ans->Size() != 1)
        MovzEmitHelper(1, ans->Size(), ans);
}


void CodeGen::VisitSelectInstr(SelectInstr* inst)
{
    auto [cond, ty] = inst->CondPair();
    auto mappedcond = MapPossibleFloat(cond);
    auto v1 = MapPossibleFloat(inst->Value1());
    auto v2 = MapPossibleFloat(inst->Value2());
    auto ans = alloc_.GetIROpMap(inst->Result());

    if (cond->Type()->Is<IntType>())
        asmfile_.EmitTest(mappedcond, mappedcond);
    else
    {
        x64Mem zero{ mappedcond->Size(), GetFpLabel(0, mappedcond->Size()) };
        asmfile_.EmitUcom(&zero, mappedcond);
    }

    if (auto t = inst->Result()->Type()->As<IntType>(); t)
    {
        asmfile_.EmitMov(v1, ans);
        CMovEmitHelper(ty ? Condition::eq : Condition::ne, t->IsSigned(), v2, ans);
    }
    else
    {
        // simply use branched code here
        // things like bitwise operations works fine as
        // well, but they require far more complex code,
        // so I don't use them here
        auto temp = GetLabel();
        VecMovEmitHelper(v1, ans);
        asmfile_.EmitJmp(ty ? "e" : "ne", temp);
        VecMovEmitHelper(v2, ans);
        asmfile_.EmitLabel(temp);
    }
}


void CodeGen::VisitPhiInstr(PhiInstr* inst)
{
    // FIXME: the implementation can't correctly solve the
    // lost-copies and phi-swap problems. With the limited
    // use of phi instructions in the code generated by the
    // front end, however, it's okay to ignore them before
    // mem2reg pass is actually implemented.

    auto curbb = asmfile_.CurBlock();
    auto mappedres = alloc_.GetIROpMap(inst->Result());

    for (auto [bb, op] : inst->GetBlockValPair())
    {
        asmfile_.SwitchBlock(bb);

        if (op->Type()->Is<IntType>())
            MovEmitHelper(alloc_.GetIROpMap(op), mappedres);
        else if (op->Type()->Is<FloatType>())
            VecMovEmitHelper(MapPossibleFloat(op), mappedres);
    }

    asmfile_.SwitchBlock(curbb);
}
