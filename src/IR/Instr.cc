#include "IR/Instr.h"
#include "IR/Value.h"
#include "visitir/IRVisitor.h"
#include <climits>
#include <cfloat>


std::string RetInstr::ToString() const
{
    if (retval_) return "ret " + retval_->ToString();
    else          return "ret void";
}


std::string BrInstr::ToString() const
{
    if (!cond_) return "br label " + true_->Name();
    else    return "br " + cond_->ToString() + ", label " + true_->Name() + ", label " + false_->Name();
}


std::string SwitchInstr::ToString() const
{
    std::string caselist{ default_->Name() + " [\n" };
    for (const auto& pair : cases_)
        caselist += "    " + pair.first->ToString() +
            ": label " + pair.second->Name() + '\n';
    caselist += "  ]";
    return "switch " + ident_->ToString() +
        ", label " + caselist;
}


std::string CallInstr::ToString() const
{
    std::string call = "call ";
    if (funcaddr_) call += funcaddr_->ToString() + " (";
    else call += proto_->ToString() + ' ' + func_ + '(';

    if (!arglist_.empty())
    {
        for (auto arg = arglist_.begin(); arg < arglist_.end() - 1; arg++)
            call += (*arg)->ToString() + ", ";
        call += arglist_.back()->ToString();
    }
    return result_ ? call + ')' : result_->Name() + " = " + call + ')';
}


std::string AddInstr::ToString() const
{ return Result()->Name() + " = add " + Lhs()->ToString() + ", " + Rhs()->ToString(); }
void AddInstr::Accept(IRVisitor* v) { v->VisitAddInstr(this); }
std::string FaddInstr::ToString() const
{ return Result()->Name() + " = fadd " + Lhs()->ToString() + ", " + Rhs()->ToString(); }
void FaddInstr::Accept(IRVisitor* v) { v->VisitFaddInstr(this); }
std::string SubInstr::ToString() const
{ return Result()->Name() + " = sub " + Lhs()->ToString() + ", " + Rhs()->ToString(); }
void SubInstr::Accept(IRVisitor* v) { v->VisitSubInstr(this); }
std::string FsubInstr::ToString() const
{ return Result()->Name() + " = fsub " + Lhs()->ToString() + ", " + Rhs()->ToString(); }
void FsubInstr::Accept(IRVisitor* v) { v->VisitFsubInstr(this); }
std::string MulInstr::ToString() const
{ return Result()->Name() + " = mul " + Lhs()->ToString() + ", " + Rhs()->ToString(); }
void MulInstr::Accept(IRVisitor* v) { v->VisitMulInstr(this); }
std::string FmulInstr::ToString() const
{ return Result()->Name() + " = fmul " + Lhs()->ToString() + ", " + Rhs()->ToString(); }
void FmulInstr::Accept(IRVisitor* v) { v->VisitFmulInstr(this); }
std::string DivInstr::ToString() const
{ return Result()->Name() + " = div " + Lhs()->ToString() + ", " + Rhs()->ToString(); }
void DivInstr::Accept(IRVisitor* v) { v->VisitDivInstr(this); }
std::string FdivInstr::ToString() const
{ return Result()->Name() + " = fdiv " + Lhs()->ToString() + ", " + Rhs()->ToString(); }
void FdivInstr::Accept(IRVisitor* v) { v->VisitFdivInstr(this); }
std::string ModInstr::ToString() const
{ return Result()->Name() + " = mod " + Lhs()->ToString() + ", " + Rhs()->ToString(); }
void ModInstr::Accept(IRVisitor* v) { v->VisitModInstr(this); }
std::string ShlInstr::ToString() const
{ return Result()->Name() + " = shl " + Lhs()->ToString() + ", " + Rhs()->ToString(); }
void ShlInstr::Accept(IRVisitor* v) { v->VisitShlInstr(this); }
std::string LshrInstr::ToString() const
{ return Result()->Name() + " = lshr " + Lhs()->ToString() + ", " + Rhs()->ToString(); }
void LshrInstr::Accept(IRVisitor* v) { v->VisitLshrInstr(this); }
std::string AshrInstr::ToString() const
{ return Result()->Name() + " = ashr " + Lhs()->ToString() + ", " + Rhs()->ToString(); }
void AshrInstr::Accept(IRVisitor* v) { v->VisitAshrInstr(this); }
std::string AndInstr::ToString() const
{ return Result()->Name() + " = and " + Lhs()->ToString() + ", " + Rhs()->ToString(); }
void AndInstr::Accept(IRVisitor* v) { v->VisitAndInstr(this); }
std::string OrInstr::ToString() const
{ return Result()->Name() + " = or " + Lhs()->ToString() + ", " + Rhs()->ToString(); }
void OrInstr::Accept(IRVisitor* v) { v->VisitOrInstr(this); }
std::string XorInstr::ToString() const
{ return Result()->Name() + " = xor " + Lhs()->ToString() + ", " + Rhs()->ToString(); }
void XorInstr::Accept(IRVisitor* v) { v->VisitXorInstr(this); }


std::string AllocaInstr::ToString() const
{
    std::string line = result_->Name() +
        " = alloca " + type_->ToString();
    if (num_ > 1)
        line += "i64 " + std::to_string(num_);
    if (align_ > 1)
        line += ", align " + std::to_string(align_);
    return line;
}

std::string LoadInstr::ToString() const
{
    std::string line = result_->Name() + " = " +
        (volatile_ ? "volatile load " : "load ");
    line += pointer_->ToString();
    if (align_ > 1)
        line += ", align" + std::to_string(align_);
    return line;
}

std::string StoreInstr::ToString() const
{
    std::string line = volatile_ ? "volatile store " : "store ";
    line += value_->ToString() + ", " + pointer_->ToString();
    return line;
}


std::string GetValInstr::ToString() const
{
    return result_->Name() + " = getval " + pointer_->ToString() +
        '[' + (std::holds_alternative<int>(index_) ?
            std::to_string(std::get<1>(index_)) : std::get<0>(index_)->ToString()) + ']';
}

std::string SetValInstr::ToString() const
{
    return "setval " + newval_->ToString() + ", " +
        pointer_->ToString() + '[' +  (std::holds_alternative<int>(index_) ?
            std::to_string(std::get<1>(index_)) : std::get<0>(index_)->ToString()) + ']';
}

std::string GetElePtrInstr::ToString() const
{
    return result_->Name() + " = geteleptr " + pointer_->ToString() +
        " [" +  (std::holds_alternative<int>(index_) ?
            std::to_string(std::get<1>(index_)) : std::get<0>(index_)->ToString()) + ']';
}


std::string TruncInstr::ToString() const
{
    return Dest()->Name() + " = trunc " + Value()->ToString() +
        " to " + Type()->ToString();
}
std::string FtruncInstr::ToString() const
{
    return Dest()->Name() + " = ftrunc " + Value()->ToString() +
        " to " + Type()->ToString();
}
std::string ZextInstr::ToString() const
{
    return Dest()->Name() + " = zext " + Value()->ToString() +
        " to " + Type()->ToString();
}
std::string SextInstr::ToString() const
{
    return Dest()->Name() + " = sext " + Value()->ToString() +
        " to " + Type()->ToString();
}
std::string FextInstr::ToString() const
{
    return Dest()->Name() + " = fext " + Value()->ToString() +
        " to " + Type()->ToString();
}
std::string FtoUInstr::ToString() const
{
    return Dest()->Name() + " = ftou " + Value()->ToString() +
        " to " + Type()->ToString();
}
std::string FtoSInstr::ToString() const
{
    return Dest()->Name() + " = ftos " + Value()->ToString() +
        " to " + Type()->ToString();
}
std::string UtoFInstr::ToString() const
{
    return Dest()->Name() + " = utof " + Value()->ToString() +
        " to " + Type()->ToString();
}
std::string StoFInstr::ToString() const
{
    return Dest()->Name() + " = stof " + Value()->ToString() +
        " to " + Type()->ToString();
}
std::string PtrtoIInstr::ToString() const
{
    return Dest()->Name() + " = ptrtoi " + Value()->ToString() +
        " to " + Type()->ToString();
}
std::string ItoPtrInstr::ToString() const
{
    return Dest()->Name() + " = itoptr " + Value()->ToString() +
        " to " + Type()->ToString();
}
std::string BitcastInstr::ToString() const
{
    return Dest()->Name() + " = bitcast " + Value()->ToString() +
        " to " + Type()->ToString();
}


namespace std
{
string to_string(Condition c)
{
    switch (c)
    {
    case Condition::eq: return "eq";
    case Condition::ne: return "ne";
    case Condition::gt: return "gt";
    case Condition::le: return "le";
    case Condition::lt: return "lt";
    case Condition::ge: return "ge";
    default: return "";
    }
}
}

std::string IcmpInstr::ToString() const
{
    return result_->ToString() + " = icmp " + std::to_string(cond_) +
        ' ' + op1_->ToString() + ", " + op2_->ToString();
}
std::string FcmpInstr::ToString() const
{
    return result_->ToString() + " = fcmp " + std::to_string(cond_) +
        ' ' + op1_->ToString() + ", " + op2_->ToString();
}


std::string SelectInstr::ToString() const
{
    return result_->ToString() + " = select " + selty_->ToString() + " ? " +
        value1_->ToString() + " : " + value2_->ToString();
}


std::string PhiInstr::ToString() const
{
    std::string line = result_->ToString() + " = phi " + type_->ToString() + ' ';
    for (auto l = labels_.begin(); l < labels_.end() - 1; ++l)
        line += '[' + l->first->Name() + ", " +
            l->second->ToString() + "] ";
    line += '[' + labels_.back().first->Name() + ", " +
        labels_.back().second->ToString() + ']';
    return line;
}

void PhiInstr::AddBlockValPair(const BasicBlock* bb, const IROperand* op)
{
    labels_.push_back({ bb, op });
}
