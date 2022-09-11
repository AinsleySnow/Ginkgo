#include "AndExpr.h"
#include "ExprGenerateHelper.h"

IR AndExpr::Generate(SymbolTable& st) const 
{
    if (!andExpr)
    // and-expr -> euqality-expr
        return equalExpr->Generate(st);
    else
    // and-expr -> and-expr & equality-expr
    {
        DeclareHelper(andExpr, equalExpr);
        firstGen.Identifier = temp1 & temp2;
        ExprGenerateHelper(IROper::bitwiseand, st);
    }   
}
