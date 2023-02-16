#ifndef _INSTR_H_
#define _INSTR_H_

#include "IR/IRType.h"
#include "IR/IROperand.h"
#include <memory>
#include <string>

class BasicBlock;


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
    static Instr* CreateInstr(BasicBlock*, std::unique_ptr<Instr>);
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
    RetInstr(const IROperand* rv) :
        Instr(InstrType::ret), retval_(rv) {}

    std::string ToString() const override;

private:
    const IROperand* retval_{};
};


class BrInstr : public Instr
{
public:
    BrInstr(const BasicBlock* l) : Instr(InstrType::br), true_(l) {}
    BrInstr(const IROperand* c, const BasicBlock* t, const BasicBlock* f) :
        Instr(InstrType::br), cond_(c), true_(t), false_(f) {}

    std::string ToString() const override;

    auto GetTrueBlk() { return true_; }
    auto GetFalseBlk() { return false_; }
    void SetTrueBlk(BasicBlock* blk) { true_ = blk; }
    void SetFalseBlk(BasicBlock* blk) { false_ = blk; }

private:
    const IROperand* cond_{};
    const BasicBlock* true_{};
    const BasicBlock* false_{};
};


class SwitchInstr : public Instr
{
public:
    using ValueBlkPair = std::vector<std::pair<const IntConst*, const BasicBlock*>>;

    SwitchInstr(const IROperand* i) : Instr(InstrType::swtch), ident_(i) {}

    std::string ToString() const override;

    void AddValueBlkPair(const IntConst* val, const BasicBlock* blk) { cases_.push_back({ val, blk }); }
    void SetDefault(const BasicBlock* bb) { default_ = bb; }

private:
    const IROperand* ident_{};
    ValueBlkPair cases_{};
    const BasicBlock* default_{};
};


class CallInstr : public Instr
{
public:
    CallInstr(const std::string& result, const FuncType* proto, const std::string& name) :
        Instr(InstrType::call), proto_(proto), func_(name) {}
    CallInstr(const std::string& result, const Register* addr) :
        Instr(InstrType::call), proto_(addr->Type()->ToFunction()), funcaddr_(addr) {}

    std::string ToString() const override;

    void AddArgv(const IROperand* argv) { arglist_.push_back(argv); }

private:
    std::string result_{};
    const FuncType* proto_{};

    std::string func_{};
    const Register* funcaddr_{};

    std::vector<const IROperand*> arglist_{};
};


class AddInstr : public Instr
{
public:
    AddInstr(const std::string& r,
        const IROperand* o1, const IROperand* o2) :
        Instr(InstrType::add), result_(r), lhs_(o1), rhs_(o2) {}

    std::string ToString() const override;

private:
    std::string result_{};
    const IROperand *lhs_{}, *rhs_{};
};


class FaddInstr : public Instr
{
public:
    FaddInstr(const std::string& r,
        const IROperand* o1, const IROperand* o2) :
        Instr(InstrType::fadd), result_(r), lhs_(o1), rhs_(o2) {}

    std::string ToString() const override;

private:
    std::string result_{};
    const IROperand *lhs_{}, *rhs_{};
};


class SubInstr : public Instr
{
public:
    SubInstr(const std::string& r, 
        const IROperand* o1, const IROperand* o2) :
        Instr(InstrType::sub), result_(r), lhs_(o1), rhs_(o2) {}

    std::string ToString() const override;

private:
    std::string result_{};
    const IROperand *lhs_{}, *rhs_{};
};


class FsubInstr : public Instr
{
public:
    FsubInstr(const std::string& r,
        const IROperand* o1, const IROperand* o2) :
        Instr(InstrType::fsub), result_(r), lhs_(o1), rhs_(o2) {}

    std::string ToString() const override;

private:
    std::string result_{};
    const IROperand *lhs_{}, *rhs_{};
};


class MulInstr : public Instr
{
public:
    MulInstr(const std::string& r,
        const IROperand* o1, const IROperand* o2) :
        Instr(InstrType::mul), result_(r), lhs_(o1), rhs_(o2) {}

    std::string ToString() const override;

private:
    std::string result_{};
    const IROperand *lhs_{}, *rhs_{};
};


class FmulInstr : public Instr
{
public:
    FmulInstr(const std::string& r,
        const IROperand* o1, const IROperand* o2) :
        Instr(InstrType::fmul), result_(r), lhs_(o1), rhs_(o2) {}

    std::string ToString() const override;

private:
    std::string result_{};
    const IROperand *lhs_{}, *rhs_{};
};


class DivInstr : public Instr
{
public:
    DivInstr(const std::string& r,
        const IROperand* o1, const IROperand* o2) :
        Instr(InstrType::div), result_(r), lhs_(o1), rhs_(o2) {}

    std::string ToString() const override;

private:
    std::string result_{};
    const IROperand *lhs_{}, *rhs_{};
};


class FdivInstr : public Instr
{
public:
    FdivInstr(const std::string& r,
        const IROperand* o1, const IROperand* o2) :
        Instr(InstrType::fdiv), result_(r), lhs_(o1), rhs_(o2) {}

    std::string ToString() const override;

private:
    std::string result_{};
    const IROperand *lhs_{}, *rhs_{};
};


class ModInstr : public Instr
{
public:
    ModInstr(const std::string& r,
        const IROperand* o1, const IROperand* o2) :
        Instr(InstrType::mod), result_(r), lhs_(o1), rhs_(o2) {}

    std::string ToString() const override;

private:
    std::string result_{};
    const IROperand *lhs_{}, *rhs_{};
};


class ShlInstr : public Instr
{
public:
    ShlInstr(const std::string& r,
        const IROperand* o1, const IROperand* o2) :
        Instr(InstrType::shl), result_(r), lhs_(o1), rhs_(o2) {}

    std::string ToString() const override;

private:
    std::string result_{};
    const IROperand *lhs_{}, *rhs_{};
};


class LshrInstr : public Instr
{
public:
    LshrInstr(const std::string& r,
        const IROperand* o1, const IROperand* o2) :
        Instr(InstrType::lshr), result_(r), lhs_(o1), rhs_(o2) {}

    std::string ToString() const override;

private:
    std::string result_{};
    const IROperand *lhs_{}, *rhs_{};
};


class AshrInstr : public Instr
{
public:
    AshrInstr(const std::string& r,
        const IROperand* o1, const IROperand* o2) :
        Instr(InstrType::ashr), result_(r), lhs_(o1), rhs_(o2) {}

    std::string ToString() const override;

private:
    std::string result_{};
    const IROperand *lhs_{}, *rhs_{};
};


class AndInstr : public Instr
{
public:
    AndInstr(const std::string& r,
        const IROperand* o1, const IROperand* o2) :
        Instr(InstrType::btand), result_(r), lhs_(o1), rhs_(o2) {}

    std::string ToString() const override;

private:
    std::string result_{};
    const IntType* type_{};
    const IROperand *lhs_{}, *rhs_{};
};


class OrInstr : public Instr
{
public:
    OrInstr(const std::string& r,
        const IROperand* o1, const IROperand* o2) :
        Instr(InstrType::btor), result_(r), lhs_(o1), rhs_(o2) {}

    std::string ToString() const override;

private:
    std::string result_{};
    const IntType* type_{};
    const IROperand *lhs_{}, *rhs_{};
};


class XorInstr : public Instr
{
public:
    XorInstr(const std::string& r,
        const IROperand* o1, const IROperand* o2) :
        Instr(InstrType::btxor), result_(r), lhs_(o1), rhs_(o2) {}

    std::string ToString() const override;

private:
    std::string result_{};
    const IntType* type_{};
    const IROperand *lhs_{}, *rhs_{};
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

    std::string ToString() const override;

private:
    std::string result_{};
    const IRType* type_{};
    size_t num_{ 1 };
    size_t align_{ 1 };
};


class LoadInstr : public Instr
{
public:
    LoadInstr(const std::string& r, const Register* p) :
        Instr(InstrType::load), result_(r), pointer_(p) {}
    LoadInstr(const std::string& r, const Register* p, size_t a) :
        Instr(InstrType::load), result_(r), pointer_(p),
        align_(a) {}
    LoadInstr(const std::string& r, const Register* p, size_t a, bool vol) :
        Instr(InstrType::load), result_(r), pointer_(p),
        align_(a), volatile_(vol) {}

    std::string ToString() const override;

private:
    std::string result_{};
    const IROperand* pointer_{};
    size_t align_{ 1 };
    bool volatile_{};
};


class StoreInstr : public Instr
{
public:
    StoreInstr(const IROperand* v, const Register* p, bool vol) :
        Instr(InstrType::store), value_(v), pointer_(p), volatile_(vol) {}

    std::string ToString() const override;

private:
    const IROperand* value_{};
    const Register* pointer_{};
    bool volatile_{};
};


class ExtractValInstr : public Instr
{
public:
    ExtractValInstr(const std::string& r, const Register* p, int i) :
        Instr(InstrType::exval), result_(r), pointer_(p), index_(i) {}

    std::string ToString() const override;

private:
    std::string result_{};
    const Register* pointer_{};
    int index_{};
};


class SetValInstr : public Instr
{
public:
    SetValInstr(const IROperand* nv, const Register* p, int i) :
        Instr(InstrType::setval), newval_(nv), pointer_(p), index_(i) {}

    std::string ToString() const override;

private:
    const IROperand* newval_{};
    const Register* pointer_{};
    int index_{};
};


class GetElePtrInstr : public Instr
{
public:
    GetElePtrInstr(const std::string& r, const Register* p, int i) :
        Instr(InstrType::geteleptr), result_(r), pointer_(p), index_(i) {}

    std::string ToString() const override;

private:
    std::string result_{};
    const Register* pointer_{};
    int index_{};
};


class TruncInstr : public Instr
{
public:
    TruncInstr(
        const std::string& r, const IntType* t, const Register* v
    ) : Instr(InstrType::trunc), result_(r), type_(t), value_(v) {}

    std::string ToString() const override;

private:
    std::string result_{};
    const IntType* type_{};
    const Register* value_{};
};


class FtruncInstr : public Instr
{
public:
    FtruncInstr(
        const std::string& r, const FloatType* t, const Register* v
    ) : Instr(InstrType::ftrunc), result_(r), type_(t), value_(v) {}

    std::string ToString() const override;

private:
    std::string result_{};
    const FloatType* type_{};
    const Register* value_{};
};


class ZextInstr : public Instr
{
public:
    ZextInstr(
        const std::string& r, const IntType* t, const Register* v
    ) : Instr(InstrType::zext), result_(r), type_(t), value_(v) {}

    std::string ToString() const override;

private:
    std::string result_{};
    const IntType* type_{};
    const Register* value_{};
};


class SextInstr : public Instr
{
public:
    SextInstr(
        const std::string& r, const IntType* t, const Register* v
    ) : Instr(InstrType::sext), result_(r), type_(t), value_(v) {}

    std::string ToString() const override;

private:
    std::string result_{};
    const IntType* type_{};
    const Register* value_{};
};


class FextInstr : public Instr
{
public:
    FextInstr(
        const std::string& r, const FloatType* t, const Register* v
    ) : Instr(InstrType::fext), result_(r), type_(t), value_(v) {}

    std::string ToString() const override;

private:
    std::string result_{};
    const FloatType* type_{};
    const Register* value_{};
};


class FtouInstr : public Instr
{
public:
    FtouInstr(
        const std::string& r, const IntType* t, const Register* v
    ) : Instr(InstrType::ftou), result_(r), type_(t), value_(v) {}

    std::string ToString() const override;

private:
    std::string result_{};
    const IntType* type_{};
    const Register* value_{};
};


class FtosInstr : public Instr
{
public:
    FtosInstr(
        const std::string& r, const IntType* t, const Register* v
    ) : Instr(InstrType::ftos), result_(r), type_(t), value_(v) {}

    std::string ToString() const override;

private:
    std::string result_{};
    const IntType* type_{};
    const Register* value_{};
};


class UtofInstr : public Instr
{
public:
    UtofInstr(
        const std::string& r, const FloatType* t1, const Register* v
    ) : Instr(InstrType::utof), result_(r), type_(t1), value_(v) {}

    std::string ToString() const override;

private:
    std::string result_{};
    const FloatType* type_{};
    const Register* value_{};
};


class StofInstr : public Instr
{
public:
    StofInstr(
        const std::string& r, const FloatType* t, const Register* v
    ) : Instr(InstrType::stof), result_(r), type_(t), value_(v) {}

    std::string ToString() const override;

private:
    std::string result_{};
    const FloatType* type_{};
    const Register* value_{};
};


class PtrtoiInstr : public Instr
{
public:
    PtrtoiInstr(
        const std::string& r, const IntType* t, const Register* v
    ) : Instr(InstrType::ptrtoi), result_(r), type_(t), value_(v) {}

    std::string ToString() const override;

private:
    std::string result_{};
    const IntType* type_{};
    const Register* value_{};
};


class ItoptrInstr : public Instr
{
public:
    ItoptrInstr(
        const std::string& r, const PtrType* t, const Register* v
    ) : Instr(InstrType::itoptr), result_(r), type_(t), value_(v) {}

    std::string ToString() const override;

private:
    std::string result_{};
    const PtrType* type_{};
    const Register* value_{};
};


class BitcastInstr : public Instr
{
public:
    BitcastInstr(
        const std::string& r, const IRType* t, const Register* v
    ) : Instr(InstrType::bitcast), result_(r), type_(t), value_(v) {}

    std::string ToString() const override;

private:
    std::string result_{};
    const IRType* type_{};
    const Register* value_{};
};


enum class Condition
{
    eq, ne,
    ge, gt, ugt, uge, sgt, sge,
    le, lt, ult, ule, slt, sle
};


class IcmpInstr : public Instr
{
public:
    IcmpInstr(const std::string& r, Condition c,
        const IROperand* o1, const IROperand* o2) :
            Instr(InstrType::icmp), result_(r), cond_(c),
            op1_(o1), op2_(o2) {}

    std::string ToString() const override;

private:
    std::string result_{};
    Condition cond_;
    const IROperand* op1_{};
    const IROperand* op2_{};
};


class FcmpInstr : public Instr
{
public:
    FcmpInstr(const std::string& r, Condition c,
        const IROperand* o1, const IROperand* o2) :
            Instr(InstrType::fcmp), result_(r), cond_(c),
            op1_(o1), op2_(o2) {}

    std::string ToString() const override;

private:
    std::string result_{};
    Condition cond_;
    const IROperand* op1_{};
    const IROperand* op2_{};
};


class SelectInstr : public Instr
{
public:
    SelectInstr(const std::string& r, const IROperand* s,
        bool c, const IROperand* v1, const IROperand* v2) :
            Instr(InstrType::select), result_(r), selty_(s),
            cond_(c), value1_(v1), value2_(v2) {}

    std::string ToString() const override;

private:
    std::string result_{};
    const IROperand* selty_{};
    bool cond_{};
    const IROperand* value1_{};
    const IROperand* value2_{};
};


class PhiInstr : public Instr
{
public:
    using BlockValPairList = std::vector<std::pair<const BasicBlock*, const IROperand*>>;

    PhiInstr(const std::string& r, const IRType* t) :
        Instr(InstrType::phi), result_(r), type_(t) {}

    std::string ToString() const override;
    void AddBlockValPair(const BasicBlock*, const IROperand*);

private:
    std::string result_{};
    const IRType* type_{};
    BlockValPairList labels_{};
};


#endif // _INSTR_H_
