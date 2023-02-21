#include "IR/Value.h"


std::string Module::ToString() const
{
    std::string mod = "module " + Name() + ":\n";
    for (const auto& psym : globalsym_)
        mod += psym->ToString() + "\n\n";
    return mod;
}

Function* Module::AddFunc(std::unique_ptr<Function> func)
{
    auto pfunc = func.get();
    globalsym_.push_back(std::move(func));
    symindex_.emplace(pfunc->Name(), globalsym_.size() - 1);
    return pfunc;
}

Function* Module::AddFunc(const std::string& name, const FuncType* functy)
{
    return AddFunc(std::make_unique<Function>(name, functy));
}

GlobalVar* Module::AddGlobalVar(const std::string& name, const IRType* ptype)
{
    auto var = std::make_unique<GlobalVar>(name, std::move(ptype));
    auto pvar = var.get();
    globalsym_.push_back(std::move(var));
    symindex_.emplace(pvar->Name(), globalsym_.size() - 1);
    return pvar;
}

Function* Module::GetFunction(const std::string& name)
{
    return static_cast<Function*>(globalsym_[symindex_[name]].get());
}


std::string Function::ToString() const
{
    std::string func = "def " + ReturnType()->ToString() + ' ';
    if (Inline()) func += "inline ";
    if (Noreturn()) func += "noreturn ";
    func += Name() + '(';

    if (blk_.empty())
    {
        auto& paramtypes = ParamType();
        if (!paramtypes.empty())
        {
            for (auto paramtype = paramtypes.begin(); paramtype < paramtypes.end() - 1; ++paramtype)
                func += (*paramtype)->ToString() + ", ";
            func += paramtypes.back()->ToString();
        }
    }
    else
    {
        auto& params = Params();
        if (!params.empty())
        {
            for (auto param = params.begin(); param < params.end() - 1; ++param)
                func += (*param)->ToString() + ", ";
            func += params.back()->ToString();
        }
    }
    func += ')';

    if (blk_.empty())
        func += ";";
    else
    {
        func += " {\n";
        for (auto i = blk_.begin(); i < blk_.end() - 1; ++i)
            func += (*i)->ToString() + '\n';
        func += blk_.back()->ToString() + '}';
    }

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

void Function::AddParam(const Register* r)
{
    params_.push_back(r);
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
