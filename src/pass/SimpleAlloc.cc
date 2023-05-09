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

void SimpleAlloc::StackCache::Access(const Register* reg) const
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
    auto px64 = std::make_unique<x64Mem>(
        reg->Type()->Size(), offset, RegTag::rsp, RegTag::none, 0);
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

    if (!lhs) stackcache_.Access(instr->Lhs()->As<Register>());
    if (!rhs) stackcache_.Access(instr->Rhs()->As<Register>());
    Allocate(instr->Result()->As<Register>());
}

void SimpleAlloc::ConvertAllocaHelper(ConvertInstr* instr)
{
    auto value = MapConstAndGlobalVar(instr->Value());
    auto dest = MapConstAndGlobalVar(instr->Dest());

    if (!value) Allocate(instr->Value()->As<Register>());
    if (!dest) stackcache_.Access(instr->Dest()->As<Register>());
}

void SimpleAlloc::DirectCastAllocaHelper(ConvertInstr* instr)
{
    auto value = MapConstAndGlobalVar(instr->Value());
    auto dest = MapConstAndGlobalVar(instr->Dest());

    if (!value) stackcache_.Access(instr->Value()->As<Register>());
    if (!dest) Allocate(instr->Dest()->As<Register>());
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
    if (!ptr) stackcache_.Access(instr->Result()->As<Register>());
    Allocate(instr->Result()->As<Register>());
}

void SimpleAlloc::VisitStoreInstr(StoreInstr* instr)
{
    MapConstAndGlobalVar(instr->Dest());
    if (MapConstAndGlobalVar(instr->Value()))
        stackcache_.Access(instr->Value()->As<Register>());
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
void SimpleAlloc::VisitItoPtrInstr(ItoPtrInstr* i) { ConvertAllocaHelper(i); }
void SimpleAlloc::VisitBitcastInstr(BitcastInstr* i) { ConvertAllocaHelper(i); }
