#include "IRBuilder.h"
#include "Value.h"
#include <memory>


void IRBuilder::InsertInstr(std::unique_ptr<Instr> instr)
{
    insertpoint_->AddInstr(std::move(instr));
}


void IRBuilder::InsertRetInstr(const IRType* valtype, const std::string& val)
{
    auto pret = std::make_unique<RetInstr>(valtype, val);
    insertpoint_->AddInstr(std::move(pret));
}

void IRBuilder::InsertBrInstr(const BasicBlock* label)
{
    auto pbr = std::make_unique<BrInstr>(label);
    insertpoint_->AddInstr(std::move(pbr));
}

void IRBuilder::InsertBrInstr(const std::string& cond,
    const BasicBlock* tblk, const BasicBlock* fblk)
{
    auto pbr = std::make_unique<BrInstr>(cond, tblk, fblk);
    insertpoint_->AddInstr(std::move(pbr));
}

void IRBuilder::InsertSwitchInstr(const std::string& ident, const IntType* ty,
    const SwitchInstr::ValueBlkPair& list)
{
    auto pswitch = std::make_unique<SwitchInstr>(ident, ty, list);
    insertpoint_->AddInstr(std::move(pswitch));
}

void IRBuilder::InsertCallInstr(const FuncType* proto, const Function* pfunc,
    const CallInstr::ArgList& arglist)
{
    auto pcall = std::make_unique<CallInstr>(proto, pfunc, arglist);
    insertpoint_->AddInstr(std::move(pcall));
}


void IRBuilder::InsertAddInstr(const std::string& result, const IntType* ty,
    const std::string& lhs, const std::string& rhs)
{
    auto padd = std::make_unique<AddInstr>(result, ty, lhs, rhs);
    insertpoint_->AddInstr(std::move(padd));
}

void IRBuilder::InsertFaddInstr(const std::string& result, const FloatType* ty,
    const std::string& lhs, const std::string& rhs)
{
    auto pfadd = std::make_unique<FaddInstr>(result, ty, lhs, rhs);
    insertpoint_->AddInstr(std::move(pfadd));
}

void IRBuilder::InsertSubInstr(const std::string& result, const IntType* ty,
    const std::string& lhs, const std::string& rhs)
{
    auto psub = std::make_unique<SubInstr>(result, ty, lhs, rhs);
    insertpoint_->AddInstr(std::move(psub));
}

void IRBuilder::InsertFsubInstr(const std::string& result, const FloatType* ty,
    const std::string& lhs, const std::string& rhs)
{
    auto pfsub = std::make_unique<FsubInstr>(result, ty, lhs, rhs);
    insertpoint_->AddInstr(std::move(pfsub));
}

void IRBuilder::InsertMulInstr(const std::string& result, const IntType* ty,
    const std::string& lhs, const std::string& rhs)
{
    auto pmul = std::make_unique<MulInstr>(result, ty, lhs, rhs);
    insertpoint_->AddInstr(std::move(pmul));
}

void IRBuilder::InsertFmulInstr(const std::string& result, const FloatType* ty,
    const std::string& lhs, const std::string& rhs)
{
    auto pfmul = std::make_unique<FmulInstr>(result, ty, lhs, rhs);
    insertpoint_->AddInstr(std::move(pfmul));
}

void IRBuilder::InsertDivInstr(const std::string& result, const IntType* ty,
    const std::string& lhs, const std::string& rhs)
{
    auto pdiv = std::make_unique<DivInstr>(result, ty, lhs, rhs);
    insertpoint_->AddInstr(std::move(pdiv));
}

void IRBuilder::InsertFdivInstr(const std::string& result, const FloatType* ty,
    const std::string& lhs, const std::string& rhs)
{
    auto pfdiv = std::make_unique<FdivInstr>(result, ty, lhs, rhs);
    insertpoint_->AddInstr(std::move(pfdiv));
}

void IRBuilder::InsertModInstr(const std::string& result, const IntType* ty,
    const std::string& lhs, const std::string& rhs)
{
    auto pmod = std::make_unique<ModInstr>(result, ty, lhs, rhs);
    insertpoint_->AddInstr(std::move(pmod));
}

void IRBuilder::InsertShlInstr(const std::string& result, const IntType* ty,
    const std::string& lhs, const std::string& rhs)
{
    auto pshl = std::make_unique<ShlInstr>(result, ty, lhs, rhs);
    insertpoint_->AddInstr(std::move(pshl));
}

void IRBuilder::InsertLshrInstr(const std::string& result, const IntType* ty,
    const std::string& lhs, const std::string& rhs)
{
    auto plshr = std::make_unique<LshrInstr>(result, ty, lhs, rhs);
    insertpoint_->AddInstr(std::move(plshr));
}

void IRBuilder::InsertAshrInstr(const std::string& result, const IntType* ty,
    const std::string& lhs, const std::string& rhs)
{
    auto pashr = std::make_unique<AshrInstr>(result, ty, lhs, rhs);
    insertpoint_->AddInstr(std::move(pashr));
}

void IRBuilder::InsertAndInstr(const std::string& result, const IntType* ty,
    const std::string& lhs, const std::string& rhs)
{
    auto pand = std::make_unique<AndInstr>(result, ty, lhs, rhs);
    insertpoint_->AddInstr(std::move(pand));
}

void IRBuilder::InsertOrInstr(const std::string& result, const IntType* ty,
    const std::string& lhs, const std::string& rhs)
{
    auto por = std::make_unique<OrInstr>(result, ty, lhs, rhs);
    insertpoint_->AddInstr(std::move(por));
}

void IRBuilder::InsertXorInstr(const std::string& result, const IntType* ty,
    const std::string& lhs, const std::string& rhs)
{
    auto pxor = std::make_unique<XorInstr>(result, ty, lhs, rhs);
    insertpoint_->AddInstr(std::move(pxor));
}


void IRBuilder::InsertAllocaInstr(const std::string& result, const IRType* ty)
{
    auto palloca = std::make_unique<AllocaInstr>(result, ty);
    insertpoint_->AddInstr(std::move(palloca));
}

void IRBuilder::InsertAllocaInstr(const std::string& result, const IRType* ty, size_t num)
{
    auto palloca = std::make_unique<AllocaInstr>(result, ty, num);
    insertpoint_->AddInstr(std::move(palloca));
}

void IRBuilder::InsertAllocaInstr(const std::string& result, const IRType* ty, size_t num, size_t align)
{
    auto palloca = std::make_unique<AllocaInstr>(result, ty, num, align);
    insertpoint_->AddInstr(std::move(palloca));
}


void IRBuilder::InsertLoadInstr(const std::string& r, const PtrType* ty, const std::string& ptr)
{
    auto pload = std::make_unique<LoadInstr>(r, ty, ptr);
    insertpoint_->AddInstr(std::move(pload));
}

void IRBuilder::InsertLoadInstr(const std::string& r, const PtrType* ty, const std::string& ptr, size_t align)
{
    auto pload = std::make_unique<LoadInstr>(r, ty, ptr, align);
    insertpoint_->AddInstr(std::move(pload));
}
    

void IRBuilder::InsertStoreInstr(const PtrType* ptrty, const std::string& val, const std::string& ptr, bool vol)
{
    auto pstore = std::make_unique<StoreInstr>(ptrty, val, ptr, vol);
    insertpoint_->AddInstr(std::move(pstore));
}


void IRBuilder::InsertExValInstr(const std::string& result, const PtrType* ty, const std::string& val, int index)
{
    auto pexval = std::make_unique<ExtractValInstr>(result, ty, val, index);
    insertpoint_->AddInstr(std::move(pexval));
}

void IRBuilder::InsertSetValInstr(const std::string& newval, const PtrType* ty, const std::string& val, int index)
{
    auto psetval = std::make_unique<SetValInstr>(newval, ty, val, index);
    insertpoint_->AddInstr(std::move(psetval));
}

void IRBuilder::InsertGetElePtrInstr(const std::string& result, const PtrType* ptrty, const std::string& val, int index)
{
    auto pgeteleptr = std::make_unique<GetElePtrInstr>(result, ptrty, val, index);
    insertpoint_->AddInstr(std::move(pgeteleptr));
}


void IRBuilder::InsertTruncInstr(const std::string& result, const IntType* ty1,
    const std::string& val, const IntType* ty2)
{
    auto ptrunc = std::make_unique<TruncInstr>(result, ty1, val, ty2);
    insertpoint_->AddInstr(std::move(ptrunc));
}

void IRBuilder::InsertFtruncInstr(const std::string& result, const FloatType* ty1, const std::string& val, const FloatType* ty2)
{
    auto pftrunc = std::make_unique<FtruncInstr>(result, ty1, val, ty2);
    insertpoint_->AddInstr(std::move(pftrunc));
}

void IRBuilder::InsertZextInstr(const std::string& result, const IntType* ty1, const std::string& val, const IntType* ty2)
{
    auto pzext = std::make_unique<ZextInstr>(result, ty1, val, ty2);
    insertpoint_->AddInstr(std::move(pzext));
}

void IRBuilder::InsertSextInstr(const std::string& result, const IntType* ty1, const std::string& val, const IntType* ty2)
{
    auto psext = std::make_unique<SextInstr>(result, ty1, val, ty2);
    insertpoint_->AddInstr(std::move(psext));
}

void IRBuilder::InsertFextInstr(const std::string& result, const FloatType* ty1, const std::string& val, const FloatType* ty2)
{
    auto pfext = std::make_unique<FextInstr>(result, ty1, val, ty2);
    insertpoint_->AddInstr(std::move(pfext));
}

void IRBuilder::InsertFtouInstr(const std::string& result, const FloatType* ty1, const std::string& val, const IntType* ty2)
{
    auto pftou = std::make_unique<FtouInstr>(result, ty1, val, ty2);
    insertpoint_->AddInstr(std::move(pftou));
}

void IRBuilder::InsertFtosInstr(const std::string& result, const FloatType* ty1, const std::string& val, const IntType* ty2)
{
    auto pftos = std::make_unique<FtosInstr>(result, ty1, val, ty2);
    insertpoint_->AddInstr(std::move(pftos));
}

void IRBuilder::InsertUtofInstr(const std::string& result, const IntType* ty1, const std::string& val, const FloatType* ty2)
{
    auto putof = std::make_unique<UtofInstr>(result, ty1, val, ty2);
    insertpoint_->AddInstr(std::move(putof));
}

void IRBuilder::InsertStofInstr(const std::string& result, const IntType* ty1, const std::string& val, const FloatType* ty2)
{
    auto pstof = std::make_unique<StofInstr>(result, ty1, val, ty2);
    insertpoint_->AddInstr(std::move(pstof));
}

void IRBuilder::InsertPtrtoiInstr(const std::string& result, const PtrType* ty1, const std::string& val, const IntType* ty2)
{
    auto pptrtoi = std::make_unique<PtrtoiInstr>(result, ty1, val, ty2);
    insertpoint_->AddInstr(std::move(pptrtoi));
}

void IRBuilder::InsertItoptrInstr(const std::string& result, const IntType* ty1, const std::string& val, const PtrType* ty2)
{
    auto pitoptr = std::make_unique<ItoptrInstr>(result, ty1, val, ty2);
    insertpoint_->AddInstr(std::move(pitoptr));
}

void IRBuilder::InsertBitcastInstr(const std::string& result, const IRType* ty1, const std::string& val, const IRType* ty2)
{
    auto pbitcast = std::make_unique<BitcastInstr>(result, ty1, val, ty2);
    insertpoint_->AddInstr(std::move(pbitcast));
}


void IRBuilder::InsertIcmpInstr(const std::string& result, Condition cond, const IntType* ty, const std::string& lhs, const std::string& rhs)
{
    auto picmp = std::make_unique<IcmpInstr>(result, cond, ty, lhs, rhs);
    insertpoint_->AddInstr(std::move(picmp));
}

void IRBuilder::InsertFcmpInstr(const std::string& result, Condition cond, const FloatType* ty, const std::string& lhs, const std::string& rhs)
{
    auto pfcmp = std::make_unique<FcmpInstr>(result, cond, ty, lhs, rhs);
    insertpoint_->AddInstr(std::move(pfcmp));
}

void IRBuilder::InsertSelectInstr(const std::string& result, const std::string& selty, bool cond, const IRType* ty, const std::string& val1, const std::string& val2)
{
    auto pselect = std::make_unique<IcmpInstr>(result, selty, cond, ty, val1, val2);
    insertpoint_->AddInstr(std::move(pselect));
}

void IRBuilder::InsertPhiInstr(const std::string& ident, const IRType* ty, const PhiInstr::BlockValPairList& list)
{
    auto pphi = std::make_unique<PhiInstr>(ident, ty, list);
    insertpoint_->AddInstr(std::move(pphi));
}
