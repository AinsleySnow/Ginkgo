#ifndef _IGENERABLE_H_
#define _IGENERABLE_H_

class IR;
class SymbolTable;

class IGenerable
{
public:
    virtual IR Generate(SymbolTable&) const = 0;
};

#endif // _IGENERABLE_H_
