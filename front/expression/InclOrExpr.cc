#include "InclOrExpr.h"
#include "ExprGenerateHelper.h"

IR InclOrExpr::Generate(SymbolTable& st) const 
{
    if (!inclOrExpr)
    // inclusive-or-expr -> add-expr
        return xorExpr->Generate(st);
    else
    // inclusive-or-expr -> inclusive-or-expr | xor-expr
    {
        ExprGenerateHelper(IROper::bitwiseor, inclOrExpr, xorExpr, st);
    }
}
