#ifndef _SYMBOLTABLE_H_
#define _SYMBOLTABLE_H_

#include <map>
#include <string>
#include "EnumsforEntry.h"
#include "../front/declaration/InitDeclList.h"
#include "../front/declaration/Declaration.h"

struct Entry
{
    TypeSpec specifier;
    unsigned quailfier;
    uint64_t size;
};

class SymbolTable
{
private:
    // Give each temporary variable a unique name.
    // Their names all start with '$'.
    size_t uniqueVarIndex{};
    std::map<std::string, Entry> content {};

public:
    Entry SymbolTable::operator[](const std::string&) const;

    std::string GenerateTempVar(TypeSpec);
    Entry GetSymbol(const std::string&) const;
    void DeleteSymbol(const std::string&);
    void RegisterSymbol(Declaration&);

    void PrintSymbols() const;
};

#endif // _SYMBOLTABLE_H_
