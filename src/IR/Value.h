#ifndef _VALUE_H_
#define _VALUE_H_

#include "Instr.h"
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

class IRType;


class Value
{
public:
    Value(const std::string& n) : name_(n) {}

    virtual ~Value() {}
    virtual std::string ToString() const { return ""; }

    virtual Module* ToModule() { return nullptr; }
    virtual Function* ToFunc() { return nullptr; }
    virtual GlobalVar* ToGlobalVar() { return nullptr; }
    BasicBlock* ToBasicBlock() { return nullptr; }

    std::string GetName() const { return Value::name_; }

protected:
    std::string name_{};
};


class Module : public Value
{
public:
    Module(const std::string& n) : Value(n) {}

    std::string ToString() const override;

    Function* AddFunc(const std::string&, const std::vector<const IRType*>&);
    GlobalVar* AddGlobalVar(const std::string&, const IRType*);


private:
    std::vector<std::unique_ptr<Value>> globalsym_{};
};


class Function : public Value
{
public:
    Function(const std::string& n, const std::vector<const IRType*>& vt) :
        Value(n), params_(vt) {}

    std::string ToString() const override;

    BasicBlock* AddBasicBlock(const std::string&);
    BasicBlock* GetBasicBlock(const std::string&);
    BasicBlock* GetBasicBlock(int);

    void AddSymbol(const std::string&, const IRType*);
    const IRType* GetSymbolType(const std::string&) const;


private:
    std::vector<const IRType*> params_{};
    std::vector<std::unique_ptr<BasicBlock>> blk_{};
    std::unordered_map<std::string, const IRType*> table_{};
};


class GlobalVar : public Value
{
public:
    GlobalVar(const std::string& n, const IRType* t) :
        Value(n), type_(t) {}

    std::string ToString() const override
    { return type_->ToString() + ' ' + name_ + ';'; }


private:
    const IRType* type_{};
};


class BasicBlock : public Value
{
public:
    BasicBlock(const std::string& n) : Value(n) {}

    void AddInstr(std::unique_ptr<Instr> instr)
    { instrs_.push_back(std::move(instr)); }
    Instr* GetLastInstr() { return instrs_.back().get(); }
    const Instr* GetLastInstr() const { return instrs_.back().get(); }

    std::string ToString() const override;

private:
    std::vector<std::unique_ptr<Instr>> instrs_{};
};


#endif // _VALUE_H_
