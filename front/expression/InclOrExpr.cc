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
        DeclareHelper(inclOrExpr, xorExpr);
        firstGen.Identifier = temp1 | temp2;
        ExprGenerateHelper(IROper::bitwiseor, st);     
    }
}
