#include "PrimaryExpr.h"

IR PrimaryExpr::Generate(SymbolTable& st) const
{
    if (literal)
    // primary-expr -> identifier
    // primary-expr -> string
    {    
        IR ir{};
        ir.Identifier = *literal;
        return ir;
    }
    else if (constValue.has_value())
    // primary-expr -> constant
    {
        IR ir{};
        ir.Identifier = constValue.value();
        return ir;
    }
    else
    // primary-expr -> ( expression )
        return expression->Generate(st);
}

