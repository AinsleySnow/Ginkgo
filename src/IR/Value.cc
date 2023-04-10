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

GlobalVar* Module::AddGlobalVar(std::unique_ptr<GlobalVar> var)
{
    auto pvar = var.get();
    globalsym_.push_back(std::move(var));
    symindex_.emplace(pvar->Name(), globalsym_.size() - 1);
    return pvar;
}

GlobalVar* Module::AddGlobalVar(const std::string& name, const IRType* ptype)
{
    return AddGlobalVar(std::make_unique<GlobalVar>(name, ptype));
}

Function* Module::GetFunction(const std::string& name)
{
    int index = symindex_.at(name);
    return static_cast<Function*>(globalsym_[index].get());
}

GlobalVar* Module::GetGlobalVar(const std::string& name)
{
    int index = symindex_.at(name);
    return static_cast<GlobalVar*>(globalsym_[index].get());
}


std::string Function::ToString() const
{
    std::string func = "def " + ReturnType()->ToString() + ' ';
    if (Inline()) func += "inline ";
    if (Noreturn()) func += "noreturn ";
    func += Name() + '(';

    if (Empty())
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

    if (Empty())
        func += ';';
    else
    {
        func += " {\n";
        for (auto i = elements_.begin(); i < elements_.end() - 1; ++i)
            func += (*i)->ToString() + '\n';
        func += elements_.back()->ToString() + '}';
    }

    return func;
}


BasicBlock* Function::GetBasicBlock(const std::string& name)
{
    for (auto& pbb : elements_)
        if (name == pbb->Name())
            return pbb.get();
    return nullptr;
}

BasicBlock* Function::GetBasicBlock(int index)
{
    if (index < 0)
        return (elements_.end() + index)->get();
    else
        return elements_[index].get();
}

void Function::AddIROperand(std::unique_ptr<IROperand> op)
{
    operands_.push_back(std::move(op));
}

void Function::AddParam(const Register* r)
{
    params_.push_back(r);
}


GlobalVar* GlobalVar::CreateGlobalVar(Module* mod, const std::string& name, const IRType* ty)
{
    auto var = std::make_unique<GlobalVar>(name, ty);
    auto pvar = var.get();
    mod->AddGlobalVar(std::move(var));
    return pvar;
}

std::string GlobalVar::ToString() const
{
    auto var = type_->ToString() + ' ' + name_ + ":\n";
    var += blk_->ToString();
    return var;
}

void GlobalVar::SetBasicBlock(std::unique_ptr<BasicBlock> bb)
{
    blk_ = std::move(bb);
}


BasicBlock* BasicBlock::CreateBasicBlock(Function* func, const std::string& name)
{
    std::unique_ptr<BasicBlock> bb = std::make_unique<BasicBlock>(name);
    auto raw = bb.get();
    func->Append(std::move(bb));
    return raw;
}

BasicBlock* BasicBlock::CreateBasicBlock(GlobalVar* global, const std::string& name)
{
    std::unique_ptr<BasicBlock> bb = std::make_unique<BasicBlock>(name);
    auto raw = bb.get();
    global->SetBasicBlock(std::move(bb));
    return raw;
}

std::string BasicBlock::ToString() const
{
    std::string blk{};
    if (!Name().empty()) blk += Name() + ":\n";
    for (auto i = elements_.begin(); i != elements_.end(); ++i)
        blk += "  " + (*i)->ToString() + ";\n";
    return blk;
}

void BasicBlock::MergePools(BasicBlock* bb)
{
    Pool<IRType>::Merge(bb);
    Pool<IROperand>::Merge(bb);
}
