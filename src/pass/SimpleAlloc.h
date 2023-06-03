#ifndef _SIMPLE_ALLOC_H_
#define _SIMPLE_ALLOC_H_

#include "pass/Pass.h"
#include "pass/x64Alloc.h"
#include "visitir/IRVisitor.h"
#include "visitir/x64.h"
#include <unordered_map>

class BinaryInstr;
class ConvertInstr;
class Register;


// the SimpleAlloc class do register allocation using the 3-TOSCA algorithm.
// DO NOT use it if any optimization pass is used, since the allocator just
// perform allocation over the original not-so-SSAic IR, and it treats the
// registers that are not given by alloca as temp registers. It'll run into
// problems if some of these registers are used more than once.
// For the TSOCA algorithm, see https://www.zhihu.com/question/29355187/answer/51935409.

class SimpleAlloc : public x64Alloc
{
public:
    SimpleAlloc(Module* m) : x64Alloc(m) {}

private:
    class StackCache
    {
    public:
        StackCache(SimpleAlloc& sa) : alloc_(sa) {}

        RegTag SpareReg() const;
        RegTag SpareFReg() const;
        void Access(const Register*) const;
        void Map2Reg(const Register*, RegTag);
        void Map2Stack(const Register*, long offset);

    private:
        SimpleAlloc& alloc_;

        // map virtual registers to where? the value can be
        // either a register or a stack address.
        std::unordered_map<const Register*, const x64*> regmap_{};

        // the stack cache; implemented as a ring buffer.
        // the three registers used are rax, rdx, rcx, respectively.
        mutable int index_{};
        // for float-points, the three registers used is xmm0, xmm1, xmm2.
        mutable int findex_{};
    };

    void Allocate(const Register*);
    long AllocateOnX64Stack(x64Stack&, size_t, size_t);

    void BinaryAllocaHelper(BinaryInstr*);
    void ConvertAllocaHelper(ConvertInstr*);

    Function* curfunc_{};
    StackCache stackcache_{ *this };

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

#endif // _SIMPLE_ALLOC_H_
