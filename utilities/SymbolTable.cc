#include "SymbolTable.h"
#include <memory>
#include <cstdio>

Entry SymbolTable::GetSymbol(const std::string& name)
{
    return content[name];
}

void SymbolTable::RegisterSymbol(Declaration& decl)
{
    std::unique_ptr<InitDeclList> list = std::move(decl.initDeclList);
    std::unique_ptr<DeclSpec> specifier = std::move(decl.declSpec);

    for (const InitDecl& id : list->initList)
    {
        Entry e{ specifier->GetSpec(), specifier->GetQual() };
        content[id.GetName()] = e;
    }
}

void SymbolTable::PrintSymbols()
{
    for (auto pair : content)
    {
        printf("name: %s\n", pair.first.c_str());
        printf("TypeSpec: %d\n", static_cast<int>(pair.second.specifier));
        printf("TypeQual: %d\n\n", static_cast<int>(pair.second.quailfier));
    }
}
