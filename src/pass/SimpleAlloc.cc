#include "pass/SimpleAlloc.h"
#include "IR/Value.h"
#include "IR/Instr.h"


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

void SimpleAlloc::StackCache::AccessStack(const Register* reg) const
{
    auto it = regmap_.find(reg);
    if (it == regmap_.end()) return;

    auto mempos = it->second;
    if (mempos->Is<x64Reg>()) index_ -= 1;
    return;
}


void SimpleAlloc::StackCache::Map2Reg(const Register* reg, RegTag tag)
{
    auto px64 = std::make_unique<x64Reg>(tag, reg->Type()->Size());
    auto raw = px64.get();
    alloc_.MapRegister(reg, std::move(px64));
    regmap_[reg] = raw;
}

void SimpleAlloc::StackCache::Map2Stack(const Register* reg, long offset)
{
    auto px64 = std::make_unique<x64Mem>(reg->Type()->Size(), offset, RegTag::rsp, RegTag::none, 0);
    auto raw = px64.get();
    alloc_.MapRegister(reg, std::move(px64));
    regmap_[reg] = raw;
}


void SimpleAlloc::Allocate(const Register* reg)
{
    auto tag = reg->Type()->IsFloat() ?
        stackcache_.SpareFReg() : stackcache_.SpareReg();

    if (tag != RegTag::none)
        stackcache_.Map2Reg(reg, tag);
    else
    {
        auto offset = AllocateOnX64Stack(
            ArchInfo(), reg->Type()->Size(), reg->Type()->Size());
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
    auto lhs = MapConstAndGlobalVar(instr->Lhs());
    auto rhs = MapConstAndGlobalVar(instr->Rhs());

    auto lreg = instr->Lhs()->As<Register>();
    auto rreg = instr->Rhs()->As<Register>();
    if (!lhs) Allocate(lreg);
    if (!rhs) Allocate(rreg);
    if (!lhs) stackcache_.AccessStack(lreg);
    if (!rhs) stackcache_.AccessStack(rreg);

    auto ansreg = instr->Result()->As<Register>();
    Allocate(ansreg);
    stackcache_.AccessStack(ansreg);
}


void SimpleAlloc::VisitFunction(Function* func)
{
    // maybe this loop can be eliminated by
    // interprocedural analysis?
    for (auto bb : *func)
        for (auto i : *bb)
            if (i->Is<CallInstr>())
            {
                ArchInfo().leaf_ = false;
                break;
            }

    for (auto bb : *func)
        for (auto i : *bb)
            if (i->Is<AllocaInstr>())
                VisitAllocaInstr(i->As<AllocaInstr>());

    for (auto bb : *func)
        VisitBasicBlock(bb);

    // we need to guarantee that info.allocated_ is always a multiple of 16
    auto& info = ArchInfo();
    info.allocated_ = MakeAlign(info.allocated_, 16);
    if (!info.leaf_)
        // eight more bytes for the return address.
        info.allocated_ += 8;
}


void SimpleAlloc::VisitBasicBlock(BasicBlock* bb)
{
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

    auto offset = AllocateOnX64Stack(ArchInfo(), size, align);
    stackcache_.Map2Stack(instr->Result()->As<Register>(), offset);
}


void SimpleAlloc::VisitLoadInstr(LoadInstr* instr)
{
    auto ptr = MapConstAndGlobalVar(instr->Pointer());
    auto ptrreg = instr->Pointer()->As<Register>();
    if (!ptr) stackcache_.AccessStack(ptrreg);

    Allocate(ptrreg);
}


void SimpleAlloc::VisitStoreInstr(StoreInstr* instr)
{
    auto value = MapConstAndGlobalVar(instr->Value());
    auto dest = MapConstAndGlobalVar(instr->Dest());

    auto valreg = instr->Value()->As<Register>();
    auto destreg = instr->Dest()->As<Register>();
    if (!dest) Allocate(destreg);
    if (!value) Allocate(valreg);
}
