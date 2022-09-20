#include "Expression.h"

IR Expression::Generate(SymbolTable& st) const
{
    IR lotsOfExpr{};
    if (exprs.size() == 1)
        return exprs.begin()->Generate(st);
    for (const AssignExpr& ae : exprs)
    {
        IR temp = ae.Generate(st);
        lotsOfExpr.Join(temp);
    }

    return lotsOfExpr;
}
