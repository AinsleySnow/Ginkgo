#include "IR/Value.h"
#include "visitir/IRVisitor.h"


std::string Module::ToString() const
{
    std::string mod = "module " + Name() + ":\n";
    for (auto psym : *this)
        mod += psym->ToString() + "\n\n";
    return mod;
}

void Module::Accept(IRVisitor* v)
{
    v->VisitModule(this);
}

Function* Module::AddFunc(std::unique_ptr<Function> func)
{
    auto pfunc = func.get();
    Append(std::move(func));
    symindex_.emplace(pfunc->Name(), Size() - 1);
    return pfunc;
}

Function* Module::AddFunc(const std::string& name, const FuncType* functy)
{
    return AddFunc(std::make_unique<Function>(name, functy));
}

GlobalVar* Module::AddGlobalVar(std::unique_ptr<GlobalVar> var)
{
    auto pvar = var.get();
    Append(std::move(var));
    symindex_.emplace(pvar->Name(), Size() - 1);
    return pvar;
}

GlobalVar* Module::AddGlobalVar(const std::string& name, const IRType* ptype)
{
    return AddGlobalVar(std::make_unique<GlobalVar>(name, ptype));
}

Function* Module::GetFunction(const std::string& name)
{
    int index = symindex_.at(name);
    return At(index)->As<Function>();
}

GlobalVar* Module::GetGlobalVar(const std::string& name)
{
    int index = symindex_.at(name);
    return At(index)->As<GlobalVar>();
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

void Function::Accept(IRVisitor* v)
{
    v->VisitFunction(this);
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
    auto var = type_->ToString() + ' ' + Name() + " = ";
    var += tree_->ToString() + ';';
    return var;
}

void GlobalVar::Accept(IRVisitor* v)
{
    v->VisitGlobalVar(this);
}

std::stack<std::unique_ptr<Node>> GlobalVar::stack_;

void GlobalVar::MergeNode(Instr::InstrId op)
{
    if (op == Instr::InstrId::geteleptr)
    {
        auto oper = std::move(stack_.top());
        stack_.pop();
        stack_.push(std::make_unique<UnaryNode>(op, std::move(oper)));
    }
    else
    {
        auto right = std::move(stack_.top());
        stack_.pop();
        auto left = std::move(stack_.top());
        stack_.pop();
        stack_.push(std::make_unique<BinaryNode>(
            std::move(left), op, std::move(right)));
    }
}

void GlobalVar::AddOpNode(const IROperand* op, int pop)
{
    // pop one element if pop == 1,
    // two if pop == 2.
    stack_.pop();
    if (pop == 2)
        stack_.pop();
    stack_.push(std::make_unique<OpNode>(op));
}

void GlobalVar::AddOpNode(const IROperand* op)
{
    stack_.push(std::make_unique<OpNode>(op));
}


BasicBlock* BasicBlock::CreateBasicBlock(Function* func, const std::string& name)
{
    std::unique_ptr<BasicBlock> bb = std::make_unique<BasicBlock>(name);
    auto raw = bb.get();
    func->Append(std::move(bb));
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

void BasicBlock::Accept(IRVisitor* v)
{
    v->VisitBasicBlock(this);
}

void BasicBlock::MergePools(BasicBlock* bb)
{
    Pool<IRType>::Merge(bb);
    Pool<IROperand>::Merge(bb);
}
