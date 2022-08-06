#include "SymbolTable.h"

Entry SymbolTable::GetSymbol(const std::string& name)
{
    return content[name];
}

void SymbolTable::RegisterSymbol(const Declaration& decl)
{
    InitDeclList list = decl.GetInitList();
    DeclSpec specifier = decl.GetSpecifier();

    for (InitDecl id : list.GetIter())
    {
        Entry e{ specifier.GetSpec(), specifier.GetQual() };
        content[id.GetName()] = e;
    }
}
