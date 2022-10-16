#include "Scope.h"


const auto Scope::GetIdentifier(const std::string& name) const
{
    auto ident = identmap_.at(name);
    if (ident) return ident;
    else return nullptr;
}

const auto Scope::GetParent() const
{
    return parent_;
}

void Scope::AddIdentifier(std::shared_ptr<Identifier> ident)
{
    identmap_[ident->GetName()] = ident;
}
