#include "pass/x64Alloc.h"
#include "IR/IROperand.h"
#include "IR/Value.h"
#include "visitir/SysVConv.h"
#include "visitir/x64.h"
#include <algorithm>
#include <fmt/format.h>


void x64Alloc::LoadParam()
{
    auto& params = CurFunc()->Params();
    SysVConv conv{ CurFunc()->Type() };
    conv.MapArgv();

    for (int i = params.size() - 1; i >= 0; --i)
    {
        auto loc = conv.PlaceOfArgv(i);
        if (loc)
            MapRegister(params[i], std::make_unique<x64Reg>(*loc->As<x64Reg>()));
        else
        {
            auto offset = conv.OffsetOfArgv(i);
            MapRegister(params[i], std::make_unique<x64Mem>(
                params[i]->Type()->Size(), offset + 16, RegTag::rbp, RegTag::none, 0));
        }
    }
}

bool x64Alloc::MapConstAndGlobalVar(const IROperand* op)
{
    if (op->Is<Constant>())
    {
        ir_[op] = std::make_unique<x64Imm>(op->As<Constant>());
        return true;
    }
    auto reg = op->As<Register>();
    if (reg->Name()[0] == '@')
    {
        ir_[op] = std::make_unique<x64Mem>(
            reg->Type()->As<PtrType>()->Point2()->Size(), reg->Name().substr(1));
        return true;
    }
    return false;
}

void x64Alloc::MapRegister(const IROperand* op, std::unique_ptr<x64> reg)
{
    ir_[op] = std::move(reg);
}

void x64Alloc::MarkLoadTwice(const IROperand* op)
{
    auto mapped = ir_[op].get();
    if (auto mem = mapped->As<x64Mem>(); mem)
        mem->LoadTwice() = true;
}


std::string x64Alloc::PrintSummary() const
{
    std::string summary{ fmt::format("Pass x64Alloc in function {}:\n", CurFunc()->Name()) };
    summary += "IR virtual reg -> x64 position\n";
    for (auto& [reg, to] : ir_)
        if (auto r = reg->As<Register>(); r)
            summary += fmt::format("{} -> {}\n", r->Name(), to->ToString());
    summary += '\n';
    return std::move(summary);
}


const x64* x64Alloc::GetIROpMap(const IROperand* op) const
{
    auto it = ir_.find(op);
    if (it == ir_.end()) return nullptr;
    return it->second.get();
}


// from https://www.zhihu.com/question/298981020/answer/519864425
// An awesome answer.
#define REG_COUNT_HELPER(...)                                   \
static auto anyone = [](auto&& k, auto&&... args) -> bool {     \
    return ((args == k) || ...);                                \
};                                                              \
std::set<x64Phys> regs{};                                       \
for (auto reg : reg_)                                           \
    if (anyone(reg, __VA_ARGS__))                               \
        regs.emplace(reg);                                      \
return regs

x64Alloc::RegSet x64Alloc::UsedCallerSaved() const
{
    REG_COUNT_HELPER(x64Phys::r10, x64Phys::r11);
}

x64Alloc::RegSet x64Alloc::UsedCalleeSaved() const
{
    REG_COUNT_HELPER(
        x64Phys::rbx,   x64Phys::rbp,   x64Phys::r12,   x64Phys::r13,
        x64Phys::r14,   x64Phys::r15,   x64Phys::xmm6,  x64Phys::xmm7,
        x64Phys::xmm8,  x64Phys::xmm9,  x64Phys::xmm10, x64Phys::xmm11,
        x64Phys::xmm12, x64Phys::xmm13, x64Phys::xmm14, x64Phys::xmm15
    );
}

x64Alloc::RegSet x64Alloc::UsedIntReg() const
{
    REG_COUNT_HELPER(
        x64Phys::rax, x64Phys::rbx, x64Phys::rcx, x64Phys::rdx,
        x64Phys::rsi, x64Phys::rdi, x64Phys::rbp, x64Phys::rsp,
        x64Phys::r8,  x64Phys::r9,  x64Phys::r10, x64Phys::r11,
        x64Phys::r12, x64Phys::r13, x64Phys::r14, x64Phys::r15
    );
}

x64Alloc::RegSet x64Alloc::UsedVecReg() const
{
    REG_COUNT_HELPER(
        x64Phys::xmm0,  x64Phys::xmm1,  x64Phys::xmm2,  x64Phys::xmm3,
        x64Phys::xmm4,  x64Phys::xmm5,  x64Phys::xmm6,  x64Phys::xmm7,
        x64Phys::xmm8,  x64Phys::xmm9,  x64Phys::xmm10, x64Phys::xmm11,
        x64Phys::xmm12, x64Phys::xmm13, x64Phys::xmm14, x64Phys::xmm15
    );
}

#undef REG_COUNT_HELPER


x64Alloc::RegSet x64Alloc::NotUsedIntReg() const
{
    static const std::set<x64Phys> intset = {
        x64Phys::rax, x64Phys::rbx, x64Phys::rcx, x64Phys::rdx,
        x64Phys::rsi, x64Phys::rdi, x64Phys::rbp, /* x64Phys::rsp, */
        x64Phys::r8,  x64Phys::r9,  x64Phys::r10, x64Phys::r11,
        x64Phys::r12, x64Phys::r13, x64Phys::r14, x64Phys::r15,
    };

    auto used = UsedIntReg();
    std::set<x64Phys> notused{};
    std::set_difference(intset.begin(), intset.end(),
        used.begin(), used.end(), std::inserter(notused, notused.begin()));
    return notused;
}

x64Alloc::RegSet x64Alloc::NotUsedVecReg() const
{
    static const std::set<x64Phys> vecset = {
        x64Phys::xmm0,  x64Phys::xmm1,  x64Phys::xmm2,  x64Phys::xmm3,
        x64Phys::xmm4,  x64Phys::xmm5,  x64Phys::xmm6,  x64Phys::xmm7,
        x64Phys::xmm8,  x64Phys::xmm9,  x64Phys::xmm10, x64Phys::xmm11,
        x64Phys::xmm12, x64Phys::xmm13, x64Phys::xmm14, x64Phys::xmm15,
    };

    auto used = UsedVecReg();
    std::set<x64Phys> notused{};
    std::set_difference(vecset.begin(), vecset.end(),
        used.begin(), used.end(), std::inserter(notused, notused.begin()));
    return notused;
}
