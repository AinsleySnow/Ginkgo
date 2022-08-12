#include "AddExpr.h"
#include "ExprGenerateHelper.h"

IR AddExpr::Generate(SymbolTable& st) const 
{
    if (multiExpr)
    // add-expr -> multiplicative-expr
        return multiExpr->Generate(st);
    else
    // add-expr -> add-expr + multiplicative-expr
    // add-expr -> add-expr - multiplicative-expr
    {
        IROper irop{};
        if (op == Tag::plus) irop = IROper::add;
        else irop = IROper::subtract;
        ExprGenerateHelper(irop, addExpr, multiExpr, st);
    }
}
