#include "LogicalOrExpr.h"
#include "ExprGenerateHelper.h"

IR LogicalOrExpr::Generate(SymbolTable& st) const 
{
    if (!logicalOrExpr)
    // logical-or-expr -> logical-and-expr
        return logicalAndExpr->Generate(st);
    else
    // logical-or-expr -> logical-or-expr || logical-and-expr
    {
        DeclareHelper(logicalOrExpr, logicalAndExpr);
        firstGen.Identifier = temp1 || temp2;
        ExprGenerateHelper(IROper::logicaland, st); 
    }
}
