#ifndef _LOGICALOREXPR_H_
#define _LOGICALOREXPR_H_

#include "../Node.h"
#include "../IGenerable.h"
#include "LogicalAndExpr.h"
#include <memory>

class LogicalOrExpr : public Node, public IGenerable
{
public:
    std::unique_ptr<LogicalAndExpr> logicalAndExpr{};
    std::unique_ptr<LogicalOrExpr> logicalOrExpr{};

    LogicalOrExpr(std::unique_ptr<LogicalAndExpr>&&);
    LogicalOrExpr(std::unique_ptr<LogicalOrExpr>&&, std::unique_ptr<LogicalAndExpr>&&);

    IR Generate(SymbolTable&) const override;
};

#endif // _LOGICALOREXPR_H_
