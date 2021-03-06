#ifndef _SYMBOLTABLE_H_
#define _SYMBOLTABLE_H_

#include <map>
#include <string>
#include "EnumsforEntry.h"
#include "declaration/InitDeclList.h"
#include "declaration/Declaration.h"

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
    SymbolTable();
    Entry Get(const std::string&);
    void RegisterSymbol(const Declaration&);
};

#endif // _SYMBOLTABLE_H_
