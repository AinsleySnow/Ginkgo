#include "RelationExpr.h"
#include "ExprGenerateHelper.h"

IR RelationExpr::Generate(SymbolTable& st) const 
{
    if (!relationExpr)
    // relational-expr -> shift-expr
        return shiftExpr->Generate(st);
    else
    // relational-expr -> relational-expr < shift-expr
    // relational-expr -> relational-expr > shift-expr
    // relational-expr -> relational-expr <= shift-expr
    // relational-expr -> relational-expr >= shift-expr
    {
        IROper irop{};
        if (op == Tag::lessthan) irop = IROper::less;
        else if (op == Tag::greathan) irop = IROper::great;
        else if (op == Tag::lessequal) irop = IROper::lessequal;
        else if (op == Tag::greatequal) irop = IROper::greatequal;
        
        DeclareHelper(relationExpr, shiftExpr);
        OperationHelper(<, irop, IROper::less);
        OperationHelper(>, irop, IROper::great);
        OperationHelper(<=, irop, IROper::lessequal);
        OperationHelper(>=, irop, IROper::greatequal);
        ExprGenerateHelper(irop, st);
    }
}
