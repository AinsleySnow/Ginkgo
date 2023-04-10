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


class Module : public Value, public Pool<IRType>, public Pool<IROperand>
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


class Function : public Value, public Container<BasicBlock>
{
public:
    static Function* CreateFunction(Module*, const FuncType*);
    Function(const std::string& n, const FuncType* f) :
        Value(n), functype_(f) {}

    std::string ToString() const override;

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
public:
    static GlobalVar* CreateGlobalVar(Module*, const std::string&, const IRType*);
    GlobalVar(const std::string& n, const IRType* t) :
        Value(n), type_(t) {}

    std::string ToString() const override;

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
