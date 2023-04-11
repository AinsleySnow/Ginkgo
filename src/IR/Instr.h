#ifndef _INSTR_H_
#define _INSTR_H_

#include "IR/IRType.h"
#include "IR/IROperand.h"
#include "utils/DynCast.h"
#include <memory>
#include <string>

class BasicBlock;
class IRVisitor;


class Instr
{
protected:
    enum class InstrId
    {
        ret, br, swtch, call,

        add, sub, mul, div, mod,
        fadd, fsub, fmul, fdiv,
        shl, lshr, ashr, btand, btor, btxor,

        alloca, load, store, exval, setval, geteleptr,

        trunc, ftrunc, zext, sext, fext,
        ftou, ftos, utof, stof, ptrtoi, itoptr, bitcast,

        icmp, fcmp, select, phi, spill, restore
    };

    static bool ClassOf(const Instr* const) { return true; }


public:
    static Instr* CreateInstr(BasicBlock*, std::unique_ptr<Instr>);
    Instr(InstrId instr) : id_(instr) {}

    virtual ~Instr() {}
    virtual std::string ToString() const { return ""; }
    virtual void Accept(IRVisitor*) {}

    ENABLE_IS;
    ENABLE_AS;

    InstrId ID() const { return id_; }
    bool IsControlInstr() const 
    { 
        return id_ == InstrId::br ||
            id_ == InstrId::ret || 
            id_ == InstrId::swtch;
    }

private:
    InstrId id_{};
};


class RetInstr : public Instr
{
protected:
    static bool ClassOf(const RetInstr const*) { return true; }
    static bool ClassOf(const Instr const* i) { return i->ID() == InstrId::ret; }

public:
    RetInstr() : Instr(InstrId::ret) {}
    RetInstr(const IROperand* rv) :
        Instr(InstrId::ret), retval_(rv) {}

    std::string ToString() const override;


private:
    const IROperand* retval_{};
};


class BrInstr : public Instr
{
protected:
    static bool ClassOf(const BrInstr const*) { return true; }
    static bool ClassOf(const Instr const* i) { return i->ID() == InstrId::br; }

public:
    BrInstr(const BasicBlock* l) : Instr(InstrId::br), true_(l) {}
    BrInstr(const IROperand* c, const BasicBlock* t, const BasicBlock* f) :
        Instr(InstrId::br), cond_(c), true_(t), false_(f) {}

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
protected:
    static bool ClassOf(const SwitchInstr const*) { return true; }
    static bool ClassOf(const Instr const* i) { return i->ID() == InstrId::swtch; }

public:
    using ValueBlkPair = std::vector<std::pair<const IntConst*, const BasicBlock*>>;

    SwitchInstr(const IROperand* i) : Instr(InstrId::swtch), ident_(i) {}

    std::string ToString() const override;

    void AddValueBlkPair(const IntConst* val, const BasicBlock* blk) { cases_.push_back({ val, blk }); }
    void SetDefault(const BasicBlock* bb) { default_ = bb; }
    auto GetDefault() { return default_; }

private:
    const IROperand* ident_{};
    ValueBlkPair cases_{};
    const BasicBlock* default_{};
};


class CallInstr : public Instr
{
protected:
    static bool ClassOf(const CallInstr const*) { return true; }
    static bool ClassOf(const Instr const* i) { return i->ID() == InstrId::call; }

public:
    CallInstr(const std::string& result, const FuncType* proto, const std::string& name) :
        Instr(InstrId::call), result_(result), proto_(proto), func_(name) {}
    CallInstr(const std::string& result, const Register* addr) :
        Instr(InstrId::call), result_(result), funcaddr_(addr) {}

    std::string ToString() const override;

    void AddArgv(const IROperand* argv) { arglist_.push_back(argv); }

private:
    std::string result_{};
    const FuncType* proto_{};

    std::string func_{};
    const Register* funcaddr_{};

    std::vector<const IROperand*> arglist_{};
};


class BinaryInstr : public Instr
{
public:
    BinaryInstr(InstrId id, const Register* r,
        const IROperand* o1, const IROperand* o2) :
        Instr(id), result_(r), lhs_(o1), rhs_(o2) {}

    auto& Lhs() { return lhs_; }
    auto Lhs() const { return lhs_; }
    auto& Rhs() { return rhs_; }
    auto Rhs() const { return rhs_; }
    auto& Result() { return result_; }
    auto Result() const { return result_; }

private:
    const Register* result_{};
    const IROperand *lhs_{}, *rhs_{};
};


class AddInstr : public BinaryInstr
{
protected:
    static bool ClassOf(const AddInstr const*) { return true; }
    static bool ClassOf(const Instr const* i) { return i->ID() == InstrId::add; }

public:
    AddInstr(const Register* r, const IROperand* o1, const IROperand* o2) :
        BinaryInstr(InstrId::add, r, o1, o2) {}

    std::string ToString() const override;
    void Accept(IRVisitor*) override;
};


class FaddInstr : public BinaryInstr
{
protected:
    static bool ClassOf(const FaddInstr const*) { return true; }
    static bool ClassOf(const Instr const* i) { return i->ID() == InstrId::fadd; }

public:
    FaddInstr(const Register* r, const IROperand* o1, const IROperand* o2) :
        BinaryInstr(InstrId::fadd, r, o1, o2) {}

    std::string ToString() const override;
    void Accept(IRVisitor*) override;
};


class SubInstr : public BinaryInstr
{
protected:
    static bool ClassOf(const SubInstr const*) { return true; }
    static bool ClassOf(const Instr const* i) { return i->ID() == InstrId::sub; }

public:
    SubInstr(const Register* r, const IROperand* o1, const IROperand* o2) :
        BinaryInstr(InstrId::sub, r, o1, o2) {}

    std::string ToString() const override;
    void Accept(IRVisitor*) override;
};


class FsubInstr : public BinaryInstr
{
protected:
    static bool ClassOf(const FsubInstr const*) { return true; }
    static bool ClassOf(const Instr const* i) { return i->ID() == InstrId::fsub; }

public:
    FsubInstr(const Register* r, const IROperand* o1, const IROperand* o2) :
        BinaryInstr(InstrId::fsub, r, o1, o2) {}

    std::string ToString() const override;
    void Accept(IRVisitor*) override;
};


class MulInstr : public BinaryInstr
{
protected:
    static bool ClassOf(const MulInstr const*) { return true; }
    static bool ClassOf(const Instr const* i) { return i->ID() == InstrId::mul; }

public:
    MulInstr(const Register* r, const IROperand* o1, const IROperand* o2) :
        BinaryInstr(InstrId::mul, r, o1, o2) {}

    std::string ToString() const override;
    void Accept(IRVisitor*) override;
};


class FmulInstr : public BinaryInstr
{
protected:
    static bool ClassOf(const FmulInstr const*) { return true; }
    static bool ClassOf(const Instr const* i) { return i->ID() == InstrId::fmul; }

public:
    FmulInstr(const Register* r, const IROperand* o1, const IROperand* o2) :
        BinaryInstr(InstrId::fmul, r, o1, o2) {}

    std::string ToString() const override;
    void Accept(IRVisitor*) override;
};


class DivInstr : public BinaryInstr
{
protected:
    static bool ClassOf(const DivInstr const*) { return true; }
    static bool ClassOf(const Instr const* i) { return i->ID() == InstrId::div; }

public:
    DivInstr(const Register* r, const IROperand* o1, const IROperand* o2) :
        BinaryInstr(InstrId::div, r, o1, o2) {}

    std::string ToString() const override;
    void Accept(IRVisitor*) override;
};


class FdivInstr : public BinaryInstr
{
protected:
    static bool ClassOf(const FdivInstr const*) { return true; }
    static bool ClassOf(const Instr const* i) { return i->ID() == InstrId::fdiv; }

public:
    FdivInstr(const Register* r, const IROperand* o1, const IROperand* o2) :
        BinaryInstr(InstrId::fdiv, r, o1, o2) {}

    std::string ToString() const override;
    void Accept(IRVisitor*) override;
};


class ModInstr : public BinaryInstr
{
protected:
    static bool ClassOf(const ModInstr const*) { return true; }
    static bool ClassOf(const Instr const* i) { return i->ID() == InstrId::mod; }

public:
    ModInstr(const Register* r, const IROperand* o1, const IROperand* o2) :
        BinaryInstr(InstrId::mod, r, o1, o2) {}

    std::string ToString() const override;
    void Accept(IRVisitor*) override;
};


class ShlInstr : public BinaryInstr
{
protected:
    static bool ClassOf(const ShlInstr const*) { return true; }
    static bool ClassOf(const Instr const* i) { return i->ID() == InstrId::shl; }

public:
    ShlInstr(const Register* r, const IROperand* o1, const IROperand* o2) :
        BinaryInstr(InstrId::shl, r, o1, o2) {}

    std::string ToString() const override;
    void Accept(IRVisitor*) override;
};


class LshrInstr : public BinaryInstr
{
protected:
    static bool ClassOf(const LshrInstr const*) { return true; }
    static bool ClassOf(const Instr const* i) { return i->ID() == InstrId::lshr; }

public:
    LshrInstr(const Register* r, const IROperand* o1, const IROperand* o2) :
        BinaryInstr(InstrId::lshr, r, o1, o2) {}

    std::string ToString() const override;
    void Accept(IRVisitor*) override;
};


class AshrInstr : public BinaryInstr
{
protected:
    static bool ClassOf(const AshrInstr const*) { return true; }
    static bool ClassOf(const Instr const* i) { return i->ID() == InstrId::ashr; }

public:
    AshrInstr(const Register* r, const IROperand* o1, const IROperand* o2) :
        BinaryInstr(InstrId::ashr, r, o1, o2) {}

    std::string ToString() const override;
    void Accept(IRVisitor*) override;
};


class AndInstr : public BinaryInstr
{
protected:
    static bool ClassOf(const AndInstr const*) { return true; }
    static bool ClassOf(const Instr const* i) { return i->ID() == InstrId::btand; }

public:
    AndInstr(const Register* r, const IROperand* o1, const IROperand* o2) :
        BinaryInstr(InstrId::btand, r, o1, o2) {}

    std::string ToString() const override;
    void Accept(IRVisitor*) override;
};


class OrInstr : public BinaryInstr
{
protected:
    static bool ClassOf(const OrInstr const*) { return true; }
    static bool ClassOf(const Instr const* i) { return i->ID() == InstrId::btor; }

public:
    OrInstr(const Register* r, const IROperand* o1, const IROperand* o2) :
        BinaryInstr(InstrId::btor, r, o1, o2) {}

    std::string ToString() const override;
    void Accept(IRVisitor*) override;
};


class XorInstr : public BinaryInstr
{
protected:
    static bool ClassOf(const XorInstr const*) { return true; }
    static bool ClassOf(const Instr const* i) { return i->ID() == InstrId::btxor; }

public:
    XorInstr(const Register* r, const IROperand* o1, const IROperand* o2) :
        BinaryInstr(InstrId::btxor, r, o1, o2) {}

    std::string ToString() const override;
    void Accept(IRVisitor*) override;
};


class AllocaInstr : public Instr
{
protected:
    static bool ClassOf(const AllocaInstr const*) { return true; }
    static bool ClassOf(const Instr const* i) { return i->ID() == InstrId::alloca; }

public:
    AllocaInstr(const std::string& r, const IRType* t) :
        Instr(InstrId::alloca), result_(r), type_(t) {}
    AllocaInstr(const std::string& r, const IRType* t, size_t n) :
        Instr(InstrId::alloca), result_(r), type_(t), num_(n) {}
    AllocaInstr(const std::string& r, const IRType* t, size_t n, size_t a) :
        Instr(InstrId::alloca), result_(r), type_(t), num_(n), align_(a) {}

    std::string ToString() const override;

private:
    std::string result_{};
    const IRType* type_{};
    size_t num_{ 1 };
    size_t align_{ 1 };
};


class LoadInstr : public Instr
{
protected:
    static bool ClassOf(const LoadInstr const*) { return true; }
    static bool ClassOf(const Instr const* i) { return i->ID() == InstrId::load; }

public:
    LoadInstr(const std::string& r, const Register* p) :
        Instr(InstrId::load), result_(r), pointer_(p) {}
    LoadInstr(const std::string& r, const Register* p, size_t a) :
        Instr(InstrId::load), result_(r), pointer_(p),
        align_(a) {}
    LoadInstr(const std::string& r, const Register* p, size_t a, bool vol) :
        Instr(InstrId::load), result_(r), pointer_(p),
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
protected:
    static bool ClassOf(const StoreInstr const*) { return true; }
    static bool ClassOf(const Instr const* i) { return i->ID() == InstrId::store; }

public:
    StoreInstr(const IROperand* v, const Register* p, bool vol) :
        Instr(InstrId::store), value_(v), pointer_(p), volatile_(vol) {}

    std::string ToString() const override;

private:
    const IROperand* value_{};
    const Register* pointer_{};
    bool volatile_{};
};


class ExtractValInstr : public Instr
{
protected:
    static bool ClassOf(const ExtractValInstr const*) { return true; }
    static bool ClassOf(const Instr const* i) { return i->ID() == InstrId::exval; }

public:
    ExtractValInstr(const std::string& r, const Register* p, const IROperand* i) :
        Instr(InstrId::exval), result_(r), pointer_(p), index_(i) {}

    std::string ToString() const override;

private:
    std::string result_{};
    const Register* pointer_{};
    const IROperand* index_{};
};


class SetValInstr : public Instr
{
protected:
    static bool ClassOf(const SetValInstr const*) { return true; }
    static bool ClassOf(const Instr const* i) { return i->ID() == InstrId::setval; }

public:
    SetValInstr(const IROperand* nv, const Register* p, const IROperand* i) :
        Instr(InstrId::setval), newval_(nv), pointer_(p), index_(i) {}

    std::string ToString() const override;

private:
    const IROperand* newval_{};
    const Register* pointer_{};
    const IROperand* index_{};
};


class GetElePtrInstr : public Instr
{
protected:
    static bool ClassOf(const GetElePtrInstr const*) { return true; }
    static bool ClassOf(const Instr const* i) { return i->ID() == InstrId::geteleptr; }

public:
    GetElePtrInstr(const std::string& r, const Register* p, const IROperand* i) :
        Instr(InstrId::geteleptr), result_(r), pointer_(p), index_(i) {}

    std::string ToString() const override;

private:
    std::string result_{};
    const Register* pointer_{};
    const IROperand* index_{};
};


class TruncInstr : public Instr
{
protected:
    static bool ClassOf(const TruncInstr const*) { return true; }
    static bool ClassOf(const Instr const* i) { return i->ID() == InstrId::trunc; }

public:
    TruncInstr(
        const std::string& r, const IntType* t, const Register* v
    ) : Instr(InstrId::trunc), result_(r), type_(t), value_(v) {}

    std::string ToString() const override;

private:
    std::string result_{};
    const IntType* type_{};
    const Register* value_{};
};


class FtruncInstr : public Instr
{
protected:
    static bool ClassOf(const FtruncInstr const*) { return true; }
    static bool ClassOf(const Instr const* i) { return i->ID() == InstrId::ftrunc; }

public:
    FtruncInstr(
        const std::string& r, const FloatType* t, const Register* v
    ) : Instr(InstrId::ftrunc), result_(r), type_(t), value_(v) {}

    std::string ToString() const override;

private:
    std::string result_{};
    const FloatType* type_{};
    const Register* value_{};
};


class ZextInstr : public Instr
{
protected:
    static bool ClassOf(const ZextInstr const*) { return true; }
    static bool ClassOf(const Instr const* i) { return i->ID() == InstrId::zext; }

public:
    ZextInstr(
        const std::string& r, const IntType* t, const Register* v
    ) : Instr(InstrId::zext), result_(r), type_(t), value_(v) {}

    std::string ToString() const override;

private:
    std::string result_{};
    const IntType* type_{};
    const Register* value_{};
};


class SextInstr : public Instr
{
protected:
    static bool ClassOf(const SextInstr const*) { return true; }
    static bool ClassOf(const Instr const* i) { return i->ID() == InstrId::sext; }

public:
    SextInstr(
        const std::string& r, const IntType* t, const Register* v
    ) : Instr(InstrId::sext), result_(r), type_(t), value_(v) {}

    std::string ToString() const override;

private:
    std::string result_{};
    const IntType* type_{};
    const Register* value_{};
};


class FextInstr : public Instr
{
protected:
    static bool ClassOf(const FextInstr const*) { return true; }
    static bool ClassOf(const Instr const* i) { return i->ID() == InstrId::fext; }

public:
    FextInstr(
        const std::string& r, const FloatType* t, const Register* v
    ) : Instr(InstrId::fext), result_(r), type_(t), value_(v) {}

    std::string ToString() const override;

private:
    std::string result_{};
    const FloatType* type_{};
    const Register* value_{};
};


class FtoUInstr : public Instr
{
protected:
    static bool ClassOf(const FtoUInstr const*) { return true; }
    static bool ClassOf(const Instr const* i) { return i->ID() == InstrId::ftou; }

public:
    FtoUInstr(
        const std::string& r, const IntType* t, const Register* v
    ) : Instr(InstrId::ftou), result_(r), type_(t), value_(v) {}

    std::string ToString() const override;

private:
    std::string result_{};
    const IntType* type_{};
    const Register* value_{};
};


class FtoSInstr : public Instr
{
protected:
    static bool ClassOf(const FtoSInstr const*) { return true; }
    static bool ClassOf(const Instr const* i) { return i->ID() == InstrId::ftos; }

public:
    FtoSInstr(
        const std::string& r, const IntType* t, const Register* v
    ) : Instr(InstrId::ftos), result_(r), type_(t), value_(v) {}

    std::string ToString() const override;

private:
    std::string result_{};
    const IntType* type_{};
    const Register* value_{};
};


class UtoFInstr : public Instr
{
protected:
    static bool ClassOf(const UtoFInstr const*) { return true; }
    static bool ClassOf(const Instr const* i) { return i->ID() == InstrId::utof; }

public:
    UtoFInstr(
        const std::string& r, const FloatType* t1, const Register* v
    ) : Instr(InstrId::utof), result_(r), type_(t1), value_(v) {}

    std::string ToString() const override;

private:
    std::string result_{};
    const FloatType* type_{};
    const Register* value_{};
};


class StoFInstr : public Instr
{
protected:
    static bool ClassOf(const StoFInstr const*) { return true; }
    static bool ClassOf(const Instr const* i) { return i->ID() == InstrId::stof; }

public:
    StoFInstr(
        const std::string& r, const FloatType* t, const Register* v
    ) : Instr(InstrId::stof), result_(r), type_(t), value_(v) {}

    std::string ToString() const override;

private:
    std::string result_{};
    const FloatType* type_{};
    const Register* value_{};
};


class PtrtoIInstr : public Instr
{
protected:
    static bool ClassOf(const PtrtoIInstr const*) { return true; }
    static bool ClassOf(const Instr const* i) { return i->ID() == InstrId::ptrtoi; }

public:
    PtrtoIInstr(
        const std::string& r, const IntType* t, const Register* v
    ) : Instr(InstrId::ptrtoi), result_(r), type_(t), value_(v) {}

    std::string ToString() const override;

private:
    std::string result_{};
    const IntType* type_{};
    const Register* value_{};
};


class ItoPtrInstr : public Instr
{
protected:
    static bool ClassOf(const ItoPtrInstr const*) { return true; }
    static bool ClassOf(const Instr const* i) { return i->ID() == InstrId::itoptr; }

public:
    ItoPtrInstr(
        const std::string& r, const PtrType* t, const Register* v
    ) : Instr(InstrId::itoptr), result_(r), type_(t), value_(v) {}

    std::string ToString() const override;

private:
    std::string result_{};
    const PtrType* type_{};
    const Register* value_{};
};


class BitcastInstr : public Instr
{
protected:
    static bool ClassOf(const BitcastInstr const*) { return true; }
    static bool ClassOf(const Instr const* i) { return i->ID() == InstrId::bitcast; }

public:
    BitcastInstr(
        const std::string& r, const IRType* t, const Register* v
    ) : Instr(InstrId::bitcast), result_(r), type_(t), value_(v) {}

    std::string ToString() const override;

private:
    std::string result_{};
    const IRType* type_{};
    const Register* value_{};
};


enum class Condition
{
    eq, ne,
    ge, gt,
    le, lt
};


class IcmpInstr : public Instr
{
protected:
    static bool ClassOf(const IcmpInstr const*) { return true; }
    static bool ClassOf(const Instr const* i) { return i->ID() == InstrId::icmp; }

public:
    IcmpInstr(const std::string& r, Condition c,
        const IROperand* o1, const IROperand* o2) :
            Instr(InstrId::icmp), result_(r), cond_(c),
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
protected:
    static bool ClassOf(const FcmpInstr const*) { return true; }
    static bool ClassOf(const Instr const* i) { return i->ID() == InstrId::fcmp; }

public:
    FcmpInstr(const std::string& r, Condition c,
        const IROperand* o1, const IROperand* o2) :
            Instr(InstrId::fcmp), result_(r), cond_(c),
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
protected:
    static bool ClassOf(const SelectInstr const*) { return true; }
    static bool ClassOf(const Instr const* i) { return i->ID() == InstrId::select; }

public:
    SelectInstr(const std::string& r, const IROperand* s,
        bool c, const IROperand* v1, const IROperand* v2) :
            Instr(InstrId::select), result_(r), selty_(s),
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
protected:
    static bool ClassOf(const PhiInstr const*) { return true; }
    static bool ClassOf(const Instr const* i) { return i->ID() == InstrId::phi; }

public:
    using BlockValPairList = std::vector<std::pair<const BasicBlock*, const IROperand*>>;

    PhiInstr(const std::string& r, const IRType* t) :
        Instr(InstrId::phi), result_(r), type_(t) {}

    std::string ToString() const override;
    void AddBlockValPair(const BasicBlock*, const IROperand*);

private:
    std::string result_{};
    const IRType* type_{};
    BlockValPairList labels_{};
};


class SpillInstr : public Instr
{
protected:
    static bool ClassOf(const SpillInstr const*) { return true; }
    static bool ClassOf(const Instr const* i) { return i->ID() == InstrId::spill; }

public:
    SpillInstr(const std::string& r, const Register* v) :
        Instr(InstrId::spill), result_(r), value_(v) {}

    std::string ToString() const override;

private:
    std::string result_{};
    const Register* value_{};
};


class RestoreInstr : public Instr
{
protected:
    static bool ClassOf(const RestoreInstr const*) { return true; }
    static bool ClassOf(const Instr const* i) { return i->ID() == InstrId::restore; }

public:
    RestoreInstr(const std::string& r, const Register* v) :
        Instr(InstrId::restore), result_(r), value_(v) {}

    std::string ToString() const override;

private:
    std::string result_{};
    const Register* value_{};
};


#endif // _INSTR_H_
