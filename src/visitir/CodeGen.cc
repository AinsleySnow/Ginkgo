#include "visitir/CodeGen.h"
#include "IR/Value.h"
#include <string>
#include <unordered_map>


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


inline void CodeGen::BinaryGenHelper(
    const std::string& name, const BinaryInstr* bi)
{
    char suffix = bi->AsmSuffix()[0];

    asmfile_.EmitBinary(suffix, name,
        bi->Rhs()->ToString(), bi->Lhs()->ToString());
    if (bi->Lhs() != bi->Result())
        asmfile_.EmitMov(suffix,
            bi->Result()->ToString(), bi->Lhs()->ToString());
}

inline void CodeGen::VarithmGenHelper(
    const std::string& name, const BinaryInstr* bi)
{
    auto precision = bi->AsmSuffix();
    if (!bi->Rhs())
        asmfile_.EmitVarithm(name, precision,
            bi->Lhs()->ToString(), "", bi->Result()->ToString());
    else
        asmfile_.EmitVarithm(name, precision,
            bi->Lhs()->ToString(), bi->Rhs()->ToString(), bi->Result()->ToString());
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
    asmfile_.EmitCxtx(inst->From(), inst->To());

    // the first op in DivInstr must be in %rax.
    // this is handled by the register allocator.
    std::string name = inst->Signed() ? "idiv" : "div";
    asmfile_.EmitUnary(inst->AsmSuffix()[0], name, inst->Rhs()->ToString());

    // if result register is not %*ax, move it to the result register.
    if (!inst->Result()->As<x64Reg>()->PartOf(RegTag::rax))
        asmfile_.EmitMov(inst->AsmSuffix()[0],
            inst->Rhs()->ToString(), inst->Result()->ToString());
}

void CodeGen::VisitModInstr(ModInstr* inst)
{
    asmfile_.EmitCxtx(inst->From(), inst->To());

    // the first op in DivInstr must be in %rax.
    // this is handled by the register allocator.
    std::string name = inst->Signed() ? "idiv" : "div";
    asmfile_.EmitUnary(inst->AsmSuffix()[0], name, inst->Rhs()->ToString());

    // if result register is not %*dx, move it to the result register.
    if (!inst->Result()->As<x64Reg>()->PartOf(RegTag::rdx))
        asmfile_.EmitMov(inst->AsmSuffix()[0],
            inst->Rhs()->ToString(), inst->Result()->ToString());
}


void CodeGen::VisitTruncInstr(TruncInstr* inst)
{
    if (inst->Type()->ToInteger()->IsSigned())
        asmfile_.EmitMovs(inst->From(), inst->To(),
            inst->Value()->ToString(), inst->Dest()->ToString());
    else
        asmfile_.EmitMovz(inst->From(), inst->To(),
            inst->Value()->ToString(), inst->Dest()->ToString());
}

void CodeGen::VisitFtruncInstr(FtruncInstr* inst)
{
    auto value = inst->Value()->ToString();
    asmfile_.EmitVcvt(inst->From(), inst->To(), value,
        value, inst->Dest()->ToString());
}

void CodeGen::VisitZextInstr(ZextInstr* inst)
{
    asmfile_.EmitMovz(inst->From(), inst->To(),
        inst->Value()->ToString(), inst->Dest()->ToString());
}

void CodeGen::VisitSextInstr(SextInstr* inst)
{
    asmfile_.EmitMovs(inst->From(), inst->To(),
        inst->Value()->ToString(), inst->Dest()->ToString());
}

void CodeGen::VisitFextInstr(FextInstr* inst)
{
    auto value = inst->Value()->ToString();
    asmfile_.EmitVcvt(inst->From(), inst->To(), value,
        value, inst->Dest()->ToString());
}

void CodeGen::VisitFtoUInstr(FtoUInstr* inst)
{
    asmfile_.EmitVcvtt(inst->From(), inst->To(),
        inst->Value()->ToString(), inst->Dest()->ToString());
}

void CodeGen::VisitFtoSInstr(FtoSInstr* inst)
{
    asmfile_.EmitVcvtt(inst->From(), inst->To(),
        inst->Value()->ToString(), inst->Dest()->ToString());
}

void CodeGen::VisitUtoFInstr(UtoFInstr* inst)
{
    auto value = inst->Value()->ToString();
    asmfile_.EmitVcvt(inst->From(), inst->To(),
        value, value, inst->Dest()->ToString());
}

void CodeGen::VisitStoFInstr(StoFInstr* inst)
{
    auto value = inst->Value()->ToString();
    asmfile_.EmitVcvt(inst->From(), inst->To(),
        value, value, inst->Dest()->ToString());
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
