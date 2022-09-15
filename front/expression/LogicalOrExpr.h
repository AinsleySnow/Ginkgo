#ifndef _LOGICALOREXPR_H_
#define _LOGICALOREXPR_H_

#include "../Node.h"
#include "../IGenerable.h"
#include <memory>

class LogicalAndExpr;
#include "LogicalAndExpr.h"

class LogicalOrExpr : public Node, public IGenerable
{
public:
    std::unique_ptr<LogicalAndExpr> logicalAndExpr{};
    std::unique_ptr<LogicalOrExpr> logicalOrExpr{};

    LogicalOrExpr(std::unique_ptr<LogicalAndExpr>&& lae) :
        logicalAndExpr(std::move(lae)) {};
    LogicalOrExpr(std::unique_ptr<LogicalOrExpr>&& loe, std::unique_ptr<LogicalAndExpr>&& lae) :
        logicalOrExpr(std::move(loe)), logicalAndExpr(std::move(lae)) {};

    IR Generate(SymbolTable&) const override;
};

#endif // _LOGICALOREXPR_H_
