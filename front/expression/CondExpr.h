#ifndef _CONDEXPR_H_
#define _CONDEXPR_H_

#include "../Node.h"
#include "../IGenerable.h"
#include <memory>

class Expression;
#include "Expression.h"
class LogicalOrExpr;
#include "LogicalOrExpr.h"

class CondExpr : public Node, public IGenerable
{
public:
    std::unique_ptr<LogicalOrExpr> logicalOrExpr{};
    std::unique_ptr<Expression> expression{};
    std::unique_ptr<CondExpr> condExpr{};

    CondExpr(std::unique_ptr<LogicalOrExpr>&& loe) :
        logicalOrExpr(std::move(loe)) {};
    CondExpr(std::unique_ptr<LogicalOrExpr>&& loe, 
        std::unique_ptr<Expression>&& e, std::unique_ptr<CondExpr>&& ce) :
        logicalOrExpr(std::move(loe)), expression(std::move(e)), condExpr(std::move(ce)) {};

    IR Generate(SymbolTable&) const override;
};

#endif // _CONDEXPR_H_
