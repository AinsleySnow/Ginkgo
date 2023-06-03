#ifndef _IR_VISITOR_H_
#define _IR_VISITOR_H_

class Module;
class GlobalVar;
class Function;
class BasicBlock;

class RetInstr;    class BrInstr;
class SwitchInstr; class CallInstr;

class AddInstr;     class FaddInstr;
class SubInstr;     class FsubInstr;
class MulInstr;     class FmulInstr;
class DivInstr;     class FdivInstr;
class ModInstr;     class ShlInstr;
class LshrInstr;    class AshrInstr;
class AndInstr;     class OrInstr;
class XorInstr;

class AllocaInstr;  class LoadInstr;
class StoreInstr;   class GetValInstr;
class SetValInstr;  class GetElePtrInstr;

class TruncInstr;   class FtruncInstr;
class ZextInstr;    class SextInstr;
class FextInstr;    class FtoUInstr;
class FtoSInstr;    class UtoFInstr;
class StoFInstr;    class PtrtoIInstr;
class ItoPtrInstr;  class BitcastInstr;

class IcmpInstr;    class FcmpInstr;
class SelectInstr;  class PhiInstr;


class IRVisitor
{
public:
    virtual void VisitModule(Module*) {}
    virtual void VisitGlobalVar(GlobalVar*) {}
    virtual void VisitFunction(Function*) {}
    virtual void VisitBasicBlock(BasicBlock*) {}

    virtual void VisitRetInstr(RetInstr*) {}
    virtual void VisitBrInstr(BrInstr*) {}
    virtual void VisitSwitchInstr(SwitchInstr*) {}
    virtual void VisitCallInstr(CallInstr*) {}

    virtual void VisitAddInstr(AddInstr*) {}
    virtual void VisitFaddInstr(FaddInstr*) {}
    virtual void VisitSubInstr(SubInstr*) {}
    virtual void VisitFsubInstr(FsubInstr*) {}
    virtual void VisitMulInstr(MulInstr*) {}
    virtual void VisitFmulInstr(FmulInstr*) {}
    virtual void VisitDivInstr(DivInstr*) {}
    virtual void VisitFdivInstr(FdivInstr*) {}
    virtual void VisitModInstr(ModInstr*) {}
    virtual void VisitShlInstr(ShlInstr*) {}
    virtual void VisitLshrInstr(LshrInstr*) {}
    virtual void VisitAshrInstr(AshrInstr*) {}
    virtual void VisitAndInstr(AndInstr*) {}
    virtual void VisitOrInstr(OrInstr*) {}
    virtual void VisitXorInstr(XorInstr*) {}

    virtual void VisitAllocaInstr(AllocaInstr*) {}
    virtual void VisitLoadInstr(LoadInstr*) {}
    virtual void VisitStoreInstr(StoreInstr*) {}
    virtual void VisitGetValInstr(GetValInstr*) {}
    virtual void VisitSetValInstr(SetValInstr*) {}
    virtual void VisitGetElePtrInstr(GetElePtrInstr*) {}

    virtual void VisitTruncInstr(TruncInstr*) {}
    virtual void VisitFtruncInstr(FtruncInstr*) {}

    virtual void VisitZextInstr(ZextInstr*) {}
    virtual void VisitSextInstr(SextInstr*) {}
    virtual void VisitFextInstr(FextInstr*) {}
    virtual void VisitFtoUInstr(FtoUInstr*) {}
    virtual void VisitFtoSInstr(FtoSInstr*) {}

    virtual void VisitUtoFInstr(UtoFInstr*) {}
    virtual void VisitStoFInstr(StoFInstr*) {}
    virtual void VisitPtrtoIInstr(PtrtoIInstr*) {}
    virtual void VisitItoPtrInstr(ItoPtrInstr*) {}
    virtual void VisitBitcastInstr(BitcastInstr*) {}

    virtual void VisitIcmpInstr(IcmpInstr*) {}
    virtual void VisitFcmpInstr(FcmpInstr*) {}
    virtual void VisitSelectInstr(SelectInstr*) {}
    virtual void VisitPhiInstr(PhiInstr*) {}
};

#endif // _IR_VISITOR_H_
