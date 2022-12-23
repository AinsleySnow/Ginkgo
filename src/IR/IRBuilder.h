#ifndef _IR_BUILDER_H_
#define _IR_BUILDER_H_

#include "Instr.h"

class BasicBlock;


class IRBuilder
{
public:
    void SetInsertPoint(BasicBlock* bb) { insertpoint_ = bb; }

    void InsertInstr(std::unique_ptr<Instr> instr);

    void InsertRetInstr(const IRType* valtype, const std::string& val);
    
    void InsertBrInstr(const std::string& label);
    void InsertBrInstr(const std::string& cond, const std::string& tlabel, const std::string& flabel);
    
    void InsertSwitchInstr(const std::string& ident, const IntType* ty,
        const SwitchInstr::LabelValPair& list);
    
    void InsertCallInstr(const FuncType* proto, const std::string& pfunc,
        const CallInstr::ArgList& arglist);

    void InsertAddInstr(const std::string& result, const IntType* ty,
        const std::string& lhs, const std::string& rhs);
    void InsertFaddInstr(const std::string& r, const FloatType* ty,
        const std::string& lhs, const std::string& rhs);
    void InsertSubInstr(const std::string& result, const IntType* ty,
        const std::string& lhs, const std::string& rhs);
    void InsertFsubInstr(const std::string& result, const FloatType* ty,
        const std::string& lhs, const std::string& rhs);
    void InsertMulInstr(const std::string& result, const IntType* ty,
        const std::string& lhs, const std::string& rhs);
    void InsertFmulInstr(const std::string& result, const FloatType* ty,
        const std::string& lhs, const std::string& rhs);
    void InsertDivInstr(const std::string& result, const IntType* ty,
        const std::string& lhs, const std::string& rhs);
    void InsertFdivInstr(const std::string& result, const FloatType* ty,
        const std::string& lhs, const std::string& rhs);
    void InsertModInstr(const std::string& result, const IntType* ty,
        const std::string& lhs, const std::string& rhs);
    void InsertShlInstr(const std::string& result, const IntType* ty,
        const std::string& lhs, const std::string& rhs);
    void InsertLshrInstr(const std::string& result, const IntType* ty,
        const std::string& lhs, const std::string& rhs);
    void InsertAshrInstr(const std::string& result, const IntType* ty,
        const std::string& lhs, const std::string& rhs);
    void InsertAndInstr(const std::string& result, const IntType* ty,
        const std::string& lhs, const std::string& rhs);
    void InsertOrInstr(const std::string& result, const IntType* ty,
        const std::string& lhs, const std::string& rhs);
    void InsertXorInstr(const std::string& result, const IntType* ty,
        const std::string& lhs, const std::string& rhs);

    void InsertAllocaInstr(const std::string& result, const IRType* ty);
    void InsertAllocaInstr(const std::string& result, const IRType* ty, size_t num);
    void InsertAllocaInstr(const std::string& result, const IRType* ty, size_t num, size_t align);
    
    void InsertLoadInstr(const IRType* ty, const std::string& ptr);
    void InsertLoadInstr(const IRType* ty, const std::string& ptr, size_t align);
    
    void InsertStoreInstr(const IRType* valty, const std::string& val, const IRType* ptrty,
        const std::string& ptr, bool vol);
    
    void InsertExValInstr(const std::string& result, const IRType* ty, const std::string& val, int index);
    void InsertSetValInstr(const std::string& newval, const IRType* ty, const std::string& val, int index);
    void InsertGetElePtrInstr(const std::string& result, const IRType* ty, const std::string& val, int index);

    void InsertTruncInstr(const std::string& result, const IntType* ty1, const std::string& val, const IntType* ty2);
    void InsertFtruncInstr(const std::string& result, const FloatType* ty1, const std::string& val, const FloatType* ty2);
    void InsertZextInstr(const std::string& result, const IntType* ty1, const std::string& val, const IntType* ty2);
    void InsertSextInstr(const std::string& result, const IntType* ty1, const std::string& val, const IntType* ty2);
    void InsertFextInstr(const std::string& result, const FloatType* ty1, const std::string& val, const FloatType* ty2);

    void InsertFtouInstr(const std::string& result, const FloatType* ty1, const std::string& val, const IntType* ty2);
    void InsertFtosInstr(const std::string& result, const FloatType* ty1, const std::string& val, const IntType* ty2);
    void InsertUtofInstr(const std::string& result, const IntType* ty1, const std::string& val, const FloatType* ty2);
    void InsertStofInstr(const std::string& result, const IntType* ty1, const std::string& val, const FloatType* ty2);
    void InsertPtrtoiInstr(const std::string& result, const PtrType* ty1, const std::string& val, const IntType* ty2);
    void InsertItoptrInstr(const std::string& result, const IntType* ty1, const std::string& val, const PtrType* ty2);
    void InsertBitcastInstr(const std::string& result, const IRType* ty1, const std::string& val, const IRType* ty2);

    void InsertIcmpInstr(const std::string& result, Condition cond, const IntType* ty, const std::string& lhs, const std::string& rhs);
    void InsertFcmpInstr(const std::string& result, Condition cond, const FloatType* ty, const std::string& lhs, const std::string& rhs);
    void InsertSelectInstr(const std::string& result, const std::string& selty, bool cond, const IRType* ty, const std::string& val1, const std::string& val2);
    void InsertPhiInstr(const std::string& ident, std::unique_ptr<IRType> ty, const PhiInstr::LabelValPairList& list);


private:
    BasicBlock* insertpoint_{};
};


#endif // _IR_BUILDER_H_
