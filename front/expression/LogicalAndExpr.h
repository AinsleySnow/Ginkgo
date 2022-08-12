#ifndef _LOGICALANDEXPR_H_
#define _LOGICALANDEXPR_H_

#include "../Node.h"
#include "../IGenerable.h"
#include "InclOrExpr.h"
#include <memory>

class LogicalAndExpr : public Node, public IGenerable
{
public:
    std::unique_ptr<InclOrExpr> inclOrExpr{};
    std::unique_ptr<LogicalAndExpr> logicalAndExpr{};

    LogicalAndExpr(std::unique_ptr<InclOrExpr>&&);
    LogicalAndExpr(std::unique_ptr<LogicalAndExpr>&&, std::unique_ptr<InclOrExpr>&&);

    IR Generate(SymbolTable&) const override;
};

#endif // _LOGICALANDEXPR_H_
