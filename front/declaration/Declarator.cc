#include "Declarator.h"

Declarator::Declarator(std::unique_ptr<DirDecl>&& dd)
{
    directdecl = std::move(dd);
}
