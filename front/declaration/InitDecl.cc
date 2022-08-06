#include "InitDecl.h"

InitDecl::InitDecl(std::unique_ptr<Declarator>& d)
{
    declarator = std::move(d);
}

InitDecl::InitDecl(std::unique_ptr<Declarator>& d, std::unique_ptr<Init>& i)
{
    initializer = std::move(i);
    declarator = std::move(d);
}
