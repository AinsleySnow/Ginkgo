#include "IR/Instr.h"
#include "IR/Value.h"
#include <climits>
#include <cfloat>


IntConst* IntConst::CreateIntConst(Function* func, unsigned long ul)
{
    if (ul < UINT8_MAX)
        return CreateIntConst(func, ul, IntType::GetInt8(false));
    else if (ul < UINT16_MAX)
        return CreateIntConst(func, ul, IntType::GetInt16(false));
    else if (ul < UINT32_MAX)
        return CreateIntConst(func, ul, IntType::GetInt32(false));
    else if (ul < UINT64_MAX)
        return CreateIntConst(func, ul, IntType::GetInt64(false));
    return nullptr;
}

IntConst* IntConst::CreateIntConst(Function* func, unsigned long ul, const IntType* t)
{
    auto intconst = std::make_unique<IntConst>(ul, t);
    auto raw = intconst.get();
    func->AddIROperand(std::move(intconst));
    return raw;
}

std::string IntConst::ToString() const
{
    unsigned long numcopy = 0;
    auto byte = reinterpret_cast<const char*>(&num_);

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    for (int i = 0; i < type_->Size(); ++i)
        reinterpret_cast<char*>(&numcopy)[i] = *(byte + i);
#else
    for (int i = 7; i >= 8 - type_->Size(); --i)
        reinterpret_cast<char*>(&numcopy)[i] = *(byte + i);
#endif

    if (type_->ToInteger()->IsSigned())
        return std::to_string((long)numcopy);
    else
        return std::to_string(numcopy);
}

FloatConst* FloatConst::CreateFloatConst(Function* func, double d)
{
    if (d < FLT_MAX)
        return CreateFloatConst(func, d, FloatType::GetFloat32());
    else if (d < DBL_MAX)
        return CreateFloatConst(func, d, FloatType::GetFloat64());
    return nullptr;
}

FloatConst* FloatConst::CreateFloatConst(Function* func, double d, const FloatType* t)
{
    auto floatconst = std::make_unique<FloatConst>(d, t);
    auto raw = floatconst.get();
    func->AddIROperand(std::move(floatconst));
    return raw;
}

std::string FloatConst::ToString() const
{
    if (type_->Size() == 4)
        return std::to_string((float)num_);
    else
        return std::to_string(num_);
}

Register* Register::CreateRegister(
    Function* func, const std::string& name, const IRType* ty)
{
    auto reg = std::make_unique<Register>(name, ty);
    auto raw = reg.get();
    func->AddIROperand(std::move(reg));
    return raw;
}

std::string Register::ToString() const
{
    return type_->ToString() + ' ' + name_;
}


std::string RetInstr::ToString() const
{
    if (retval_) return "ret " + retval_->ToString();
    else          return "ret void";
}


std::string BrInstr::ToString() const
{
    if (!cond_) return "br label " + true_->Name();
    else    return "br " + cond_->ToString() + ' ' + true_->Name() + ' ' + false_->Name();
}


std::string SwitchInstr::ToString() const
{
    std::string caselist{ default_->Name() + ", [\n" };
    for (const auto& pair : cases_)
        caselist += "    " + pair.first->ToString() +
            ": " + pair.second->Name() + '\n';
    caselist += ']';
    return "switch " + ident_->ToString() + ", " +
        default_->Name() + caselist;
}


std::string CallInstr::ToString() const
{
    std::string call = "call " + proto_->ToString() + ' ' + func_ + '(';
    for (const auto& arg : arglist_)
        call += arg->ToString() + ", ";
    return call + ')';
}


std::string AddInstr::ToString() const
{ return result_ + " = add " + lhs_->ToString() + ' ' + rhs_->ToString(); }
std::string FaddInstr::ToString() const
{ return result_ + " = fadd " + lhs_->ToString() + ' ' + rhs_->ToString(); }
std::string SubInstr::ToString() const
{ return result_ + " = sub " + lhs_->ToString() + ' ' + rhs_->ToString(); }
std::string FsubInstr::ToString() const
{ return result_ + " = fsub " + lhs_->ToString() + ' ' + rhs_->ToString(); }
std::string MulInstr::ToString() const
{ return result_ + " = mul " + lhs_->ToString() + ' ' + rhs_->ToString(); }
std::string FmulInstr::ToString() const
{ return result_ + " = fmul " + lhs_->ToString() + ' ' + rhs_->ToString(); }
std::string DivInstr::ToString() const
{ return result_ + " = div " + lhs_->ToString() + ' ' + rhs_->ToString(); }
std::string FdivInstr::ToString() const
{ return result_ + " = fdiv " + lhs_->ToString() + ' ' + rhs_->ToString(); }
std::string ModInstr::ToString() const
{ return result_ + " = mod " + lhs_->ToString() + ' ' + rhs_->ToString(); }
std::string ShlInstr::ToString() const
{ return result_ + " = shl " + lhs_->ToString() + ' ' + rhs_->ToString(); }
std::string LshrInstr::ToString() const
{ return result_ + " = lshr " + lhs_->ToString() + ' ' + rhs_->ToString(); }
std::string AshrInstr::ToString() const
{ return result_ + " = ashr " + lhs_->ToString() + ' ' + rhs_->ToString(); }
std::string AndInstr::ToString() const
{ return result_ + " = and " + lhs_->ToString() + ' ' + rhs_->ToString(); }
std::string OrInstr::ToString() const
{ return result_ + " = or " + lhs_->ToString() + ' ' + rhs_->ToString(); }
std::string XorInstr::ToString() const
{ return result_ + " = xor " + lhs_->ToString() + ' ' + rhs_->ToString(); }


std::string AllocaInstr::ToString() const
{
    std::string line = result_ + " = alloca " + type_->ToString();
    if (num_ > 1)
        line += "i64 " + std::to_string(num_);
    if (align_ > 1)
        line += ", align " + std::to_string(align_);
    return line;
}

std::string LoadInstr::ToString() const
{
    std::string line = result_ + " = " + (volatile_ ? "volatile load " : "load ");
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


std::string ExtractValInstr::ToString() const
{
    return result_ + " = extractval " + pointer_->ToString() +
        '[' + std::to_string(index_) + ']';
}

std::string SetValInstr::ToString() const
{
    return "setval " + newval_->ToString() + ", " +
        pointer_->ToString() + '[' + std::to_string(index_) + ']';
}

std::string GetElePtrInstr::ToString() const
{
    return result_ + " = geteleptr " + pointer_->ToString() +
        " [" + std::to_string(index_) + ']';
}


std::string TruncInstr::ToString() const
{
    return result_ + " = trunc " + value_->ToString() +
        " to " + type_->ToString();
}
std::string FtruncInstr::ToString() const
{
    return result_ + " = ftrunc " + value_->ToString() +
        " to " + type_->ToString();
}
std::string ZextInstr::ToString() const
{
    return result_ + " = zext " + value_->ToString() +
        " to " + type_->ToString();
}
std::string SextInstr::ToString() const
{
    return result_ + " = sext " + value_->ToString() +
        " to " + type_->ToString();
}
std::string FextInstr::ToString() const
{
    return result_ + " = fext " + value_->ToString() +
        " to " + type_->ToString();
}
std::string FtouInstr::ToString() const
{
    return result_ + " = ftou " + value_->ToString() +
        " to " + type_->ToString();
}
std::string FtosInstr::ToString() const
{
    return result_ + " = ftos " + value_->ToString() +
        " to " + type_->ToString();
}
std::string UtofInstr::ToString() const
{
    return result_ + " = utof " + value_->ToString() +
        " to " + type_->ToString();
}
std::string StofInstr::ToString() const
{
    return result_ + " = stof " + value_->ToString() +
        " to " + type_->ToString();
}
std::string PtrtoiInstr::ToString() const
{
    return result_ + " = ptrtoi " + value_->ToString() +
        " to " + type_->ToString();
}
std::string ItoptrInstr::ToString() const
{
    return result_ + " = itoptr " + value_->ToString() +
        " to " + type_->ToString();
}
std::string BitcastInstr::ToString() const
{
    return result_ + " = bitcast " + value_->ToString() +
        " to " + type_->ToString();
}


namespace std
{
string to_string(Condition c)
{
    switch (c)
    {
    case Condition::eq: return "eq";
    case Condition::ne: return "ne";
    case Condition::ugt: return "ugt";
    case Condition::uge: return "uge";
    case Condition::ult: return "ult";
    case Condition::ule: return "ule";
    case Condition::sgt: return "sgt";
    case Condition::sge: return "sge";
    case Condition::slt: return "slt";
    case Condition::sle: return "sle";
    default: return "";
    }
}
}

std::string IcmpInstr::ToString() const
{
    return result_ + " = icmp " + std::to_string(cond_) +
        ' ' + op1_->ToString() + ' ' + op2_->ToString();
}
std::string FcmpInstr::ToString() const
{
    return result_ + " = fcmp " + std::to_string(cond_) +
        ' ' + op1_->ToString() + ' ' + op2_->ToString();
}


std::string SelectInstr::ToString() const
{
    return result_ + " = select " + selty_->ToString() + " ? " +
        value1_->ToString() + ' ' + value2_->ToString();
}


std::string PhiInstr::ToString() const
{
    std::string line = result_ + " = phi " + type_->ToString() + '\n';
    for (const auto& l : labels_)
        line += '[' + l.first->Name() + ", " + l.second->ToString() + "] ";
    return line;
}

void PhiInstr::AddBlockValPair(const BasicBlock* bb, const IROperand* op)
{
    labels_.push_back({ bb, op });
}
