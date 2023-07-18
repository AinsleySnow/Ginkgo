#include "pass/DUChains.h"
#include "IR/Instr.h"
#include "IR/IROperand.h"
#include "IR/Value.h"


void DUChains::BinaryDUHelper(BinaryInstr* bin)
{
    def_.emplace(bin->Result(), bin);
    if (bin->Lhs()->Is<Register>())
        uses_.emplace(bin->Lhs(), bin);
    if (bin->Rhs()->Is<Register>())
        uses_.emplace(bin->Rhs(), bin);
}

void DUChains::ConvertDUHelper(ConvertInstr* cvt)
{
    def_.emplace(cvt->Dest(), cvt);
    uses_.emplace(cvt->Value(), cvt);
}


void DUChains::VisitFunction(Function* func)
{
    for (auto b : *func)
        VisitBasicBlock(b);
}

void DUChains::VisitBasicBlock(BasicBlock* b)
{
    for (auto i : *b)
        i->Accept(this);
}


void DUChains::VisitRetInstr(RetInstr* ret)
{
    if (ret->ReturnValue() && ret->ReturnValue()->Is<Register>())
        uses_.emplace(ret->ReturnValue(), ret);
}

void DUChains::VisitBrInstr(BrInstr* br)
{
    if (br->Cond() && br->Cond()->Is<Register>())
        uses_.emplace(br->Cond(), br);
}

void DUChains::VisitSwitchInstr(SwitchInstr* swtch)
{
    if (swtch->GetIdent()->Is<Register>())
        uses_.emplace(swtch->GetIdent(), swtch);
}

void DUChains::VisitCallInstr(CallInstr* call)
{
    if (call->Result())
        def_.emplace(call->Result(), call);
    for (auto op : call->ArgvList())
        if (op->Is<Register>())
            uses_.emplace(op, call);
}

void DUChains::VisitAddInstr(AddInstr* i)   { BinaryDUHelper(i); }
void DUChains::VisitFaddInstr(FaddInstr* i) { BinaryDUHelper(i); }
void DUChains::VisitSubInstr(SubInstr* i)   { BinaryDUHelper(i); }
void DUChains::VisitFsubInstr(FsubInstr* i) { BinaryDUHelper(i); }
void DUChains::VisitMulInstr(MulInstr* i)   { BinaryDUHelper(i); }
void DUChains::VisitFmulInstr(FmulInstr* i) { BinaryDUHelper(i); }
void DUChains::VisitDivInstr(DivInstr* i)   { BinaryDUHelper(i); }
void DUChains::VisitFdivInstr(FdivInstr* i) { BinaryDUHelper(i); }
void DUChains::VisitModInstr(ModInstr* i)   { BinaryDUHelper(i); }
void DUChains::VisitShlInstr(ShlInstr* i)   { BinaryDUHelper(i); }
void DUChains::VisitLshrInstr(LshrInstr* i) { BinaryDUHelper(i); }
void DUChains::VisitAshrInstr(AshrInstr* i) { BinaryDUHelper(i); }
void DUChains::VisitAndInstr(AndInstr* i)   { BinaryDUHelper(i); }
void DUChains::VisitOrInstr(OrInstr* i)     { BinaryDUHelper(i); }
void DUChains::VisitXorInstr(XorInstr* i)   { BinaryDUHelper(i); }

void DUChains::VisitAllocaInstr(AllocaInstr* alloca)
{
    def_.emplace(alloca->Result(), alloca);
}

void DUChains::VisitLoadInstr(LoadInstr* load)
{
    def_.emplace(load->Result(), load);
    uses_.emplace(load->Pointer(), load);
}

void DUChains::VisitStoreInstr(StoreInstr* store)
{
    if (store->Value()->Is<Register>())
        uses_.emplace(store->Value(), store);
    uses_.emplace(store->Dest(), store);
}

void DUChains::VisitGetElePtrInstr(GetElePtrInstr* gep)
{
    def_.emplace(gep->Result(), gep);
    if (!gep->HoldsInt() && gep->OpIndex()->Is<Register>())
        uses_.emplace(gep->OpIndex(), gep);
    uses_.emplace(gep->OpIndex(), gep);
}

void DUChains::VisitTruncInstr(TruncInstr* i)       { ConvertDUHelper(i); }
void DUChains::VisitFtruncInstr(FtruncInstr* i)     { ConvertDUHelper(i); }
void DUChains::VisitZextInstr(ZextInstr* i)         { ConvertDUHelper(i); }
void DUChains::VisitSextInstr(SextInstr* i)         { ConvertDUHelper(i); }
void DUChains::VisitFextInstr(FextInstr* i)         { ConvertDUHelper(i); }
void DUChains::VisitFtoUInstr(FtoUInstr* i)         { ConvertDUHelper(i); }
void DUChains::VisitFtoSInstr(FtoSInstr* i)         { ConvertDUHelper(i); }
void DUChains::VisitUtoFInstr(UtoFInstr* i)         { ConvertDUHelper(i); }
void DUChains::VisitStoFInstr(StoFInstr* i)         { ConvertDUHelper(i); }
void DUChains::VisitPtrtoIInstr(PtrtoIInstr* i)     { ConvertDUHelper(i); }
void DUChains::VisitItoPtrInstr(ItoPtrInstr* i)     { ConvertDUHelper(i); }
void DUChains::VisitBitcastInstr(BitcastInstr* i)   { ConvertDUHelper(i); }

#define CMP_HELPER                  \
def_.emplace(cmp->Result(), cmp);   \
if (cmp->Op1()->Is<Register>())     \
    uses_.emplace(cmp->Op1(), cmp); \
if (cmp->Op2()->Is<Register>())     \
    uses_.emplace(cmp->Op2(), cmp)

void DUChains::VisitIcmpInstr(IcmpInstr* cmp) { CMP_HELPER; }
void DUChains::VisitFcmpInstr(FcmpInstr* cmp) { CMP_HELPER; }

void DUChains::VisitSelectInstr(SelectInstr* sel)
{
    def_.emplace(sel->Result(), sel);
    if (sel->Value1()->Is<Register>())
        uses_.emplace(sel->Value1(), sel);
    if (sel->Value2()->Is<Register>())
        uses_.emplace(sel->Value2(), sel);
}

void DUChains::VisitPhiInstr(PhiInstr* phi)
{
    def_.emplace(phi->Result(), phi);
    for (auto [_, op] : phi->GetBlockValPair())
        if (op->Is<Register>())
            uses_.emplace(op, phi);
}
