#include "visitast/Scope.h"


Identifier* Scope::FindingHelper(const std::string& name, Identifier::IdentType it) const
{
    auto result = identmap_.find(name);
    for (; result != identmap_.end(); ++result)
        if (result->second->GetIdentType() == it)
            return result->second.get();
    return nullptr;
}

void Scope::Extend(const Scope& scope)
{
    for (auto& [name, ident] : scope.identmap_)
        identmap_.emplace(name, ident->Clone());
}

const Object* Scope::GetObject(const std::string& name) const
{
    Identifier* ident = FindingHelper(name, Identifier::IdentType::obj);
    return ident ? ident->ToObject() : nullptr;
}

const Func* Scope::GetFunc(const std::string& name) const
{
    Identifier* ident = FindingHelper(name, Identifier::IdentType::func);
    return ident ? ident->ToFunc() : nullptr;
}

const Label* Scope::GetLabel(const std::string& name) const
{
    Identifier* ident = FindingHelper(name, Identifier::IdentType::obj);
    return ident ? ident->ToLabel() : nullptr;
}

const Typedef* Scope::GetTypedef(const std::string& name) const
{
    Identifier* ident = FindingHelper(name, Identifier::IdentType::obj);
    return ident ? ident->ToTypedef() : nullptr;
}

const CustomedType* Scope::GetCustomed(const std::string& name) const
{
    Identifier* ident = FindingHelper(name, Identifier::IdentType::custom);
    return ident ? ident->ToCustomed() : nullptr;
}

const Member* Scope::GetMember(const std::string& name) const
{
    Identifier* ident = FindingHelper(name, Identifier::IdentType::member);
    return ident ? ident->ToMember() : nullptr;
}


Object* Scope::AddObject(
    const std::string& name, const CType* ty, const Register* reg)
{
    auto object = std::make_unique<Object>(name, ty, reg);
    auto pobj = object.get();
    identmap_.emplace(name, std::move(object));
    return pobj;
}

Func* Scope::AddFunc(
    const std::string& name, const CFuncType* functy, const Register* addr)
{
    auto func = std::make_unique<Func>(name, functy, addr);
    auto pfunc = func.get();
    identmap_.emplace(name, std::move(func));
    return pfunc;
}

Label* Scope::AddLabel(const std::string& name)
{
    auto label = std::make_unique<Label>(name);
    auto plabel = label.get();
    identmap_.emplace(name, std::move(label));
    return plabel;
}

Typedef* Scope::AddTypedef(const std::string& name, const CType* ty)
{
    auto typedef_ = std::make_unique<Typedef>(name, ty);
    auto ptypedef = typedef_.get();
    identmap_.emplace(name, std::move(typedef_));
    return ptypedef;
}

Typedef* Scope::AddTypedef(const std::string& name, const Typedef* def)
{
    auto typedef_ = std::make_unique<Typedef>(name, def);
    auto ptypedef = typedef_.get();
    identmap_.emplace(name, std::move(typedef_));
    return ptypedef;
}

CustomedType* Scope::AddCustomed(const std::string& name, const CType* ty)
{
    auto customed = std::make_unique<CustomedType>(name, ty);
    auto pcustomed = customed.get();
    identmap_.emplace(name, std::move(customed));
    return pcustomed;
}

Member* Scope::AddMember(
    const std::string& name, const CType* ty, const IntConst* val)
{
    auto member = std::make_unique<Member>(name, ty);
    auto pmember = member.get();
    member->Value() = val;
    identmap_.emplace(name, std::move(member));
    return pmember;
}


const Object* ScopeStack::SearchObject(const std::string& name)
{
    if (blockscope_)
    {
        const Object* target = blockscope_->GetObject(name);
        if (target) return target;
    }

    for (auto iter = stack_.rbegin(); iter != stack_.rend(); ++iter)
    {
        const Object* target = (*iter)->GetObject(name);
        if (target)
        {
            blockscope_ = iter->get();
            return target;
        }
    }

    return nullptr;
}

const Func* ScopeStack::SearchFunc(const std::string& name)
{
    if (filescope_)
    {
        const Func* func = filescope_->GetFunc(name);
        if (func) return func;
    }

    for (auto iter = stack_.rbegin(); iter != stack_.rend(); ++iter)
    {
        if ((*iter)->GetScopeType() != Scope::ScopeType::file)
            continue;
        const Func* target = (*iter)->GetFunc(name);
        if (target)
        {
            filescope_ = iter->get();
            return target;
        }
    }

    return nullptr;
}

const Label* ScopeStack::SearchLabel(const std::string& name)
{
    if (funcscope_)
    {
        const Label* label = funcscope_->GetLabel(name);
        if (label) return label;
    }

    for (auto iter = stack_.rbegin(); iter != stack_.rend(); ++iter)
    {
        if ((*iter)->GetScopeType() != Scope::ScopeType::func)
            continue;
        const Label* target = (*iter)->GetLabel(name);
        if (target)
        {
            funcscope_ = iter->get();
            return target;
        }
    }

    return nullptr;
}

const CType* ScopeStack::UnderlyingTydef(const std::string& name)
{
    auto n = name;
    for (auto iter = stack_.rbegin(); iter != stack_.rend(); )
    {
        const Typedef* target = (*iter)->GetTypedef(n);
        if (target)
        {
            const auto& ty = target->Type();
            if (std::holds_alternative<const CType*>(ty))
                return std::get<const CType*>(ty);

            // if the typedef is of another typedef
            auto tydef = std::get<const Typedef*>(ty);
            n = tydef->GetName();
            continue; // stay in current scope
        }
        ++iter;
    }
    return nullptr;
}

const CustomedType* ScopeStack::SearchCustomed(const std::string& name)
{
    for (auto iter = stack_.rbegin(); iter != stack_.rend(); ++iter)
    {
        const CustomedType* target = (*iter)->GetCustomed(name);
        if (target) return target;
    }
    return nullptr;
}

const Member* ScopeStack::SearchMember(const std::string& name)
{
    for (auto iter = stack_.rbegin(); iter != stack_.rend(); ++iter)
    {
        const Member* target = (*iter)->GetMember(name);
        if (target) return target;
    }
    return nullptr;
}


void ScopeStack::PushNewScope(Scope::ScopeType scpty)
{
    stack_.push_back(std::make_unique<Scope>(scpty));
    if (scpty == Scope::ScopeType::file)
        filescope_ = stack_.back().get();
    else if (scpty == Scope::ScopeType::block)
        blockscope_ = stack_.back().get();
}

void ScopeStack::PopScope()
{
    if (stack_.back().get() == filescope_)
        filescope_ = nullptr;
    if (stack_.back().get() == blockscope_)
        blockscope_ = nullptr;
    else if (stack_.back().get() == blockscope_)
        blockscope_ = nullptr;
    stack_.pop_back();
}

void ScopeStack::LoadNewScope(std::unique_ptr<Scope> scope)
{
    stack_.push_back(std::move(scope));
}

std::unique_ptr<Scope> ScopeStack::RestoreScope()
{
    auto back = std::move(stack_.back());
    stack_.pop_back();
    return std::move(back);
}


Scope& ScopeStack::Top()
{
    return *(stack_.back());
}

Scope& ScopeStack::File()
{
    return *(stack_.front());
}
