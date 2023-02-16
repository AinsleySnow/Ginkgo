#include "IR/Value.h"


std::string Module::ToString() const
{
    std::string mod = "module " + Name() + ":\n";
    for (const auto& psym : globalsym_)
        mod += psym.second->ToString() + '\n';
    return mod;
}

Function* Module::AddFunc(const std::string& name, const FuncType* functy)
{
    auto func = std::make_unique<Function>(name, functy);
    auto pfunc = func.get();
    globalsym_.emplace(name, std::move(func));
    return pfunc;
}

GlobalVar* Module::AddGlobalVar(const std::string& name, const IRType* ptype)
{
    auto var = std::make_unique<GlobalVar>(name, std::move(ptype));
    auto pvar = var.get();
    globalsym_.emplace(name, std::move(var));
    return pvar;
}


std::string Function::ToString() const
{
    std::string func = "def " +
        ReturnType()->ToString() + ' ' + Name() + '(';
    for (auto param : ParamType())
        func += param->ToString();
    func += ") {\n";
    for (auto i = blk_.begin(); i < blk_.end() - 1; ++i)
        func += (*i)->ToString() + '\n';
    func += (*(blk_.end() - 1))->ToString() + '}';
    return func;
}

void Function::AddBasicBlock(std::unique_ptr<BasicBlock> bb)
{
    blk_.push_back(std::move(bb));

}

BasicBlock* Function::GetBasicBlock(const std::string& name)
{
    for (auto& pbb : blk_)
        if (name == pbb->Name())
            return pbb.get();
    return nullptr;
}

BasicBlock* Function::GetBasicBlock(int index)
{
    if (index < 0)
        return (blk_.end() + index)->get();
    else
        return blk_[index].get();
}

void Function::AddIROperand(std::unique_ptr<IROperand> op)
{
    operands_.push_back(std::move(op));
}


BasicBlock* BasicBlock::CreateBasicBlock(Function* func, const std::string& name)
{
    std::unique_ptr<BasicBlock> bb = std::make_unique<BasicBlock>(name);
    bb->parent_ = func;
    auto raw = bb.get();
    func->AddBasicBlock(std::move(bb));
    return raw;
}

std::string BasicBlock::ToString() const
{ 
    std::string blk = Name() + ":\n";
    for (const auto& instr : instrs_)
        blk += "  " + instr->ToString() + ";\n";
    return blk;
}
