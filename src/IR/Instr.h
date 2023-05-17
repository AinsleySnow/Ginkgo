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
public:
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
    InstrId id_{};

    static Instr* CreateInstr(BasicBlock*, std::unique_ptr<Instr>);
    Instr(InstrId instr) : id_(instr) {}

    virtual ~Instr() {}
    virtual std::string ToString() const { return ""; }
    virtual void Accept(IRVisitor*) {}

    ENABLE_IS;
    ENABLE_AS;

    bool IsControlInstr() const 
    { 
        return id_ == InstrId::br ||
            id_ == InstrId::ret || 
            id_ == InstrId::swtch;
    }
};


class RetInstr : public Instr
{
public:
    static bool ClassOf(const RetInstr* const) { return true; }
    static bool ClassOf(const Instr* const i) { return i->id_ == InstrId::ret; }

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
public:
    static bool ClassOf(const BrInstr* const) { return true; }
    static bool ClassOf(const Instr* const i) { return i->id_ == InstrId::br; }

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
public:
    static bool ClassOf(const SwitchInstr* const) { return true; }
    static bool ClassOf(const Instr* const i) { return i->id_ == InstrId::swtch; }

    using ValueBlkPair = std::vector<std::pair<const IntConst*, const BasicBlock*>>;

    SwitchInstr(const IROperand* i) : Instr(InstrId::swtch), ident_(i) {}

    std::string ToString() const override;

    void AddValueBlkPair(const IntConst* val, const BasicBlock* blk) { cases_.push_back({ val, blk }); }
    const auto& GetValueBlkPairs() const { return cases_; }
    void SetDefault(const BasicBlock* bb) { default_ = bb; }
    auto GetDefault() const { return default_; }
    auto GetIdent() const { return ident_; }

private:
    const IROperand* ident_{};
    ValueBlkPair cases_{};
    const BasicBlock* default_{};
};


class CallInstr : public Instr
{
public:
    static bool ClassOf(const CallInstr* const) { return true; }
    static bool ClassOf(const Instr* const i) { return i->id_ == InstrId::call; }

    CallInstr(const Register* result, const FuncType* proto, const std::string& name) :
        Instr(InstrId::call), result_(result), proto_(proto), func_(name) {}
    CallInstr(const Register* result, const Register* addr) :
        Instr(InstrId::call), result_(result), funcaddr_(addr) {}

    std::string ToString() const override;

    void AddArgv(const IROperand* argv) { arglist_.push_back(argv); }
    const auto& ArgvList() const { return arglist_; }
    auto Result() const { return result_; }
    auto Proto() const { return proto_; }
    auto FuncName() const { return func_; }
    auto FuncAddr() const { return funcaddr_; }

private:
    const Register* result_{};
    const FuncType* proto_{};

    std::string func_{};
    const Register* funcaddr_{};

    std::vector<const IROperand*> arglist_{};
};


class BinaryInstr : public Instr
{
public:
    static bool ClassOf(const BinaryInstr* const) { return true; }
    static bool ClassOf(const Instr* const i)
    {
        return int(i->id_) >= int(InstrId::add) &&
            int(i->id_) <= int(InstrId::btxor); 
    }

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
public:
    static bool ClassOf(const AddInstr* const) { return true; }
    static bool ClassOf(const Instr* const i) { return i->id_ == InstrId::add; }

    AddInstr(const Register* r, const IROperand* o1, const IROperand* o2) :
        BinaryInstr(InstrId::add, r, o1, o2) {}

    std::string ToString() const override;
    void Accept(IRVisitor*) override;
};


class FaddInstr : public BinaryInstr
{
public:
    static bool ClassOf(const FaddInstr* const) { return true; }
    static bool ClassOf(const Instr* const i) { return i->id_ == InstrId::fadd; }

    FaddInstr(const Register* r, const IROperand* o1, const IROperand* o2) :
        BinaryInstr(InstrId::fadd, r, o1, o2) {}

    std::string ToString() const override;
    void Accept(IRVisitor*) override;
};


class SubInstr : public BinaryInstr
{
public:
    static bool ClassOf(const SubInstr* const) { return true; }
    static bool ClassOf(const Instr* const i) { return i->id_ == InstrId::sub; }

    SubInstr(const Register* r, const IROperand* o1, const IROperand* o2) :
        BinaryInstr(InstrId::sub, r, o1, o2) {}

    std::string ToString() const override;
    void Accept(IRVisitor*) override;
};


class FsubInstr : public BinaryInstr
{
public:
    static bool ClassOf(const FsubInstr* const) { return true; }
    static bool ClassOf(const Instr* const i) { return i->id_ == InstrId::fsub; }

    FsubInstr(const Register* r, const IROperand* o1, const IROperand* o2) :
        BinaryInstr(InstrId::fsub, r, o1, o2) {}

    std::string ToString() const override;
    void Accept(IRVisitor*) override;
};


class MulInstr : public BinaryInstr
{
public:
    static bool ClassOf(const MulInstr* const) { return true; }
    static bool ClassOf(const Instr* const i) { return i->id_ == InstrId::mul; }

    MulInstr(const Register* r, const IROperand* o1, const IROperand* o2) :
        BinaryInstr(InstrId::mul, r, o1, o2) {}

    std::string ToString() const override;
    void Accept(IRVisitor*) override;
};


class FmulInstr : public BinaryInstr
{
public:
    static bool ClassOf(const FmulInstr* const) { return true; }
    static bool ClassOf(const Instr* const i) { return i->id_ == InstrId::fmul; }

    FmulInstr(const Register* r, const IROperand* o1, const IROperand* o2) :
        BinaryInstr(InstrId::fmul, r, o1, o2) {}

    std::string ToString() const override;
    void Accept(IRVisitor*) override;
};


class DivInstr : public BinaryInstr
{
public:
    static bool ClassOf(const DivInstr* const) { return true; }
    static bool ClassOf(const Instr* const i) { return i->id_ == InstrId::div; }

    DivInstr(const Register* r, const IROperand* o1, const IROperand* o2) :
        BinaryInstr(InstrId::div, r, o1, o2) {}

    std::string ToString() const override;
    void Accept(IRVisitor*) override;
};


class FdivInstr : public BinaryInstr
{
public:
    static bool ClassOf(const FdivInstr* const) { return true; }
    static bool ClassOf(const Instr* const i) { return i->id_ == InstrId::fdiv; }

    FdivInstr(const Register* r, const IROperand* o1, const IROperand* o2) :
        BinaryInstr(InstrId::fdiv, r, o1, o2) {}

    std::string ToString() const override;
    void Accept(IRVisitor*) override;
};


class ModInstr : public BinaryInstr
{
public:
    static bool ClassOf(const ModInstr* const) { return true; }
    static bool ClassOf(const Instr* const i) { return i->id_ == InstrId::mod; }

    ModInstr(const Register* r, const IROperand* o1, const IROperand* o2) :
        BinaryInstr(InstrId::mod, r, o1, o2) {}

    std::string ToString() const override;
    void Accept(IRVisitor*) override;
};


class ShlInstr : public BinaryInstr
{
public:
    static bool ClassOf(const ShlInstr* const) { return true; }
    static bool ClassOf(const Instr* const i) { return i->id_ == InstrId::shl; }

    ShlInstr(const Register* r, const IROperand* o1, const IROperand* o2) :
        BinaryInstr(InstrId::shl, r, o1, o2) {}

    std::string ToString() const override;
    void Accept(IRVisitor*) override;
};


class LshrInstr : public BinaryInstr
{
public:
    static bool ClassOf(const LshrInstr* const) { return true; }
    static bool ClassOf(const Instr* const i) { return i->id_ == InstrId::lshr; }

    LshrInstr(const Register* r, const IROperand* o1, const IROperand* o2) :
        BinaryInstr(InstrId::lshr, r, o1, o2) {}

    std::string ToString() const override;
    void Accept(IRVisitor*) override;
};


class AshrInstr : public BinaryInstr
{
public:
    static bool ClassOf(const AshrInstr* const) { return true; }
    static bool ClassOf(const Instr* const i) { return i->id_ == InstrId::ashr; }

    AshrInstr(const Register* r, const IROperand* o1, const IROperand* o2) :
        BinaryInstr(InstrId::ashr, r, o1, o2) {}

    std::string ToString() const override;
    void Accept(IRVisitor*) override;
};


class AndInstr : public BinaryInstr
{
public:
    static bool ClassOf(const AndInstr* const) { return true; }
    static bool ClassOf(const Instr* const i) { return i->id_ == InstrId::btand; }

    AndInstr(const Register* r, const IROperand* o1, const IROperand* o2) :
        BinaryInstr(InstrId::btand, r, o1, o2) {}

    std::string ToString() const override;
    void Accept(IRVisitor*) override;
};


class OrInstr : public BinaryInstr
{
public:
    static bool ClassOf(const OrInstr* const) { return true; }
    static bool ClassOf(const Instr* const i) { return i->id_ == InstrId::btor; }

    OrInstr(const Register* r, const IROperand* o1, const IROperand* o2) :
        BinaryInstr(InstrId::btor, r, o1, o2) {}

    std::string ToString() const override;
    void Accept(IRVisitor*) override;
};


class XorInstr : public BinaryInstr
{
public:
    static bool ClassOf(const XorInstr* const) { return true; }
    static bool ClassOf(const Instr* const i) { return i->id_ == InstrId::btxor; }

    XorInstr(const Register* r, const IROperand* o1, const IROperand* o2) :
        BinaryInstr(InstrId::btxor, r, o1, o2) {}

    std::string ToString() const override;
    void Accept(IRVisitor*) override;
};


class AllocaInstr : public Instr
{
public:
    static bool ClassOf(const AllocaInstr* const) { return true; }
    static bool ClassOf(const Instr* const i) { return i->id_ == InstrId::alloca; }

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
public:
    static bool ClassOf(const LoadInstr* const) { return true; }
    static bool ClassOf(const Instr* const i) { return i->id_ == InstrId::load; }

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
public:
    static bool ClassOf(const StoreInstr* const) { return true; }
    static bool ClassOf(const Instr* const i) { return i->id_ == InstrId::store; }

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
public:
    static bool ClassOf(const ExtractValInstr* const) { return true; }
    static bool ClassOf(const Instr* const i) { return i->id_ == InstrId::exval; }

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
public:
    static bool ClassOf(const SetValInstr* const) { return true; }
    static bool ClassOf(const Instr* const i) { return i->id_ == InstrId::setval; }

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
public:
    static bool ClassOf(const GetElePtrInstr* const) { return true; }
    static bool ClassOf(const Instr* const i) { return i->id_ == InstrId::geteleptr; }

    GetElePtrInstr(const Register* r, const Register* p, const IROperand* i) :
        Instr(InstrId::geteleptr), result_(r), pointer_(p), index_(i) {}

    std::string ToString() const override;

    auto Result() const { return result_; }
    auto Pointer() const { return pointer_; }
    auto Index() const { return index_; }

private:
    const Register* result_{};
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
public:
    static bool ClassOf(const TruncInstr* const) { return true; }
    static bool ClassOf(const Instr* const i) { return i->id_ == InstrId::trunc; }

    TruncInstr(
        const Register* r, const IntType* t, const Register* v
    ) : ConvertInstr(InstrId::trunc, r, t, v) {}

    std::string ToString() const override;
};


class FtruncInstr : public ConvertInstr
{
public:
    static bool ClassOf(const FtruncInstr* const) { return true; }
    static bool ClassOf(const Instr* const i) { return i->id_ == InstrId::ftrunc; }

    FtruncInstr(
        const Register* r, const FloatType* t, const Register* v
    ) : ConvertInstr(InstrId::ftrunc, r, t, v) {}

    std::string ToString() const override;
};


class ZextInstr : public ConvertInstr
{
public:
    static bool ClassOf(const ZextInstr* const) { return true; }
    static bool ClassOf(const Instr* const i) { return i->id_ == InstrId::zext; }

    ZextInstr(
        const Register* r, const IntType* t, const Register* v
    ) : ConvertInstr(InstrId::zext, r, t, v) {}

    std::string ToString() const override;
};


class SextInstr : public ConvertInstr
{
public:
    static bool ClassOf(const SextInstr* const) { return true; }
    static bool ClassOf(const Instr* const i) { return i->id_ == InstrId::sext; }

    SextInstr(
        const Register* r, const IntType* t, const Register* v
    ) : ConvertInstr(InstrId::sext, r, t, v) {}

    std::string ToString() const override;
};


class FextInstr : public ConvertInstr
{
public:
    static bool ClassOf(const FextInstr* const) { return true; }
    static bool ClassOf(const Instr* const i) { return i->id_ == InstrId::fext; }

    FextInstr(
        const Register* r, const FloatType* t, const Register* v
    ) : ConvertInstr(InstrId::fext, r, t, v) {}

    std::string ToString() const override;
};


class FtoUInstr : public ConvertInstr
{
public:
    static bool ClassOf(const FtoUInstr* const) { return true; }
    static bool ClassOf(const Instr* const i) { return i->id_ == InstrId::ftou; }

    FtoUInstr(
        const Register* r, const IntType* t, const Register* v
    ) : ConvertInstr(InstrId::ftou, r, t, v) {}

    std::string ToString() const override;
};


class FtoSInstr : public ConvertInstr
{
public:
    static bool ClassOf(const FtoSInstr* const) { return true; }
    static bool ClassOf(const Instr* const i) { return i->id_ == InstrId::ftos; }

public:
    FtoSInstr(
        const Register* r, const IntType* t, const Register* v
    ) : ConvertInstr(InstrId::ftos, r, t, v) {}

    std::string ToString() const override;
};


class UtoFInstr : public ConvertInstr
{
public:
    static bool ClassOf(const UtoFInstr* const) { return true; }
    static bool ClassOf(const Instr* const i) { return i->id_ == InstrId::utof; }

    UtoFInstr(
        const Register* r, const FloatType* t, const Register* v
    ) : ConvertInstr(InstrId::utof, r, t, v) {}

    std::string ToString() const override;
};


class StoFInstr : public ConvertInstr
{
public:
    static bool ClassOf(const StoFInstr* const) { return true; }
    static bool ClassOf(const Instr* const i) { return i->id_ == InstrId::stof; }

    StoFInstr(
        const Register* r, const FloatType* t, const Register* v
    ) : ConvertInstr(InstrId::stof, r, t, v) {}

    std::string ToString() const override;
};


class PtrtoIInstr : public ConvertInstr
{
public:
    static bool ClassOf(const PtrtoIInstr* const) { return true; }
    static bool ClassOf(const Instr* const i) { return i->id_ == InstrId::ptrtoi; }

    PtrtoIInstr(
        const Register* r, const IntType* t, const Register* v
    ) : ConvertInstr(InstrId::ptrtoi, r, t, v) {}

    std::string ToString() const override;
};


class ItoPtrInstr : public ConvertInstr
{
public:
    static bool ClassOf(const ItoPtrInstr* const) { return true; }
    static bool ClassOf(const Instr* const i) { return i->id_ == InstrId::itoptr; }

    ItoPtrInstr(
        const Register* r, const PtrType* t, const Register* v
    ) : ConvertInstr(InstrId::itoptr, r, t, v) {}

    std::string ToString() const override;
};


class BitcastInstr : public ConvertInstr
{
public:
    static bool ClassOf(const BitcastInstr* const) { return true; }
    static bool ClassOf(const Instr* const i) { return i->id_ == InstrId::bitcast; }

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
public:
    static bool ClassOf(const IcmpInstr* const) { return true; }
    static bool ClassOf(const Instr* const i) { return i->id_ == InstrId::icmp; }

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
public:
    static bool ClassOf(const FcmpInstr* const) { return true; }
    static bool ClassOf(const Instr* const i) { return i->id_ == InstrId::fcmp; }

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
public:
    static bool ClassOf(const SelectInstr* const) { return true; }
    static bool ClassOf(const Instr* const i) { return i->id_ == InstrId::select; }

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
public:
    static bool ClassOf(const PhiInstr* const) { return true; }
    static bool ClassOf(const Instr* const i) { return i->id_ == InstrId::phi; }

    using BlockValPairList = std::vector<std::pair<const BasicBlock*, const IROperand*>>;

    PhiInstr(const Register* r, const IRType* t) :
        Instr(InstrId::phi), result_(r), type_(t) {}

    std::string ToString() const override;
    void AddBlockValPair(const BasicBlock*, const IROperand*);
    const auto& GetBlockValPair() const { return labels_; }
    auto Result() const { return result_; }

private:
    const Register* result_{};
    const IRType* type_{};
    BlockValPairList labels_{};
};


#endif // _INSTR_H_
