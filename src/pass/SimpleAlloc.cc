#include "pass/SimpleAlloc.h"
#include "IR/Value.h"


const x64* SimpleAlloc::IRTox64(const IROperand* op)
{
    if (op->Is<Constant>())
        return x64Imm::CreateX64Imm(curbb_, op->As<Constant>());

    auto reg = op->As<Register>();
    if (reg->Name()[0] == '@')
        return x64Mem::CreateX64Mem(curbb_, reg->Name().substr(1));

    if (addrmap_.find(reg) != addrmap_.end())
        return addrmap_[reg];
    return nullptr;
}


int SimpleAlloc::Allocate(BasicBlock* bb, const Register* reg)
{
    auto index = index_ % 3;

    RegTag tag = RegTag::none;
    if (index == 0) tag = RegTag::rax;
    else if (index == 1) tag = RegTag::rdx;
    else if (index == 2) tag = RegTag::rcx;

    scache_[index] = x64Reg::CreateX64Reg(bb, tag);
    addrmap_[reg] = scache_[index];

    index_++;
    return index;
}

int SimpleAlloc::FAllocate(BasicBlock* bb, const Register* reg)
{
    auto index = index_ % 3;

    RegTag tag = RegTag::none;
    if (index == 0) tag = RegTag::xmm0;
    else if (index == 1) tag = RegTag::xmm1;
    else if (index == 2) tag = RegTag::xmm2;

    fscache_[index] = x64Reg::CreateX64Reg(bb, tag);
    addrmap_[reg] = fscache_[index];

    index_++;
    return index;
}


void SimpleAlloc::BinaryAllocaHelper(BinaryInstr* instr)
{
    auto lhs = IRTox64(instr->Lhs());
    if (!lhs) lhs = scache_[Allocate(curbb_, instr->Lhs()->As<Register>())];
    auto rhs = IRTox64(instr->Rhs());
    if (!rhs) rhs = scache_[Allocate(curbb_, instr->Rhs()->As<Register>())];

    index_ -= 2;
    instr->Lhs() = lhs;
    instr->Rhs() = rhs;
    instr->Result() = scache_[Allocate(curbb_, instr->Result()->As<Register>())];
}

void SimpleAlloc::FBinaryAllocaHelper(BinaryInstr* instr)
{
    auto lhs = IRTox64(instr->Lhs());
    if (!lhs) lhs = fscache_[FAllocate(curbb_, instr->Lhs()->As<Register>())];
    auto rhs = IRTox64(instr->Rhs());
    if (!rhs) rhs = fscache_[FAllocate(curbb_, instr->Rhs()->As<Register>())];

    index_ -= 2;
    instr->Lhs() = lhs;
    instr->Rhs() = rhs;
    instr->Result() = fscache_[FAllocate(curbb_, instr->Result()->As<Register>())];
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
void SimpleAlloc::VisitFaddInstr(FaddInstr* instr) { FBinaryAllocaHelper(instr); }
void SimpleAlloc::VisitSubInstr(SubInstr* instr) { BinaryAllocaHelper(instr); }
void SimpleAlloc::VisitFsubInstr(FsubInstr* instr) { FBinaryAllocaHelper(instr); }
void SimpleAlloc::VisitMulInstr(MulInstr* instr) { BinaryAllocaHelper(instr); }
void SimpleAlloc::VisitFmulInstr(FmulInstr* instr) { FBinaryAllocaHelper(instr); }
void SimpleAlloc::VisitDivInstr(DivInstr* instr) { BinaryAllocaHelper(instr); }
void SimpleAlloc::VisitFdivInstr(FdivInstr* instr) { FBinaryAllocaHelper(instr); }
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

    // not a leaf; just use allocated_ as the offset.
    if (!info.leaf_)
    {
        info.allocated_ = MakeAlign(info.allocated_, align);
        info.offset_[instr->Result()] = info.allocated_;
        return;
    }

    // leaf; use both red zone and zone above rsp.
    if (info.belowrsp_ + size <= 128)
    {
        // meet strict alignment requirements.
        info.belowrsp_ = MakeAlign(info.belowrsp_, align);
        info.offset_[instr->Result()] = -info.belowrsp_;
        info.belowrsp_ += size;
    }
    else
    {
        info.offset_[instr->Result()] = info.allocated_;
        info.allocated_ += size;
    }
}


void SimpleAlloc::VisitLoadInstr(LoadInstr* instr)
{
    auto loc = IRTox64(instr->Pointer());
    auto dest = instr->Result()->As<Register>();
    if (loc)
    {
        // must be x64Mem here.
        addrmap_[dest] = loc->As<x64Mem>();
        return;
    }

    auto offset = InfoAt(curfunc_).offset_[instr->Pointer()->As<Register>()];
    auto rsp = x64Reg::CreateX64Reg(curbb_, RegTag::rsp);
    addrmap_[dest] = x64Mem::CreateX64Mem(curbb_, offset, rsp);
}


void SimpleAlloc::VisitStoreInstr(StoreInstr* instr)
{
    instr->Value() = IRTox64(instr->Value());
    instr->Dest() = IRTox64(instr->Dest());
}
