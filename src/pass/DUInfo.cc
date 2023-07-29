#include "pass/DUInfo.h"
#include "IR/Instr.h"
#include "IR/IROperand.h"
#include "IR/Value.h"


void DUInfo::BinaryDUHelper(BinaryInstr* bin)
{
    AddDef(bin->Result(), bin);
    if (bin->Lhs()->Is<Register>())
        AddUse(bin->Lhs(), bin);
    if (bin->Rhs()->Is<Register>())
        AddUse(bin->Rhs(), bin);
}

void DUInfo::ConvertDUHelper(ConvertInstr* cvt)
{
    AddDef(cvt->Dest(), cvt);
    AddUse(cvt->Value(), cvt);
}


void DUInfo::VisitFunction(Function* func)
{
    for (auto b : *func)
        VisitBasicBlock(b);
}

void DUInfo::VisitBasicBlock(BasicBlock* b)
{
    for (auto i : *b)
        i->Accept(this);
}


void DUInfo::VisitRetInstr(RetInstr* ret)
{
    if (ret->ReturnValue() && ret->ReturnValue()->Is<Register>())
        AddUse(ret->ReturnValue(), ret);
}

void DUInfo::VisitBrInstr(BrInstr* br)
{
    if (br->Cond() && br->Cond()->Is<Register>())
        AddUse(br->Cond(), br);
}

void DUInfo::VisitSwitchInstr(SwitchInstr* swtch)
{
    if (swtch->GetIdent()->Is<Register>())
        AddUse(swtch->GetIdent(), swtch);
}

void DUInfo::VisitCallInstr(CallInstr* call)
{
    if (call->Result())
        AddDef(call->Result(), call);
    for (auto op : call->ArgvList())
        if (op->Is<Register>())
            AddUse(op, call);
}

void DUInfo::VisitAddInstr(AddInstr* i)   { BinaryDUHelper(i); }
void DUInfo::VisitFaddInstr(FaddInstr* i) { BinaryDUHelper(i); }
void DUInfo::VisitSubInstr(SubInstr* i)   { BinaryDUHelper(i); }
void DUInfo::VisitFsubInstr(FsubInstr* i) { BinaryDUHelper(i); }
void DUInfo::VisitMulInstr(MulInstr* i)   { BinaryDUHelper(i); }
void DUInfo::VisitFmulInstr(FmulInstr* i) { BinaryDUHelper(i); }
void DUInfo::VisitDivInstr(DivInstr* i)   { BinaryDUHelper(i); }
void DUInfo::VisitFdivInstr(FdivInstr* i) { BinaryDUHelper(i); }
void DUInfo::VisitModInstr(ModInstr* i)   { BinaryDUHelper(i); }
void DUInfo::VisitShlInstr(ShlInstr* i)   { BinaryDUHelper(i); }
void DUInfo::VisitLshrInstr(LshrInstr* i) { BinaryDUHelper(i); }
void DUInfo::VisitAshrInstr(AshrInstr* i) { BinaryDUHelper(i); }
void DUInfo::VisitAndInstr(AndInstr* i)   { BinaryDUHelper(i); }
void DUInfo::VisitOrInstr(OrInstr* i)     { BinaryDUHelper(i); }
void DUInfo::VisitXorInstr(XorInstr* i)   { BinaryDUHelper(i); }

void DUInfo::VisitAllocaInstr(AllocaInstr* alloca)
{
    AddDef(alloca->Result(), alloca);
}

void DUInfo::VisitLoadInstr(LoadInstr* load)
{
    AddDef(load->Result(), load);
    AddUse(load->Pointer(), load);
}

void DUInfo::VisitStoreInstr(StoreInstr* store)
{
    if (store->Value()->Is<Register>())
        AddUse(store->Value(), store);
    AddUse(store->Dest(), store);
}

void DUInfo::VisitGetElePtrInstr(GetElePtrInstr* gep)
{
    AddDef(gep->Result(), gep);
    if (!gep->HoldsInt() && gep->OpIndex()->Is<Register>())
        AddUse(gep->OpIndex(), gep);
    AddUse(gep->OpIndex(), gep);
}

void DUInfo::VisitTruncInstr(TruncInstr* i)       { ConvertDUHelper(i); }
void DUInfo::VisitFtruncInstr(FtruncInstr* i)     { ConvertDUHelper(i); }
void DUInfo::VisitZextInstr(ZextInstr* i)         { ConvertDUHelper(i); }
void DUInfo::VisitSextInstr(SextInstr* i)         { ConvertDUHelper(i); }
void DUInfo::VisitFextInstr(FextInstr* i)         { ConvertDUHelper(i); }
void DUInfo::VisitFtoUInstr(FtoUInstr* i)         { ConvertDUHelper(i); }
void DUInfo::VisitFtoSInstr(FtoSInstr* i)         { ConvertDUHelper(i); }
void DUInfo::VisitUtoFInstr(UtoFInstr* i)         { ConvertDUHelper(i); }
void DUInfo::VisitStoFInstr(StoFInstr* i)         { ConvertDUHelper(i); }
void DUInfo::VisitPtrtoIInstr(PtrtoIInstr* i)     { ConvertDUHelper(i); }
void DUInfo::VisitItoPtrInstr(ItoPtrInstr* i)     { ConvertDUHelper(i); }
void DUInfo::VisitBitcastInstr(BitcastInstr* i)   { ConvertDUHelper(i); }

#define CMP_HELPER                  \
AddDef(cmp->Result(), cmp);         \
if (cmp->Op1()->Is<Register>())     \
    AddUse(cmp->Op1(), cmp);        \
if (cmp->Op2()->Is<Register>())     \
    AddUse(cmp->Op2(), cmp)

void DUInfo::VisitIcmpInstr(IcmpInstr* cmp) { CMP_HELPER; }
void DUInfo::VisitFcmpInstr(FcmpInstr* cmp) { CMP_HELPER; }

#undef CMP_HELPER

void DUInfo::VisitSelectInstr(SelectInstr* sel)
{
    AddDef(sel->Result(), sel);
    if (sel->SelType()->Is<Register>())
        AddUse(sel->SelType(), sel);
    if (sel->Value1()->Is<Register>())
        AddUse(sel->Value1(), sel);
    if (sel->Value2()->Is<Register>())
        AddUse(sel->Value2(), sel);
}

void DUInfo::VisitPhiInstr(PhiInstr* phi)
{
    AddDef(phi->Result(), phi);
    for (auto [_, op] : phi->GetBlockValPair())
        if (op->Is<Register>())
            AddUse(op, phi);
}
