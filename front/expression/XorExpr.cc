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
        DeclareHelper(xorExpr, andExpr);
        firstGen.Identifier = temp1 ^ temp2;
        ExprGenerateHelper(IROper::bitwisexor, st);    
    }
}
