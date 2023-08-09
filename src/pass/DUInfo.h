#ifndef _DU_INFO_H_
#define _DU_INFO_H_

#include "pass/Pass.h"
#include "visitir/IRVisitor.h"
#include <list>
#include <string>
#include <unordered_map>
#include <unordered_set>

class BinaryInstr;
class ConvertInstr;
class IROperand;
class Instr;


class DUInfo : public FunctionPass, private IRVisitor
{
public:
    DUInfo(Module* m) : FunctionPass(m) {}

    std::string PrintSummary() const override;

    void ExecuteOnFunction(Function* func) override
    {
        CurFunc() = func;
        VisitFunction(func);
    }
    void ExitFunction() override
    {
        def_.clear(); uses_.clear(); bbdef_.clear();
        bbuse_.clear(); bbphidef_.clear(); bbphiuse_.clear();
    }

    bool HasDef(const BasicBlock* bb) const { return bbdef_.count(bb); }
    bool HasPhiDef(const BasicBlock* bb) const { return bbphidef_.count(bb); }
    bool HasUse(const BasicBlock* bb) const { return bbuse_.count(bb); }
    bool HasPhiUse(const BasicBlock* bb) const { return bbphiuse_.count(bb); }

    const Instr* GetDef(const IROperand* op) const { return def_.at(op); }
    const auto& GetDef(const BasicBlock* bb) const { return bbdef_.at(bb); }
    auto GetPhiDef(const BasicBlock* bb) const { return bbphidef_.at(bb); }

    const auto& GetUse(const IROperand* op) const { return uses_.at(op); }
    const auto& GetUse(const BasicBlock* bb) const { return bbuse_.at(bb); }
    const auto& GetPhiUse(const BasicBlock* bb) const { return bbphiuse_.at(bb); }
    bool IsLastUse(const IROperand* op, const Instr* i) const { return uses_.at(op).back() == i; }

    void AddDef(const IROperand* op, const Instr* i) { def_.emplace(op, i); }
    void AddDef(const BasicBlock* bb, const IROperand* op) { bbdef_[bb].insert(op); }
    void AddPhiDef(const BasicBlock* bb, const IROperand* op) { bbphidef_[bb].insert(op); }

    void AddUse(const IROperand* op, const Instr* i) { uses_[op].push_back(i); }
    void AddUse(const BasicBlock* bb, const IROperand* op) { bbuse_[bb].insert(op); }
    void AddPhiUse(const BasicBlock* bb, const IROperand* op) { bbphiuse_[bb].push_back(op); }

    void DelDef(const IROperand* op) { def_.erase(op); }
    void DelDef(const BasicBlock* bb) { bbdef_.erase(bb); }
    void DelDef(const BasicBlock* bb, const IROperand* op) { bbdef_.at(bb).erase(op); }
    void DelPhiDef(const BasicBlock* bb) { bbphidef_.erase(bb); }

    void DelUse(const IROperand* op) { uses_.erase(op); }
    void DelUse(const IROperand* op, const Instr* i) { uses_.at(op).remove(i); }
    void DelUse(const BasicBlock* bb) { bbuse_.erase(bb); }
    void DelUse(const BasicBlock* bb, const IROperand* op) { bbuse_.at(bb).erase(op); }
    void DelPhiUse(const BasicBlock* bb, const IROperand* op) { bbphiuse_.at(bb).remove(op); }

private:
    void BinaryDUHelper(BinaryInstr*);
    void ConvertDUHelper(ConvertInstr*);

    const BasicBlock* curbb_{};

    std::unordered_map<const IROperand*, const Instr*> def_{};
    std::unordered_map<const IROperand*, std::list<const Instr*>> uses_{};

    // variables defined (without phi) in basic blocks
    std::unordered_map<const BasicBlock*, std::unordered_set<const IROperand*>> bbdef_{};
    // variables used (without phi) in basic blocks
    std::unordered_map<const BasicBlock*, std::unordered_set<const IROperand*>> bbuse_{};
    // variables defined by a phi instruction;
    std::unordered_map<const BasicBlock*, std::unordered_set<const IROperand*>> bbphidef_{};
    // variables used in phi instructions; use list to save some memory,
    // since there's no need to keep the elements unique
    std::unordered_map<const BasicBlock*, std::list<const IROperand*>> bbphiuse_{};

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

#endif // _DU_INFO_H_
