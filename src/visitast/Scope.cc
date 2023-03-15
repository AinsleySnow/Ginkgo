#include "visitast/Scope.h"
#include "ast/CType.h"


Identifier* Scope::FindingHelper(const std::string& name, Identifier::IdentType it) const
{
    auto result = identmap_.find(name);
    for (; result != identmap_.end(); ++result)
        if (result->second->GetIdentType() == it)
            return result->second.get();
    return nullptr;
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

const Member* Scope::GetMember(const std::string& name) const
{
    Identifier* ident = FindingHelper(name, Identifier::IdentType::obj);
    return ident ? ident->ToMember() : nullptr;
}


void Scope::AddObject(
    const std::string& name, const CType* ty, const Register* reg)
{
    auto object = std::make_unique<Object>(name, ty, reg);
    identmap_.emplace(name, std::move(object));
}

void Scope::AddFunc(const std::string& name, const CFuncType* functy, const Register* addr)
{
    auto func = std::make_unique<Func>(name, functy, addr);
    identmap_.emplace(name, std::move(func));
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
    if (stack_.back()->GetScopeType() == Scope::ScopeType::file)
        filescope_ = nullptr;
    else if (stack_.back()->GetScopeType() == Scope::ScopeType::block)
        blockscope_ = nullptr;
    stack_.pop_back();
}


Scope& ScopeStack::Top()
{
    return *(stack_.back());
}

Scope& ScopeStack::File()
{
    return *(stack_.front());
}
