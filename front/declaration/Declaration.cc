#include "Declaration.h"

Declaration::Declaration(const InitDeclList& id, const DeclSpec& ds) : Node(Tag::declaration)
{
    initDeclList = id;
    declSpec = ds;
}
