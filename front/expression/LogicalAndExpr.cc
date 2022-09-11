#include "LogicalAndExpr.h"
#include "ExprGenerateHelper.h"

IR LogicalAndExpr::Generate(SymbolTable& st) const 
{
    if (!logicalAndExpr)
    // logical-and-expr -> inclusive-or-expr
        return inclOrExpr->Generate(st);
    else
    // logical-and-expr -> logical-and-expr && inclusive-or-expr
    {
        DeclareHelper(logicalAndExpr, inclOrExpr);
        firstGen.Identifier = temp1 && temp2;
        ExprGenerateHelper(IROper::logicaland, st); 
    }
}
