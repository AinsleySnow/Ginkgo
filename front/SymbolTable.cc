#include "SymbolTable.h"

int SymbolTable::Get(const std::string& name)
{
    return content[name];
}

void SymbolTable::Set(const std::string& name, int value)
{
    content[name] = value;
}
