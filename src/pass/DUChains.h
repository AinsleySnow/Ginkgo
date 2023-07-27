#ifndef _DU_CHAIN_H_
#define _DU_CHAIN_H_

#include "pass/Pass.h"
#include "visitir/IRVisitor.h"
#include <list>
#include <unordered_map>

class BinaryInstr;
class ConvertInstr;
class IROperand;
class Instr;


class DUChains : public FunctionPass, private IRVisitor
{
public:
    DUChains(Module* m) : FunctionPass(m) {}

    void ExecuteOnFunction(Function* func) override { VisitFunction(func); }
    void ExitFunction(Function*) override {}

    const Instr* GetDef(const IROperand* op) const { return def_.at(op); }
    const auto& GetUse(const IROperand* op) const { return uses_.at(op); }
    bool IsLastUse(const IROperand* op, const Instr* i) const { return uses_.at(op).back() == i; }

    void AddDef(const IROperand* op, const Instr* i) { def_.emplace(op, i); }
    void AddUse(const IROperand* op, const Instr* i) { uses_[op].push_back(i); }
    void DelDef(const IROperand* op) { def_.erase(op); }
    void DelUse(const IROperand* op) { uses_.erase(op); }
    void DelUse(const IROperand* op, const Instr* i) { uses_.at(op).remove(i); }

private:
    void BinaryDUHelper(BinaryInstr*);
    void ConvertDUHelper(ConvertInstr*);

    std::unordered_map<const IROperand*, const Instr*> def_{};
    std::unordered_map<const IROperand*, std::list<const Instr*>> uses_{};

private:
    void VisitFunction(Function*) override;
    void VisitBasicBlock(BasicBlock*) override;

    void VisitRetInstr(RetInstr*) override;
    void VisitBrInstr(BrInstr*) override;
    void VisitSwitchInstr(SwitchInstr*) override;
    void VisitCallInstr(CallInstr*) override;

    void VisitAddInstr(AddInstr*) override;
    void VisitFaddInstr(FaddInstr*) override;
    void VisitSubInstr(SubInstr*) override;
    void VisitFsubInstr(FsubInstr*) override;
    void VisitMulInstr(MulInstr*) override;
    void VisitFmulInstr(FmulInstr*) override;
    void VisitDivInstr(DivInstr*) override;
    void VisitFdivInstr(FdivInstr*) override;
    void VisitModInstr(ModInstr*) override;
    void VisitShlInstr(ShlInstr*) override;
    void VisitLshrInstr(LshrInstr*) override;
    void VisitAshrInstr(AshrInstr*) override;
    void VisitAndInstr(AndInstr*) override;
    void VisitOrInstr(OrInstr*) override;
    void VisitXorInstr(XorInstr*) override;

    void VisitAllocaInstr(AllocaInstr*) override;
    void VisitLoadInstr(LoadInstr*) override;
    void VisitStoreInstr(StoreInstr*) override;

    // void VisitGetValInstr(GetValInstr*) override;
    // void VisitSetValInstr(SetValInstr*) override;
    void VisitGetElePtrInstr(GetElePtrInstr*) override;

    void VisitTruncInstr(TruncInstr*) override;
    void VisitFtruncInstr(FtruncInstr*) override;

    void VisitZextInstr(ZextInstr*) override;
    void VisitSextInstr(SextInstr*) override;
    void VisitFextInstr(FextInstr*) override;
    void VisitFtoUInstr(FtoUInstr*) override;
    void VisitFtoSInstr(FtoSInstr*) override;

    void VisitUtoFInstr(UtoFInstr*) override;
    void VisitStoFInstr(StoFInstr*) override;
    void VisitPtrtoIInstr(PtrtoIInstr*) override;
    void VisitItoPtrInstr(ItoPtrInstr*) override;
    void VisitBitcastInstr(BitcastInstr*) override;

    void VisitIcmpInstr(IcmpInstr*) override;
    void VisitFcmpInstr(FcmpInstr*) override;
    void VisitSelectInstr(SelectInstr*) override;
    void VisitPhiInstr(PhiInstr*) override;
};

#endif // _DU_CHAIN_H_
