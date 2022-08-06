#include "InitDecl.h"

InitDecl::InitDecl(const InitDecl& id)
{
    InitDecl& nonconst = const_cast<InitDecl&>(id);
    initializer = std::move(nonconst.initializer);
    declarator = std::move(nonconst.declarator);
}

InitDecl::InitDecl(std::unique_ptr<Declarator>&& d)
{
    declarator = std::move(d);
}

InitDecl::InitDecl(std::unique_ptr<Declarator>&& d, std::unique_ptr<Init>&& i)
{
    initializer = std::move(i);
    declarator = std::move(d);
}

const std::string& InitDecl::GetName() const
{
    return declarator->directdecl->identifier;
}
