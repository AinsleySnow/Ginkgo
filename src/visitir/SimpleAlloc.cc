#include "visitir/SimpleAlloc.h"
#include "IR/Value.h"


RegTag SimpleAlloc::StackCache::SpareReg() const
{
    auto ret = RegTag::none;
    if (index_ >= 3) return ret;
    if (index_ == 0) ret = RegTag::rax;
    if (index_ == 1) ret = RegTag::rdx;
    if (index_ == 2) ret = RegTag::rcx;
    index_ += 1;
    return ret;
}

RegTag SimpleAlloc::StackCache::SpareFReg() const
{
    auto ret = RegTag::none;
    if (findex_ >= 3) return ret;
    if (findex_ == 0) ret = RegTag::xmm0;
    if (findex_ == 1) ret = RegTag::xmm1;
    if (findex_ == 2) ret = RegTag::xmm2;
    findex_ += 1;
    return ret;
}

const x64* SimpleAlloc::StackCache::AccessStack(const Register* reg) const
{
    auto it = regmap_.find(reg);
    if (it == regmap_.end()) return nullptr;

    auto mempos = it->second;
    if (mempos->Is<x64Reg>()) index_ -= 1;
    return it->second;
}

const x64* SimpleAlloc::StackCache::GetPosition(const Register* reg) const
{
    auto it = regmap_.find(reg);
    if (it == regmap_.end()) return nullptr;
    return it->second;
}

void SimpleAlloc::StackCache::Map2Reg(const Register* reg, RegTag tag)
{
    regmap_[reg] = x64Reg::CreateX64Reg(basicblock_, tag);
}

void SimpleAlloc::StackCache::Map2Stack(const Register* reg, long offset)
{
    regmap_[reg] = x64Mem::CreateX64Mem(
        basicblock_, offset, x64Reg::CreateX64Reg(basicblock_, RegTag::rsp));
}


void SimpleAlloc::Allocate(BasicBlock* bb, const Register* reg)
{
    auto tag = reg->Type()->IsFloat() ?
        stackcache_.SpareFReg() : stackcache_.SpareReg();

    if (tag != RegTag::none)
        stackcache_.Map2Reg(reg, tag);
    else
    {
        auto info = InfoAt(curfunc_);
        auto offset = AllocateOnX64Stack(
            info, reg->Type()->Size(), reg->Type()->Size());
        stackcache_.Map2Stack(reg, offset);
    }
}


long SimpleAlloc::AllocateOnX64Stack(x64Stack& info, size_t size, size_t align)
{
    long base = 0;

    // not a leaf; just use allocated_ as the offset.
    if (!info.leaf_ || info.allocated_ + size > 128)
    {
        info.allocated_ = MakeAlign(info.allocated_, align);
        base = info.allocated_;
        info.allocated_ += size;
    }
    // leaf; use both red zone and zone above rsp.
    else if (info.belowrsp_ + size <= 128)
    {
        // meet strict alignment requirements.
        info.belowrsp_ = MakeAlign(info.belowrsp_, align);
        base = -info.belowrsp_;
        info.belowrsp_ += size;
    }

    return base;
}


void SimpleAlloc::BinaryAllocaHelper(BinaryInstr* instr)
{
    auto lhs = MapConstAndGlobalVar(curbb_, instr->Lhs());
    auto rhs = MapConstAndGlobalVar(curbb_, instr->Rhs());

    auto lreg = instr->Lhs()->As<Register>();
    auto rreg = instr->Rhs()->As<Register>();
    if (!lhs) Allocate(curbb_, lreg);
    if (!rhs) Allocate(curbb_, rreg);
    if (!lhs) lhs = stackcache_.AccessStack(lreg);
    if (!rhs) rhs = stackcache_.AccessStack(rreg);

    auto ansreg = instr->Result()->As<Register>();
    Allocate(curbb_, ansreg);
    instr->Result() = stackcache_.AccessStack(ansreg);
}


void SimpleAlloc::VisitFunction(Function* func)
{
    // maybe this loop can be eliminated by
    // interprocedural analysis?
    for (auto bb : *func)
        for (auto i : *bb)
            if (i->Is<CallInstr>())
            {
                InfoAt(func).leaf_ = false;
                break;
            }

    for (auto bb : *func)
        for (auto i : *bb)
            if (i->Is<AllocaInstr>())
                VisitAllocaInstr(i->As<AllocaInstr>());

    for (auto bb : *func)
        VisitBasicBlock(bb);

    // we need to guarantee that info.allocated_ is always a multiple of 16
    auto& info = InfoAt(func);
    info.allocated_ = MakeAlign(info.allocated_, 16);
    if (!info.leaf_)
        // eight more bytes for the return address.
        info.allocated_ += 8;
}


void SimpleAlloc::VisitBasicBlock(BasicBlock* bb)
{
    curbb_ = bb;
    for (auto instr : *bb)
        if (!instr->Is<AllocaInstr>())
            instr->Accept(this);
}


void SimpleAlloc::VisitAddInstr(AddInstr* instr) { BinaryAllocaHelper(instr); }
void SimpleAlloc::VisitFaddInstr(FaddInstr* instr) { BinaryAllocaHelper(instr); }
void SimpleAlloc::VisitSubInstr(SubInstr* instr) { BinaryAllocaHelper(instr); }
void SimpleAlloc::VisitFsubInstr(FsubInstr* instr) { BinaryAllocaHelper(instr); }
void SimpleAlloc::VisitMulInstr(MulInstr* instr) { BinaryAllocaHelper(instr); }
void SimpleAlloc::VisitFmulInstr(FmulInstr* instr) { BinaryAllocaHelper(instr); }
void SimpleAlloc::VisitDivInstr(DivInstr* instr) { BinaryAllocaHelper(instr); }
void SimpleAlloc::VisitFdivInstr(FdivInstr* instr) { BinaryAllocaHelper(instr); }
void SimpleAlloc::VisitModInstr(ModInstr* instr) { BinaryAllocaHelper(instr); }
void SimpleAlloc::VisitShlInstr(ShlInstr* instr) { BinaryAllocaHelper(instr); }
void SimpleAlloc::VisitLshrInstr(LshrInstr* instr) { BinaryAllocaHelper(instr); }
void SimpleAlloc::VisitAshrInstr(AshrInstr* instr) { BinaryAllocaHelper(instr); }
void SimpleAlloc::VisitAndInstr(AndInstr* instr) { BinaryAllocaHelper(instr); }
void SimpleAlloc::VisitOrInstr(OrInstr* instr) { BinaryAllocaHelper(instr); }
void SimpleAlloc::VisitXorInstr(XorInstr* instr) { BinaryAllocaHelper(instr); }


void SimpleAlloc::VisitAllocaInstr(AllocaInstr* instr)
{
    auto size = instr->Type()->Size();
    auto align = instr->Type()->Align();

    auto& info = InfoAt(curfunc_);
    auto offset = AllocateOnX64Stack(info, size, align);
    stackcache_.Map2Stack(instr->Result()->As<Register>(), offset);
}


void SimpleAlloc::VisitLoadInstr(LoadInstr* instr)
{
    auto ptr = MapConstAndGlobalVar(curbb_, instr->Pointer());
    auto ptrreg = instr->Pointer()->As<Register>();
    if (!ptr) instr->Pointer() = stackcache_.AccessStack(ptrreg);

    Allocate(curbb_, ptrreg);
    instr->Result() = stackcache_.GetPosition(ptrreg);
}


void SimpleAlloc::VisitStoreInstr(StoreInstr* instr)
{
    auto value = MapConstAndGlobalVar(curbb_, instr->Value());
    auto dest = MapConstAndGlobalVar(curbb_, instr->Dest());

    auto valreg = instr->Value()->As<Register>();
    auto destreg = instr->Dest()->As<Register>();
    if (!dest) Allocate(curbb_, destreg);
    if (!value) Allocate(curbb_, valreg);

    instr->Value() = stackcache_.GetPosition(valreg);
    instr->Dest() = stackcache_.GetPosition(destreg);
}
