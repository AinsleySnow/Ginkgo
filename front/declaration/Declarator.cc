#include "Declarator.h"

Declarator::Declarator(const DirDecl& dd) : Node(Tag::declaration)
{
    directdecl = DirDecl(dd);
}
