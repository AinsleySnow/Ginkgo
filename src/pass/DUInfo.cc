#include "pass/DUInfo.h"
#include "IR/Instr.h"
#include "IR/IROperand.h"
#include "IR/Value.h"


void DUInfo::BinaryDUHelper(BinaryInstr* bin)
{
    AddDef(bin->Result(), bin);
    AddDef(curbb_, bin->Result());
    if (bin->Lhs()->Is<Register>())
    {
        AddUse(bin->Lhs(), bin);
        AddUse(curbb_, bin->Lhs());
    }
    if (bin->Rhs()->Is<Register>())
    {
        AddUse(bin->Rhs(), bin);
        AddUse(curbb_, bin->Rhs());
    }
}

void DUInfo::ConvertDUHelper(ConvertInstr* cvt)
{
    AddDef(cvt->Dest(), cvt);
    AddDef(curbb_, cvt->Dest());
    AddUse(cvt->Value(), cvt);
    AddUse(curbb_, cvt->Value());
}


void DUInfo::VisitFunction(Function* func)
{
    for (auto b : *func)
        VisitBasicBlock(b);
}

void DUInfo::VisitBasicBlock(BasicBlock* b)
{
    curbb_ = b;
    for (auto i : *b)
        i->Accept(this);
}


void DUInfo::VisitRetInstr(RetInstr* ret)
{
    if (ret->ReturnValue() && ret->ReturnValue()->Is<Register>())
    {
        AddUse(ret->ReturnValue(), ret);
        AddUse(curbb_, ret->ReturnValue());
    }
}

void DUInfo::VisitBrInstr(BrInstr* br)
{
    if (br->Cond() && br->Cond()->Is<Register>())
    {
        AddUse(br->Cond(), br);
        AddUse(curbb_, br->Cond());
    }
}

void DUInfo::VisitSwitchInstr(SwitchInstr* swtch)
{
    if (swtch->GetIdent()->Is<Register>())
    {
        AddUse(swtch->GetIdent(), swtch);
        AddUse(curbb_, swtch->GetIdent());
    }
}

void DUInfo::VisitCallInstr(CallInstr* call)
{
    if (call->Result())
    {
        AddDef(call->Result(), call);
        AddDef(curbb_, call->Result());
    }
    for (auto op : call->ArgvList())
    {
        if (op->Is<Register>())
        {
            AddUse(op, call);
            AddUse(curbb_, op);
        }
    }
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
    AddDef(curbb_, alloca->Result());
}

void DUInfo::VisitLoadInstr(LoadInstr* load)
{
    AddDef(load->Result(), load);
    AddDef(curbb_, load->Result());
    AddUse(load->Pointer(), load);
    AddUse(curbb_, load->Pointer());
}

void DUInfo::VisitStoreInstr(StoreInstr* store)
{
    if (store->Value()->Is<Register>())
    {
        AddUse(store->Value(), store);
        AddUse(curbb_, store->Value());
    }
    AddUse(store->Dest(), store);
    AddUse(curbb_, store->Dest());
}

void DUInfo::VisitGetElePtrInstr(GetElePtrInstr* gep)
{
    AddDef(gep->Result(), gep);
    AddDef(curbb_, gep->Result());
    if (!gep->HoldsInt() && gep->OpIndex()->Is<Register>())
    {
        AddUse(gep->OpIndex(), gep);
        AddUse(curbb_, gep->OpIndex());
    }
    AddUse(gep->Pointer(), gep);
    AddUse(curbb_, gep->Pointer());
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
AddDef(curbb_, cmp->Result());      \
if (cmp->Op1()->Is<Register>())     \
{                                   \
    AddUse(cmp->Op1(), cmp);        \
    AddUse(curbb_, cmp->Op1());     \
}                                   \
if (cmp->Op2()->Is<Register>())     \
{                                   \
    AddUse(cmp->Op2(), cmp);        \
    AddUse(curbb_, cmp->Op2());     \
}

void DUInfo::VisitIcmpInstr(IcmpInstr* cmp) { CMP_HELPER; }
void DUInfo::VisitFcmpInstr(FcmpInstr* cmp) { CMP_HELPER; }

#undef CMP_HELPER

void DUInfo::VisitSelectInstr(SelectInstr* sel)
{
    AddDef(sel->Result(), sel);
    AddDef(curbb_, sel->Result());
    if (sel->SelType()->Is<Register>())
    {
        AddUse(sel->SelType(), sel);
        AddUse(curbb_, sel->SelType());
    }
    if (sel->Value1()->Is<Register>())
    {
        AddUse(sel->Value1(), sel);
        AddUse(curbb_, sel->Value1());
    }
    if (sel->Value2()->Is<Register>())
    {
        AddUse(sel->Value2(), sel);
        AddUse(curbb_, sel->Value2());
    }
}

void DUInfo::VisitPhiInstr(PhiInstr* phi)
{
    AddDef(phi->Result(), phi);
    AddPhiDef(curbb_, phi->Result());
    for (auto [_, op] : phi->GetBlockValPair())
    {
        if (op->Is<Register>())
        {
            AddUse(op, phi);
            AddPhiUse(curbb_, op);
        }
    }
}
