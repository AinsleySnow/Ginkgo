#include "XorExpr.h"
#include "ExprGenerateHelper.h"

IR XorExpr::Generate(SymbolTable& st) const 
{
    if (!xorExpr)
    // xor-expr -> and-expr
        return andExpr->Generate(st);
    else
    // xor-expr -> xor-expr ^ and-expr
    {
        ExprGenerateHelper(IROper::bitwisexor, xorExpr, andExpr, st);
    }
}
