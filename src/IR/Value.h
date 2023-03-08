#ifndef _VALUE_H_
#define _VALUE_H_

#include "IR/Instr.h"
#include <algorithm>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

class IRType;
class Function;
class GlobalVar;


class Value
{
public:
    Value(const std::string& n) : name_(n) {}

    virtual ~Value() {}
    virtual std::string ToString() const { return ""; }

    std::string Name() const { return name_; }

protected:
    std::string name_{};
};


class Module : public Value, public MemPool<IRType>
{
public:
    Module(const std::string& n) : Value(n) {}

    std::string ToString() const override;

    Function* AddFunc(std::unique_ptr<Function>);
    Function* AddFunc(const std::string&, const FuncType*);
    GlobalVar* AddGlobalVar(std::unique_ptr<GlobalVar>);
    GlobalVar* AddGlobalVar(const std::string&, const IRType*);
    Function* GetFunction(const std::string&);
    GlobalVar* GetGlobalVar(const std::string&);


private:
    std::vector<std::unique_ptr<Value>> globalsym_{};
    std::unordered_map<std::string, int> symindex_{};
};


template <class ELE>
class Container
{
public:
    class Iterator: public std::iterator<
        std::random_access_iterator_tag, ELE*, ptrdiff_t,
        std::unique_ptr<ELE>*,
        std::unique_ptr<ELE>&>
    {
        using ptr = std::unique_ptr<ELE>;

    public:
        Iterator(ptr* p, size_t i = 0) : start_(p), index_(i) {}

        Iterator& operator++() { index_++; return *this; }
        Iterator operator++(int) { Iterator retval = *this; ++(*this); return retval; }
        Iterator& operator--() { index_--; return *this; }
        Iterator operator--(int) { Iterator retval = *this; --(*this); return retval; }

        Iterator& operator+=(size_t off) { index_ += off; return *this; }
        Iterator& operator-=(size_t off) { index_ -= off; return *this; }
        Iterator operator+(size_t off) const { Iterator retval = *this; return retval += off; }
        Iterator operator-(size_t off) const { Iterator retval = *this; return retval -= off; }
        size_t operator+(Iterator other) const { return index_ + other.index_; }
        size_t operator-(Iterator other) const { return index_ - other.index_; }

        bool operator==(Iterator other) const
        {
            return start_ == other.start_ &&
                index_ == other.index_;
        }
        bool operator!=(Iterator other) const { return !(*this == other); }

        ELE* operator*() const { return start_[index_].get(); }

    private:
        ptr* start_{};
        size_t index_{};
    };

    auto begin() { return Iterator(elements_.data()); }
    auto end() { return Iterator(elements_.data(), elements_.size()); }

    void Append(std::unique_ptr<ELE> ele)
    {
        elements_.push_back(std::move(ele));
    }
    void Insert(int i, std::unique_ptr<ELE> ele)
    {
        elements_.insert(elements_.begin() + i,
            std::move(ele));
    }
    void Remove() { elements_.pop_back(); }
    void Remove(int i) { elements_.erase(elements_.begin() + i); }

    bool Empty() const { return elements_.empty(); }
    auto Size() const { return elements_.size(); }

    ELE* At(int i) { return elements_[i].get(); }
    int IndexOf(const ELE* ptr) const
    {
        auto begin = std::make_move_iterator(elements_.begin());
        auto end = std::make_move_iterator(elements_.end());
        auto pos = std::find_if(begin, end,
            [ptr] (const auto& ele) { return ptr == ele.get(); });
        return pos == end ? -1 : std::distance(begin, pos);
    }


protected:
    std::vector<std::unique_ptr<ELE>> elements_{};
};


class Function : public Value, public Container<BasicBlock>
{
public:
    static Function* CreateFunction(Module*, const FuncType*);
    Function(const std::string& n, const FuncType* f) :
        Value(n), functype_(f) {}

    std::string ToString() const override;

    BasicBlock* GetBasicBlock(const std::string&);
    BasicBlock* GetBasicBlock(int);
    void AddIROperand(std::unique_ptr<IROperand>);
    void AddParam(const Register*);

    auto Type() const { return functype_; }
    const auto& Params() const { return params_; }
    const auto& ParamType() const { return functype_->ParamType(); }
    const IRType* ReturnType() const { return functype_->ReturnType(); }

    bool Inline() const { return inline_; }
    bool& Inline() { return inline_; }
    bool Noreturn() const { return noreturn_; }
    bool& Noreturn() { return noreturn_; }

    auto ReturnValue() const { return returnvalue_; }
    auto& ReturnValue() { return returnvalue_; }


private:
    std::vector<std::unique_ptr<IROperand>> operands_{};

    bool inline_{}, noreturn_{};

    const Register* returnvalue_{};
    const FuncType* functype_{};
    std::vector<const Register*> params_{};
};


class GlobalVar : public Value
{
public:
    static GlobalVar* CreateGlobalVar(Module*, const std::string&, const IRType*);
    GlobalVar(const std::string& n, const IRType* t) :
        Value(n), type_(t) {}

    std::string ToString() const override;

    void SetBasicBlock(std::unique_ptr<BasicBlock>);
    BasicBlock* GetBasicBlock() { return blk_.get(); }


private:
    const IRType* type_{};
    std::unique_ptr<BasicBlock> blk_{};
};


class BasicBlock : public Value, public Container<Instr>,
                   public MemPool<IROperand>, public MemPool<IRType>
{
public:
    static BasicBlock* CreateBasicBlock(Function*, const std::string&);
    static BasicBlock* CreateBasicBlock(GlobalVar*, const std::string&);
    BasicBlock(const std::string& n) : Value(n) {}

    std::string ToString() const override;

    Instr* LastInstr() { return *std::prev(end()); }
    const Instr* LastInstr() const { return LastInstr(); }

    void MergePools(BasicBlock*);
};


#endif // _VALUE_H_
