#ifndef _CODE_GEN_H_
#define _CODE_GEN_H_

#include "visitir/IRVisitor.h"
#include "visitir/EmitAsm.h"
#include "pass/x64Alloc.h"
#include <cstdio>
#include <memory>
#include <string>
#include <unordered_map>

class BinaryInstr;
class Constant;
class FuncType;
class IROperand;
class x64;
enum class Condition;
enum class RegTag;


class CodeGen : public IRVisitor
{
public:
    CodeGen(x64Alloc& a) : alloc_(a) {}
    CodeGen(const std::string& f, x64Alloc& a) : asmfile_(f), alloc_(a) {}

    std::string GetAsmName() const { return asmfile_.AsmName(); }

    void VisitModule(Module*) override;
    void VisitGlobalVar(GlobalVar*) override;
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

    // registers given by alloca is directly mapped
    // to some locations; so nothing to do here

    void VisitLoadInstr(LoadInstr*) override;
    void VisitStoreInstr(StoreInstr*) override;

    // commented; since we don't have struct now
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

    void VisitIcmpInstr(IcmpInstr*) override;
    void VisitFcmpInstr(FcmpInstr*) override;
    void VisitSelectInstr(SelectInstr*) override;
    void VisitPhiInstr(PhiInstr*) override;

private:
    std::string GetLCLabel() const;
    std::string GetTempLabel() const;

    // since x64 requires that float point constants
    // must be loaded from the memory, so when the operators
    // in some instructions can be floatpoint constants,
    // use this method to get the value correctly loaded.
    // in other cases, use bare alloc_.GetIROpMap would be enough.
    const x64* MapPossibleFloat(const IROperand*);
    // sometimes an operator with pointer type maps to a stack
    // address where the address stores itself, not the value
    // it points to. MapPossiblePointer tackle with this problem.
    std::pair<const x64*, bool> MapPossiblePointer(const IROperand*); 

    void AlignRspAs(size_t);
    void AdjustRsp(long);
    void DeallocFrame();

    void PassParam(const FuncType*, const std::vector<const IROperand*>&);
    void SaveCalleeSaved();
    void RestoreCalleeSaved();
    void SaveCallerSaved();
    void RestoreCallerSaved();

    void BinaryGenHelper(const std::string&, const BinaryInstr*);
    void VarithmGenHelper(const std::string&, const BinaryInstr*);

    void LeaqEmitHelper(const x64*, const x64*);
    void MovEmitHelper(const x64*, const x64*);
    void MovzEmitHelper(const x64*, const x64*);
    void MovzEmitHelper(size_t, size_t, const x64*);
    void MovsEmitHelper(const x64*, const x64*);
    void MovsEmitHelper(size_t, size_t, const x64*);
    void VecMovEmitHelper(const x64*, const x64*);
    void VecMovEmitHelper(const x64*, RegTag);
    void VecMovEmitHelper(RegTag, const x64*);

    void VcvtEmitHelper(const x64*, const x64*);
    void VcvttEmitHelper(const x64*, const x64*);

    void PushEmitHelper(const x64*);
    void PushEmitHelper(RegTag, size_t);
    void PopEmitHelper(const x64*);
    void PopEmitHelper(RegTag, size_t);
    void PushXmmReg(RegTag);
    void PopXmmReg(RegTag);

    size_t stacksize_{};
    mutable int labelindex_{};

    std::unordered_map<
        const IROperand*, std::unique_ptr<const x64>> tempmap_{};
    std::unordered_map<double, std::string> fpconst_{};

    x64Alloc& alloc_;
    EmitAsm asmfile_{ "" };
};


class GlobalVarVisitor : public IRVisitor
{
public:
    GlobalVarVisitor(EmitAsm& f) : asmfile_(f) {}

    void VisitBasicBlock(BasicBlock*) override;
    void VisitStoreInstr(StoreInstr*) override;
    void VisitAddInstr(AddInstr*) override;
    void VisitSubInstr(SubInstr*) override;

    std::string GetExpr(const std::string& name) const { return map_.at(name); }

private:
    std::string Find(const std::string&) const;
    std::string Op2Str(const IROperand* op) const;

    std::unordered_map<std::string, std::string> map_{};
    EmitAsm& asmfile_;
};

#endif // _CODE_GEN_H_
