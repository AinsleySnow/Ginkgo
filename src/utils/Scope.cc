#include "Scope.h"
#include "ast/Object.h"
#include "ast/Label.h"
#include "types/Type.h"


Identifier* Scope::GetIdentifier(const std::string& name) const
{
    auto ident = identmap_.at(name);
    if (ident) return ident.get();
    else return nullptr;
}

Identifier* Scope::operator[](const std::string& name) const
{
    return GetIdentifier(name);
}

void Scope::AddIdentifier(std::shared_ptr<Identifier> ident)
{
    identmap_[ident->GetName()] = ident;
}


const Object* Scope::GenerateTempVar()
{
    auto name = '@' + std::to_string(tempvarindex_++);
    auto pobj = std::make_shared<Object>(name);
    identmap_[name] = pobj;
    return pobj.get();
}

const Object* Scope::GetLastestTempVar(unsigned long back = 0) const
{
    auto name = '@' + std::to_string(tempvarindex_ - back);
    return GetIdentifier(name)->ToObject();
}

const Label* Scope::GenerateTempLabel()
{
    auto name = '#' + std::to_string(templabelindex_);
    auto plabel = std::make_shared<Label>(name);
    identmap_[name] = plabel;
    return plabel.get();
}


const Scope* Scope::GetParent() const
{
    return parent_;
}
