#include "AddExpr.h"
#include "ExprGenerateHelper.h"

IR AddExpr::Generate(SymbolTable& st) const 
{
    if (!addExpr)
    // add-expr -> multiplicative-expr
        return multiExpr->Generate(st);
    else
    // add-expr -> add-expr + multiplicative-expr
    // add-expr -> add-expr - multiplicative-expr
    {
        IROper irop{};
        if (op == Tag::plus) irop = IROper::add;
        else irop = IROper::subtract;
        DeclareHelper(addExpr, multiExpr);
        YieldHelper();
        OperationHelper(+, irop, IROper::add);
        OperationHelper(-, irop, IROper::subtract);
        ExprGenerateHelper(irop, st);
    }
}
