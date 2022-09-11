#include "MultiExpr.h"
#include "ExprGenerateHelper.h"

IR MultiExpr::Generate(SymbolTable& st) const 
{
    if (!multiExpr)
    // multiplicative-expr -> cast-expr
        return castExpr->Generate(st);
    else
    // multiplicative-expr -> multiplicative-expr * cast-expr
    // multiplicative-expr -> multiplicative-expr / cast-expr
    // multiplicative-expr -> multiplicative-expr % cast-expr
    {
        IROper irop{};
        if (op == Tag::star) irop = IROper::multiple;
        else if (op == Tag::slash) irop = IROper::divide;
        else irop = IROper::mod;

        DeclareHelper(multiExpr, castExpr);
        OperationHelper(*, irop, IROper::multiple);
        OperationHelper(/, irop, IROper::divide);
        OperationHelper(%, irop, IROper::mod);
        ExprGenerateHelper(irop, st);
    }
}
