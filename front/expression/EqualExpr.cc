#include "EqualExpr.h"
#include "ExprGenerateHelper.h"

IR EqualExpr::Generate(SymbolTable& st) const 
{
    if (!equalExpr)
    // equality-expr -> relational-expr
        return relationExpr->Generate(st);
    else
    // equality-expr -> equality-expr == relational-expr
    // equality-expr -> equality-expr != relational-expr
    {
        IROper irop{};
        if (op == Tag::equal) irop = IROper::equal;
        else irop = IROper::notequal;
        
        DeclareHelper(equalExpr, relationExpr);
        OperationHelper(==, irop, IROper::equal);
        OperationHelper(!=, irop, IROper::notequal);
        ExprGenerateHelper(irop, st);    
    }
}
