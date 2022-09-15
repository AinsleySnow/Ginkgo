#ifndef _LOGICALANDEXPR_H_
#define _LOGICALANDEXPR_H_

#include "../Node.h"
#include "../IGenerable.h"
#include <memory>

class InclOrExpr;
#include "InclOrExpr.h"

class LogicalAndExpr : public Node, public IGenerable
{
public:
    std::unique_ptr<LogicalAndExpr> logicalAndExpr{};
    std::unique_ptr<InclOrExpr> inclOrExpr{};

    LogicalAndExpr(std::unique_ptr<InclOrExpr>&& ioe) :
        inclOrExpr(std::move(ioe)) {};
    LogicalAndExpr(std::unique_ptr<LogicalAndExpr>&& lae, std::unique_ptr<InclOrExpr>&& ioe) :
        logicalAndExpr(std::move(lae)), inclOrExpr(std::move(ioe)) {};

    IR Generate(SymbolTable&) const override;
};

#endif // _LOGICALANDEXPR_H_
