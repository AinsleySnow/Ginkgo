#include "ShiftExpr.h"
#include "ExprGenerateHelper.h"

IR ShiftExpr::Generate(SymbolTable& st) const 
{
    if (addExpr)
    // shift-expr -> add-expr
        return addExpr->Generate(st);
    else
    // shift-expr -> shift-expr << add-expr
    // shift-expr -> shift-expr >> add-expr
    {
        IROper irop{};
        if (op == Tag::lshift) irop = IROper::lshift;
        else irop = IROper::rshift;
        ExprGenerateHelper(irop, shiftExpr, addExpr, st);
    }
}