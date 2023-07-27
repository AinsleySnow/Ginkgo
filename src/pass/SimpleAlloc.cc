#include "pass/SimpleAlloc.h"
#include "IR/Value.h"
#include "IR/Instr.h"


RegTag SimpleAlloc::StackCache::SpareReg() const
{
    if (intreg_.empty())
        return RegTag::none;
    auto tag = *intreg_.begin();
    intreg_.erase(intreg_.begin());
    return tag;
}

RegTag SimpleAlloc::StackCache::SpareFReg() const
{
    if (vecreg_.empty())
        return RegTag::none;
    auto tag = *vecreg_.begin();
    vecreg_.erase(vecreg_.begin());
    return tag;
}

void SimpleAlloc::StackCache::Access(const Register* reg, const Instr* i) const
{
    auto it = regmap_.find(reg);
    if (it == regmap_.end()) return;

    if (auto pos = it->second->As<x64Reg>(); pos && chain_->IsLastUse(reg, i))
    {
        if (static_cast<int>(pos->Tag()) <= 15)
            intreg_.emplace(pos->Tag());
        else
            vecreg_.emplace(pos->Tag());
        alloc_->Unmark(static_cast<x64Phys>(static_cast<int>(pos->Tag()) - 2));
    }
}


void SimpleAlloc::StackCache::Map2Reg(const Register* reg, RegTag tag)
{
    auto px64 = std::make_unique<x64Reg>(tag, reg->Type()->Size());
    auto raw = px64.get();
    alloc_->MapRegister(reg, std::move(px64));
    regmap_[reg] = raw;
}

void SimpleAlloc::StackCache::Map2Stack(const Register* reg, long offset)
{
    Map2Stack(reg, reg->Type()->Size(), offset);
}

void SimpleAlloc::StackCache::Map2Stack(const Register* reg, size_t size, long offset)
{
    auto px64 = std::make_unique<x64Mem>(size, offset, RegTag::rbp, RegTag::none, 0);
    auto raw = px64.get();
    alloc_->MapRegister(reg, std::move(px64));
    regmap_[reg] = raw;
}


void SimpleAlloc::Allocate(const Register* reg)
{
    auto tag = reg->Type()->Is<FloatType>() ?
        stackcache_.SpareFReg() : stackcache_.SpareReg();

    if (tag != RegTag::none)
    {
        stackcache_.Map2Reg(reg, tag);
        Mark(static_cast<x64Phys>(static_cast<int>(tag) - 2));
    }
    else
    {
        auto offset = AllocateOnX64Stack(
            ArchInfo(), reg->Type()->Size(), reg->Type()->Size());
        stackcache_.Map2Stack(reg, offset);
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


void SimpleAlloc::BinaryAllocaHelper(BinaryInstr* i)
{
    auto lhs = MapConstAndGlobalVar(i->Lhs());
    auto rhs = MapConstAndGlobalVar(i->Rhs());

    // Make sure that 'result' never share
    // the same register with 'rhs'.
    if (!lhs) stackcache_.Access(i->Lhs()->As<Register>(), i);
    Allocate(i->Result()->As<Register>());
    if (!rhs) stackcache_.Access(i->Rhs()->As<Register>(), i);
}

void SimpleAlloc::ConvertAllocaHelper(ConvertInstr* i)
{
    if (!MapConstAndGlobalVar(i->Value()))
        stackcache_.Access(i->Value()->As<Register>(), i);
    Allocate(i->Dest()->As<Register>());
}


void SimpleAlloc::VisitFunction(Function* func)
{
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
    stackcache_ = StackCache(this, chain_);
}


void SimpleAlloc::VisitBasicBlock(BasicBlock* bb)
{
    for (auto i : *bb)
        if (!i->Is<AllocaInstr>())
            i->Accept(this);
}


void SimpleAlloc::VisitRetInstr(RetInstr* i)
{
    if (!i->ReturnValue())
        return;
    if (!MapConstAndGlobalVar(i->ReturnValue()))
        stackcache_.Access(i->ReturnValue()->As<Register>(), i);
}

void SimpleAlloc::VisitBrInstr(BrInstr* i)
{
    if (!i->Cond()) return;
    if (!MapConstAndGlobalVar(i->Cond()))
        stackcache_.Access(i->Cond()->As<Register>(), i);
}

void SimpleAlloc::VisitSwitchInstr(SwitchInstr* i)
{
    if (!MapConstAndGlobalVar(i->GetIdent()))
        stackcache_.Access(i->GetIdent()->As<Register>(), i);
}

void SimpleAlloc::VisitCallInstr(CallInstr* i)
{
    for (auto op : i->ArgvList())
        if (!MapConstAndGlobalVar(op))
            stackcache_.Access(op->As<Register>(), i);
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
    // rsp is always a multiple of 16, but there's no guarantee that
    // rsp will be a multiple of the desired big alignment. Thus,
    // it's hard to identify the offsets of variables using the simple
    // method below. I'd like to simply ignore the issue for the time being.

    auto offset = AllocateOnX64Stack(ArchInfo(), size + extra, align);
    stackcache_.Map2Stack(i->Result()->As<Register>(), size, offset);
}

void SimpleAlloc::VisitLoadInstr(LoadInstr* i)
{
    if (!MapConstAndGlobalVar(i->Pointer()))
        stackcache_.Access(i->Pointer()->As<Register>(), i);
    Allocate(i->Result()->As<Register>());
    if (i->Result()->Type()->Is<PtrType>())
        MarkLoadTwice(i->Result());
}

void SimpleAlloc::VisitStoreInstr(StoreInstr* i)
{
    if (!MapConstAndGlobalVar(i->Dest()))
        stackcache_.Access(i->Dest()->As<Register>(), i);
    if (!MapConstAndGlobalVar(i->Value()))
        stackcache_.Access(i->Value()->As<Register>(), i);
}

void SimpleAlloc::VisitGetElePtrInstr(GetElePtrInstr* i)
{
    if (!MapConstAndGlobalVar(i->OpIndex()))
        stackcache_.Access(i->OpIndex()->As<Register>(), i);
    if (!MapConstAndGlobalVar(i->Pointer()))
        stackcache_.Access(i->Pointer()->As<Register>(), i);
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


#define CMP_HELPER                                      \
if (!MapConstAndGlobalVar(i->Op1()))                    \
    stackcache_.Access(i->Op1()->As<Register>(), i);    \
if (!MapConstAndGlobalVar(i->Op2()))                    \
    stackcache_.Access(i->Op2()->As<Register>(), i);    \
Allocate(i->Result())

void SimpleAlloc::VisitIcmpInstr(IcmpInstr* i) { CMP_HELPER; }
void SimpleAlloc::VisitFcmpInstr(FcmpInstr* i) { CMP_HELPER; }

#undef CMP_HELPER

void SimpleAlloc::VisitSelectInstr(SelectInstr* i)
{
    if (!MapConstAndGlobalVar(i->SelType()))
        stackcache_.Access(i->SelType()->As<Register>(), i);
    if (!MapConstAndGlobalVar(i->Value1()))
        stackcache_.Access(i->Value1()->As<Register>(), i);
    if (!MapConstAndGlobalVar(i->Value2()))
        stackcache_.Access(i->Value2()->As<Register>(), i);
    Allocate(i->Result());
}

void SimpleAlloc::VisitPhiInstr(PhiInstr* i)
{
    for (auto [_, op] : i->GetBlockValPair())
        if (!MapConstAndGlobalVar(op))
            stackcache_.Access(op->As<Register>(), i);
    Allocate(i->Result());
}
