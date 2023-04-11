#ifndef _VALUE_H_
#define _VALUE_H_

#include "IR/IROperand.h"
#include "IR/IRType.h"
#include "IR/Instr.h"
#include "utils/Container.h"
#include "utils/DynCast.h"
#include "utils/Pool.h"
#include <algorithm>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

class Function;
class GlobalVar;
class IRVisitor;


class Value
{
protected:
    enum class ValueId { value, module, function, globalvar, basicblock };
    static bool ClassOf(const Value const*) { return true; }
    ValueId id_ = ValueId::value;

public:
    ENABLE_IS;
    ENABLE_AS;

    Value(const std::string& n) : name_(n) {}

    virtual ~Value() {}
    virtual std::string ToString() const { return ""; }
    virtual void Accept(IRVisitor*) {}

    ValueId ID() const { return id_; }
    std::string Name() const { return name_; }


private:
    std::string name_{};
};


class Module : public Value, public Container<Value>,
               public Pool<IRType>, public Pool<IROperand>
{
protected:
    static bool ClassOf(const Module const*) { return true; }
    static bool ClassOf(const Value const* v) { return v->ID() == ValueId::module; }

public:
    Module(const std::string& n) : Value(n) { id_ = ValueId::module; }

    std::string ToString() const override;
    void Accept(IRVisitor*) override;

    Function* AddFunc(std::unique_ptr<Function>);
    Function* AddFunc(const std::string&, const FuncType*);
    GlobalVar* AddGlobalVar(std::unique_ptr<GlobalVar>);
    GlobalVar* AddGlobalVar(const std::string&, const IRType*);
    Function* GetFunction(const std::string&);
    GlobalVar* GetGlobalVar(const std::string&);


private:
    std::unordered_map<std::string, int> symindex_{};
};


class Function : public Value, public Container<BasicBlock>
{
protected:
    static bool ClassOf(const Function const*) { return true; }
    static bool ClassOf(const Value const* v) { return v->ID() == ValueId::function; }

public:
    static Function* CreateFunction(Module*, const FuncType*);
    Function(const std::string& n, const FuncType* f) :
        Value(n), functype_(f) { id_ = ValueId::function; }

    std::string ToString() const override;
    void Accept(IRVisitor*) override;

    auto& Addr() { return addr_; }
    const auto Addr() const { return addr_; }

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
    const Register* addr_{};

    std::vector<std::unique_ptr<IROperand>> operands_{};

    bool inline_{}, noreturn_{};

    const Register* returnvalue_{};
    const FuncType* functype_{};
    std::vector<const Register*> params_{};
};


class GlobalVar : public Value
{
protected:
    static bool ClassOf(const Module const*) { return true; }
    static bool ClassOf(const Value const* v) { return v->ID() == ValueId::globalvar; }

public:
    static GlobalVar* CreateGlobalVar(Module*, const std::string&, const IRType*);
    GlobalVar(const std::string& n, const IRType* t) :
        Value(n), type_(t) { id_ = ValueId::globalvar; }

    std::string ToString() const override;
    void Accept(IRVisitor*) override;

    auto& Addr() { return addr_; }
    const auto Addr() const { return addr_; }

    void SetBasicBlock(std::unique_ptr<BasicBlock>);
    BasicBlock* GetBasicBlock() { return blk_.get(); }


private:
    const IRType* type_{};
    const Register* addr_{};
    std::unique_ptr<BasicBlock> blk_{};
};


class BasicBlock : public Value, public Container<Instr>,
                   public Pool<IROperand>, public Pool<IRType>
{
protected:
    static bool ClassOf(const BasicBlock const*) { return true; }
    static bool ClassOf(const Value const* v) { return v->ID() == ValueId::basicblock; }

public:
    static BasicBlock* CreateBasicBlock(Function*, const std::string&);
    static BasicBlock* CreateBasicBlock(GlobalVar*, const std::string&);
    BasicBlock(const std::string& n) : Value(n) { id_ = ValueId::basicblock; }

    std::string ToString() const override;
    void Accept(IRVisitor*) override;

    Instr* LastInstr() { return *std::prev(end()); }
    const Instr* LastInstr() const { return LastInstr(); }

    void MergePools(BasicBlock*);
};


#endif // _VALUE_H_
