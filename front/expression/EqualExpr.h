#ifndef _EQUALEXPR_H_
#define _EQUALEXPR_H_

#include "../Node.h"
#include "../IGenerable.h"
#include <memory>

class RelationExpr;
#include "RelationExpr.h"

class EqualExpr : public Node, public IGenerable
{
public:
    std::unique_ptr<EqualExpr> equalExpr{};
    Tag op;
    std::unique_ptr<RelationExpr> relationExpr{};

    EqualExpr(std::unique_ptr<RelationExpr>&& re) :
        relationExpr(std::move(re)) {}
    EqualExpr(std::unique_ptr<EqualExpr>&& ee, Tag t, std::unique_ptr<RelationExpr>&& re) :
        equalExpr(std::move(ee)), op(t), relationExpr(std::move(re)) {}

    IR Generate(SymbolTable&) const override;
};

#endif // _EQUALEXPR_H_
