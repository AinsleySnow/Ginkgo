#ifndef _IGENERABLE_H_
#define _IGENERABLE_H_

#include "../utilities/Constant.h"
#include "../utilities/IR.h"
#include "../utilities/SymbolTable.h"

class IGenerable
{
public:
    virtual IR Generate(SymbolTable&) const = 0;
};

#endif // _IGENERABLE_H_
