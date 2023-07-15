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
class IROperand;
class SysVConv;
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

    void VisitNode(OpNode*) override;
    void VisitNode(BinaryNode*) override;
    void VisitNode(UnaryNode*) override;

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
    struct FpRepr
    {
        struct Equal
        {
            bool operator()(const FpRepr& r1, const FpRepr& r2) const
            {
                return r1.first_ == r2.first_ &&
                    r1.second_ == r2.second_ && r1.size_ == r2.size_;
            }
        };
        struct Hash
        {
            size_t operator()(const FpRepr& repr) const
            {
            #define HASH 0x9e3779b9 + (hash << 6) + (hash >> 2)
                size_t hash = repr.first_;
                hash ^= repr.second_ + HASH;
                hash ^= repr.size_ + HASH;
                return hash;
            #undef HASH
            }
        };

        unsigned long first_;
        unsigned long second_;
        size_t size_;
    };

    std::string GetFpLabel(unsigned long, size_t);
    std::string GetFpLabel(std::pair<unsigned long, unsigned long>, size_t);
    std::string GetLabel() const;
    std::string GetLabel(const BasicBlock*) const;

    // since x64 requires that float point constants
    // must be loaded from the memory, so when the operators
    // in some instructions can be floatpoint constants,
    // use this method to get the value correctly loaded.
    // in other cases, use bare alloc_.GetIROpMap would be enough.
    const x64* MapPossibleFloat(const IROperand*);
    // sometimes an operator with pointer type maps to a stack
    // address where the address stores itself, not the value
    // it points to. MapPossiblePointer tackle with this problem.
    const x64* MapPossiblePointer(const IROperand*); 

    void AlignRspBy(size_t, size_t);
    void AdjustRsp(long);
    void DeallocFrame();

    void MapPtrParam2Mem(const x64*);
    void MapPtrParam2Reg(const x64*, const x64Reg*);
    void MapFltParam2Mem(const x64*);
    void MapFltParam2Reg(const x64*, const x64Reg*);
    void MapOtherParam2Mem(const x64*);
    void MapOtherParam2Reg(const x64*, const x64Reg*);
    void PassParam(const SysVConv&, const std::vector<const IROperand*>&);

    void SaveCalleeSaved();
    void RestoreCalleeSaved();
    void SaveCallerSaved();
    void RestoreCallerSaved();

    RegTag GetSpareIntReg() const;
    RegTag GetSpareVecReg() const;

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
    void VcvtsiEmitHelper(const x64*, const x64*);
    void VcvttEmitHelper(const x64*, const x64*);
    void UcomEmitHelper(const x64*, const x64*);

    void PushEmitHelper(const x64*);
    void PushEmitHelper(RegTag, size_t);
    void PopEmitHelper(const x64*);
    void PopEmitHelper(RegTag, size_t);
    void PushXmmReg(RegTag);
    void PopXmmReg(RegTag);

    void CMovEmitHelper(Condition, bool, const x64*, const x64*);
    void CmpEmitHelper(const x64*, const x64*);
    void SetEmitHelper(Condition, bool, const x64*);
    void TestEmitHelper(const x64*, const x64*);

    size_t stacksize_{};
    mutable int labelindex_{};
    mutable std::unordered_map<
        const BasicBlock*, std::string> bb2label_{};

    std::unordered_map<
        const IROperand*, std::unique_ptr<const x64>> tempmap_{};
    std::unordered_map<FpRepr, std::string, FpRepr::Hash, FpRepr::Equal> fpconst_{};

    x64Alloc& alloc_;
    EmitAsm asmfile_{ "" };
};


#endif // _CODE_GEN_H_
