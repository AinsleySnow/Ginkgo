#ifndef _CODE_GEN_H_
#define _CODE_GEN_H_

#include "visitir/IRVisitor.h"
#include "visitir/EmitAsm.h"
#include "visitir/x64.h"
#include "pass/x64Alloc.h"
#include <cstdio>
#include <string>
#include <unordered_map>

class BinaryInstr;
class IROperand;
class Constant;
enum class Condition;


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

    void VisitLoadInstr(LoadInstr*) override {}
    void VisitStoreInstr(StoreInstr*) override {} 

    void VisitTruncInstr(TruncInstr*) override;
    void VisitFtruncInstr(FtruncInstr*) override;

    void VisitZextInstr(ZextInstr*) override;
    void VisitSextInstr(SextInstr*) override;
    void VisitFextInstr(FextInstr*) override;
    void VisitFtoUInstr(FtoUInstr*) override;
    void VisitFtoSInstr(FtoSInstr*) override;
    void VisitUtoFInstr(UtoFInstr*) override;
    void VisitStoFInstr(StoFInstr*) override;

    // instructions like PtrtoI, ItoPtr and Bitcast
    // Will be handled in register allocators

    void VisitIcmpInstr(IcmpInstr*) override;
    void VisitFcmpInstr(FcmpInstr*) override;
    void VisitSelectInstr(SelectInstr*) override;
    void VisitPhiInstr(PhiInstr*) override;

private:
    void AdjustRsp();
    std::string Cond2Str(Condition, bool);
    void BinaryGenHelper(const std::string&, const BinaryInstr*);
    void VarithmGenHelper(const std::string&, const BinaryInstr*);

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
    std::string Const2Str(const Constant* c) const;
    std::string Strip(const std::string& s) const;
    std::string Find(const std::string&) const;
    std::string Op2Str(const IROperand* op) const;

    std::unordered_map<std::string, std::string> map_{};
    EmitAsm& asmfile_;
};

#endif // _CODE_GEN_H_
