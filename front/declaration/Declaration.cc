#include "Declaration.h"

Declaration::Declaration(const InitDeclList& id, const DeclSpec& ds) : Node(Tag::declaration)
{
    initDeclList = id;
    declSpec = ds;
}

DeclSpec Declaration::GetSpecifier()
{
    return declSpec;
}

InitDeclList Declaration::GetInitList()
{
    return initDeclList;
}
