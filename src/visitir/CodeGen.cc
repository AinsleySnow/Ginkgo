#include "visitir/CodeGen.h"
#include "visitir/SysVConv.h"
#include "visitir/x64.h"
#include "IR/Value.h"
#include "pass/x64Alloc.h"
#include <climits>
#include <fmt/format.h>
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

static bool IsVecReg(x64Phys phys)
{
    return static_cast<int>(phys) > 15;
}

static bool IsVecReg(RegTag tag)
{
    return static_cast<int>(tag) > 17;
}


std::string CodeGen::GetFpLabel(unsigned long repr, size_t size)
{
    return GetFpLabel(std::make_pair(repr, 0), size);
}

std::string CodeGen::GetFpLabel(
    std::pair<unsigned long, unsigned long> pair, size_t size)
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
        return alloc_->GetIROpMap(op);

    if (auto res = tempmap_.find(op); res != tempmap_.end())
        return res->second.get();

    auto imm = alloc_->GetIROpMap(op)->As<x64Imm>();
    tempmap_[op] = std::make_unique<x64Mem>(
        op->Type()->Size(), GetFpLabel(imm->GetRepr(), imm->Size()));
    return tempmap_[op].get();
}

const x64* CodeGen::MapPossibleRegister(const IROperand* op)
{
    if (!op->Type()->Is<PtrType>())
        return alloc_->GetIROpMap(op);

    auto mappedop = alloc_->GetIROpMap(op);
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
    auto mapped = alloc_->GetIROpMap(reg);
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


void CodeGen::MapHeterParam(const x64Mem* mem, const x64Heter* heter)
{
    auto oldoff = mem->Offset();
    auto chunks = mem->Size() / 8;
    if (chunks)
        const_cast<x64Mem*>(mem)->Offset() += (chunks - 1) * 8;

    auto& phigh = heter->Back();
    auto remain = mem->Size() - chunks * 8;
    if (remain == 0)
        remain = 8;

    if (phigh.InReg())
    {
        if (IsVecReg(phigh.ToReg()))
        {
            Copy8Bytes(mem, RegTag::rax, RegTag::r11, remain);
            x64Reg to{ phigh.ToReg() };
            asmfile_.EmitInstr(fmt::format(
                "    vmovq %r11, {}\n", to.ToString()));
        }
        else
            Copy8Bytes(mem, RegTag::rax, phigh.ToReg(), remain);
    }
    else
    {
        AdjustRsp(-8);
        x64Mem rsp{ 0, 0, RegTag::rsp, RegTag::none, 0 };
        Copy8Bytes(mem, RegTag::rax, &rsp, remain);
    }

    for (auto i = heter->rbegin() + 1; i != heter->rend(); ++i)
    {
        const_cast<x64Mem*>(mem)->Offset() -= 8;
        if (i->InReg())
        {
            x64Reg r{ i->ToReg() };
            if (IsVecReg(i->ToReg()))
                asmfile_.EmitInstr(fmt::format(
                    "    vmovq {}, {}\n", mem->ToString(), r.ToString()));
            else
                asmfile_.EmitMov(mem, &r);
        }
        else
        {
            asmfile_.EmitInstr(
                fmt::format("    pushq {}\n", mem->ToString()));
            stacksize_ += 8;
        }
    }
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
// will add rsp by the size of the parameters and padding directly.
void CodeGen::PassParam(
    const SysVConv& conv, int count, const std::vector<const IROperand*>& param)
{
    long prevoffset = 0;
    for (int i = param.size() - 1; i >= 0; --i)
    {
        auto load2reg = conv.PlaceOfArgv(i);
        if (prevoffset && !load2reg)
        {
            // +--------------------------+ <- rsp in caller before
            // |    80-byte big heter,    |    passing params
            // |       align = 16         |
            // +--------------------------+ <- prevoffset
            // |         Padding          | <- size of padding is calc based
            // +--------------------------+    on val of rbp in the callee,
            // |       8-byte chunk       |    but not rsp in the caller
            // +--------------------------+ <- OffsetOfArgv(i)
            // |       Return addr        |
            // +--------------------------+
            // |         Saved rbp        |    rbp in callee;
            // +--------------------------+ <- base of the offsets
            //
            // Sometimes heterogeneous parameters can have alignment
            // more than 8. Things become tricky when alignment exceeds
            // 16 and the case is now ignored for simplicity.
            // If the alignment is just 16, we may need to add padding
            // between the current and the next parameter. However, the
            // padding is calculated from the perspective of rbp in callee,
            // and we have to get it from the mapped offset.
            auto chunk = param[i]->Type()->Size() <= 8 ?
                8 : param[i]->Type()->Size();
            auto padding = prevoffset - (conv.OffsetOfArgv(i) + chunk);
            if (padding)
                AdjustRsp(-padding);
            prevoffset = 0;
        }

        if (param[i]->Type()->Is<HeterType>())
        {
            // i >= count: if the function is variadic,
            // pass struct/union on the stack
            if (auto size = param[i]->Type()->Size(); size > 64 || i >= count)
            {
                AdjustRsp(-size);
                asmfile_.EmitPush(RegTag::rcx, 8);
                asmfile_.EmitPush(RegTag::rsi, 8);
                asmfile_.EmitPush(RegTag::rdi, 8);

                asmfile_.EmitInstr("    leaq 24(%rsp), %rdi\n");
                asmfile_.EmitLeaq(alloc_->GetIROpMap(param[i]), RegTag::rsi);
                asmfile_.EmitInstr(fmt::format("    movq ${}, %rcx\n", size));
                asmfile_.EmitInstr("    rep movsb\n");

                asmfile_.EmitPop(RegTag::rdi, 8);
                asmfile_.EmitPop(RegTag::rsi, 8);
                asmfile_.EmitPop(RegTag::rcx, 8);

                prevoffset = conv.OffsetOfArgv(i);
            }
            else
                MapHeterParam(alloc_->GetIROpMap(param[i])->As<x64Mem>(),
                    load2reg->As<x64Heter>());
            continue;
        }

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

        if (!load2reg)
            prevoffset = conv.OffsetOfArgv(i);
    }
}


void CodeGen::SaveCalleeSaved()
{
    auto regs = alloc_->UsedCalleeSaved();
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
    auto regs = alloc_->UsedCalleeSaved();
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
    auto regs = alloc_->UsedCallerSaved();
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
    auto regs = alloc_->UsedCallerSaved();
    for (auto i = regs.rbegin(); i != regs.rend(); ++i)
    {
        if (IsVecReg(*i))
            PopXmmReg(X64Phys2RegTag(*i));
        else
            PopEmitHelper(X64Phys2RegTag(*i), 8);
    }
}


void CodeGen::HandleVaStart(CallInstr* inst)
{
    auto& argvs = inst->ArgvList();
    auto addr = LoadPointer(argvs[0]->As<Register>())->As<x64Mem>();
    auto oldoff = addr->Offset();
    x64Reg rax{ RegTag::rax, 8 };

    if (argvs.size() == 1) // Only one argument?
    {
        // Supposed the function has prototype func(...).
        // An error should have reported by CodeChk if not so.
        // gp_offset = 0
        asmfile_.EmitInstr(fmt::format("    movl $0, {}\n", addr->ToString()));
        // fp_offset = 0
        const_cast<x64Mem*>(addr)->Offset() += 4;
        asmfile_.EmitInstr(fmt::format("    movl $0, {}\n", addr->ToString()));
        // overflow_arg_area = 16(%rbp)
        const_cast<x64Mem*>(addr)->Offset() += 4;
        asmfile_.EmitInstr(fmt::format("    leaq 16(%rbp), %rax\n"));
        asmfile_.EmitMov(&rax, addr, 0);
    }
    else
    {
        // gp_offset = count of used GP registers * 8
        asmfile_.EmitInstr(fmt::format(
            "    movl ${}, {}\n", alloc_->IntRegCount() * 8, addr->ToString()));
        // fp_offset = 48 + count of used vector registers * 8
        const_cast<x64Mem*>(addr)->Offset() += 4;
        asmfile_.EmitInstr(fmt::format(
            "    movl ${}, {}\n", 48 + alloc_->VecRegCount() * 8, addr->ToString()));
        // overflow_arg_area = address of the last address-known argument
        // plus its size; 16(%rbp) if it is passed in the register
        const_cast<x64Mem*>(addr)->Offset() += 4;
        auto last = alloc_->GetIROpMap(argvs[1]);
        if (last->Is<x64Reg>())
        {
            asmfile_.EmitInstr(fmt::format("    leaq 16(%rbp), %rax\n"));
            asmfile_.EmitMov(&rax, addr, 0);
        }
        else // if (last->Is<x64Mem>())
        {
            auto mem = last->As<x64Mem>();
            auto oldmem = mem->Offset();
            auto size = mem->Size();
            if (size < 8)
                size = 8;
            else if (size > 8)
                size += size % 8;
            const_cast<x64Mem*>(mem)->Offset() += size;
            asmfile_.EmitInstr(fmt::format("    leaq {}, %rax\n", mem->ToString()));
            asmfile_.EmitMov(&rax, addr, 0);
            const_cast<x64Mem*>(mem)->Offset() = oldmem;
        }
    }

    // reg_save_area = -(112 + size of stack vars)(%rbp)
    const_cast<x64Mem*>(addr)->Offset() += 8;
    auto offset = -(alloc_->RspOffset() + 112);
    asmfile_.EmitInstr(fmt::format("    leaq {}(%rbp), %rax\n", offset));
    asmfile_.EmitMov(&rax, addr, 0);

    const_cast<x64Mem*>(addr)->Offset() = oldoff;
}


void CodeGen::Copy8Bytes(
    const x64* mem, RegTag r1, RegTag r2, size_t size)
{
    if (size == 1 || size == 2 || size == 4 || size == 8)
    {
        x64Reg reg{ r2, size };
        asmfile_.EmitMov(mem, &reg);
        return;
    }

    // else if size == 3, 5, 6, 7
    auto m = mem->As<x64Mem>();
    auto oldoff = m->Offset();
    x64Reg via1{ r1, 1 };
    x64Reg via8{ r1 };
    x64Reg to{ r2 };

    asmfile_.EmitBinary("xor", &via8, &via8);
    for (auto off = 0; off < size; ++off)
    {
        asmfile_.EmitMov(m, &via1);
        asmfile_.EmitBinary("shl", off * 8, &via8);
        asmfile_.EmitBinary("or", &via8, &to);
        asmfile_.EmitBinary("xor", &via8, &via8);
        const_cast<x64Mem*>(m)->Offset()++;
    }
    const_cast<x64Mem*>(m)->Offset() = oldoff;
}

void CodeGen::Copy8Bytes(RegTag from, const x64Mem* to, size_t remain)
{
    if (remain == 1 || remain == 2 || remain == 4 || remain == 8)
    {
        x64Reg reg{ from, remain };
        asmfile_.EmitMov(&reg, to, 0);
        return;
    }

    // else if remain == 3, 5, 6, 7
    x64Reg reg{ from, 1 };
    x64Reg reg8{ from };
    auto oldoffset = to->Offset();
    for (int i = 0; i < remain; ++i)
    {
        asmfile_.EmitMov(&reg, to, 0);
        asmfile_.EmitBinary("shr", 8, &reg8);
        const_cast<x64Mem*>(to)->Offset() += 1;
    }
    const_cast<x64Mem*>(to)->Offset() = oldoffset;
}

void CodeGen::Copy8Bytes(
    const x64Mem* from, RegTag v, const x64Mem* to, size_t remain)
{
    if (remain == 1 || remain == 2 || remain == 4 || remain == 8)
    {
        x64Reg via{ v, remain };
        asmfile_.EmitMov(from, &via);
        asmfile_.EmitMov(&via, to, 0);
        return;
    }

    x64Reg via{ v, 1 };
    auto oldfrom = from->Offset();
    auto oldto = to->Offset();
    for (int i = 0; i < remain; ++i)
    {
        asmfile_.EmitMov(from, &via);
        asmfile_.EmitMov(&via, to, 0);
        const_cast<x64Mem*>(from)->Offset() += 1;
        const_cast<x64Mem*>(to)->Offset() += 1;
    }
    const_cast<x64Mem*>(from)->Offset() = oldfrom;
    const_cast<x64Mem*>(to)->Offset() = oldto;
}


void CodeGen::CopySmallHeterIn(const x64* mem, const HeterType* ty)
{
    Copy8Bytes(mem, RegTag::rdi,
        RegTag::rax, ty->Size() > 8 ? 8 : ty->Size());
    if (ty->Size() > 8)
    {
        auto m = mem->As<x64Mem>();
        const_cast<x64Mem*>(m)->Offset() += 8;
        Copy8Bytes(m, RegTag::rdi, RegTag::rdx, ty->Size() - 8);
    }

    if (SysVConv::HasFloat(ty, 0, 8, 0))
        asmfile_.EmitInstr("    vmovq %rax, %xmm0\n");
    if (SysVConv::HasFloat(ty, 8, 16, 0))
        asmfile_.EmitInstr("    vmovq %rdx, %xmm1\n");
}

void CodeGen::CopySmallHeterOut(const x64* mem, const HeterType* h)
{
    if (SysVConv::HasFloat(h, 0, 8, 0))
        asmfile_.EmitInstr("    vmovq %xmm0, %rax\n");
    if (SysVConv::HasFloat(h, 8, 16, 0))
        asmfile_.EmitInstr("    vmovq %xmm1, %rdx\n");

    auto size = h->Size();
    Copy8Bytes(RegTag::rax, mem->As<x64Mem>(), size > 8 ? 8 : size);
    if (size > 8)
    {
        auto m = mem->As<x64Mem>();
        const_cast<x64Mem*>(m)->Offset() += 8;
        Copy8Bytes(RegTag::rdx, m, size - 8);
        const_cast<x64Mem*>(m)->Offset() -= 8;
    }
}

void CodeGen::CopyBigHeter(const x64* m)
{
    // The destination is already in rdi.
    asmfile_.EmitLeaq(m, RegTag::rsi);
    asmfile_.EmitInstr(fmt::format("    movq ${}, %rcx\n", m->Size()));
    asmfile_.EmitInstr("    rep movsb\n");
}

void CodeGen::LoadHeterParam(const x64Heter* heter, const x64Mem* mem)
{
    auto oldoff = mem->Offset();
    for (auto i = heter->begin(); i != heter->end() - 1; ++i)
    {
        if (i->InReg())
        {
            x64Reg reg{ i->ToReg() };
            if (IsVecReg(i->ToReg()))
                asmfile_.EmitInstr(fmt::format(
                    "    vmovq {}, {}\n", reg.ToString(), mem->ToString()));
            else
                asmfile_.EmitMov(&reg, mem, 0);
        }
        else // if i in stack
        {
            auto offset = i->ToOffset();
            asmfile_.EmitInstr(fmt::format(
                "    movq {}(%rbp), %rax\n", static_cast<long>(offset) + 16));
            asmfile_.EmitInstr(fmt::format(
                "    movq %rax, {}\n", mem->ToString()));
        }
        const_cast<x64Mem*>(mem)->Offset() += 8;
    }

    auto remain = heter->Size() - (heter->Count() - 1) * 8;
    if (heter->Back().InReg())
        Copy8Bytes(heter->Back().ToReg(), mem, remain);
    else // heter->Back().InStack()
    {
        x64Mem rbp{ 8, 
            static_cast<long>(heter->Back().ToOffset()) + 16,
            RegTag::rbp, RegTag::none, 0 };
        Copy8Bytes(&rbp, RegTag::rax, mem, remain);
    }
    const_cast<x64Mem*>(mem)->Offset() = oldoff;
}


RegTag CodeGen::GetSpareIntReg(int i) const
{
    auto notused = alloc_->NotUsedIntReg();
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
    auto notused = alloc_->NotUsedVecReg();
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
        x64Mem mem{
            8, static_cast<long>(i->GetRepr().first * scl),
            reg, RegTag::none, 0
        };
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
        x64Reg reg{ GetSpareIntReg(1) };
        asmfile_.EmitMov(i, &reg);
        asmfile_.EmitBinary("imul", scl, &reg);
        LeaqEmitHelper(ptr, rlt);
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
    auto lhs = alloc_->GetIROpMap(bi->Lhs());
    auto rhs = alloc_->GetIROpMap(bi->Rhs());
    auto ans = alloc_->GetIROpMap(bi->Result());

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
    auto ans = alloc_->GetIROpMap(bi->Result());

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
    auto lhs = alloc_->GetIROpMap(bin->Lhs());
    auto rhs = alloc_->GetIROpMap(bin->Rhs());
    auto ans = alloc_->GetIROpMap(bin->Result());

    if (!(rhs->Is<x64Reg>() && *rhs == RegTag::rcx))
        asmfile_.EmitMov(rhs, RegTag::rcx);
    if (*lhs != *ans)
        MovEmitHelper(lhs, ans);

    x64Reg rcx{ RegTag::rcx, rhs->Size() };
    asmfile_.EmitBinary(name, &rcx, ans);
}

const x64* CodeGen::PrepareDivMod(const BinaryInstr* bin)
{
    auto lhs = alloc_->GetIROpMap(bin->Lhs());
    auto rhs = alloc_->GetIROpMap(bin->Rhs());
    auto ans = alloc_->GetIROpMap(bin->Result());
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

void CodeGen::MovEmitHelper(const x64* from, const x64* to, int suffix)
{
    if (auto imm = from->As<x64Imm>(); imm && to->Is<x64Mem>())
    {
        auto val = imm->GetRepr().first;
        if (val > INT32_MAX && val <= UINT32_MAX && to->Size() != 4)
        {
            auto oldsz = to->Size();
            const_cast<x64*>(to)->Size() = 4;
            asmfile_.EmitMov(from, to);
            const_cast<x64*>(to)->Size() = oldsz;
            return;
        }
        else if (val > UINT32_MAX)
            goto usereg;
    }
    if (from->Is<x64Reg>() || from->Is<x64Imm>() || to->Is<x64Reg>())
    {
        asmfile_.EmitMov(from, to, suffix);
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

void CodeGen::VcvtsiEmitHelper(bool sign, const x64* op1, const x64* op2)
{
    size_t oldsz = 0;
    if (op1->Size() != 4 && op1->Size() != 8)
    {
        if (op1->Is<x64Reg>() && sign)
            asmfile_.EmitMovs(op1->Size(), 8, op1);
        else if (op1->Is<x64Reg>() && !sign)
            asmfile_.EmitMovz(op1->Size(), 8, op1);
        oldsz = op1->Size();
        const_cast<x64*>(op1)->Size() = 8;
    }

    if (op2->Is<x64Reg>())
        asmfile_.EmitVcvtsi(op1, op2);
    else
    {
        auto tag = GetSpareVecReg(0);
        asmfile_.EmitVcvtsi(op1, tag);
        asmfile_.EmitVmov(tag, op2);
    }

    if (oldsz)
        const_cast<x64*>(op1)->Size() = oldsz;
}

void CodeGen::VcvttEmitHelper(const x64* op1, const x64* op2)
{
    auto oldsz = op2->Size();
    if (oldsz < 4)
        const_cast<x64*>(op2)->Size() = 4;
    if (op2->Is<x64Reg>())
        asmfile_.EmitVcvtt(op1, op2);
    else
    {
        auto tag = GetSpareVecReg(0);
        asmfile_.EmitVcvtt(op1, tag);
        asmfile_.EmitVmov(tag, op2);
    }
    if (oldsz < 4)
        const_cast<x64*>(op2)->Size() = oldsz;
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
    if (reg->Is<x64Imm>())
    {
        asmfile_.EmitInstr("    pushq " + reg->ToString() + '\n');
        stacksize_ += 8;
        return;
    }

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
    if (auto imm = op1->As<x64Imm>(); imm && imm->GetRepr().first > INT32_MAX)
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
    pipeline_->ExecuteOnModule();
    if (summary_)
    {
        for (auto pass : modulepass_)
        {
            *summary_ << pass->PrintSummary();
            *summary_ << '\n';
        }
    }

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

    if (var->GetExprTree())
        var->GetExprTree()->Accept(this);
    auto repr = var->GetExprTree() ? var->GetExprTree()->repr_ : "";

    if (repr[0] == '"') // A string?
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

        if (!var->GetExprTree())
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
            asmfile_.EmitPseudoInstr(pseudo, { var->GetExprTree()->repr_ });
        }
    }
    asmfile_.EmitBlankLine();
}

void CodeGen::VisitFunction(Function* func)
{
    if (func->Empty())
        return;

    pipeline_->ExecuteOnFunction(func);
    if (summary_)
    {
        for (auto pass : funcpass_)
        {
            *summary_ << pass->PrintSummary();
            *summary_ << '\n';
        }
    }

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
    AdjustRsp(-alloc_->RspOffset());

    if (func->Variadic())
    {
        AlignRspBy(0, 8);
        AdjustRsp(-112);
        asmfile_.EmitMov(RegTag::rdi, (long)0);
        asmfile_.EmitMov(RegTag::rsi, 8);
        asmfile_.EmitMov(RegTag::rdx, 16);
        asmfile_.EmitMov(RegTag::rcx, 24);
        asmfile_.EmitMov(RegTag::r8, 32);
        asmfile_.EmitMov(RegTag::r9, 40);

        auto rax = x64Reg(RegTag::rax, 1);
        auto label = GetLabel();
        asmfile_.EmitTest(&rax, &rax);
        asmfile_.EmitJmp("e", label);

        asmfile_.EmitVmov(RegTag::xmm0, 48);
        asmfile_.EmitVmov(RegTag::xmm1, 56);
        asmfile_.EmitVmov(RegTag::xmm2, 64);
        asmfile_.EmitVmov(RegTag::xmm3, 72);
        asmfile_.EmitVmov(RegTag::xmm4, 80);
        asmfile_.EmitVmov(RegTag::xmm5, 88);
        asmfile_.EmitVmov(RegTag::xmm6, 96);
        asmfile_.EmitVmov(RegTag::xmm7, 104);

        asmfile_.EmitLabel(label);
    }

    SaveCalleeSaved();
    asmfile_.Write2Mem();

    for (auto bb : *func)
        VisitBasicBlock(bb);

    asmfile_.Dump2File();
    asmfile_.EmitBlankLine();

    pipeline_->ExitFunction();
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
        auto mapped = alloc_->GetIROpMap(inst->ReturnValue());
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
    else if (inst->ReturnValue()->Type()->Is<HeterType>())
    {
        auto mapped = alloc_->GetIROpMap(inst->ReturnValue());
        if (inst->ReturnValue()->Type()->Size() > 16)
        {
            CopyBigHeter(mapped);
            asmfile_.EmitInstr("    movq %rdi, %rax\n");
        }
        else if (mapped->Is<x64Mem>())
            CopySmallHeterIn(mapped, inst->ReturnValue()->Type()->As<HeterType>());
        else // if (mapped->Is<x64Heter>())
        {
            auto fromreg = [this] (RegTag from, RegTag to) {
                x64Reg f{ from, 8 };
                asmfile_.EmitMov(&f, RegTag::rax);
            };
            auto fromstack = [this] (size_t offset, const std::string& r) {
                auto o = static_cast<long>(offset) + 16;
                asmfile_.EmitInstr(
                    fmt::format("    movq {}(%rbp), {}\n", o, r));
            };
            auto h = mapped->As<x64Heter>();
            auto& first = h->Front();
            if (first.InReg())
                fromreg(first.ToReg(), RegTag::rax);
            else // if (first.InStack())
                fromstack(first.ToOffset(), "%rax");

            if (h->Size() <= 8)
                goto ret;

            auto& second = h->Back();
            if (second.InReg())
                fromreg(second.ToReg(), RegTag::rdx);
            else // if (second.InStack())
                fromstack(second.ToOffset(), "%rdx");
        }
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
    if (!inst->Cond())
    {
        asmfile_.EmitJmp("", GetLabel(inst->GetTrueBlk()));
        return;
    }
    if (auto ic = inst->Cond()->As<IntConst>(); ic)
    {
        if (ic->Val())
            asmfile_.EmitJmp("", GetLabel(inst->GetTrueBlk()));
        else
            asmfile_.EmitJmp("", GetLabel(inst->GetFalseBlk()));
        return;
    }

    if (inst->Cond()->Type()->Is<FloatType>())
    {
        auto cond = MapPossibleFloat(inst->Cond());
        auto zero = GetFpLabel({ 0, 0 }, cond->Size());
        x64Mem mem{ cond->Size(), std::move(zero) };
        asmfile_.EmitUcom(&mem, cond);
    }
    else
        asmfile_.EmitCmp((unsigned long)0, alloc_->GetIROpMap(inst->Cond()));
    asmfile_.EmitJmp("ne", GetLabel(inst->GetTrueBlk()));
    asmfile_.EmitJmp("", GetLabel(inst->GetFalseBlk()));
}


void CodeGen::VisitSwitchInstr(SwitchInstr* inst)
{
    // I use a somewhat straight forward way to translate
    // the switch instruction. no jump table is used, though.

    auto ident = alloc_->GetIROpMap(inst->GetIdent());
    for (auto [tag, bb] : inst->GetValueBlkPairs())
    {
        CmpEmitHelper(ident, alloc_->GetIROpMap(tag));
        asmfile_.EmitJmp("z", GetLabel(bb));
    }
    asmfile_.EmitJmp("", GetLabel(inst->GetDefault()));
}


void CodeGen::VisitCallInstr(CallInstr* inst)
{
    if (inst->FuncName() == "@__Ginkgo_va_start")
    {
        HandleVaStart(inst);
        return;
    }

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
    auto proto = inst->Proto() ?
        inst->Proto() :
        // Call through a function pointer?
        inst->FuncAddr()->Type()->As<PtrType>()->Point2()->As<FuncType>();

    SysVConv conv{ proto, &inst->ArgvList() };
    conv.MapArgv();

    auto [_, vec] = conv.CountRegs();
    if (proto->Variadic())
    {
        if (vec == 0)
            asmfile_.EmitInstr("    xorq %rax, %rax\n");
        else
            asmfile_.EmitInstr("    movb $" + std::to_string(vec) + ", %al\n");
    }

    auto extra = GetAlign(stacksize_, 16);
    AdjustRsp(-(conv.Padding() + extra));

    if (proto->ReturnType()->Is<HeterType>() && proto->ReturnType()->Size() > 16)
        asmfile_.EmitLeaq(alloc_->GetIROpMap(inst->Result()), RegTag::rdi);

    // If an SysVConv object is constructured by the two-parameter ctor,
    // the size of variadic part will be included in conv.StackSize() as well.
    PassParam(conv, proto->ParamType().size(), inst->ArgvList());

    if (inst->FuncAddr())
        asmfile_.EmitCall(alloc_->GetIROpMap(inst->FuncAddr()));
    else
        asmfile_.EmitCall(inst->FuncName().substr(1) + "@PLT");
    // adjust rsp directly here since conv.StackSize()
    // has already included 'padding'
    AdjustRsp(conv.StackSize() + extra);
    RestoreCallerSaved();

    if (!inst->Result())
        return;

    if (inst->Result()->Type()->Is<IntType>())
    {
        auto x64reg = alloc_->GetIROpMap(inst->Result());
        if (!x64reg->Is<x64Reg>() || x64reg->As<x64Reg>()->Tag() != RegTag::rax)
            asmfile_.EmitMov(RegTag::rax, x64reg);
    }
    else if (inst->Result()->Type()->Is<FloatType>())
    {
        auto vecreg = alloc_->GetIROpMap(inst->Result());
        if (!vecreg->Is<x64Reg>() || vecreg->As<x64Reg>()->Tag() != RegTag::xmm0)
            VecMovEmitHelper(RegTag::xmm0, vecreg);
    }
    else if (inst->Result()->Type()->Is<HeterType>() &&
        inst->Result()->Type()->Size() <= 16)
    {
        CopySmallHeterOut(
            alloc_->GetIROpMap(inst->Result()),
            inst->Result()->Type()->As<HeterType>());
    }
    // else if the result with hetertype is geater than 16 bytes
    // the return value will be loaded directly to the desired address
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
    if (inst->Lhs()->Type()->As<IntType>()->IsSigned() ||
        inst->Rhs()->Type()->As<IntType>()->IsSigned())
    {
        BinaryGenHelper("imul", inst);
        return;
    }
    auto lhs = alloc_->GetIROpMap(inst->Lhs());
    asmfile_.EmitMov(lhs, RegTag::rax);
    asmfile_.EmitUnary("mul", alloc_->GetIROpMap(inst->Rhs()));
    asmfile_.EmitMov(RegTag::rax, alloc_->GetIROpMap(inst->Result()));
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
    if (!(*alloc_->GetIROpMap(inst->Result()) == RegTag::rdx))
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
    else if (value->Type()->Is<HeterType>())
    {
        auto mapped = alloc_->GetIROpMap(value);
        if (mapped->Is<x64Heter>())
        {
            LoadHeterParam(mapped->As<x64Heter>(),
                mappeddest->As<x64Mem>());
        }
        else // if (mapped->Is<x64Mem>())
        {
            // save all involved registers here - if the virtual register
            // is actually a function parameter, not saving these registers
            // could be a disaster.
            asmfile_.EmitPush(RegTag::rcx, 8);
            asmfile_.EmitPush(RegTag::rsi, 8);
            asmfile_.EmitPush(RegTag::rdi, 8);

            asmfile_.EmitLeaq(mappeddest, RegTag::rdi);
            asmfile_.EmitLeaq(mapped, RegTag::rsi);
            asmfile_.EmitInstr(fmt::format(
                "    movq ${}, %rcx\n", value->Type()->Size()));
            asmfile_.EmitInstr("    rep movsb\n");

            asmfile_.EmitPop(RegTag::rdi, 8);
            asmfile_.EmitPop(RegTag::rsi, 8);
            asmfile_.EmitPop(RegTag::rcx, 8);
        }
        return;
    }

    auto mappedval = alloc_->GetIROpMap(value);
    if (value->Type()->Is<PtrType>() &&
        mappedval->Is<x64Mem>() && !mappedval->As<x64Mem>()->LoadTwice())
        LeaqEmitHelper(mappedval, mappeddest);
    else
        MovEmitHelper(mappedval, mappeddest, 0);
}

void CodeGen::VisitLoadInstr(LoadInstr* inst)
{
    auto result = inst->Result();
    auto mappedptr = LoadPointer(inst->Pointer());

    if (inst->Pointer()->Name()[0] == '@') // Global variable or extern symbol?
    {
        auto ptr = inst->Pointer()->Type()->As<PtrType>();
        if (auto ptr2 = ptr->Point2()->As<PtrType>();
            ptr2 && ptr2->Point2()->Is<FuncType>())
            LeaqEmitHelper(mappedptr, alloc_->GetIROpMap(result));
        else if (auto ptr2 = ptr->Point2()->As<FloatType>(); ptr2)
            VecMovEmitHelper(mappedptr, alloc_->GetIROpMap(result));
        else
            MovEmitHelper(mappedptr, alloc_->GetIROpMap(result));
        return;
    }

    if (result->Type()->Is<HeterType>())
    {
        auto used = alloc_->UsedIntReg();
        if (used.count(RegTag2X64Phys(RegTag::rcx)))
            asmfile_.EmitPush(RegTag::rcx, 8);
        if (used.count(RegTag2X64Phys(RegTag::rsi)))
            asmfile_.EmitPush(RegTag::rsi, 8);
        if (used.count(RegTag2X64Phys(RegTag::rdi)))
            asmfile_.EmitPush(RegTag::rdi, 8);

        auto mappedres = alloc_->GetIROpMap(result);
        asmfile_.EmitLeaq(mappedres, RegTag::rdi);
        asmfile_.EmitLeaq(mappedptr, RegTag::rsi);
        asmfile_.EmitInstr(fmt::format(
            "    movq ${}, %rcx\n", result->Type()->Size()));
        asmfile_.EmitInstr("    rep movsb\n");

        if (used.count(RegTag2X64Phys(RegTag::rcx)))
            asmfile_.EmitPop(RegTag::rdi, 8);
        if (used.count(RegTag2X64Phys(RegTag::rsi)))
            asmfile_.EmitPop(RegTag::rsi, 8);
        if (used.count(RegTag2X64Phys(RegTag::rdi)))
            asmfile_.EmitPop(RegTag::rcx, 8);
        return;
    }

    if (result->Type()->Is<FloatType>())
        VecMovEmitHelper(mappedptr, MapPossibleFloat(result));
    else
        MovEmitHelper(mappedptr, alloc_->GetIROpMap(result));
}

void CodeGen::VisitGetElePtrInstr(GetElePtrInstr* inst)
{
    if (inst->HoldsInt()) // Want to get pointer to a field in a structure?
    {
        auto heter = inst->Pointer()->
            Type()->As<PtrType>()->Point2()->As<HeterType>();
        auto offset = heter->At(inst->IntIndex()).second;
        auto ptr = alloc_->GetIROpMap(inst->Pointer());
        auto result = alloc_->GetIROpMap(inst->Result());

        if (auto p = ptr->As<x64Reg>(); p)
        {
            x64Mem mem{ 8, static_cast<long>(offset),
                p->Tag(), RegTag::none, 0 };
            LeaqEmitHelper(&mem, result);
            return;
        }
        // else if (ptr->Is<x64Mem>())
        auto m = ptr->As<x64Mem>();
        if (m->LoadTwice())
        {
            x64Reg reg{ GetSpareIntReg(0), 8 };
            asmfile_.EmitMov(m, &reg);
            asmfile_.EmitBinary("add", offset, &reg);
            asmfile_.EmitMov(&reg, result);
        }
        else
        {
            LeaqEmitHelper(m, result);
            asmfile_.EmitBinary("add", offset, result);
        }
        return;
    }

    size_t size = 0;
    if (auto ptr = inst->Pointer()->Type()->As<PtrType>(); ptr)
    {
        if (ptr->Point2()->Is<ArrayType>() && inst->IsInner())
            size = ptr->Point2()->As<ArrayType>()->ArrayOf()->Size();
        else
            size = ptr->Point2()->Size();
    }
    else // if "pointer" is an array
        size = inst->Pointer()->Type()->As<ArrayType>()->ArrayOf()->Size();

    auto pointer = MapPossibleRegister(inst->Pointer())->As<x64Mem>();
    auto index = alloc_->GetIROpMap(inst->OpIndex());
    auto result = alloc_->GetIROpMap(inst->Result());

    if (index->Is<x64Imm>())
        GetElePtrImmHelper(pointer, index->As<x64Imm>(), result, size);
    else if (index->Is<x64Reg>())
        GetElePtrRegHelper(pointer, index->As<x64Reg>(), result, size);
    else if (index->Is<x64Mem>())
        GetElePtrMemHelper(pointer, index->As<x64Mem>(), result, size);
}


void CodeGen::VisitTruncInstr(TruncInstr* inst)
{
    auto from = alloc_->GetIROpMap(inst->Value());
    auto to = alloc_->GetIROpMap(inst->Dest());

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
        auto mask = ~((long)(0x8000000000000000) >> (63 - to->Size() * 8));
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
    auto to = alloc_->GetIROpMap(inst->Dest());
    VcvtEmitHelper(from, to);
}

void CodeGen::VisitZextInstr(ZextInstr* inst)
{
    auto from = alloc_->GetIROpMap(inst->Value());
    auto to = alloc_->GetIROpMap(inst->Dest());
    if (from->Size() < 4)
        MovzEmitHelper(from, to);
    else // clean the higher 32 bits as well
        asmfile_.EmitMov(from, from);
}

void CodeGen::VisitSextInstr(SextInstr* inst)
{
    auto from = alloc_->GetIROpMap(inst->Value());
    auto to = alloc_->GetIROpMap(inst->Dest());
    MovsEmitHelper(from, to);
}

void CodeGen::VisitFextInstr(FextInstr* inst)
{
    auto from = MapPossibleFloat(inst->Value());
    auto to = alloc_->GetIROpMap(inst->Dest());
    VcvtEmitHelper(from, to);
}

void CodeGen::VisitFtoUInstr(FtoUInstr* inst)
{
    auto from = MapPossibleFloat(inst->Value());
    auto to = alloc_->GetIROpMap(inst->Dest());
    VcvttEmitHelper(from, to);
}

void CodeGen::VisitFtoSInstr(FtoSInstr* inst)
{
    auto from = MapPossibleFloat(inst->Value());
    auto to = alloc_->GetIROpMap(inst->Dest());
    VcvttEmitHelper(from, to);
}

void CodeGen::VisitUtoFInstr(UtoFInstr* inst)
{
    auto from = alloc_->GetIROpMap(inst->Value());
    auto to = alloc_->GetIROpMap(inst->Dest());
    if (from->Size() != 8)
    {
        MovzEmitHelper(from->Size(), 8, from);
        VcvtsiEmitHelper(false, from, to);
        return;
    }

    auto movebiguint = GetLabel();
    auto end = GetLabel();
    TestEmitHelper(from, from);
    asmfile_.EmitJmp("s", movebiguint);
    // For uint less than 64 bit,
    // just use a single vcvtsixx2xx instruction
    VcvtsiEmitHelper(false, from, to);
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
    VcvtsiEmitHelper(false, &temp1, to);
    asmfile_.EmitLabel(end);
}

void CodeGen::VisitStoFInstr(StoFInstr* inst)
{
    auto from = alloc_->GetIROpMap(inst->Value());
    auto to = alloc_->GetIROpMap(inst->Dest());
    VcvtsiEmitHelper(true, from, to);
}


void CodeGen::VisitPtrtoIInstr(PtrtoIInstr* inst)
{
    auto from = alloc_->GetIROpMap(inst->Value());
    auto to = alloc_->GetIROpMap(inst->Dest());
    size_t oldsz = 0;
    if (to->Size() != 8)
    {
        oldsz = to->Size();
        const_cast<x64*>(to)->Size() = 8;
    }
    auto mem = from->As<x64Mem>();
    if ((mem && mem->LoadTwice()) || !mem)
        MovEmitHelper(from, to);
    else // if (mem && mem->LoadOnce())
        LeaqEmitHelper(from, to);
    if (oldsz)
        const_cast<x64*>(to)->Size() = oldsz;
}

void CodeGen::VisitItoPtrInstr(ItoPtrInstr* inst)
{
    auto from = alloc_->GetIROpMap(inst->Value());
    auto to = alloc_->GetIROpMap(inst->Dest());
    // LoadTwice has been set in the register allocator,
    // and the destination of itoptr must be a load-twice
    // pointer if it is mapped onto the stack. So its value
    // can be set by a single mov instruction.
    if (from->Size() == 8)
        MovEmitHelper(from, to);
    else if (from->Size() == 4)
    {
        const_cast<x64*>(to)->Size() = 4;
        MovEmitHelper(from, to);
        const_cast<x64*>(to)->Size() = 8;
    }
    else // if (from->Size() == 1 || from->Size() == 2)
        MovzEmitHelper(from, to);
}

void CodeGen::VisitBitcastInstr(BitcastInstr* inst)
{
    auto from = alloc_->GetIROpMap(inst->Value());
    auto to = alloc_->GetIROpMap(inst->Dest());
    if (from->Is<x64Reg>() && to->Is<x64Reg>())
    {
        auto f = from->As<x64Reg>();
        auto t = to->As<x64Reg>();
        if (f->Tag() != t->Tag())
            asmfile_.EmitMov(from, to);
        return;
    }
    else if (from->Is<x64Reg>())
    {
        asmfile_.EmitMov(from, to);
        return;
    }
    auto m = from->As<x64Mem>();
    if (m->LoadTwice())
        MovEmitHelper(m, to);
    else
        LeaqEmitHelper(m, to);
}


void CodeGen::VisitIcmpInstr(IcmpInstr* inst)
{
    auto lhs = alloc_->GetIROpMap(inst->Op1());
    auto rhs = alloc_->GetIROpMap(inst->Op2());
    auto ans = alloc_->GetIROpMap(inst->Result());
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
    auto ans = alloc_->GetIROpMap(inst->Result());

    UcomEmitHelper(rhs, lhs);
    asmfile_.EmitSet(Cond2Str(inst->Cond(), false), ans);
    if (ans->Size() != 1)
        MovzEmitHelper(1, ans->Size(), ans);
}


void CodeGen::VisitSelectInstr(SelectInstr* inst)
{
    auto [cond, ty] = inst->CondPair();
    auto mappedcond = MapPossibleFloat(cond);
    auto v1 = MapPossibleFloat(inst->Value1());
    auto v2 = MapPossibleFloat(inst->Value2());
    auto ans = alloc_->GetIROpMap(inst->Result());

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
    // FIXME: the current implementation can't correctly solve
    // the lost-copies and phi-swap problems. With limited
    // use of phi instructions in the code generated by the
    // front end, however, it's okay to ignore them before
    // mem2reg pass is actually implemented.

    auto curbb = asmfile_.CurBlock();
    auto mappedres = alloc_->GetIROpMap(inst->Result());

    for (auto [bb, op] : inst->GetBlockValPair())
    {
        asmfile_.SwitchBlock(bb);
        auto br = bb->LastInstr()->As<BrInstr>();
        if (br && br->Cond() && !br->Cond()->Is<IntConst>())
            asmfile_.SetInsertPoint(-2);
        else if (br /* && !br->Cond() */)
        // Direct jump; Note that constant condition
        // is treated as if it were a direct jump
            asmfile_.SetInsertPoint(-1);
        else // if the last instruction of bb is SwitchInstr
        {
            auto swtch = bb->LastInstr()->As<SwitchInstr>();
            asmfile_.SetInsertPoint(-(swtch->GetValueBlkPairs().size() * 2 + 1));
        }
        if (op->Type()->Is<IntType>())
            MovEmitHelper(alloc_->GetIROpMap(op), mappedres);
        else if (op->Type()->Is<FloatType>())
            VecMovEmitHelper(MapPossibleFloat(op), mappedres);
    }

    asmfile_.SwitchBlock(curbb);
    asmfile_.SetInsertPoint(0);
}
