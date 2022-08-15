#include "Expression.h"

IR Expression::Generate(SymbolTable& st) const
{
    IR lotsOfExpr{};
    for (const AssignExpr& ae : exprs)
    {
        IR temp = ae.Generate(st);
        lotsOfExpr.Join(temp);
    }

    return lotsOfExpr;
}
