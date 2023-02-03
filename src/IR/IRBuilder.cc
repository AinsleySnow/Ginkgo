#include "IR/IRBuilder.h"
#include "IR/Value.h"
#include <memory>


void IRBuilder::InsertInstr(std::unique_ptr<Instr> instr)
{
    insertpoint_->AddInstr(std::move(instr));
}


const IROperand* IRBuilder::MatchArithmType(
    const IRType* target, const IROperand* val)
{
    if (!dynamic_cast<const Register*>(val))
    {
        if (target->IsFloat() && val->Type()->IsInt())
        {
            auto integer = static_cast<const IntConst*>(val);
            return FloatConst::CreateFloatConst(
                insertpoint_->Parent(), integer->Val(), target->ToFloatPoint());
        }
        else if (target->IsInt() && val->Type()->IsFloat())
        {
            auto floatpoint = static_cast<const FloatConst*>(val);
            return IntConst::CreateIntConst(
                insertpoint_->Parent(), floatpoint->Val(), target->ToInteger());
        }
        else if (target->IsFloat())
        {
            auto floatpoint = static_cast<const FloatConst*>(val);
            return FloatConst::CreateFloatConst(
                insertpoint_->Parent(), floatpoint->Val(), target->ToFloatPoint());   
        }
        else
        {
            auto integer = static_cast<const IntConst*>(val);
            return IntConst::CreateIntConst(
                insertpoint_->Parent(), integer->Val(), target->ToInteger());
        }
    }

    auto reg = static_cast<const Register*>(val);
    auto regty = reg->Type();
    auto trgname = reg->Name() + '\'';

    if (target->IsFloat() && regty->IsInt())
    {
        if (regty->ToInteger()->IsSigned())
            return InsertStofInstr(trgname, target->ToFloatPoint(), reg);
        else
            return InsertUtofInstr(trgname, target->ToFloatPoint(), reg);
    }
    else if (target->IsInt() && regty->IsFloat())
    {
        if (target->ToInteger()->IsSigned())
            return InsertFtosInstr(trgname, target->ToInteger(), reg);
        else
            return InsertFtouInstr(trgname, target->ToInteger(), reg);
    }
    else if (target->operator>(*regty))
    {
        if (target->IsFloat())
            return InsertFextInstr(trgname, target->ToFloatPoint(), reg);
        else if (regty->ToInteger()->IsSigned())
            return InsertSextInstr(trgname, target->ToInteger(), reg);
        else
            return InsertZextInstr(trgname, target->ToInteger(), reg);
    }
    else
    {
        if (target->IsFloat())
            return InsertFtruncInstr(trgname, target->ToFloatPoint(), reg);
        else
            return InsertTruncInstr(trgname, target->ToInteger(), reg);
    }
}

void IRBuilder::MatchArithmType(
    const IROperand*& reg1, const IROperand*& reg2)
{
    if (reg1->Type()->operator>(*(reg2->Type())))
        reg2 = MatchArithmType(reg1->Type(), reg2);
    else if (reg1->Type()->operator<(*(reg2->Type())))
        reg1 = MatchArithmType(reg2->Type(), reg1);
    else return;
}


void IRBuilder::InsertRetInstr()
{
    auto pret = std::make_unique<RetInstr>();
    insertpoint_->AddInstr(std::move(pret));
}

void IRBuilder::InsertRetInstr(const IROperand* val)
{
    auto pret = std::make_unique<RetInstr>(val);
    insertpoint_->AddInstr(std::move(pret));
}


void IRBuilder::InsertBrInstr(const BasicBlock* label)
{
    auto pbr = std::make_unique<BrInstr>(label);
    insertpoint_->AddInstr(std::move(pbr));
}

void IRBuilder::InsertBrInstr(const IROperand* cond,
    const BasicBlock* tblk, const BasicBlock* fblk)
{
    auto pbr = std::make_unique<BrInstr>(cond, tblk, fblk);
    insertpoint_->AddInstr(std::move(pbr));
}


void IRBuilder::InsertSwitchInstr(const IROperand* ident)
{
    auto pswitch = std::make_unique<SwitchInstr>(ident);
    insertpoint_->AddInstr(std::move(pswitch));
}


const Register* IRBuilder::InsertCallInstr(
    const std::string& result, const FuncType* proto, const std::string& func)
{
    auto pcall = std::make_unique<CallInstr>(result, proto, func);
    insertpoint_->AddInstr(std::move(pcall));
    return Register::CreateRegister(
        insertpoint_->Parent(), result, proto->ReturnType());
}

const Register* IRBuilder::InsertCallInstr(
    const std::string& result, const Register* func)
{
    auto rety = func->Type()->ToFunction()->ReturnType();
    auto pcall = std::make_unique<CallInstr>(result, func);
    insertpoint_->AddInstr(std::move(pcall));
    return Register::CreateRegister(
        insertpoint_->Parent(), result, rety);
}


const Register* IRBuilder::InsertAddInstr(
    const std::string& result, const IROperand* lhs, const IROperand* rhs)
{
    MatchArithmType(lhs, rhs);
    auto padd = std::make_unique<AddInstr>(result, lhs, rhs);
    insertpoint_->AddInstr(std::move(padd));
    return Register::CreateRegister(
        insertpoint_->Parent(), result, lhs->Type());
}

const Register* IRBuilder::InsertFaddInstr(
    const std::string& result, const IROperand* lhs, const IROperand* rhs)
{
    MatchArithmType(lhs, rhs);
    auto pfadd = std::make_unique<FaddInstr>(result, lhs, rhs);
    insertpoint_->AddInstr(std::move(pfadd));
    return Register::CreateRegister(
        insertpoint_->Parent(), result, lhs->Type());
}

const Register* IRBuilder::InsertSubInstr(
    const std::string& result, const IROperand* lhs, const IROperand* rhs)
{
    MatchArithmType(lhs, rhs);
    auto psub = std::make_unique<SubInstr>(result, lhs, rhs);
    insertpoint_->AddInstr(std::move(psub));
    return Register::CreateRegister(
        insertpoint_->Parent(), result, lhs->Type());
}

const Register* IRBuilder::InsertFsubInstr(
    const std::string& result, const IROperand* lhs, const IROperand* rhs)
{
    MatchArithmType(lhs, rhs);
    auto pfsub = std::make_unique<FsubInstr>(result, lhs, rhs);
    insertpoint_->AddInstr(std::move(pfsub));
    return Register::CreateRegister(
        insertpoint_->Parent(), result, lhs->Type());
}

const Register* IRBuilder::InsertMulInstr(
    const std::string& result, const IROperand* lhs, const IROperand* rhs)
{
    MatchArithmType(lhs, rhs);
    auto pmul = std::make_unique<MulInstr>(result, lhs, rhs);
    insertpoint_->AddInstr(std::move(pmul));
    return Register::CreateRegister(
        insertpoint_->Parent(), result, lhs->Type());
}

const Register* IRBuilder::InsertFmulInstr(
    const std::string& result, const IROperand* lhs, const IROperand* rhs)
{
    MatchArithmType(lhs, rhs);
    auto pfmul = std::make_unique<FmulInstr>(result, lhs, rhs);
    insertpoint_->AddInstr(std::move(pfmul));
    return Register::CreateRegister(
        insertpoint_->Parent(), result, lhs->Type());
}

const Register* IRBuilder::InsertDivInstr(
    const std::string& result, const IROperand* lhs, const IROperand* rhs)
{
    MatchArithmType(lhs, rhs);
    auto pdiv = std::make_unique<DivInstr>(result, lhs, rhs);
    insertpoint_->AddInstr(std::move(pdiv));
    return Register::CreateRegister(
        insertpoint_->Parent(), result, lhs->Type());
}

const Register* IRBuilder::InsertFdivInstr(
    const std::string& result, const IROperand* lhs, const IROperand* rhs)
{
    MatchArithmType(lhs, rhs);
    auto pfdiv = std::make_unique<FdivInstr>(result, lhs, rhs);
    insertpoint_->AddInstr(std::move(pfdiv));
    return Register::CreateRegister(
        insertpoint_->Parent(), result, lhs->Type());
}

const Register* IRBuilder::InsertModInstr(
    const std::string& result, const IROperand* lhs, const IROperand* rhs)
{
    MatchArithmType(lhs, rhs);
    auto pmod = std::make_unique<ModInstr>(result, lhs, rhs);
    insertpoint_->AddInstr(std::move(pmod));
    return Register::CreateRegister(
        insertpoint_->Parent(), result, lhs->Type());
}

const Register* IRBuilder::InsertShlInstr(
    const std::string& result, const IROperand* lhs, const IROperand* rhs)
{
    MatchArithmType(lhs, rhs);
    auto pshl = std::make_unique<ShlInstr>(result, lhs, rhs);
    insertpoint_->AddInstr(std::move(pshl));
    return Register::CreateRegister(
        insertpoint_->Parent(), result, lhs->Type());
}

const Register* IRBuilder::InsertLshrInstr(
    const std::string& result, const IROperand* lhs, const IROperand* rhs)
{
    MatchArithmType(lhs, rhs);
    auto plshr = std::make_unique<LshrInstr>(result, lhs, rhs);
    insertpoint_->AddInstr(std::move(plshr));
    return Register::CreateRegister(
        insertpoint_->Parent(), result, lhs->Type());
}

const Register* IRBuilder::InsertAshrInstr(
    const std::string& result, const IROperand* lhs, const IROperand* rhs)
{
    MatchArithmType(lhs, rhs);
    auto pashr = std::make_unique<AshrInstr>(result, lhs, rhs);
    insertpoint_->AddInstr(std::move(pashr));
    return Register::CreateRegister(
        insertpoint_->Parent(), result, lhs->Type());
}

const Register* IRBuilder::InsertAndInstr(
    const std::string& result, const IROperand* lhs, const IROperand* rhs)
{
    MatchArithmType(lhs, rhs);
    auto pand = std::make_unique<AndInstr>(result, lhs, rhs);
    insertpoint_->AddInstr(std::move(pand));
    return Register::CreateRegister(
        insertpoint_->Parent(), result, lhs->Type());
}

const Register* IRBuilder::InsertOrInstr(
    const std::string& result, const IROperand* lhs, const IROperand* rhs)
{
    MatchArithmType(lhs, rhs);
    auto por = std::make_unique<OrInstr>(result, lhs, rhs);
    insertpoint_->AddInstr(std::move(por));
    return Register::CreateRegister(
        insertpoint_->Parent(), result, lhs->Type());
}

const Register* IRBuilder::InsertXorInstr(
    const std::string& result, const IROperand* lhs, const IROperand* rhs)
{
    MatchArithmType(lhs, rhs);
    auto pxor = std::make_unique<XorInstr>(result, lhs, rhs);
    insertpoint_->AddInstr(std::move(pxor));
    return Register::CreateRegister(
        insertpoint_->Parent(), result, lhs->Type());
}


const Register* IRBuilder::InsertAllocaInstr(const std::string& result, const IRType* ty)
{
    auto palloca = std::make_unique<AllocaInstr>(result, ty);
    insertpoint_->AddInstr(std::move(palloca));
    return Register::CreateRegister(insertpoint_->Parent(), result, ty);
}

const Register* IRBuilder::InsertAllocaInstr(
    const std::string& result, const IRType* ty, size_t num)
{
    auto palloca = std::make_unique<AllocaInstr>(result, ty, num);
    insertpoint_->AddInstr(std::move(palloca));
    return Register::CreateRegister(insertpoint_->Parent(), result, ty);
}

const Register* IRBuilder::InsertAllocaInstr(
    const std::string& result, const IRType* ty, size_t num, size_t align)
{
    auto palloca = std::make_unique<AllocaInstr>(result, ty, num, align);
    insertpoint_->AddInstr(std::move(palloca));
    return Register::CreateRegister(insertpoint_->Parent(), result, ty);
}


const Register* IRBuilder::InsertLoadInstr(const std::string& result, const Register* ptr)
{
    auto pload = std::make_unique<LoadInstr>(result, ptr);
    insertpoint_->AddInstr(std::move(pload));
    return Register::CreateRegister(
        insertpoint_->Parent(), result, ptr->Type()->ToPointer()->Point2());
}

const Register* IRBuilder::InsertLoadInstr(
    const std::string& result, const Register* ptr, size_t align)
{
    auto pload = std::make_unique<LoadInstr>(result, ptr, align);
    insertpoint_->AddInstr(std::move(pload));
    return Register::CreateRegister(
        insertpoint_->Parent(), result, ptr->Type()->ToPointer()->Point2());
}

const Register* IRBuilder::InsertLoadInstr(
    const std::string& result, const Register* ptr, size_t align, bool vol)
{
    auto pload = std::make_unique<LoadInstr>(result, ptr, align, vol);
    insertpoint_->AddInstr(std::move(pload));
    return Register::CreateRegister(
        insertpoint_->Parent(), result, ptr->Type()->ToPointer()->Point2());
}


void IRBuilder::InsertStoreInstr(const IROperand* val, const Register* ptr, bool vol)
{
    auto pstore = std::make_unique<StoreInstr>(val, ptr, vol);
    insertpoint_->AddInstr(std::move(pstore));
}


const Register* IRBuilder::InsertExtractValInstr(
    const std::string& result, const Register* val, int index)
{
    auto pexval = std::make_unique<ExtractValInstr>(result, val, index);
    insertpoint_->AddInstr(std::move(pexval));
    // FIXME : register's type shouldn't be "val->Type()"
    return Register::CreateRegister(insertpoint_->Parent(), result, val->Type());
}

void IRBuilder::InsertSetValInstr(
    const IROperand* newval, const Register* val, int index)
{
    auto psetval = std::make_unique<SetValInstr>(newval, val, index);
    insertpoint_->AddInstr(std::move(psetval));
}

const Register* IRBuilder::InsertGetElePtrInstr(
    const std::string& result, const Register* val, int index)
{
    auto pgeteleptr = std::make_unique<GetElePtrInstr>(result, val, index);
    insertpoint_->AddInstr(std::move(pgeteleptr));
    // FIXME: register type shouldn't be val->Type()
    return Register::CreateRegister(insertpoint_->Parent(), result, val->Type());
}


const Register* IRBuilder::InsertTruncInstr(
    const std::string& result, const IntType* ty, const Register* val)
{
    auto ptrunc = std::make_unique<TruncInstr>(result, ty, val);
    insertpoint_->AddInstr(std::move(ptrunc));
    return Register::CreateRegister(insertpoint_->Parent(), result, ty);
}

const Register* IRBuilder::InsertFtruncInstr(
    const std::string& result, const FloatType* ty, const Register* val)
{
    auto pftrunc = std::make_unique<FtruncInstr>(result, ty, val);
    insertpoint_->AddInstr(std::move(pftrunc));
    return Register::CreateRegister(insertpoint_->Parent(), result, ty);
}

const Register* IRBuilder::InsertZextInstr(
    const std::string& result, const IntType* ty, const Register* val)
{
    auto pzext = std::make_unique<ZextInstr>(result, ty, val);
    insertpoint_->AddInstr(std::move(pzext));
    return Register::CreateRegister(insertpoint_->Parent(), result, ty);
}

const Register* IRBuilder::InsertSextInstr(
    const std::string& result, const IntType* ty, const Register* val)
{
    auto psext = std::make_unique<SextInstr>(result, ty, val);
    insertpoint_->AddInstr(std::move(psext));
    return Register::CreateRegister(insertpoint_->Parent(), result, ty);
}

const Register* IRBuilder::InsertFextInstr(
    const std::string& result, const FloatType* ty, const Register* val)
{
    auto pfext = std::make_unique<FextInstr>(result, ty, val);
    insertpoint_->AddInstr(std::move(pfext));
    return Register::CreateRegister(insertpoint_->Parent(), result, ty);
}

const Register* IRBuilder::InsertFtouInstr(
    const std::string& result, const IntType* ty, const Register* val)
{
    auto pftou = std::make_unique<FtouInstr>(result, ty, val);
    insertpoint_->AddInstr(std::move(pftou));
    return Register::CreateRegister(insertpoint_->Parent(), result, ty);
}

const Register* IRBuilder::InsertFtosInstr(
    const std::string& result, const IntType* ty, const Register* val)
{
    auto pftos = std::make_unique<FtosInstr>(result, ty, val);
    insertpoint_->AddInstr(std::move(pftos));
    return Register::CreateRegister(insertpoint_->Parent(), result, ty);
}

const Register* IRBuilder::InsertUtofInstr(
    const std::string& result, const FloatType* ty, const Register* val)
{
    auto putof = std::make_unique<UtofInstr>(result, ty, val);
    insertpoint_->AddInstr(std::move(putof));
    return Register::CreateRegister(insertpoint_->Parent(), result, ty);
}

const Register* IRBuilder::InsertStofInstr(
    const std::string& result, const FloatType* ty, const Register* val)
{
    auto pstof = std::make_unique<StofInstr>(result, ty, val);
    insertpoint_->AddInstr(std::move(pstof));
    return Register::CreateRegister(insertpoint_->Parent(), result, ty);
}

const Register* IRBuilder::InsertPtrtoiInstr(
    const std::string& result, const IntType* ty, const Register* val)
{
    auto pptrtoi = std::make_unique<PtrtoiInstr>(result, ty, val);
    insertpoint_->AddInstr(std::move(pptrtoi));
    return Register::CreateRegister(insertpoint_->Parent(), result, ty);
}

const Register* IRBuilder::InsertItoptrInstr(
    const std::string& result, const PtrType* ty, const Register* val)
{
    auto pitoptr = std::make_unique<ItoptrInstr>(result, ty, val);
    insertpoint_->AddInstr(std::move(pitoptr));
    return Register::CreateRegister(insertpoint_->Parent(), result, ty);
}

const Register* IRBuilder::InsertBitcastInstr(
    const std::string& result, const IRType* ty, const Register* val)
{
    auto pbitcast = std::make_unique<BitcastInstr>(result, ty, val);
    insertpoint_->AddInstr(std::move(pbitcast));
    return Register::CreateRegister(insertpoint_->Parent(), result, ty);
}


const Register* IRBuilder::InsertCmpInstr(
    const std::string& result, Condition cond, const IROperand* lhs, const IROperand* rhs)
{
    MatchArithmType(lhs, rhs);
    if (lhs->Type()->IsInt())
        return InsertIcmpInstr(result, cond, lhs, rhs);
    else
        return InsertFcmpInstr(result, cond, lhs, rhs);
}

const Register* IRBuilder::InsertIcmpInstr(
    const std::string& result, Condition cond, const IROperand* lhs, const IROperand* rhs)
{
    auto picmp = std::make_unique<IcmpInstr>(result, cond, lhs, rhs);
    insertpoint_->AddInstr(std::move(picmp));
    return Register::CreateRegister(insertpoint_->Parent(), result, lhs->Type());
}

const Register* IRBuilder::InsertFcmpInstr(
    const std::string& result, Condition cond, const IROperand* lhs, const IROperand* rhs)
{
    auto pfcmp = std::make_unique<FcmpInstr>(result, cond, lhs, rhs);
    insertpoint_->AddInstr(std::move(pfcmp));
    return Register::CreateRegister(insertpoint_->Parent(), result, lhs->Type());
}


const Register* IRBuilder::InsertSelectInstr(
    const std::string& result, const IROperand* selty,
    bool cond, const IROperand* lhs, const IROperand* rhs)
{
    auto pselect = std::make_unique<SelectInstr>(result, selty, cond, lhs, rhs);
    insertpoint_->AddInstr(std::move(pselect));
    return Register::CreateRegister(insertpoint_->Parent(), result, lhs->Type());
}

const Register* IRBuilder::InsertPhiInstr(const std::string& result, const IRType* ty)
{
    auto pphi = std::make_unique<PhiInstr>(result, ty);
    insertpoint_->AddInstr(std::move(pphi));
    return Register::CreateRegister(insertpoint_->Parent(), result, ty);
}
