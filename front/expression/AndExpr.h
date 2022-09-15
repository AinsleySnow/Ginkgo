#ifndef _ANDEXPR_H_
#define _ANDEXPR_H_

#include "../Node.h"
#include "../IGenerable.h"
#include "EqualExpr.h"
#include <memory>

class EqualExpr;

class AndExpr : public Node, public IGenerable
{
public:
    std::unique_ptr<AndExpr> andExpr{};
    std::unique_ptr<EqualExpr> equalExpr{};

    AndExpr(std::unique_ptr<EqualExpr>&& ee) :
        equalExpr(std::move(ee)) {};
    AndExpr(std::unique_ptr<AndExpr>&& ae, std::unique_ptr<EqualExpr>&& ee) :
        andExpr(std::move(ae)), equalExpr(std::move(ee)) {};

    IR Generate(SymbolTable&) const override;
};

#endif // _ANDEXPR_H_
