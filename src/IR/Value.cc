#include "IR/Value.h"


std::string Module::ToString() const
{
    std::string mod{};
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
    std::string func{};
    for (const auto& pb : blk_)
        func += pb->ToString() + '\n';
    return func;
}

BasicBlock* Function::AddBasicBlock(const std::string& name)
{
    auto bb = std::make_unique<BasicBlock>(name);
    auto pbb = bb.get();
    blk_.push_back(std::move(bb));
    return pbb; 
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


std::string BasicBlock::ToString() const
{ 
    std::string blk{};
    for (const auto& instr : instrs_)
        blk += instr->ToString() + ";\n";
    return blk;
}
