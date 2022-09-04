#include "PostfixExpr.h"

IR PostfixExpr::Generate(SymbolTable& st) const
{
    if (primaryExpr)
    // postfix-expr -> primary-expr
        return primaryExpr->Generate(st);

    else if (postfixExpr && expression)
    // postfix-expr -> postfix-expr [ expression ]
    {
        // TODO
    }
    else if (postfixExpr && op == Tag::left)
    // postfix-expr -> postfix-expr ( )
    {
        // TODO
    }
    else if (postfixExpr && argvExprList)
    // postfix-expr -> postfix-expr ( argv-expr-list )
    {
        // TODO
    }
    else if (postfixExpr && op == Tag::dot)
    // postfix-expr -> postfix-expr . identifier
    {
        // TODO
    }
    else if (postfixExpr && op == Tag::arrow)
    // postfix-expr -> postfix-expr -> identifier
    {
        // TODO
    }
    else if (postfixExpr && (op == Tag::inc || op == Tag::dec))
    // postfix-expr -> postfix-expr ++
    // postfix-expr -> postfix-expr --
    {
        IR postfixGen = postfixExpr->Generate(st);
        std::string assignTo = st.GenerateTempVar(st[postfixGen.GetLastVar()].specifier);

        Quadruple operation {
            op == Tag::inc ? IROper::inc : IROper::dec,
            postfixGen.GetLastVar()
        };
        Quadruple assign {
            IROper::assign,
            postfixGen.GetLastVar(),
            assignTo
        };

        postfixGen.Append(operation);
        postfixGen.Append(assign);
        return postfixGen;
    }
    else if (typeName && initList)
    // postfix-expr -> ( type-name ) { init-list }
    // postfix-expr -> ( type-name ) { init-list, }
    {
        // TODO
    }
}
