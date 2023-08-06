#include "pass/SimpleAlloc.h"
#include "IR/Value.h"
#include "IR/Instr.h"


RegTag SimpleAlloc::SpareHelper(RegList& v, const Register* r) const
{
    for (auto& pair : v)
    {
        if (!pair.second)
        {
            pair.second = r;
            return pair.first;
        }
    }
    return RegTag::none;
}

RegTag SimpleAlloc::SpareReg(const Register* r) const { return SpareHelper(intreg_, r); }
RegTag SimpleAlloc::SpareFReg(const Register* r) const { return SpareHelper(vecreg_, r); }


void SimpleAlloc::Map2Reg(const Register* reg, RegTag tag)
{
    auto px64 = std::make_unique<x64Reg>(tag, reg->Type()->Size());
    auto raw = px64.get();
    MapRegister(reg, std::move(px64));
}

void SimpleAlloc::Map2Stack(const Register* reg, long offset)
{
    Map2Stack(reg, reg->Type()->Size(), offset);
}

void SimpleAlloc::Map2Stack(const Register* reg, size_t size, long offset)
{
    auto px64 = std::make_unique<x64Mem>(size, offset, RegTag::rbp, RegTag::none, 0);
    auto raw = px64.get();
    MapRegister(reg, std::move(px64));
}


void SimpleAlloc::Allocate(const Register* reg)
{
    auto tag = reg->Type()->Is<FloatType>() ?
        SpareFReg(reg) : SpareReg(reg);

    if (tag != RegTag::none)
    {
        Map2Reg(reg, tag);
        Mark(static_cast<x64Phys>(static_cast<int>(tag) - 2));
    }
    else
    {
        auto offset = AllocateOnX64Stack(
            ArchInfo(), reg->Type()->Size(), reg->Type()->Size());
        Map2Stack(reg, offset);
    }
}


long SimpleAlloc::AllocateOnX64Stack(x64Stack& info, size_t size, size_t align)
{
    info.allocated_ = MakeAlign(info.allocated_, align);
    info.allocated_ += size;
    long base = -info.allocated_;
    info.rspoffset_ = info.allocated_;
    return base;
}


void SimpleAlloc::Access(
    const Register* reg, const BasicBlock* bb, const Instr* i)
{
    RegList& v = intreg_;
    if (reg->Type()->Is<FloatType>())
        v = vecreg_;

    auto iter = v.begin();
    for (; iter != v.end(); iter++)
        if (iter->second == reg)
            goto find;
    return;

find:
    // live-in at some successor of bb as well
    if (live_->LiveOutAt(reg, bb))
        return;
    // live-in at bb but not at its successor,
    // and no other instruction uses reg
    if (info_->IsLastUse(reg, i))
    {
        iter->second = nullptr;
        Unmark(static_cast<x64Phys>(static_cast<int>(iter->first) - 2));
    }
}


void SimpleAlloc::BinaryAllocaHelper(BinaryInstr* i)
{
    if (!MapConstAndGlobalVar(i->Lhs()))
        Access(i->Lhs()->As<Register>(), curbb_, i);
    if (!MapConstAndGlobalVar(i->Rhs()))
        Access(i->Rhs()->As<Register>(), curbb_, i);
    Allocate(i->Result()->As<Register>());
}

void SimpleAlloc::ConvertAllocaHelper(ConvertInstr* i)
{
    if (!MapConstAndGlobalVar(i->Value()))
        Access(i->Value()->As<Register>(), curbb_, i);
    Allocate(i->Dest()->As<Register>());
}

void SimpleAlloc::ResetRegList()
{
    for (auto& pair : intreg_)
        pair.second = nullptr;
    for (auto& pair : vecreg_)
        pair.second = nullptr;
}


void SimpleAlloc::VisitFunction(Function* func)
{
    ResetRegList();

    for (auto bb : *func)
        for (auto i : *bb)
            if (i->Is<AllocaInstr>())
                VisitAllocaInstr(i->As<AllocaInstr>());
    LoadParam();

    for (auto bb : *func)
        VisitBasicBlock(bb);

    // we need to guarantee that info.allocated_ is always a multiple of 16
    auto& info = ArchInfo();
    info.allocated_ = MakeAlign(info.allocated_, 16);
    info.rspoffset_ = info.allocated_;
}


#define CLEAN_UNUSED(v)                                             \
for (int i = 0; i < v.size(); ++i)                                  \
{                                                                   \
    /* Not live-in at bb? That means the mapped */                  \
    if (auto reg = v[i].second; reg && !live_->LiveInAt(reg, bb))   \
    {                                                               \
        auto iter = v.begin() + i;                                  \
        iter->second = nullptr;                                     \
    }                                                               \
}

void SimpleAlloc::VisitBasicBlock(BasicBlock* bb)
{
    CLEAN_UNUSED(intreg_);
    CLEAN_UNUSED(vecreg_);

    curbb_ = bb;
    for (auto i : *bb)
        if (!i->Is<AllocaInstr>())
            i->Accept(this);
}


void SimpleAlloc::VisitRetInstr(RetInstr* i)
{
    if (!i->ReturnValue())
        return;
    if (!MapConstAndGlobalVar(i->ReturnValue()))
        Access(i->ReturnValue()->As<Register>(), curbb_, i);
}

void SimpleAlloc::VisitBrInstr(BrInstr* i)
{
    if (!i->Cond()) return;
    if (!MapConstAndGlobalVar(i->Cond()))
        Access(i->Cond()->As<Register>(), curbb_, i);
}

void SimpleAlloc::VisitSwitchInstr(SwitchInstr* i)
{
    if (!MapConstAndGlobalVar(i->GetIdent()))
        Access(i->GetIdent()->As<Register>(), curbb_, i);
    for (auto [tag, _] : i->GetValueBlkPairs())
        MapConstAndGlobalVar(tag);
}

void SimpleAlloc::VisitCallInstr(CallInstr* i)
{
    for (auto op : i->ArgvList())
        if (!MapConstAndGlobalVar(op))
            Access(op->As<Register>(), curbb_, i);
    if (i->FuncAddr()) // Call through a function pointer?
        Access(i->FuncAddr()->As<Register>(), curbb_, i);
    if (i->Result())
        Allocate(i->Result()->As<Register>());
}


void SimpleAlloc::VisitAddInstr(AddInstr* i) { BinaryAllocaHelper(i); }
void SimpleAlloc::VisitFaddInstr(FaddInstr* i) { BinaryAllocaHelper(i); }
void SimpleAlloc::VisitSubInstr(SubInstr* i) { BinaryAllocaHelper(i); }
void SimpleAlloc::VisitFsubInstr(FsubInstr* i) { BinaryAllocaHelper(i); }
void SimpleAlloc::VisitMulInstr(MulInstr* i) { BinaryAllocaHelper(i); }
void SimpleAlloc::VisitFmulInstr(FmulInstr* i) { BinaryAllocaHelper(i); }
void SimpleAlloc::VisitDivInstr(DivInstr* i) { BinaryAllocaHelper(i); }
void SimpleAlloc::VisitFdivInstr(FdivInstr* i) { BinaryAllocaHelper(i); }
void SimpleAlloc::VisitModInstr(ModInstr* i) { BinaryAllocaHelper(i); }
void SimpleAlloc::VisitShlInstr(ShlInstr* i) { BinaryAllocaHelper(i); }
void SimpleAlloc::VisitLshrInstr(LshrInstr* i) { BinaryAllocaHelper(i); }
void SimpleAlloc::VisitAshrInstr(AshrInstr* i) { BinaryAllocaHelper(i); }
void SimpleAlloc::VisitAndInstr(AndInstr* i) { BinaryAllocaHelper(i); }
void SimpleAlloc::VisitOrInstr(OrInstr* i) { BinaryAllocaHelper(i); }
void SimpleAlloc::VisitXorInstr(XorInstr* i) { BinaryAllocaHelper(i); }


void SimpleAlloc::VisitAllocaInstr(AllocaInstr* i)
{
    auto size = i->Type()->Size();
    auto align = i->Type()->Align();
    auto extra = align > size ? align - size : 0;

    // FIXME:
    // The case where align > 16 is very tricky. Say, k with int
    // type is aligned as 128. Although at the beginning of a function,
    // rsp is always a multiple of 16, there's no guarantee that
    // rsp will be a multiple of the desired big alignment. Thus,
    // it's hard to identify the offsets of variables using the simple
    // method below. I'v like to simply ignore the issue for the time being.

    auto offset = AllocateOnX64Stack(ArchInfo(), size + extra, align);
    Map2Stack(i->Result()->As<Register>(), size, offset);
}

void SimpleAlloc::VisitLoadInstr(LoadInstr* i)
{
    if (!MapConstAndGlobalVar(i->Pointer()))
        Access(i->Pointer()->As<Register>(), curbb_, i);
    Allocate(i->Result()->As<Register>());
    if (i->Result()->Type()->Is<PtrType>())
        MarkLoadTwice(i->Result());
}

void SimpleAlloc::VisitStoreInstr(StoreInstr* i)
{
    if (!MapConstAndGlobalVar(i->Dest()))
        Access(i->Dest()->As<Register>(), curbb_, i);
    if (!MapConstAndGlobalVar(i->Value()))
        Access(i->Value()->As<Register>(), curbb_, i);
}

void SimpleAlloc::VisitGetElePtrInstr(GetElePtrInstr* i)
{
    if (!MapConstAndGlobalVar(i->OpIndex()))
        Access(i->OpIndex()->As<Register>(), curbb_, i);
    if (!MapConstAndGlobalVar(i->Pointer()))
        Access(i->Pointer()->As<Register>(), curbb_, i);
    Allocate(i->Result()->As<Register>());
    MarkLoadTwice(i->Result());
}


void SimpleAlloc::VisitTruncInstr(TruncInstr* i) { ConvertAllocaHelper(i); }
void SimpleAlloc::VisitFtruncInstr(FtruncInstr* i) { ConvertAllocaHelper(i); }

void SimpleAlloc::VisitZextInstr(ZextInstr* i) { ConvertAllocaHelper(i); }
void SimpleAlloc::VisitSextInstr(SextInstr* i) { ConvertAllocaHelper(i); }
void SimpleAlloc::VisitFextInstr(FextInstr* i) { ConvertAllocaHelper(i); }
void SimpleAlloc::VisitFtoUInstr(FtoUInstr* i) { ConvertAllocaHelper(i); }
void SimpleAlloc::VisitFtoSInstr(FtoSInstr* i) { ConvertAllocaHelper(i); }

void SimpleAlloc::VisitUtoFInstr(UtoFInstr* i) { ConvertAllocaHelper(i); }
void SimpleAlloc::VisitStoFInstr(StoFInstr* i) { ConvertAllocaHelper(i); }
void SimpleAlloc::VisitPtrtoIInstr(PtrtoIInstr* i) { ConvertAllocaHelper(i); }

void SimpleAlloc::VisitItoPtrInstr(ItoPtrInstr* i)
{
    ConvertAllocaHelper(i);
    MarkLoadTwice(i->Dest());
}

void SimpleAlloc::VisitBitcastInstr(BitcastInstr* i)
{
    ConvertAllocaHelper(i);
    if (auto mapped = GetIROpMap(i->Value())->As<x64Mem>();
        mapped && mapped->LoadTwice())
        MarkLoadTwice(i->Dest());
}


#define CMP_HELPER                                  \
if (!MapConstAndGlobalVar(i->Op1()))                \
    Access(i->Op1()->As<Register>(), curbb_, i);    \
if (!MapConstAndGlobalVar(i->Op2()))                \
    Access(i->Op2()->As<Register>(), curbb_, i);    \
Allocate(i->Result())

void SimpleAlloc::VisitIcmpInstr(IcmpInstr* i) { CMP_HELPER; }
void SimpleAlloc::VisitFcmpInstr(FcmpInstr* i) { CMP_HELPER; }

#undef CMP_HELPER

void SimpleAlloc::VisitSelectInstr(SelectInstr* i)
{
    if (!MapConstAndGlobalVar(i->SelType()))
        Access(i->SelType()->As<Register>(), curbb_, i);
    if (!MapConstAndGlobalVar(i->Value1()))
        Access(i->Value1()->As<Register>(), curbb_, i);
    if (!MapConstAndGlobalVar(i->Value2()))
        Access(i->Value2()->As<Register>(), curbb_, i);
    Allocate(i->Result());
}

void SimpleAlloc::VisitPhiInstr(PhiInstr* i)
{
    for (auto [_, op] : i->GetBlockValPair())
        if (!MapConstAndGlobalVar(op))
            Access(op->As<Register>(), curbb_, i);
    Allocate(i->Result());
}
