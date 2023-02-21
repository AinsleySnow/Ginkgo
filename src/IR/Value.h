#ifndef _VALUE_H_
#define _VALUE_H_

#include "IR/Instr.h"
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

    virtual Value* Parent() const { return parent_; }
    std::string Name() const { return name_; }

protected:
    Value* parent_{};
    std::string name_{};
};


class Module : public Value
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

    auto& TypePool() { return pool_; }


private:
    std::vector<std::unique_ptr<Value>> globalsym_{};
    std::unordered_map<std::string, int> symindex_{};
    IRTypePool pool_{};
};


class Function : public Value
{
public:
    static Function* CreateFunction(Module*, const FuncType*);
    Function(const std::string& n, const FuncType* f) :
        Value(n), functype_(f) {}

    std::string ToString() const override;
    Module* Parent() const override { return static_cast<Module*>(parent_); }

    void AddBasicBlock(std::unique_ptr<BasicBlock>);
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

    auto& TypePool() { return pool_; }


private:
    std::vector<std::unique_ptr<BasicBlock>> blk_{};
    std::vector<std::unique_ptr<IROperand>> operands_{};

    bool inline_{}, noreturn_{};

    const Register* returnvalue_{};
    const FuncType* functype_{};
    std::vector<const Register*> params_{};

    IRTypePool pool_{};
};


class GlobalVar : public Value
{
public:
    static GlobalVar* CreateGlobalVar(Module*, const std::string&, const IRType*);
    GlobalVar(const std::string& n, const IRType* t) :
        Value(n), type_(t) {}

    std::string ToString() const override
    { return type_->ToString() + ' ' + name_ + ';'; }
    Module* Parent() const override { return static_cast<Module*>(parent_); }

private:
    const IRType* type_{};
};


class BasicBlock : public Value
{
public:
    static BasicBlock* CreateBasicBlock(Function*, const std::string&);
    BasicBlock(const std::string& n) : Value(n) {}

    std::string ToString() const override;
    Function* Parent() const override { return static_cast<Function*>(parent_); }

    void AddInstr(std::unique_ptr<Instr> instr)
    { instrs_.push_back(std::move(instr)); }
    Instr* GetLastInstr() { return instrs_.back().get(); }
    const Instr* GetLastInstr() const { return instrs_.back().get(); }
    bool Empty() const { return instrs_.empty(); }

private:
    std::vector<std::unique_ptr<Instr>> instrs_{};
};


#endif // _VALUE_H_
