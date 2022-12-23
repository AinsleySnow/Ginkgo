#include "Value.h"


std::string Module::ToString() const
{
    std::string mod{};
    for (const auto& psym : globalsym_)
        mod += psym->ToString() + '\n';
    return mod;
}

Function* Module::AddFunc(const std::string& name,
    const std::vector<const IRType*>& param)
{
    auto func = std::make_unique<Function>(name, param);
    auto pfunc = func.get();
    globalsym_.push_back(std::move(func));
    return pfunc;
}

GlobalVar* Module::AddGlobalVar(const std::string& name, const IRType* ptype)
{
    auto var = std::make_unique<GlobalVar>(name, std::move(ptype));
    auto pvar = var.get();
    globalsym_.push_back(std::move(var));
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

void Function::AddSymbol(const std::string& name, const IRType* ty)
{
    if (table_.count(name)) return;
    table_[name] = ty;
}

const IRType* Function::GetSymbolType(const std::string& name) const
{
    if (!table_.count(name)) return nullptr;
    return table_.at(name);
}


std::string BasicBlock::ToString() const
{ 
    std::string blk{};
    for (const auto& instr : instrs_)
        blk += instr->ToString() + ";\n";
    return blk;
}

