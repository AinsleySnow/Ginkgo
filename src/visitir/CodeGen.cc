#include "visitir/CodeGen.h"
#include "IR/Value.h"
#include <string>
#include <unordered_map>


void CodeGen::BinaryGenHelper(
    const std::string& name, const BinaryInstr* bi)
{
    auto lhs = alloc_.GetIROpMap(bi->Lhs());
    auto rhs = alloc_.GetIROpMap(bi->Rhs());
    auto ans = alloc_.GetIROpMap(bi->Result());
    char suffix = SuffixOf(rhs)[0];

    asmfile_.EmitBinary(suffix, name, rhs->ToString(), lhs->ToString());
    if (lhs->operator!=(*ans))
        asmfile_.EmitMov(suffix, lhs->ToString(), ans->ToString());
}

void CodeGen::VarithmGenHelper(
    const std::string& name, const BinaryInstr* bi)
{
    auto lhs = alloc_.GetIROpMap(bi->Lhs());
    auto rhs = alloc_.GetIROpMap(bi->Rhs());
    auto ans = alloc_.GetIROpMap(bi->Result());
    auto precision = SuffixOf(rhs);

    if (!rhs)
        asmfile_.EmitVarithm(name, precision,
            lhs->ToString(), "", ans->ToString());
    else
        asmfile_.EmitVarithm(name, precision,
            lhs->ToString(), rhs->ToString(), ans->ToString());
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

    // the first op in DivInstr may not be in %*ax.
    // move it to %*ax if it is not.
    if (!lhs->As<x64Reg>()->PartOf(RegTag::rax))
        asmfile_.EmitMov(SuffixOf(lhs)[0], lhs->ToString(), "%rax");

    asmfile_.EmitCxtx(SuffixOf(lhs)[0], SuffixOf(rhs)[0]);

    bool issigned = inst->Lhs()->Type()->ToInteger()->IsSigned() ||
        inst->Rhs()->Type()->ToInteger()->IsSigned();
    std::string name = issigned ? "idiv" : "div";
    asmfile_.EmitUnary(SuffixOf(rhs)[0], name, rhs->ToString());

    // if result register is not %*ax, move it to the result register.
    if (!alloc_.GetIROpMap(inst->Result())->As<x64Reg>()->PartOf(RegTag::rax))
        asmfile_.EmitMov(SuffixOf(lhs)[0], "%rax", ans->ToString());
}

void CodeGen::VisitModInstr(ModInstr* inst)
{
    auto lhs = alloc_.GetIROpMap(inst->Lhs());
    auto rhs = alloc_.GetIROpMap(inst->Rhs());
    auto ans = alloc_.GetIROpMap(inst->Result());

    // the first op in DivInstr may not be in %*ax.
    // move it to %*ax if it is not.
    if (!lhs->As<x64Reg>()->PartOf(RegTag::rax))
        asmfile_.EmitMov(SuffixOf(lhs)[0], lhs->ToString(), "%rax");

    asmfile_.EmitCxtx(SuffixOf(lhs)[0], SuffixOf(rhs)[0]);

    bool issigned = inst->Lhs()->Type()->ToInteger()->IsSigned() ||
        inst->Rhs()->Type()->ToInteger()->IsSigned();
    std::string name = issigned ? "idiv" : "div";
    asmfile_.EmitUnary(SuffixOf(rhs)[0], name, rhs->ToString());

    // if result register is not %*dx, move it to the result register.
    if (!alloc_.GetIROpMap(inst->Result())->As<x64Reg>()->PartOf(RegTag::rdx))
        asmfile_.EmitMov(SuffixOf(lhs)[0], "%rdx", ans->ToString());
}


void CodeGen::VisitTruncInstr(TruncInstr* inst)
{
    auto from = alloc_.GetIROpMap(inst->Value());
    auto to = alloc_.GetIROpMap(inst->Dest());
    char sfrm = SuffixOf(from)[0];
    char sto = SuffixOf(to)[0];

    if (inst->Type()->ToInteger()->IsSigned())
        asmfile_.EmitMovs(sfrm, sto, from->ToString(), to->ToString());
    else
        asmfile_.EmitMovz(sfrm, sto, from->ToString(), to->ToString());
}

void CodeGen::VisitFtruncInstr(FtruncInstr* inst)
{
    auto from = alloc_.GetIROpMap(inst->Value());
    auto to = alloc_.GetIROpMap(inst->Dest());

    asmfile_.EmitVcvt(SuffixOf(from), SuffixOf(to),
        from->ToString(), from->ToString(), to->ToString());
}

void CodeGen::VisitZextInstr(ZextInstr* inst)
{
    auto from = alloc_.GetIROpMap(inst->Value());
    auto to = alloc_.GetIROpMap(inst->Dest());
    char sfrm = SuffixOf(from)[0];
    char sto = SuffixOf(to)[0];

    asmfile_.EmitMovz(sfrm, sto, from->ToString(), to->ToString());
}

void CodeGen::VisitSextInstr(SextInstr* inst)
{
    auto from = alloc_.GetIROpMap(inst->Value());
    auto to = alloc_.GetIROpMap(inst->Dest());
    char sfrm = SuffixOf(from)[0];
    char sto = SuffixOf(to)[0];

    asmfile_.EmitMovs(sfrm, sto, from->ToString(), to->ToString());
}

void CodeGen::VisitFextInstr(FextInstr* inst)
{
    auto from = alloc_.GetIROpMap(inst->Value());
    auto to = alloc_.GetIROpMap(inst->Dest());

    asmfile_.EmitVcvt(SuffixOf(from), SuffixOf(to),
        from->ToString(), from->ToString(), to->ToString());
}

void CodeGen::VisitFtoUInstr(FtoUInstr* inst)
{
    auto from = alloc_.GetIROpMap(inst->Value());
    auto to = alloc_.GetIROpMap(inst->Dest());

    asmfile_.EmitVcvtt(SuffixOf(from), SuffixOf(to),
        from->ToString(), to->ToString());
}

void CodeGen::VisitFtoSInstr(FtoSInstr* inst)
{
    auto from = alloc_.GetIROpMap(inst->Value());
    auto to = alloc_.GetIROpMap(inst->Dest());

    asmfile_.EmitVcvtt(SuffixOf(from), SuffixOf(to),
        from->ToString(), to->ToString());
}

void CodeGen::VisitUtoFInstr(UtoFInstr* inst)
{
    auto from = alloc_.GetIROpMap(inst->Value());
    auto to = alloc_.GetIROpMap(inst->Dest());

    asmfile_.EmitVcvt(SuffixOf(from), SuffixOf(to),
        from->ToString(), from->ToString(), to->ToString());
}

void CodeGen::VisitStoFInstr(StoFInstr* inst)
{
    auto from = alloc_.GetIROpMap(inst->Value());
    auto to = alloc_.GetIROpMap(inst->Dest());

    asmfile_.EmitVcvt(SuffixOf(from), SuffixOf(to),
        from->ToString(), from->ToString(), to->ToString());
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
