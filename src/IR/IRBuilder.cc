#include "IR/IRBuilder.h"
#include "IR/IROperand.h"
#include "IR/IRType.h"
#include "IR/Value.h"
#include <memory>


void BlockBuilder::InsertBasicBlock(const std::string& name)
{
    auto bb = std::make_unique<BasicBlock>(name);
    Insert(std::move(bb));
}

BasicBlock* BlockBuilder::GetBasicBlock(const std::string& name)
{
    auto bb = std::make_unique<BasicBlock>(name);
    auto pbb = bb.get();
    Insert(std::move(bb));
    return pbb;
}

void BlockBuilder::PopBack()
{
    if (InsertPoint() != Container()->begin())
        (*(InsertPoint() - 2))->MergePools(*(InsertPoint() - 1));
    Remove();
}

void BlockBuilder::RemoveBlk(BasicBlock* bb)
{
    auto index = Container()->IndexOf(bb);
    if (Container()->Size() != 1)
    {
        if (index == Container()->Size() - 1) // end of function
            Container()->At(index - 1)->MergePools(bb);
        else Container()->At(index + 1)->MergePools(bb);
    }

    Remove(bb);
}


void InstrBuilder::MatchArithmType(
    const IRType* target, const IROperand*& val)
{
    if (!val->Is<Register>())
    {
        if (target->Is<FloatType>() && val->Type()->Is<IntType>())
        {
            auto integer = static_cast<const IntConst*>(val);
            val = FloatConst::CreateFloatConst(
                Container(), integer->Val(), target->As<FloatType>());
        }
        else if (target->Is<IntType>() && val->Type()->Is<FloatType>())
        {
            auto floatpoint = static_cast<const FloatConst*>(val);
            val = IntConst::CreateIntConst(
                Container(), floatpoint->Val(), target->As<IntType>());
        }
        else if (target->Is<FloatType>())
        {
            auto floatpoint = static_cast<const FloatConst*>(val);
            val = FloatConst::CreateFloatConst(
                Container(), floatpoint->Val(), target->As<FloatType>());
        }
        else
        {
            auto integer = static_cast<const IntConst*>(val);
            val = IntConst::CreateIntConst(
                Container(), integer->Val(), target->As<IntType>());
        }
        return;
    }

    auto reg = static_cast<const Register*>(val);
    auto regty = reg->Type();
    auto trgname = reg->Name() + "cvt";

    if (target->Is<FloatType>() && regty->Is<IntType>())
    {
        if (regty->As<IntType>()->IsSigned())
            val = InsertStoFInstr(trgname, target->As<FloatType>(), reg);
        else val = InsertUtoFInstr(trgname, target->As<FloatType>(), reg);
    }
    else if (target->Is<IntType>() && regty->Is<FloatType>())
    {
        if (target->As<IntType>()->IsSigned())
            val = InsertFtoSInstr(trgname, target->As<IntType>(), reg);
        else val = InsertFtoUInstr(trgname, target->As<IntType>(), reg);
    }
    else if (target->operator>(*regty))
    {
        if (target->Is<FloatType>())
            val = InsertFextInstr(trgname, target->As<FloatType>(), reg);
        else if (regty->As<IntType>()->IsSigned())
            val = InsertSextInstr(trgname, target->As<IntType>(), reg);
        else val = InsertZextInstr(trgname, target->As<IntType>(), reg);
    }
    else if (target->operator<(*regty))
    {
        if (target->Is<FloatType>())
            val = InsertFtruncInstr(trgname, target->As<FloatType>(), reg);
        else val = InsertTruncInstr(trgname, target->As<IntType>(), reg);
    }
    // Here, target and regty belong to the same category.
    else if (target->Is<IntType>() &&
        (target->As<IntType>()->IsSigned() ^ regty->As<IntType>()->IsSigned()))
        val = InsertBitcastInstr(trgname, target, val->As<Register>());
}

void InstrBuilder::MatchArithmType(
    const IROperand*& reg1, const IROperand*& reg2)
{
    if (reg1->Type()->operator>(*(reg2->Type())))
        MatchArithmType(reg1->Type(), reg2);
    else if (reg1->Type()->operator<(*(reg2->Type())))
        MatchArithmType(reg2->Type(), reg1);
    // Here, reg1 and reg2 has the same size. One being
    // float point means the other one must have the same
    // float point type.
    if (reg1->Type()->Is<FloatType>())
        return;
    auto ty1 = reg1->Type()->As<IntType>(),
        ty2 = reg2->Type()->As<IntType>();
    if (!(ty1->IsSigned() ^ ty2->IsSigned()))
        return;
    if (ty1->IsSigned())
        MatchArithmType(ty2, reg1);
    else
        MatchArithmType(ty1, reg2);
}


void InstrBuilder::InsertRetInstr()
{
    auto pret = std::make_unique<RetInstr>();
    Insert(std::move(pret));
}

void InstrBuilder::InsertRetInstr(const IROperand* val)
{
    auto pret = std::make_unique<RetInstr>(val);
    Insert(std::move(pret));
}


void InstrBuilder::InsertBrInstr(const BasicBlock* label)
{
    auto pbr = std::make_unique<BrInstr>(label);
    Insert(std::move(pbr));
}

void InstrBuilder::InsertBrInstr(const IROperand* cond,
    const BasicBlock* tblk, const BasicBlock* fblk)
{
    auto pbr = std::make_unique<BrInstr>(cond, tblk, fblk);
    Insert(std::move(pbr));
}


void InstrBuilder::InsertSwitchInstr(const IROperand* ident)
{
    auto pswitch = std::make_unique<SwitchInstr>(ident);
    Insert(std::move(pswitch));
}


const Register* InstrBuilder::InsertCallInstr(
    const std::string& result, const FuncType* proto, const std::string& func)
{
    auto preg = result.empty() ?
        nullptr : Register::CreateRegister((Container()), result, proto->ReturnType());
    Insert(std::make_unique<CallInstr>(preg, proto, func));
    return preg;
}

const Register* InstrBuilder::InsertCallInstr(
    const std::string& result, const Register* func)
{
    // func->Type() must be a pointer in this situation.
    auto rety = func->Type()->As<PtrType>()->
        Point2()->As<FuncType>()->ReturnType();

    auto preg = result.empty() ?
        nullptr : Register::CreateRegister(Container(), result, rety);
    Insert(std::make_unique<CallInstr>(preg, func));
    return preg;
}


const IROperand* InstrBuilder::InsertArithmCastInstr(const IRType* ty, const IROperand* op)
{
    MatchArithmType(ty, op);
    return op;
}


#define INSERT_BINARY_INSTR(name)                                       \
MatchArithmType(lhs, rhs);                                              \
auto ans = Register::CreateRegister(Container(), result, lhs->Type());  \
Insert(std::make_unique<name##Instr>(ans, lhs, rhs));                   \
return ans;

const Register* InstrBuilder::InsertAddInstr(const std::string& result,
    const IROperand* lhs, const IROperand* rhs) { INSERT_BINARY_INSTR(Add); }
const Register* InstrBuilder::InsertFaddInstr(const std::string& result,
    const IROperand* lhs, const IROperand* rhs) { INSERT_BINARY_INSTR(Fadd); }
const Register* InstrBuilder::InsertSubInstr(const std::string& result,
    const IROperand* lhs, const IROperand* rhs) { INSERT_BINARY_INSTR(Sub); }
const Register* InstrBuilder::InsertFsubInstr(const std::string& result,
    const IROperand* lhs, const IROperand* rhs) { INSERT_BINARY_INSTR(Fsub); }
const Register* InstrBuilder::InsertMulInstr(const std::string& result,
    const IROperand* lhs, const IROperand* rhs) { INSERT_BINARY_INSTR(Mul); }
const Register* InstrBuilder::InsertFmulInstr(const std::string& result,
    const IROperand* lhs, const IROperand* rhs) { INSERT_BINARY_INSTR(Fmul); }
const Register* InstrBuilder::InsertDivInstr(const std::string& result,
    const IROperand* lhs, const IROperand* rhs) { INSERT_BINARY_INSTR(Div); }
const Register* InstrBuilder::InsertFdivInstr(const std::string& result,
    const IROperand* lhs, const IROperand* rhs) { INSERT_BINARY_INSTR(Fdiv); }
const Register* InstrBuilder::InsertModInstr(const std::string& result,
    const IROperand* lhs, const IROperand* rhs) { INSERT_BINARY_INSTR(Mod); }
const Register* InstrBuilder::InsertShlInstr(const std::string& result,
    const IROperand* lhs, const IROperand* rhs) { INSERT_BINARY_INSTR(Shl); }
const Register* InstrBuilder::InsertLshrInstr(const std::string& result,
    const IROperand* lhs, const IROperand* rhs) { INSERT_BINARY_INSTR(Lshr); }
const Register* InstrBuilder::InsertAshrInstr(const std::string& result,
    const IROperand* lhs, const IROperand* rhs) { INSERT_BINARY_INSTR(Ashr); }
const Register* InstrBuilder::InsertAndInstr(const std::string& result,
    const IROperand* lhs, const IROperand* rhs) { INSERT_BINARY_INSTR(And); }
const Register* InstrBuilder::InsertOrInstr(const std::string& result,
    const IROperand* lhs, const IROperand* rhs) { INSERT_BINARY_INSTR(Or); }
const Register* InstrBuilder::InsertXorInstr(const std::string& result,
    const IROperand* lhs, const IROperand* rhs) { INSERT_BINARY_INSTR(Xor); }

#undef INSERT_BINARY_INSTR


const Register* InstrBuilder::InsertAllocaInstr(const std::string& result, const IRType* ty)
{
    auto ptrty = PtrType::GetPtrType(Container(), ty);
    auto ans = Register::CreateRegister(Container(), result, ptrty);
    Insert(std::make_unique<AllocaInstr>(ans, ty));
    return ans;
}

const Register* InstrBuilder::InsertAllocaInstr(
    const std::string& result, const IRType* ty, size_t num)
{
    auto ptrty = PtrType::GetPtrType(Container(), ty);
    auto ans = Register::CreateRegister(Container(), result, ptrty);
    Insert(std::make_unique<AllocaInstr>(ans, ty, num));
    return ans;
}

const Register* InstrBuilder::InsertAllocaInstr(
    const std::string& result, const IRType* ty, size_t num, size_t align)
{
    auto ptrty = PtrType::GetPtrType(Container(), ty);
    auto ans = Register::CreateRegister(Container(), result, ptrty);
    Insert(std::make_unique<AllocaInstr>(ans, ty, num, align));
    return ans;
}


const Register* InstrBuilder::InsertLoadInstr(const std::string& result, const Register* ptr)
{
    auto ans = Register::CreateRegister(Container(), result, ptr->Type()->As<PtrType>()->Point2());
    Insert(std::make_unique<LoadInstr>(ans, ptr));
    return ans;
}

const Register* InstrBuilder::InsertLoadInstr(
    const std::string& result, const Register* ptr, size_t align)
{
    auto ans = Register::CreateRegister(Container(), result, ptr->Type()->As<PtrType>()->Point2());
    Insert(std::make_unique<LoadInstr>(ans, ptr, align));
    return ans;
}

const Register* InstrBuilder::InsertLoadInstr(
    const std::string& result, const Register* ptr, size_t align, bool vol)
{
    auto ans = Register::CreateRegister(Container(), result, ptr->Type()->As<PtrType>()->Point2());
    Insert(std::make_unique<LoadInstr>(ans, ptr, align, vol));
    return ans;
}


void InstrBuilder::InsertStoreInstr(const IROperand* val, const Register* ptr, bool vol)
{
    auto pstore = std::make_unique<StoreInstr>(val, ptr, vol);
    Insert(std::move(pstore));
}


const Register* InstrBuilder::InsertGetValInstr(
    const std::string& result, const Register* val, std::variant<const IROperand*, int> index)
{
    auto reg = Register::CreateRegister(Container(), result,
        val->Type()->As<HeterType>()->At(std::get<int>(index)));
    Insert(std::move(std::make_unique<GetValInstr>(reg, val, index)));
    return reg;
}

void InstrBuilder::InsertSetValInstr(
    const IROperand* newval, const Register* val, std::variant<const IROperand*, int> index)
{
    auto psetval = std::make_unique<SetValInstr>(newval, val, index);
    Insert(std::move(psetval));
}

const Register* InstrBuilder::InsertGetElePtrInstr(
    const std::string& result, const Register* val, std::variant<const IROperand*, int> index)
{
    auto point2 = val->Type()->As<PtrType>()->Point2();
    const IRType* rety = nullptr;

    if (point2->Is<ArrayType>())
        rety = PtrType::GetPtrType(Container(), point2->As<ArrayType>()->ArrayOf());
    else if (point2->Is<HeterType>())
    {
        rety = PtrType::GetPtrType(Container(),
            point2->As<HeterType>()->At(std::get<int>(index)));
    }
    else
        rety = PtrType::GetPtrType(Container(), point2);

    const auto* reg = Register::CreateRegister(Container(), result, rety);
    if (index.index() == 0 && std::get<0>(index)->Type()->Size() != 8)
    {
        auto i = std::get<0>(index);
        if (i->Type()->As<IntType>()->IsSigned())
            MatchArithmType(IntType::GetInt64(true), i);
        else
            MatchArithmType(IntType::GetInt64(false), i);
        index = i;
    }

    Insert(std::make_unique<GetElePtrInstr>(reg, val, index));
    return reg;
}


const Register* InstrBuilder::InsertTruncInstr(
    const std::string& result, const IntType* ty, const Register* val)
{
    auto ans = Register::CreateRegister(Container(), result, ty);
    Insert(std::make_unique<TruncInstr>(ans, ty, val));
    return ans;
}

const Register* InstrBuilder::InsertFtruncInstr(
    const std::string& result, const FloatType* ty, const Register* val)
{
    auto ans = Register::CreateRegister(Container(), result, ty);
    Insert(std::make_unique<FtruncInstr>(ans, ty, val));
    return ans;
}

const Register* InstrBuilder::InsertZextInstr(
    const std::string& result, const IntType* ty, const Register* val)
{
    auto ans = Register::CreateRegister(Container(), result, ty);
    Insert(std::make_unique<ZextInstr>(ans, ty, val));
    return ans;
}

const Register* InstrBuilder::InsertSextInstr(
    const std::string& result, const IntType* ty, const Register* val)
{
    auto ans = Register::CreateRegister(Container(), result, ty);
    Insert(std::make_unique<SextInstr>(ans, ty, val));
    return ans;
}

const Register* InstrBuilder::InsertFextInstr(
    const std::string& result, const FloatType* ty, const Register* val)
{
    auto ans = Register::CreateRegister(Container(), result, ty);
    Insert(std::make_unique<FextInstr>(ans, ty, val));
    return ans;
}

const Register* InstrBuilder::InsertFtoUInstr(
    const std::string& result, const IntType* ty, const Register* val)
{
    auto ans = Register::CreateRegister(Container(), result, ty);
    Insert(std::make_unique<FtoUInstr>(ans, ty, val));
    return ans;
}

const Register* InstrBuilder::InsertFtoSInstr(
    const std::string& result, const IntType* ty, const Register* val)
{
    auto ans = Register::CreateRegister(Container(), result, ty);
    Insert(std::make_unique<FtoSInstr>(ans, ty, val));
    return ans;
}

const Register* InstrBuilder::InsertUtoFInstr(
    const std::string& result, const FloatType* ty, const Register* val)
{
    auto ans = Register::CreateRegister(Container(), result, ty);
    Insert(std::make_unique<UtoFInstr>(ans, ty, val));
    return ans;
}

const Register* InstrBuilder::InsertStoFInstr(
    const std::string& result, const FloatType* ty, const Register* val)
{
    auto ans = Register::CreateRegister(Container(), result, ty);
    Insert(std::make_unique<StoFInstr>(ans, ty, val));
    return ans;
}

const Register* InstrBuilder::InsertPtrtoIInstr(
    const std::string& result, const IntType* ty, const Register* val)
{
    auto ans = Register::CreateRegister(Container(), result, ty);
    Insert(std::make_unique<PtrtoIInstr>(ans, ty, val));
    return ans;
}

const Register* InstrBuilder::InsertItoPtrInstr(
    const std::string& result, const PtrType* ty, const Register* val)
{
    auto ans = Register::CreateRegister(Container(), result, ty);
    Insert(std::make_unique<ItoPtrInstr>(ans, ty, val));
    return ans;
}

const Register* InstrBuilder::InsertBitcastInstr(
    const std::string& result, const IRType* ty, const Register* val)
{
    auto ans = Register::CreateRegister(Container(), result, ty);
    Insert(std::make_unique<BitcastInstr>(ans, ty, val));
    return ans;
}


const Register* InstrBuilder::InsertCmpInstr(
    const std::string& result, Condition cond, const IROperand* lhs, const IROperand* rhs)
{
    MatchArithmType(lhs, rhs);
    if (lhs->Type()->Is<IntType>())
        return InsertIcmpInstr(result, cond, lhs, rhs);
    else
        return InsertFcmpInstr(result, cond, lhs, rhs);
}

const Register* InstrBuilder::InsertIcmpInstr(
    const std::string& result, Condition cond, const IROperand* lhs, const IROperand* rhs)
{
    auto ans = Register::CreateRegister(Container(), result, IntType::GetInt8(true));
    Insert(std::make_unique<IcmpInstr>(ans, cond, lhs, rhs));
    return ans;
}

const Register* InstrBuilder::InsertFcmpInstr(
    const std::string& result, Condition cond, const IROperand* lhs, const IROperand* rhs)
{
    auto ans = Register::CreateRegister(Container(), result, IntType::GetInt8(true));
    Insert(std::make_unique<FcmpInstr>(ans, cond, lhs, rhs));
    return ans;
}


const Register* InstrBuilder::InsertSelectInstr(
    const std::string& result, const IROperand* selty,
    bool cond, const IROperand* lhs, const IROperand* rhs)
{
    auto ans = Register::CreateRegister(Container(), result, lhs->Type());
    Insert(std::make_unique<SelectInstr>(ans, selty, cond, lhs, rhs));
    return ans;
}

const Register* InstrBuilder::InsertPhiInstr(const std::string& result, const IRType* ty)
{
    auto ans = Register::CreateRegister(Container(), result, ty);
    Insert(std::make_unique<PhiInstr>(ans, ty));
    return ans;
}
