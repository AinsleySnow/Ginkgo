#ifndef _INSTR_H_
#define _INSTR_H_

#include "IRType.h"
#include <memory>
#include <string>


enum class InstrType
{
    ret, br, swtch, call,

    add, sub, mul, div, mod,
    fadd, fsub, fmul, fdiv,
    shl, lshr, ashr, btand, btor, btxor,

    alloca, load, store, exval, setval, geteleptr,

    trunc, ftrunc, zext, sext, fext,
    ftou, ftos, utof, stof, ptrtoi, itoptr, bitcast,

    icmp, fcmp, select, phi
};


class Instr
{
public:
    Instr(InstrType instr) : instr_(instr) {}

    virtual ~Instr() {}
    virtual std::string ToString() const { return ""; }

    InstrType GetInstrType() const { return instr_; }


private:
    InstrType instr_{};
};


class RetInstr : public Instr
{
public:
    RetInstr() : Instr(InstrType::ret) {}
    RetInstr(const IRType* vt, const std::string& v) :
        Instr(InstrType::ret), valtype_(vt), value_(v) {}

    std::string ToString() const override
    {
        if (valtype_) return "ret " + valtype_->ToString() + ' ' + value_;
        else          return "ret void";
    }

private:
    const IRType* valtype_{};
    std::string value_{};
};


class BrInstr : public Instr
{
public:
    BrInstr(const BasicBlock* l) : Instr(InstrType::br), true_(l) {}
    BrInstr(const std::string& c, const BasicBlock* t, const BasicBlock* f) :
        Instr(InstrType::br), cond_(c), true_(t), false_(f) {}

    std::string ToString() const override
    {
        if (!false_) return "br label " + true_->GetName();
        else    return "br " + cond_ + ' ' + true_->GetName() + ' ' + false_->GetName();
    }


private:
    std::string cond_{};
    const BasicBlock* true_{};
    const BasicBlock* false_{};
};


class SwitchInstr : public Instr
{
public:
    using ValueBlkPair = std::vector<std::pair<std::string, const BasicBlock*>>;

    SwitchInstr(
        const std::string& i,
        const IntType* t,
        const ValueBlkPair& l
    ) : Instr(InstrType::swtch), ident_(i), type_(t), cases_(l) {}

    std::string ToString() const override
    {
        std::string caselist{ "[\n" };
        for (const auto& pair : cases_)
            caselist += "    " + pair.first + ": " + pair.second->GetName() + '\n';
        caselist += ']';
        return "switch " + type_->ToString() + ident_ + caselist;
    }


private:
    std::string ident_{};
    const IntType* type_{};
    ValueBlkPair cases_{};
};


class CallInstr : public Instr
{
public:
    using ArgList = std::vector<std::pair<const IRType*, std::string>>;

    CallInstr(const FuncType* proto, const Function* pfunc, const ArgList& args) :
        Instr(InstrType::call), proto_(proto), func_(pfunc), arglist_(args) {}

    std::string ToString() const override
    {
        std::string call = "call " + proto_->ToString() + ' ' + func_->GetName() + '(';
        for (const auto& pair : arglist_)
            call += pair.first->ToString() + ' ' + pair.second + ", ";
        return call + ')';
    }


private:
    const FuncType* proto_{};
    const Function* func_{};
    ArgList arglist_{};
};


class AddInstr : public Instr
{
public:
    AddInstr(const std::string& r, const IntType* t,
        const std::string& o1, const std::string& o2) :
        Instr(InstrType::add), result_(r), type_(t), lhs_(o1), rhs_(o2) {}

    std::string ToString() const override
    { return result_ + " = add " + type_->ToString() + ' ' + lhs_ + ' ' + rhs_; }


private:
    std::string result_{};
    const IntType* type_{};
    std::string lhs_{}, rhs_{};
};


class FaddInstr : public Instr
{
public:
    FaddInstr(const std::string& r, const FloatType* t,
        const std::string& o1, const std::string& o2) :
        Instr(InstrType::fadd), result_(r), type_(t), lhs_(o1), rhs_(o2) {}

    std::string ToString() const override
    { return result_ + " = fadd " + type_->ToString() + ' ' + lhs_ + ' ' + rhs_; }


private:
    std::string result_{};
    const FloatType* type_{};
    std::string lhs_{}, rhs_{};
};


class SubInstr : public Instr
{
public:
    SubInstr(const std::string& r, const IntType* t,
        const std::string& o1, const std::string& o2) :
        Instr(InstrType::sub), result_(r), type_(t), lhs_(o1), rhs_(o2) {}

    std::string ToString() const override
    { return result_ + " = sub " + type_->ToString() + ' ' + lhs_ + ' ' + rhs_; }


private:
    std::string result_{};
    const IntType* type_{};
    std::string lhs_{}, rhs_{};
};


class FsubInstr : public Instr
{
public:
    FsubInstr(const std::string& r, const FloatType* t,
        const std::string& o1, const std::string& o2) :
        Instr(InstrType::fsub), result_(r), type_(t), lhs_(o1), rhs_(o2) {}

    std::string ToString() const override
    { return result_ + " = fsub " + type_->ToString() + ' ' + lhs_ + ' ' + rhs_; }


private:
    std::string result_{};
    const FloatType* type_{};
    std::string lhs_{}, rhs_{};
};


class MulInstr : public Instr
{
public:
    MulInstr(const std::string& r, const IntType* t,
        const std::string& o1, const std::string& o2) :
        Instr(InstrType::mul), result_(r), type_(t), lhs_(o1), rhs_(o2) {}

    std::string ToString() const override
    { return result_ + " = mul " + type_->ToString() + ' ' + lhs_ + ' ' + rhs_; }


private:
    std::string result_{};
    const IntType* type_{};
    std::string lhs_{}, rhs_{};
};


class FmulInstr : public Instr
{
public:
    FmulInstr(const std::string& r, const FloatType* t,
        const std::string& o1, const std::string& o2) :
        Instr(InstrType::fmul), result_(r), type_(t), lhs_(o1), rhs_(o2) {}

    std::string ToString() const override
    { return result_ + " = fmul " + type_->ToString() + ' ' + lhs_ + ' ' + rhs_; }


private:
    std::string result_{};
    const FloatType* type_{};
    std::string lhs_{}, rhs_{};
};


class DivInstr : public Instr
{
public:
    DivInstr(const std::string& r, const IntType* t,
        const std::string& o1, const std::string& o2) :
        Instr(InstrType::div), result_(r), type_(t), lhs_(o1), rhs_(o2) {}

    std::string ToString() const override
    { return result_ + " = div " + type_->ToString() + ' ' + lhs_ + ' ' + rhs_; }


private:
    std::string result_{};
    const IntType* type_{};
    std::string lhs_{}, rhs_{};
};


class FdivInstr : public Instr
{
public:
    FdivInstr(const std::string& r, const FloatType* t,
        const std::string& o1, const std::string& o2) :
        Instr(InstrType::fdiv), result_(r), type_(t), lhs_(o1), rhs_(o2) {}

    std::string ToString() const override
    { return result_ + " = fdiv " + type_->ToString() + ' ' + lhs_ + ' ' + rhs_; }


private:
    std::string result_{};
    const FloatType* type_{};
    std::string lhs_{}, rhs_{};
};


class ModInstr : public Instr
{
public:
    ModInstr(const std::string& r, const IntType* t,
        const std::string& o1, const std::string& o2) :
        Instr(InstrType::mod), result_(r), type_(t), lhs_(o1), rhs_(o2) {}

    std::string ToString() const override
    { return result_ + " = mod " + type_->ToString() + ' ' + lhs_ + ' ' + rhs_; }


private:
    std::string result_{};
    const IntType* type_{};
    std::string lhs_{}, rhs_{};
};


class ShlInstr : public Instr
{
public:
    ShlInstr(const std::string& r, const IntType* t,
        const std::string& o1, const std::string& o2) :
        Instr(InstrType::shl), result_(r), type_(t), lhs_(o1), rhs_(o2) {}

    std::string ToString() const override
    { return result_ + " = shl " + type_->ToString() + ' ' + lhs_ + ' ' + rhs_; }


private:
    std::string result_{};
    const IntType* type_{};
    std::string lhs_{}, rhs_{};
};


class LshrInstr : public Instr
{
public:
    LshrInstr(const std::string& r, const IntType* t,
        const std::string& o1, const std::string& o2) :
        Instr(InstrType::lshr), result_(r), type_(t), lhs_(o1), rhs_(o2) {}

    std::string ToString() const override
    { return result_ + " = lshr " + type_->ToString() + ' ' + lhs_ + ' ' + rhs_; }


private:
    std::string result_{};
    const IntType* type_{};
    std::string lhs_{}, rhs_{};
};


class AshrInstr : public Instr
{
public:
    AshrInstr(const std::string& r, const IntType* t,
        const std::string& o1, const std::string& o2) :
        Instr(InstrType::ashr), result_(r), type_(t), lhs_(o1), rhs_(o2) {}

    std::string ToString() const override
    { return result_ + " = ashr " + type_->ToString() + ' ' + lhs_ + ' ' + rhs_; }


private:
    std::string result_{};
    const IntType* type_{};
    std::string lhs_{}, rhs_{};
};


class AndInstr : public Instr
{
public:
    AndInstr(const std::string& r, const IntType* t,
        const std::string& o1, const std::string& o2) :
        Instr(InstrType::btand), result_(r), type_(t), lhs_(o1), rhs_(o2) {}

    std::string ToString() const override
    { return result_ + " = and " + type_->ToString() + ' ' + lhs_ + ' ' + rhs_; }


private:
    std::string result_{};
    const IntType* type_{};
    std::string lhs_{}, rhs_{};
};


class OrInstr : public Instr
{
public:
    OrInstr(const std::string& r, const IntType* t,
        const std::string& o1, const std::string& o2) :
        Instr(InstrType::btor), result_(r), type_(t), lhs_(o1), rhs_(o2) {}

    std::string ToString() const override
    { return result_ + " = or " + type_->ToString() + ' ' + lhs_ + ' ' + rhs_; }


private:
    std::string result_{};
    const IntType* type_{};
    std::string lhs_{}, rhs_{};
};


class XorInstr : public Instr
{
public:
    XorInstr(const std::string& r, const IntType* t,
        const std::string& o1, const std::string& o2) :
        Instr(InstrType::btxor), result_(r), type_(t), lhs_(o1), rhs_(o2) {}

    std::string ToString() const override
    { return result_ + " = xor " + type_->ToString() + ' ' + lhs_ + ' ' + rhs_; }


private:
    std::string result_{};
    const IntType* type_{};
    std::string lhs_{}, rhs_{};
};


class AllocaInstr : public Instr
{
public:
    AllocaInstr(const std::string& r, const IRType* t) :
        Instr(InstrType::alloca), result_(r), type_(t) {}
    AllocaInstr(const std::string& r, const IRType* t, size_t n) :
        Instr(InstrType::alloca), result_(r), type_(t), num_(n) {}
    AllocaInstr(const std::string& r, const IRType* t, size_t n, size_t a) :
        Instr(InstrType::alloca), result_(r), type_(t), num_(n), align_(a) {}

    std::string ToString() const override
    {
        std::string line = result_ + " = alloca " + type_->ToString();
        if (num_ > 1)
            line += "i64 " + std::to_string(num_);
        if (align_ > 1)
            line += ", align " + std::to_string(align_);
        return line;
    }


private:
    std::string result_{};
    const IRType* type_{};
    size_t num_{ 1 };
    size_t align_{ 1 };
};


class LoadInstr : public Instr
{
public:
    LoadInstr(const std::string& r, const PtrType* t, const std::string& p) :
        Instr(InstrType::load), result_(r), type_(t), ptr_(p) {}
    LoadInstr(const std::string& r, const PtrType* t, const std::string& p, size_t a) :
        Instr(InstrType::load), result_(r), type_(t), ptr_(p), align_(a), volatile_(true) {}

    std::string ToString() const override
    {
        std::string line = result_ + " = " + (volatile_ ? "load " : "volatile load ");
        line += type_->Dereference()->ToString() + ", " + ptr_;
        if (align_ > 1)
            line += ", align" + std::to_string(align_);
        return line;
    }


private:
    std::string result_{};
    const PtrType* type_{};
    std::string ptr_{};
    size_t align_{ 1 };
    bool volatile_{};
};


class StoreInstr : public Instr
{
public:
    StoreInstr(const PtrType* pt, const std::string& v, const std::string& p, bool vol) :
        Instr(InstrType::store), ptrtype_(pt), value_(v),  ptr_(p), volatile_(vol) {}

    std::string ToString() const override
    {
        std::string line = volatile_ ? "volatile stroe " : "store";
        line += ptrtype_->Dereference()->ToString() + ' ' + value_ + ", " + ptrtype_->ToString() + ptr_;
        return line;
    }


private:
    const PtrType* ptrtype_{};
    std::string value_{};
    std::string ptr_{};
    bool volatile_{};
};


class ExtractValInstr : public Instr
{
public:
    ExtractValInstr(const std::string& r, const PtrType* t, const std::string& v, int i) :
        Instr(InstrType::exval), result_(r), ptrtype_(t), value_(v), index_(i) {}

    std::string ToString() const override
    {
        return result_ + " = extractval " + ptrtype_->ToString() + "* " +
            value_ + '[' + std::to_string(index_) + ']';
    }


private:
    std::string result_{};
    const PtrType* ptrtype_{};
    std::string value_{};
    int index_{};
};


class SetValInstr : public Instr
{
public:
    SetValInstr(const std::string& nv, const PtrType* t, const std::string& v, int i) :
        Instr(InstrType::setval), newval_(nv), ptrtype_(t), value_(v), index_(i) {}

    std::string ToString() const override
    {
        return "setval " + newval_ + ", " + ptrtype_->ToString() +
            value_ + '[' + std::to_string(index_) + ']';
    }


private:
    std::string newval_{};
    const PtrType* ptrtype_{};
    std::string value_{};
    int index_{};
};


class GetElePtrInstr : public Instr
{
public:
    GetElePtrInstr(const std::string& r, const PtrType* t, const std::string& v, int i) :
        Instr(InstrType::geteleptr), result_(r), ptrtype_(t), ptrvalue_(v), index_(i) {}

    std::string ToString() const override
    {
        return result_ + " = geteleptr " + ptrtype_->ToString() +
            ptrvalue_ + '[' + std::to_string(index_) + ']';
    }


private:
    std::string result_{};
    const PtrType* ptrtype_{};
    std::string ptrvalue_{};
    int index_{};
};


class TruncInstr : public Instr
{
public:
    TruncInstr(const std::string& r, const IntType* t1,
        const std::string& v, const IntType* t2) :
        Instr(InstrType::trunc), result_(r), type1_(t1), type2_(t2), value_(v) {}

    std::string ToString() const override
    {
        return result_ + " = trunc " + type1_->ToString() + ' ' +
            value_ + " to " + type2_->ToString();
    }


private:
    std::string result_{};
    const IntType* type1_{};
    const IntType* type2_{};
    std::string value_{};
};


class FtruncInstr : public Instr
{
public:
    FtruncInstr(const std::string& r, const FloatType* t1,
        const std::string& v, const FloatType* t2) :
        Instr(InstrType::ftrunc), result_(r), type1_(t1), type2_(t2), value_(v) {}

    std::string ToString() const override
    {
        return result_ + " = ftrunc " + type1_->ToString() + ' ' +
            value_ + " to " + type2_->ToString();
    }


private:
    std::string result_{};
    const FloatType* type1_{};
    const FloatType* type2_{};
    std::string value_{};
};


class ZextInstr : public Instr
{
public:
    ZextInstr(const std::string& r, const IntType* t1,
        const std::string& v, const IntType* t2) :
        Instr(InstrType::zext), result_(r), type1_(t1), type2_(t2), value_(v) {}

    std::string ToString() const override
    {
        return result_ + " = zext " + type1_->ToString() + ' ' +
            value_ + " to " + type2_->ToString();
    }


private:
    std::string result_{};
    const IntType* type1_{};
    const IntType* type2_{};
    std::string value_{};
};


class SextInstr : public Instr
{
public:
    SextInstr(const std::string& r, const IntType* t1,
        const std::string& v, const IntType* t2) :
        Instr(InstrType::sext), result_(r), type1_(t1), type2_(t2), value_(v) {}

    std::string ToString() const override
    {
        return result_ + " = sext " + type1_->ToString() + ' ' +
            value_ + " to " + type2_->ToString();
    }


private:
    std::string result_{};
    const IntType* type1_{};
    const IntType* type2_{};
    std::string value_{};
};


class FextInstr : public Instr
{
public:
    FextInstr(const std::string& r, const FloatType* t1,
        const std::string& v, const FloatType* t2) :
        Instr(InstrType::fext), result_(r), type1_(t1), type2_(t2), value_(v) {}

    std::string ToString() const override
    {
        return result_ + " = fext " + type1_->ToString() + ' ' +
            value_ + " to " + type2_->ToString();
    }


private:
    std::string result_{};
    const FloatType* type1_{};
    const FloatType* type2_{};
    std::string value_{};
};


class FtouInstr : public Instr
{
public:
    FtouInstr(const std::string& r, const FloatType* t1,
        const std::string& v, const IntType* t2) :
        Instr(InstrType::ftou), result_(r), type1_(t1), type2_(t2), value_(v) {}

    std::string ToString() const override
    {
        return result_ + " = ftou " + type1_->ToString() + ' ' +
            value_ + " to " + type2_->ToString();
    }


private:
    std::string result_{};
    const FloatType* type1_{};
    const IntType* type2_{};
    std::string value_{};
};


class FtosInstr : public Instr
{
public:
    FtosInstr(const std::string& r, const FloatType* t1,
        const std::string& v, const IntType* t2) :
        Instr(InstrType::ftos), result_(r), type1_(t1), type2_(t2), value_(v) {}

    std::string ToString() const override
    {
        return result_ + " = ftos " + type1_->ToString() + ' ' +
            value_ + " to " + type2_->ToString();
    }


private:
    std::string result_{};
    const FloatType* type1_{};
    const IntType* type2_{};
    std::string value_{};
};


class UtofInstr : public Instr
{
public:
    UtofInstr(const std::string& r, const IntType* t1,
        const std::string& v, const FloatType* t2) :
        Instr(InstrType::utof), result_(r), type1_(t1), type2_(t2), value_(v) {}

    std::string ToString() const override
    {
        return result_ + " = utof " + type1_->ToString() + ' ' +
            value_ + " to " + type2_->ToString();
    }


private:
    std::string result_{};
    const IntType* type1_{};
    const FloatType* type2_{};
    std::string value_{};
};


class StofInstr : public Instr
{
public:
    StofInstr(const std::string& r, const IntType* t1,
        const std::string& v, const FloatType* t2) :
        Instr(InstrType::stof), result_(r), type1_(t1), type2_(t2), value_(v) {}

    std::string ToString() const override
    {
        return result_ + " = stof " + type1_->ToString() + ' ' +
            value_ + " to " + type2_->ToString();
    }


private:
    std::string result_{};
    const IntType* type1_{};
    const FloatType* type2_{};
    std::string value_{};
};


class PtrtoiInstr : public Instr
{
public:
    PtrtoiInstr(const std::string& r, const PtrType* t1,
        const std::string& v, const IntType* t2) :
        Instr(InstrType::ptrtoi), result_(r), type1_(t1), type2_(t2), value_(v) {}

    std::string ToString() const override
    {
        return result_ + " = ptrtoi " + type1_->ToString() + ' ' +
            value_ + " to " + type2_->ToString();
    }


private:
    std::string result_{};
    const PtrType* type1_{};
    const IntType* type2_{};
    std::string value_{};
};


class ItoptrInstr : public Instr
{
public:
    ItoptrInstr(const std::string& r, const IntType* t1,
        const std::string& v, const PtrType* t2) :
        Instr(InstrType::itoptr), result_(r), type1_(t1), type2_(t2), value_(v) {}

    std::string ToString() const override
    {
        return result_ + " = itoptr " + type1_->ToString() + ' ' +
            value_ + " to " + type2_->ToString();
    }


private:
    std::string result_{};
    const IntType* type1_{};
    const PtrType* type2_{};
    std::string value_{};
};


class BitcastInstr : public Instr
{
public:
    BitcastInstr(const std::string& r, const IRType* t1,
        const std::string& v, const IRType* t2) :
        Instr(InstrType::bitcast), result_(r), type1_(t1), type2_(t2), value_(v) {}

    std::string ToString() const override
    {
        return result_ + " = bitcast " + type1_->ToString() + ' ' +
            value_ + " to " + type2_->ToString();
    }


private:
    std::string result_{};
    const IRType* type1_{};
    const IRType* type2_{};
    std::string value_{};
};


enum class Condition
{
    eq, ne, ugt, uge, ult, 
    ule, sgt, sge, slt, sle
};

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


class IcmpInstr : public Instr
{
public:
    IcmpInstr(const std::string& r, Condition c, const IntType* t,
        const std::string& o1, const std::string& o2) :
            Instr(InstrType::icmp), result_(r), cond_(c),
            type_(t), op1_(o1), op2_(o2) {}

    std::string ToString()
    {
        return result_ + " = icmp " + std::to_string(cond_) +
            ' ' + type_->ToString() + ' ' + op1_ + ' ' + op2_;
    }


private:
    std::string result_{};
    Condition cond_;
    const IntType* type_{};
    std::string op1_{};
    std::string op2_{};
};


class FcmpInstr : public Instr
{
public:
    FcmpInstr(const std::string& r, Condition c, const FloatType* t,
        const std::string& o1, const std::string& o2) :
            Instr(InstrType::fcmp), result_(r), cond_(c),
            type_(t), op1_(o1), op2_(o2) {}

    std::string ToString()
    {
        return result_ + " = fcmp " + std::to_string(cond_) +
            ' ' + type_->ToString() + ' ' + op1_ + ' ' + op2_;
    }


private:
    std::string result_{};
    Condition cond_;
    const FloatType* type_{};
    std::string op1_{};
    std::string op2_{};
};


class SelectInstr : public Instr
{
public:
    SelectInstr(const std::string& r, const std::string& s,
        bool c, const IRType* t,
        const std::string& v1, const std::string& v2) :
            Instr(InstrType::select), result_(r), selty_(s), cond_(c),
            type_(t), value1_(v1), value2_(v2) {}

    std::string ToString()
    {
        return result_ + " = select " + selty_ + ", " +
            (cond_ ? "true: " : "false: ") + type_->ToString() + 
            ' ' + value1_ + ' ' + value2_;
    }


private:
    std::string result_{};
    std::string selty_{};
    bool cond_{};
    const IRType* type_{};
    std::string value1_{};
    std::string value2_{};
};


class PhiInstr : public Instr
{
public:
    using BlockValPairList = std::vector<std::pair<const BasicBlock*, std::string>>;

    PhiInstr(const std::string& i, const IRType* t, const BlockValPairList& l) :
        Instr(InstrType::phi), result_(i), type_(t), labels_(l) {}

    std::string ToString()
    {
        std::string line = result_ + " = phi " + type_->ToString() + ' ';
        for (const auto& l : labels_)
            line += '[' + l.first->GetName() + ", " + l.second + "] ";
        return line;
    }


private:
    std::string result_{};
    const IRType* type_{};
    BlockValPairList labels_{};
};


#endif // _INSTR_H_
