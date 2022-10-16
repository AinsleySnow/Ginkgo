#include "CondExpr.h"
#include <string>

const int Type::undef;
IR CondExpr::Generate(SymbolTable &st) const
{
    if (!condExpr)
        // conditional-expr -> logical-or-expr
        return logicalOrExpr->Generate(st);

    // conditional-expr -> logical-or-expr ? expr : conditional-expr
    IR logicalOrGen = logicalOrExpr->Generate(st);
    std::string lastVar = logicalOrGen.GetLastVar();
    std::string assignTo = st.GenerateTempVar(Type::undef);
    IR exprGen = expression->Generate(st);
    IR condGen = expression->Generate(st);

    std::string lableTrue = IR::GetLable();
    std::string lableFalse = IR::GetLable();
    std::string end = IR::GetLable();
    Quadruple quadTrue{
        IROper::jmptrue,
        lastVar,
        lableTrue
    };
    Quadruple jumpToEnd{
        IROper::jmp,
        end
    };

    logicalOrGen.Append(quadTrue);
    logicalOrGen.Join(condGen);
    Quadruple assign{
        IROper::assign,
        condGen.GetLastVar(),
        assignTo
    };
    logicalOrGen.Append(assign);
    logicalOrGen.Append(jumpToEnd);

    logicalOrGen.AppendLable(lableTrue);
    logicalOrGen.Join(exprGen);
    assign.arg2 = exprGen.GetLastVar();
    logicalOrGen.Append(assign);
    logicalOrGen.AppendLable(end);

    return logicalOrGen;
}
