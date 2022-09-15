#ifndef _TYPENAME_H_
#define _TYPENAME_H_

#include "../../utilities/SymbolTable.h"

class TypeName
{
public:
    size_t GetSize(const SymbolTable&) const { return 0; };
    size_t GetAlign(const SymbolTable&) const { return 0; };
    TypeSpec GetSpec(const SymbolTable&) const { return 0; };
};

#endif // _TYPENAME_H_
