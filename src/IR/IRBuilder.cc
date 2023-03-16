#include "IR/IRBuilder.h"
#include "IR/IROperand.h"
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
    if (!dynamic_cast<const Register*>(val))
    {
        if (target->IsFloat() && val->Type()->IsInt())
        {
            auto integer = static_cast<const IntConst*>(val);
            val = FloatConst::CreateFloatConst(
                Container(), integer->Val(), target->ToFloatPoint());
        }
        else if (target->IsInt() && val->Type()->IsFloat())
        {
            auto floatpoint = static_cast<const FloatConst*>(val);
            val = IntConst::CreateIntConst(
                Container(), floatpoint->Val(), target->ToInteger());
        }
        else if (target->IsFloat())
        {
            auto floatpoint = static_cast<const FloatConst*>(val);
            val = FloatConst::CreateFloatConst(
                Container(), floatpoint->Val(), target->ToFloatPoint());
        }
        else
        {
            auto integer = static_cast<const IntConst*>(val);
            val = IntConst::CreateIntConst(
                Container(), integer->Val(), target->ToInteger());
        }
        return;
    }

    auto reg = static_cast<const Register*>(val);
    auto regty = reg->Type();
    auto trgname = reg->Name() + '\'';

    if (target->IsFloat() && regty->IsInt())
    {
        if (regty->ToInteger()->IsSigned())
            val = InsertStofInstr(trgname, target->ToFloatPoint(), reg);
        else val = InsertUtofInstr(trgname, target->ToFloatPoint(), reg);
    }
    else if (target->IsInt() && regty->IsFloat())
    {
        if (target->ToInteger()->IsSigned())
            val = InsertFtosInstr(trgname, target->ToInteger(), reg);
        else val = InsertFtouInstr(trgname, target->ToInteger(), reg);
    }
    else if (target->operator>(*regty))
    {
        if (target->IsFloat())
            val = InsertFextInstr(trgname, target->ToFloatPoint(), reg);
        else if (regty->ToInteger()->IsSigned())
            val = InsertSextInstr(trgname, target->ToInteger(), reg);
        else val = InsertZextInstr(trgname, target->ToInteger(), reg);
    }
    else if (target->operator<(*regty))
    {
        if (target->IsFloat())
            val = InsertFtruncInstr(trgname, target->ToFloatPoint(), reg);
        else val = InsertTruncInstr(trgname, target->ToInteger(), reg);
    }
}

void InstrBuilder::MatchArithmType(
    const IROperand*& reg1, const IROperand*& reg2)
{
    if (reg1->Type()->operator>(*(reg2->Type())))
        MatchArithmType(reg1->Type(), reg2);
    else if (reg1->Type()->operator<(*(reg2->Type())))
        MatchArithmType(reg2->Type(), reg1);
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
    auto pcall = std::make_unique<CallInstr>(result, proto, func);
    Insert(std::move(pcall));
    return result.empty() ?
        nullptr : Register::CreateRegister((Container()), result, proto->ReturnType());
}

const Register* InstrBuilder::InsertCallInstr(
    const std::string& result, const Register* func)
{
    // func->Type() must be a pointer in this situation.
    auto rety = func->Type()->ToPointer()->
        Point2()->ToFunction()->ReturnType();

    auto pcall = std::make_unique<CallInstr>(result, func);
    Insert(std::move(pcall));
    return result.empty() ?
        nullptr : Register::CreateRegister(Container(), result, rety);
}


const Register* InstrBuilder::InsertAddInstr(
    const std::string& result, const IROperand* lhs, const IROperand* rhs)
{
    MatchArithmType(lhs, rhs);
    auto padd = std::make_unique<AddInstr>(result, lhs, rhs);
    Insert(std::move(padd));
    return Register::CreateRegister(Container(), result, lhs->Type());
}

const Register* InstrBuilder::InsertFaddInstr(
    const std::string& result, const IROperand* lhs, const IROperand* rhs)
{
    MatchArithmType(lhs, rhs);
    auto pfadd = std::make_unique<FaddInstr>(result, lhs, rhs);
    Insert(std::move(pfadd));
    return Register::CreateRegister(Container(), result, lhs->Type());
}

const Register* InstrBuilder::InsertSubInstr(
    const std::string& result, const IROperand* lhs, const IROperand* rhs)
{
    MatchArithmType(lhs, rhs);
    auto psub = std::make_unique<SubInstr>(result, lhs, rhs);
    Insert(std::move(psub));
    return Register::CreateRegister(Container(), result, lhs->Type());
}

const Register* InstrBuilder::InsertFsubInstr(
    const std::string& result, const IROperand* lhs, const IROperand* rhs)
{
    MatchArithmType(lhs, rhs);
    auto pfsub = std::make_unique<FsubInstr>(result, lhs, rhs);
    Insert(std::move(pfsub));
    return Register::CreateRegister(Container(), result, lhs->Type());
}

const Register* InstrBuilder::InsertMulInstr(
    const std::string& result, const IROperand* lhs, const IROperand* rhs)
{
    MatchArithmType(lhs, rhs);
    auto pmul = std::make_unique<MulInstr>(result, lhs, rhs);
    Insert(std::move(pmul));
    return Register::CreateRegister(Container(), result, lhs->Type());
}

const Register* InstrBuilder::InsertFmulInstr(
    const std::string& result, const IROperand* lhs, const IROperand* rhs)
{
    MatchArithmType(lhs, rhs);
    auto pfmul = std::make_unique<FmulInstr>(result, lhs, rhs);
    Insert(std::move(pfmul));
    return Register::CreateRegister(Container(), result, lhs->Type());
}

const Register* InstrBuilder::InsertDivInstr(
    const std::string& result, const IROperand* lhs, const IROperand* rhs)
{
    MatchArithmType(lhs, rhs);
    auto pdiv = std::make_unique<DivInstr>(result, lhs, rhs);
    Insert(std::move(pdiv));
    return Register::CreateRegister(Container(), result, lhs->Type());
}

const Register* InstrBuilder::InsertFdivInstr(
    const std::string& result, const IROperand* lhs, const IROperand* rhs)
{
    MatchArithmType(lhs, rhs);
    auto pfdiv = std::make_unique<FdivInstr>(result, lhs, rhs);
    Insert(std::move(pfdiv));
    return Register::CreateRegister(Container(), result, lhs->Type());
}

const Register* InstrBuilder::InsertModInstr(
    const std::string& result, const IROperand* lhs, const IROperand* rhs)
{
    MatchArithmType(lhs, rhs);
    auto pmod = std::make_unique<ModInstr>(result, lhs, rhs);
    Insert(std::move(pmod));
    return Register::CreateRegister(Container(), result, lhs->Type());
}

const Register* InstrBuilder::InsertShlInstr(
    const std::string& result, const IROperand* lhs, const IROperand* rhs)
{
    MatchArithmType(lhs->Type(), rhs);
    auto pshl = std::make_unique<ShlInstr>(result, lhs, rhs);
    Insert(std::move(pshl));
    return Register::CreateRegister(Container(), result, lhs->Type());
}

const Register* InstrBuilder::InsertLshrInstr(
    const std::string& result, const IROperand* lhs, const IROperand* rhs)
{
    MatchArithmType(lhs->Type(), rhs);
    auto plshr = std::make_unique<LshrInstr>(result, lhs, rhs);
    Insert(std::move(plshr));
    return Register::CreateRegister(Container(), result, lhs->Type());
}

const Register* InstrBuilder::InsertAshrInstr(
    const std::string& result, const IROperand* lhs, const IROperand* rhs)
{
    MatchArithmType(lhs->Type(), rhs);
    auto pashr = std::make_unique<AshrInstr>(result, lhs, rhs);
    Insert(std::move(pashr));
    return Register::CreateRegister(Container(), result, lhs->Type());
}

const Register* InstrBuilder::InsertAndInstr(
    const std::string& result, const IROperand* lhs, const IROperand* rhs)
{
    MatchArithmType(lhs, rhs);
    auto pand = std::make_unique<AndInstr>(result, lhs, rhs);
    Insert(std::move(pand));
    return Register::CreateRegister(Container(), result, lhs->Type());
}

const Register* InstrBuilder::InsertOrInstr(
    const std::string& result, const IROperand* lhs, const IROperand* rhs)
{
    MatchArithmType(lhs, rhs);
    auto por = std::make_unique<OrInstr>(result, lhs, rhs);
    Insert(std::move(por));
    return Register::CreateRegister(Container(), result, lhs->Type());
}

const Register* InstrBuilder::InsertXorInstr(
    const std::string& result, const IROperand* lhs, const IROperand* rhs)
{
    MatchArithmType(lhs, rhs);
    auto pxor = std::make_unique<XorInstr>(result, lhs, rhs);
    Insert(std::move(pxor));
    return Register::CreateRegister(Container(), result, lhs->Type());
}


const Register* InstrBuilder::InsertAllocaInstr(const std::string& result, const IRType* ty)
{
    auto palloca = std::make_unique<AllocaInstr>(result, ty);
    Insert(std::move(palloca));
    auto ptrty = PtrType::GetPtrType(Container(), ty);
    return Register::CreateRegister(Container(), result, ptrty);
}

const Register* InstrBuilder::InsertAllocaInstr(
    const std::string& result, const IRType* ty, size_t num)
{
    auto palloca = std::make_unique<AllocaInstr>(result, ty, num);
    Insert(std::move(palloca));
    auto ptrty = PtrType::GetPtrType(Container(), ty);
    return Register::CreateRegister(Container(), result, ptrty);
}

const Register* InstrBuilder::InsertAllocaInstr(
    const std::string& result, const IRType* ty, size_t num, size_t align)
{
    auto palloca = std::make_unique<AllocaInstr>(result, ty, num, align);
    Insert(std::move(palloca));
    auto ptrty = PtrType::GetPtrType(Container(), ty);
    return Register::CreateRegister(Container(), result, ptrty);
}


const Register* InstrBuilder::InsertLoadInstr(const std::string& result, const Register* ptr)
{
    auto pload = std::make_unique<LoadInstr>(result, ptr);
    Insert(std::move(pload));
    return Register::CreateRegister(Container(), result, ptr->Type()->ToPointer()->Point2());
}

const Register* InstrBuilder::InsertLoadInstr(
    const std::string& result, const Register* ptr, size_t align)
{
    auto pload = std::make_unique<LoadInstr>(result, ptr, align);
    Insert(std::move(pload));
    return Register::CreateRegister(Container(), result, ptr->Type()->ToPointer()->Point2());
}

const Register* InstrBuilder::InsertLoadInstr(
    const std::string& result, const Register* ptr, size_t align, bool vol)
{
    auto pload = std::make_unique<LoadInstr>(result, ptr, align, vol);
    Insert(std::move(pload));
    return Register::CreateRegister(Container(), result, ptr->Type()->ToPointer()->Point2());
}


void InstrBuilder::InsertStoreInstr(const IROperand* val, const Register* ptr, bool vol)
{
    auto pstore = std::make_unique<StoreInstr>(val, ptr, vol);
    Insert(std::move(pstore));
}


const Register* InstrBuilder::InsertExtractValInstr(
    const std::string& result, const Register* val, const IROperand* index)
{
    auto pexval = std::make_unique<ExtractValInstr>(result, val, index);
    Insert(std::move(pexval));
    // FIXME : register's type shouldn't be "val->Type()"
    return Register::CreateRegister(Container(), result, val->Type());
}

void InstrBuilder::InsertSetValInstr(
    const IROperand* newval, const Register* val, const IROperand* index)
{
    auto psetval = std::make_unique<SetValInstr>(newval, val, index);
    Insert(std::move(psetval));
}

const Register* InstrBuilder::InsertGetElePtrInstr(
    const std::string& result, const Register* val, const IROperand* index)
{
    auto pgeteleptr = std::make_unique<GetElePtrInstr>(result, val, index);
    Insert(std::move(pgeteleptr));
    // FIXME: register type shouldn't be val->Type()
    return Register::CreateRegister(Container(), result, val->Type());
}


const Register* InstrBuilder::InsertTruncInstr(
    const std::string& result, const IntType* ty, const Register* val)
{
    auto ptrunc = std::make_unique<TruncInstr>(result, ty, val);
    Insert(std::move(ptrunc));
    return Register::CreateRegister(Container(), result, ty);
}

const Register* InstrBuilder::InsertFtruncInstr(
    const std::string& result, const FloatType* ty, const Register* val)
{
    auto pftrunc = std::make_unique<FtruncInstr>(result, ty, val);
    Insert(std::move(pftrunc));
    return Register::CreateRegister(Container(), result, ty);
}

const Register* InstrBuilder::InsertZextInstr(
    const std::string& result, const IntType* ty, const Register* val)
{
    auto pzext = std::make_unique<ZextInstr>(result, ty, val);
    Insert(std::move(pzext));
    return Register::CreateRegister(Container(), result, ty);
}

const Register* InstrBuilder::InsertSextInstr(
    const std::string& result, const IntType* ty, const Register* val)
{
    auto psext = std::make_unique<SextInstr>(result, ty, val);
    Insert(std::move(psext));
    return Register::CreateRegister(Container(), result, ty);
}

const Register* InstrBuilder::InsertFextInstr(
    const std::string& result, const FloatType* ty, const Register* val)
{
    auto pfext = std::make_unique<FextInstr>(result, ty, val);
    Insert(std::move(pfext));
    return Register::CreateRegister(Container(), result, ty);
}

const Register* InstrBuilder::InsertFtouInstr(
    const std::string& result, const IntType* ty, const Register* val)
{
    auto pftou = std::make_unique<FtouInstr>(result, ty, val);
    Insert(std::move(pftou));
    return Register::CreateRegister(Container(), result, ty);
}

const Register* InstrBuilder::InsertFtosInstr(
    const std::string& result, const IntType* ty, const Register* val)
{
    auto pftos = std::make_unique<FtosInstr>(result, ty, val);
    Insert(std::move(pftos));
    return Register::CreateRegister(Container(), result, ty);
}

const Register* InstrBuilder::InsertUtofInstr(
    const std::string& result, const FloatType* ty, const Register* val)
{
    auto putof = std::make_unique<UtofInstr>(result, ty, val);
    Insert(std::move(putof));
    return Register::CreateRegister(Container(), result, ty);
}

const Register* InstrBuilder::InsertStofInstr(
    const std::string& result, const FloatType* ty, const Register* val)
{
    auto pstof = std::make_unique<StofInstr>(result, ty, val);
    Insert(std::move(pstof));
    return Register::CreateRegister(Container(), result, ty);
}

const Register* InstrBuilder::InsertPtrtoiInstr(
    const std::string& result, const IntType* ty, const Register* val)
{
    auto pptrtoi = std::make_unique<PtrtoiInstr>(result, ty, val);
    Insert(std::move(pptrtoi));
    return Register::CreateRegister(Container(), result, ty);
}

const Register* InstrBuilder::InsertItoptrInstr(
    const std::string& result, const PtrType* ty, const Register* val)
{
    auto pitoptr = std::make_unique<ItoptrInstr>(result, ty, val);
    Insert(std::move(pitoptr));
    return Register::CreateRegister(Container(), result, ty);
}

const Register* InstrBuilder::InsertBitcastInstr(
    const std::string& result, const IRType* ty, const Register* val)
{
    auto pbitcast = std::make_unique<BitcastInstr>(result, ty, val);
    Insert(std::move(pbitcast));
    return Register::CreateRegister(Container(), result, ty);
}


const Register* InstrBuilder::InsertCmpInstr(
    const std::string& result, Condition cond, const IROperand* lhs, const IROperand* rhs)
{
    MatchArithmType(lhs, rhs);
    if (lhs->Type()->IsInt())
        return InsertIcmpInstr(result, cond, lhs, rhs);
    else
        return InsertFcmpInstr(result, cond, lhs, rhs);
}

const Register* InstrBuilder::InsertIcmpInstr(
    const std::string& result, Condition cond, const IROperand* lhs, const IROperand* rhs)
{
    auto picmp = std::make_unique<IcmpInstr>(result, cond, lhs, rhs);
    Insert(std::move(picmp));
    return Register::CreateRegister(Container(), result, IntType::GetInt8(true));
}

const Register* InstrBuilder::InsertFcmpInstr(
    const std::string& result, Condition cond, const IROperand* lhs, const IROperand* rhs)
{
    auto pfcmp = std::make_unique<FcmpInstr>(result, cond, lhs, rhs);
    Insert(std::move(pfcmp));
    return Register::CreateRegister(Container(), result, IntType::GetInt8(true));
}


const Register* InstrBuilder::InsertSelectInstr(
    const std::string& result, const IROperand* selty,
    bool cond, const IROperand* lhs, const IROperand* rhs)
{
    auto pselect = std::make_unique<SelectInstr>(result, selty, cond, lhs, rhs);
    Insert(std::move(pselect));
    return Register::CreateRegister(Container(), result, lhs->Type());
}

const Register* InstrBuilder::InsertPhiInstr(const std::string& result, const IRType* ty)
{
    auto pphi = std::make_unique<PhiInstr>(result, ty);
    Insert(std::move(pphi));
    return Register::CreateRegister(Container(), result, ty);
}
