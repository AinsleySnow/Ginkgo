#include "SymbolTable.h"
#include <memory>
#include <cstdio>

Entry SymbolTable::operator[](const std::string& name) const
{
    return GetSymbol(name);
}

std::string SymbolTable::GenerateTempVar(TypeSpec ts)
{
    std::string name{ '$' + std::to_string(uniqueVarIndex) };
    content[name] = { ts };
    return name;
}

Entry SymbolTable::GetSymbol(const std::string& name) const
{
    return content.at(name);
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

void SymbolTable::PrintSymbols() const
{
    for (auto pair : content)
    {
        printf("name: %s\n", pair.first.c_str());
        printf("TypeSpec: %d\n", static_cast<int>(pair.second.specifier));
        printf("TypeQual: %d\n\n", static_cast<int>(pair.second.quailfier));
    }
}
