#ifndef _CONDEXPR_H_
#define _CONDEXPR_H_

#include "../Node.h"
#include "../IGenerable.h"
#include "Expression.h"
#include "LogicalOrExpr.h"
#include <memory>

class CondExpr : public Node, public IGenerable
{
public:
    std::unique_ptr<LogicalOrExpr> logicalOrExpr{};
    std::unique_ptr<Expression> expression{};
    std::unique_ptr<CondExpr> condExpr{};

    CondExpr(std::unique_ptr<LogicalOrExpr>&&);
    CondExpr(std::unique_ptr<LogicalOrExpr>&&, 
        std::unique_ptr<Expression>&&, std::unique_ptr<CondExpr>&&);

    IR Generate(SymbolTable&) const override;
};

#endif // _CONDEXPR_H_
