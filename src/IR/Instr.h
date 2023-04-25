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

    auto& ReturnValue() { return retval_; }
    auto ReturnValue() const { return retval_; }

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

    auto& Cond() { return cond_; }
    auto Cond() const { return cond_; }
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
protected:
    static bool ClassOf(const BinaryInstr const*) { return true; }
    static bool ClassOf(const Instr const* i)
    {
        return int(i->ID()) >= int(InstrId::add) &&
            int(i->ID()) <= int(InstrId::btxor); 
    }

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
    AllocaInstr(const Register* r, const IRType* t) :
        Instr(InstrId::alloca), result_(r), type_(t) {}
    AllocaInstr(const Register* r, const IRType* t, size_t n) :
        Instr(InstrId::alloca), result_(r), type_(t), num_(n) {}
    AllocaInstr(const Register* r, const IRType* t, size_t n, size_t a) :
        Instr(InstrId::alloca), result_(r), type_(t), num_(n), align_(a) {}

    std::string ToString() const override;

    auto& Result() { return result_; }
    auto Result() const { return result_; }
    auto Type() const { return type_; }
    auto Num() const { return num_; }
    auto Align() const { return align_; }

private:
    const Register* result_{};
    const IRType* type_{};
    size_t num_{ 1 };
    size_t align_{ type_->Size() };
};


class LoadInstr : public Instr
{
protected:
    static bool ClassOf(const LoadInstr const*) { return true; }
    static bool ClassOf(const Instr const* i) { return i->ID() == InstrId::load; }

public:
    LoadInstr(const Register* r, const Register* p) :
        Instr(InstrId::load), result_(r), pointer_(p) {}
    LoadInstr(const Register* r, const Register* p, size_t a) :
        Instr(InstrId::load), result_(r), pointer_(p),
        align_(a) {}
    LoadInstr(const Register* r, const Register* p, size_t a, bool vol) :
        Instr(InstrId::load), result_(r), pointer_(p),
        align_(a), volatile_(vol) {}

    std::string ToString() const override;

    auto& Result() { return result_; }
    auto Result() const { return result_; }
    auto& Pointer() { return pointer_; }
    auto Pointer() const { return pointer_; }
    auto Align() const { return align_; }
    auto Volatile() const { return volatile_; }

private:
    const Register* result_{};
    const Register* pointer_{};
    size_t align_{ result_->Type()->Size() };
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

    auto& Value() { return value_; }
    auto Value() const { return value_; }
    auto& Dest() { return pointer_; }
    auto Dest() const { return pointer_; }

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


class ConvertInstr : public Instr
{
public:
    ConvertInstr(
        InstrId id, const Register* r, const IRType* t, const Register* v
    ) : Instr(id), result_(r), type_(t), value_(v) {}

    auto& Dest() { return result_; }
    auto Dest() const { return result_; }
    auto& Type() { return type_; }
    auto Type() const { return type_; }
    auto& Value() { return value_; }
    auto Value() const { return value_; }

private:
    const Register* result_{};
    const IRType* type_{};
    const Register* value_{};
};


class TruncInstr : public ConvertInstr
{
protected:
    static bool ClassOf(const TruncInstr const*) { return true; }
    static bool ClassOf(const Instr const* i) { return i->ID() == InstrId::trunc; }

public:
    TruncInstr(
        const Register* r, const IntType* t, const Register* v
    ) : ConvertInstr(InstrId::trunc, r, t, v) {}

    std::string ToString() const override;
};


class FtruncInstr : public ConvertInstr
{
protected:
    static bool ClassOf(const FtruncInstr const*) { return true; }
    static bool ClassOf(const Instr const* i) { return i->ID() == InstrId::ftrunc; }

public:
    FtruncInstr(
        const Register* r, const FloatType* t, const Register* v
    ) : ConvertInstr(InstrId::ftrunc, r, t, v) {}

    std::string ToString() const override;
};


class ZextInstr : public ConvertInstr
{
protected:
    static bool ClassOf(const ZextInstr const*) { return true; }
    static bool ClassOf(const Instr const* i) { return i->ID() == InstrId::zext; }

public:
    ZextInstr(
        const Register* r, const IntType* t, const Register* v
    ) : ConvertInstr(InstrId::zext, r, t, v) {}

    std::string ToString() const override;
};


class SextInstr : public ConvertInstr
{
protected:
    static bool ClassOf(const SextInstr const*) { return true; }
    static bool ClassOf(const Instr const* i) { return i->ID() == InstrId::sext; }

public:
    SextInstr(
        const Register* r, const IntType* t, const Register* v
    ) : ConvertInstr(InstrId::sext, r, t, v) {}

    std::string ToString() const override;
};


class FextInstr : public ConvertInstr
{
protected:
    static bool ClassOf(const FextInstr const*) { return true; }
    static bool ClassOf(const Instr const* i) { return i->ID() == InstrId::fext; }

public:
    FextInstr(
        const Register* r, const FloatType* t, const Register* v
    ) : ConvertInstr(InstrId::fext, r, t, v) {}

    std::string ToString() const override;
};


class FtoUInstr : public ConvertInstr
{
protected:
    static bool ClassOf(const FtoUInstr const*) { return true; }
    static bool ClassOf(const Instr const* i) { return i->ID() == InstrId::ftou; }

public:
    FtoUInstr(
        const Register* r, const IntType* t, const Register* v
    ) : ConvertInstr(InstrId::ftou, r, t, v) {}

    std::string ToString() const override;
};


class FtoSInstr : public ConvertInstr
{
protected:
    static bool ClassOf(const FtoSInstr const*) { return true; }
    static bool ClassOf(const Instr const* i) { return i->ID() == InstrId::ftos; }

public:
    FtoSInstr(
        const Register* r, const IntType* t, const Register* v
    ) : ConvertInstr(InstrId::ftos, r, t, v) {}

    std::string ToString() const override;
};


class UtoFInstr : public ConvertInstr
{
protected:
    static bool ClassOf(const UtoFInstr const*) { return true; }
    static bool ClassOf(const Instr const* i) { return i->ID() == InstrId::utof; }

public:
    UtoFInstr(
        const Register* r, const FloatType* t, const Register* v
    ) : ConvertInstr(InstrId::utof, r, t, v) {}

    std::string ToString() const override;
};


class StoFInstr : public ConvertInstr
{
protected:
    static bool ClassOf(const StoFInstr const*) { return true; }
    static bool ClassOf(const Instr const* i) { return i->ID() == InstrId::stof; }

public:
    StoFInstr(
        const Register* r, const FloatType* t, const Register* v
    ) : ConvertInstr(InstrId::stof, r, t, v) {}

    std::string ToString() const override;
};


class PtrtoIInstr : public ConvertInstr
{
protected:
    static bool ClassOf(const PtrtoIInstr const*) { return true; }
    static bool ClassOf(const Instr const* i) { return i->ID() == InstrId::ptrtoi; }

public:
    PtrtoIInstr(
        const Register* r, const IntType* t, const Register* v
    ) : ConvertInstr(InstrId::ptrtoi, r, t, v) {}

    std::string ToString() const override;
};


class ItoPtrInstr : public ConvertInstr
{
protected:
    static bool ClassOf(const ItoPtrInstr const*) { return true; }
    static bool ClassOf(const Instr const* i) { return i->ID() == InstrId::itoptr; }

public:
    ItoPtrInstr(
        const Register* r, const PtrType* t, const Register* v
    ) : ConvertInstr(InstrId::itoptr, r, t, v) {}

    std::string ToString() const override;
};


class BitcastInstr : public ConvertInstr
{
protected:
    static bool ClassOf(const BitcastInstr const*) { return true; }
    static bool ClassOf(const Instr const* i) { return i->ID() == InstrId::bitcast; }

public:
    BitcastInstr(
        const Register* r, const IRType* t, const Register* v
    ) : ConvertInstr(InstrId::bitcast, r, t, v) {}

    std::string ToString() const override;
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
    IcmpInstr(const Register* r, Condition c,
        const IROperand* o1, const IROperand* o2) :
            Instr(InstrId::icmp), result_(r), cond_(c),
            op1_(o1), op2_(o2) {}

    std::string ToString() const override;

    auto Cond() const { return cond_; }
    auto Op1() const { return op1_; }
    auto Op2() const { return op2_; }
    auto Result() const { return result_; }

private:
    const Register* result_{};
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
    FcmpInstr(const Register* r, Condition c,
        const IROperand* o1, const IROperand* o2) :
            Instr(InstrId::fcmp), result_(r), cond_(c),
            op1_(o1), op2_(o2) {}

    std::string ToString() const override;

    auto Cond() const { return cond_; }
    auto Op1() const { return op1_; }
    auto Op2() const { return op2_; }
    auto Result() const { return result_; }

private:
    const Register* result_{};
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
    SelectInstr(const Register* r, const IROperand* s,
        bool c, const IROperand* v1, const IROperand* v2) :
            Instr(InstrId::select), result_(r), selty_(s),
            cond_(c), value1_(v1), value2_(v2) {}

    std::string ToString() const override;

    auto& Result() { return result_; }
    auto Result() const { return result_; }
    auto& SelType() { return selty_; }
    auto& Cond() { return cond_; }
    auto CondPair() const { return std::make_pair(selty_, cond_); }
    auto& Value1() { return value1_; }
    auto Value1() const { return value1_; }
    auto& Value2() { return value2_; }
    auto Value2() const { return value2_; }

private:
    const Register* result_{};
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

    PhiInstr(const Register* r, const IRType* t) :
        Instr(InstrId::phi), result_(r), type_(t) {}

    std::string ToString() const override;
    void AddBlockValPair(const BasicBlock*, const IROperand*);

private:
    const Register* result_{};
    const IRType* type_{};
    BlockValPairList labels_{};
};


#endif // _INSTR_H_
