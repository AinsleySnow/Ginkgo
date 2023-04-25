#include "visitir/CodeGen.h"
#include "IR/Value.h"
#include <string>
#include <unordered_map>


std::string CodeGen::Cond2Str(Condition cond, bool issigned)
{
    std::string str = "";
    switch (cond)
    {
    case Condition::eq: str = "e"; break;
    case Condition::ne: str = "ne"; break;
    case Condition::gt: str = issigned ? "g" : "a"; break;
    case Condition::le: str = issigned ? "le" : "be"; break;
    case Condition::ge: str = issigned ? "ge" : "ae"; break;
    case Condition::lt: str = issigned ? "l" : "b"; break;
    }

    return str;
}


void CodeGen::BinaryGenHelper(
    const std::string& name, const BinaryInstr* bi)
{
    auto lhs = alloc_.GetIROpMap(bi->Lhs());
    auto rhs = alloc_.GetIROpMap(bi->Rhs());
    auto ans = alloc_.GetIROpMap(bi->Result());

    asmfile_.EmitBinary(name, rhs, lhs);
    if (lhs->operator!=(*ans))
        asmfile_.EmitMov(lhs, ans);
}

void CodeGen::VarithmGenHelper(
    const std::string& name, const BinaryInstr* bi)
{
    auto lhs = alloc_.GetIROpMap(bi->Lhs());
    auto rhs = alloc_.GetIROpMap(bi->Rhs());
    auto ans = alloc_.GetIROpMap(bi->Result());
    asmfile_.EmitVarithm(name, lhs, rhs, ans);
}


void CodeGen::VisitModule(Module* mod)
{
    asmfile_.EmitPseudoInstr(".file", { "\"" + mod->Name() + "\"" });
    for (auto val : *mod)
        val->Accept(this);
}

void CodeGen::VisitGlobalVar(GlobalVar* var)
{
    GlobalVarVisitor visitor{ asmfile_ };
    visitor.VisitBasicBlock(var->GetBasicBlock());

    // stripping the leading '@'
    auto name = var->Name().substr(1);
    auto size = std::to_string(var->Type()->Size());

    asmfile_.EmitPseudoInstr(".data");
    asmfile_.EmitPseudoInstr(".globl", { name });
    asmfile_.EmitPseudoInstr(".align", { size });
    asmfile_.EmitPseudoInstr(".type", { name, "@object" });
    asmfile_.EmitPseudoInstr(".size", { name, size });
    asmfile_.EmitLabel(name);
    asmfile_.EmitPseudoInstr(".long", { visitor.GetExpr(name) });
}

void CodeGen::VisitFunction(Function* func)
{
    alloc_.EnterFunction(func);
    asmfile_.EmitPseudoInstr(".text");
    asmfile_.EmitLabel(func->Name());
    for (auto bb : *func)
        VisitBasicBlock(bb);
}

void CodeGen::VisitBasicBlock(BasicBlock* bb)
{
    asmfile_.EmitLabel(bb->Name());
    for (auto inst : *bb)
        inst->Accept(this);
}


void CodeGen::VisitRetInstr(RetInstr* inst)
{
    if (inst->ReturnValue())
    {
        auto val = alloc_.GetIROpMap(inst->ReturnValue());
        auto rax = x64Reg(RegTag::rax, val->Size());
        asmfile_.EmitMov(val, &rax);
    }
    asmfile_.EmitRet();
}


void CodeGen::VisitBrInstr(BrInstr* inst)
{
    if (inst->Cond())
    {
        auto cond = alloc_.GetIROpMap(inst->Cond());
        asmfile_.EmitCmp(cond, (unsigned long)0);
        asmfile_.EmitJmp("ne", inst->GetTrueBlk()->Name());
        asmfile_.EmitJmp("", inst->GetFalseBlk()->Name());
    }
    else asmfile_.EmitJmp("", inst->GetTrueBlk()->Name());
}


void CodeGen::VisitSwitchInstr(SwitchInstr* inst)
{
    /*auto val = alloc_.GetIROpMap(inst->Value());
    auto def = inst->GetDefaultBlk();
    auto cases = inst->GetCases();

    asmfile_.EmitCmp(SuffixOf(val)[0], val->ToString(), "$0");
    asmfile_.EmitJmp("ne", cases[0].second->Name());
    asmfile_.EmitJmp("", def->Name());*/
}


void CodeGen::VisitCallInstr(CallInstr* inst)
{
    /*auto func = inst->GetFunction();
    auto args = inst->GetArgs();
    auto ret = inst->GetRet();

    // push args in reverse order
    for (auto it = args.rbegin(); it != args.rend(); ++it)
    {
        auto arg = alloc_.GetIROpMap(*it);
        asmfile_.EmitPush(SuffixOf(arg)[0], arg->ToString());
    }

    // call the function
    asmfile_.EmitCall(func->Name());

    // pop args
    auto size = std::to_string(func->GetArgsSize());
    asmfile_.EmitAdd('q', "$" + size, "%rsp");

    // move return value to %rax
    if (ret)
    {
        auto ans = alloc_.GetIROpMap(ret);
        asmfile_.EmitMov(SuffixOf(ans)[0], "%rax", ans->ToString());
    }*/
}


void CodeGen::VisitAddInstr(AddInstr* inst)     { BinaryGenHelper("add", inst); }
void CodeGen::VisitFaddInstr(FaddInstr* inst)   { VarithmGenHelper("add", inst); }
void CodeGen::VisitSubInstr(SubInstr* inst)     { BinaryGenHelper("sub", inst); }
void CodeGen::VisitFsubInstr(FsubInstr* inst)   { VarithmGenHelper("sub", inst); }
void CodeGen::VisitMulInstr(MulInstr* inst)     { BinaryGenHelper("imul", inst); }
void CodeGen::VisitFmulInstr(FmulInstr* inst)   { VarithmGenHelper("mul", inst); }
void CodeGen::VisitFdivInstr(FdivInstr* inst)   { VarithmGenHelper("div", inst); }
void CodeGen::VisitShlInstr(ShlInstr* inst)     { BinaryGenHelper("shl", inst); }
void CodeGen::VisitLshrInstr(LshrInstr* inst)   { BinaryGenHelper("shr", inst); }
void CodeGen::VisitAshrInstr(AshrInstr* inst)   { BinaryGenHelper("sar", inst); }
void CodeGen::VisitAndInstr(AndInstr* inst)     { BinaryGenHelper("and", inst); }
void CodeGen::VisitOrInstr(OrInstr* inst)       { BinaryGenHelper("or", inst); }
void CodeGen::VisitXorInstr(XorInstr* inst)     { BinaryGenHelper("xor", inst); }

void CodeGen::VisitDivInstr(DivInstr* inst)
{
    auto lhs = alloc_.GetIROpMap(inst->Lhs());
    auto rhs = alloc_.GetIROpMap(inst->Rhs());
    auto ans = alloc_.GetIROpMap(inst->Result());
    auto rax = x64Reg(RegTag::rax, lhs->Size());

    // the first op in DivInstr may not be in %*ax.
    // move it to %*ax if it is not.
    if (!lhs->As<x64Reg>()->operator==(RegTag::rax))
        asmfile_.EmitMov(lhs, &rax);

    asmfile_.EmitCxtx(&rax);

    bool issigned = inst->Lhs()->Type()->ToInteger()->IsSigned() ||
        inst->Rhs()->Type()->ToInteger()->IsSigned();
    std::string name = issigned ? "idiv" : "div";
    asmfile_.EmitUnary(name, rhs);

    // if result register is not %*ax, move it to the result register.
    if (!alloc_.GetIROpMap(inst->Result())->As<x64Reg>()->operator==(RegTag::rax))
        asmfile_.EmitMov(&rax, ans);
}

void CodeGen::VisitModInstr(ModInstr* inst)
{
    auto lhs = alloc_.GetIROpMap(inst->Lhs());
    auto rhs = alloc_.GetIROpMap(inst->Rhs());
    auto ans = alloc_.GetIROpMap(inst->Result());
    auto rax = x64Reg(RegTag::rax, lhs->Size());

    // the first op in DivInstr may not be in %*ax.
    // move it to %*ax if it is not.
    if (!lhs->As<x64Reg>()->operator==(RegTag::rax))
        asmfile_.EmitMov(lhs, &rax);

    asmfile_.EmitCxtx(&rax);

    bool issigned = inst->Lhs()->Type()->ToInteger()->IsSigned() ||
        inst->Rhs()->Type()->ToInteger()->IsSigned();
    std::string name = issigned ? "idiv" : "div";
    asmfile_.EmitUnary(name, rhs);

    // if result register is not %*dx, move it to the result register.
    if (!alloc_.GetIROpMap(inst->Result())->As<x64Reg>()->operator==(RegTag::rdx))
    {
        auto rdx = x64Reg(RegTag::rdx, ans->Size());
        asmfile_.EmitMov(&rdx, ans);
    }
}


void CodeGen::VisitTruncInstr(TruncInstr* inst)
{
    auto from = alloc_.GetIROpMap(inst->Value());
    auto to = alloc_.GetIROpMap(inst->Dest());

    if (inst->Type()->ToInteger()->IsSigned())
        asmfile_.EmitMovs(from, to);
    else
        asmfile_.EmitMovz(from, to);
}

void CodeGen::VisitFtruncInstr(FtruncInstr* inst)
{
    auto from = alloc_.GetIROpMap(inst->Value());
    auto to = alloc_.GetIROpMap(inst->Dest());

    asmfile_.EmitVcvt(from, from, to);
}

void CodeGen::VisitZextInstr(ZextInstr* inst)
{
    auto from = alloc_.GetIROpMap(inst->Value());
    auto to = alloc_.GetIROpMap(inst->Dest());
    asmfile_.EmitMovz(from, to);
}

void CodeGen::VisitSextInstr(SextInstr* inst)
{
    auto from = alloc_.GetIROpMap(inst->Value());
    auto to = alloc_.GetIROpMap(inst->Dest());
    asmfile_.EmitMovs(from, to);
}

void CodeGen::VisitFextInstr(FextInstr* inst)
{
    auto from = alloc_.GetIROpMap(inst->Value());
    auto to = alloc_.GetIROpMap(inst->Dest());
    asmfile_.EmitVcvt(from, from, to);
}

void CodeGen::VisitFtoUInstr(FtoUInstr* inst)
{
    auto from = alloc_.GetIROpMap(inst->Value());
    auto to = alloc_.GetIROpMap(inst->Dest());
    asmfile_.EmitVcvtt(from, to);
}

void CodeGen::VisitFtoSInstr(FtoSInstr* inst)
{
    auto from = alloc_.GetIROpMap(inst->Value());
    auto to = alloc_.GetIROpMap(inst->Dest());
    asmfile_.EmitVcvtt(from, to);
}

void CodeGen::VisitUtoFInstr(UtoFInstr* inst)
{
    auto from = alloc_.GetIROpMap(inst->Value());
    auto to = alloc_.GetIROpMap(inst->Dest());
    asmfile_.EmitVcvt(from, from, to);
}

void CodeGen::VisitStoFInstr(StoFInstr* inst)
{
    auto from = alloc_.GetIROpMap(inst->Value());
    auto to = alloc_.GetIROpMap(inst->Dest());
    asmfile_.EmitVcvt(from, from, to);
}


void CodeGen::VisitIcmpInstr(IcmpInstr* inst)
{
    auto lhs = alloc_.GetIROpMap(inst->Op1());
    auto rhs = alloc_.GetIROpMap(inst->Op2());
    auto ans = alloc_.GetIROpMap(inst->Result());
    auto issigned = inst->Op1()->Type()->ToInteger()->IsSigned() ||
        inst->Op2()->Type()->ToInteger()->IsSigned();

    asmfile_.EmitCmp(rhs, lhs);
    asmfile_.EmitSet(Cond2Str(inst->Cond(), issigned), ans);
    asmfile_.EmitMovz(1, ans->Size(), ans);
}


void CodeGen::VisitFcmpInstr(FcmpInstr* inst)
{
    auto lhs = alloc_.GetIROpMap(inst->Op1());
    auto rhs = alloc_.GetIROpMap(inst->Op2());
    auto ans = alloc_.GetIROpMap(inst->Result());

    asmfile_.EmitUcom(rhs, lhs);
    asmfile_.EmitSet(Cond2Str(inst->Cond(), true), ans);
    asmfile_.EmitMovz(ans, ans);
}


void CodeGen::VisitSelectInstr(SelectInstr* inst)
{
    auto [cond, ty] = inst->CondPair();
    auto xcond = alloc_.GetIROpMap(cond);
    auto v1 = alloc_.GetIROpMap(inst->Value1());
    auto v2 = alloc_.GetIROpMap(inst->Value2());
    auto ans = alloc_.GetIROpMap(inst->Result());

    asmfile_.EmitTest(xcond, xcond);
    asmfile_.EmitMov(v1, ans);
    asmfile_.EmitCMov(ty ? "e" : "ne", v2, ans);
}


void CodeGen::VisitPhiInstr(PhiInstr* inst)
{
    /*auto ans = alloc_.GetIROpMap(inst->Result());

    for (auto& pair : inst->Values())
    {
        auto val = alloc_.GetIROpMap(pair.second);
        asmfile_.EmitMov(sto, val->ToString(), ans->ToString());
    }*/
}



std::string GlobalVarVisitor::Const2Str(const Constant* c) const
{
    if (c->Is<IntConst>())
        return std::to_string(c->As<IntConst>()->Val());
    double val = c->As<FloatConst>()->Val();
    unsigned long repr = *reinterpret_cast<unsigned long*>(&val);
    return std::to_string(repr);
}

std::string GlobalVarVisitor::Strip(const std::string& s) const
{
    if (s[0] == '@' || s[0] == '%')
        return s.substr(1);
    else return s;
}

std::string GlobalVarVisitor::Find(const std::string& s) const
{
    if (map_.find(s) != map_.end())
        return map_.at(s);
    else return s;
};

std::string GlobalVarVisitor::Op2Str(const IROperand* op) const
{
    if (op->Is<Constant>())
        return Const2Str(op->As<Constant>());
    else
        return Find(Strip(op->As<Register>()->Name()));
}

void GlobalVarVisitor::VisitBasicBlock(BasicBlock* bb)
{
    for (auto i : *bb)
        i->Accept(this);
}

void GlobalVarVisitor::VisitStoreInstr(StoreInstr* store)
{
    auto dest = Strip(store->Dest()->Name());
    map_[dest] = Op2Str(store->Value());
}

void GlobalVarVisitor::VisitAddInstr(AddInstr* add)
{
    auto lhs = Op2Str(add->Lhs());
    auto rhs = Op2Str(add->Rhs());
    map_[Strip(add->Result()->As<Register>()->Name())] = lhs + '+' + rhs;
}

void GlobalVarVisitor::VisitSubInstr(SubInstr* sub)
{
    auto lhs = Op2Str(sub->Lhs());
    auto rhs = Op2Str(sub->Rhs());
    map_[Strip(sub->Result()->As<Register>()->Name())] = lhs + '-' + rhs;
}
