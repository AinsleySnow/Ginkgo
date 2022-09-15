#ifndef _SYMBOLTABLE_H_
#define _SYMBOLTABLE_H_

#include <map>
#include <memory>
#include <string>
#include "IR.h"
#include "EnumsforEntry.h"
#include "TypeSpec.h"

class Declaration;
class InitDeclList;

struct Entry
{
    TypeSpec specifier;
    unsigned quailfier;
    uint64_t size;
    std::shared_ptr<IR> initial;
};

class SymbolTable
{
private:
    // Give each temporary variable a unique name.
    // Their names all start with '$'.
    size_t uniqueVarIndex{};
    std::map<std::string, Entry> content {};

public:
    Entry operator[](const std::string&) const;

    std::string GenerateTempVar(TypeSpec);
    Entry GetSymbol(const std::string&) const;
    void DeleteSymbol(const std::string&);
    void RegisterSymbol(Declaration&);

    void PrintSymbols() const;
};

#endif // _SYMBOLTABLE_H_
