#include "DirDecl.h"

DirDecl::DirDecl(const std::string& s) : Node(Tag::direct_declarator)
{
    identifier = s;
}

DirDecl::DirDecl(const Node& n) : Node(Tag::direct_declarator)
{
    identifier = n.GetLiteral();
}

std::string DirDecl::GetIdentifier() const
{
    return identifier;
}
