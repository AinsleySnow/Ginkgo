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
};

class SymbolTable
{
private:
    std::map<std::string, Entry> content {};

public:
    Entry GetSymbol(const std::string&);
    void RegisterSymbol(Declaration&);

    void PrintSymbols();
};

#endif // _SYMBOLTABLE_H_
