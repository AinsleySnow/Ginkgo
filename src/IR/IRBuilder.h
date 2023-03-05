#ifndef _IR_BUILDER_H_
#define _IR_BUILDER_H_

#include "IR/Instr.h"

class BasicBlock;


class IRBuilder
{
public:
    IRBuilder() {}
    IRBuilder(const IRBuilder&) = delete;
    IRBuilder(IRBuilder&&) = delete;

    auto& InsertPoint() { return insertpoint_; }
    auto InsertPoint() const { return insertpoint_; }

    void InsertInstr(std::unique_ptr<Instr> instr);
    Instr* GetLastInstr();

    void InsertRetInstr();
    void InsertRetInstr(const IROperand* val);

    void InsertBrInstr(const BasicBlock* label);
    void InsertBrInstr(
        const IROperand* cond, const BasicBlock* tlabel, const BasicBlock* flabel);

    void InsertSwitchInstr(const IROperand* ident);

    const Register* InsertCallInstr(
        const std::string& result, const FuncType* proto, const std::string& func);
    const Register* InsertCallInstr(const std::string& result, const Register* func);

    const Register* InsertAddInstr(
        const std::string& result, const IROperand* lhs, const IROperand* rhs);
    const Register* InsertFaddInstr(
        const std::string& result, const IROperand* lhs, const IROperand* rhs);
    const Register* InsertSubInstr(
        const std::string& result, const IROperand* lhs, const IROperand* rhs);
    const Register* InsertFsubInstr(
        const std::string& result, const IROperand* lhs, const IROperand* rhs);
    const Register* InsertMulInstr(
        const std::string& result, const IROperand* lhs, const IROperand* rhs);
    const Register* InsertFmulInstr(
        const std::string& result, const IROperand* lhs, const IROperand* rhs);
    const Register* InsertDivInstr(
        const std::string& result, const IROperand* lhs, const IROperand* rhs);
    const Register* InsertFdivInstr(
        const std::string& result, const IROperand* lhs, const IROperand* rhs);
    const Register* InsertModInstr(
        const std::string& result, const IROperand* lhs, const IROperand* rhs);
    const Register* InsertShlInstr(
        const std::string& result, const IROperand* lhs, const IROperand* rhs);
    const Register* InsertLshrInstr(
        const std::string& result, const IROperand* lhs, const IROperand* rhs);
    const Register* InsertAshrInstr(
        const std::string& result, const IROperand* lhs, const IROperand* rhs);
    const Register* InsertAndInstr(
        const std::string& result, const IROperand* lhs, const IROperand* rhs);
    const Register* InsertOrInstr(
        const std::string& result, const IROperand* lhs, const IROperand* rhs);
    const Register* InsertXorInstr(
        const std::string& result, const IROperand* lhs, const IROperand* rhs);

    const Register* InsertAllocaInstr(
        const std::string& result, const IRType* ty);
    const Register* InsertAllocaInstr(
        const std::string& result, const IRType* ty, size_t num);
    const Register* InsertAllocaInstr(
        const std::string& result, const IRType* ty, size_t num, size_t align);
    
    const Register* InsertLoadInstr(
        const std::string& result, const Register* ptr);
    const Register* InsertLoadInstr(
        const std::string& result, const Register* ptr, size_t align);
    const Register* InsertLoadInstr(
        const std::string& result, const Register* ptr, size_t align, bool vol);

    void InsertStoreInstr(const IROperand* val, const Register* ptr, bool vol);
    
    const Register* InsertExtractValInstr(
        const std::string& result, const Register* val, int index);
    void InsertSetValInstr(
        const IROperand* newval, const Register* val, int index);
    const Register* InsertGetElePtrInstr(
        const std::string& result, const Register* val, int index);


    const Register* InsertTruncInstr(
        const std::string& result, const IntType* ty, const Register* val);
    const Register* InsertFtruncInstr(
        const std::string& result, const FloatType* ty, const Register* val);
    const Register* InsertZextInstr(
        const std::string& result, const IntType* ty, const Register* val);
    const Register* InsertSextInstr(
        const std::string& result, const IntType* ty, const Register* val);
    const Register* InsertFextInstr(
        const std::string& result, const FloatType* ty, const Register* val);

    const Register* InsertFtouInstr(
        const std::string& r, const IntType* t, const Register* v);
    const Register* InsertFtosInstr(
        const std::string& r, const IntType* t, const Register* v);
    const Register* InsertUtofInstr(
        const std::string& r, const FloatType* t1, const Register* v);
    const Register* InsertStofInstr(
        const std::string& r, const FloatType* t, const Register* v);
    const Register* InsertPtrtoiInstr(
        const std::string& r, const IntType* t, const Register* v);
    const Register* InsertItoptrInstr(
        const std::string& r, const PtrType* t, const Register* v);
    const Register* InsertBitcastInstr(
        const std::string& r, const IRType* t, const Register* v);

    const Register* InsertCmpInstr(
        const std::string& result, Condition c, const IROperand* lhs, const IROperand* rhs);
    const Register* InsertIcmpInstr(
        const std::string& result, Condition c, const IROperand* lhs, const IROperand* rhs);
    const Register* InsertFcmpInstr(
        const std::string& result, Condition c, const IROperand* lhs, const IROperand* rhs);

    const Register* InsertSelectInstr(
        const std::string& result, const IROperand* selty, 
        bool cond, const IROperand* lhs, const IROperand* rhs);
    const Register* InsertPhiInstr(const std::string& result, const IRType* ty);


private:
    void MatchArithmType(const IRType*, const IROperand*&);
    void MatchArithmType(const IROperand*&, const IROperand*&);

    BasicBlock* insertpoint_{};
};


#endif // _IR_BUILDER_H_
