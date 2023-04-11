#ifndef _IR_BUILDER_H_
#define _IR_BUILDER_H_

#include "IR/Instr.h"
#include "IR/Value.h"


template <class CNT, class ELE>
            // container, element
class IRBuilderBase
{
public:
    IRBuilderBase() = default;
    virtual ~IRBuilderBase() = default;
    IRBuilderBase(const IRBuilderBase&) = delete;
    IRBuilderBase(IRBuilderBase&&) = delete;

    // Set the insert point to the end of a CNT.
    void SetInsertPoint(CNT* container)
    {
        insertmode_ = InsertMode::end;
        insertpoint_.container_ = container;
        insertpoint_.index_ = container->Size();
    }
    // Equivalent to SetInsertPoint(current_container, element).
    void SetInsertPoint(ELE* element)
    {
        insertmode_ = InsertMode::front;
        insertpoint_.index_ = insertpoint_.container_->IndexOf(element);
    }
    // Set the insert point to the front of a ELE in CNT.
    void SetInsertPoint(CNT* container, ELE* element)
    {
        insertmode_ = InsertMode::front;
        insertpoint_.container_ = container;
        insertpoint_.index_ = container->IndexOf(element);
    }
    // Set the insert point to a certain position.
    // Suppose insertpoint_.container_ is already set.
    void SetInsertPoint(typename CNT::IterType iter)
    {
        insertmode_ = InsertMode::front;
        insertpoint_.index_ = std::distance(insertpoint_.container_->begin(), iter);
    }

    auto Container() { return insertpoint_.container_; }
    auto InsertPoint()
    {
        return insertpoint_.container_->begin() +
            insertpoint_.index_;
    }

    void Insert(std::unique_ptr<ELE> ele)
    {
        if (insertmode_ == InsertMode::end)
            insertpoint_.container_->Append(std::move(ele));
        else if (insertmode_ == InsertMode::front)
            insertpoint_.container_->Insert(
                insertpoint_.index_, std::move(ele));
        insertpoint_.index_++;
    }

    void Remove()
    {
        if (insertmode_ == InsertMode::end)
        {
            insertpoint_.container_->Remove();
            insertpoint_.index_--;
        }
        else if (insertmode_ == InsertMode::front)
            insertpoint_.container_->Remove(insertpoint_.index_);
    }

    void Remove(ELE* ele)
    {
        auto index = insertpoint_.container_->IndexOf(ele);
        insertpoint_.container_->Remove(index);
        if (insertmode_ == InsertMode::end)
            insertpoint_.index_--;
        else if (insertmode_ == InsertMode::front &&
            InsertPoint() == insertpoint_.container_->end())
        {
            insertpoint_.index_ = insertpoint_.container_->Size();
            insertmode_ = InsertMode::end;
        }
    }


private:
    enum class InsertMode { end, front };
    InsertMode insertmode_{};
    struct
    {
        CNT* container_{};
        int index_{};
    } insertpoint_;
};


class BlockBuilder : public IRBuilderBase<Function, BasicBlock>
{
public:
    void InsertBasicBlock(const std::string&);
    BasicBlock* GetBasicBlock(const std::string&);

    void PopBack();
    void RemoveBlk(BasicBlock*);
};


class InstrBuilder : public IRBuilderBase<BasicBlock, Instr>
{
public:
    Instr* LastInstr() { return *std::prev(InsertPoint()); }

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
        const std::string& result, const Register* val, const IROperand* index);
    void InsertSetValInstr(
        const IROperand* newval, const Register* val, const IROperand* index);
    const Register* InsertGetElePtrInstr(
        const std::string& result, const Register* val, const IROperand* index);


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
};


#endif // _IR_BUILDER_H_
