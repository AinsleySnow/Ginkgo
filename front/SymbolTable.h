#ifndef _SYMBOLTABLE_H_
#define _SYMBOLTABLE_H_

#include <map>
#include <string>

class SymbolTable
{
private:
    std::map<std::string, int> content {};

public:
    SymbolTable();
    int Get(const std::string&);
    void Set(const std::string&, int);
};

#endif // _SYMBOLTABLE_H_
